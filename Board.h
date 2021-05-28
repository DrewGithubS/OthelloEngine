#ifndef BOARD_H
#define BOARD_H

#include <cstdint>

#define BLACK 1
#define WHITE 0

static const uint64_t ONE64 = 1;

typedef struct {
    uint64_t team[2];
} Position;

class MoveList;
class MoveUndo;

class Board {
public:
    Position pos;
    bool lastMoveSkipped;
    bool turn;
    uint64_t occupied;

    Board();
    Board(char * position);
    bool squareIsOccupied(uint8_t square);
    // Assumes that a piece is on the square.
    bool getPieceAt(uint8_t square);
    void print(bool extraInfo = false);
    void readFromString(char * position);
    void getAllLegalMoves(uint8_t ** mlPointer);
    void turnStonesFromMove(uint8_t square);
    bool doMove(int8_t square);
    int8_t getWinner();
};

#endif