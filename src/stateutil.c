#include <stdint.h>

#include "state.h"


unsigned int Piece_num_above(const struct Piece *piece) {
    unsigned int n = 0;
    const struct Piece *p = piece;
    while (p->on_top) {
        n += 1;
        p = p->on_top;
    }
    return n;
}


struct Piece *Piece_above(const struct Piece *piece, unsigned int n) {
    struct Piece *p = (struct Piece *)piece;
    for (int i = 0; i < n; i++) {
        p = p->on_top;
    }
    return p;
}
