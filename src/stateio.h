#ifndef STATEIO_H
#define STATEIO_H

#include <stdio.h>

#include "state.h"

// 3 chars per piece * 22 pieces + 1 char for turn + 1 char for terminator
#define STATE_STRING_SIZE (3 * NUM_PLAYERS * PLAYER_PIECES + 1 + 1)
// +qaa or aabc
#define ACTION_STRING_SIZE 5

char Piece_char(const struct Piece* piece);

void State_normalize(struct State* state);
void State_print(const struct State* state, FILE* stream);

void check_state_string(const char string[]);

void State_from_string(struct State* state, const char string[]);
void State_to_string(const struct State* state, char string[]);

void Action_from_string(struct Action* action, const char string[]);
void Action_to_string(const struct Action* action, char string[]);
void Action_print(const struct Action* action, FILE* stream);

#endif
