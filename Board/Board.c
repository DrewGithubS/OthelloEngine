#include <stdio.h>
#include <stdbool.h>

#include "Board.h"

const bool BLACK = 1;
const bool WHITE = 0;
const uint64_t ONE64 = 1;

uint8_t countBitsSet(uint64_t in) {
    return (uint8_t) __builtin_popcountll(in);
}

Position createBoard(char * position) {
    return readFromString(position);
}

// Reads a position from a string
/* This takes some inspiration from Forsyth–Edwards Notation.
A W means white, a B means black. Any numbers represent how many squares are blank.
A slash means that it is beginning a new line.
The number after the position is read is the turn, 0 is white, 1 is black.
The number after that number is whether the last turn was skipped, 0 is false, 1 is true.

Here is the starting position:

8/8/8/3WB3/3BW3/8/8/8 1 0

---------------------------------
|   |   |   |   |   |   |   |   |
|---+---+---+---+---+---+---+---|
|   |   |   |   |   |   |   |   |
|---+---+---+---+---+---+---+---|
|   |   |   |   |   |   |   |   |
|---+---+---+---+---+---+---+---|
|   |   |   | W | B |   |   |   |
|---+---+---+---+---+---+---+---|
|   |   |   | B | W |   |   |   |
|---+---+---+---+---+---+---+---|
|   |   |   |   |   |   |   |   |
|---+---+---+---+---+---+---+---|
|   |   |   |   |   |   |   |   |
|---+---+---+---+---+---+---+---|
|   |   |   |   |   |   |   |   |
---------------------------------
Black to move and the last turn was not skipped.
*/
// This is only called once, so efficiency doesn't really matter.
Position readFromString(char * posString) {
    Position pos;
    // Resetting to make sure no weird memory stuff happens.
    pos.team[WHITE] = 0;
    pos.team[BLACK] = 0;
    uint8_t charCount = 0;
    uint8_t boardCount = 0;

    // Foreach row
    for(int row = 0; row < 8; ++row) {
        // Until it hits new line
        while(posString[charCount] != '/' && boardCount < 64) {
            // If the current character is a number
            if(posString[charCount] <= '8') {
                // Increment the index by the number
                boardCount += posString[charCount] - '0';
            } else {
                // This just adds the piece to the square if it is the right color.
                pos.team[WHITE] |= ((ONE64 & (posString[charCount] == 'W')) << boardCount);
                pos.team[BLACK] |= ((ONE64 & (posString[charCount] == 'B')) << boardCount);
                ++boardCount;
            }
            ++charCount;
        }
        ++charCount;
    }
    // At this point the charCount is already at the next number
    pos.turn = posString[charCount] == '1';
    // Move charCount to the next number
    charCount += 2;
    pos.lastMoveSkipped = posString[charCount] == '1';

    pos.occupied = pos.team[pos.turn] | pos.team[!pos.turn];

    return pos;
}

bool squareIsOccupied(Position * pos, uint8_t square) {
    return (pos->occupied) >> square & 1;
}

// Assumes that a piece is on the square.
bool getPieceAt(Position * pos, uint8_t square) {
    return (pos->team[BLACK]) >> square & 1;
}

void print(Position * pos, bool extraInfo) {
    printf("\n");
    for(int y = 0; y < 8; ++y) {
        printf("  +––––+––––+––––+––––+––––+––––+––––+––––+\n");
        for(int x = 0; x < 8; ++x) {
            if(x == 0) {
                printf("  ");
            }
            if(squareIsOccupied(pos, y*8+x)) {
                printf("| %s ", (getPieceAt(pos, y*8+x) ? "@@" : "\\/"));
            } else {
                printf("|    ");
            }
        }
        printf("|\n");
        for(int x = 0; x < 8; ++x) {
            if(x == 0) {
                printf(" %d", (8 - y));
            }
            if(squareIsOccupied(pos, y*8+x)) {
                printf("| %s ", (getPieceAt(pos, y*8+x) ? "@@" : "/\\"));
            } else {
                printf("|    ");
            }
        }
        printf("|\n");
    }
    printf("  +––––+––––+––––+––––+––––+––––+––––+––––+\n");
    char chars[8] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'};
    printf("    ");
    for(int i = 0; i < 8; ++i) {
        printf("%c    ", chars[i]);
    }
    printf("\nWhite Pieces: %d\n", (int) countBitsSet(pos->team[WHITE]));
    printf("Black Pieces: %d\n", (int) countBitsSet(pos->team[BLACK]));
    if(extraInfo) {
        printf("Move: %s", (pos->turn ? "Black\n" : "White\n"));
        printf("Last Move Passed: %d", pos->lastMoveSkipped);
    }
}

