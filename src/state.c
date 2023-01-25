#include <stdbool.h>
#include <string.h>

#include "state.h"

#ifdef CHECK_ACTIONS
#include <stdio.h>

#include "errorcodes.h"
#endif


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


void State_derive_neighbor_count(struct State *state) {
    memset(state->neighbor_count, 0,
        sizeof(uint_fast8_t) * NUM_PLAYERS * GRID_SIZE * GRID_SIZE);

    for (int p = 0; p < NUM_PLAYERS; p++) {
        for (int i = 0; i < state->piece_count[p]; i++) {
            struct Piece *piece = &state->pieces[p][i];

            for (int d = 0; d < NUM_DIRECTIONS; d++) {
                struct Coords coords = piece->coords;
                Coords_move(&coords, d);
                state->neighbor_count[piece->player][coords.q][coords.r] += 1;
            }
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
        for (int t = 0; t < NUM_PIECETYPES; t++ ) {
            if (t == QUEEN_BEE) continue;

            for (int d = 0; d < NUM_DIRECTIONS; d++) {
                state->actions[state->action_count].from.q = PLACE_ACTION;
                state->actions[state->action_count].from.r = t;
                state->actions[state->action_count].to.q = 0;
                state->actions[state->action_count].to.r = 0;
                Coords_move(&state->actions[state->action_count++].to, d);
            }
        }

        return;
    }

    // Places
    bool pieces_to_place = false;
    for (int t = 0; t < NUM_PIECETYPES; t++) {
        if (state->hands[state->turn][t] > 0) {
            pieces_to_place = true;
            break;
        }
    }
    if (pieces_to_place) {
        struct Coords place_coords[MAX_PLACE_SPOTS];
        int place_coords_count = 0;
        bool place_crumbs[GRID_SIZE][GRID_SIZE];
        memset(place_crumbs, 0, sizeof(bool) * GRID_SIZE * GRID_SIZE);
        for (int i = 0; i < state->piece_count[state->turn]; i++) {
            for (int d = 0; d < NUM_DIRECTIONS; d++) {
                struct Coords coords = state->pieces[state->turn][i].coords;
                Coords_move(&coords, d);

                if (!state->grid[coords.q][coords.r]
                        && state->neighbor_count[!state->turn][coords.q][coords.r] == 0
                        && !place_crumbs[coords.q][coords.r]) {
                    place_coords[place_coords_count++] = coords;
                    place_crumbs[coords.q][coords.r] = true;
                }
            }
        }
        for (int t = 0; t < NUM_PIECETYPES; t++) {
            if (state->hands[state->turn][t] == 0) continue;

            for (int i = 0; i < place_coords_count; i++) {
                state->actions[state->action_count].from.q = PLACE_ACTION;
                state->actions[state->action_count].from.r = t;
                state->actions[state->action_count++].to = place_coords[i];
            }
        }
    }

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
    State_derive_neighbor_count(state);
    State_derive_actions(state);
}


void State_new(struct State *state) {
    memset(state, 0, sizeof(struct State));
    State_derive(state);
}


void State_act(struct State *state, const struct Action *action) {
    #ifdef CHECK_ACTIONS
    bool valid_action = false;
    for (int i = 0; i < state->action_count; i++) {
        if (!memcmp(&state->actions[i], action, sizeof(struct Action))) {
            valid_action = true;
            break;
        }
    }
    if (!valid_action) {
        fprintf(stderr, "Illegal action!\n");
        exit(ERROR_ILLEGAL_ACTION);
    }
    #endif

    if (action->from.q == PLACE_ACTION) {
        struct Piece *piece =
            &state->pieces[state->turn][state->piece_count[state->turn]];
        piece->type = action->from.r;
        piece->coords.q = action->to.q;
        piece->coords.r = action->to.r;
        piece->player = state->turn;;

        state->grid[action->to.q][action->to.r] = piece;

        state->piece_count[state->turn]++;
        state->hands[state->turn][piece->type]--;

        for (int d = 0; d < NUM_DIRECTIONS; d++) {
            struct Coords coords = piece->coords;
            Coords_move(&coords, d);
            state->neighbor_count[piece->player][coords.q][coords.r] += 1;
        }

        state->turn = !state->turn;

        State_derive_actions(state);

        return;
    }
}
