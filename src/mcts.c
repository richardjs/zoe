#include "mcts.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

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
    o->queen_adjacent_action_bias = DEFAULT_QUEEN_ADJACENT_ACTION_BIAS;
    o->beetle_move_bias = DEFAULT_BEETLE_MOVE_BIAS;
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
 * simulates play (in place) on a state, stopping at game end or
 * MAX_SIM_DEPTH, and returns 1.0 if the initial state won, -1.0 if it
 * lost, and 0.0 on a draw or depth out
 */
float simulate(struct State* state)
{
    results->stats.simulations++;

    enum Player original_turn = state->turn;

    int depth = 0;
    while (state->result == NO_RESULT) {
        if (depth++ > options.max_sim_depth) {
            results->stats.depth_outs++;
            return 0.0;
        }

        if (state->winning_action) {
            State_act(state, state->winning_action);
            continue;
        }

        enum Player turn = state->turn;

        struct Action* action = NULL;
        while (1) {
            if (options.queen_sidestep_bias && state->queen_move_count) {
                struct Action* actions[4];
                int action_count = 0;
                for (int i = 0; i < state->queen_move_count; i++) {
                    struct Action* action = state->queen_moves[i];
                    if (state->neighbor_count[!turn][action->to.q][action->to.r] == 1
                        && state->neighbor_count[turn][action->to.q][action->to.r] == 1) {
                        actions[action_count++] = action;
                    }
                }

                if (action_count
                    && (rand() / (float)RAND_MAX) < options.queen_sidestep_bias) {
                    action = actions[rand() % action_count];
                    goto action_chosen;
                }
            }

            if (state->queen_adjacent_action_count
                && (rand() / (float)RAND_MAX) < options.queen_adjacent_action_bias) {
                action = state->queen_adjacent_actions[rand() % state->queen_adjacent_action_count];
                goto action_chosen;
            }

            if (state->beetle_move_count
                && (rand() / (float)RAND_MAX) < options.beetle_move_bias) {
                action = state->beetle_moves[rand() % state->beetle_move_count];
                goto action_chosen;
            }

            action = &state->actions[rand() % state->action_count];

        action_chosen:

            State_act(state, action);

            break;
        }
    }

    results->stats.mean_sim_depth += (depth - results->stats.mean_sim_depth) / results->stats.simulations;

    if (state->result == DRAW) {
        return 0.0;
    }

    if ((state->result == P1_WIN && original_turn == P1)
        || (state->result == P2_WIN && original_turn == P2)) {
        return 1.0;
    }

    return -1.0;
}

/**
 * single MCTS iteration: recursively walk down tree with state
 * (choosing promising children), simulate when we get to the end of the
 * tree, and update visited nodes with the results
 */
float iterate(struct Node* root, struct State* state)
{
    // game-terminal node
    if (state->result != NO_RESULT) {
        if (state->result == DRAW) {
            root->visits++;
            root->value += 0.0; // heh
            return 0.0;
        }

        if ((state->result == P1_WIN && state->turn == P1) || (state->result == P2_WIN && state->turn == P2)) {
            // TODO How often do we hit this block? We won after an opponent move
            // ~24/1000 times; may want to prevent it from happening
            root->visits++;
            root->value += 1.0;
            return 1.0;
        }

        root->visits++;
        root->value += -1.0;
        return -1.0;
    }

    // Treat a state that has a winning moves a game-terminal
    if (state->winning_action) {
        root->visits++;
        root->value += 1.0;
        return 1.0;
    }

    if (!root->expanded) {
        Node_expand(root, state);
    }

    if (root->visits == 0) {
        float score = simulate(state);

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
