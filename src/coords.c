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
