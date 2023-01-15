#ifndef STATE_H
#define STATE_H


#include <stdint.h>


#define NUM_PLAYERS 2
enum Player {P1=0, P2};


#define PLAYER_PIECES 11
#define MAX_PIECES (PLAYER_PIECES * NUM_PLAYERS)
// The +2 is to give a buffer for wrapping if every piece is placed in a line
#define GRID_SIZE (MAX_PIECES + 2)


#define NUM_PIECETYPES 5
enum PieceType {ANT=0, BEETLE, GRASSHOPPER, SPIDER, QUEEN_BEE};
const int PIECE_COUNT[] = {3, 2, 3, 2, 1};


// Way-high estimate for max actions
// An ant on the end of a line of all pieces (greatest surface area)
// would be able to move to 1 + 20*2 + 4 places.
// ants         3 * 1+20*2+4 =  135
// Beetles and grasshoppers can, at most, have one move per direction.
// beetle       2 * 6 =         12
// grasshopper  3 * 6 =         18
// Assuming some ideal layout where the spider has max choices of moves
// each time (but can't backgrack, as normal)
// spider       2 * 4*3*3 =     72
// Most queen can move is 4 (if in a ring)
// queen bee    1 * 4 =         4
// Ideal estimate for places, ignoring color restriction
// places =                     72
// TOTAL =                      313
#define MAX_ACTIONS 313


struct Coords {
    uint8_t q;
    uint8_t r;
};


struct Action {
    struct Coord from;
    struct Coord to;
};


struct Piece {
    enum PieceType type;
    enum Player player;

    struct Coords coords;
    
    // Points to a beetle on top of the piece
    struct Piece *on_top;
};


struct State {
    // Core information
    struct Piece pieces[NUM_PLAYERS][PLAYER_PIECES];
    uint_fast8_t piece_count[NUM_PLAYERS];
    enum Player turn;

    // Derived information
    struct Piece *grid[GRID_SIZE][GRID_SIZE];
    uint_fast8_t hand[NUM_PLAYERS][NUM_PIECETYPES];

    struct Action *actions[MAX_ACTIONS];
    uint_fast8_t action_count;
};


#endif
