#ifndef ENC_H
#define ENC_H

#include <stdio.h>
#include <gmp.h>
#include <stdint.h>

int streamCipher(mpz_t res, const mpz_t m, const mpz_t M, const uint8_t *key);

void cleanOpenSSL();

int initialiseOpenSSL();

#endif
