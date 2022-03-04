#include <stdio.h>
#include "Board.h"

#define INT_MAX  2147483647
#define INT_MIN -2147483648

const uint8_t MAXPOSSIBLEMOVES = 32;

int8_t heuristic(Position * pos) {
	return countBitsSet(pos->team[0]) - countBitsSet(pos->team[1]);
}

int8_t getPlayerMove(Position * pos) {
	int8_t move;
	uint64_t legalMoves = getAllLegalMovesMask(pos);
	char col;
	int row;
	if(!legalMoves) {
		return -1;
	}
	do {
		printf("Enter a move: ");
		scanf(" %c%d", &col, &row);
		row -= 1;
		row = 7 - row;
		col = col >= 'a' ? col - 32 : col;
		col -= 'A';
		move = row * 8 + col;
		if(!(legalMoves >> move & 1)) {
			printf("Illegal move, please try again.\n");
		} else {
			printf("Played move.");
			return move;
		}
	} while(1);
}

int min(int in, int in2) {
	return in > in2 ? in2 : in;
}

int max(int in, int in2) {
	return in < in2 ? in2 : in;
}

int alphaBeta(Position * pos, int depth, int alpha, int beta, bool maximizingPlayer) {
    if(depth == 0) {
        return heuristic(pos);
    }

    int8_t moveList[MAXPOSSIBLEMOVES];
    int8_t * last = moveList;

    getAllLegalMoves(pos, &last);

    int value;
    Position undoMove = *pos;
    if(maximizingPlayer) {
        value = INT_MIN;
        for(int i = 0; __builtin_expect(i < (last - moveList), 1); ++i) {
            if(doMove(pos, moveList[i])) {
	            *pos = undoMove;
	            value = getWinner(pos) == maximizingPlayer ? INT_MAX : INT_MIN;
	            return value;
	        }
            value = max(value, alphaBeta(pos, depth - 1, alpha, beta, !maximizingPlayer));
            *pos = undoMove;
            if (value >= beta) {
                break; // beta cutoff
            }
            alpha = max(alpha, value);
        }
        return value;
    } else {
        value = INT_MAX;
        for(int i = 0; __builtin_expect(i < (last - moveList), 1); ++i) {
        	if(doMove(pos, moveList[i])) {
	            *pos = undoMove;
	            value = getWinner(pos) == maximizingPlayer ? INT_MAX : INT_MIN;
	            return value;
	        }
            value = min(value, alphaBeta(pos, depth - 1, alpha, beta, !maximizingPlayer));
            *pos = undoMove;
            if (value <= alpha) {
                break; // alpha cutoff
            }
            beta = min(beta, value);
    	}
        return value;
    }
}

int8_t getComputerMove(Position * pos, int depth, bool maximizingPlayer) {
	int8_t moveList[MAXPOSSIBLEMOVES];
    int8_t * last = moveList;

    getAllLegalMoves(pos, &last);
    int8_t bestMove = moveList[0];

    int value = INT_MIN;
    int tempVal;
    Position undoMove = *pos;
    for(int i = 0; __builtin_expect(i < (last - moveList), 1); ++i) {
        // This just readies the CPU to access pos. Adds ~4m Nodes/s
        __builtin_prefetch(pos);
        if(doMove(pos, moveList[i])) {
            *pos = undoMove;
            if(getWinner(pos) == maximizingPlayer) {
            	return moveList[i];
            }

        }
        tempVal = alphaBeta(pos, depth, INT_MIN, INT_MAX, !maximizingPlayer);
      	if(tempVal > value) {
      		value = tempVal;
      		bestMove = moveList[i];
      	}
        *pos = undoMove;
    }
    return bestMove;
}

int main() {
	Position pos = createBoard("8/8/8/3WB3/3BW3/8/8/8 1 0");
	Position * posPtr = &pos;
	int8_t move;
	bool playerMove = true;
	do {
		print(&pos, false);
		if(pos.turn & playerMove) {
			move = getPlayerMove(posPtr);
		} else {
			move = getComputerMove(posPtr, 12, pos.turn);
		}
	} while(!doMove(posPtr, move));

	printf("%s wins!\n", getWinner(posPtr) == 1 ? "\\/" : "@@");
	printf("\\/ had %d pieces.\n@@ had %d pieces.\n", countBitsSet(pos.team[1]), countBitsSet(pos.team[0]));
}