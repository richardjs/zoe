#ifndef SIMULATE_H
#define SIMULATE_H

#include "mcts.h"
#include "state.h"

float State_simulate(struct State* state,
    const struct MCTSOptions* options, struct MCTSStats* stats);

#endif
