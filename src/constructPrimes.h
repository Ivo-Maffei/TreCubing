#ifndef CONSTRUCT_PRIMES_H
#define CONSTRUCT_PRIMES_H

#include <gmp.h>

// construct a prime and stores it in p
// p should already be initialised with the correct size
void constructSafePrime(mpz_t p, const unsigned long N);

// construct a prime power q = p^k such that p is a prime of (at least) secpar bits and
// q has at least N bits
void constructPrimePower(mpz_t q, mpz_t p, const unsigned long secpar, const unsigned long N);

void findOpensslSafePrime(mpz_t p, const unsigned long Nbits);
void findPseudoSafePrime(mpz_t p, const unsigned long Nbits);
void findAlmostSafePrime(mpz_t p, const unsigned long Nbits);

extern const unsigned long numAvailablePrimes;

extern const unsigned long availablePrimeSizes[30]; // assuming we never exceed length 30

// AVALIABLE SIZES (i.e. valid inputs for N rather than actual size):
// - 1k
// - 2k
// - 5k
// - 10k
// - 20k
// - 30k
// - 40k
// - 50k
// - 60k
// - 70k
// - 80k
// - 90k
// - 100k

#endif
