#ifndef CONSTRUCT_PRIMES_H
#define CONSTRUCT_PRIMES_H

#include <stdbool.h> // for bool
#include <stdio.h>
#include <gmp.h>

// construct a prime power q = p^k such that p is a prime of (at least) secpar bits and
// if b!= NULL, set b to be the inverse of 3 mod \phi(q) = p^(k-1)(p-1)
void constructPrimePower(mpz_t q, mpz_t b, const unsigned long secpar, const unsigned long N);

extern const unsigned long numAvailablePrimes;

extern const unsigned long availablePrimeSizes[30]; // assuming we never exceed length 30

#endif
