#include "Board.h"
#include <cstdlib>

#define shift1 1
#define shift2 9
#define shift3 8
#define shift4 7

#define MACROmask1 0x7F7F7F7F7F7F7F7F
#define MACROmask2 0xFEFEFEFEFEFEFEFE
#define MACROmask3 0x007F7F7F7F7F7F7F
#define MACROmask4 0xFEFEFEFEFEFEFE00
#define MACROmask5 0xFFFFFFFFFFFFFFFF
#define MACROmask6 0xFFFFFFFFFFFFFFFF
#define MACROmask7 0x00FEFEFEFEFEFEFE
#define MACROmask8 0x7F7F7F7F7F7F7F00

//  Original Code
// This is the slightly more readable version, the version in use is the same thing but without loops and with constants.
// Using constants allows the CPU to spend less time with memory and more time doing actual math.


// // The following constants are ordered by: Right, down right, down, down left, left, up left, up, up right.

// // When you shift the entire number, these are used to prevent wrapping around
// static const uint64_t MASKS[] = {
//     0x7F7F7F7F7F7F7F7F, 0x007F7F7F7F7F7F7F, 0xFFFFFFFFFFFFFFFF, 0x00FEFEFEFEFEFEFE,
//     0xFEFEFEFEFEFEFEFE, 0xFEFEFEFEFEFEFE00, 0xFFFFFFFFFFFFFFFF, 0x7F7F7F7F7F7F7F00};

// // These are the amount to shift for each direction
// // I repeated values so you wouldn't have to modular
// static const uint64_t SHIFTS[] = {1, 9, 8, 7, 1, 9, 8, 7};
// void Board::getAllLegalMoves(uint8_t ** mlPointer) {
//     uint64_t friendlyStones = pos.team[turn];
//     uint64_t enemyStones = pos.team[!turn];
//     // A temporary holder for the moves in each direction
//     uint64_t tempMoves;
//     // Squares that don't have a stone on them.
//     uint64_t emptySquares = ~occupied;
//     uint64_t output = 0;

//     uint8_t shift;
//     uint64_t mask2;
//     uint64_t fastMask1;
//     uint64_t mask1;
//     uint64_t fastMask2;
//     for(uint8_t dir = 0; dir < DIROVER2; dir++) {
//         // The shift + DIROVER2 is the same as not adding + DIROVER2
//         shift = SHIFTS[dir];
//         mask1 = MASKS[dir + DIROVER2];
//         fastMask1 = mask1 & enemyStones;
//         mask2 = MASKS[dir];
//         fastMask2 = mask2 & enemyStones;


//         // This takes all the friendly disks and adds all the enemy 
//         // disks that are adjacent in the direction of the friendly disk.
//         tempMoves = (friendlyStones >> shift & fastMask2);
//         // Five is the length of the board -1 for the shift above,
//         // -1 for the friendly piece above and -1 for the piece going to be placed.
//         // this loop goes through all the enemy pieces found above and traces along 
//         // in that direction until it hits an empty square.
//         tempMoves |= (tempMoves >> shift & fastMask2);
//         tempMoves |= (tempMoves >> shift & fastMask2);
//         tempMoves |= (tempMoves >> shift & fastMask2);
//         tempMoves |= (tempMoves >> shift & fastMask2);
//         tempMoves |= (tempMoves >> shift & fastMask2);
//         // Now we have all the enemy stones in the direction from all the friendlies.
//         // Now you shift one more time and & it with empty squares. This gets all the 
//         // places you could place a stone to turn enemy squares in the direction.
//         output |= (tempMoves >> shift & mask2) & emptySquares;


//         // Same logic as above but different direction
//         tempMoves = (friendlyStones << shift & fastMask1);
//         tempMoves |= (tempMoves << shift & fastMask1);
//         tempMoves |= (tempMoves << shift & fastMask1);
//         tempMoves |= (tempMoves << shift & fastMask1);
//         tempMoves |= (tempMoves << shift & fastMask1);
//         tempMoves |= (tempMoves << shift & fastMask1);
//         output |= (tempMoves << shift & mask1) & emptySquares;
//     }

//     if(output) {
//         // This little trick saves a few cycles over looping from 0 to 64.
//         // Worst case is the amount of squares
//         while(output) {
//             *(*mlPointer)++ = __builtin_ctzl(output);
//             // output ^= ONE64 << __builtin_ctzl(output);
//             output = (output-1) & output; // Slightly faster than the one above
//         }
//         return;
//     }
//     *(*mlPointer)++ = -1;
// }

// void Board::turnStonesFromMove(uint8_t square) {
//     // This assumes the piece is already placed, it will not
//     // placed the piece or turn the pieces.
//     uint64_t friendlyStones = pos.team[turn];
//     uint64_t enemyStones = pos.team[!turn];
//     uint64_t tempOutput;
//     uint64_t piecePlaced = ONE64 << square;
//     uint64_t ifCaptured;
//     uint64_t output = 0;

//     uint8_t shift;
//     uint64_t mask1;
//     uint64_t fastMask1;
//     uint64_t mask2;
//     uint64_t fastMask2;
//     for(uint8_t dir = 0; dir < DIROVER2; dir++) {
//         // The shift + 4 is the same as not adding + 4
//         shift = SHIFTS[dir];
//         mask1 = MASKS[dir + DIROVER2];
//         fastMask1 = mask1 & enemyStones;
//         mask2 = MASKS[dir];
//         fastMask2 = mask2 & enemyStones;


//         // Gets the pieces next to the new piece
//         tempOutput = (piecePlaced << shift & fastMask1);
//         // This uses mostly the same logic as the getLegalMoves function
//         // This one traces from the placed piece to a friendly
//         // If there is a friendly, all the stones between get flipped.
//         tempOutput |= (tempOutput << shift & fastMask1);
//         tempOutput |= (tempOutput << shift & fastMask1);
//         tempOutput |= (tempOutput << shift & fastMask1);
//         tempOutput |= (tempOutput << shift & fastMask1);
//         tempOutput |= (tempOutput << shift & fastMask1);
//         ifCaptured = (tempOutput << shift & mask1) & friendlyStones;
//         // Determine whether the disks were captured. 
//         output |= (ifCaptured ? tempOutput : 0);

//         // Same logic as before
//         tempOutput = (piecePlaced >> shift & fastMask2);
//         tempOutput |= (tempOutput >> shift & fastMask2);
//         tempOutput |= (tempOutput >> shift & fastMask2);
//         tempOutput |= (tempOutput >> shift & fastMask2);
//         tempOutput |= (tempOutput >> shift & fastMask2);
//         tempOutput |= (tempOutput >> shift & fastMask2);
//         ifCaptured = (tempOutput >> shift & mask2) & friendlyStones;
//         output |= (ifCaptured ? tempOutput : 0);
//     }

//     pos.team[BLACK] ^= output;
//     pos.team[WHITE] ^= output;
// } 

void Board::getAllLegalMoves(uint8_t ** mlPointer) {
    uint64_t friendlyStones = pos.team[turn];
    uint64_t enemyStones = pos.team[!turn];
    // A temporary holder for the moves in each direction
    register uint64_t tempMoves;
    // Squares that don't have a stone on them.
    uint64_t emptySquares = ~occupied;
    uint64_t output = 0;

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

    if(output) {
        // This little trick saves a few cycles over looping from 0 to 64.
        // Worst case is the amount of squares
        while(output) {
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
    uint64_t friendlyStones = pos.team[turn];
    uint64_t enemyStones = pos.team[!turn];
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