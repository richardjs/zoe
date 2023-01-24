#ifndef COORDS_H
#define COORDS_H


#include <stdint.h>


#define NUM_DIRECTIONS 6
enum Direction {NORTH=0, NORTHEAST, SOUTHEAST, SOUTH, SOUTHWEST, NORTHWEST};


struct Coords {
    uint8_t q;
    uint8_t r;
};


void Coords_go(struct Coords *coords, enum Direction direction);


#endif
