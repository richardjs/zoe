#ifndef STATEIO_H
#define STATEIO_H


#include <stdio.h>

#include "state.h"


void State_normalize(struct State *state);
void State_print(const struct State *state, FILE *stream);


#endif
