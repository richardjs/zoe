#include "state.h"

#include <stdbool.h>
#include <string.h>

#ifdef CHECK_ACTIONS
#include <stdio.h>
#include <stdlib.h>

#include "errorcodes.h"
#endif

bool State_cut_point_neighbor(
    const struct State* state,
    const struct Coords* coords)
{
    struct Coords c;
    for (enum Direction d = 0; d < NUM_DIRECTIONS; d++) {
        c = *coords;
        Coords_move(&c, d);
        if (state->cut_points[c.q][c.r]) {
            return true;
        }
    }

    return false;
}

struct Piece* State_neighbor_piece(
    const struct State* state,
    const struct Coords* coords)
{
    struct Coords c;
    for (enum Direction d = 0; d < NUM_DIRECTIONS; d++) {
        c = *coords;
        Coords_move(&c, d);
        if (state->grid[c.q][c.r]) {
            return state->grid[c.q][c.r];
        }
    }

    return NULL;
}

int State_height_at(const struct State* state, const struct Coords* coords)
{
    struct Piece* piece = state->grid[coords->q][coords->r];
    if (!piece) {
        return 0;
    }
    int height = 1;
    while (piece->on_top) {
        height++;
        piece = piece->on_top;
    }
    return height;
}

void State_derive_piece_players(struct State* state)
{
    for (int p = 0; p < NUM_PLAYERS; p++) {
        for (int i = 0; i < state->piece_count[p]; i++) {
            state->pieces[p][i].player = p;
        }
    }
}

void State_derive_grid(struct State* state)
{
    memset(state->grid, 0, sizeof(struct Piece*) * GRID_SIZE * GRID_SIZE);

    for (int p = 0; p < NUM_PLAYERS; p++) {
        for (int i = 0; i < state->piece_count[p]; i++) {
            struct Piece* piece = &state->pieces[p][i];

            // Check to see if this piece is under another piece
            // that's already on the grid
            struct Piece* p = state->grid[piece->coords.q][piece->coords.r];
            while (p && p->on_top) {
                if (p->on_top == piece) {
                    goto skip;
                }
                p = p->on_top;
            }

            state->grid[piece->coords.q][piece->coords.r] = piece;
        skip:
            continue;
        }
    }
}

