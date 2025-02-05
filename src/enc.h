#ifndef ENC_H
#define ENC_H

#include <stdio.h>
#include <gmp.h>
#include <stdint.h>

int bothEndsEnc(mpz_t res, const mpz_t m, const uint64_t N, const uint8_t *key);

int bothEndsDec(mpz_t res, const mpz_t c, const uint64_t N, const uint8_t *key);

void cycleEnc(mpz_t c, const mpz_t m, const mpz_t p, const uint8_t *key);

void cycleDec(mpz_t m, const mpz_t c, const mpz_t p, const uint8_t *key);

int streamCipher(mpz_t res, const mpz_t m, const mpz_t M, const uint8_t *key);

void cleanOpenSSL();

int initialiseOpenSSL();

#endif
