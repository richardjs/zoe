#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "state.h"
#include "stateio.h"


const char PIECE_CHAR[NUM_PLAYERS][NUM_PIECETYPES] = {
    {'A', 'B', 'G', 'S', 'Q'},
    {'a', 'b', 'g', 's', 'q'}
};


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


void State_print(const struct State *s, FILE *stream) {
    struct State state = *s;
    State_normalize(&state);

    // Convert to double-height coordinate space; see
    // https://www.redblobgames.com/grids/hexagons/#coordinates-doubled
    struct Piece *grid[GRID_SIZE][GRID_SIZE];
    memset(grid, 0, sizeof(struct Piece*) * GRID_SIZE * GRID_SIZE);
    int min_x = GRID_SIZE - 2;
    int max_x = 0;
    int min_y = GRID_SIZE - 2;
    int max_y = 0;
    for (int p = 0; p < NUM_PLAYERS; p++) {
        for (int i = 0; i < state.piece_count[p]; i++) {
            struct Piece *piece = &state.pieces[p][i];

            // https://www.redblobgames.com/grids/hexagons/#conversions-doubled
            int x = piece->coords.q;
            int y = 2*piece->coords.r + piece->coords.q;
            grid[x][y] = piece;

            if (x < min_x) min_x = x;
            if (x > max_x) max_x = x;
            if (y < min_y) min_y = y;
            if (y > max_y) max_y = y;
        }
    }

    for (int x = min_x; x <= max_x + 1; x+=2){
        bool here = grid[x][0];
        fputc(' ', stream);
        fputc(here ? '_' : ' ', stream);
        fputc(here ? '_' : ' ', stream);
        fputc(' ', stream);
    }
    fputc('\n', stream);
    for (int y = min_y; y <= max_y + 1; y+=2) {
        for (int x = min_x; x <= max_x + 1; x+=2) {
            struct Piece *here = grid[x][y];
            bool nw = x > 0 && y > 0 && grid[x-1][y-1];
            bool ne = x < max_x && y > 0 && grid[x+1][y-1];
            bool se = x < max_x && y < max_y && grid[x+1][y+1];

            fputc(here || nw ? '/' : ' ', stream);
            fputc(' ', stream); // TODO beetles on top
            fputc(here ? PIECE_CHAR[here->player][here->type] : ' ', stream);
            fputc(here || ne ? '\\' : ' ', stream);
            fputc(ne || se ? '_' : ' ', stream); // TODO beetles on top
            fputc(ne || se ? '_' : ' ', stream); // TODO beetles on top
        }
        fputc('\n', stream);
        for (int x = min_x; x <= max_x + 1; x+=2){
            bool here = grid[x][y];
            bool sw = x > 0 && y < max_y && grid[x-1][y+1];
            bool s = y+2 <= max_y && grid[x][y+2];
            struct Piece *se = x < max_x && y < max_y && grid[x+1][y+1] ?
                grid[x+1][y+1] : NULL;

            fputc(here || sw ? '\\' : ' ', stream);
            fputc(here || s ? '_' : ' ', stream); // TODO beetles on top
            fputc(here || s ? '_' : ' ', stream); // TODO beetles on top
            fputc(here || se ? '/' : ' ', stream);
            fputc(' ', stream); // TODO beetles on top
            fputc(se ? PIECE_CHAR[se->player][se->type] : ' ', stream);
        }
        fputc('\n', stream);
    }
}
