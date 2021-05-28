#ifndef MOVELIST_H
#define MOVELIST_H

#include <cstdint>

#include "MoveUndo.h"

// More than the most possible moves that can be made in any position.
static const uint8_t MAXMOVES = 64;

class MoveList {
public:
    uint8_t moves[MAXMOVES];
    uint8_t * first;
    uint8_t * last;

    MoveList() {
        first = &moves[0];
        last = first;
    }

    void add(uint8_t move) {
        *last = move;
        last++;
    }

    inline uint8_t length() {
        return last - first;
    }

    inline void clear() {
        last = first;
    }
};

#endif