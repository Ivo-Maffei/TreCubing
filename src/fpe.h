#ifndef FPE_H
#define FPE_H

#include <gmp.h>
#include <stdint.h>

void thorp(mpz_t c, const mpz_t m, const mpz_t N, const unsigned long R, const uint8_t* key);

void inverse_thorp(mpz_t m, const mpz_t c, const mpz_t N, const unsigned long R, const uint8_t* key);

void swapOrNot(mpz_t x, const mpz_t N, const unsigned long R);

void sometimesRecurse(mpz_t x, const mpz_t N, const unsigned long R);

#endif
