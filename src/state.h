#ifndef STATE_H
#define STATE_H


#include <stdint.h>

#include "coords.h"


#define NUM_PLAYERS 2

#define PLAYER_PIECES 11
#define MAX_PIECES (PLAYER_PIECES * NUM_PLAYERS)
#define GRID_SIZE 24

#define NUM_PIECETYPES 5
#define NUM_ANTS 3
#define NUM_BEETLES 2
#define NUM_GRASSHOPPERS 3
#define NUM_SPIDERS 2
#define NUM_QUEEN_BEES 1

/* Way-high estimate for max actions:
 * An ant on the end of a line of all pieces (greatest surface area)
 * would be able to move to 1 + 20*2 + 4 places.
 * ants         3 * 1+20*2+4 =  135
 * Beetles and grasshoppers can, at most, have one move per direction.
 * beetle       2 * 6 =         12
 * grasshopper  3 * 6 =         18
 * Assuming some ideal layout where the spider has max choices of moves
 * each time (but can't backgrack, as normal)
 * spider       2 * 4*3*3 =     72
 * Most queen can move is 4 (if in a ring)
 * queen bee    1 * 4 =         4
 * Ideal estimate for places, ignoring color restriction
 * places =                     72
 * TOTAL =                      313
 */
#define MAX_ACTIONS 313

#define MAX_ABOVE (NUM_PLAYERS*NUM_BEETLES)
#define MAX_STACK_SIZE (1 MAX_STACK_SIZE)

// If Action.from.q == PLACE_ACTION, it means the action is a place, with
// Action.r = PieceType
#define PLACE_ACTION (2 << 6)


enum Player {P1=0, P2};
enum PieceType {ANT=0, BEETLE, GRASSHOPPER, SPIDER, QUEEN_BEE};


struct Action {
    struct Coords from;
    struct Coords to;
};


struct Piece {
    // Core information
    enum PieceType type;
    struct Coords coords;
    // Points to a beetle on top of the piece
    struct Piece *on_top;

    // Derived information
    enum Player player;
};


struct State {
    // Core information
    struct Piece pieces[NUM_PLAYERS][PLAYER_PIECES];
    uint_fast8_t piece_count[NUM_PLAYERS];

    enum Player turn;

    // Derived information
    struct Piece *grid[GRID_SIZE][GRID_SIZE];

    struct Action actions[MAX_ACTIONS];
    uint_fast8_t action_count;

    uint_fast8_t hands[NUM_PLAYERS][NUM_PIECETYPES];

    uint_fast8_t neighbor_count[NUM_PLAYERS][GRID_SIZE][GRID_SIZE];
};


void State_new(struct State *state);
void State_derive(struct State *state);

void State_act(struct State *state, const struct Action *action);


#endif
