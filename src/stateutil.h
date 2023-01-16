/* This file is for code that operates on states, but isn't required for
 * core operations. Any state code used by state.c should be in state.c.
 * Any code dealing with state I/O should go in stateio.c.
 */

#ifndef STATEUTL_H
#define STATEUTL_H


#include <stdint.h>

#include "state.h"


unsigned int Piece_num_above(const struct Piece *piece);
struct Piece *Piece_above(const struct Piece *piece, unsigned int n);


#endif
