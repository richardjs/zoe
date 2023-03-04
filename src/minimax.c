#include <math.h>
#include <string.h>

#include "minimax.h"
#include "state.h"


static struct MinimaxOptions options;
static struct MinimaxResults *results;


void MinimaxOptions_default(struct MinimaxOptions *options) {
    options->depth = DEFAULT_MINIMAX_DEPTH;
}


float evaluate(const struct State *state) {
    return 0.0;
}


float search(const struct State *state, int depth) {
    results->stats.nodes++;

    if (State_find_win(state) >= 0) {
        return depth + 1;
    }

    if (depth == 0) {
        results->stats.leaves++;
        return evaluate(state);
    }

    float best_score = -INFINITY;
    for (int i = 0; i < state->action_count; i++) {
        struct State child;
        State_copy(state, &child);
        State_act(&child, &state->actions[i]);
        float child_score = -search(&child, depth - 1);
        if (child_score >= best_score) {
           best_score = child_score;
        }
    }

    return best_score;
}


void minimax(const struct State *state, struct MinimaxResults *r, const struct MinimaxOptions *o) {
    if (o == NULL) {
        MinimaxOptions_default(&options);
    } else {
        options = *o;
    }

    results = r;
    memset(results, 0, sizeof(struct MinimaxResults));

    results->stats.nodes++;

    int win = State_find_win(state);
    if (win >= 0) {
        results->actioni = win;
        results->score = INFINITY;
        results->stats.leaves++;
        return;
    }

    if (options.depth == 0) {
        results->score = evaluate(state);
        results->stats.leaves++;
        return;
    }

    results->score = -INFINITY;
    for (int i = 0; i < state->action_count; i++) {
        struct State child;
        State_copy(state, &child);
        State_act(&child, &state->actions[i]);
        float child_score = -search(&child, options.depth - 1);
        if (child_score > results->score) {
            results->score = child_score;
            results->actioni = i;
        }
    }
}
