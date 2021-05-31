#ifndef BOARD_H
#define BOARD_H

#include <cstdint>


const uint8_t shift1 = 1;
const uint8_t shift2 = 9;
const uint8_t shift3 = 8;
const uint8_t shift4 = 7;

const uint64_t MACROmask1 = 0x7F7F7F7F7F7F7F7F;
const uint64_t MACROmask2 = 0xFEFEFEFEFEFEFEFE;
const uint64_t MACROmask3 = 0x007F7F7F7F7F7F7F;
const uint64_t MACROmask4 = 0xFEFEFEFEFEFEFE00;
const uint64_t MACROmask5 = 0xFFFFFFFFFFFFFFFF;
const uint64_t MACROmask6 = 0xFFFFFFFFFFFFFFFF;
const uint64_t MACROmask7 = 0x00FEFEFEFEFEFEFE;
const uint64_t MACROmask8 = 0x7F7F7F7F7F7F7F00;

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

uint8_t countBitsSet(uint64_t in);

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
    void getAllLegalMoves(int8_t ** mlPointer);
    void turnStonesFromMove(uint8_t square);
    bool doMove(int8_t square);
    int8_t getWinner();
};

#endif