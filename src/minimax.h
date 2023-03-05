#ifndef MINIMAX_H
#define MINIMAX_H

#include <stdint.h>

#include "state.h"

#define DEFAULT_MINIMAX_DEPTH 3

struct MinimaxOptions {
  int depth;
};

struct MinimaxStats {
  uint64_t nodes;
  uint64_t leaves;
};

struct MinimaxResults {
  uint16_t actioni;
  float score;
  struct MinimaxStats stats;
};

void MinimaxOptions_default(struct MinimaxOptions *);

void minimax(const struct State *state, struct MinimaxResults *r,
             const struct MinimaxOptions *o);

#endif
