#include "mcts.h"
#include "state.h"

void think(
    const struct State* state,
    struct MCTSResults* results,
    const struct MCTSOptions* options,
    int workers);
