#include <stdio.h>

#include "coords.h"
#include "state.h"


const enum Direction OPPOSITE[NUM_DIRECTIONS] = {
    SOUTH, SOUTHWEST, NORTHWEST, NORTH, NORTHEAST, SOUTHEAST
};


void Coords_move(struct Coords *coords, enum Direction direction) {
    switch(direction) {
        case NORTH:
            if (coords->r == 0) coords->r = GRID_SIZE - 1;
            else coords->r--;
            break;

        case NORTHEAST:
            if (coords->q == GRID_SIZE - 1) coords->q = 0;
            else coords->q++;
            if (coords->r == 0) coords->r = GRID_SIZE - 1;
            else coords->r--;
            break;

        case SOUTHEAST:
            if (coords->q == GRID_SIZE - 1) coords->q = 0;
            else coords->q++;
            break;

        case SOUTH:
            if (coords->r == GRID_SIZE - 1)  coords->r = 0;
            else coords->r++;
            break;

        case SOUTHWEST:
            if (coords->q == 0) coords->q = GRID_SIZE - 1;
            else coords->q--;
            if (coords->r == GRID_SIZE - 1) coords->r = 0;
            else coords->r++;
            break;

        case NORTHWEST:
            if (coords->q == 0) coords->q = GRID_SIZE - 1;
            else coords->q--;
            break;
    }
}


bool Coords_adjacent(const struct Coords *coords, const struct Coords *other) {
    for (enum Direction d = 0; d < NUM_DIRECTIONS; d++ ) {
        struct Coords c = *coords;
        Coords_move(&c, d);
        if (c.q == other->q &&  c.r == other->r) {
            return true;
        }
    }

    return false;
}


enum Direction Direction_rotate(enum Direction direction, int n) {
    direction += n;
    // The (unsigned) enum needs to be cast to int in these comparisons,
    // else a negative number will be greater than than NUM_DIRECTIONS
    // and not less than 0
    if ((int)direction >= NUM_DIRECTIONS) {
        direction -= NUM_DIRECTIONS;
    } else if ((int)direction < 0) {
        direction += NUM_DIRECTIONS;
    }
    return direction;
}


void Coords_print(const struct Coords *coords, FILE *stream) {
    fprintf(stream, "%c%c\n", coords->q + 'a', coords->r + 'a');
}


void Direction_print(enum Direction direction, FILE *stream) {
    switch(direction) {
        case NORTH:
            fprintf(stream, "north\n");
            break;
        case NORTHEAST:
            fprintf(stream, "northeast\n");
            break;
        case SOUTHEAST:
            fprintf(stream, "southeast\n");
            break;
        case SOUTH:
            fprintf(stream, "south\n");
            break;
        case SOUTHWEST:
            fprintf(stream, "southwest\n");
            break;
        case NORTHWEST:
            fprintf(stream, "northwest\n");
            break;
    }
}
