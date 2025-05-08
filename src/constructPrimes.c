#include "constructPrimes.h"

#include <assert.h>
#include "rand.h"
#include <openssl/bn.h>

#define N_BEST_PRIMES 49091941

uint32_t* dbprimes = NULL;

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

void clearPrimesDB(){
    if (dbprimes) free(dbprimes);
    dbprimes=NULL;
}

int loadPrimesDB(){
    if (dbprimes) return 0; // already initialised

    FILE *db;
    dbprimes = malloc(N_BEST_PRIMES*sizeof(uint32_t));
    if (dbprimes == NULL){
	fprintf(stderr, "ERROR failed to initialise prime database\n");
	return 1;
    }
    db = fopen("bestprimes.32b", "rb");
    if (!db) return 1;

    size_t e = fread(dbprimes, sizeof(*dbprimes), N_BEST_PRIMES, db);
    if (e != N_BEST_PRIMES) {
	fprintf(stderr, "ERROR read less primes: %lu instead of %lu\n", e, N_BEST_PRIMES);
	return 1;
    }
    fclose(db);
    return 0;
}

// p = 4133481*2^{5002} - 1
void constructSafePrime5k(mpz_t p) {
    mpz_set_ui(p, 1l);
    mpz_mul_2exp(p, p, 5002l);
    mpz_mul_ui(p, p, 4133481l);
    mpz_sub_ui(p, p, 1l);
}

// p = 9402702309*10^{3000} - 1
// bits: 9999
void constructSafePrime10k(mpz_t p) {
    mpz_set_ui(p, 10l);
    mpz_pow_ui(p, p, 3000l);
    mpz_mul_ui(p, p, 9402702309l);
    mpz_sub_ui(p, p, 1l);
}

// p = 29553033*2^{19991} - 1
// bits: 20016
void constructSafePrime20k(mpz_t p) {
    mpz_set_ui(p, 1l);
    mpz_mul_2exp(p, p, 19991l);
    mpz_mul_ui(p, p, 29553033l);
    mpz_sub_ui(p, p, 1l);
}

// p = 415365* 2^{30053} - 1
// bits: 30072
void constructSafePrime30k(mpz_t p) {
    mpz_set_ui(p, 1l);
    mpz_mul_2exp(p, p, 30053l);
    mpz_mul_ui(p, p, 415365l);
    mpz_sub_ui(p, p, 1l);
}

// p = 774951567* 2^{40961} - 1
// bits: 40991
void constructSafePrime40k(mpz_t p) {
    mpz_set_ui(p, 1l);
    mpz_mul_2exp(p, p, 40961l);
    mpz_mul_ui(p, p, 774951567l);
    mpz_sub_ui(p, p, 1l);
}

// p = 4127632557* 2^{50002} - 1
// bits: 50034
void constructSafePrime50k(mpz_t p) {
    mpz_set_ui(p, 1l);
    mpz_mul_2exp(p, p, 50002l);
    mpz_mul_ui(p, p, 4127632557l);
    mpz_sub_ui(p, p, 1l);
}

// p = 3714089895285* 2^{60001} - 1
// bits: 60043
void constructSafePrime60k(mpz_t p) {
    mpz_set_ui(p, 1l);
    mpz_mul_2exp(p, p, 60001l);
    mpz_mul_ui(p, p, 3714089895285l);
    mpz_sub_ui(p, p, 1l);
}

// p = 256685167*2^{70002} - 1
// bits: 70034
void constructSafePrime70k(mpz_t p) {
    mpz_set_ui(p, 1l);
    mpz_mul_2exp(p, p, 70002l);
    mpz_mul_ui(p, p, 2566851867l);
    mpz_sub_ui(p, p, 1l);
}

// p = 1213822389 * 2^{81132} - 1
// bits:
void constructSafePrime80k(mpz_t p) {
    mpz_set_ui(p, 1l);
    mpz_mul_2exp(p, p, 81132l);
    mpz_mul_ui(p, p, 1213822389l);
    mpz_sub_ui(p, p, 1l);
}

// p = 3364553235* 2^{88889} - 1
// bits:
void constructSafePrime90k(mpz_t p) {
    mpz_set_ui(p, 1l);
    mpz_mul_2exp(p, p, 88889l);
    mpz_mul_ui(p, p, 3364553235l);
    mpz_sub_ui(p, p, 1l);
}

