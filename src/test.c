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
            printf("grid[0][1] does not match state.pieces[P1][0]");
        }
        if (state.grid[1][0] != &state.pieces[P2][0]) {
            printf("grid[1][0] does not match state.pieces[P2][0]");
        }
    }

    return 0;
}
