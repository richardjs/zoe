#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "errorcodes.h"
#include "state.h"
#include "stateio.h"


const char PIECE_CHAR[NUM_PLAYERS][NUM_PIECETYPES] = {
    {'A', 'B', 'G', 'S', 'Q'},
    {'a', 'b', 'g', 's', 'q'}
};

char Piece_char(const struct Piece *piece) {
    return PIECE_CHAR[piece->player][piece->type];
}


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
    struct Piece *grid[GRID_SIZE][GRID_SIZE*3];
    memset(grid, 0, sizeof(struct Piece*) * GRID_SIZE*GRID_SIZE*3);
    int min_x = GRID_SIZE - 2;
    int max_x = 0;
    int min_y = GRID_SIZE - 2;
    int max_y = 0;
    for (int q = 0; q < GRID_SIZE; q++) {
        for (int r = 0; r < GRID_SIZE; r++) {
            // https://www.redblobgames.com/grids/hexagons/#conversions-doubled
            int x = q;
            int y = 2*r + q;
            grid[x][y] = state.grid[q][r];

            if (!grid[x][y]) continue;
            if (x < min_x) min_x = x;
            if (x > max_x) max_x = x;
            if (y < min_y) min_y = y;
            if (y > max_y) max_y = y;
        }
    }

    for (int x = min_x; x <= max_x + 1; x += 2){
        bool here = grid[x][0];
        fputc(' ', stream);
        fputc(here ? '_' : ' ', stream);
        fputc(here ? '_' : ' ', stream);
        fputc(' ', stream);
    }
    fputc('\n', stream);
    for (int y = min_y; y <= max_y + 1; y += 2) {
        for (int x = min_x; x <= max_x + 1; x += 2) {
            struct Piece *here = grid[x][y];
            bool nw = x > 0 && y > 0 && grid[x-1][y-1];
            bool ne = x < max_x && y > 0 && grid[x+1][y-1];
            bool se = x < max_x && y < max_y && grid[x+1][y+1];

            fputc(here || nw ? '/' : ' ', stream);
            fputc(' ', stream); // TODO beetles on top
            fputc(here ? Piece_char(here) : ' ', stream);
            fputc(here || ne ? '\\' : ' ', stream);
            fputc(ne || se ? '_' : ' ', stream); // TODO beetles on top
            fputc(ne || se ? '_' : ' ', stream); // TODO beetles on top
        }
        fputc('\n', stream);
        for (int x = min_x; x <= max_x + 1; x += 2){
            bool here = grid[x][y];
            bool sw = x > 0 && y < max_y && grid[x-1][y+1];
            bool s = y+2 <= max_y && grid[x][y+2];
            struct Piece *se = x < max_x && y < max_y && grid[x+1][y+1] ? grid[x+1][y+1] : NULL;

            fputc(here || sw ? '\\' : ' ', stream);
            fputc(here || s ? '_' : ' ', stream); // TODO beetles on top
            fputc(here || s ? '_' : ' ', stream); // TODO beetles on top
            fputc(here || se ? '/' : ' ', stream);
            fputc(' ', stream); // TODO beetles on top
            fputc(se ? Piece_char(se) : ' ', stream);
        }
        fputc('\n', stream);
    }
}


bool Piece_from_string(struct Piece *piece, const char string[]) {
    memset(piece, 0, sizeof(struct Piece));

    for (int p = 0; p < NUM_PLAYERS; p++) {
        for (int t = 0; t < NUM_PIECETYPES; t++) {
            if (string[0] == PIECE_CHAR[p][t]) {
                piece->player = p;
                piece->type = t;
                goto type_parse_success;
            }
        }
    }

    return false;

    type_parse_success:

    piece->coords.q = tolower(string[1]) - 'a';
    piece->coords.r = tolower(string[2]) - 'a';

    return piece->coords.q < MAX_PIECES && piece->coords.r < MAX_PIECES;
}


void State_from_string(struct State *state, const char string[]) {
    State_new(state);

    if (!string[0]) {
        return;
    }

    int i;
    for (i = 0; i < STATE_STRING_SIZE; i += 3) {
        if (string[i] - '1' < NUM_PLAYERS) {
            break;
        }

        struct Piece piece;
        if (!Piece_from_string(&piece, &string[i])) {
            fprintf(stderr, "Error parsing piece at position %d: %s\n", i, string);
            exit(ERROR_PIECE_PARSE);
        }

        state->pieces[piece.player][state->piece_count[piece.player]] = piece;
        struct Piece *state_piece = &state->pieces[piece.player][state->piece_count[piece.player]++];

        if (state->grid[piece.coords.q][piece.coords.r]) {
            if (piece.type != BEETLE) {
                fprintf(stderr,
                    "Piece '%c' at coords (%c, %c) can't be on top of the hive: %s\n",
                    string[i], string[i+1], string[i+2], string
                );
                exit(ERROR_ILLEGAL_PIECE_ON_HIVE);
            }

            struct Piece *p = state->grid[piece.coords.q][piece.coords.r];
            while (p->on_top) {
                p = p->on_top;
            }
            p->on_top = state_piece;
        } else {
            state->grid[piece.coords.q][piece.coords.r] = state_piece;
        }
    }

    state->turn = string[i] - '1';

    State_derive(state);
}