// p = 35909079387* 2^{100001} - 1
// bits:
void constructSafePrime100k(mpz_t p) {
    mpz_set_ui(p, 1l);
    mpz_mul_2exp(p, p, 100001l);
    mpz_mul_ui(p, p, 35909079387l);
    mpz_sub_ui(p, p, 1l);
}

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


// get 32 bit primes using the saved database
int get32bprimes(uint32_t* primes, const int numprimes){

    int fail = loadPrimesDB();
    if (fail) return fail;

    uint32_t place;

    for (int i=0; i<numprimes; ++i){
	place = rand() % N_BEST_PRIMES;

	primes[i] = dbprimes[place];
	// if we already found such prime, then decrease i so that we replace it
	for (int j=0; j<i; ++j) if (primes[j] == primes[i]) { --i; break;}
    }

    return 0;
}

void constructmPower(mpz_t q, mpz_t b, const int nprimes, const unsigned long N) {

    mp_bitcnt_t k;
    uint32_t* ps = malloc(nprimes*sizeof(uint32_t));

    if (ps == NULL){
	fprintf(stderr, "ERRROR initialising temporary primes\n");
	return;
    }

    if (get32bprimes(ps, nprimes) != 0) {
	fprintf(stderr, "Error with prime generations\n");
	return;
    }

    mpz_set_ui(q, 1);

    for (int i =0; i<nprimes; ++i) {
	mpz_mul_ui(q, q, (unsigned long)(ps[i]));
    }

    k = N - mpz_sizeinbase(q, 2);

    mpz_mul_2exp(q, q, k);

    // now we must compute the inverse of 3 mod \phi(q)
    // recall b = (1+l*\phi(q))/3; so we need l*\phi(q) = 2 mod 3
    // recall \phi(q) = 2^{k-1} prod_{i=1}^{nprimes} (p_i-1)
    // recall p_i = 2 mod 3; so \phi(q) = 2^{k-1} mod 3
    // hence b = [1 + (2-(k-1)%2)*\phi(q)]/3 = [1 + (1+k%2)*\phi(q)]/3

    if (b) {
	// compute \phi(q)
	mpz_set_ui(b, 1);
	for (int i=0; i<nprimes; ++i) {
	    mpz_mul_ui(b, b, (unsigned long)(ps[i]-1));
	} // b = \phi(m)
	mpz_mul_2exp(b, b, k-1); // b = \phi(m2^k)

	mpz_mul_ui(b, b, 1+(k%2));
	mpz_add_ui(b, b, 1);

	if (mpz_fdiv_ui(b, 3) != 0) fprintf(stderr, "b is not divisible by 3\n");
	mpz_divexact_ui(b, b, 3);
    }

    free(ps);
}

// construct a prime and stores it in p
// p should already be initialised with the correct size
void constructSafePrime(mpz_t p, mpz_t b, const unsigned long N) {
    switch (N) {
    case 5000l:
	constructSafePrime5k(p);
	break;
    case 10000l:
	constructSafePrime10k(p);
	break;
    case 20000l:
	constructSafePrime20k(p);
	break;
    case 30000l:
	constructSafePrime30k(p);
	break;
    case 40000l:
	constructSafePrime40k(p);
	break;
    case 50000l:
	constructSafePrime50k(p);
	break;
    case 60000l:
	constructSafePrime60k(p);
	break;
    case 70000l:
	constructSafePrime70k(p);
	break;
    case 80000l:
	constructSafePrime80k(p);
	break;
    case 90000l:
	constructSafePrime90k(p);
	break;
    case 100000l:
	constructSafePrime100k(p);
	break;
    default:
	if (N < 3500l){
	    findOpensslPrime(p, N, true);
	}
	else {
	    fprintf(stderr, "constructing primes of size %lu is not supported\n", N);
	    assert(0);
	}
    }

    // set b to the inverse of 3 mod p-1
    // this is (2p-1)/3
    if (b) {
	mpz_mul_2exp(b, p, 1);
	mpz_sub_ui(b, b, 1);
	mpz_divexact_ui(b, b, 3);
    }
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
