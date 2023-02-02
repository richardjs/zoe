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


void State_spider_walk(
    struct State *state,
    const struct Piece *piece,
    const struct Coords *coords,
    bool crumbs[GRID_SIZE][GRID_SIZE],
    bool tos[GRID_SIZE][GRID_SIZE],
    int depth
) {
    if (crumbs[coords->q][coords->r]) {
        return;
    }

    // If this is the root call, temporarily remove the piece from the
    // grid so it can't walk along itself
    if (depth == 0) {
        state->grid[coords->q][coords->r] = NULL;
    }


    if (depth == SPIDER_MOVES) {
        if (!tos[coords->q][coords->r]) {
            state->actions[state->action_count].from = piece->coords;
            state->actions[state->action_count++].to = *coords;
            tos[coords->q][coords->r] = true;
        }
        return;
    }

    // TODO Do we need to worry about different parts of the search
    // hitting the same hex at different depths?
    crumbs[coords->q][coords->r] = true;

    for (int d = 0; d < NUM_DIRECTIONS; d++) {
        struct Coords c = *coords;
        Coords_move(&c, d);
        if (!state->grid[c.q][c.r]) {
            continue;
        }

        c = *coords;
        Coords_move(&c, Direction_rotate(d, 2));
        if (!state->grid[c.q][c.r]) {
            c = *coords;
            Coords_move(&c, Direction_rotate(d, 1));
            if (!state->grid[c.q][c.r]) {
                State_spider_walk(state, piece, &c, crumbs, tos, depth+1);
            }
        }

        c = *coords;
        Coords_move(&c, Direction_rotate(d, -2));
        if (!state->grid[c.q][c.r]) {
            c = *coords;
            Coords_move(&c, Direction_rotate(d, -1));
            if (!state->grid[c.q][c.r]) {
                State_spider_walk(state, piece, &c, crumbs, tos, depth+1);
            }
        }
    }

    // Put the piece back on the grid
    if (depth == 0) {
        state->grid[coords->q][coords->r] = (struct Piece *)piece;
    }

    crumbs[coords->q][coords->r] = false;
}


/* Finds cut points (i.e. pieces that can't be moved due to the one hive
 * rule) using an adaptation of Hopcroft & Tarjan's algorithm.
 * See:
 * https://dl.acm.org/doi/10.1145/362248.362272
 * https://en.wikipedia.org/wiki/Biconnected_component
 */
