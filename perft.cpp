#include <cstdlib>
#include <iostream>
#include <chrono>
#include <pthread.h>

#include "doPerft.cpp"
#include "MoveUndo.h"

// int main() {
//     Board board;
//     MoveList moves;
//     uint8_t depth = 2;

//     uint64_t total = 0;
//     uint64_t positions;
//     board.print(true);
//     depth--;

//     auto start = std::chrono::high_resolution_clock::now();
//     board.getAllLegalMoves(&moves.last);
//     MoveUndo * undo = new MoveUndo(&board);
//     for(int i = 0; i < moves.length(); i++) {
//         positions = 0;
//         board.doMove(moves.moves[i]);
//         doPerft(&board, depth, &positions);
//         total += positions;
//         // printf("%d: %lu\n", moves.moves[i], positions);
//         undo->undoMove(&board);
//     }
//     auto end = std::chrono::high_resolution_clock::now();
//     uint64_t time = (uint64_t) (std::chrono::duration_cast<std::chrono::microseconds>(end - start)).count();
//     printf("%lu nodes searched in %lu microseconds.", total, time);
//     std::cout << "That's about " << (float(total) / (float(time) / float(1000000))) << " nodes per second." << std::endl;
//     free(undo);
// }


typedef struct {
    Board * board;
    uint8_t depth;
    uint64_t * positions;
} Input;

void * threadDoPerft(void * in) {
    Input * input = (Input *) in;
    doPerft(input->board, input->depth, input->positions);
}

