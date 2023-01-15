#include <string.h>

#include "state.h"


void State_derive_piece_players(struct State *state) {
    for (int p = 0; p < NUM_PLAYERS; p++) {
        for (int i = 0; i < state->piece_count[p]; i++) {
            state->pieces[p][i].player = p;
        }
    }
}


void State_derive_grid(struct State *state) {
    // TODO Is there a better way to handle cleaning stale data here?
    memset(state->grid, 0, sizeof(struct Piece*) * GRID_SIZE * GRID_SIZE);

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


/* Note on efficiency:
 * State_derive is not guranteed to be efficient, and is intended as a
 * convenient one-stop function for code without efficiency needs. It's
 * assumed optimized code will understand the technical details of the
 * State structure and granularly derive (or manually manage) as needed.
 */
void State_derive(struct State *state) {
    State_derive_piece_players(state);
    State_derive_grid(state);
    State_derive_actions(state);
}


void State_new(struct State *state) {
    memset(state, 0, sizeof(struct State));
    State_derive(state);
}
