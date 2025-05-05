#ifndef TEST_TIMES_H
#define TEST_TIMES_H

#include <gmp.h>
#include <stdbool.h>
#include <stdio.h>

void testModuloConstruction(const unsigned long N, const unsigned long nprimes, const unsigned long secpar, const int nIters, FILE* const fileptr);

// picks a random message m, computes m^3 mod p and then (m^3)^b mod p
// prints out the time taken and stores the time as well
// repeate for nIters and outputs means and std
void testTimesSq(mpz_t p, const mpz_t b, const unsigned long N, const int nIters, FILE * const fileptr);

// test stream cipher encryption AES256-OFB with cycle walking
// we generate new moduli at each iteration to avoid biases in the modulo
void testTimesEnc(const size_t N, const size_t secpar, const int nIters, FILE * const fileptr);

// test the times it takes to hash random messages modulo M
void testTimesHash(const mpz_t M, const int nIters, FILE* const fileptr);

#endif
