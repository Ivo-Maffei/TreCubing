#ifndef CONSTRUCT_PRIMES_H
#define CONSTRUCT_PRIMES_H

#include <stdbool.h> // for bool
#include <gmp.h>

// construct a safe prime p, and if b!= NULL, set b to be the inverse of 3 mod p-1
void constructSafePrime(mpz_t p, mpz_t b, const unsigned long N);

// construct a prime power q = p^k such that p is a prime of (at least) secpar bits and
// if b!= NULL, set b to be the inverse of 3 mod \phi(q) = p^(k-1)(p-1)
void constructPrimePower(mpz_t q, mpz_t b, const unsigned long secpar, const unsigned long N);

// returns a random prime of Nbits bits, if safe is set, a safe prime is returned
void findOpensslPrime(mpz_t p, const unsigned long Nbits, const bool safe);

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
