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

void randomNumber(mpz_t n, const unsigned long nbits) {
    if (randomState == NULL) {
	randomState = (gmp_randstate_t*) malloc(sizeof(gmp_randstate_t));
	gmp_randinit_default(*randomState);
    }

    // we get a random number up to 2^(n-1)-1 (i.e. at most n-1 bits)
    // and then add 2^(n-1) to get a number of n bits
    mpz_t temp;
    mpz_init(temp);
    mpz_urandomb(temp, *randomState, nbits-1);

    mpz_set_ui(n, 1l);
    mpz_mul_2exp(n, n, nbits-1);
    mpz_add(n, n, temp);

    mpz_clear(temp);
}

void randomMessage(mpz_t m, const mpz_t p) {
    if (randomState == NULL) {
	randomState = (gmp_randstate_t*) malloc(sizeof(gmp_randstate_t));
	gmp_randinit_default(*randomState);
    }

    mpz_t temp;
    mpz_init(temp);
    mpz_sub_ui(temp, p, 1l); // temp <- p-1
    mpz_urandomm(m, *randomState, temp); // random message in the range 0 ... p-2

    mpz_add_ui(m, m, 1l); // random message in the range 1 ... p-1

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

// this function returns the next random bit
// we use this to minimise the calls to xorshf64
uint64_t nextRand() {
    currentRandomWord >>= 1l; // shift right by 1
    ++randomBitsUsed; // increase number of used bits

    if (randomBitsUsed >= 64){ // all bits used
	randomBitsUsed = 0;
	currentRandomWord = xorshf64();
    }
    return currentRandomWord & 1l;
}