int8_t getWinner(Position * pos) {
    uint8_t blackCount = countBitsSet(pos->team[BLACK]);
    uint8_t whiteCount = countBitsSet(pos->team[WHITE]);
    return blackCount > whiteCount ? -1 : (whiteCount > blackCount ? 1 : 0);
}

// These functions need to be fast.
void getAllLegalMoves(Position * pos, int8_t ** mlPointer) {

    // Used later in the function
    bool temp;
    int8_t * originalPointer = *mlPointer;

    // Used in masking
    const uint64_t friendlyStones = pos->team[pos->turn];
    const uint64_t enemyStones = pos->team[!pos->turn];
    // Squares that don't have a stone on them.
    const uint64_t emptySquares = ~pos->occupied;
    uint64_t output = 0;
    // A temporary holder forthe moves in each direction
    uint64_t tempMoves;
    uint64_t fastMask;
    const uint8_t shift0 = 1;
    const uint8_t shift1 = 9;
    const uint8_t shift2 = 8;
    const uint8_t shift3 = 7;
    const uint64_t MACROMASKS0 = 0x7F7F7F7F7F7F7F7F;
    const uint64_t MACROMASKS1 = 0xFEFEFEFEFEFEFEFE;
    const uint64_t MACROMASKS2 = 0x007F7F7F7F7F7F7F;
    const uint64_t MACROMASKS3 = 0xFEFEFEFEFEFEFE00; 
    const uint64_t MACROMASKS4 = 0xFFFFFFFFFFFFFFFF;
    const uint64_t MACROMASKS5 = 0xFFFFFFFFFFFFFFFF;
    const uint64_t MACROMASKS6 = 0x00FEFEFEFEFEFEFE;
    const uint64_t MACROMASKS7 = 0x7F7F7F7F7F7F7F00;

    // Each set is 24 ASM instructions in x86
    fastMask = MACROMASKS0 & enemyStones;
    tempMoves = (friendlyStones >> shift0 & fastMask);
    tempMoves |= (tempMoves >> shift0 & fastMask);
    tempMoves |= (tempMoves >> shift0 & fastMask);
    tempMoves |= (tempMoves >> shift0 & fastMask);
    tempMoves |= (tempMoves >> shift0 & fastMask);
    tempMoves |= (tempMoves >> shift0 & fastMask);
    output |= (tempMoves >> shift0 & MACROMASKS0) & emptySquares;

    fastMask = MACROMASKS1 & enemyStones;
    tempMoves = (friendlyStones << shift0 & fastMask);
    tempMoves |= (tempMoves << shift0 & fastMask);
    tempMoves |= (tempMoves << shift0 & fastMask);
    tempMoves |= (tempMoves << shift0 & fastMask);
    tempMoves |= (tempMoves << shift0 & fastMask);
    tempMoves |= (tempMoves << shift0 & fastMask);
    output |= (tempMoves << shift0 & MACROMASKS1) & emptySquares;

    fastMask = MACROMASKS2 & enemyStones;
    tempMoves = (friendlyStones >> shift1 & fastMask);
    tempMoves |= (tempMoves >> shift1 & fastMask);
    tempMoves |= (tempMoves >> shift1 & fastMask);
    tempMoves |= (tempMoves >> shift1 & fastMask);
    tempMoves |= (tempMoves >> shift1 & fastMask);
    tempMoves |= (tempMoves >> shift1 & fastMask);
    output |= (tempMoves >> shift1 & MACROMASKS2) & emptySquares;

    fastMask = MACROMASKS3 & enemyStones;
    tempMoves = (friendlyStones << shift1 & fastMask);
    tempMoves |= (tempMoves << shift1 & fastMask);
    tempMoves |= (tempMoves << shift1 & fastMask);
    tempMoves |= (tempMoves << shift1 & fastMask);
    tempMoves |= (tempMoves << shift1 & fastMask);
    tempMoves |= (tempMoves << shift1 & fastMask);
    output |= (tempMoves << shift1 & MACROMASKS3) & emptySquares;

    fastMask = MACROMASKS4 & enemyStones;
    tempMoves = (friendlyStones >> shift2 & fastMask);
    tempMoves |= (tempMoves >> shift2 & fastMask);
    tempMoves |= (tempMoves >> shift2 & fastMask);
    tempMoves |= (tempMoves >> shift2 & fastMask);
    tempMoves |= (tempMoves >> shift2 & fastMask);
    tempMoves |= (tempMoves >> shift2 & fastMask);
    output |= (tempMoves >> shift2 & MACROMASKS4) & emptySquares;

    fastMask = MACROMASKS5 & enemyStones;
    tempMoves = (friendlyStones << shift2 & fastMask);
    tempMoves |= (tempMoves << shift2 & fastMask);
    tempMoves |= (tempMoves << shift2 & fastMask);
    tempMoves |= (tempMoves << shift2 & fastMask);
    tempMoves |= (tempMoves << shift2 & fastMask);
    tempMoves |= (tempMoves << shift2 & fastMask);
    output |= (tempMoves << shift2 & MACROMASKS5) & emptySquares;

    fastMask = MACROMASKS6 & enemyStones;
    tempMoves = (friendlyStones >> shift3 & fastMask);
    tempMoves |= (tempMoves >> shift3 & fastMask);
    tempMoves |= (tempMoves >> shift3 & fastMask);
    tempMoves |= (tempMoves >> shift3 & fastMask);
    tempMoves |= (tempMoves >> shift3 & fastMask);
    tempMoves |= (tempMoves >> shift3 & fastMask);
    output |= (tempMoves >> shift3 & MACROMASKS6) & emptySquares;

    fastMask = MACROMASKS7 & enemyStones;
    tempMoves = (friendlyStones << shift3 & fastMask);
    tempMoves |= (tempMoves << shift3 & fastMask);
    tempMoves |= (tempMoves << shift3 & fastMask);
    tempMoves |= (tempMoves << shift3 & fastMask);
    tempMoves |= (tempMoves << shift3 & fastMask);
    tempMoves |= (tempMoves << shift3 & fastMask);
    output |= (tempMoves << shift3 & MACROMASKS7) & emptySquares;

    // These builtin_expects add about 1 million nodes/s
    // Worst case is the amount of squares set
    while(__builtin_expect(output, 1)) {
        *(*mlPointer)++ = __builtin_ctzl(output);
        // Clear the least significant set bit.
        output &= (output-1);
    }
    // This is a branchless way of setting the first element to -1 if there are no items in the array.
    temp = (*mlPointer == originalPointer);
    *(*mlPointer) = (temp) ? -1 : *(*mlPointer);
    *(mlPointer) += temp;
}

