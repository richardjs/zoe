#include <stdlib.h>

#include "mcts.h"
#include "state.h"

#ifdef WATCH_SIMS
#include "stateio.h"
#include <stdio.h>
#endif

int State_beetle_seek_path(
    const struct State* state,
    const struct Piece* piece,
    struct Coords* path)
{
    if (!state->queens[!piece->player]) {
        return -1;
    }
    struct Coords target = state->queens[!piece->player]->coords;

    struct Coords queue[MAX_PIECES];
    unsigned int queue_size = 0;
    queue[queue_size++] = piece->coords;

    struct Coords crumbs[GRID_SIZE][GRID_SIZE];
    for (int q = 0; q < GRID_SIZE; q++) {
        for (int r = 0; r < GRID_SIZE; r++) {
            crumbs[q][r].q = GRID_SIZE + 1;
        }
    }

    struct Coords coords;
    bool found = false;
    while (queue_size) {
        coords = queue[--queue_size];

        if (coords.q == target.q && coords.r == target.r) {
            found = true;
            break;
        }

        for (enum Direction d = 0; d < NUM_DIRECTIONS; d++) {
            struct Coords neighbor = coords;
            Coords_move(&neighbor, d);

            if (piece->coords.q == neighbor.q && piece->coords.r == neighbor.r) {
                continue;
            }

            if (!state->grid[neighbor.q][neighbor.r]) {
                continue;
            }

            if (crumbs[neighbor.q][neighbor.r].q != GRID_SIZE + 1) {
                continue;
            }
            crumbs[neighbor.q][neighbor.r] = coords;
            queue[queue_size++] = neighbor;
        }
    }

    if (!found) {
        return -1;
    }

    int path_size = 0;
    while (coords.q != piece->coords.q && coords.r != piece->coords.r) {
        path[path_size++] = coords;
        coords = crumbs[coords.q][coords.r];
    }

    return path_size;
}

bool State_cut_point_neighbor(
    const struct State* state,
    const struct Coords* coords);

bool State_is_queen_sidestep(const struct State* state, const struct Action* action)
{
    return state->neighbor_count[!state->turn][action->to.q][action->to.r] == 1
        && state->neighbor_count[state->turn][action->to.q][action->to.r] == 1
        // Not a sidestep if we're already in a similar position
        && State_hex_neighbor_count(state, &action->from) > 1;
}

/**
 * simulates play (in place) on a state, stopping at game end or
 * MAX_SIM_DEPTH, and returns 1.0 if the initial turn won, -1.0 if it
 * lost, and 0.0 on a draw or depth out
 */
