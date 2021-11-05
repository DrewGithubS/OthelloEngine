#include <iostream>

#include "Board.h"


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
    std::cout << "\n";
    for(int y = 0; y < 8; ++y) {
        std::cout << "  +––––+––––+––––+––––+––––+––––+––––+––––+\n";
        for(int x = 0; x < 8; ++x) {
            if(x == 0) {
                std::cout << "  ";
            }
            std::cout << "| ";
            if(squareIsOccupied(pos, y*8+x)) {
                std::cout << (getPieceAt(pos, y*8+x) ? "@@" : "\\/") << " ";
            } else {
                std::cout << "   ";
            }
        }
        std::cout << "|\n";
        for(int x = 0; x < 8; ++x) {
            if(x == 0) {
                std::cout << (8 - y) << " ";
            }
            std::cout << "| ";
            if(squareIsOccupied(pos, y*8+x)) {
                std::cout << (getPieceAt(pos, y*8+x) ? "@@" : "/\\") << " ";
            } else {
                std::cout << "   ";
            }
        }
        std::cout << "|\n";
    }
    std::cout << "  +––––+––––+––––+––––+––––+––––+––––+––––+\n";
    std::string chars[] = {"A", "B", "C", "D", "E", "F", "G", "H"};
    std::cout << "    ";
    for(int i = 0; i < 8; ++i) {
        std::cout << chars[i] << "    ";
    }
    std::cout << "\nWhite Pieces: " << (int) countBitsSet(pos->team[WHITE]) << std::endl;
    std::cout << "Black Pieces: " << (int) countBitsSet(pos->team[BLACK]) << std::endl;
    if(extraInfo) {
        std::cout << "Move: " << (pos->turn ? "Black\n" : "White\n");
        std::cout << "Last Move Passed: " << pos->lastMoveSkipped << std::endl;
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
    register uint64_t tempMoves;
    register uint64_t fastMask;
    register const uint8_t shifts[4] = {1,9, 8, 7};
    register const uint64_t MACROMASKS[8] = {0x7F7F7F7F7F7F7F7F, 0xFEFEFEFEFEFEFEFE, 
                                             0x007F7F7F7F7F7F7F, 0xFEFEFEFEFEFEFE00, 
                                             0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF,
                                             0x00FEFEFEFEFEFEFE, 0x7F7F7F7F7F7F7F00};

    // Each set is 24 ASM instructions in x86
    fastMask = MACROMASKS[0] & enemyStones;
    tempMoves = (friendlyStones >> shifts[0] & fastMask);
    tempMoves |= (tempMoves >> shifts[0] & fastMask);
    tempMoves |= (tempMoves >> shifts[0] & fastMask);
    tempMoves |= (tempMoves >> shifts[0] & fastMask);
    tempMoves |= (tempMoves >> shifts[0] & fastMask);
    tempMoves |= (tempMoves >> shifts[0] & fastMask);
    output |= (tempMoves >> shifts[0] & MACROMASKS[0]) & emptySquares;

    fastMask = MACROMASKS[1] & enemyStones;
    tempMoves = (friendlyStones << shifts[0] & fastMask);
    tempMoves |= (tempMoves << shifts[0] & fastMask);
    tempMoves |= (tempMoves << shifts[0] & fastMask);
    tempMoves |= (tempMoves << shifts[0] & fastMask);
    tempMoves |= (tempMoves << shifts[0] & fastMask);
    tempMoves |= (tempMoves << shifts[0] & fastMask);
    output |= (tempMoves << shifts[0] & MACROMASKS[1]) & emptySquares;

    fastMask = MACROMASKS[2] & enemyStones;
    tempMoves = (friendlyStones >> shifts[1] & fastMask);
    tempMoves |= (tempMoves >> shifts[1] & fastMask);
    tempMoves |= (tempMoves >> shifts[1] & fastMask);
    tempMoves |= (tempMoves >> shifts[1] & fastMask);
    tempMoves |= (tempMoves >> shifts[1] & fastMask);
    tempMoves |= (tempMoves >> shifts[1] & fastMask);
    output |= (tempMoves >> shifts[1] & MACROMASKS[2]) & emptySquares;

    fastMask = MACROMASKS[3] & enemyStones;
    tempMoves = (friendlyStones << shifts[1] & fastMask);
    tempMoves |= (tempMoves << shifts[1] & fastMask);
    tempMoves |= (tempMoves << shifts[1] & fastMask);
    tempMoves |= (tempMoves << shifts[1] & fastMask);
    tempMoves |= (tempMoves << shifts[1] & fastMask);
    tempMoves |= (tempMoves << shifts[1] & fastMask);
    output |= (tempMoves << shifts[1] & MACROMASKS[3]) & emptySquares;

    fastMask = MACROMASKS[4] & enemyStones;
    tempMoves = (friendlyStones >> shifts[2] & fastMask);
    tempMoves |= (tempMoves >> shifts[2] & fastMask);
    tempMoves |= (tempMoves >> shifts[2] & fastMask);
    tempMoves |= (tempMoves >> shifts[2] & fastMask);
    tempMoves |= (tempMoves >> shifts[2] & fastMask);
    tempMoves |= (tempMoves >> shifts[2] & fastMask);
    output |= (tempMoves >> shifts[2] & MACROMASKS[4]) & emptySquares;

    fastMask = MACROMASKS[5] & enemyStones;
    tempMoves = (friendlyStones << shifts[2] & fastMask);
    tempMoves |= (tempMoves << shifts[2] & fastMask);
    tempMoves |= (tempMoves << shifts[2] & fastMask);
    tempMoves |= (tempMoves << shifts[2] & fastMask);
    tempMoves |= (tempMoves << shifts[2] & fastMask);
    tempMoves |= (tempMoves << shifts[2] & fastMask);
    output |= (tempMoves << shifts[2] & MACROMASKS[5]) & emptySquares;

    fastMask = MACROMASKS[6] & enemyStones;
    tempMoves = (friendlyStones >> shifts[3] & fastMask);
    tempMoves |= (tempMoves >> shifts[3] & fastMask);
    tempMoves |= (tempMoves >> shifts[3] & fastMask);
    tempMoves |= (tempMoves >> shifts[3] & fastMask);
    tempMoves |= (tempMoves >> shifts[3] & fastMask);
    tempMoves |= (tempMoves >> shifts[3] & fastMask);
    output |= (tempMoves >> shifts[3] & MACROMASKS[6]) & emptySquares;

    fastMask = MACROMASKS[7] & enemyStones;
    tempMoves = (friendlyStones << shifts[3] & fastMask);
    tempMoves |= (tempMoves << shifts[3] & fastMask);
    tempMoves |= (tempMoves << shifts[3] & fastMask);
    tempMoves |= (tempMoves << shifts[3] & fastMask);
    tempMoves |= (tempMoves << shifts[3] & fastMask);
    tempMoves |= (tempMoves << shifts[3] & fastMask);
    output |= (tempMoves << shifts[3] & MACROMASKS[7]) & emptySquares;

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
    if(square == -1) {
        // Return true if the variable was already true, but also toggle the varible.
        return !(pos->lastMoveSkipped = !pos->lastMoveSkipped);
    }

    // Keeping in mind that the turn has already been toggled.
    const uint64_t friendlyStones = pos->team[!pos->turn];
    const uint64_t enemyStones = pos->team[pos->turn];
    const uint64_t piecePlaced = ONE64 << square;
    uint64_t ifCaptured;
    uint64_t output = 0;
    register uint64_t tempOutput;
    register uint64_t fastMask;
    register const uint8_t shifts[4] = {1,9, 8, 7};
    register const uint64_t MACROMASKS[8] = {0x7F7F7F7F7F7F7F7F, 0xFEFEFEFEFEFEFEFE, 
                                             0x007F7F7F7F7F7F7F, 0xFEFEFEFEFEFEFE00, 
                                             0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF,
                                             0x00FEFEFEFEFEFEFE, 0x7F7F7F7F7F7F7F00};
    
    fastMask = MACROMASKS[0] & enemyStones;
    tempOutput = (piecePlaced >> shifts[0] & fastMask);
    tempOutput |= (tempOutput >> shifts[0] & fastMask);
    tempOutput |= (tempOutput >> shifts[0] & fastMask);
    tempOutput |= (tempOutput >> shifts[0] & fastMask);
    tempOutput |= (tempOutput >> shifts[0] & fastMask);
    tempOutput |= (tempOutput >> shifts[0] & fastMask);
    ifCaptured = (tempOutput >> shifts[0] & MACROMASKS[0]) & friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    fastMask = MACROMASKS[1] & enemyStones;
    tempOutput = (piecePlaced << shifts[0] & fastMask);
    tempOutput |= (tempOutput << shifts[0] & fastMask);
    tempOutput |= (tempOutput << shifts[0] & fastMask);
    tempOutput |= (tempOutput << shifts[0] & fastMask);
    tempOutput |= (tempOutput << shifts[0] & fastMask);
    tempOutput |= (tempOutput << shifts[0] & fastMask);
    ifCaptured = (tempOutput << shifts[0] & MACROMASKS[1]) & friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    fastMask = MACROMASKS[2] & enemyStones;
    tempOutput = (piecePlaced >> shifts[1] & fastMask);
    tempOutput |= (tempOutput >> shifts[1] & fastMask);
    tempOutput |= (tempOutput >> shifts[1] & fastMask);
    tempOutput |= (tempOutput >> shifts[1] & fastMask);
    tempOutput |= (tempOutput >> shifts[1] & fastMask);
    tempOutput |= (tempOutput >> shifts[1] & fastMask);
    ifCaptured = (tempOutput >> shifts[1] & MACROMASKS[2]) & friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    fastMask = MACROMASKS[3] & enemyStones;
    tempOutput = (piecePlaced << shifts[1] & fastMask);
    tempOutput |= (tempOutput << shifts[1] & fastMask);
    tempOutput |= (tempOutput << shifts[1] & fastMask);
    tempOutput |= (tempOutput << shifts[1] & fastMask);
    tempOutput |= (tempOutput << shifts[1] & fastMask);
    tempOutput |= (tempOutput << shifts[1] & fastMask);
    ifCaptured = (tempOutput << shifts[1] & MACROMASKS[3]) & friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    fastMask = MACROMASKS[4] & enemyStones;
    tempOutput = (piecePlaced >> shifts[2] & fastMask);
    tempOutput |= (tempOutput >> shifts[2] & fastMask);
    tempOutput |= (tempOutput >> shifts[2] & fastMask);
    tempOutput |= (tempOutput >> shifts[2] & fastMask);
    tempOutput |= (tempOutput >> shifts[2] & fastMask);
    tempOutput |= (tempOutput >> shifts[2] & fastMask);
    ifCaptured = (tempOutput >> shifts[2] & MACROMASKS[4]) & friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    fastMask = MACROMASKS[5] & enemyStones;
    tempOutput = (piecePlaced << shifts[2] & fastMask);
    tempOutput |= (tempOutput << shifts[2] & fastMask);
    tempOutput |= (tempOutput << shifts[2] & fastMask);
    tempOutput |= (tempOutput << shifts[2] & fastMask);
    tempOutput |= (tempOutput << shifts[2] & fastMask);
    tempOutput |= (tempOutput << shifts[2] & fastMask);
    ifCaptured = (tempOutput << shifts[2] & MACROMASKS[5]) & friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    fastMask = MACROMASKS[6] & enemyStones;
    tempOutput = (piecePlaced >> shifts[3] & fastMask);
    tempOutput |= (tempOutput >> shifts[3] & fastMask);
    tempOutput |= (tempOutput >> shifts[3] & fastMask);
    tempOutput |= (tempOutput >> shifts[3] & fastMask);
    tempOutput |= (tempOutput >> shifts[3] & fastMask);
    tempOutput |= (tempOutput >> shifts[3] & fastMask);
    ifCaptured = (tempOutput >> shifts[3] & MACROMASKS[6]) & friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    fastMask = MACROMASKS[7] & enemyStones;
    tempOutput = (piecePlaced << shifts[3] & fastMask);
    tempOutput |= (tempOutput << shifts[3] & fastMask);
    tempOutput |= (tempOutput << shifts[3] & fastMask);
    tempOutput |= (tempOutput << shifts[3] & fastMask);
    tempOutput |= (tempOutput << shifts[3] & fastMask);
    tempOutput |= (tempOutput << shifts[3] & fastMask);
    ifCaptured = (tempOutput << shifts[3] & MACROMASKS[7]) & friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    pos->team[BLACK] ^= output;
    pos->team[WHITE] ^= output;

    pos->team[BLACK] |= ((ONE64 & !pos->turn) << square);
    pos->team[WHITE] |= ((ONE64 & pos->turn) << square);

    pos->occupied = pos->team[pos->turn] | pos->team[!pos->turn];

    // This move was not passed.
    pos->lastMoveSkipped = false;

    // The game is not over
    return false;
}