#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "state.h"
#include "uhp.h"

const char IDENTIFIER[] = "Zo\u00e9 v1.0";

struct State state;

int move_number = 1;

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

void print_movestring(const struct Action *action)
{
    // TODO
}

void error(char message[])
{
    printf("err %s\n", message);
}

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
    for (int i = 0; i < state.action_count; i++) {
        // TODO
    }
}

void uhp_loop()
{
    State_new(&state);

    info();

    char* line;

    while (1) {
        int n = scanf("%m[^\n]", &line);

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
