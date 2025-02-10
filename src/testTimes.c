#include "testTimes.h"

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h> // for memcpy

//#define NDEBUG
#include <assert.h>


#include "enc.h"
#include "rand.h"
#include "constructPrimes.h"
#include "hash.h"


#define TIMER_INIT(name, iters)  \
    clock_t start_ ## name, end_ ## name; \
    unsigned long* const allTime_ ## name = (unsigned long*) malloc(iters*sizeof(unsigned long)); \
    double avgTime_ ## name, stdTime_ ## name; \
    const unsigned long nIters_ ## name = iters; \
    unsigned long current_iter_ ## name = 0l;

#define TIMER_TIME(name, work, fp) { \
    start_ ## name = clock();\
    work; \
    end_ ## name = clock();\
    assert(current_iter_ ## name < nIters_ ## name); \
    allTime_ ## name[current_iter_ ## name] = end_ ## name - start_ ## name; \
    fprintf(fp, #name " took %.3fms\n", allTime_ ## name[current_iter_ ## name]/(double)CLOCKS_PER_SEC*1000.0); \
    ++current_iter_ ## name;}

#define TIMER_REPORT(name, fp) { \
    avgTime_ ## name = 0.0; \
    for (size_t i =0; i < nIters_ ## name; ++i) avgTime_ ## name += allTime_ ## name[i]; \
    avgTime_ ## name /= (double)(nIters_ ## name); \
    stdTime_ ## name = 0.0; \
    for (size_t i=0; i < nIters_ ## name; ++i) stdTime_ ## name += (allTime_ ## name[i] - avgTime_ ## name)*(allTime_ ## name[i] - avgTime_ ## name); \
    stdTime_ ## name /= (double)(nIters_ ## name); \
    stdTime_ ## name = sqrt(stdTime_ ## name); \
    fprintf(fp, "mean and std " #name " time %.9fms (%.9fms)\n", avgTime_ ## name/(double)CLOCKS_PER_SEC*1000.0, stdTime_ ## name/(double)CLOCKS_PER_SEC*1000.0); \
    free(allTime_ ## name); }


// little helper to write a line of equal sings
void writelineSep(FILE * const fileptr) {
    for (int i = 0; i < 50; ++i) fprintf(fileptr, "=");
    fprintf(fileptr, "\n\n");
}

void writeTimestamp(FILE* const fileptr) {
    time_t rawtime;
    time(&rawtime);
    fprintf(fileptr, "Current localtime: %s\n", asctime(localtime(&rawtime)));
}


void testModuloConstruction(const unsigned long N, const unsigned long secpar, const int nIters, FILE* const fileptr) {
    writeTimestamp(fileptr);
    writeTimestamp(stdout);
    fprintf(fileptr, "Testing construction of moduli of %lu bits\n", N);

    TIMER_INIT(PrimePower, nIters);

    mpz_t q;
    mpz_init2(q, N+5);

    for (int i=0; i < nIters; ++i){
	if (secpar) TIMER_TIME(PrimePower, constructPrimePower(q, NULL, secpar, N), fileptr);
    }

    TIMER_REPORT(PrimePower, fileptr);

    writelineSep(fileptr);

    mpz_clear(q);
}

// picks a random message m, computes m^3 mod p and then (m^3)^b mod p
// prints out the time taken and stores the time as well
// repeate for nIters and outputs means and std
void testTimesSq(mpz_t p, const mpz_t b, const unsigned long N, const int nIters, FILE * const fileptr) {

    writeTimestamp(fileptr);
    writeTimestamp(stdout);
    fprintf(fileptr, "Testing cubing using a prime of %lu bits\n", N);

    // variable for timing
    TIMER_INIT(Cubing, nIters);
    TIMER_INIT(CubeRoot, nIters);
    TIMER_INIT(FastSqGMP, nIters);

    // variables for the computation
    mpz_t m, m2, c;
    const size_t nlimbs = mpz_size(p);
    mp_limb_t *mptr, *tptr;
    const mp_limb_t *cptr, *pptr;
    size_t tsize;

    // initialse variables
    mpz_init2(m, N+1);
    mpz_init2(m2, N+1);
    mpz_init2(c, N+1);

    const unsigned long nSquarings = mpz_sizeinbase(b, 2) - 1l;
    fprintf(fileptr, "Number of squarings: %lu\n", nSquarings);

    // compute size for scratch space for low level exponentiation
    tsize = mpn_binvert_itch(nlimbs);
    tsize = (tsize > 2*nlimbs) ? tsize : 2*nlimbs;
    tsize += nlimbs; // just to be sure

    // allocate memory for low level exponentiation
    tptr = (mp_limb_t*) malloc( tsize*sizeof(mp_limb_t));
    assert(tptr);

    for (int i=0; i < nIters; ++i){

	randomMessage(m, p);

	// quick tests
	// m in Z^*_p
        mpz_gcd(c, m, p);
	if (mpz_cmp_ui(c, 1l)){
	    fprintf(stderr, "ERROR: message is not in correct group\n");
	}

	// encryption
	TIMER_TIME(Cubing, mpz_powm_ui(c, m, 3l, p), fileptr);

        // decryption
	TIMER_TIME(CubeRoot, mpz_powm(m2, c, b, p), fileptr);

	//check correctness
        if (mpz_cmp(m2, m) != 0){
            fprintf(fileptr, "ERROR: cube root is wrong!!!! -> %i\n", mpz_cmp(m2, m) );
	    fprintf(stderr, "ERROR: cube root failed\n");
        }

	// try using the mpn_powm_2exp
	mptr = mpz_limbs_modify(m, nlimbs);
	cptr = mpz_limbs_read(c);
	pptr = mpz_limbs_read(p);
	TIMER_TIME(FastSqGMP, mpn_powm_2exp(mptr, cptr, mpz_size(c), nSquarings, pptr, nlimbs, tptr), fileptr);

    }// end for loop


    TIMER_REPORT(Cubing, fileptr);
    TIMER_REPORT(CubeRoot, fileptr);
    TIMER_REPORT(FastSqGMP, fileptr);
    fprintf(fileptr, "Number of squarings: %lu\n", nSquarings);

    writelineSep(fileptr);

    free(tptr);
    mpz_clears(m, m2, c, NULL);
    clearRandomness();
}


void testTimesEnc(const size_t N, const size_t secpar, const int nIters, FILE * const fileptr) {

    writeTimestamp(fileptr);
    writeTimestamp(stdout);

    fprintf(fileptr, "Testing AES256-OFB encryption with a modulo of size %lu\n", N);

    mpz_t m, M;

    TIMER_INIT(streamCipher, nIters);

    mpz_inits(m, M, NULL);

    if (0 != initialiseOpenSSL()){
	fprintf(stderr, "Failed to initialise OpenSSL\n");
	fprintf(fileptr, "Failed to initialise OpenSSL\n");
	goto free;
    }

    for (int i = 0; i < nIters; ++i) {
	constructPrimePower(M, NULL, secpar, N);

	randomMessage(m, M);

	// construct random key of 256 bits (32 bytes or 4 64-bits words)
	// and 128 bits of IV (so 48 bytes total)
	uint8_t key[48];
	for (int i = 0; i < 6; ++i) {
	    uint64_t t = xorshf64(); // get a random 64-bit word
	    memcpy(key + i*8, &t, 8);
	}

	TIMER_TIME(streamCipher, streamCipher(m, m, M, key), fileptr);

    }

    TIMER_REPORT(streamCipher, fileptr);

    writelineSep(fileptr);
 free:
    mpz_clears(m, M, NULL);
    clearRandomness();
    cleanOpenSSL();
}

void testTimesHash(const mpz_t M, const int nIters, FILE* const fileptr){

    writeTimestamp(fileptr);
    writeTimestamp(stdout);

    fprintf(fileptr, "Testing hasing (SHA3-256) with modulo of size %lu\n", mpz_sizeinbase(M, 2));

    mpz_t m;
    uint8_t* digest;

    TIMER_INIT(hashing, nIters);

    mpz_init(m);
    digest = (uint8_t*) malloc(32); // digest is 32 bytes = 256 bits

    if (0 != initialiseHashing() ){
	fprintf(stderr, "Error initialising OpenSSL\n");
	fprintf(fileptr, "Error initialising OpenSSL\n");
	goto free;
    }

    for (int i=0; i < nIters; ++i){
	randomMessage(m, M);

	TIMER_TIME(hashing, hash(digest, m), fileptr);
    }

    TIMER_REPORT(hashing, fileptr);

    writelineSep(fileptr);

 free:
    mpz_clear(m);
    free(digest);
}
