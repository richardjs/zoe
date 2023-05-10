#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "errorcodes.h"
#include "examine.h"
#include "mcts.h"
#include "minimax.h"
#include "state.h"
#include "stateio.h"
#include "think.h"

enum Command {
    NONE,
    THINK,
    SEARCH,
    RANDOM,
    NORMALIZE,
    LIST_ACTIONS,
    EXAMINE,
    ACT
};

int main(int argc, char* argv[])
{
    fprintf(stderr, "Zo\u00e9 v1.1a (built %s %s)\n", __DATE__, __TIME__);

    time_t seed = time(NULL);
    srand(seed);

    init_coords();

    enum Command command = NONE;
    int workers = 1;

    struct MCTSOptions options;
    MCTSOptions_default(&options);

    struct MinimaxOptions minimax_options;
    MinimaxOptions_default(&minimax_options);

    int opt;
    struct Action action;
    while ((opt = getopt(argc, argv, "vnltsrxa:i:c:w:j:k:z:b:d:p:u:o:")) != -1) {
        switch (opt) {
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

        case 's':
            command = SEARCH;
            break;

        case 'r':
            command = RANDOM;
            break;

        case 'x':
            command = EXAMINE;
            break;

        case 'a':
            command = ACT;
            Action_from_string(&action, optarg);
            break;

        case 'i':
            options.iterations = atoi(optarg);
            minimax_options.depth = atoi(optarg);
            break;

        case 'c':
            options.uctc = atof(optarg);
            break;

        case 'j':
            options.queen_adjacent_action_bias = atof(optarg);
            break;

        case 'k':
            options.queen_nearby_action_bias = atof(optarg);
            break;

        case 'z':
            options.queen_sidestep_bias = atof(optarg);
            break;

        case 'b':
            options.beetle_move_bias = atof(optarg);
            break;

        case 'p':
            options.pin_move_bias = atof(optarg);
            break;

        case 'u':
            options.unpin_move_bias = atof(optarg);
            break;

        case 'o':
            options.own_pin_pass = atof(optarg);
            break;

        case 'd':
            options.cut_point_diff_terminate = atof(optarg);
            break;

        case 'w':
            workers = atoi(optarg);
            break;
        }
    }

    if (argc == optind) {
        fprintf(stderr, "No state provided\n");
        return ERROR_NO_STATE_GIVEN;
    }

    struct State state;
    State_from_string(&state, argv[optind]);

    fprintf(stderr, "input: %s\n", argv[optind]);
    State_print(&state, stderr);

    char state_string[STATE_STRING_SIZE];
    char action_string[ACTION_STRING_SIZE];

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
        State_print(&state, stderr);
        printf("%s\n", state_string);
        return 0;

    case SEARCH: {
        struct MinimaxResults minimax_results;
        minimax(&state, &minimax_results, &minimax_options);
        fprintf(stderr, "action:\t");
        Action_print(&minimax_results.action, stderr);
        fprintf(stderr, "score:\t%.2f\n", minimax_results.score);
        fprintf(stderr, "nodes:\t%ld\n", minimax_results.stats.nodes);
        fprintf(stderr, "leaves:\t%ld\n", minimax_results.stats.leaves);
        return 0;
    }

    case RANDOM: {
        struct Action* action = &state.actions[rand() % state.action_count];

        Action_to_string(action, action_string);
        printf("%s\n", action_string);

        struct State after;
        State_copy(&state, &after);
        State_act(&after, action);
        State_normalize(&after);
        State_to_string(&after, state_string);
        fprintf(stderr, "next:\t%s\n", state_string);
        return 0;
    }

    case EXAMINE:
        State_examine(&state);
        return 0;

    case THINK:
        break;
    }

    struct MCTSResults results;
    think(&state, &results, &options, workers);

    const struct Action* selected_action;
    if (results.presearch_action) {
        selected_action = results.presearch_action;
    } else {
        selected_action = &state.actions[results.actioni];
    }

    struct State after;
    State_copy(&state, &after);
    State_act(&after, selected_action);
    State_print(&after, stderr);

    return 0;
}
