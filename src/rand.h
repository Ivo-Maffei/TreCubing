#ifndef RAND_H
#define RAND_H

#include <gmp.h>
#include <stdint.h>

// random number of exactly nbits bits
void randomNumber(mpz_t n, const unsigned long nbits);

// random function that returns a random message m in ZZ_p^*
void randomMessage(mpz_t m, const mpz_t p);

// frees any space allocated for randomness
void clearRandomness();

// set a seed for xorshf64 and nextRand
void setSeed(uint64_t seed);

uint64_t xorshf64();

uint64_t nextRand();


#endif