void State_add_action(struct State* state, int piecei,
    const struct Coords* from, const struct Coords* to)
{
    struct Action* action = &state->actions[state->action_count++];
    action->from = *from;
    action->to = *to;

    struct Piece* piece = &state->pieces[state->turn][piecei];
    struct Piece* turn_queen = state->queens[state->turn];
    struct Piece* other_queen = state->queens[!state->turn];

    bool draw_action = false;

    // Don't allow actions that result in a loss for current player
    if (turn_queen
        && State_hex_neighbor_count(state, &turn_queen->coords) == NUM_DIRECTIONS - 1
        && Coords_adjacent(&turn_queen->coords, to)
        && (!Coords_adjacent(&turn_queen->coords, from)
            || from->q == PLACE_ACTION
            || (piece->type == BEETLE
                && state->grid[from->q][from->r]->on_top))
        && !state->grid[to->q][to->r]) {

        // ..but allow draws
        if (other_queen
            && State_hex_neighbor_count(state, &other_queen->coords) == NUM_DIRECTIONS - 1
            && Coords_adjacent(&other_queen->coords, to)
            && !(Coords_adjacent(&other_queen->coords, from)
                || (from->q == PLACE_ACTION
                    || (piece->type == BEETLE
                        && state->grid[from->q][from->r]->on_top)))) {

            draw_action = true;

        } else {
            // Remove the action from the stack and stop adding it, but
            // keep a reference to it (e.g for UHP validmoves)
            state->actions[MAX_ACTIONS - 1 - state->losing_action_count++] = *action;
            state->action_count--;
            return;
        }
    }

    if (other_queen) {
        if (!state->winning_action
            && State_hex_neighbor_count(state, &other_queen->coords) == NUM_DIRECTIONS - 1
            && Coords_adjacent(to, &other_queen->coords)
            && (from->q == PLACE_ACTION
                || (!Coords_adjacent(&other_queen->coords, from)
                    || (piece->type == BEETLE
                        && state->grid[from->q][from->r]->on_top)))
            && !state->grid[to->q][to->r]
            && !draw_action) {

            state->winning_action = action;
        }

        if (Coords_adjacent(to, &other_queen->coords)
            && (from->q == PLACE_ACTION
                || !Coords_adjacent(&other_queen->coords, from))) {

            state->queen_adjacent_actions[state->queen_adjacent_action_count++] = action;
        }
        // else if (Coords_distance(to, &other_queen->coords) <= 2
        //    && (from->q == PLACE_ACTION
        //        || Coords_distance(from, &other_queen->coords) > 2)) {

        //    state->queen_nearby_actions[state->queen_nearby_action_count++] = action;
        //}
    }

    if (from->q != PLACE_ACTION) {
        if (piece->type == QUEEN_BEE) {
            state->queen_moves[state->queen_move_count++] = action;
        } else if (piece->type == BEETLE) {
            state->beetle_moves[state->beetle_move_count++] = action;
        }
        state->piece_moves[piecei][state->piece_move_count[piecei]++] = action;
    }

    // TODO technically, a piece pinning in the center could move left
    // and right and keep the pin, but this won't detect that because the piece
    // is also a neighbor to those spots
    if (state->neighbor_count[!state->turn][action->to.q][action->to.r] == 1
        && state->neighbor_count[state->turn][action->to.q][action->to.r] == 0
        && !State_cut_point_neighbor(state, &action->to)
        // Don't bias moving an already-pinning piece
        && !(state->neighbor_count[!state->turn][action->from.q][action->from.r] == 1
            && state->neighbor_count[state->turn][action->from.q][action->from.r] == 0)) {

        state->pin_moves[state->pin_move_count++] = action;

        if (State_neighbor_piece(state, &action->to)->type == QUEEN_BEE) {
            state->queen_pin_moves[state->queen_pin_move_count++] = action;
        }
    }

    if (state->neighbor_count[state->turn][action->from.q][action->from.r] == 1
        && state->neighbor_count[!state->turn][action->from.q][action->from.r] == 0) {
        // We can't use this normal test, because it will always be a cut point
        //&& !State_cut_point_neighbor(state, &action->from)) {
        // Don't move to a new location that pins us
        // TODO
        //&& !(state->neighbor_count[state->turn][action->to.q][action->to.r] == 1
        //    && state->neighbor_count[!state->turn][action->to.q][action->to.r] == 0)
        //    && !State_cut_point_neighbor(state, &action->to)) {

        state->unpin_moves[state->unpin_move_count++] = action;
    }

    if (state->queens[state->turn]
        && action->from.q != PLACE_ACTION
        && action->from.q != PASS_ACTION
        && State_hex_neighbor_count(state, &state->queens[state->turn]->coords) == NUM_DIRECTIONS - 1
        && Coords_adjacent(&state->queens[state->turn]->coords, &action->from)
        // TODO I think this is segfaulting; shouldn't any action from have a from on the grid (if not one of the above)?
        //&& !state->grid[action->from.q][action->from.r]->on_top
        && (!Coords_adjacent(&state->queens[state->turn]->coords, &action->to)
            || state->grid[action->to.q][action->to.r])) {

        state->queen_away_moves[state->queen_away_move_count++] = action;
    }
}

void State_ant_walk(struct State* state, int piecei,
    const struct Piece* piece,
    const struct Coords* coords,
    bool crumbs[GRID_SIZE][GRID_SIZE])
{
    if (crumbs[coords->q][coords->r]) {
        return;
    }

    // If this is the root call, temporarily remove the piece from the
    // grid so it can't walk along itself; if it's not the root call,
    // add it as a move
    if (&piece->coords == coords) {
        state->grid[coords->q][coords->r] = NULL;
    } else {
        State_add_action(state, piecei, &piece->coords, coords);
    }

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
                State_ant_walk(state, piecei, piece, &c, crumbs);
            }
        }

        c = *coords;
        Coords_move(&c, Direction_rotate(d, -2));
        if (!state->grid[c.q][c.r]) {
            c = *coords;
            Coords_move(&c, Direction_rotate(d, -1));
            if (!state->grid[c.q][c.r]) {
                State_ant_walk(state, piecei, piece, &c, crumbs);
            }
        }
    }

    // Put the piece back on the grid
    if (&piece->coords == coords) {
        state->grid[coords->q][coords->r] = (struct Piece*)piece;
    }
}

