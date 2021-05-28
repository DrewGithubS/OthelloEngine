#ifndef BOARD_CPP
#define BOARD_CPP

#include <iostream>
#include "MoveUndo.h"
#include "Board.h"
#include "BoardMoveGen.cpp"
#include "MoveList.cpp"


uint8_t countBitsSet(uint64_t in) {
    uint8_t output = 0;
    while(in) {
        output++;
        in = (in-1) & in;
    }
    return output;
}

Board::Board() {
    readFromString("8/8/8/3WB3/3BW3/8/8/8 1 0");
}
Board::Board(char * position) {
    readFromString(position);
}

bool Board::squareIsOccupied(uint8_t square) {
    return (occupied) >> square & 1;
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
        std::cout << "Move: " << (turn ? "Black\n" : "White\n");
        std::cout << "Last Move Passed: " << lastMoveSkipped << std::endl;
    }
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
    turn = posString[charCount] == '1';
    // Move charCount to the next number
    charCount += 2;
    lastMoveSkipped = posString[charCount] == '1';

    occupied = pos.team[turn] | pos.team[!turn];
}

bool Board::doMove(int8_t square) {
    // If the player is passing
    if(square == -1) {
        if(lastMoveSkipped) {
            return true;
        }
        lastMoveSkipped = true;
        turn = !turn;
        return false;
    }

    turnStonesFromMove(square);

    pos.team[BLACK] |= ((ONE64 & turn) << square);
    pos.team[WHITE] |= ((ONE64 & !turn) << square);

    // Set the turn to the other player
    turn = !turn;

    // This move was not passed.
    lastMoveSkipped = false;

    occupied = pos.team[turn] | pos.team[!turn];

    // The game is not over
    return false;
}

int8_t Board::getWinner() {
    uint8_t blackCount = countBitsSet(pos.team[BLACK]);
    uint8_t whiteCount = countBitsSet(pos.team[WHITE]);
    return blackCount > whiteCount ? -1 : (whiteCount > blackCount ? 1 : 0);
}

#endif