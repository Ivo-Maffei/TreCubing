#include "constructPrimes.h"

#include <assert.h>
#include "rand.h"
#include <openssl/bn.h>

const unsigned long availablePrimeSizes[] = {
    256,
    512,
    1024,
    2048,
    3072,
    10000,
    50000,
    70000,
    100000
};

const unsigned long numAvailablePrimes = 9;

// use openssl for generating primes
// TODO: if not safe, we still want p=2 mod 3
void findOpensslPrime(mpz_t p, const unsigned long Nbits, const bool safe) {
    // we must have Nbits < 2^31 to fit an int
    assert(Nbits < INT_MAX);

    int bytelen, nlimbs;
    BIGNUM* ossl_num, *bn_2, *bn_3;

    // check if we are using little endian
    mp_limb_t temp_limb=1;
    assert(((uint8_t*)&temp_limb)[0] & 1);

    ossl_num  = BN_new();
    bn_2 = BN_new();
    bn_3 = BN_new();

    assert(ossl_num && bn_2 && bn_3);
    BN_set_word(bn_2, 2);
    BN_set_word(bn_3, 3);

    // generate the safe prime
    // the NULLs are for callback and requirements on type of prime
    if (!BN_generate_prime_ex(ossl_num, Nbits, safe,  bn_3, bn_2, NULL)) {
	fprintf(stderr, "ERROR with openssl prime generation\n");
	mpz_set_ui(p, 0l);
    }

    // now convert BN to GMP
    // get "dump" BN number into a byte buffer
    nlimbs = (Nbits+63)/64;
    mp_limb_t* rawp = mpz_limbs_write(p, nlimbs);

    // our machine is in little endian, so this works perfect
    bytelen = BN_bn2lebinpad(ossl_num, (uint8_t*)rawp, nlimbs * 8); // BN_bn2bin(ossl_num, (uint8_t*)rawp);
    nlimbs = (bytelen+7)/8; // actual limbs used

    mpz_limbs_finish(p, nlimbs);

    BN_free(ossl_num);
    BN_free(bn_2);
    BN_free(bn_3);
}

// assuming secpar is not crazt high, we just compute a random number of secpar bits and find the next prime
// use this as the basis for q
// MUST ENSURE p=2 mod 3 otherwise cubing is not invertible in ZZ_q^*
void constructPrimePower(mpz_t q, mpz_t b, const unsigned long secpar, const unsigned long N){

    mpz_t p;
    unsigned long k;

    mpz_init(p);

    // get a random number of exactly secpar bits
    findOpensslPrime(p, secpar, false); // gets random prime of secpar bits congruent to 2 modulo 3

    k = mpz_sizeinbase(p, 2); // actual bitsize of p
    k = (N + k -1) / k; // ceil (N/k)

    assert(k>1 && "Prime power is a trivial power");

    mpz_pow_ui(q, p, k); // q = p^k

    // note that an extra multiplication could be needed
    if (mpz_sizeinbase(q, 2) < N - (k/2)) {
	mpz_mul(q, q, p);
	++k; // preserve invariant q = p^k
    }

    // set b to the inverse of 3 mod \phi(q)
    // \phi(q) mod 3 = 2^(k-1) mod 3 = 2^(k-1%2) mod 3 = 1 + (k-1%2) = 2 - (k%2)
    // so 2\phi(q) mod 3 = 2^(k%2) mod 3 = 1 + (k%2)
    // note that (2\phi(q)%3) * \phi(q) = 2 mod 3
    // hence b = (1 + (1+(k%2))\phi(q))/3
    if (b) {
	mpz_divexact(b, q, p); // b <- p^(k-1)
	mpz_sub(b, q, b); // b = p^k - p^(k-1) = p^(k-1) (p-1) = \phi(q)

	mpz_mul_ui(b, b, 1 + (k%2)); // b <- b *  (2\phi(q) mod 3) so that 1 + b = 0 mod 3
	mpz_add_ui(b, b, 1);

	mpz_divexact_ui(b, b, 3);
    }

    mpz_clear(p);
}
