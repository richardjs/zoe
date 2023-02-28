#include <stdlib.h>
#include <time.h>

#include "bench.h"
#include "coords.h"
#include "mcts.h"


void Coords_calc_move(struct Coords *coords, enum Direction direction);
void Coords_map_move(struct Coords *coords, enum Direction direction);


int main() {
    time_t seed = time(NULL);
    srand(seed);

    init_coords();

    enum Direction d[100];
    for (int i = 0; i < 100; i++) {
        d[i] = rand() % NUM_DIRECTIONS;
    }
    {
        struct Coords coords;
        coords.q = 0;
        coords.r = 0;
        BENCHMARK_START("Coords_map_move", 1000000)
        for (int i = 0; i < 100; i++) {
            Coords_map_move(&coords, d[i]);
        }
        BENCHMARK_END
    }
    {
        struct Coords coords;
        coords.q = 0;
        coords.r = 0;
        enum Direction d[100];
        BENCHMARK_START("Coords_calc_move", 1000000)
        for (int i = 0; i < 100; i++) {
            Coords_calc_move(&coords, d[i]);
        }
        BENCHMARK_END
    }

    {
        struct State state;
        State_new(&state);

        struct MCTSOptions options;
        MCTSOptions_default(&options);
        options.iterations = 1;

        struct MCTSResults result;

        BENCHMARK_START("MCTS, 1 iteration", 1000);
        mcts(&state, &result, &options);
        BENCHMARK_END
    }

    {
        struct State state;
        State_new(&state);

        struct MCTSOptions options;
        MCTSOptions_default(&options);
        options.iterations = 10;

        struct MCTSResults result;

        BENCHMARK_START("MCTS, 10 iterations", 100);
        mcts(&state, &result, &options);
        BENCHMARK_END
    }

    return 0;
}
