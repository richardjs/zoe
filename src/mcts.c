#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "mcts.h"
#include "simulate.h"
#include "state.h"

// store these globally so we don't have to pass them around
static struct MCTSOptions options;
static struct MCTSResults* results;

/**
 * mallocs, checks for null, and increases results.stats.tree_bytes
 */
void* mctsmalloc(size_t size)
{
    void* ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "ERROR: failure to malloc in MCTS\n");
        exit(1);
    }
    results->stats.tree_bytes += size;
    return ptr;
}

void MCTSOptions_default(struct MCTSOptions* o)
{
    o->iterations = DEFAULT_ITERATIONS;
    o->uctc = DEFAULT_UCTC;
    o->max_sim_depth = DEFAULT_MAX_SIM_DEPTH;
    o->save_tree = DEFAULT_SAVE_TREE;

    o->queen_sidestep_bias = DEFAULT_QUEEN_SIDESTEP_BIAS;
    o->queen_pin_move_bias = DEFAULT_QUEEN_PIN_BIAS;
    o->pin_move_bias = DEFAULT_PIN_BIAS;
    o->queen_adjacent_action_bias = DEFAULT_QUEEN_ADJACENT_ACTION_BIAS;
    o->unpin_move_bias = DEFAULT_UNPIN_MOVE_BIAS;
    o->queen_nearby_action_bias = DEFAULT_QUEEN_NEARBY_ACTION_BIAS;
    o->beetle_move_bias = DEFAULT_BEETLE_MOVE_BIAS;
    o->from_queen_pass = DEFAULT_FROM_QUEEN_PASS;
    o->own_pin_pass = DEFAULT_OWN_PIN_PASS;
    o->cut_point_diff_terminate = DEFAULT_CUT_POINT_DIFF_TERM;
    o->cut_point_diff_terminate_value = DEFAULT_CUT_POINT_DIFF_TERM_VALUE;
}

void Node_init(struct Node* node, uint8_t depth)
{
    node->expanded = false;
    node->visits = 0;
    node->value = 0;
    // TODO we probably could pass this around mcts() and iterate()
    // instead of storing it here
    node->depth = depth;

    results->stats.nodes++;
    if (depth > results->stats.tree_depth) {
        results->stats.tree_depth = depth;
    }
}

/**
 * allocates space for children pointers and the child nodes themselves,
 * and calls Node_init on each child
 */
void Node_expand(struct Node* node, const struct State* state)
{
    node->children_count = state->action_count;
    node->children = mctsmalloc(sizeof(struct Node*) * node->children_count);

    for (int i = 0; i < state->action_count; i++) {
        node->children[i] = mctsmalloc(sizeof(struct Node));
        Node_init(node->children[i], node->depth + 1);
    }

    node->expanded = true;
}

/**
 * frees the node's children along with the node
 */
void Node_free(struct Node* node)
{
    if (node->expanded) {
        for (int i = 0; i < node->children_count; i++) {
            Node_free(node->children[i]);
        }
        free(node->children);
    }

    free(node);
}

/**
 * single MCTS iteration: recursively walk down tree with state
 * (choosing promising children), simulate when we get to the end of the
 * tree, and update visited nodes with the results
 */
float iterate(struct Node* root, struct State* state)
{
    // Treat a state that has a winning moves as game-terminal
    if (state->winning_action) {
        root->visits++;
        root->value += 1.0;
        return 1.0;
    }

    if (state->result == DRAW) {
        return 0.0;
    }

    if (!root->expanded) {
        Node_expand(root, state);
    }

    if (root->visits == 0) {
        float score = State_simulate(state, &options, &results->stats);

        root->visits++;
        root->value += score;
        return score;
    }

    int childi = 0;
    float best_uct = -INFINITY;
    for (int i = 0; i < state->action_count; i++) {
        if (root->children[i]->visits == 0) {
            childi = i;
            break;
        }

        float uct = -1 * root->children[i]->value / root->children[i]->visits + options.uctc * sqrtf(logf(root->visits) / root->children[i]->visits);

        if (uct >= best_uct) {
            best_uct = uct;
            childi = i;
        }
    }

    struct Node* child = root->children[childi];
    State_act(state, &state->actions[childi]);

    float score = -1 * iterate(child, state);

    root->visits++;
    root->value += score;
    return score;
}

void mcts(const struct State* state,
    struct MCTSResults* r,
    const struct MCTSOptions* o)
{
    results = r;
    memset(results, 0, sizeof(struct MCTSResults));

    if (o == NULL) {
        MCTSOptions_default(&options);
    } else {
        options = *o;
    }

    if (state->action_count == 0) {
        fprintf(stderr, "Can't run MCTS on state with no actions\n");
        return;
    }

    struct Node* root = mctsmalloc(sizeof(struct Node));
    Node_init(root, 0);
    Node_expand(root, state);

    struct timeval start;
    gettimeofday(&start, NULL);

    int last_actioni = -1;
    for (int i = 0; i < options.iterations; i++) {
        struct State s;
        State_copy(state, &s);
        iterate(root, &s);
        results->stats.iterations++;

        results->score = -INFINITY;
        for (int a = 0; a < state->action_count; a++) {
            float score = -1 * root->children[a]->value / root->children[a]->visits;

            if (score >= results->score) {
                results->score = score;
                results->actioni = a;
            }
        }

        if (last_actioni != results->actioni) {
            results->stats.change_iterations = i + 1;
        }
        last_actioni = results->actioni;
    }

    struct timeval end;
    gettimeofday(&end, NULL);
    results->stats.duration = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;

    for (int i = 0; i < state->action_count; i++) {
        results->nodes[i] = *root->children[i];
    }

    if (options.save_tree) {
        results->tree = root;
    } else {
        Node_free(root);
        results->tree = NULL;
    }
}
