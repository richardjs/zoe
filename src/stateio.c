#include "stateio.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "coords.h"
#include "errorcodes.h"
#include "state.h"
#include "stateutil.h"

// Space between the edge of grid and the first piece in a normalized state
#define NORMALIZATION_GAP 1

const char PIECE_CHAR[NUM_PLAYERS][NUM_PIECETYPES] = {
    { 'A', 'B', 'G', 'S', 'Q' },
    { 'a', 'b', 'g', 's', 'q' }
};

char Piece_char(const struct Piece* piece)
{
    return PIECE_CHAR[piece->player][piece->type];
}

void State_translate_grid(struct State* state, enum Direction direction)
{
    for (int p = 0; p < NUM_PLAYERS; p++) {
        for (int i = 0; i < state->piece_count[p]; i++) {
            struct Piece* piece = &state->pieces[p][i];
            Coords_move(&piece->coords, direction);
        }
    }
    State_derive(state);
}

void State_normalize(struct State* state)
{
    if (state->piece_count[P1] == 0 && state->piece_count[P2] == 0) {
        // Nothing to normalize
        return;
    }

    // Move pieces out of the NORMALIZATION_GAP
    bool piece_in_gap;
    do {
        piece_in_gap = false;
        for (int q = 0; q < GRID_SIZE; q++) {
            for (int r = 0; r < NORMALIZATION_GAP; r++) {
                if (state->grid[q][r] != NULL) {
                    piece_in_gap = true;
                    break;
                }
            }
        }
        if (piece_in_gap) {
            State_translate_grid(state, NORTH);
        }
    } while (piece_in_gap);
    do {
        piece_in_gap = false;
        for (int r = 0; r < GRID_SIZE; r++) {
            for (int q = 0; q < NORMALIZATION_GAP; q++) {
                if (state->grid[q][r] != NULL) {
                    piece_in_gap = true;
                    break;
                }
            }
        }
        if (piece_in_gap) {
            State_translate_grid(state, NORTHWEST);
        }
    } while (piece_in_gap);

    // Elimanate any wrapping by shifting everything off the far edges
    bool piece_on_edge;
    do {
        piece_on_edge = false;
        for (int q = 0; q < GRID_SIZE; q++) {
            if (state->grid[q][GRID_SIZE - 1] != NULL) {
                piece_on_edge = true;
                break;
            }
        }
        if (piece_on_edge) {
            State_translate_grid(state, NORTH);
        }
    } while (piece_on_edge);
    do {
        piece_on_edge = false;
        for (int r = 0; r < GRID_SIZE; r++) {
            if (state->grid[GRID_SIZE - 1][r] != NULL) {
                piece_on_edge = true;
                break;
            }
        }
        if (piece_on_edge) {
            State_translate_grid(state, NORTHWEST);
        }
    } while (piece_on_edge);

    // Now that there's no wrapping, shift everything to the near edges
    /// (leaving a gap of 1 hex, for moves to that side)
    piece_on_edge = false;
    while (!piece_on_edge) {
        for (int q = 0; q < GRID_SIZE; q++) {
            if (state->grid[q][NORMALIZATION_GAP] != NULL) {
                piece_on_edge = true;
                break;
            }
        }
        if (!piece_on_edge) {
            State_translate_grid(state, NORTH);
        }
    }
    piece_on_edge = false;
    while (!piece_on_edge) {
        for (int r = 0; r < GRID_SIZE; r++) {
            if (state->grid[NORMALIZATION_GAP][r] != NULL) {
                piece_on_edge = true;
                break;
            }
        }
        if (!piece_on_edge) {
            State_translate_grid(state, NORTHWEST);
        }
    }
}

