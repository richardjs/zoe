#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "state.h"
#include "uhp.h"

#define PIECESTRING_SIZE 3
#define DESTSTRING_SIZE (PIECESTRING_SIZE + 2)
#define MOVESTRING_SIZE (PIECESTRING_SIZE + 1 + DESTSTRING_SIZE)

#define HISTORY_CHUNK_SIZE 100

const char IDENTIFIER[] = "Zo\u00e9 v1.0";

const char UHP_PLAYER_CHAR[NUM_PLAYERS] = { 'w', 'b' };
const char UHP_PIECE_CHAR[NUM_PIECETYPES] = { 'A', 'B', 'G', 'S', 'Q' };

// UHP assumes pointy-top hexes, and Zoé uses flat-top. Convert by
// rotating Zoé pieces clockwise (or UHP counterclockwise)--so NORTH
// becomes / and NORTHEAST becomes -.
const char UHP_DIRECTION_CHAR[NUM_DIRECTIONS] = { '/', '-', '\\', '/', '-', '\\' };

struct HistoryMove {
    char movestring[MOVESTRING_SIZE];
};

struct State state;
struct HistoryMove* history = NULL;
int move_number;
int allocated_size;

void reset_game_data()
{
    State_new(&state);
    free(history);
    history = malloc(sizeof(struct HistoryMove) * HISTORY_CHUNK_SIZE);
    allocated_size = HISTORY_CHUNK_SIZE;
    move_number = 0;
}

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
    printf("Base;");

    switch (state.result) {
    case NO_RESULT:
        if (move_number == 0) {
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

    printf("%s[%d]", state.turn == P1 ? "White" : "Black", move_number / 2 + 1);

    for (int i = 0; i < move_number; i++) {
        printf(";%s", history[i].movestring);
    }
}

enum Direction action_to_movestring(
    const struct Action* action,
    char movestring[],
    enum Direction starting_dir)
{
    // TODO add a variable for a starting value for reference_dir, so
    // we can continue searching for other possible move strings
    if (action->from.q == PASS_ACTION) {
        strcpy(movestring, "pass");
        // Meaningless return value
        return NORTHWEST;
    }

    size_t size = 0;

    size += coords_to_piecestring(&action->from, movestring);

    if (move_number == 0) {
        movestring[size++] = '\0';
        // Meaningless return value
        return NORTHWEST;
    }

    movestring[size++] = ' ';

    // Move on top of the hive
    if (state.grid[action->to.q][action->to.r]) {
        size += coords_to_piecestring(&action->from, movestring);
        movestring[size++] = '\0';
        // Meaningless return value
        return NORTHWEST;
    }

    struct Coords reference;
    enum Direction reference_dir;
    for (reference_dir = starting_dir; reference_dir < NUM_DIRECTIONS; reference_dir++) {
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
        size += coords_to_piecestring(&reference, &movestring[size]);
        movestring[size++] = UHP_DIRECTION_CHAR[reference_dir];
        break;
    case SOUTH:
    case SOUTHWEST:
    case NORTHWEST:
        movestring[size++] = UHP_DIRECTION_CHAR[reference_dir];
        size += coords_to_piecestring(&reference, &movestring[size]);
        break;
    }

    movestring[size++] = '\0';
    return reference_dir;
}

int parse_movestring(const char movestring[])
{
    // TODO this only works for one representation of a move
    char test_movestring[MOVESTRING_SIZE];
    for (int i = 0; i < state.action_count; i++) {
        struct Action* action = &state.actions[i];
        enum Direction dir = NORTH;
        while (dir < NORTHWEST) {
            dir = action_to_movestring(action, test_movestring, dir) + 1;

            if (strcmp(movestring, test_movestring) == 0) {
                return i;
            } else {
            }
        }
    }

    return -1;
}

// Note that this doesn't update history
bool act_movestring(const char movestring[])
{
    int actioni = parse_movestring(movestring);

    if (actioni < 0) {
        error("invalid move");
        return false;
    }

    State_act(&state, &state.actions[actioni]);
    return true;
}

// Commands

void info()
{
    printf("id %s\n", IDENTIFIER);
    printf("ok\n");
}

void newgame(const char* args)
{
    reset_game_data();

    if (args) {
        // TODO
    }

    print_gamestring();
    printf("\n");
    printf("ok\n");
}

void play(const char movestring[])
{
    if (movestring == NULL) {
        error("no movestring");
        return;
    }

    if (!act_movestring(movestring)) {
        return;
    }

    strcpy(history[move_number].movestring, movestring);
    move_number++;
    if (move_number == allocated_size) {
        allocated_size += HISTORY_CHUNK_SIZE;
        history = realloc(history, allocated_size * sizeof(struct HistoryMove));
    }

    print_gamestring();
    printf("\n");
    printf("ok\n");
}

void validmoves()
{
    char movestring[MOVESTRING_SIZE];
    action_to_movestring(&state.actions[0], movestring, NORTH);
    printf("%s", movestring);
    for (int i = 1; i < state.action_count; i++) {
        printf(";");
        action_to_movestring(&state.actions[i], movestring, NORTH);
        printf("%s", movestring);
    }
    printf("\n");
    printf("ok\n");
}

void undo(const char args[])
{
    int to_undo = 1;
    if (args != NULL) {
        to_undo = atoi(args);
    }

    if (to_undo == 0 || to_undo > move_number) {
        error("invalid number to undo");
        return;
    }

    move_number -= to_undo;
    State_new(&state);
    for (int i = 0; i < move_number; i++) {
        act_movestring(history[i].movestring);
    }

    print_gamestring();
    printf("\n");
    printf("ok\n");
}

// Input loop

void uhp_loop()
{
    reset_game_data();

    info();

    char* line = NULL;
    size_t line_size = 0;

    while (1) {
        ssize_t read = getline(&line, &line_size, stdin);

        if (read == -1) {
            break;
        }

        if (strlen(line) == 1) {
            continue;
        }

        char* command;
        char* args;
        sscanf(line, "%ms %m[^\n]", &command, &args);

        if (!strcmp(command, "info")) {
            info();
        } else if (!strcmp(command, "newgame")) {
            newgame(args);
        } else if (!strcmp(command, "play")) {
            play(args);
        } else if (!strcmp(command, "pass")) {
            play("pass");
        } else if (!strcmp(command, "validmoves")) {
            validmoves();
        } else if (!strcmp(command, "bestmove")) {
            // TODO
        } else if (!strcmp(command, "undo")) {
            undo(args);
        } else if (!strcmp(command, "options")) {
            // TODO
        } else if (!strcmp(command, "exit")) {
            free(command);
            free(args);
            break;
        } else {
            error("invalid command");
        }

        free(command);
        free(args);
    }

    free(line);
}
