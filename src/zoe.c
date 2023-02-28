#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "errorcodes.h"
#include "mcts.h"
#include "state.h"
#include "stateio.h"


#define TOP_ACTIONS 5


enum Command {NONE, THINK, NORMALIZE, LIST_ACTIONS, ACT};


int main(int argc, char *argv[]) {
    fprintf(stderr, "Zo\u00e9 v.1a (built %s %s)\n", __DATE__, __TIME__);

    time_t seed = time(NULL);
    srand(seed);

    enum Command command = NONE;
    //int workers = 1;

    struct MCTSOptions options;
    MCTSOptions_default(&options);

    int opt;
    struct Action action;
    while ((opt = getopt(argc, argv, "vnlta:i:c:w:")) != -1) {
        switch(opt) {
            case 'v':
                return 0;
                break;

            case 'n':
                command = NORMALIZE;
                break;

            case 'l':
                command = LIST_ACTIONS;
                break;

			case 't':
                command = THINK;
                break;

            case 'a':
                command = ACT;
                Action_from_string(&action, optarg);
                break;

            case 'i':
                options.iterations = atoi(optarg);
                break;

            case 'c':
                options.uctc = atof(optarg);
                break;

            case 'w':
            //    workers = atoi(optarg);
            //    break;
        }
    }

    if (argc == optind) {
        fprintf(stderr, "No state provided\n");
        return ERROR_NO_STATE_GIVEN;
    }

    struct State state;
    State_from_string(&state, argv[optind]);

    char state_string[STATE_STRING_SIZE];

    switch (command) {
        case NONE:
            fprintf(stderr, "No command given\n");
            return ERROR_NO_COMMAND_GIVEN;

        case NORMALIZE:
            State_normalize(&state);
            State_to_string(&state, state_string);
            printf("%s\n", state_string);
            return 0;

        case LIST_ACTIONS:
            for (int i = 0; i < state.action_count; i++) {
                Action_print(&state.actions[i], stdout);
            }
            return 0;

        case ACT:
            State_act(&state, &action);
            State_normalize(&state);
            State_to_string(&state, state_string);
            printf("%s\n", state_string);
            return 0;

        case THINK:
    }

    struct MCTSResults results;
    memset(&results, 0, sizeof(struct MCTSResults));

    // TODO
    struct timeval start;
    gettimeofday(&start, NULL);

    mcts(&state, &results, &options);

    struct timeval end;
    gettimeofday(&end, NULL);
    results.stats.duration = (end.tv_sec - start.tv_sec)*1000 + (end.tv_usec - start.tv_usec)/1000;

    results.score = -INFINITY;
    int top_actionis[TOP_ACTIONS];
    memset(top_actionis, -1, sizeof(int) * TOP_ACTIONS);
    for (int i = 0; i < state.action_count; i++) {
        float score = -1 * results.nodes[i].value / results.nodes[i].visits;

        if (score >= results.score) {
            results.score = score;
            results.actioni = i;
        }


        for (int j = 0; j < TOP_ACTIONS; j++) {
            if (top_actionis[j] < 0) {
                top_actionis[j] = i;
                break;
            }

            float s = -1 * results.nodes[top_actionis[j]].value / results.nodes[top_actionis[j]].visits;
            if (score > s) {
                for (int k = TOP_ACTIONS - 2; k >= j ; k--) {
                    top_actionis[k+1] = top_actionis[k];
                }
                top_actionis[j] = i;
                break;
            }
        }
    }

    char action_string[ACTION_STRING_SIZE];
    Action_to_string(&state.actions[results.actioni], action_string);
    printf("%s\n", action_string);

    fprintf(stderr, "action:\t\t%s\n", action_string);
    fprintf(stderr, "score:\t\t%.2f\n", results.score);

    fprintf(stderr, "iterations:\t%ld\n", results.stats.iterations);
    //fprintf(stderr, "workers:\t%d\n", workers);
    fprintf(stderr, "time:\t\t%ld ms\n", results.stats.duration);
    fprintf(stderr, "iters/s:\t%ld\n",
        results.stats.duration ?
            1000 * results.stats.iterations / results.stats.duration : 0);
    fprintf(stderr, "depth outs:\t%.2f%%\n",
        100 * (float)results.stats.depth_outs / results.stats.simulations);
    fprintf(stderr, "tree size:\t%ld MiB\n",
        results.stats.tree_bytes / 1024 / 1024);

    for (int i = 1; i < TOP_ACTIONS; i++) {
        Action_to_string(&state.actions[top_actionis[i]], action_string);
        float score = -1 * results.nodes[top_actionis[i]].value / results.nodes[top_actionis[i]].visits;
        fprintf(stderr, "alt:\t%.2f\t%s\n", score, action_string);
    }

    struct State after = state;
    State_act(&after, &state.actions[results.actioni]);
    State_print(&after, stderr);

    State_to_string(&after, state_string);
    fprintf(stderr, "next:\t%s\n", state_string);

    return 0;
}
