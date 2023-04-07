#ifndef MCTS_H
#define MCTS_H

#include "state.h"

#define DEFAULT_ITERATIONS 50000
#define DEFAULT_MAX_SIM_DEPTH 300
#define DEFAULT_UCTC .4
#define DEFAULT_SAVE_TREE false

// #define DEFAULT_PLACE_BIAS .9
#define DEFAULT_QUEEN_PIN_BIAS .9
#define DEFAULT_QUEEN_SIDESTEP_BIAS 0.9
// #define DEFAULT_FROM_OWN_QUEEN_BIAS .9
#define DEFAULT_PIN_BIAS 0.3
#define DEFAULT_QUEEN_ADJACENT_ACTION_BIAS 0.65
#define DEFAULT_QUEEN_NEARBY_ACTION_BIAS 0.0
#define DEFAULT_BEETLE_MOVE_BIAS 0.1
// #define DEFAULT_PIN_LEAVE_PASS 0.8
#define DEFAULT_OWN_PIN_PASS .1
#define DEFAULT_FROM_QUEEN_PASS .9

#define DEFAULT_CUT_POINT_DIFF_TERM 7

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
    float queen_pin_move_bias;
    float queen_adjacent_action_bias;
    float queen_nearby_action_bias;
    float beetle_move_bias;
    float pin_move_bias;
    float from_queen_pass;
    float own_pin_pass;
    int cut_point_diff_terminate;
};

struct MCTSStats {
    uint64_t iterations;
    uint64_t nodes;
    uint64_t tree_bytes;
    uint16_t tree_depth;
    uint32_t simulations;
    float mean_sim_depth;
    uint32_t cut_point_terminations;
    uint32_t depth_outs;
    uint64_t duration;
    uint32_t change_iterations;
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
