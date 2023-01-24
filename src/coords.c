#include "coords.h"
#include "state.h"


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
            coords->q--;
            break;
    }
}
