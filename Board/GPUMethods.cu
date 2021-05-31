#include <cstdint>
#include <iostream>

#include "GPUMethods.cudah"

const uint64_t ONE64GPU = 1;

const uint8_t shift1 = 1;
const uint8_t shift2 = 9;
const uint8_t shift3 = 8;
const uint8_t shift4 = 7;

const uint64_t MACROmask1 = 0x7F7F7F7F7F7F7F7F;
const uint64_t MACROmask2 = 0xFEFEFEFEFEFEFEFE;
const uint64_t MACROmask3 = 0x007F7F7F7F7F7F7F;
const uint64_t MACROmask4 = 0xFEFEFEFEFEFEFE00;
const uint64_t MACROmask5 = 0xFFFFFFFFFFFFFFFF;
const uint64_t MACROmask6 = 0xFFFFFFFFFFFFFFFF;
const uint64_t MACROmask7 = 0x00FEFEFEFEFEFEFE;
const uint64_t MACROmask8 = 0x7F7F7F7F7F7F7F00;


__device__ void getAllLegalMoves_d(int8_t ** mlPointer, uint64_t friendlyStones, uint64_t enemyStones, bool turn) {
    // A temporary holder for the moves in each direction
    uint64_t tempMoves;
    // Squares that don't have a stone on them.
    uint64_t emptySquares = ~(friendlyStones | enemyStones);
    uint64_t output = 0;

    uint64_t fastMask;

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
            *(*mlPointer)++ = __ffs(output);
            // output ^= ONE64 << __builtin_ctzl(output);
            output = (output-1) & output; // Slightly faster than the one above
        }
        return;
    }
    *(*mlPointer)++ = -1;
}

__device__ void turnStonesFromMove_d(int8_t square, uint64_t * friendlyStones, uint64_t * enemyStones) {
    // This assumes the piece is already placed, it will not
    // placed the piece or turn the pieces.
    uint64_t tempOutput;
    uint64_t piecePlaced = ONE64GPU << square;
    uint64_t ifCaptured;
    uint64_t output = 0;
    uint64_t fastMask;

    fastMask = MACROmask1 & *enemyStones;
    tempOutput = (piecePlaced >> shift1 & fastMask);
    tempOutput |= (tempOutput >> shift1 & fastMask);
    tempOutput |= (tempOutput >> shift1 & fastMask);
    tempOutput |= (tempOutput >> shift1 & fastMask);
    tempOutput |= (tempOutput >> shift1 & fastMask);
    tempOutput |= (tempOutput >> shift1 & fastMask);
    ifCaptured = (tempOutput >> shift1 & MACROmask1) & *friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    fastMask = MACROmask2 & *enemyStones;
    tempOutput = (piecePlaced << shift1 & fastMask);
    tempOutput |= (tempOutput << shift1 & fastMask);
    tempOutput |= (tempOutput << shift1 & fastMask);
    tempOutput |= (tempOutput << shift1 & fastMask);
    tempOutput |= (tempOutput << shift1 & fastMask);
    tempOutput |= (tempOutput << shift1 & fastMask);
    ifCaptured = (tempOutput << shift1 & MACROmask2) & *friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    fastMask = MACROmask3 & *enemyStones;
    tempOutput = (piecePlaced >> shift2 & fastMask);
    tempOutput |= (tempOutput >> shift2 & fastMask);
    tempOutput |= (tempOutput >> shift2 & fastMask);
    tempOutput |= (tempOutput >> shift2 & fastMask);
    tempOutput |= (tempOutput >> shift2 & fastMask);
    tempOutput |= (tempOutput >> shift2 & fastMask);
    ifCaptured = (tempOutput >> shift2 & MACROmask3) & *friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    fastMask = MACROmask4 & *enemyStones;
    tempOutput = (piecePlaced << shift2 & fastMask);
    tempOutput |= (tempOutput << shift2 & fastMask);
    tempOutput |= (tempOutput << shift2 & fastMask);
    tempOutput |= (tempOutput << shift2 & fastMask);
    tempOutput |= (tempOutput << shift2 & fastMask);
    tempOutput |= (tempOutput << shift2 & fastMask);
    ifCaptured = (tempOutput << shift2 & MACROmask4) & *friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    fastMask = MACROmask5 & *enemyStones;
    tempOutput = (piecePlaced >> shift3 & fastMask);
    tempOutput |= (tempOutput >> shift3 & fastMask);
    tempOutput |= (tempOutput >> shift3 & fastMask);
    tempOutput |= (tempOutput >> shift3 & fastMask);
    tempOutput |= (tempOutput >> shift3 & fastMask);
    tempOutput |= (tempOutput >> shift3 & fastMask);
    ifCaptured = (tempOutput >> shift3 & MACROmask5) & *friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    fastMask = MACROmask6 & *enemyStones;
    tempOutput = (piecePlaced << shift3 & fastMask);
    tempOutput |= (tempOutput << shift3 & fastMask);
    tempOutput |= (tempOutput << shift3 & fastMask);
    tempOutput |= (tempOutput << shift3 & fastMask);
    tempOutput |= (tempOutput << shift3 & fastMask);
    tempOutput |= (tempOutput << shift3 & fastMask);
    ifCaptured = (tempOutput << shift3 & MACROmask6) & *friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    fastMask = MACROmask7 & *enemyStones;
    tempOutput = (piecePlaced >> shift4 & fastMask);
    tempOutput |= (tempOutput >> shift4 & fastMask);
    tempOutput |= (tempOutput >> shift4 & fastMask);
    tempOutput |= (tempOutput >> shift4 & fastMask);
    tempOutput |= (tempOutput >> shift4 & fastMask);
    tempOutput |= (tempOutput >> shift4 & fastMask);
    ifCaptured = (tempOutput >> shift4 & MACROmask7) & *friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    fastMask = MACROmask8 & *enemyStones;
    tempOutput = (piecePlaced << shift4 & fastMask);
    tempOutput |= (tempOutput << shift4 & fastMask);
    tempOutput |= (tempOutput << shift4 & fastMask);
    tempOutput |= (tempOutput << shift4 & fastMask);
    tempOutput |= (tempOutput << shift4 & fastMask);
    tempOutput |= (tempOutput << shift4 & fastMask);
    ifCaptured = (tempOutput << shift4 & MACROmask8) & *friendlyStones;
    output |= (ifCaptured ? tempOutput : 0);

    *friendlyStones ^= output;
    *enemyStones ^= output;
}


