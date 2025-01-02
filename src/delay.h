#ifndef DELAY_H
#define DELAY_H

#include <stdio.h>
#include <gmp.h>
#include <stdint.h>

// delay message m using cubing modulo p, chained C times with Thorp shuffle using R rounds of shuffling
void delay(mpz_t r, const mpz_t m, const mpz_t p, const unsigned long C,  const unsigned long R, const uint64_t randSeed, const uint8_t *key);

// opens the puzzle c and place the result in m
void open(mpz_t m, const mpz_t c, const mpz_t p, const mpz_t b, const unsigned long C, const unsigned long R, const uint64_t randSeed, const uint8_t *key);

#endif
