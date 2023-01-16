#include <string.h>

#include "state.h"
#include <stdio.h>


void Piece_pieces_above(const struct Piece *piece, struct Piece *above[]) {
    const struct Piece *p = piece;
    memset(above, 0, sizeof(struct Piece *) * MAX_ABOVE);

    if (!piece) {
        return;
    }

    for (int i = 0; i < MAX_ABOVE; i++) {
        if (!p->on_top) {
            return;
        }

        above[i] = p->on_top;
        p = p->on_top;
    }
}
