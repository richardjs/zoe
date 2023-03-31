#include <stdlib.h>

#include "book.h"
#include "state.h"

const struct Action* opening_move(const struct State* state)
{
    if (state->piece_count[P1] == 0 || state->piece_count[P2] == 0) {
        const struct Action* actions[MAX_ACTIONS];
        int action_count = 0;
        for (int i = 0; i < state->action_count; i++) {
            switch (state->actions[i].from.r) {
            case BEETLE:
            case GRASSHOPPER:
            case SPIDER:
                actions[action_count++] = &state->actions[i];
                break;
            default:
            }
        }
        return actions[rand() % action_count];
    }

    return NULL;
}
