#ifndef SIMULATE_H
#define SIMULATE_H

#include "mcts.h"
#include "state.h"

float State_simulate(struct State* state,
    const struct MCTSOptions* options, struct MCTSStats* stats);

bool State_is_queen_sidestep(const struct State* state,
    const struct Action* action);

#endif