void State_derive_cut_points(struct State *state) {
    memset(state->cut_points, 0, sizeof(bool)*GRID_SIZE*GRID_SIZE);

    struct Coords stack[MAX_PIECES];
    int_fast8_t sp = 0;
    stack[sp] = state->pieces[P1][0].coords;

    uint_fast8_t depth[MAX_PIECES];
    depth[sp] = 0;

    uint_fast8_t lowpoint[MAX_PIECES];

    // crumbs[q][r] is the same as depth[sp] for a particular Coords
    int_fast8_t crumbs[GRID_SIZE][GRID_SIZE];
    memset(&crumbs, -1, sizeof(int_fast8_t)*GRID_SIZE*GRID_SIZE);
    crumbs[stack[sp].q][stack[sp].r] = 0;

    uint_fast8_t next_direction[MAX_PIECES];
    next_direction[MAX_PIECES] = 0;

    uint_fast8_t parent_direction[MAX_PIECES];
    parent_direction[0] = -1;

    // If the root (i.e. sp=0) has more than one child, it is an
    // articulation point, because otherwise the DFS would have worked
    // its away around to root's other children
    uint_fast8_t root_children = 0;

    while (sp >= 0) {
        struct Coords head;
        bool found_head = false;
        // Try each direction looking for an edge
        while (!found_head && next_direction[sp] != NUM_DIRECTIONS) {
            // Don't search back to the parent
            if (next_direction[sp] == parent_direction[sp]) {
                next_direction[sp]++;
                continue;
            }
            head = stack[sp];
            Coords_move(&head, next_direction[sp]);
            if (state->grid[head.q][head.r]) {
                found_head = true;
            }
            next_direction[sp]++;
        }

        if (found_head) {
            if (crumbs[head.q][head.r] >= 0) {
                if (crumbs[head.q][head.r] < lowpoint[sp]) {
                    lowpoint[sp] = crumbs[head.q][head.r];
                }
            } else {
                if (sp == 0) {
                    root_children++;
                }

                depth[sp+1] = depth[sp] + 1;
                crumbs[head.q][head.r] = depth[sp] + 1;
                lowpoint[sp+1] = depth[sp];
                next_direction[sp+1] = 0;
                parent_direction[sp+1] = OPPOSITE[next_direction[sp] - 1];
                stack[sp+1] = head;
                sp++;
            }
        } else {
            if (lowpoint[sp] == depth[sp-1] && (sp-1 != 0)) {
                state->cut_points[stack[sp-1].q][stack[sp-1].r] = true;
            } else {
                if (lowpoint[sp] < lowpoint[sp-1]) {
                    lowpoint[sp-1] = lowpoint[sp];
                }
            }
            sp--;
        }
    }

    if (root_children > 1) {
        state->cut_points[stack[0].q][stack[0].r] = true;
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

    // Moves

    // A player can't move until their queen is placed
    if (state->hands[state->turn][QUEEN_BEE]) {
        return;
    }

    for (int i = 0; i < state->piece_count[state->turn]; i++) {
        struct Piece *piece = &state->pieces[state->turn][i];
        struct Coords *coords = &piece->coords;

        // One hive rule
        if (state->cut_points[piece->coords.q][piece->coords.r]) {
            continue;
        }

        switch (piece->type) {
            case GRASSHOPPER:
                for (int d = 0; d < NUM_DIRECTIONS; d++) {
                    // Look for adjacent pieces
                    struct Coords c = *coords;
                    Coords_move(&c, d);
                    if (!state->grid[c.q][c.r]) {
                        continue;
                    }

                    // Find an empty spot in the direction of the piece
                    do {
                        Coords_move(&c, d);
                    } while (state->grid[c.q][c.r]);

                    state->actions[state->action_count].from = piece->coords;
                    state->actions[state->action_count++].to = c;
                }
                break;

            case QUEEN_BEE:
                for (int d = 0; d < NUM_DIRECTIONS; d++) {
                    // Look for adjacent pieces
                    struct Coords c = *coords;
                    Coords_move(&c, d);
                    if (!state->grid[c.q][c.r]) {
                        continue;
                    }

                    // For every adjacent piece, try to move to the
                    // right and left of it, first checking for
                    // freedom to move
                    c = *coords;
                    Coords_move(&c, Direction_rotate(d, 2));
                    if (!state->grid[c.q][c.r]) {
                        c = *coords;
                        Coords_move(&c, Direction_rotate(d, 1));
                        if (!state->grid[c.q][c.r]) {
                            state->actions[state->action_count].from = piece->coords;
                            state->actions[state->action_count++].to = c;
                        }
                    }
                    c = *coords;
                    Coords_move(&c, Direction_rotate(d, -2));
                    if (!state->grid[c.q][c.r]) {
                        c = *coords;
                        Coords_move(&c, Direction_rotate(d, -1));
                        if (!state->grid[c.q][c.r]) {
                            state->actions[state->action_count].from = piece->coords;
                            state->actions[state->action_count++].to = c;
                        }
                    }
                }
                break;

            case SPIDER:
                bool crumbs[GRID_SIZE][GRID_SIZE];
                memset(crumbs, 0, sizeof(bool)*GRID_SIZE*GRID_SIZE);
                bool tos[GRID_SIZE][GRID_SIZE];
                memset(tos, 0, sizeof(bool)*GRID_SIZE*GRID_SIZE);
                State_spider_walk(state, piece, coords, crumbs, tos, 0);
                break;

            // TODO
            default:
        }
    }
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
    State_derive_cut_points(state);
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
