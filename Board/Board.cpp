#include <iostream>

#include "Board.h"


uint8_t countBitsSet(uint64_t in) {
    return (uint8_t) __builtin_popcountll(in);
}

Board::Board() {
    readFromString(const_cast<char*>("8/8/8/3WB3/3BW3/8/8/8 1 0"));
}
Board::Board(char * position) {
    readFromString(position);
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
void Board::readFromString(char * posString) {
    // Resetting to make sure no weird memory stuff happens.
    pos.team[WHITE] = 0;
    pos.team[BLACK] = 0;
    uint8_t charCount = 0;
    uint8_t boardCount = 0;

    // For each row
    for(int row = 0; row < 8; row++) {
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
                boardCount++;
            }
            charCount++;
        }
        charCount++;
    }
    // At this point the charCount is already at the next number
    pos.turn = posString[charCount] == '1';
    // Move charCount to the next number
    charCount += 2;
    pos.lastMoveSkipped = posString[charCount] == '1';

    pos.occupied = pos.team[pos.turn] | pos.team[!pos.turn];
}

bool Board::squareIsOccupied(uint8_t square) {
    return (pos.occupied) >> square & 1;
}

// Assumes that a piece is on the square.
bool Board::getPieceAt(uint8_t square) {
    return (pos.team[BLACK]) >> square & 1;
}

void Board::print(bool extraInfo) {
    std::cout << "\n";
    for(int y = 0; y < 8; y++) {
        std::cout << "  +----+----+----+----+----+----+----+----+\n";
        for(int x = 0; x < 8; x++) {
            if(x == 0) {
                std::cout << "  ";
            }
            std::cout << "| ";
            if(squareIsOccupied(y*8+x)) {
                std::cout << (getPieceAt(y*8+x) ? "@@" : "\\/") << " ";
            } else {
                std::cout << "   ";
            }
        }
        std::cout << "|\n";
        for(int x = 0; x < 8; x++) {
            if(x == 0) {
                std::cout << (8 - y) << " ";
            }
            std::cout << "| ";
            if(squareIsOccupied(y*8+x)) {
                std::cout << (getPieceAt(y*8+x) ? "@@" : "/\\") << " ";
            } else {
                std::cout << "   ";
            }
        }
        std::cout << "|\n";
    }
    std::cout << "  +----+----+----+----+----+----+----+----+\n";
    std::string chars[] = {"A", "B", "C", "D", "E", "F", "G", "H"};
    std::cout << "    ";
    for(int i = 0; i < 8; i++) {
        std::cout << chars[i] << "    ";
    }
    std::cout << "\nWhite Pieces: " << (int) countBitsSet(pos.team[WHITE]) << std::endl;
    std::cout << "Black Pieces: " << (int) countBitsSet(pos.team[BLACK]) << std::endl;
    if(extraInfo) {
        std::cout << "Move: " << (pos.turn ? "Black\n" : "White\n");
        std::cout << "Last Move Passed: " << pos.lastMoveSkipped << std::endl;
    }
}

int8_t Board::getWinner() {
    uint8_t blackCount = countBitsSet(pos.team[BLACK]);
    uint8_t whiteCount = countBitsSet(pos.team[WHITE]);
    return blackCount > whiteCount ? -1 : (whiteCount > blackCount ? 1 : 0);
}


// These functions need to be fast.

