#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "state.h"
#include "uhp.h"

#define MOVESTRING_SIZE 9

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

int coords_to_piecestring(const struct Coords* coords, char piecestring[])
{
    char player_char = 'x';
    char piece_char = 'x';
    int piece_num = 1;
    if (coords->q == PLACE_ACTION) {
        player_char = UHP_PLAYER_CHAR[state.turn];
        piece_char = UHP_PIECE_CHAR[coords->r];
    } else {
        struct Piece* piece = state.grid[coords->q][coords->r];

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

    size_t size = 0;
    piecestring[size++] = player_char;
    piecestring[size++] = piece_char;

    if (piece_char != 'Q') {
        piecestring[size++] = piece_num + '0';
    }

    return size;
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

void action_to_movestring(const struct Action* action, char movestring[])
{
    if (action->from.q == PASS_ACTION) {
        strcpy(movestring, "pass");
        return;
    }

    size_t size = 0;

    size += coords_to_piecestring(&action->from, movestring);

    if (move_number == 1 && state.turn == P1) {
        movestring[size++] = '\0';
        return;
    }

    movestring[size++] = ' ';

    // Move on top of the hive
    if (state.grid[action->to.q][action->to.r]) {
        size += coords_to_piecestring(&action->from, movestring);
        movestring[size++] = '\0';
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
        size += coords_to_piecestring(&reference, movestring);
        movestring[size++] = UHP_DIRECTION_CHAR[reference_dir];
        break;
    case SOUTH:
    case SOUTHWEST:
    case NORTHWEST:
        movestring[size++] = UHP_DIRECTION_CHAR[reference_dir];
        size += coords_to_piecestring(&reference, movestring);
        break;
    }

    movestring[size++] = '\0';
}

int parse_movestring(const char movestring[], struct Action *action)
{
    char test_movestring[MOVESTRING_SIZE];
    for (int i = 0; i < state.action_count; i++) {
        struct Action *action = &state.actions[i];
        action_to_movestring(action, test_movestring);

        if (strcmp(movestring, test_movestring) == 0) {
            return i;
        }
    }

    return -1;
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

void play(char movestring[])
{
    struct Action action;
    int i = parse_movestring(movestring, &action);
}

void validmoves()
{
    char movestring[MOVESTRING_SIZE];
    action_to_movestring(&state.actions[0], movestring);
    printf("%s", movestring);
    for (int i = 1; i < state.action_count; i++) {
        printf(";");
        action_to_movestring(&state.actions[i], movestring);
        printf("%s", movestring);
    }
    printf("\n");
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