void State_spider_walk(struct State* state, int piecei,
    const struct Piece* piece,
    const struct Coords* coords,
    bool crumbs[GRID_SIZE][GRID_SIZE],
    bool tos[GRID_SIZE][GRID_SIZE],
    int depth)
{
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
            State_add_action(state, piecei, &piece->coords, coords);
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
                State_spider_walk(state, piecei, piece, &c, crumbs, tos, depth + 1);
            }
        }

        c = *coords;
        Coords_move(&c, Direction_rotate(d, -2));
        if (!state->grid[c.q][c.r]) {
            c = *coords;
            Coords_move(&c, Direction_rotate(d, -1));
            if (!state->grid[c.q][c.r]) {
                State_spider_walk(state, piecei, piece, &c, crumbs, tos, depth + 1);
            }
        }
    }

    // Put the piece back on the grid
    if (depth == 0) {
        state->grid[coords->q][coords->r] = (struct Piece*)piece;
    }

    crumbs[coords->q][coords->r] = false;
}

/* Finds cut points (i.e. pieces that can't be moved due to the one hive
 * rule) using an adaptation of Hopcroft & Tarjan's algorithm.
 * See:
 * https://dl.acm.org/doi/10.1145/362248.362272
 * https://en.wikipedia.org/wiki/Biconnected_component
 */
void State_derive_cut_points(struct State* state)
{
    memset(state->cut_points, 0, sizeof(bool) * GRID_SIZE * GRID_SIZE);
    memset(state->cut_point_count, 0, sizeof(uint_fast8_t) * NUM_PLAYERS);

    if (state->piece_count[P1] <= 1 && state->piece_count[P2] <= 1) {
        return;
    }

    struct Coords stack[MAX_PIECES];
    int_fast8_t sp = 0;
    stack[sp] = state->pieces[P1][0].coords;

    uint_fast8_t depth[MAX_PIECES];
    depth[sp] = 0;

    uint_fast8_t lowpoint[MAX_PIECES];
    lowpoint[sp] = 0;

    // crumbs[q][r] is the same as depth[sp] for a particular Coords
    int_fast8_t crumbs[GRID_SIZE][GRID_SIZE];
    memset(&crumbs, -1, sizeof(int_fast8_t) * GRID_SIZE * GRID_SIZE);
    crumbs[stack[sp].q][stack[sp].r] = 0;

    uint_fast8_t next_direction[MAX_PIECES];
    next_direction[sp] = 0;

    uint_fast8_t parent_direction[MAX_PIECES];
    parent_direction[sp] = -1;

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

                depth[sp + 1] = depth[sp] + 1;
                crumbs[head.q][head.r] = depth[sp] + 1;
                lowpoint[sp + 1] = depth[sp];
                next_direction[sp + 1] = 0;
                parent_direction[sp + 1] = OPPOSITE[next_direction[sp] - 1];
                stack[sp + 1] = head;
                sp++;
            }
        } else {
            if (sp != 0) {
                if (lowpoint[sp] == depth[sp - 1] && (sp - 1 != 0)) {
                    if (!state->cut_points[stack[sp - 1].q][stack[sp - 1].r]) {
                        state->cut_points[stack[sp - 1].q][stack[sp - 1].r] = true;
                        state->cut_point_count[state->grid[stack[sp - 1].q][stack[sp - 1].r]->player]++;
                    }
                } else {
                    if (lowpoint[sp] < lowpoint[sp - 1]) {
                        lowpoint[sp - 1] = lowpoint[sp];
                    }
                }
            }
            sp--;
        }
    }

    if (root_children > 1) {
        if (!state->cut_points[stack[0].q][stack[0].r]) {
            state->cut_points[stack[0].q][stack[0].r] = true;
            state->cut_point_count[state->grid[stack[0].q][stack[0].r]->player]++;
        }
    }
}