int main() {
    // Multithreaded approach to perft.
    uint8_t THREADS = 4;

    pthread_t * threadList = (pthread_t *) malloc(sizeof(pthread_t) * THREADS);


    Board board;
    MoveList moves;
    uint8_t depth = 13;
    uint64_t total = 0;
    depth--;

    auto start = std::chrono::high_resolution_clock::now();
    board.getAllLegalMoves(&moves.last);

    uint8_t movesLength = moves.length();

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
                inputList[index]->board->doMove(moves.moves[index]);
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


// int main() {
//     // Multithreaded approach to perft.
//     uint8_t THREADS = 12;

//     pthread_t * threadList = (pthread_t *) malloc(sizeof(pthread_t) * THREADS);


//     Board board;
//     MoveList moves;
//     uint8_t depth = 13;
//     uint64_t total = 0;
//     depth--;

//     auto start = std::chrono::high_resolution_clock::now();
//     board.getAllLegalMoves(&moves.last);

//     uint8_t movesLength = moves.length();

//     Input ** inputList = (Input **) malloc(movesLength * sizeof(Input *));
//     uint64_t ** positionCountList = (uint64_t **) calloc(movesLength, sizeof(uint64_t *));
//     for(int i = 0; i < THREADS; i++) {
//         positionCountList[index] = new uint64_t;
//         inputList[index] = new Input();
//         inputList[index]->board = new Board();
//         inputList[index]->depth = depth;
//         inputList[index]->positions = positionCountList[index];
//     }

//     uint8_t index;
//     // For each set of threads
//     uint8_t threadCount = 0;
//     for(int i = 0; i < movesLength; i++) {
//         MoveUndo * undo = new MoveUndo(&board);
//         board.doMove(moves.moves[i]);
//         MoveList moves2;
//         board.getAllLegalMoves(&moves2.last);
//         for(int j = 0; j < moves2.length(); i++) {
//             inputList[threadCount]->board->doMove(moves.moves[i]);
//             inputList[threadCount]->board->doMove(moves.moves[j]);
//             pthread_create(&threadList[j], NULL, &threadDoPerft, (void *) inputList[index]);
//         }
//     }

//     for(int i = 0; i < (movesLength + THREADS)/THREADS; i++) {
//         for(int j = 0; j < THREADS && j < movesLength; j++) {
//             index = i * THREADS + j;
//             if(index < movesLength) {
//                 positionCountList[index] = new uint64_t;
//                 inputList[index] = new Input();
//                 inputList[index]->board = new Board();
//                 inputList[index]->depth = depth;
//                 inputList[index]->positions = positionCountList[index];
//                 inputList[index]->board->doMove(moves.moves[index]);
//                 pthread_create(&threadList[j], NULL, &threadDoPerft, (void *) inputList[index]);
//             }
//         }
//         for(int j = 0; j < THREADS; j++) {
//             void * pv;
//             pthread_join(threadList[j], &pv);
//             total += *positionCountList[j];
//         }
//     }
//     auto end = std::chrono::high_resolution_clock::now();
//     uint64_t time = (uint64_t) (std::chrono::duration_cast<std::chrono::microseconds>(end - start)).count();
//     printf("%lu nodes searched in %lu microseconds.", total, time);
//     std::cout << "That's about " << (float(total) / (float(time) / float(1000000))) << " nodes per second." << std::endl;
// }


// int main() {
//     // Multithreaded approach to perft.
//     uint8_t THREADS = 6;
//     Board board;
//     MoveList moves;
//     uint8_t depth = 3;
//     uint64_t total = 0;
//     depth--;


//     pthread_t * threadList = (pthread_t *) malloc(sizeof(pthread_t) * THREADS);
//     Input ** inputList = (Input **) malloc(THREADS * sizeof(Input *));
//     uint64_t ** positionCountList = (uint64_t **) malloc(THREADS * sizeof(uint64_t *));
//     for(int i = 0; i < 12; i++) {
//         inputList[i] = new Input();
//         inputList[i]->board = new Board();
//         positionCountList[i] = new uint64_t;
//         *positionCountList[i] = 0;
//         inputList[i]->positions = positionCountList[i];
//         inputList[i]->depth = depth;
//     }


//     uint64_t * positions = new uint64_t;
//     uint8_t depthToThreads = 1;

//     *positions = 0;
//     doPerft(&board, depthToThreads, positions);
//     while(*positions < THREADS) {
//         *positions = 0;
//         doPerft(&board, ++depthToThreads, positions);
//     }
//     auto start = std::chrono::high_resolution_clock::now();
    

//     uint8_t movesLength = moves.length();

//     uint64_t positionCount = 0;

//     MoveUndo ** moveUndoList = (MoveUndo **) malloc(depthToThreads * sizeof(MoveUndo *));
//     for(uint8_t i = 0; i < depthToThreads; i++) {
//         moveUndoList[i] = new MoveUndo();
//     }
//     uint8_t * indices = (uint8_t *) calloc(depthToThreads, sizeof(uint8_t));
//     uint8_t currentDepth = 0;
//     uint8_t threadCount = 0;
//     while(positionCount < *positions) {
//         std::cout << (int) positionCount << std::endl;;
//         board.getAllLegalMoves(&moves.last);
//         uint8_t amountOfMoves = moves.length();
//         while(indices[currentDepth] < amountOfMoves && currentDepth < depthToThreads) {
//             std::cout << "DEPTH: " << (int) currentDepth << std::endl;
//             *moveUndoList[currentDepth] = MoveUndo(&board);
//             board.doMove(moves.moves[indices[currentDepth]]);
//             currentDepth++;
//         }
//         if(threadCount >= THREADS) {
//             for(int j = 0; j < THREADS; j++) {
//                 void * pv;
//                 pthread_join(threadList[j], &pv);
//                 total += *positionCountList[j];
//                 *positionCountList[j] = 0;
//                 threadCount = 0;
//             }
//         }
//         inputList[threadCount]->board->pos = board.pos;
//         inputList[threadCount]->board->lastMoveSkipped = board.lastMoveSkipped;
//         inputList[threadCount]->board->turn = board.turn;
//         inputList[threadCount]->board->occupied = board.occupied;
//         pthread_create(&threadList[threadCount++], NULL, &threadDoPerft, (void *) inputList[threadCount]);
//         positionCount++;

//         moveUndoList[currentDepth-1]->undoMove(&board);
//         indices[currentDepth]++;
//         currentDepth--;
//     }
//     for(int j = 0; j < THREADS; j++) {
//         void * pv;
//         pthread_join(threadList[j], &pv);
//         total += *positionCountList[j];
//         *positionCountList[j] = 0;
//         threadCount = 0;
//     }
//     auto end = std::chrono::high_resolution_clock::now();
//     uint64_t time = (uint64_t) (std::chrono::duration_cast<std::chrono::microseconds>(end - start)).count();
//     printf("%lu nodes searched in %lu microseconds.", total, time);
//     std::cout << "That's about " << (float(total) / (float(time) / float(1000000))) << " nodes per second." << std::endl;
// }