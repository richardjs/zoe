#include "bench.h"
#include "coords.h"


int main() {
    {
        struct Coords coords;
        coords.q = 0;
        coords.r = 0;
        BENCHMARK_START("Coords_move", 10000000)
        Coords_move(&coords, SOUTHEAST);
        BENCHMARK_END
    }

    return 0;
}
