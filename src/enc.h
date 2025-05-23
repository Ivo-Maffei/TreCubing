#ifndef ENC_H
#define ENC_H

#include <stdio.h>
#include <gmp.h>
#include <stdint.h>
#include <stdbool.h>

int streamCipher(mpz_t res, const mpz_t m, const mpz_t M, const uint8_t *key, const bool m2k);

void cleanOpenSSL();

int initialiseOpenSSL();

#endif
