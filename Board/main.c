#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

#include "Board.c"

const int32_t DEPTHDEFAULT = 12;
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

void handleArgs(int argc, char ** argv) {
    for(int i = 1; i < argc; ++i) {
        // Arguments with parameters
        if(strcmp("-depth", argv[i]) == 0 && (i < (argc-1))) {
            ++i;
            depth = atoi(argv[i]);
            if(depth < DEPTHMIN || depth > DEPTHMAX) {
                printf("Depth was out of bounds, depth should be between %d and %d.\n", DEPTHMIN, DEPTHMAX);
                exit(1);
            }
        }
        else if(strcmp("-type", argv[i]) == 0 && (i < (argc-1))) {
            ++i;
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

void doPerft(Position * pos, int32_t depth, uint64_t * output) {
    int8_t moveList[MAXPOSSIBLEMOVES];
    int8_t * last = moveList;

    getAllLegalMoves(pos, &last);
    if(depth == 1) {
        (*output) += last - moveList;
        return;
    }

    Position undo = *pos;
    for(int i = 0; __builtin_expect(i < (last - moveList), 1); ++i) {
        // This just readies the CPU to access pos. Adds ~4m Nodes/s
        __builtin_prefetch(pos);
        if(doMove(pos, moveList[i])) {
            *pos = undo;
            ++(*output);
            return;
        }
        doPerft(pos, depth-1, output);
        *pos = undo;
    }
}


void runSingle() {
    Position pos = createBoard("8/8/8/3WB3/3BW3/8/8/8 1 0");
    printf("Depth: %d\n", depth);

    print(&pos, true);

    uint64_t total = 0;

    // Warming up? Seems to make the timed thing faster for some reason.
    doPerft(&pos, depth - 3, &total);

    clock_t timeBegin;
    clock_t timeEnd;
    for(int i = 1; i <= depth; i++) {
        total = 0;

        timeBegin = clock();

        doPerft(&pos, i, &total);
        
        timeEnd = clock();
        double timeTaken = ((double) (timeEnd - timeBegin)) / CLOCKS_PER_SEC;
        timeTaken *= 1000000;
        printf("Depth %d: %llu nodes searched in %.0lf microseconds.", i, total, timeTaken);
        printf("\n\t That's about %.0lf nodes per second.\n\n", 1000000 * ((double) total) / (timeTaken));
    }
}


typedef struct {
    Position * pos;
    uint8_t depth;
    uint64_t * positions;
} Input;

void * threadDoPerft(void * in) {
    Input * input = (Input *) in;
    int8_t moveListList[depth][MAXPOSSIBLEMOVES];
    int8_t * lastPointers [depth];
    doPerft(input->pos, input->depth, input->positions);//, moveListList, lastPointers);
    return (void *) 0;
}

void runMulti() {
    // Multithreaded approach to perft.

    Position pos = createBoard("8/8/8/3WB3/3BW3/8/8/8 1 0");
    int8_t moveList[MAXPOSSIBLEMOVES];
    int8_t * last = &moveList[0];
    uint64_t total = 0;
    depth--;

    clock_t timeBegin = clock();
    getAllLegalMoves(&pos, &last);

    uint8_t movesLength = last - moveList;

    Input ** inputList = (Input **) malloc(movesLength * sizeof(Input *));
    uint64_t ** positionCountList = (uint64_t **) calloc(movesLength, sizeof(uint64_t *));
    pthread_t * threadList = (pthread_t *) malloc(movesLength * sizeof(pthread_t));

    // For each set of threads
    for(int i = 0; i < movesLength; ++i) {
        inputList[i]->pos = &pos;
        inputList[i]->depth = depth;
        inputList[i]->positions = positionCountList[i];
        doMove(inputList[i]->pos, moveList[i]);
        pthread_create(&threadList[i], NULL, &threadDoPerft, (void *) inputList[i]);
    }
    for(int j = 0; j < movesLength; ++j) {
        void * pv;
        pthread_join(threadList[j], &pv);
        total += *positionCountList[j];
    }
    clock_t timeEnd = clock();
    double timeTaken = ((double)(timeEnd - timeBegin))/CLOCKS_PER_SEC;
    timeTaken *= 1000000;
    printf("%llu nodes searched in %lf microseconds.", total, timeTaken);
    printf(" That's about %lf nodes per second.", ((double) total) / (timeTaken));
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
