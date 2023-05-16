#include "coords.h"
#include "uhp.h"

int main()
{
    // TODO don't use seed while we're hammering out UHP bugs
    // time_t seed = time(NULL);
    // srand(seed);

    init_coords();

    uhp_loop();

    return 0;
}
