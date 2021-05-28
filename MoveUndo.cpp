#include "MoveUndo.h"
#include "Board.h"

#define BLACK 1
#define WHITE 0

MoveUndo::MoveUndo(Board * board) {
    // Set the board to the saved states.
    pos = board->pos;
    turn = board->turn;
    lastMoveSkipped = board->lastMoveSkipped;
}

void MoveUndo::undoMove(Board * board) {
    // Set the board to the saved states.
    board->pos = pos;
    board->turn = turn;
    board->lastMoveSkipped = lastMoveSkipped;
}