#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint64_t team[2];
    uint64_t occupied;
    bool lastMoveSkipped;
    bool turn;
    // Not used
    uint8_t pad[6];
} Position;


uint8_t countBitsSet(uint64_t in);

// "8/8/8/3WB3/3BW3/8/8/8 1 0"
Position createBoard(char * position);
bool squareIsOccupied(Position * pos, uint8_t square);
// Assumes that a piece is on the square.
bool getPieceAt(Position * pos, uint8_t square);
void print(Position * pos, bool extraInfo);
Position readFromString(char * position);
void getAllLegalMoves(Position * pos, int8_t ** mlPointer);
void turnStonesFromMove(Position * pos, uint8_t square);
bool doMove(Position * pos, int8_t square);
int8_t getWinner(Position * pos);

#endif