bool doMove(Position * pos, int8_t square) {
    // Toggle the turn
    pos->turn = !pos->turn;

    // If the player is passing
    if(__builtin_expect(square == -1, 0)) {
        // Return true if the variable was already true, but also toggle the varible.
        return !(pos->lastMoveSkipped = !pos->lastMoveSkipped);
    }
    // This move was not passed.
    pos->lastMoveSkipped = false;

    // Keeping in mind that the turn has already been toggled.
    uint64_t ifCaptured;
    uint64_t output = 0;
    const uint64_t friendlyStones = pos->team[!pos->turn];
    const uint64_t enemyStones = pos->team[pos->turn];
    // The order of the variables is important.
    const uint64_t piecePlaced = ONE64 << square;

    // Putting these two lines up here instead of at the bottom adds 4 million Nodes/s
    pos->team[BLACK] |= ((ONE64 & !pos->turn) << square);
    pos->team[WHITE] |= ((ONE64 & pos->turn) << square);

    const uint64_t MACROMASKS0 = 0x7F7F7F7F7F7F7F7F;
    const uint64_t MACROMASKS1 = 0xFEFEFEFEFEFEFEFE;
    const uint64_t MACROMASKS2 = 0x007F7F7F7F7F7F7F;
    const uint64_t MACROMASKS3 = 0xFEFEFEFEFEFEFE00; 
    const uint64_t MACROMASKS4 = 0xFFFFFFFFFFFFFFFF;
    const uint64_t MACROMASKS5 = 0xFFFFFFFFFFFFFFFF;
    const uint64_t MACROMASKS6 = 0x00FEFEFEFEFEFEFE;
    const uint64_t MACROMASKS7 = 0x7F7F7F7F7F7F7F00;

    uint64_t fastMask;
    const uint8_t shift0 = 1;
    const uint8_t shift1 = 9;
    const uint8_t shift2 = 8;
    const uint8_t shift3 = 7;
    uint64_t tempOutput;
    
    fastMask = MACROMASKS0 & enemyStones;
    tempOutput = (piecePlaced >> shift0 & fastMask);
    tempOutput |= (tempOutput >> shift0 & fastMask);
    tempOutput |= (tempOutput >> shift0 & fastMask);
    tempOutput |= (tempOutput >> shift0 & fastMask);
    tempOutput |= (tempOutput >> shift0 & fastMask);
    tempOutput |= (tempOutput >> shift0 & fastMask);
    ifCaptured = (tempOutput >> shift0 & MACROMASKS0) & friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    fastMask = MACROMASKS1 & enemyStones;
    tempOutput = (piecePlaced << shift0 & fastMask);
    tempOutput |= (tempOutput << shift0 & fastMask);
    tempOutput |= (tempOutput << shift0 & fastMask);
    tempOutput |= (tempOutput << shift0 & fastMask);
    tempOutput |= (tempOutput << shift0 & fastMask);
    tempOutput |= (tempOutput << shift0 & fastMask);
    ifCaptured = (tempOutput << shift0 & MACROMASKS1) & friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    fastMask = MACROMASKS2 & enemyStones;
    tempOutput = (piecePlaced >> shift1 & fastMask);
    tempOutput |= (tempOutput >> shift1 & fastMask);
    tempOutput |= (tempOutput >> shift1 & fastMask);
    tempOutput |= (tempOutput >> shift1 & fastMask);
    tempOutput |= (tempOutput >> shift1 & fastMask);
    tempOutput |= (tempOutput >> shift1 & fastMask);
    ifCaptured = (tempOutput >> shift1 & MACROMASKS2) & friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    fastMask = MACROMASKS3 & enemyStones;
    tempOutput = (piecePlaced << shift1 & fastMask);
    tempOutput |= (tempOutput << shift1 & fastMask);
    tempOutput |= (tempOutput << shift1 & fastMask);
    tempOutput |= (tempOutput << shift1 & fastMask);
    tempOutput |= (tempOutput << shift1 & fastMask);
    tempOutput |= (tempOutput << shift1 & fastMask);
    ifCaptured = (tempOutput << shift1 & MACROMASKS3) & friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    fastMask = MACROMASKS4 & enemyStones;
    tempOutput = (piecePlaced >> shift2 & fastMask);
    tempOutput |= (tempOutput >> shift2 & fastMask);
    tempOutput |= (tempOutput >> shift2 & fastMask);
    tempOutput |= (tempOutput >> shift2 & fastMask);
    tempOutput |= (tempOutput >> shift2 & fastMask);
    tempOutput |= (tempOutput >> shift2 & fastMask);
    ifCaptured = (tempOutput >> shift2 & MACROMASKS4) & friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    fastMask = MACROMASKS5 & enemyStones;
    tempOutput = (piecePlaced << shift2 & fastMask);
    tempOutput |= (tempOutput << shift2 & fastMask);
    tempOutput |= (tempOutput << shift2 & fastMask);
    tempOutput |= (tempOutput << shift2 & fastMask);
    tempOutput |= (tempOutput << shift2 & fastMask);
    tempOutput |= (tempOutput << shift2 & fastMask);
    ifCaptured = (tempOutput << shift2 & MACROMASKS5) & friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    fastMask = MACROMASKS6 & enemyStones;
    tempOutput = (piecePlaced >> shift3 & fastMask);
    tempOutput |= (tempOutput >> shift3 & fastMask);
    tempOutput |= (tempOutput >> shift3 & fastMask);
    tempOutput |= (tempOutput >> shift3 & fastMask);
    tempOutput |= (tempOutput >> shift3 & fastMask);
    tempOutput |= (tempOutput >> shift3 & fastMask);
    ifCaptured = (tempOutput >> shift3 & MACROMASKS6) & friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    fastMask = MACROMASKS7 & enemyStones;
    tempOutput = (piecePlaced << shift3 & fastMask);
    tempOutput |= (tempOutput << shift3 & fastMask);
    tempOutput |= (tempOutput << shift3 & fastMask);
    tempOutput |= (tempOutput << shift3 & fastMask);
    tempOutput |= (tempOutput << shift3 & fastMask);
    tempOutput |= (tempOutput << shift3 & fastMask);
    ifCaptured = (tempOutput << shift3 & MACROMASKS7) & friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    __builtin_prefetch(&(pos->occupied));
    pos->team[BLACK] ^= output;
    pos->team[WHITE] ^= output;

    pos->occupied |= piecePlaced;

    // The game is not over
    return false;
}