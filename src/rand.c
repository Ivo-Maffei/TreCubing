#include "rand.h"

#include <stdlib.h>
#include <string.h>

#include <stdio.h>

// intial seed for xorshft
#define INITIAL_SEED 88172645463325252l

// GMP random staet
static gmp_randstate_t *randomState = NULL;


static uint64_t xorshfstate = INITIAL_SEED;
static uint64_t currentRandomWord, randomBitsUsed=64;

void randomMessage(mpz_t m, const mpz_t q) {
    if (randomState == NULL) {
	randomState = (gmp_randstate_t*) malloc(sizeof(gmp_randstate_t));
	gmp_randinit_default(*randomState);
    }

    mpz_t temp;
    mpz_init(temp);

    do {
	mpz_urandomm(m, *randomState, q); // random message in the range 0 ... q
	mpz_gcd(temp, m, q); // temp <- gcd(m,q)
    } while (mpz_cmp_ui(temp, 1) != 0); // temp != 1 <-> m not in ZZ^*_q

    mpz_clear(temp);
}

// frees any space allocated for randomness
void clearRandomness() {
    if (randomState){
	gmp_randclear(*randomState);
	free(randomState);
	randomState = NULL;
    }
}

void setSeed(uint64_t seed) {
    randomBitsUsed = 64; // this forces nextRand to use a new word
    xorshfstate = seed;
}

// for the random bits we use a fast PRNG
// this is a xor shift generator form the Marsaglia's family
uint64_t xorshf64() {
    xorshfstate ^= xorshfstate << 13;
    xorshfstate ^= xorshfstate >> 7;
    xorshfstate ^= xorshfstate << 17;
    return xorshfstate;
}
