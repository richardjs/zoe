#ifndef STATE_H
#define STATE_H

#include <stdbool.h>
#include <stdint.h>

#include "coords.h"

#define NUM_PLAYERS 2

#define NUM_PIECETYPES 5
#define NUM_ANTS 3
#define NUM_BEETLES 2
#define NUM_GRASSHOPPERS 3
#define NUM_SPIDERS 2
#define NUM_QUEEN_BEES 1

#define PLAYER_PIECES (NUM_ANTS + NUM_BEETLES + NUM_GRASSHOPPERS + NUM_SPIDERS + NUM_QUEEN_BEES)
#define MAX_PIECES (PLAYER_PIECES * NUM_PLAYERS)
#define GRID_SIZE (MAX_PIECES + 2)

#define ANT_INDEX 0
#define BEETLE_INDEX (ANT_INDEX + NUM_ANTS)
#define GRASSHOPPER_INDEX (BEETLE_INDEX + NUM_BEETLES)
#define SPIDER_INDEX (GRASSHOPPER_INDEX + NUM_GRASSHOPPERS)
#define QUEEN_INDEX (SPIDER_INDEX + NUM_SPIDERS)

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
#define MAX_PIECE_MOVES 135
#define MAX_QUEEN_MOVES 4
#define MAX_BEETLE_MOVES (NUM_DIRECTIONS * 2)

// It's actually lower than this; 26 is if you have all your pieces in
// a line with no opposite color pieces anywhere
#define MAX_PLACE_SPOTS 26

#define MAX_ABOVE (NUM_PLAYERS * NUM_BEETLES)
#define MAX_STACK_SIZE (1 MAX_STACK_SIZE)

// If Action.from.q == PLACE_ACTION, it means the action is a place, with
// Action.r = PieceType
#define PLACE_ACTION (GRID_SIZE + 1)
// TODO IN_HAND will probably replace PLACE_ACTION
#define IN_HAND (GRID_SIZE + 1)

// TODO support passing when no moves are possible
#define PASS_ACTION (PLAYER_PIECES + 1)

#define SPIDER_MOVES 3

enum Player {
    P1 = 0,
    P2
};

enum PieceType {
    ANT = 0,
    BEETLE,
    GRASSHOPPER,
    SPIDER,
    QUEEN_BEE
};

// Index for state.pieces
const uint_fast8_t TYPE_INDEX[NUM_PIECETYPES] = {
    ANT_INDEX,
    BEETLE_INDEX,
    GRASSHOPPER_INDEX,
    SPIDER_INDEX,
    QUEEN_INDEX
};

const uint_fast8_t TYPE_END[NUM_PIECETYPES] = {
    ANT_INDEX + NUM_ANTS,
    BEETLE_INDEX + NUM_BEETLES,
    GRASSHOPPER_INDEX + NUM_GRASSHOPPERS,
    SPIDER_INDEX + NUM_SPIDERS,
    QUEEN_INDEX + NUM_QUEEN_BEES
};

const uint_fast8_t TYPE_COUNT[NUM_PIECETYPES] = {
    NUM_ANTS,
    NUM_BEETLES,
    NUM_GRASSHOPPERS,
    NUM_SPIDERS,
    NUM_QUEEN_BEES
};

enum Result {
    P1_WIN = 0,
    P2_WIN,
    DRAW,
    NO_RESULT
};

struct Action {
    uint_fast8_t piecei;
    struct Coords to;
};

struct Piece {
    // Core information
    enum PieceType type;
    struct Coords coords;
    // Points to a beetle on top of the piece
    struct Piece* on_top;

    // Derived information
    enum Player player;
};

struct State {
    // Core information
    struct Piece pieces[NUM_PLAYERS][PLAYER_PIECES];
    uint_fast8_t piece_count[NUM_PLAYERS];

    enum Player turn;

    // Derived information
    struct Piece* grid[GRID_SIZE][GRID_SIZE];

    struct Action actions[MAX_ACTIONS];
    uint_fast16_t action_count;

    struct Action* winning_action;

    struct Action* queen_moves[MAX_QUEEN_MOVES];
    uint_fast8_t queen_move_count;

    struct Action* piece_moves[PLAYER_PIECES][MAX_PIECE_MOVES];
    uint_fast16_t piece_move_count[PLAYER_PIECES];

    struct Action* queen_adjacent_actions[MAX_ACTIONS];
    uint_fast16_t queen_adjacent_action_count;

    struct Action* queen_away_moves[MAX_ACTIONS];
    uint_fast16_t queen_away_move_count;

    struct Action* queen_nearby_actions[MAX_ACTIONS];
    uint_fast16_t queen_nearby_action_count;

    struct Action* pin_moves[MAX_ACTIONS];
    uint_fast16_t pin_move_count;

    struct Action* unpin_moves[MAX_ACTIONS];
    uint_fast16_t unpin_move_count;

    struct Action* queen_pin_moves[MAX_ACTIONS];
    uint_fast16_t queen_pin_move_count;

    struct Action* beetle_moves[MAX_BEETLE_MOVES];
    uint_fast8_t beetle_move_count;

    uint_fast8_t hands[NUM_PLAYERS][NUM_PIECETYPES];

    uint_fast8_t neighbor_count[NUM_PLAYERS][GRID_SIZE][GRID_SIZE];

    bool cut_points[GRID_SIZE][GRID_SIZE];
    uint_fast8_t cut_point_count[NUM_PLAYERS];

    enum Result result;
};

void State_new(struct State* state);
void State_derive(struct State* state);

void State_copy(const struct State* source, struct State* dest);

void State_act(struct State* state, const struct Action* action);

int State_hex_neighbor_count(const struct State* state, const struct Coords* coords);

#endif
