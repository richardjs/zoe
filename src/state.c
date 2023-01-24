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
    memset(state->grid, 0, sizeof(struct Piece*) * GRID_SIZE * GRID_SIZE);

    for (int p = 0; p < NUM_PLAYERS; p++) {
        for (int i = 0; i < state->piece_count[p]; i++) {
            struct Piece *piece = &state->pieces[p][i];
            struct Piece *p = state->grid[piece->coords.q][piece->coords.r];
            while (p && p->on_top) {
                if (p->on_top == piece) {
                    goto skip;
                }
                p = p->on_top;
            }

            state->grid[piece->coords.q][piece->coords.r] = piece;
            skip:
        }
    }
}


void State_derive_hands(struct State *state) {
    for (int p = 0; p < NUM_PLAYERS; p++) {
        state->hands[p][ANT] = NUM_ANTS;
        state->hands[p][BEETLE] = NUM_BEETLES;
        state->hands[p][GRASSHOPPER] = NUM_GRASSHOPPERS;
        state->hands[p][SPIDER] = NUM_SPIDERS;
        state->hands[p][QUEEN_BEE] = NUM_QUEEN_BEES;

        for (int i = 0; i < state->piece_count[p]; i++) {
            state->hands[p][state->pieces[p][i].type]--;
        }
    }
}


void State_derive_actions(struct State *state) {
    state->action_count = 0;

    // P1 start actions
    if (state->piece_count[P1] == 0) {
        for (int t = 0; t < NUM_PIECETYPES; t++) {
            if (t == QUEEN_BEE) continue;

            state->actions[state->action_count].from.q = PLACE_ACTION;
            state->actions[state->action_count].from.r = t;
            state->actions[state->action_count].to.q = 0;
            state->actions[state->action_count++].to.r = 0;
        }

        return;
    }
    // P2 start actions
    if (state->piece_count[P2] == 0) {
        for (int t =0; t < NUM_PIECETYPES; t++ ) {
            if (t == QUEEN_BEE) continue;

            state->actions[state->action_count].from.q = PLACE_ACTION;
            state->actions[state->action_count].from.r = t;
            state->actions[state->action_count].to.q = 0;
            state->actions[state->action_count++].to.r = 0;
        }

        return;
    }

    // TODO places

    // TODO moves
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
    State_derive_hands(state);
    State_derive_actions(state);
}


void State_new(struct State *state) {
    memset(state, 0, sizeof(struct State));
    State_derive(state);
}
