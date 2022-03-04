#include <stdio.h>
#include <stdbool.h>

#include "Board.h"

#define shiftR(var) (var | (var >> shift & fastMask))
#define shiftL(var) (var | (var << shift & fastMask))

const bool BLACK = 1;
const bool WHITE = 0;
const uint64_t ONE64 = 1;

inline uint8_t countBitsSet(uint64_t in) {
    return (uint8_t) __builtin_popcountll(in);
}

inline Position createBoard(char * position) {
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

inline bool squareIsOccupied(Position * pos, uint8_t square) {
    return (pos->occupied) >> square & 1;
}

// Assumes that a piece is on the square.
inline bool getPieceAt(Position * pos, uint8_t square) {
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
    char chars[8] = "ABCDEFGH";
    printf("    ");
    for(int i = 0; i < 8; ++i) {
        printf("%c    ", chars[i]);
    }
    printf("\n");
    if(extraInfo) {
        printf("White Pieces: %d\n", (int) countBitsSet(pos->team[WHITE]));
        printf("Black Pieces: %d\n", (int) countBitsSet(pos->team[BLACK]));
        printf("Move: %s", (pos->turn ? "Black\n" : "White\n"));
        printf("Last Move Passed: %d\n", pos->lastMoveSkipped);
    }
}

inline int8_t getWinner(Position * pos) {
    uint8_t blackCount = countBitsSet(pos->team[BLACK]);
    uint8_t whiteCount = countBitsSet(pos->team[WHITE]);
    return blackCount > whiteCount ? -1 : (whiteCount > blackCount ? 1 : 0);
}

__attribute__((always_inline)) uint64_t getAllLegalMovesMask(Position * pos) {
    // Used in masking
    const uint64_t friendlyStones = pos->team[pos->turn];
    const uint64_t enemyStones = pos->team[!pos->turn];
    // Squares that don't have a stone on them.
    const uint64_t emptySquares = ~pos->occupied;
    uint64_t output = 0;
    // A temporary holder for the moves in each direction
    register uint64_t tempMoves asm("x9");
    uint64_t fastMask;

    // Each set is ~26 ASM instructions in x86
    register int8_t shift asm("x10");
    shift = 1;
    register uint64_t MACROMASK asm("x11");
    MACROMASK = 0x7F7F7F7F7F7F7F7F;
    fastMask = MACROMASK & enemyStones;
    tempMoves = (friendlyStones >> shift & fastMask);
    tempMoves = shiftR(shiftR(shiftR(shiftR(shiftR(tempMoves)))));
    output |= (tempMoves >> shift & MACROMASK) & emptySquares;

    MACROMASK = 0xFEFEFEFEFEFEFEFE;
    fastMask = MACROMASK & enemyStones;
    tempMoves = (friendlyStones << shift & fastMask);
    tempMoves = shiftL(shiftL(shiftL(shiftL(shiftL(tempMoves)))));
    output |= (tempMoves << shift & MACROMASK) & emptySquares;

    shift = 9;
    MACROMASK = 0x007F7F7F7F7F7F7F;
    fastMask = MACROMASK & enemyStones;
    tempMoves = (friendlyStones >> shift & fastMask);
    tempMoves = shiftR(shiftR(shiftR(shiftR(shiftR(tempMoves)))));
    output |= (tempMoves >> shift & MACROMASK) & emptySquares;

    MACROMASK = 0xFEFEFEFEFEFEFE00;
    fastMask = MACROMASK & enemyStones;
    tempMoves = (friendlyStones << shift & fastMask);
    tempMoves = shiftL(shiftL(shiftL(shiftL(shiftL(tempMoves)))));
    output |= (tempMoves << shift & MACROMASK) & emptySquares;

    shift = 8;
    MACROMASK = 0xFFFFFFFFFFFFFFFF;
    fastMask = MACROMASK & enemyStones;
    tempMoves = (friendlyStones >> shift & fastMask);
    tempMoves = shiftR(shiftR(shiftR(shiftR(shiftR(tempMoves)))));
    output |= (tempMoves >> shift & MACROMASK) & emptySquares;

    MACROMASK = 0xFFFFFFFFFFFFFFFF;
    fastMask = MACROMASK & enemyStones;
    tempMoves = (friendlyStones << shift & fastMask);
    tempMoves = shiftL(shiftL(shiftL(shiftL(shiftL(tempMoves)))));
    output |= (tempMoves << shift & MACROMASK) & emptySquares;

    shift = 7;
    MACROMASK = 0x00FEFEFEFEFEFEFE;
    fastMask = MACROMASK & enemyStones;
    tempMoves = (friendlyStones >> shift & fastMask);
    tempMoves = shiftR(shiftR(shiftR(shiftR(shiftR(tempMoves)))));
    output |= (tempMoves >> shift & MACROMASK) & emptySquares;

    MACROMASK = 0x7F7F7F7F7F7F7F00;
    fastMask = MACROMASK & enemyStones;
    tempMoves = (friendlyStones << shift & fastMask);
    tempMoves = shiftL(shiftL(shiftL(shiftL(shiftL(tempMoves)))));
    output |= (tempMoves << shift & MACROMASK) & emptySquares;

    return output;
}

// These functions need to be fast.
// This function is 400 instructions. Ignoring the loop, this should take 100 cycles on an M1 mac
// Ignoring branches this should take 1/(3.2 * 10 ^ 7) seconds
__attribute__((always_inline)) void getAllLegalMoves(Position * pos, int8_t ** mlPointer) {

    // Used later in the function
    bool temp;
    int8_t * originalPointer = *mlPointer;

    uint64_t output = getAllLegalMovesMask(pos);

    // Worst case is the amount of squares set
    for(; output; output &= (output-1)) {
        *(*mlPointer)++ = __builtin_ctzl(output);
    }
    // This is a branchless way of setting the first element to -1 if there are no items in the array.
    temp = (*mlPointer == originalPointer);
    *(*mlPointer) = temp ? -1 : *(*mlPointer);
    *(mlPointer) += temp;
}

// This function is 600 instructions. Ignoring the loop, this should take 150 cycles on an M1 mac
// Ignoring branches this should take 1.5/(3.2 * 10 ^ 7) seconds
__attribute__((always_inline)) bool doMove(Position * pos, int8_t square) {
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

    uint64_t fastMask;
    register uint64_t tempOutput asm("x9");

    register int8_t shift asm("x10");
    shift = 1;
    register uint64_t MACROMASK asm("x11");
    MACROMASK = 0x7F7F7F7F7F7F7F7F;
    fastMask = MACROMASK & enemyStones;
    tempOutput = (piecePlaced >> shift & fastMask);
    tempOutput = shiftR(shiftR(shiftR(shiftR(shiftR(tempOutput)))));
    ifCaptured = (tempOutput >> shift & MACROMASK) & friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    MACROMASK = 0xFEFEFEFEFEFEFEFE;
    fastMask = MACROMASK & enemyStones;
    tempOutput = (piecePlaced << shift & fastMask);
    tempOutput = shiftL(shiftL(shiftL(shiftL(shiftL(tempOutput)))));
    ifCaptured = (tempOutput << shift & MACROMASK) & friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    shift = 9;
    MACROMASK = 0x007F7F7F7F7F7F7F;
    fastMask = MACROMASK & enemyStones;
    tempOutput = (piecePlaced >> shift & fastMask);
    tempOutput = shiftR(shiftR(shiftR(shiftR(shiftR(tempOutput)))));
    ifCaptured = (tempOutput >> shift & MACROMASK) & friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    MACROMASK = 0xFEFEFEFEFEFEFE00;
    fastMask = MACROMASK & enemyStones;
    tempOutput = (piecePlaced << shift & fastMask);
    tempOutput = shiftL(shiftL(shiftL(shiftL(shiftL(tempOutput)))));
    ifCaptured = (tempOutput << shift & MACROMASK) & friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    shift = 8;
    MACROMASK = 0xFFFFFFFFFFFFFFFF;
    fastMask = MACROMASK & enemyStones;
    tempOutput = (piecePlaced >> shift & fastMask);
    tempOutput = shiftR(shiftR(shiftR(shiftR(shiftR(tempOutput)))));
    ifCaptured = (tempOutput >> shift & MACROMASK) & friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    MACROMASK = 0xFFFFFFFFFFFFFFFF;
    fastMask = MACROMASK & enemyStones;
    tempOutput = (piecePlaced << shift & fastMask);
    tempOutput = shiftL(shiftL(shiftL(shiftL(shiftL(tempOutput)))));
    ifCaptured = (tempOutput << shift & MACROMASK) & friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    shift = 7;
    MACROMASK = 0x00FEFEFEFEFEFEFE;
    fastMask = MACROMASK & enemyStones;
    tempOutput = (piecePlaced >> shift & fastMask);
    tempOutput = shiftR(shiftR(shiftR(shiftR(shiftR(tempOutput)))));
    ifCaptured = (tempOutput >> shift & MACROMASK) & friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    MACROMASK = 0x7F7F7F7F7F7F7F00;
    fastMask = MACROMASK & enemyStones;
    tempOutput = (piecePlaced << shift & fastMask);
    tempOutput = shiftL(shiftL(shiftL(shiftL(shiftL(tempOutput)))));
    ifCaptured = (tempOutput << shift & MACROMASK) & friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    __builtin_prefetch(&(pos->occupied)); // Adds roughly 2MM Nodes/s
    pos->team[BLACK] ^= output;
    pos->team[WHITE] ^= output;

    pos->occupied |= piecePlaced;

    // The game is not over
    return false;
}