#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "book.h"
#include "mcts.h"
#include "state.h"
#include "stateio.h"
#include "think.h"

#define TOP_ACTIONS 10

void think(
    const struct State* state,
    struct MCTSResults* results,
    const struct MCTSOptions* options,
    int workers)
{
    char state_string[STATE_STRING_SIZE];
    char action_string[ACTION_STRING_SIZE];

    if (state->winning_action) {
        fprintf(stderr, "Taking win\n");
        fprintf(stderr, "result: win\n");

        results->presearch_action = state->winning_action;

        struct State after;
        State_copy(state, &after);
        State_act(&after, state->winning_action);
        State_normalize(&after);
        State_to_string(&after, state_string);
        fprintf(stderr, "next:\t%s\n", state_string);
        return;
    }

    if (state->action_count == 1) {
        fprintf(stderr, "Single action\n");

        results->presearch_action = &state->actions[0];

        struct State after;
        State_copy(state, &after);
        State_act(&after, &state->actions[0]);
        State_normalize(&after);
        State_to_string(&after, state_string);
        fprintf(stderr, "next:\t%s\n", state_string);
        return;
    }

    const struct Action* book_action = opening_move(state);
    if (book_action) {
        fprintf(stderr, "Book action\n");

        results->presearch_action = book_action;

        struct State after;
        State_copy(state, &after);
        State_act(&after, book_action);
        State_normalize(&after);
        State_to_string(&after, state_string);
        fprintf(stderr, "next:\t%s\n", state_string);
        return;
    }

    fprintf(stderr, "MCTS options:\titerations=%ld workers=%d uctc=%.2f\n",
        options->iterations,
        workers,
        options->uctc);
    fprintf(stderr, "sim options:\tmax_depth=%d queen_adjacent_action_bias=%.2f queen_nearby_action_bias=%.2f queen_sidestep_bias=%.2f beetle_move_bias=%.2f cut_point_diff_terminate=%d\n",
        options->max_sim_depth,
        options->queen_adjacent_action_bias,
        options->queen_nearby_action_bias,
        options->queen_sidestep_bias,
        options->beetle_move_bias,
        options->cut_point_diff_terminate);

    int pipefd[2];
    pipe(pipefd);

    for (int i = 0; i < workers; i++) {
        srand(rand());
        if (fork() > 0) {
            continue;
        }
        struct MCTSResults results;
        mcts(state, &results, options);
        write(pipefd[1], &results, sizeof(struct MCTSResults));
        exit(0);
    }

    memset(results, 0, sizeof(struct MCTSResults));

    // TODO
    struct timeval start;
    gettimeofday(&start, NULL);

    for (int i = 0; i < workers; i++) {
        wait(NULL);

        struct MCTSResults worker_results;
        read(pipefd[0], &worker_results, sizeof(struct MCTSResults));

        for (int j = 0; j < state->action_count; j++) {
            results->nodes[j].visits += worker_results.nodes[j].visits;
            results->nodes[j].value += worker_results.nodes[j].value;
        }

        results->stats.iterations += worker_results.stats.iterations;
        results->stats.nodes += worker_results.stats.nodes;
        results->stats.tree_bytes += worker_results.stats.tree_bytes;
        results->stats.simulations += worker_results.stats.simulations;
        results->stats.mean_sim_depth += worker_results.stats.mean_sim_depth / workers;
        results->stats.depth_outs += worker_results.stats.depth_outs;
        results->stats.cut_point_terminations += worker_results.stats.cut_point_terminations;
        results->stats.change_iterations = results->stats.change_iterations > worker_results.stats.change_iterations ? results->stats.change_iterations : worker_results.stats.change_iterations;
    }

    struct timeval end;
    gettimeofday(&end, NULL);
    results->stats.duration = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;

    results->score = -INFINITY;
    int top_actionis[TOP_ACTIONS];
    memset(top_actionis, -1, sizeof(int) * TOP_ACTIONS);
    for (int i = 0; i < state->action_count; i++) {
        float score = -1 * results->nodes[i].value / results->nodes[i].visits;

        if (score >= results->score) {
            results->score = score;
            results->actioni = i;
        }

        for (int j = 0; j < TOP_ACTIONS && j < state->action_count; j++) {
            if (top_actionis[j] < 0) {
                top_actionis[j] = i;
                break;
            }

            float s = -1 * results->nodes[top_actionis[j]].value / results->nodes[top_actionis[j]].visits;
            if (score > s) {
                for (int k = TOP_ACTIONS - 2; k >= j; k--) {
                    top_actionis[k + 1] = top_actionis[k];
                }
                top_actionis[j] = i;
                break;
            }
        }
    }

    struct State after;
    State_copy(state, &after);
    State_act(&after, &state->actions[results->actioni]);

    fprintf(stderr, "action:\t\t%s\n", action_string);
    fprintf(stderr, "score:\t\t%.2f\n", results->score);

    fprintf(stderr, "iterations:\t%ld\n", results->stats.iterations);
    fprintf(stderr, "change iters:\t%d\n", results->stats.change_iterations);
    fprintf(stderr, "time:\t\t%ld ms\n", results->stats.duration);
    fprintf(stderr,
        "iters/s:\t%ld\n",
        results->stats.duration
            ? 1000 * results->stats.iterations / results->stats.duration
            : 0);
    fprintf(stderr, "actions:\t%ld\n", state->action_count);
    fprintf(stderr, "cut point diff:\t%d\n", after.cut_point_count[P2] - after.cut_point_count[P1]);
    fprintf(stderr, "q.a. actions:\t%ld\n", state->queen_adjacent_action_count);
    fprintf(stderr, "q.n. actions:\t%ld\n", state->queen_nearby_action_count);
    fprintf(stderr, "pin moves:\t%ld\n", state->pin_move_count);
    fprintf(stderr, "action iters:\t%d\n", results->nodes[results->actioni].visits);
    fprintf(stderr, "mean sim depth:\t%.2f\n", results->stats.mean_sim_depth);
    fprintf(stderr,
        "cut point outs:\t%.2f%%\n",
        100 * (float)results->stats.cut_point_terminations / results->stats.simulations);
    fprintf(stderr,
        "depth outs:\t%.2f%%\n",
        100 * (float)results->stats.depth_outs / results->stats.simulations);
    fprintf(
        stderr, "tree size:\t%ld MiB\n", results->stats.tree_bytes / 1024 / 1024);

    for (int i = 0; i < TOP_ACTIONS && i < state->action_count; i++) {
        Action_to_string(&state->actions[top_actionis[i]], action_string);
        float score = -1 * results->nodes[top_actionis[i]].value / results->nodes[top_actionis[i]].visits;
        fprintf(stderr, "%.2f\t%s\t%d\n", score, action_string, results->nodes[top_actionis[i]].visits);
    }

    State_normalize(&after);
    State_to_string(&after, state_string);
    fprintf(stderr, "next:\t%s\n", state_string);
}