// Counts the number of cut points each player has, without modifying the state
void State_count_cut_points(
    const struct State* state,
    const struct Coords* start_coords,
    unsigned int cut_points[NUM_PLAYERS])
{
    memset(cut_points, 0, sizeof(int) * NUM_PLAYERS);

    if (state->piece_count[P1] <= 1 && state->piece_count[P2] <= 1) {
        return;
    }

    struct Coords stack[MAX_PIECES];
    int_fast8_t sp = 0;
    stack[sp] = *start_coords;

    uint_fast8_t depth[MAX_PIECES];
    depth[sp] = 0;

    uint_fast8_t lowpoint[MAX_PIECES];
    lowpoint[sp] = 0;

    // crumbs[q][r] is the same as depth[sp] for a particular Coords
    int_fast8_t crumbs[GRID_SIZE][GRID_SIZE];
    memset(&crumbs, -1, sizeof(int_fast8_t) * GRID_SIZE * GRID_SIZE);
    crumbs[stack[sp].q][stack[sp].r] = 0;

    uint_fast8_t next_direction[MAX_PIECES];
    next_direction[sp] = 0;

    uint_fast8_t parent_direction[MAX_PIECES];
    parent_direction[sp] = -1;

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

                depth[sp + 1] = depth[sp] + 1;
                crumbs[head.q][head.r] = depth[sp] + 1;
                lowpoint[sp + 1] = depth[sp];
                next_direction[sp + 1] = 0;
                parent_direction[sp + 1] = OPPOSITE[next_direction[sp] - 1];
                stack[sp + 1] = head;
                sp++;
            }
        } else {
            if (sp != 0) {
                if (lowpoint[sp] == depth[sp - 1] && (sp - 1 != 0)) {
                    cut_points[state->grid[stack[sp - 1].q][stack[sp - 1].r]->player]++;
                } else {
                    if (lowpoint[sp] < lowpoint[sp - 1]) {
                        lowpoint[sp - 1] = lowpoint[sp];
                    }
                }
            }
            sp--;
        }
    }

    if (root_children > 1) {
        cut_points[state->grid[stack[0].q][stack[0].r]->player]++;
    }
}

void State_derive_piece_pointers(struct State* state)
{
    for (int p = 0; p < NUM_PLAYERS; p++) {
        state->queens[p] = NULL;
        state->beetle_count[p] = 0;
        for (int i = 0; i < state->piece_count[p]; i++) {
            if (state->pieces[p][i].type == QUEEN_BEE) {
                state->queens[p] = &state->pieces[p][i];
            }

            if (state->pieces[p][i].type == BEETLE) {
                state->beetles[p][state->beetle_count[p]++] = &state->pieces[p][i];
            }
        }
    }
}

