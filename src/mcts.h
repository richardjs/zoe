#ifndef MCTS_H
#define MCTS_H

#include "state.h"

#define DEFAULT_ITERATIONS 100000
#define DEFAULT_MAX_SIM_DEPTH 300
#define DEFAULT_UCTC .3
#define DEFAULT_SAVE_TREE false

#define DEFAULT_QUEEN_SIDESTEP_BIAS 0
#define DEFAULT_QUEEN_ADJACENT_ACTION_BIAS 0.65

struct Node {
    bool expanded;
    unsigned int visits;
    float value;

    struct Node** children;
    uint_fast16_t children_count;

    uint16_t depth;
};

struct MCTSOptions {
    uint64_t iterations;
    float uctc;
    uint16_t max_sim_depth;
    bool save_tree;

    float queen_sidestep_bias;
    float queen_adjacent_action_bias;
};

struct MCTSStats {
    uint64_t iterations;
    uint64_t nodes;
    uint64_t tree_bytes;
    uint16_t tree_depth;
    uint32_t simulations;
    float mean_sim_depth;
    uint32_t depth_outs;
    uint64_t duration;
};

struct MCTSResults {
    uint16_t actioni;
    float score;
    struct MCTSStats stats;
    struct Node nodes[MAX_ACTIONS];
    struct Node* tree;
};

void MCTSOptions_default(struct MCTSOptions*);

void mcts(const struct State*, struct MCTSResults*, const struct MCTSOptions*);

#endif
