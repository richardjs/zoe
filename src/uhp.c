#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "state.h"
#include "uhp.h"

const char IDENTIFIER[] = "Zo\u00e9 v1.0";

const char UHP_PLAYER_CHAR[NUM_PLAYERS] = { 'w', 'b' };
const char UHP_PIECE_CHAR[NUM_PIECETYPES] = { 'A', 'B', 'G', 'S', 'Q' };

// UHP assumes pointy-top hexes, and Zoé uses flat-top. Convert by
// rotating Zoé pieces clockwise (or UHP counterclockwise)--so NORTH
// becomes / and NORTHEAST becomes -.
const char UHP_DIRECTION_CHAR[NUM_DIRECTIONS] = { '/', '-', '\\', '/', '-', '\\' };

struct State state;

int move_number = 1;

void error(char message[])
{
    printf("err %s\n", message);
}

void print_piece_string(const struct Coords* coords)
{
    char player_char = 'x';
    char piece_char = 'x';
    int piece_num = 1;
    if (coords->q == PLACE_ACTION) {
        player_char = UHP_PLAYER_CHAR[state.turn];
        piece_char = UHP_PIECE_CHAR[coords->r];
    } else {
        struct Piece* piece = state.grid[coords->q][coords->r];
        if (!piece) {
            return;
        }

        player_char = UHP_PLAYER_CHAR[piece->player];
        piece_char = UHP_PIECE_CHAR[piece->type];

        for (int i = 0; i < state.piece_count[piece->player]; i++) {
            if (piece == &state.pieces[piece->player][i]) {
                break;
            }
            if (piece->type == state.pieces[piece->player][i].type) {
                piece_num++;
            }
        }
    }

    printf("%c%c", player_char, piece_char);
    if (piece_char != 'Q') {
        printf("%d", piece_num);
    }
}

void print_gamestring()
{
    State_new(&state);
    move_number = 1;

    printf("Base;");

    switch (state.result) {
    case NO_RESULT:
        if (move_number == 1 && state.turn == P1) {
            printf("NotStarted;");
        } else {
            printf("InProgress;");
        }
        break;
    case P1_WIN:
        printf("WhiteWins;");
        break;
    case P2_WIN:
        printf("BlackWins;");
        break;
    case DRAW:
        printf("Draw;");
        break;
    }

    printf("%s[%d]", state.turn == P1 ? "White" : "Black", move_number);
}

void print_movestring(const struct Action* action)
{
    if (action->from.q == PASS_ACTION) {
        printf("pass");
        return;
    }

    print_piece_string(&action->from);

    if (move_number == 1 && state.turn == P1) {
        return;
    }

    printf(" ");

    // Move on top of the hive
    if (state.grid[action->to.q][action->to.r]) {
        print_piece_string(&action->to);
        return;
    }

    struct Coords reference;
    enum Direction reference_dir;
    for (reference_dir = 0; reference_dir < NUM_DIRECTIONS; reference_dir++) {
        reference = action->to;
        Coords_move(&reference, reference_dir);
        if (state.grid[reference.q][reference.r]) {
            break;
        }
    }

    switch (reference_dir) {
    case NORTH:
    case NORTHEAST:
    case SOUTHEAST:
        print_piece_string(&reference);
        printf("%c", UHP_DIRECTION_CHAR[reference_dir]);
        break;
    case SOUTH:
    case SOUTHWEST:
    case NORTHWEST:
        printf("%c", UHP_DIRECTION_CHAR[reference_dir]);
        print_piece_string(&reference);
        break;
    }
}

// Commands

void info()
{
    printf("id %s\n", IDENTIFIER);
    puts("ok");
}

void newgame(char* args)
{
    State_new(&state);

    if (args) {
        // TODO
    }

    print_gamestring();
    printf("\n");
    printf("ok\n");
}

void play(char move[])
{
}

void validmoves()
{
    print_movestring(&state.actions[0]);
    for (int i = 1; i < state.action_count; i++) {
        printf(";");
        print_movestring(&state.actions[i]);
    }
}

// Input loop

void uhp_loop()
{
    State_new(&state);

    info();

    char* line;

    while (1) {
        int n = scanf("%m[^\n]", &line);

        if (n == EOF) {
            break;
        }

        if (n == 0) {
            getchar();
            continue;
        }

        char* command;
        char* args;
        sscanf(line, "%ms %ms", &command, &args);

        if (!strcmp(command, "info")) {
            info();
        } else if (!strcmp(command, "newgame")) {
            newgame(args);
        } else if (!strcmp(command, "play")) {
            play(args);
        } else if (!strcmp(command, "validmoves")) {
            validmoves();
        } else if (!strcmp(command, "exit")) {
            free(line);
            free(command);
            free(args);
            getchar();
            break;
        } else {
            error("invalid command");
        }

        free(line);
        free(command);
        free(args);
        getchar();
    }
}