void State_derive_hands(struct State* state)
{
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

void State_add_neighor_count(struct State* state,
    const struct Coords* coords,
    enum Player player,
    int x)
{
    for (int d = 0; d < NUM_DIRECTIONS; d++) {
        struct Coords c = *coords;
        Coords_move(&c, d);
        state->neighbor_count[player][c.q][c.r] += x;
    }
}

void State_derive_neighbor_count(struct State* state)
{
    memset(state->neighbor_count,
        0,
        sizeof(uint_fast8_t) * NUM_PLAYERS * GRID_SIZE * GRID_SIZE);

    for (int p = 0; p < NUM_PLAYERS; p++) {
        for (int i = 0; i < state->piece_count[p]; i++) {
            struct Piece* piece = &state->pieces[p][i];

            if (piece->on_top) {
                continue;
            }

            State_add_neighor_count(state, &piece->coords, piece->player, 1);
        }
    }
}

void State_derive_result(struct State* state)
{
    state->result = NO_RESULT;
    for (int p = 0; p < NUM_PLAYERS; p++) {
        if (!state->queens[p]) {
            continue;
        }
        struct Piece* queen = state->queens[p];

        int neighbors = State_hex_neighbor_count(state, &queen->coords);
        if (neighbors == 6) {
            switch (state->result) {
            case P1_WIN:
            case P2_WIN:
                state->result = DRAW;
                break;
            default:
                if (p == P1) {
                    state->result = P2_WIN;
                } else {
                    state->result = P1_WIN;
                }
                break;
            }
        }
    }
}

void State_derive_piece_moves(struct State* state, int piecei)
{
    struct Piece* piece = &state->pieces[state->turn][piecei];
    struct Coords* coords = &piece->coords;

    // A piece can't move if it's under another piece
    if (piece->on_top) {
        return;
    }

    // One hive rule
    if (state->cut_points[piece->coords.q][piece->coords.r]) {
        // Beetles on top of the hive ignore one hive rule
        if (piece->type != BEETLE || state->grid[coords->q][coords->r] == piece) {
            return;
        }
    }

    bool crumbs[GRID_SIZE][GRID_SIZE];
    switch (piece->type) {
    case ANT:
        memset(crumbs, 0, sizeof(bool) * GRID_SIZE * GRID_SIZE);
        State_ant_walk(state, piecei, piece, coords, crumbs);
        break;

    case BEETLE: {
        bool on_top = state->grid[coords->q][coords->r] != piece;

        if (on_top) {
            for (int d = 0; d < NUM_DIRECTIONS; d++) {
                struct Coords c = *coords;
                int from_height = State_height_at(state, &c);

                Coords_move(&c, d);
                int to_height = State_height_at(state, &c);

                int move_height = from_height > to_height ? from_height : to_height;

                struct Coords test = *coords;
                int test_height;
                Coords_move(&test, Direction_rotate(d, 1));
                test_height = State_height_at(state, &test);

                if (move_height >= test_height) {
                    goto can_move_on_top;
                }

                test = *coords;
                Coords_move(&test, Direction_rotate(d, -1));
                test_height = State_height_at(state, &test);

                if (move_height >= test_height) {
                    goto can_move_on_top;
                }

                continue;

            can_move_on_top:
                State_add_action(state, piecei, &piece->coords, &c);
            }

            break;
        }

        for (int d = 0; d < NUM_DIRECTIONS; d++) {
            // Look for adjacent pieces
            struct Coords c = *coords;
            Coords_move(&c, d);
            if (!state->grid[c.q][c.r]) {
                continue;
            }

            // Climb on top of adjacent piece
            int move_height = State_height_at(state, &c);

            struct Coords test = *coords;
            int test_height;
            Coords_move(&test, Direction_rotate(d, 1));
            test_height = State_height_at(state, &test);

            if (move_height >= test_height) {
                goto can_move_climb;
            }

            test = *coords;
            Coords_move(&test, Direction_rotate(d, -1));
            test_height = State_height_at(state, &test);

            if (move_height >= test_height) {
                goto can_move_climb;
            }

            continue;

        can_move_climb:
            // TODO what is this doing?
            State_add_action(state, piecei, &piece->coords, &c);

            // For every adjacent piece, try to move to the
            // right and left of it, first checking for
            // freedom to move
            c = *coords;
            Coords_move(&c, Direction_rotate(d, 2));
            if (!state->grid[c.q][c.r]) {
                c = *coords;
                Coords_move(&c, Direction_rotate(d, 1));
                if (!state->grid[c.q][c.r]) {
                    State_add_action(state, piecei, &piece->coords, &c);
                }
            }
            c = *coords;
            Coords_move(&c, Direction_rotate(d, -2));
            if (!state->grid[c.q][c.r]) {
                c = *coords;
                Coords_move(&c, Direction_rotate(d, -1));
                if (!state->grid[c.q][c.r]) {
                    State_add_action(state, piecei, &piece->coords, &c);
                }
            }
        }
        break;
    }

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

            State_add_action(state, piecei, &piece->coords, &c);
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
                    State_add_action(state, piecei, &piece->coords, &c);
                }
            }
            c = *coords;
            Coords_move(&c, Direction_rotate(d, -2));
            if (!state->grid[c.q][c.r]) {
                c = *coords;
                Coords_move(&c, Direction_rotate(d, -1));
                if (!state->grid[c.q][c.r]) {
                    State_add_action(state, piecei, &piece->coords, &c);
                }
            }
        }
        break;

    case SPIDER:
        memset(crumbs, 0, sizeof(bool) * GRID_SIZE * GRID_SIZE);
        bool tos[GRID_SIZE][GRID_SIZE];
        memset(tos, 0, sizeof(bool) * GRID_SIZE * GRID_SIZE);
        State_spider_walk(state, piecei, piece, coords, crumbs, tos, 0);
        break;
    }
}

