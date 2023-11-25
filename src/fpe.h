#ifndef FPE_H
#define FPE_H

#include <gmp.h>

void thorp(mpz_t x, const mpz_t N, const unsigned long R);

void inverse_thorp(mpz_t x, const mpz_t N, const unsigned long R);

void swapOrNot(mpz_t x, const mpz_t N, const unsigned long R);

void sometimesRecurse(mpz_t x, const mpz_t N, const unsigned long R);

#endif