__device__ bool doMove_d(int8_t square, uint64_t * friendlyStones, uint64_t * enemyStones, bool * lastMoveSkipped, bool * turn) {
    // If the player is passing
    if(square == -1) {
        // if(pos.lastMoveSkipped) {
        //     return true;
        // }
        // pos.lastMoveSkipped = true;
        // pos.turn = !pos.turn;
        // return false;

        // The code above is what this is doing, but this is branchless
        *turn = !*turn;
        return !(*lastMoveSkipped = !*lastMoveSkipped);
    }

    turnStonesFromMove_d(square, friendlyStones, enemyStones);

    *friendlyStones |= ((ONE64GPU & *turn) << square);
    *enemyStones |= ((ONE64GPU & !*turn) << square);

    // Set the turn to the other player
    *turn = !*turn;

    // This move was not passed.
    *lastMoveSkipped = false;

    // The game is not over
    return false;
}

__device__ void doPerft_d(uint64_t team[2], bool lastMoveSkipped, bool turn, int32_t depth, uint64_t * output) {
    int8_t moveList[64];
    int8_t * last = &moveList[0];
    uint64_t teamLocal[2] = {team[0], team[1]};

    getAllLegalMoves_d(&last, teamLocal[turn], teamLocal[!turn], turn);
    if(depth == 1) {
        (*output) += last - moveList;
        return;
    }

    // No need to undo because these are passed by value.
    for(int i = 0; i < (last - moveList); i++) {
        if(doMove_d(moveList[i], &teamLocal[turn], &teamLocal[!turn], &lastMoveSkipped, &turn)) {
            (*output)++;
            return;
        }
        doPerft_d(teamLocal, lastMoveSkipped, turn, depth-1, output);
    }
}

__global__ void doPerftsOnGPU(uint64_t ** teams, bool * lastMovesSkipped, bool * turns, int32_t depth, uint64_t * outputs, uint64_t threads) {
    const int threadNumber = blockDim.x * blockIdx.x + threadIdx.x;
    printf("Hello from block %d, thread %d\n", blockIdx.x, threadNumber);

    if(threadNumber < threads) {
        
        doPerft_d(teams[threadNumber], lastMovesSkipped[threadNumber], turns[threadNumber], depth, &outputs[threadNumber]);
    }
}