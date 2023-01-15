#include <string.h>

#include "state.h"


void State_derive_grid(struct State *state) {
    for (int p = 0; p < NUM_PLAYERS; p++) {
        for (int i = 0; i < state->piece_count[p]; i++) {
            struct Piece *piece = &state->pieces[p][i];
            state->grid[piece->coords.q][piece->coords.r] = piece;
        }
    }
}


void State_derive_actions(struct State *state) {
    // TODO (heh)
}


void State_derive(struct State *state) {
    State_derive_grid(state);
    State_derive_actions(state);
}


void State_new(struct State *state) {
    memset(state, 0, sizeof(struct State));
}
