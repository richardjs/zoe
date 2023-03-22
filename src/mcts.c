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

    o->queen_move_bias = DEFAULT_QUEEN_MOVE_BIAS;
    o->queen_move_smart_bias = DEFAULT_QUEEN_MOVE_SMART_BIAS;
    o->queen_adjacent_action_bias = DEFAULT_QUEEN_ADJACENT_ACTION_BIAS;
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

    enum Player turn = state->turn;

    int depth = 0;
    while (state->result == NO_RESULT) {
        if (depth++ > options.max_sim_depth) {
            results->stats.depth_outs++;
            return 0.0;
        }

        int win = State_find_win(state);
        if (win >= 0) {
            State_act(state, &state->actions[win]);
            continue;
        }

        if (state->queens[state->turn]) {
            int queen_move_count = state->piece_move_count[state->queeni[state->turn]];
            if (queen_move_count
                && (rand() / (float)RAND_MAX) < options.queen_move_smart_bias) {
                struct Piece* queen = state->queens[state->turn];
                struct Action** queen_moves = state->piece_moves[state->queeni[state->turn]];

                int neighbor_count = State_hex_neighbor_count(state, &queen->coords);
                struct Action* better_actions[4];
                int better_action_count = 0;

                for (int i = 0; i < queen_move_count; i++) {
                    struct Action* a = queen_moves[i];
                    if (State_hex_neighbor_count(state, &a->to) < neighbor_count) {
                        better_actions[better_action_count++] = a;
                    }
                }

                if (better_action_count) {
                    State_act(state, better_actions[rand() % better_action_count]);
                    continue;
                }
            }

            if (queen_move_count
                && (rand() / (float)RAND_MAX) < options.queen_move_bias) {
                struct Action** queen_moves = state->piece_moves[state->queeni[state->turn]];
                State_act(state, queen_moves[rand() % queen_move_count]);
                continue;
            }
        }

        // if (state->queen_adjacent_action_count
        //     && (rand() / (float)RAND_MAX) < options.queen_adjacent_action_bias) {
        //     State_act(state, state->queen_adjacent_actions[rand() % state->queen_adjacent_action_count]);
        //     continue;
        // }

        struct Action* action;
        if (state->queen_adjacent_action_count
            && (rand() / (float)RAND_MAX) < options.queen_adjacent_action_bias) {
            action = state->queen_adjacent_actions[rand() % state->queen_adjacent_action_count];
        } else {
            action = &state->actions[rand() % state->action_count];
        }
        for (int i = 0; i < 10; i++) {
            if (action->from.q == PLACE_ACTION || action->from.q == PASS_ACTION) {
                break;
            }

            struct Piece* piece = state->grid[action->from.q][action->from.r];
            struct Piece* from_under = NULL;
            while (piece->on_top) {
                from_under = piece;
                piece = piece->on_top;
            }
            if (from_under) {
                from_under->on_top = NULL;
            } else {
                state->grid[action->from.q][action->from.r] = NULL;
            }

            struct Piece* to_under = NULL;
            if (state->grid[action->to.q][action->to.r]) {
                struct Piece* p = state->grid[action->to.q][action->to.r];
                while (p->on_top) {
                    p = p->on_top;
                }
                p->on_top = piece;
                to_under = p;
            } else {
                state->grid[action->to.q][action->to.r] = piece;
            }

            unsigned int new_cut_points[NUM_PLAYERS];
            State_count_cut_points(state, &action->to, new_cut_points);

            if (to_under) {
                to_under->on_top = NULL;
            } else {
                state->grid[action->to.q][action->to.r] = NULL;
            }
            if (from_under) {
                from_under->on_top = piece;
            } else {
                state->grid[action->from.q][action->from.r] = piece;
            }
            if ((new_cut_points[state->turn] - new_cut_points[!state->turn])
                <= (state->cut_points[state->turn] - state->cut_points[!state->turn])) {
                break;
            }

            if (state->queen_adjacent_action_count
                && (rand() / (float)RAND_MAX) < options.queen_adjacent_action_bias) {
                action = state->queen_adjacent_actions[rand() % state->queen_adjacent_action_count];
            } else {
                action = &state->actions[rand() % state->action_count];
            }
        }
        State_act(state, action);
    }

    // TODO
    if (state->result == NO_RESULT) {
        puts("error! we shouldnt be here");
    }

    results->stats.mean_sim_depth += (depth - results->stats.mean_sim_depth) / results->stats.simulations;

    if (state->result == DRAW) {
        return 0.0;
    }

    if ((state->result == P1_WIN && turn == P1) || (state->result == P2_WIN && turn == P2)) {
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
    if (State_find_win(state) >= 0) {
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

    for (int i = 0; i < options.iterations; i++) {
        struct State s;
        State_copy(state, &s);
        iterate(root, &s);
        results->stats.iterations++;
    }

    struct timeval end;
    gettimeofday(&end, NULL);
    results->stats.duration = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;

    results->score = -INFINITY;
    for (int i = 0; i < state->action_count; i++) {
        float score = -1 * root->children[i]->value / root->children[i]->visits;

        if (score >= results->score) {
            results->score = score;
            results->actioni = i;
        }
    }

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
