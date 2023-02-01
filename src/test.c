#include <stdio.h>
#include <string.h>

#include "state.h"
#include "stateio.h"
#include "stateutil.h"



uint_fast8_t State_articulation_points(const struct State *state, struct Coords points[]);
void State_derive_neighbor_count(struct State *state);


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


    // Neighbor count
    {
        State_new(&state);
        for (int p = 0; p < NUM_PLAYERS; p++) {
            for (int q = 0; q < GRID_SIZE; q++) {
                for (int r = 0; r < GRID_SIZE; r++) {
                    if (state.neighbor_count[p][q][r] > 0) {
                        printf("New state has neighbor count!\n");
                    }
                }
            }
        }

        State_act(&state, &state.actions[0]);
        if (state.neighbor_count[P1][0][1] != 1) {
            printf("Neighbor count not incremented\n");
        }
        if (state.neighbor_count[P2][0][1] > 0) {
            printf("Neighbor incremented when it shouldn't have been\n");
        }
    }


    // Place action count
    {
        State_new(&state);
        State_act(&state, &state.actions[0]);
        State_act(&state, &state.actions[0]);
        if (state.action_count != 15) {
            printf("Incorrect action count after 2 moves: %d (should be 15)\n", state.action_count);
        }
        State_act(&state, &state.actions[0]);
        if (state.action_count != 15) {
            printf("Incorrect action count after 3 moves: %d (should be 15)\n", state.action_count);
        }
    }


    // Ariculation points
    {
        // Square: no APs
        char state_string[STATE_STRING_SIZE] = "aaaAababaAbb1";
        State_from_string(&state, state_string);
        for (int q = 0; q < GRID_SIZE; q++) {
            for (int r = 0; r < GRID_SIZE; r++) {
                if (state.cut_points[q][r]) {
                    printf("Incorrect cut points:\n");
                    State_print(&state, stdout);
                }
            }
        }

        // Single piece: no APs
        strcpy(state_string, "Aaa1");
        State_from_string(&state, state_string);
        for (int q = 0; q < GRID_SIZE; q++) {
            for (int r = 0; r < GRID_SIZE; r++) {
                if (state.cut_points[q][r]) {
                    printf("Incorrect cut points:\n");
                    State_print(&state, stdout);
                }
            }
        }

        // 3-piece line: 1 AP
        strcpy(state_string, "AaaAabAbb1");
        State_from_string(&state, state_string);
        for (int q = 0; q < GRID_SIZE; q++) {
            for (int r = 0; r < GRID_SIZE; r++) {
                if (
                    (state.cut_points[q][r] && (q != 0 && r != 1))
                    || (!state.cut_points[q][r] && (q == 0 && r == 1))
                ) {
                    printf("Incorrect cut points:\n");
                    State_print(&state, stdout);
                }
            }
        }

        // Same as previous, but start from AP (search starts at pieces[P1][0]
        strcpy(state_string, "aaaAababb1");
        State_from_string(&state, state_string);
        for (int q = 0; q < GRID_SIZE; q++) {
            for (int r = 0; r < GRID_SIZE; r++) {
                if (
                    (state.cut_points[q][r] && (q != 0 && r != 1))
                    || (!state.cut_points[q][r] && (q == 0 && r == 1))
                ) {
                    printf("Incorrect cut points:\n");
                    State_print(&state, stdout);
                }
            }
        }

        // 4-piece line: 2 APs
        strcpy(state_string, "aaaAababbabc1");
        State_from_string(&state, state_string);
        int point_count = 0;
        for (int q = 0; q < GRID_SIZE; q++) {
            for (int r = 0; r < GRID_SIZE; r++) {
                if (state.cut_points[q][r]) {
                    point_count++;
                }
            }
        }
        if (point_count != 2) {
            printf("Incorrect cut points:\n");
            State_print(&state, stdout);
        }

        // Ring: 0 APs
        strcpy(state_string, "aabAacabcAcbacaAba1");
        State_from_string(&state, state_string);
        point_count = 0;
        for (int q = 0; q < GRID_SIZE; q++) {
            for (int r = 0; r < GRID_SIZE; r++) {
                if (state.cut_points[q][r]) {
                    point_count++;
                }
            }
        }
        if (point_count > 0) {
            printf("Incorrect cut points:\n");
            State_print(&state, stdout);
        }

        // Ring with piece on edge: 0 APs
        strcpy(state_string, "aabAacabcAcbacaAbaqcc1");
        State_from_string(&state, state_string);
        point_count = 0;
        for (int q = 0; q < GRID_SIZE; q++) {
            for (int r = 0; r < GRID_SIZE; r++) {
                if (state.cut_points[q][r]) {
                    point_count++;
                }
            }
        }
        if (point_count > 0) {
            printf("Incorrect cut points:\n");
            State_print(&state, stdout);
        }

        // Ring with line ("Q"): 1 APs
        strcpy(state_string, "aabAacabcAcbacaAbaqccQdc1");
        State_from_string(&state, state_string);
        point_count = 0;
        for (int q = 0; q < GRID_SIZE; q++) {
            for (int r = 0; r < GRID_SIZE; r++) {
                if (state.cut_points[q][r]) {
                    point_count++;
                }
            }
        }
        if (point_count != 1) {
            printf("Incorrect cut points:\n");
            State_print(&state, stdout);
        }

        // Example game state: 4 APs
        strcpy(state_string, "AadabdaccbcdSceSdbqdcQdeaee1");
        State_from_string(&state, state_string);
        point_count = 0;
        for (int q = 0; q < GRID_SIZE; q++) {
            for (int r = 0; r < GRID_SIZE; r++) {
                if (state.cut_points[q][r]) {
                    point_count++;
                }
            }
        }
        if (point_count != 4) {
            printf("Incorrect cut points:\n");
            State_print(&state, stdout);
        }
    }


    // Queen moves
    {
        char state_string[STATE_STRING_SIZE];
        int move_count;

        // No moves if queen isn't present
        strcpy(state_string, "AaaqabAbaabb1");
        State_from_string(&state, state_string);
        for (int i = 0; i < state.action_count; i++) {
            if (state.actions[i].from.q != PLACE_ACTION) {
                printf("Move action without queen placed\n");
                State_print(&state, stdout);
                Action_print(&state.actions[i], stdout);
            }
        }

        // Basic queen move
        strcpy(state_string, "Qaaaababb1");
        State_from_string(&state, state_string);
        move_count = 0;
        for (int i = 0; i < state.action_count; i++) {
            if (state.actions[i].from.q != PLACE_ACTION) {
                move_count++;
            }
        }
        if (move_count != 2) {
            printf("Improper move count: %d\n", move_count);
            State_print(&state, stdout);
        }

        // Third move blocked by freedom to move
        strcpy(state_string, "Qabaacabcacbgcagba1");
        State_from_string(&state, state_string);
        move_count = 0;
        for (int i = 0; i < state.action_count; i++) {
            if (state.actions[i].from.q != PLACE_ACTION) {
                move_count++;
            }
        }
        if (move_count != 2) {
            printf("Improper move count: %d\n", move_count);
            State_print(&state, stdout);
        }
    }


    printf("Done\n");
    return 0;
}