void Board::getAllLegalMoves(int8_t ** mlPointer) {
    // Squares that don't have a stone on them.
    uint64_t emptySquares = ~pos.occupied;
    uint64_t output = 0;

    uint64_t friendlyStones = pos.team[pos.turn];
    uint64_t enemyStones = pos.team[!pos.turn];
    // A temporary holder for the moves in each direction
    register uint64_t tempMoves;
    register uint64_t fastMask;

    // Each set is 24 ASM instructions
    fastMask = MACROmask1 & enemyStones;
    tempMoves = (friendlyStones >> shift1 & fastMask);
    tempMoves |= (tempMoves >> shift1 & fastMask);
    tempMoves |= (tempMoves >> shift1 & fastMask);
    tempMoves |= (tempMoves >> shift1 & fastMask);
    tempMoves |= (tempMoves >> shift1 & fastMask);
    tempMoves |= (tempMoves >> shift1 & fastMask);
    output |= (tempMoves >> shift1 & MACROmask1) & emptySquares;

    fastMask = MACROmask2 & enemyStones;
    tempMoves = (friendlyStones << shift1 & fastMask);
    tempMoves |= (tempMoves << shift1 & fastMask);
    tempMoves |= (tempMoves << shift1 & fastMask);
    tempMoves |= (tempMoves << shift1 & fastMask);
    tempMoves |= (tempMoves << shift1 & fastMask);
    tempMoves |= (tempMoves << shift1 & fastMask);
    output |= (tempMoves << shift1 & MACROmask2) & emptySquares;

    fastMask = MACROmask3 & enemyStones;
    tempMoves = (friendlyStones >> shift2 & fastMask);
    tempMoves |= (tempMoves >> shift2 & fastMask);
    tempMoves |= (tempMoves >> shift2 & fastMask);
    tempMoves |= (tempMoves >> shift2 & fastMask);
    tempMoves |= (tempMoves >> shift2 & fastMask);
    tempMoves |= (tempMoves >> shift2 & fastMask);
    output |= (tempMoves >> shift2 & MACROmask3) & emptySquares;

    fastMask = MACROmask4 & enemyStones;
    tempMoves = (friendlyStones << shift2 & fastMask);
    tempMoves |= (tempMoves << shift2 & fastMask);
    tempMoves |= (tempMoves << shift2 & fastMask);
    tempMoves |= (tempMoves << shift2 & fastMask);
    tempMoves |= (tempMoves << shift2 & fastMask);
    tempMoves |= (tempMoves << shift2 & fastMask);
    output |= (tempMoves << shift2 & MACROmask4) & emptySquares;

    fastMask = MACROmask5 & enemyStones;
    tempMoves = (friendlyStones >> shift3 & fastMask);
    tempMoves |= (tempMoves >> shift3 & fastMask);
    tempMoves |= (tempMoves >> shift3 & fastMask);
    tempMoves |= (tempMoves >> shift3 & fastMask);
    tempMoves |= (tempMoves >> shift3 & fastMask);
    tempMoves |= (tempMoves >> shift3 & fastMask);
    output |= (tempMoves >> shift3 & MACROmask5) & emptySquares;

    fastMask = MACROmask6 & enemyStones;
    tempMoves = (friendlyStones << shift3 & fastMask);
    tempMoves |= (tempMoves << shift3 & fastMask);
    tempMoves |= (tempMoves << shift3 & fastMask);
    tempMoves |= (tempMoves << shift3 & fastMask);
    tempMoves |= (tempMoves << shift3 & fastMask);
    tempMoves |= (tempMoves << shift3 & fastMask);
    output |= (tempMoves << shift3 & MACROmask6) & emptySquares;

    fastMask = MACROmask7 & enemyStones;
    tempMoves = (friendlyStones >> shift4 & fastMask);
    tempMoves |= (tempMoves >> shift4 & fastMask);
    tempMoves |= (tempMoves >> shift4 & fastMask);
    tempMoves |= (tempMoves >> shift4 & fastMask);
    tempMoves |= (tempMoves >> shift4 & fastMask);
    tempMoves |= (tempMoves >> shift4 & fastMask);
    output |= (tempMoves >> shift4 & MACROmask7) & emptySquares;

    fastMask = MACROmask8 & enemyStones;
    tempMoves = (friendlyStones << shift4 & fastMask);
    tempMoves |= (tempMoves << shift4 & fastMask);
    tempMoves |= (tempMoves << shift4 & fastMask);
    tempMoves |= (tempMoves << shift4 & fastMask);
    tempMoves |= (tempMoves << shift4 & fastMask);
    tempMoves |= (tempMoves << shift4 & fastMask);
    output |= (tempMoves << shift4 & MACROmask8) & emptySquares;

    // These builtin_expects add about 1 million nodes/s
    if(__builtin_expect(output, 1)) {
        // This little trick saves a few cycles over looping from 0 to 64.
        // Worst case is the amount of squares
        while(__builtin_expect(output, 1)) {
            *(*mlPointer)++ = __builtin_ctzl(output);
            // output ^= ONE64 << __builtin_ctzl(output);
            output = (output-1) & output; // Slightly faster than the one above
        }
        return;
    }
    *(*mlPointer)++ = -1;
}

