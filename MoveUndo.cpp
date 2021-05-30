#include "MoveUndo.h"
#include "Board.h"

MoveUndo::MoveUndo(Board * board) {
    // Set the board to the saved states.
    pos = board->pos;
}

void MoveUndo::undoMove(Board * board) {
    // Set the board to the saved states.
    board->pos = pos;
}