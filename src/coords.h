#ifndef COORDS_H
#define COORDS_H


#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>


#define NUM_DIRECTIONS 6
enum Direction {NORTH=0, NORTHEAST, SOUTHEAST, SOUTH, SOUTHWEST, NORTHWEST};

extern const enum Direction OPPOSITE[NUM_DIRECTIONS];


struct Coords {
    uint8_t q;
    uint8_t r;
};


void Coords_move(struct Coords *coords, enum Direction direction);

bool Coords_adjacent(const struct Coords *coords, const struct Coords *other);


enum Direction Direction_rotate(enum Direction direction, int n);


void Coords_print(const struct Coords *coords, FILE *stream);
void Direction_print(enum Direction direction, FILE *stream);


#endif