void Board::turnStonesFromMove(uint8_t square) {
    // This assumes the piece is already placed, it will not
    // placed the piece or turn the pieces.
    uint64_t friendlyStones = pos.team[pos.turn];
    uint64_t enemyStones = pos.team[!pos.turn];
    register uint64_t tempOutput;
    uint64_t piecePlaced = ONE64 << square;
    uint64_t ifCaptured;
    uint64_t output = 0;
    register uint64_t fastMask;

    fastMask = MACROmask1 & enemyStones;
    tempOutput = (piecePlaced >> shift1 & fastMask);
    tempOutput |= (tempOutput >> shift1 & fastMask);
    tempOutput |= (tempOutput >> shift1 & fastMask);
    tempOutput |= (tempOutput >> shift1 & fastMask);
    tempOutput |= (tempOutput >> shift1 & fastMask);
    tempOutput |= (tempOutput >> shift1 & fastMask);
    ifCaptured = (tempOutput >> shift1 & MACROmask1) & friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    fastMask = MACROmask2 & enemyStones;
    tempOutput = (piecePlaced << shift1 & fastMask);
    tempOutput |= (tempOutput << shift1 & fastMask);
    tempOutput |= (tempOutput << shift1 & fastMask);
    tempOutput |= (tempOutput << shift1 & fastMask);
    tempOutput |= (tempOutput << shift1 & fastMask);
    tempOutput |= (tempOutput << shift1 & fastMask);
    ifCaptured = (tempOutput << shift1 & MACROmask2) & friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    fastMask = MACROmask3 & enemyStones;
    tempOutput = (piecePlaced >> shift2 & fastMask);
    tempOutput |= (tempOutput >> shift2 & fastMask);
    tempOutput |= (tempOutput >> shift2 & fastMask);
    tempOutput |= (tempOutput >> shift2 & fastMask);
    tempOutput |= (tempOutput >> shift2 & fastMask);
    tempOutput |= (tempOutput >> shift2 & fastMask);
    ifCaptured = (tempOutput >> shift2 & MACROmask3) & friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    fastMask = MACROmask4 & enemyStones;
    tempOutput = (piecePlaced << shift2 & fastMask);
    tempOutput |= (tempOutput << shift2 & fastMask);
    tempOutput |= (tempOutput << shift2 & fastMask);
    tempOutput |= (tempOutput << shift2 & fastMask);
    tempOutput |= (tempOutput << shift2 & fastMask);
    tempOutput |= (tempOutput << shift2 & fastMask);
    ifCaptured = (tempOutput << shift2 & MACROmask4) & friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    fastMask = MACROmask5 & enemyStones;
    tempOutput = (piecePlaced >> shift3 & fastMask);
    tempOutput |= (tempOutput >> shift3 & fastMask);
    tempOutput |= (tempOutput >> shift3 & fastMask);
    tempOutput |= (tempOutput >> shift3 & fastMask);
    tempOutput |= (tempOutput >> shift3 & fastMask);
    tempOutput |= (tempOutput >> shift3 & fastMask);
    ifCaptured = (tempOutput >> shift3 & MACROmask5) & friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    fastMask = MACROmask6 & enemyStones;
    tempOutput = (piecePlaced << shift3 & fastMask);
    tempOutput |= (tempOutput << shift3 & fastMask);
    tempOutput |= (tempOutput << shift3 & fastMask);
    tempOutput |= (tempOutput << shift3 & fastMask);
    tempOutput |= (tempOutput << shift3 & fastMask);
    tempOutput |= (tempOutput << shift3 & fastMask);
    ifCaptured = (tempOutput << shift3 & MACROmask6) & friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    fastMask = MACROmask7 & enemyStones;
    tempOutput = (piecePlaced >> shift4 & fastMask);
    tempOutput |= (tempOutput >> shift4 & fastMask);
    tempOutput |= (tempOutput >> shift4 & fastMask);
    tempOutput |= (tempOutput >> shift4 & fastMask);
    tempOutput |= (tempOutput >> shift4 & fastMask);
    tempOutput |= (tempOutput >> shift4 & fastMask);
    ifCaptured = (tempOutput >> shift4 & MACROmask7) & friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    fastMask = MACROmask8 & enemyStones;
    tempOutput = (piecePlaced << shift4 & fastMask);
    tempOutput |= (tempOutput << shift4 & fastMask);
    tempOutput |= (tempOutput << shift4 & fastMask);
    tempOutput |= (tempOutput << shift4 & fastMask);
    tempOutput |= (tempOutput << shift4 & fastMask);
    tempOutput |= (tempOutput << shift4 & fastMask);
    ifCaptured = (tempOutput << shift4 & MACROmask8) & friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    pos.team[BLACK] ^= output;
    pos.team[WHITE] ^= output;
}

bool Board::doMove(int8_t square) {
    // If the player is passing
    if(square == -1) {
        // if(pos.lastMoveSkipped) {
        //     return true;
        // }
        // pos.lastMoveSkipped = true;
        // pos.turn = !pos.turn;
        // return false;

        // The code above is what this is doing, but this is branchless
        pos.turn = !pos.turn;
        return !(pos.lastMoveSkipped = !pos.lastMoveSkipped);
    }

    turnStonesFromMove(square);

    pos.team[BLACK] |= ((ONE64 & pos.turn) << square);
    pos.team[WHITE] |= ((ONE64 & !pos.turn) << square);

    // Set the turn to the other player
    pos.turn = !pos.turn;

    // This move was not passed.
    pos.lastMoveSkipped = false;

    pos.occupied = pos.team[pos.turn] | pos.team[!pos.turn];

    // The game is not over
    return false;
}