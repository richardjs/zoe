#include <unistd.h>

#include "errorcodes.h"
#include "state.h"
#include "stateio.h"


enum Command {NONE, VERSION, LIST_ACTIONS, ACT};


int main(int argc, char *argv[]) {
    fprintf(stderr, "Zo\u00e9 v.1a\n");

    enum Command command = NONE;

    int opt;
    struct Action action;
    while ((opt = getopt(argc, argv, "vla:")) != -1) {
        switch(opt) {
            case 'v':
                command = VERSION;
                break;

            case 'l':
                command = LIST_ACTIONS;
                break;

            case 'a':
                command = ACT;
                Action_from_string(&action, optarg);
        }
    }

    if (argc == optind && command != VERSION) {
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

        case VERSION:
            break;

        case LIST_ACTIONS:
            for (int i = 0; i < state.action_count; i++) {
                Action_print(&state.actions[i], stdout);
            }
            break;

        case ACT:
            State_act(&state, &action);
            State_to_string(&state, state_string);
            printf("%s\n", state_string);
            break;
    }

    return 0;
}
