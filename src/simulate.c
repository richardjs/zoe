#include <stdlib.h>

#include "mcts.h"
#include "state.h"

bool State_is_queen_sidestep(const struct State* state, const struct Action* action)
{
    return state->neighbor_count[!state->turn][action->to.q][action->to.r] == 1
        && state->neighbor_count[state->turn][action->to.q][action->to.r] == 1;
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

    //int last_queen_move_count[NUM_PLAYERS] = {-1, -1};
    //int last_movable_ants[NUM_PLAYERS] = {-1, -1};

    int depth = 0;
    while (state->result == NO_RESULT) {
        if (state->winning_action) {
            State_act(state, state->winning_action);
            continue;
        }

        int moveable_ants = state->hands[state->turn][ANT];
        for (int i = 0; i < state->piece_count[state->turn]; i++) {
            struct Piece *piece = &state->pieces[state->turn][i];
            if (piece->type != ANT) continue;

            if (state->piece_move_count[i]) {
                moveable_ants++;
            }
        }
        if (moveable_ants == 0) {
            return -1.0;
        }

        if (state->cut_point_count[!original_turn] - state->cut_point_count[original_turn] >= options->cut_point_diff_terminate) {
            stats->cut_point_terminations++;
            return 1.0;
        }
        if (state->cut_point_count[!original_turn] - state->cut_point_count[original_turn] <= -options->cut_point_diff_terminate) {
            stats->cut_point_terminations++;
            return -1.0;
        }

        if (depth++ > options->max_sim_depth) {
            stats->depth_outs++;
            return 0.0;
        }

        struct Action* action = NULL;
        while (1) {
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
                    goto action_chosen;
                }
            }

            if (state->queen_adjacent_action_count
                && (rand() / (float)RAND_MAX) < options->queen_adjacent_action_bias) {
                action = state->queen_adjacent_actions[rand() % state->queen_adjacent_action_count];
                goto action_chosen;
            }

            if (state->beetle_move_count
                && (rand() / (float)RAND_MAX) < options->beetle_move_bias) {
                action = state->beetle_moves[rand() % state->beetle_move_count];
                goto action_chosen;
            }

            action = &state->actions[rand() % state->action_count];

            struct State next;
            State_copy(state, &next);
            State_act(&next, action);

            if (next.cut_point_count[!state->turn] - next.cut_point_count[state->turn]
                    <= state->cut_point_count[!state->turn] - state->cut_point_count[state->turn]
                && rand() / (float)RAND_MAX < .9) {
                continue;
            }

        action_chosen:

            State_act(state, action);
            break;
        }

        //if (state->queens[state->turn]) {
        //    last_queen_move_count[state->turn] = state->queen_move_count;
        //}

        //pick_action:
        //bool queen_adjacent_action = false;
        //if (state->queen_adjacent_action_count
        //    && (rand() / (float)RAND_MAX) < options->queen_adjacent_action_bias) {
        //    action = state->queen_adjacent_actions[rand() % state->queen_adjacent_action_count];
        //    queen_adjacent_action = true;
        //} else {
        //    action = &state->actions[rand() % state->action_count];
        //}

        //struct State next;
        //State_copy(state, &next);
        //State_act(&next, action);

        //if (state->winning_action
        //    && rand() / (float)RAND_MAX < .95) {
        //    goto pick_action;
        //}

        //if (next.cut_point_count[!state->turn] < state->cut_point_count[!state->turn]
        //    && !queen_adjacent_action
        //    && rand() / (float)RAND_MAX < .5) {
        //    goto pick_action;
        //}

        //if (last_queen_move_count[next.turn] >= 0
        //    && next.queen_move_count > last_queen_move_count[next.turn]
        //    && rand() / (float)RAND_MAX < .9) {
        //    goto pick_action;
        //}

        // State_act(state, action);
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
