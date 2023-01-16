/* This file is for code that operates on states, but isn't required for
 * core operations. Any state code used by state.c should be in state.c.
 * Any code dealing with state I/O should go in stateio.c.
 */

#ifndef STATEUTL_H
#define STATEUTL_H


#include "state.h"


void Piece_pieces_above(const struct Piece *piece, struct Piece *above[]);


#endif
