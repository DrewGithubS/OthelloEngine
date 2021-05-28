#ifndef MOVEUNDO_H
#define MOVEUNDO_H

#include "Board.h"

class MoveUndo {
public:
    uint8_t square;
    bool skipping;

    // This holds all the past Board information so it can easily undo moves.
    Position pos;
    bool turn;
    bool lastMoveSkipped;

    MoveUndo(){};
    MoveUndo(Board * board);
    // Returns true if the game is over.
    void undoMove(Board * board);
};

#endif