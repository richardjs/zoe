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

    int depth = 0;
    while (state->result == NO_RESULT) {
        if (state->winning_action) {
            State_act(state, state->winning_action);
            continue;
        }

        if (depth++ > options->max_sim_depth) {
            stats->depth_outs++;
            return 0.0;
        }

        // select_action:

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
                goto action_selected;
            }
        }

        if (state->queen_adjacent_action_count
            && (rand() / (float)RAND_MAX) < options->queen_adjacent_action_bias) {
            action = state->queen_adjacent_actions[rand() % state->queen_adjacent_action_count];
            goto action_selected;
        }

        if (state->beetle_move_count
            && (rand() / (float)RAND_MAX) < options->beetle_move_bias) {
            action = state->beetle_moves[rand() % state->beetle_move_count];
            goto action_selected;
        }

        action = &state->actions[rand() % state->action_count];

    action_selected:
        State_act(state, action);
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
