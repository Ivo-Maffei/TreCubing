#ifndef TEST_TIMES_H
#define TEST_TIMES_H

#include <gmp.h>
#include <stdbool.h>
#include <stdio.h>

// picks a random message m, computes m^3 mod p and then (m^3)^b mod p
// prints out the time taken and stores the time as well
// repeate for nIters and outputs means and std
void testTimesSq(mpz_t p, const unsigned long N, const int nIters, FILE * const fileptr);

// picks random numbers modulo p and encrypts them with R rounds of various FPE schemes
void testTimesFpe(const mpz_t p, const unsigned long R, const bool t, const bool son, const bool sr, const int nIters, FILE * const fileptr);

// test the overall delay and open
void testTimesAll(const mpz_t p, const unsigned long R, const unsigned long C, const int nIters, FILE * const fileptr);

// test LLL reduction
//void testLLL(const mpz_t p, const unsigned long s, const unsigned long n, const int nIters);

// test bothEnds encryption
void testTimesEnc(const mpz_t p, const int nIters, FILE * const fileptr);

#endif
