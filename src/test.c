#include <stdio.h>
#include <string.h>

#include "state.h"
#include "stateio.h"
#include "stateutil.h"


int main(int argc, char *argv[]) {
    struct State state;


    printf("Running tests...\n");


    // Grid derive
    {
        State_new(&state);
        state.pieces[P1][state.piece_count[P1]].type = SPIDER;
        state.pieces[P1][state.piece_count[P1]].coords.q = 0;
        state.pieces[P1][state.piece_count[P1]++].coords.r = 1;
        state.pieces[P2][state.piece_count[P2]].type = GRASSHOPPER;
        state.pieces[P2][state.piece_count[P2]].coords.q = 1;
        state.pieces[P2][state.piece_count[P2]++].coords.r = 0;
        State_derive(&state);

        if (state.grid[0][1] != &state.pieces[P1][0]) {
            printf("grid[0][1] does not match state.pieces[P1][0]\n");
        }
        if (state.grid[1][0] != &state.pieces[P2][0]) {
            printf("grid[1][0] does not match state.pieces[P2][0]\n");
        }
    }


    // State normalizaiton
    {
        State_new(&state);
        struct Piece *p1_piece = &state.pieces[P1][state.piece_count[P1]++];
        p1_piece->type = SPIDER;
        p1_piece->coords.q = 0;
        p1_piece->coords.r = 10;
        struct Piece *p2_piece = &state.pieces[P2][state.piece_count[P2]++];
        p2_piece->type = SPIDER;
        p2_piece->coords.q = GRID_SIZE - 1;
        p2_piece->coords.r = 10;
        State_derive(&state);
        State_normalize(&state);

        if (p1_piece->coords.q != 1 || p2_piece->coords.r != 0) {
            printf(
                "q not normalized: %d %d\n",
                p1_piece->coords.q,
                p2_piece->coords.q
            );
        }
        if (p1_piece->coords.r != 0 || p2_piece->coords.r != 0) {
            printf(
                "r not normalized: %d %d\n",
                p1_piece->coords.r,
                p2_piece->coords.r
            );
        }
    }


    // Piece comparison
    {
        struct Piece p1;
        p1.type = GRASSHOPPER;
        p1.coords.q = 3;
        p1.coords.r = 5;
        p1.player = P1;

        struct Piece p2 = p1;

        if (Piece_compare(&p1, &p2)) {
            printf("Identical pieces compare as different\n");
        }

        p2.coords.r = 4;
        if (!Piece_compare(&p1, &p2)) {
            printf("Different pieces compare as identical\n");
        }
        p2.coords.r = 5;

        struct Piece b1 = p1;
        b1.type = BEETLE;
        p1.on_top = &b1;
        if (!Piece_compare(&p1, &b1)) {
            printf("Piece and beetle on top of it compare as same\n");
        }

        if (!Piece_compare(&p1, &p2)) {
            printf("Pieces compare as same with different pieces above them\n");
        }

        p2.on_top = &b1;
        if (Piece_compare(&p1, &p2)) {
            printf("Pieces compare as different when identical (with pieces above them\n");
        }
    }


    // State comparison
    {
        char state_string[STATE_STRING_SIZE] = "aacQbbScbsdaBdaqea1";
        State_from_string(&state, state_string);

        struct State other = state;

        if (State_compare(&state, &other)) {
            printf("Identical states compare differently\n");
        }

        other.pieces[P1][2].type = GRASSHOPPER;
        if (!State_compare(&state, &other)) {
            printf("Different states compare as same\n");
        }
    }


    // State deserialization and serialization
    {
        char state_string[STATE_STRING_SIZE] = "aacQbbScbsdaBdaqea1";
        State_from_string(&state, state_string);

        char to_string[STATE_STRING_SIZE];
        State_to_string(&state, to_string);

        if (strncmp(state_string, to_string, STATE_STRING_SIZE)) {
            printf("State deserializes and serializes to different string: %s %s\n", state_string, to_string);
        }

        struct State other;
        State_from_string(&other, state_string);

        if (State_compare(&state, &other)) {
            printf("States deserialized from the same string compare as different\n");
        }
    }


    // Hands derivation
    {
        char state_string[STATE_STRING_SIZE] = "aacQbbScbsdaBdaqea1";
        State_from_string(&state, state_string);

        if (state.hands[P1][QUEEN_BEE] != 0
                || state.hands[P2][SPIDER] != 1
                || state.hands[P1][ANT] != 3
                || state.hands[P2][ANT] != 2) {
            printf("Hands not deriving correctly\n");
        }
    }


    // Initial place moves
    {
        State_new(&state);
        if (state.action_count != NUM_PIECETYPES -1) {
            printf("Incorrect number of starting places\n");
        }
    }


    // Action serialization
    {
        State_new(&state);

        char action_string[ACTION_STRING_SIZE];
        Action_to_string(&state.actions[3], action_string);

        if (strcmp(action_string, "+saa")) {
            printf("Action not serializing properly\n");
        }

        struct Action from_string;
        Action_from_string(&from_string, action_string);

        if (memcmp(&state.actions[3], &from_string, sizeof(struct Action))) {
            printf("Action serialization and deserialization results changes:\n");
            Action_to_string(&state.actions[3], action_string);
            printf("before: %s\n", action_string);
            Action_to_string(&from_string, action_string);
            printf("after: %s\n", action_string);
        }
    }


    printf("Done\n");

    return 0;
}
