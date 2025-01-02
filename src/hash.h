#ifndef HASH_H
#define HASH_H

#include <gmp.h>
#include <stdint.h>

// hashes input and save the results inside digest
// returns the number of bytes of digest used
// if digest is NULL, then memory will be allocated
size_t hash(uint8_t* digest, const mpz_t input);

int initialiseHashing();

void cleanHashing();

#endif
