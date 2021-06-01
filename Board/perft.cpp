#include <cstdlib>
#include <iostream>
#include <chrono>
#include <pthread.h>
#include <cstring>

#include "Board.h"

const int32_t DEPTHDEFAULT = 11;
const int32_t DEPTHMIN = 1;
const int32_t DEPTHMAX = 60;
int32_t depth = DEPTHDEFAULT;

const uint8_t TYPETORUNDEFAULT = 0;
uint8_t typeToRun = TYPETORUNDEFAULT;

const uint8_t MAXPOSSIBLEMOVES = 32;



void printUsage(char **argv) {
    printf("Usage: %s [-depth #] [-type <type>]\n", argv[0]);
    printf("\n\t-depth - Select a depth for the perft to run at, default %d.\n", DEPTHDEFAULT);
    printf("\n\t-type - Selects the type of perft test, default single.");
    printf("\n\t\t(single) - runs a single-threaded perft.");
    printf("\n\t\t(multi) - runs a multi-threaded perft.");
}

void handleArgs(int argc, char **argv) {
    for(int i = 1; i < argc; i++) {
        // Arguments with parameters
        if(strcmp("-depth", argv[i]) == 0 && (i < (argc-1))) {
            i++;
            depth = atoi(argv[i]);
            if(depth < DEPTHMIN || depth > DEPTHMAX) {
                printf("Depth was out of bounds, depth should be between %d and %d.\n", DEPTHMIN, DEPTHMAX);
                exit(1);
            }
        }
        else if(strcmp("-type", argv[i]) == 0 && (i < (argc-1))) {
            i++;
            if (strcmp("single", argv[i]) == 0) {
                typeToRun = 0;
            } else if(strcmp("multi", argv[i]) == 0) {
                typeToRun = 1;
            } else {
                printf("Unknown type to run, use \"%s -help\" for help.\n", argv[0]);
                printUsage(argv);
                exit(0);
            }
        }
        // Arguments with no parameters
        else if(strcmp("-help", argv[i]) == 0) {
            printUsage(argv);
            exit(0);
        }
        else {
            printf("Unknown argument: %s\n", argv[i]);
            printUsage(argv);
            exit(1);
        }
    }
}

void doPerft(Board * board, int32_t depth, uint64_t * output) {
    int8_t moveList[MAXPOSSIBLEMOVES];
    int8_t * last = &moveList[0];

    board->getAllLegalMoves(&last);
    if(depth == 1) {
        (*output) += last - moveList;
        return;
    }

    Position undo = board->pos;
    for(int i = 0; i < (last - moveList); i++) {
        // This just readies the CPU to access pos.
        __builtin_prefetch(&board->pos);
        if(board->doMove(moveList[i])) {
            board->pos = undo;
            (*output)++;
            return;
        }
        doPerft(board, depth-1, output);
        board->pos = undo;
    }
}

void runSingle() {
    Board board;
    std::cout << "Depth: " << depth << std::endl;

    board.print(true);

    uint64_t total = 0;

    auto start = std::chrono::high_resolution_clock::now();

    doPerft(&board, depth, &total);
    
    auto end = std::chrono::high_resolution_clock::now();
    uint64_t time = (uint64_t) (std::chrono::duration_cast<std::chrono::microseconds>(end - start)).count();
    printf("%lu nodes searched in %lu microseconds.", total, time);
    std::cout << " That's about " << (float(total) / (float(time) / float(1000000))) << " nodes per second." << std::endl;
}


typedef struct {
    Board * board;
    uint8_t depth;
    uint64_t * positions;
} Input;

void * threadDoPerft(void * in) {
    Input * input = (Input *) in;
    doPerft(input->board, input->depth, input->positions);
    return (void *) 0;
}

void runMulti() {
    // Multithreaded approach to perft.
    uint8_t THREADS = 4;

    pthread_t * threadList = (pthread_t *) malloc(sizeof(pthread_t) * THREADS);

    Board board;
    int8_t moveList[MAXPOSSIBLEMOVES];
    int8_t * last = &moveList[0];
    uint64_t total = 0;
    depth--;

    auto start = std::chrono::high_resolution_clock::now();
    board.getAllLegalMoves(&last);

    uint8_t movesLength = last - moveList;

    Input ** inputList = (Input **) malloc(movesLength * sizeof(Input *));
    uint64_t ** positionCountList = (uint64_t **) calloc(movesLength, sizeof(uint64_t *));

    uint8_t index;
    // For each set of threads
    for(int i = 0; i < (movesLength + THREADS)/THREADS; i++) {
        for(int j = 0; j < THREADS && j < movesLength; j++) {
            index = i * THREADS + j;
            if(index < movesLength) {
                positionCountList[index] = new uint64_t;
                inputList[index] = new Input();
                inputList[index]->board = new Board();
                inputList[index]->depth = depth;
                inputList[index]->positions = positionCountList[index];
                inputList[index]->board->doMove(moveList[index]);
                pthread_create(&threadList[j], NULL, &threadDoPerft, (void *) inputList[index]);
            }
        }
        for(int j = 0; j < THREADS; j++) {
            void * pv;
            pthread_join(threadList[j], &pv);
            total += *positionCountList[j];
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    uint64_t time = (uint64_t) (std::chrono::duration_cast<std::chrono::microseconds>(end - start)).count();
    printf("%lu nodes searched in %lu microseconds.", total, time);
    std::cout << "That's about " << (float(total) / (float(time) / float(1000000))) << " nodes per second." << std::endl;
}

int main(int argc, char **argv) {
	handleArgs(argc, argv);
    if(typeToRun == 0) {
        runSingle();
    }
    if(typeToRun == 1) {
        runMulti();
    }
}
