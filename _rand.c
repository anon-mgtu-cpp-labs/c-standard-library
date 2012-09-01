#include "_rand.h"
#include "stdint.h"

/* Mersenne Twister magic numbers */
#define _RAND_N        624
#define _RAND_M        397
#define _RAND_U        0x80000000L
#define _RAND_L        0x7fffffffL

/* Mersenne Twister state and seed storage */
static int32_t rand_state[_RAND_N];
static int32_t rand_next;

/*
    @description:
        Seeds the Mersenne Twister state.
*/
void _srand(unsigned seed)
{
    int32_t i;

    rand_state[0] = seed;

    for (i = 1; i < _RAND_N; ++i)
        rand_state[i] = (1812433253L * (rand_state[i - 1] ^ (rand_state[i - 1] >> 30)) + i);
}

/*
    @description:
        Generates a random number in the range of [0,RAND_MAX)
        using the Mersenne Twister algorithm.
*/
int _rand(void)
{
    int32_t y, i;

    /* Refill rand_state if exhausted */
    if (rand_next == _RAND_N) {
        rand_next = 0;

        for (i = 0; i < _RAND_N - 1; i++) {
            y = (rand_state[i] & _RAND_U) | rand_state[i + 1] & _RAND_L;
            rand_state[i] = rand_state[(i + _RAND_M) % _RAND_N] ^ (y >> 1) ^ ((y & 1L) ? 0x9908b0dfL : 0x0UL);
        }

        y = (rand_state[_RAND_N - 1] & _RAND_U) | rand_state[0] & _RAND_L;
        rand_state[_RAND_N - 1] = rand_state[_RAND_M - 1] ^ (y >> 1) ^ ((y & 0x1UL) ? 0x9908b0dfL : 0L);
    }

    y = rand_state[rand_next++];

    /* Add a little extra mixing */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);

    return y;
}