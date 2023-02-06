/* This file is for code that operates on states, but isn't required for
 * core operations. Any state code used by state.c should be in state.c.
 * Any code dealing with state I/O should go in stateio.c.
 */

#ifndef STATEUTL_H
#define STATEUTL_H


#include <stdbool.h>


#include "state.h"


struct Piece *State_top_piece(const struct State *state, unsigned int q, unsigned r);

void Piece_pieces_above(const struct Piece *piece, struct Piece *above[]);

// Following the basic interface of strcmp, these return true if a
// difference is found, and false if the structs are (functionally)
// identical. We can't just memcmp because of all the pointers.
bool Piece_compare(const struct Piece *piece, const struct Piece *other);
bool State_compare(const struct State *state, const struct State *other, bool debug_print);

#endif
