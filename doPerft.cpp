#include <cstdlib>

#include "Board.cpp"
#include "MoveList.cpp"
#include "MoveUndo.cpp"

void doPerft(Board * board, uint8_t depth, uint64_t * output) {
    MoveList moves;

    board->getAllLegalMoves(&moves.last);
    if(depth == 1) {
        (*output) += moves.length();
        return;
    }
    MoveUndo * undo = new MoveUndo(board);
    for(unsigned int i = 0; i < moves.length(); i++) {
        if(board->doMove(moves.moves[i])) {
            undo->undoMove(board);
            (*output)++;
            return;
        }
        doPerft(board, depth-1, output);
        undo->undoMove(board);
    }
    free(undo);
}