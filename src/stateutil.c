#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "state.h"


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


bool Piece_compare(const struct Piece *piece, const struct Piece *other) {
    return !(
        piece->type == other->type &&
        piece->coords.q == other->coords.q &&
        piece->coords.r == other->coords.r &&
        piece->player == other->player &&
        (bool)piece->on_top == (bool)other->on_top &&
        !(piece->on_top && Piece_compare(piece->on_top, other->on_top))
    );
}


bool State_compare(const struct State *state, const struct State *other) {
    if (state->turn != other->turn) {
        return true;
    }

    for (int p = 0; p < NUM_PLAYERS; p++ ){
        if (state->piece_count[p] != other->piece_count[p]) {
            return true;
        }

        for (int i = 0; i < state->piece_count[p]; i++) {
            if (Piece_compare(&state->pieces[p][i], &other->pieces[p][i])) {
                return true;
            }
        }
    }

    for (int q = 0; q < GRID_SIZE; q++) {
        for (int r = 0; r < GRID_SIZE; r++) {
            if (state->grid[q][r] == NULL) {
                if (other->grid[q][r] == NULL) {
                    continue;
                }
                return true;
            }

            if (Piece_compare(state->grid[q][r], other->grid[q][r])) {
                return true;
            }
        }
    }

    // TODO Compare actions, once we implement them

    return false;
}
