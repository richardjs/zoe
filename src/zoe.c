#include <unistd.h>

#include "errorcodes.h"
#include "state.h"
#include "stateio.h"


enum Command {NONE, VERSION, LIST_ACTIONS};


int main(int argc, char *argv[]) {
    fprintf(stderr, "Zo\u00e9 v.1a\n");

    enum Command command = NONE;

    int opt;
    while ((opt = getopt(argc, argv, "vl")) != -1) {
        switch(opt) {
            case 'v':
                command = VERSION;
                break;

            case 'l':
                command = LIST_ACTIONS;
                break;
        }
    }

    if (argc == optind && command != VERSION) {
        fprintf(stderr, "No state provided\n");
        return ERROR_NO_STATE_GIVEN;
    }


    struct State state;
    State_from_string(&state, argv[optind]);

    switch (command) {
        case NONE:
            fprintf(stderr, "No command given\n");
            return ERROR_NO_COMMAND_GIVEN;

        case VERSION:
            return 0;

        case LIST_ACTIONS:
            for (int i = 0; i < state.action_count; i++) {
                Action_print(&state.actions[i], stdout);
            }
            return 0;
    }

    return 0;
}
