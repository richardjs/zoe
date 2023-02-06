#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "state.h"


struct Piece *State_top_piece(const struct State *state, unsigned int q, unsigned r) {
    struct Piece *piece = state->grid[q][r];
    if (!piece) return piece;

    while (piece->on_top) {
        piece = piece->on_top;
    }


    return piece;
}


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


bool State_compare(const struct State *state, const struct State *other, bool debug_print) {
    if (state->turn != other->turn) {
        if (debug_print) fprintf(stderr, "Different turn\n");
        return true;
    }

    for (int p = 0; p < NUM_PLAYERS; p++ ){
        if (state->piece_count[p] != other->piece_count[p]) {
            if (debug_print) fprintf(stderr, "Different piece_count\n");
            return true;
        }

        for (int i = 0; i < state->piece_count[p]; i++) {
            if (Piece_compare(&state->pieces[p][i], &other->pieces[p][i])) {
                if (debug_print) fprintf(stderr, "Different pieces\n");
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
                if (debug_print) fprintf(stderr, "Different grid NULLs\n");
                return true;
            }

            if (Piece_compare(state->grid[q][r], other->grid[q][r])) {
                if (debug_print) fprintf(stderr, "Different grid\n");
                return true;
            }

            for (int p = 0; p < NUM_PLAYERS; p++) {
                if (state->neighbor_count[p][q][r] != other->neighbor_count[p][q][r]) {
                    if (debug_print) fprintf(stderr, "Different neighbor_count\n");
                    return true;
                }
            }
        }
    }

    if (state->action_count != other->action_count) {
        if (debug_print) fprintf(stderr, "Different action_count: %d != %d\n",
            state->action_count, other->action_count);
        return true;
    }

    // TODO Compare actions
    // Actions may be generated in a different order for the same state
    // (if pieces are in a different order), so a straight comparision
    // will not work

    return false;
}
