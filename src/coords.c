#include "coords.h"

#include <stdio.h>
#include <stdlib.h>

#include "state.h"

const enum Direction OPPOSITE[NUM_DIRECTIONS] = { SOUTH, SOUTHWEST, NORTHWEST,
    NORTH, NORTHEAST, SOUTHEAST };

struct Coords coords_neighbors_map[GRID_SIZE][GRID_SIZE][NUM_DIRECTIONS];
bool coords_adjacent_map[GRID_SIZE][GRID_SIZE][GRID_SIZE][GRID_SIZE];

void Coords_calc_move(struct Coords* coords, enum Direction direction)
{
    switch (direction) {
    case NORTH:
        if (coords->r == 0)
            coords->r = GRID_SIZE - 1;
        else
            coords->r--;
        break;

    case NORTHEAST:
        if (coords->q == GRID_SIZE - 1)
            coords->q = 0;
        else
            coords->q++;
        if (coords->r == 0)
            coords->r = GRID_SIZE - 1;
        else
            coords->r--;
        break;

    case SOUTHEAST:
        if (coords->q == GRID_SIZE - 1)
            coords->q = 0;
        else
            coords->q++;
        break;

    case SOUTH:
        if (coords->r == GRID_SIZE - 1)
            coords->r = 0;
        else
            coords->r++;
        break;

    case SOUTHWEST:
        if (coords->q == 0)
            coords->q = GRID_SIZE - 1;
        else
            coords->q--;
        if (coords->r == GRID_SIZE - 1)
            coords->r = 0;
        else
            coords->r++;
        break;

    case NORTHWEST:
        if (coords->q == 0)
            coords->q = GRID_SIZE - 1;
        else
            coords->q--;
        break;
    }
}

void Coords_map_move(struct Coords* coords, enum Direction direction)
{
    *coords = coords_neighbors_map[coords->q][coords->r][direction];
}

void Coords_move(struct Coords* coords, enum Direction direction)
{
    Coords_map_move(coords, direction);
}

// https://www.redblobgames.com/grids/hexagons/#distances-axial
unsigned int Coords_distance(const struct Coords* coords, const struct Coords* other)
{
    return (abs(coords->q - other->q)
               + abs(coords->q + coords->r - other->q - other->r)
               + abs(coords->r - other->r))
        / 2;
}

bool Coords_calc_adjacent(const struct Coords* coords, const struct Coords* other)
{
    for (enum Direction d = 0; d < NUM_DIRECTIONS; d++) {
        struct Coords c = *coords;
        Coords_move(&c, d);
        if (c.q == other->q && c.r == other->r) {
            return true;
        }
    }

    return false;
}

bool Coords_map_adjacent(const struct Coords* coords, const struct Coords* other)
{
    return coords_adjacent_map[coords->q][coords->r][other->q][other->r];
}

bool Coords_adjacent(const struct Coords* coords, const struct Coords* other)
{
    return Coords_calc_adjacent(coords, other);
}

void init_coords()
{
    for (int q = 0; q < GRID_SIZE; q++) {
        for (int r = 0; r < GRID_SIZE; r++) {
            for (int d = 0; d < NUM_DIRECTIONS; d++) {
                coords_neighbors_map[q][r][d].q = q;
                coords_neighbors_map[q][r][d].r = r;
                Coords_calc_move(&coords_neighbors_map[q][r][d], d);
            }

            for (int oq = 0; oq < GRID_SIZE; oq++) {
                for (int or = 0; or < GRID_SIZE; or ++) {
                    struct Coords coords;
                    coords.q = q;
                    coords.r = r;
                    struct Coords other;
                    other.q = oq;
                    other.r = or ;
                    coords_adjacent_map[q][r][oq][or] = Coords_calc_adjacent(&coords, &other);
                }
            }
        }
    }
}

enum Direction
Direction_rotate(enum Direction direction, int n)
{
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

void Coords_print(const struct Coords* coords, FILE* stream)
{
    fprintf(stream, "%c%c\n", coords->q + 'a', coords->r + 'a');
}

void Direction_print(enum Direction direction, FILE* stream)
{
    switch (direction) {
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