void State_derive_actions(struct State* state)
{
    state->action_count = 0;
    for (int i = 0; i < state->piece_count[state->turn]; i++) {
        state->piece_move_count[i] = 0;
    }
    state->queen_move_count = 0;
    state->queen_away_move_count = 0;
    state->queen_adjacent_action_count = 0;
    state->queen_nearby_action_count = 0;
    state->pin_move_count = 0;
    state->unpin_move_count = 0;
    state->queen_pin_move_count = 0;
    state->beetle_move_count = 0;

    state->winning_action = NULL;
    state->losing_action_count = 0;

    if (state->result != NO_RESULT) {
        return;
    }

    // P1 start actions
    if (state->piece_count[P1] == 0) {
        for (int t = 0; t < NUM_PIECETYPES; t++) {
            if (t == QUEEN_BEE)
                continue;

            state->actions[state->action_count].from.q = PLACE_ACTION;
            state->actions[state->action_count].from.r = t;
            state->actions[state->action_count].to.q = 0;
            state->actions[state->action_count++].to.r = 0;
        }

        return;
    }
    // P2 start actions
    if (state->piece_count[P2] == 0) {
        struct Coords* p1_piece_coords = &state->pieces[P1][0].coords;
        for (int t = 0; t < NUM_PIECETYPES; t++) {
            if (t == QUEEN_BEE)
                continue;

            for (int d = 0; d < NUM_DIRECTIONS; d++) {
                state->actions[state->action_count].from.q = PLACE_ACTION;
                state->actions[state->action_count].from.r = t;
                state->actions[state->action_count].to = *p1_piece_coords;
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

    bool force_queen_place = state->hands[state->turn][QUEEN_BEE] && state->piece_count[state->turn] >= 3;

    if (pieces_to_place) {
        struct Coords place_coords[MAX_PLACE_SPOTS];
        int place_coords_count = 0;
        bool place_crumbs[GRID_SIZE][GRID_SIZE];
        memset(place_crumbs, 0, sizeof(bool) * GRID_SIZE * GRID_SIZE);
        for (int i = 0; i < state->piece_count[state->turn]; i++) {
            for (int d = 0; d < NUM_DIRECTIONS; d++) {
                struct Coords coords = state->pieces[state->turn][i].coords;
                Coords_move(&coords, d);

                if (!state->grid[coords.q][coords.r] && state->neighbor_count[!state->turn][coords.q][coords.r] == 0 && !place_crumbs[coords.q][coords.r]) {
                    place_coords[place_coords_count++] = coords;
                    place_crumbs[coords.q][coords.r] = true;
                }
            }
        }
        for (int t = 0; t < NUM_PIECETYPES; t++) {
            if (state->hands[state->turn][t] == 0)
                continue;
            if (force_queen_place && t != QUEEN_BEE)
                continue;

            for (int i = 0; i < place_coords_count; i++) {
                struct Coords from;
                from.q = PLACE_ACTION;
                from.r = t;
                State_add_action(state, 0, &from, &place_coords[i]);
            }
        }
    }

    // Moves

    // A player can't move until their queen is placed
    if (state->hands[state->turn][QUEEN_BEE]) {
        return;
    }

    for (int i = 0; i < state->piece_count[state->turn]; i++) {
        State_derive_piece_moves(state, i);
    }

    if (state->action_count == 0) {
        state->actions[0].from.q = PASS_ACTION;
        state->action_count = 1;
    }
}

/* Note on efficiency:
 * State_derive is not guranteed to be efficient, and is intended as a
 * convenient one-stop function for code without efficiency needs. It's
 * assumed optimized code will understand the technical details of the
 * State structure and granularly derive (or manually manage) as needed.
 */
void State_derive(struct State* state)
{
    State_derive_piece_players(state);
    State_derive_grid(state);
    State_derive_cut_points(state);
    State_derive_piece_pointers(state);
    State_derive_hands(state);
    State_derive_neighbor_count(state);
    State_derive_result(state);
    State_derive_actions(state);
}

void State_copy(const struct State* source, struct State* dest)
{
    // TODO If we're deriving everything we can, we only need to copy
    // core information
    memcpy(dest, source, sizeof(struct State));

    // Transfer on_top pointers
    for (int p = 0; p < NUM_PLAYERS; p++) {
        for (int i = 0; i < source->piece_count[p]; i++) {
            const struct Piece* source_piece = &source->pieces[p][i];
            if (!source_piece->on_top) {
                continue;
            }

            struct Piece* dest_piece = &dest->pieces[p][i];
            struct Piece* source_on_top = source_piece->on_top;
            for (int j = 0; j < source->piece_count[source_on_top->player]; j++) {
                if (&source->pieces[source_on_top->player][j] == source_on_top) {
                    dest_piece->on_top = &dest->pieces[source_on_top->player][j];
                    break;
                }
            }
        }
    }

    // TODO We can optimize this by only deriving what we need to
    State_derive(dest);
}

void State_new(struct State* state)
{
    memset(state, 0, sizeof(struct State));
    State_derive(state);
}

void State_act(struct State* state, const struct Action* action)
{
#ifdef CHECK_ACTIONS
    bool valid_action = false;
    for (int i = 0; i < state->action_count; i++) {
        if (!memcmp(&state->actions[i], action, sizeof(struct Action))) {
            valid_action = true;
            break;
        }
    }
    if (!valid_action) {
        exit(ERROR_ILLEGAL_ACTION);
    }
#endif

    // Pass action
    if (action->from.q == PASS_ACTION) {
        state->turn = !state->turn;
        State_derive_actions(state);
        return;
    }

    struct Piece* piece;

    // Place action
    if (action->from.q == PLACE_ACTION) {
        piece = &state->pieces[state->turn][state->piece_count[state->turn]];
        piece->type = action->from.r;
        piece->coords.q = action->to.q;
        piece->coords.r = action->to.r;
        piece->player = state->turn;

        state->grid[action->to.q][action->to.r] = piece;
        if (piece->type == QUEEN_BEE) {
            state->queens[state->turn] = piece;
        } else if (piece->type == BEETLE) {
            state->beetles[state->turn][state->beetle_count[state->turn]++] = piece;
        }

        state->piece_count[state->turn]++;
        state->hands[state->turn][piece->type]--;

        for (int d = 0; d < NUM_DIRECTIONS; d++) {
            struct Coords coords = piece->coords;
            Coords_move(&coords, d);
            state->neighbor_count[piece->player][coords.q][coords.r] += 1;
        }

        state->turn = !state->turn;
        State_derive_cut_points(state);
        State_derive_actions(state);
        return;
    }

    // Move action

    // Find piece being moved, walking up a stack of pieces if necessary
    piece = state->grid[action->from.q][action->from.r];
    struct Piece* under = NULL;
    while (piece->on_top) {
        under = piece;
        piece = piece->on_top;
    }

    // Move piece to new location
    piece->coords.q = action->to.q;
    piece->coords.r = action->to.r;
    State_add_neighor_count(state, &action->from, piece->player, -1);
    State_add_neighor_count(state, &action->to, piece->player, 1);

    if (under) {
        under->on_top = NULL;
        State_add_neighor_count(state, &action->from, under->player, 1);
    } else {
        state->grid[action->from.q][action->from.r] = NULL;
    }

    // If there's already a piece at new location, put moved piece on top of it
    if (state->grid[action->to.q][action->to.r]) {
        struct Piece* p = state->grid[action->to.q][action->to.r];
        while (p->on_top) {
            p = p->on_top;
        }
        p->on_top = piece;
        State_add_neighor_count(state, &action->to, p->player, -1);
    } else {
        state->grid[action->to.q][action->to.r] = piece;
    }

    state->turn = !state->turn;
    State_derive_result(state);
    if (state->result == NO_RESULT) {
        // TODO don't need to do this for beetle moves on hive
        State_derive_cut_points(state);
    }
    State_derive_actions(state);
}

int State_hex_neighbor_count(const struct State* state, const struct Coords* coords)
{
    return state->neighbor_count[P1][coords->q][coords->r]
        + state->neighbor_count[P2][coords->q][coords->r];
}
