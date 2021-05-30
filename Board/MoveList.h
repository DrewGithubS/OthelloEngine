#ifndef MOVELIST_H
#define MOVELIST_H

#include <cstdint>

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

    inline uint8_t length() {
        return last - first;
    }

    inline void clear() {
        last = first;
    }
};

#endif
