#include <stdio.h>

#include "state.h"
#include "stateio.h"


int main(int argc, char *argv[]) {
    struct State state;

    // Grid derive
    {
        State_new(&state);
        state.pieces[P1][state.piece_count[P1]].type = SPIDER;
        state.pieces[P1][state.piece_count[P1]].player = P1;
        state.pieces[P1][state.piece_count[P1]].coords.q = 0;
        state.pieces[P1][state.piece_count[P1]++].coords.r = 1;
        state.pieces[P2][state.piece_count[P2]].type = GRASSHOPPER;
        state.pieces[P2][state.piece_count[P2]].player = P2;
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
        p1_piece->player = P1;
        p1_piece->coords.q = 0;
        p1_piece->coords.r = 10;
        struct Piece *p2_piece = &state.pieces[P2][state.piece_count[P2]++];
        p2_piece->type = SPIDER;
        p2_piece->player = P2;
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

    printf("Done\n");

    return 0;
}
