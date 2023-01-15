#include <stdbool.h>
#include <stdio.h>

#include "state.h"
#include "stateio.h"


void State_translate_grid(struct State *state, int8_t q, int8_t r) {
    for (int p = 0; p < NUM_PLAYERS; p++) {
        for (int i = 0; i < state->piece_count[p]; i++) {
            struct Piece *piece = &state->pieces[p][i];
            piece->coords.q += q;
            piece->coords.r += r;
        }
    }
    State_derive(state);
}


void State_normalize(struct State *state) {
    if (state->piece_count[P1] == 0 && state->piece_count[P2] == 0) {
        return;
    }

    // Elimanate any wrapping by shifting everything off the far edges
    bool piece_on_edge;
    do {
        piece_on_edge = false;
        for (int q = 0; q < GRID_SIZE; q++) {
            if (state->grid[q][GRID_SIZE-1] != NULL) {
                piece_on_edge = true;
                break;
            }
        }
        if (piece_on_edge) {
            State_translate_grid(state, 0, -1);
        }
    } while (piece_on_edge);
    do {
        piece_on_edge = false;
        for (int r = 0; r < GRID_SIZE; r++) {
            if (state->grid[GRID_SIZE-1][r] != NULL) {
                piece_on_edge = true;
                break;
            }
        }
        if (piece_on_edge) {
            State_translate_grid(state, -1, 0);
        }
    } while (piece_on_edge);

    // Now that there's no wrapping, shift everything to the near edges
    piece_on_edge = false;
    while (!piece_on_edge) {
        for (int q = 0; q < GRID_SIZE; q++) {
            if (state->grid[q][0] != NULL) {
                piece_on_edge = true;
                break;
            }
        }
        if (!piece_on_edge) {
            State_translate_grid(state, 0, -1);
        }
    }
    piece_on_edge = false;
    while (!piece_on_edge) {
        for (int r = 0; r < GRID_SIZE; r++) {
            if (state->grid[0][r] != NULL) {
                piece_on_edge = true;
                break;
            }
        }
        if (!piece_on_edge) {
            State_translate_grid(state, -1, 0);
        }
    }
}


void State_print(const struct State *state, FILE *stream) {
    //struct State normalized = *state;
}