void State_print(const struct State* s, FILE* stream)
{
    // TODO non-normalized beetle stacking doesn't print correctly
    // e.g. abbgbcaccQdcBdc1

    struct State state = *s;
    State_normalize(&state);

    // Convert to double-height coordinate space; see
    // https://www.redblobgames.com/grids/hexagons/#coordinates-doubled
    struct Piece* grid[GRID_SIZE][GRID_SIZE * 3];
    memset(grid, 0, sizeof(struct Piece*) * GRID_SIZE * GRID_SIZE * 3);
    int min_x = GRID_SIZE;
    int max_x = NORMALIZATION_GAP;
    int min_y = GRID_SIZE;
    int max_y = NORMALIZATION_GAP;
    for (int q = 0; q < GRID_SIZE - 1; q++) {
        for (int r = 0; r < GRID_SIZE - 1; r++) {
            // https://www.redblobgames.com/grids/hexagons/#conversions-doubled
            int x = q;
            int y = 2 * r + q;
            // Adjust for NORMALIZATION_GAP so that it's not included the output
            grid[x][y] = state.grid[q + NORMALIZATION_GAP][r + NORMALIZATION_GAP];

            if (!grid[x][y])
                continue;
            if (x < min_x)
                min_x = x;
            if (x > max_x)
                max_x = x;
            if (y < min_y)
                min_y = y;
            if (y > max_y)
                max_y = y;
        }
    }

    // If an odd column is the highest, add an undraw row above it (that
    // would have an even row as the highest). This is because the
    // following code assumes hexes in odd columns will always have a
    // hex to the northwest.
    if (min_y % 2 == 1) {
        min_y -= 1;
    }

    for (int x = min_x; x <= max_x + 1; x += 2) {
        bool here = grid[x][min_y];
        fputc(' ', stream);
        fputc(here ? '_' : ' ', stream);
        fputc(here ? '_' : ' ', stream);
        fputc(' ', stream);
    }
    fputc('\n', stream);
    for (int y = min_y; y <= max_y + 1; y += 2) {
        // Hexes take up two terminal lines
        // Line 1
        for (int x = min_x; x <= max_x + 1; x += 2) {
            struct Piece* here = grid[x][y];
            struct Piece* nw = (x > 0 && y > 0) ? grid[x - 1][y - 1] : NULL;
            struct Piece* ne = (x < max_x && y > 0) ? grid[x + 1][y - 1] : NULL;
            bool se = x < max_x && y < max_y && grid[x + 1][y + 1];

            struct Piece* above_here[MAX_ABOVE];
            Piece_pieces_above(here, above_here);
            struct Piece* above_ne[MAX_ABOVE];
            Piece_pieces_above(ne, above_ne);
            struct Piece* above_nw[MAX_ABOVE];
            Piece_pieces_above(nw, above_nw);

            fputc(above_nw[3]    ? Piece_char(above_nw[3])
                    : here || nw ? '/'
                                 : ' ',
                stream);
            fputc(above_here[0] ? Piece_char(above_here[0]) : ' ', stream);
            fputc(here ? Piece_char(here) : ' ', stream);
            fputc(here || ne ? '\\' : ' ', stream);
            fputc(above_ne[1]  ? Piece_char(above_ne[1])
                    : ne || se ? '_'
                               : ' ',
                stream);
            fputc(above_ne[2]  ? Piece_char(above_ne[2])
                    : ne || se ? '_'
                               : ' ',
                stream);
        }
        fputc('\n', stream);
        // Line 2
        for (int x = min_x; x <= max_x + 1; x += 2) {
            struct Piece* here = grid[x][y];
            bool sw = x > 0 && y < max_y && grid[x - 1][y + 1];
            bool s = y + 2 <= max_y && grid[x][y + 2];
            struct Piece* se = (x < max_x && y < max_y) ? grid[x + 1][y + 1] : NULL;

            struct Piece* above_here[MAX_ABOVE];
            Piece_pieces_above(here, above_here);
            struct Piece* above_se[MAX_ABOVE];
            Piece_pieces_above(se, above_se);

            fputc(here || sw ? '\\' : ' ', stream);
            fputc(above_here[1] ? Piece_char(above_here[1])
                    : here || s ? '_'
                                : ' ',
                stream);
            fputc(above_here[2] ? Piece_char(above_here[2])
                    : here || s ? '_'
                                : ' ',
                stream);
            fputc(above_here[3]  ? Piece_char(above_here[3])
                    : here || se ? '/'
                                 : ' ',
                stream);
            fputc(above_se[0] ? Piece_char(above_se[0]) : ' ', stream);
            fputc(se ? Piece_char(se) : ' ', stream);
        }
        fputc('\n', stream);
    }
}

bool Piece_from_string(struct Piece* piece, const char string[])
{
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

    return piece->coords.q < GRID_SIZE && piece->coords.r < GRID_SIZE;
}

void State_from_string(struct State* state, const char string[])
{
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
        struct Piece* state_piece = &state->pieces[piece.player][state->piece_count[piece.player]++];

        if (state->grid[piece.coords.q][piece.coords.r]) {
            if (piece.type != BEETLE) {
                fprintf(stderr,
                    "Piece '%c' at coords (%c, %c) can't be on top of the "
                    "hive: %s\n",
                    string[i],
                    string[i + 1],
                    string[i + 2],
                    string);
                exit(ERROR_ILLEGAL_PIECE_ON_HIVE);
            }

            struct Piece* p = state->grid[piece.coords.q][piece.coords.r];
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

void State_to_string(const struct State* s, char string[])
{
    struct State state = *s;

    memset(string, 0, sizeof(char) * STATE_STRING_SIZE);
    int c = 0;

    for (int q = 0; q < GRID_SIZE; q += 1) {
        for (int r = 0; r < GRID_SIZE; r += 1) {
            if (!state.grid[q][r])
                continue;

            string[c++] = Piece_char(state.grid[q][r]);
            string[c++] = 'a' + q;
            string[c++] = 'a' + r;

            struct Piece* p = state.grid[q][r]->on_top;
            while (p) {
                string[c++] = Piece_char(p);
                string[c++] = 'a' + q;
                string[c++] = 'a' + r;

                p = p->on_top;
            }
        }
    }

    string[c++] = '1' + state.turn;
}

void Action_from_string(struct Action* action, const char string[])
{
    if (string[0] == 'z') {
        action->from.q = PASS_ACTION;
        return;
    } else if (string[0] == '+') {
        action->from.q = PLACE_ACTION;
        for (int t = 0; t < NUM_PIECETYPES; t++) {
            if (tolower(PIECE_CHAR[P1][t]) == string[1]) {
                action->from.r = t;
                break;
            }
        }
    } else {
        action->from.q = string[0] - 'a';
        action->from.r = string[1] - 'a';
    }

    action->to.q = string[2] - 'a';
    action->to.r = string[3] - 'a';
}

void Action_to_string(const struct Action* action, char string[])
{
    string[4] = '\0';

    if (action->from.q == PASS_ACTION) {
        string[0] = 'z';
        string[1] = 'z';
        string[2] = 'z';
        string[3] = 'z';
        return;
    } else if (action->from.q == PLACE_ACTION) {
        string[0] = '+';
        string[1] = tolower(PIECE_CHAR[P1][action->from.r]);
    } else {
        string[0] = action->from.q + 'a';
        string[1] = action->from.r + 'a';
    }

    string[2] = action->to.q + 'a';
    string[3] = action->to.r + 'a';
}

void Action_print(const struct Action* action, FILE* stream)
{
    char action_string[ACTION_STRING_SIZE];
    Action_to_string(action, action_string);
    fprintf(stream, "%s\n", action_string);
}
