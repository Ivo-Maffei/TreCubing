#ifndef LLL_H
#define LLL_H

#include "flint/fmpz.h"
#include "flint/fmpz_mat.h"

void LLLReduction_mat(fmpz_mat_t L, const unsigned long m, const unsigned long l, const unsigned long n);

void LLL_mat_to_file(const char* filename, const mpz_t p, const unsigned long s, const unsigned long n);

void cvp_to_file(const char* filename, const mpz_t p, const unsigned long s, const unsigned long n);

#endif
