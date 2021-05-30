#ifndef BOARD_H
#define BOARD_H

#include <cstdint>

const bool BLACK = 1;
const bool WHITE = 0;

static const uint64_t ONE64 = 1;

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

class Board {
public:
    Position pos;

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