float State_simulate(struct State* state,
    const struct MCTSOptions* options, struct MCTSStats* stats)
{
    stats->simulations++;

    enum Player original_turn = state->turn;
    int original_cut_point_diff = state->cut_point_count[!original_turn] - state->cut_point_count[original_turn];

    int depth = 0;
    while (state->result == NO_RESULT) {
        if (state->winning_action) {
            State_act(state, state->winning_action);
            continue;
        }

        int cut_point_diff = state->cut_point_count[!original_turn] - state->cut_point_count[original_turn];
        int cut_point_diff_change = cut_point_diff - original_cut_point_diff;
        if (cut_point_diff_change >= options->cut_point_diff_terminate) {
            stats->cut_point_terminations++;
            // TODO correct sign?
            return -DEFAULT_CUT_POINT_DIFF_TERM_VALUE;
        } else if (cut_point_diff_change <= -options->cut_point_diff_terminate) {
            stats->cut_point_terminations++;
            // TODO correct sign?
            return DEFAULT_CUT_POINT_DIFF_TERM_VALUE;
        }

        if (depth++ > options->max_sim_depth) {
            stats->depth_outs++;
            return 0.0;
        }

    select_action : {

        struct Action* action = NULL;

        if (options->queen_sidestep_bias && state->queen_move_count) {
            struct Action* actions[MAX_QUEEN_MOVES];
            int action_count = 0;
            for (int i = 0; i < state->queen_move_count; i++) {
                struct Action* action = state->queen_moves[i];
                if (State_is_queen_sidestep(state, action)) {
                    actions[action_count++] = action;
                }
            }

            if (action_count
                && (rand() / (float)RAND_MAX) < options->queen_sidestep_bias) {
                action = actions[rand() % action_count];
#ifdef WATCH_SIMS
                printf("queen sidestep\n");
#endif
                goto action_selected;
            }
        }

        if (state->queen_away_move_count
            && (rand() / (float)RAND_MAX) < options->queen_away_move_bias) {
            action = state->queen_away_moves[rand() % state->queen_away_move_count];
#ifdef WATCH_SIMS
            printf("queen away move\n");
#endif
            goto action_selected;
        }

        if (state->queen_pin_move_count
            && (rand() / (float)RAND_MAX) < options->queen_pin_move_bias) {
            action = state->queen_pin_moves[rand() % state->queen_pin_move_count];
#ifdef WATCH_SIMS
            printf("queen pin move\n");
#endif
            goto action_selected;
        }

        if (state->beetle_move_count
            && (rand() / (float)RAND_MAX) < options->beetle_seek_move_bias) {
            struct Piece* beetle = state->beetles[state->turn][rand() % state->beetle_count[state->turn]];
            struct Coords path[MAX_PIECES];
            int path_size = State_beetle_seek_path(state, beetle, path);

            if (path_size > 1) {
                for (int i = 0; i < state->beetle_move_count; i++) {
                    if (state->beetle_moves[i]->to.q == path[path_size - 1].q
                        && state->beetle_moves[i]->to.r == path[path_size - 1].r) {

                        action = state->beetle_moves[i];
#ifdef WATCH_SIMS
                        printf("beetle seek move\n");
#endif
                        goto action_selected;
                    }
                }
            }
        }

        if (state->pin_move_count
            && (rand() / (float)RAND_MAX) < options->pin_move_bias) {
            action = state->pin_moves[rand() % state->pin_move_count];
#ifdef WATCH_SIMS
            printf("pin move\n");
#endif
            goto action_selected;
        }

        // TODO only do this if the queen is pinned?
        if (state->queen_adjacent_action_count
            && (rand() / (float)RAND_MAX) < options->queen_adjacent_action_bias) {
            action = state->queen_adjacent_actions[rand() % state->queen_adjacent_action_count];
#ifdef WATCH_SIMS
            printf("queen adjacent action\n");
#endif
            goto action_selected;
        }

        if (state->unpin_move_count
            && (rand() / (float)RAND_MAX) < options->unpin_move_bias) {
            action = state->unpin_moves[rand() % state->unpin_move_count];
#ifdef WATCH_SIMS
            printf("unpin move\n");
#endif
            goto action_selected;
        }

        if (state->queen_nearby_action_count
            && (rand() / (float)RAND_MAX) < options->queen_nearby_action_bias) {
            action = state->queen_nearby_actions[rand() % state->queen_nearby_action_count];
#ifdef WATCH_SIMS
            printf("queen nearby action\n");
#endif
            goto action_selected;
        }

        if (state->beetle_move_count
            && (rand() / (float)RAND_MAX) < options->beetle_move_bias) {
            action = state->beetle_moves[rand() % state->beetle_move_count];
#ifdef WATCH_SIMS
            printf("beetle move\n");
#endif
            goto action_selected;
        }

#ifdef WATCH_SIMS
        printf("random\n");
#endif
        action = &state->actions[rand() % state->action_count];

    action_selected:
#ifdef WATCH_SIMS
        Action_print(action, stderr);
#endif

        if (action->from.q != PLACE_ACTION
            && action->from.q != PASS_ACTION
            && state->queens[!state->turn]
            && Coords_adjacent(&action->from, &state->queens[!state->turn]->coords)
            && (rand() / (float)RAND_MAX) < options->from_queen_pass) {
#ifdef WATCH_SIMS
            printf("from queen pass\n");
#endif
            goto select_action;
        }

        if (state->neighbor_count[state->turn][action->to.q][action->to.r] == 1
            && state->neighbor_count[!state->turn][action->to.q][action->to.r] == 0
            && !State_cut_point_neighbor(state, &action->to)
            // Make sure this isn't a beetle-on-hive move
            && !state->grid[action->to.q][action->to.r]
            && (rand() / (float)RAND_MAX) < options->own_pin_pass) {
#ifdef WATCH_SIMS
            printf("own pin pass\n");
#endif
            goto select_action;
        }

        State_act(state, action);

#ifdef WATCH_SIMS
        State_print(state, stderr);
        char state_string[STATE_STRING_SIZE];
        State_to_string(state, state_string);
        printf("%s\n", state_string);
        getchar();
#endif
    }
    }

    stats->mean_sim_depth += (depth - stats->mean_sim_depth) / stats->simulations;

    if (state->result == DRAW) {
        return 0.0;
    }

    if ((state->result == P1_WIN && original_turn == P1)
        || (state->result == P2_WIN && original_turn == P2)) {
        return 1.0;
    }

    return -1.0;
}
