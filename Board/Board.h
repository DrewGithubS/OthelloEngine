#ifndef BOARD_H
#define BOARD_H

#include <cstdint>

const bool BLACK = 1;
const bool WHITE = 0;

const uint64_t ONE64 = 1;

typedef struct {
    uint64_t team[2];
    uint64_t occupied;
    bool lastMoveSkipped;
    bool turn;
    // Not used
    uint8_t pad[6];
} Position;

class MoveList;
class MoveUndo;

uint8_t countBitsSet(uint64_t in);

Position createBoard(char * position = "8/8/8/3WB3/3BW3/8/8/8 1 0");
bool squareIsOccupied(uint8_t square);
// Assumes that a piece is on the square.
bool getPieceAt(Position * pos, uint8_t square);
void print(Position * pos, bool extraInfo = false);
Position readFromString(char * position);
void getAllLegalMovesASM(Position * pos, int8_t ** mlPointer);
void getAllLegalMoves(Position * pos, int8_t ** mlPointer);
void turnStonesFromMove(Position * pos, uint8_t square);
bool doMove(Position * pos, int8_t square);
int8_t getWinner(Position * pos);

#endif