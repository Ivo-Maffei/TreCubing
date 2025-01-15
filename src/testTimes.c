#include "testTimes.h"

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h> // for memcpy

#define NDEBUG
#include <assert.h>


#include "fpe.h"
#include "delay.h"
#include "enc.h"
#include "rand.h"
#include "constructPrimes.h"


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

    TIMER_INIT(OSSLsafe, nIters);
    TIMER_INIT(AlmostSafe, nIters);
    TIMER_INIT(PrimePower, nIters);

    mpz_t q;
    mpz_init2(q, N+5);

    for (int i=0; i < nIters; ++i){

	TIMER_TIME(OSSLsafe, findOpensslPrime(q, N, 1), fileptr);
	TIMER_TIME(AlmostSafe, constructAlmostSafePrime(q, NULL, N), fileptr);
	if (secpar) TIMER_TIME(PrimePower, constructPrimePower(q, NULL, secpar, N), fileptr);
    }

    TIMER_REPORT(OSSLsafe, fileptr);
    TIMER_REPORT(AlmostSafe, fileptr);
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
    TIMER_INIT(SqGMP, nIters);
    TIMER_INIT(FastSqGMP, nIters);

    // variables for the computation
    mpz_t m, m2, c, fexp;
    const size_t nlimbs = mpz_size(p);
    mp_limb_t *mptr, *tptr;
    const mp_limb_t *cptr, *pptr;
    size_t tsize;

    // initialse variables
    mpz_init2(m, N+1);
    mpz_init2(m2, N+1);
    mpz_init2(c, N+1);
    mpz_init2(fexp, N+1);

    const unsigned long nSquarings = mpz_sizeinbase(b, 2) - 1l;
    fprintf(fileptr, "Number of squarings: %lu\n", nSquarings);

    // fast exponent
    mpz_set_ui(fexp, 1l);
    mpz_mul_2exp(fexp, fexp, nSquarings);

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

	// repeated squarings only
	TIMER_TIME(SqGMP, mpz_powm(m2, c, fexp, p), fileptr);

	// try using the mpn_powm_2exp
	mptr = mpz_limbs_modify(m, nlimbs);
	cptr = mpz_limbs_read(c);
	pptr = mpz_limbs_read(p);
	TIMER_TIME(FastSqGMP, mpn_powm_2exp(mptr, cptr, mpz_size(c), nSquarings, pptr, nlimbs, tptr), fileptr);

	if (mpn_cmp(mptr, mpz_limbs_read(m2), mpz_size(m2)) != 0) {
	    printf("ERROR\n");
	}
    }// end for loop


    TIMER_REPORT(Cubing, fileptr);
    TIMER_REPORT(CubeRoot, fileptr);
    TIMER_REPORT(SqGMP, fileptr);
    TIMER_REPORT(FastSqGMP, fileptr);
    fprintf(fileptr, "Number of squarings: %lu\n", nSquarings);

    writelineSep(fileptr);

    free(tptr);
    mpz_clears(m, m2, c, fexp, NULL);
    clearRandomness();
}


void testTimesFpe(const mpz_t N, const unsigned long R, const bool t, const bool son, const int nIters, FILE * const fileptr){

    writeTimestamp(fileptr);
    writeTimestamp(stdout);

    fprintf(fileptr, "Testing FPE methods assuming a modulus of %lu bits and with %lu rounds\n", mpz_sizeinbase(N, 2), R);

     // variable for timing
    TIMER_INIT(Th, nIters);
    TIMER_INIT(Sw, nIters);

    mpz_t m;
    int keyLength = (R+7)/8;
    uint8_t* key = (uint8_t*) malloc( keyLength * sizeof(uint8_t));
    assert(key);
    uint64_t randSeed;

    mpz_init2(m, mpz_sizeinbase(N, 2));

    for (int word=0; word < (keyLength+7)/8; ++word) {
	randSeed = xorshf64();
	memcpy(key + word*8, &randSeed, 8);
    }

    for (int i=0; i<nIters; ++i){
	randomMessage(m, N);

	if(t){
	    TIMER_TIME(Th, thorp(m, m, N, R, key), fileptr);
	}

	if(son){
	    TIMER_TIME(Sw, swapOrNot(m, N, R), fileptr);
	}
    }


    TIMER_REPORT(Th, fileptr);
    TIMER_REPORT(Sw, fileptr);

    writelineSep(fileptr);

    mpz_clears(m, NULL);
    clearRandomness();
    free(key);
}


void testTimesAll(const mpz_t p, const mpz_t b, const unsigned long R, const unsigned long C, const int nIters, FILE * const fileptr) {

    writeTimestamp(fileptr);
    writeTimestamp(stdout);

    fprintf(fileptr, "Testing delay enc with a prime of size %lu with a chian of length %lu and using ", mpz_sizeinbase(p, 2), C);
    if (R) {
	fprintf(fileptr, "Thorp with %lu rounds\n", R);
    } else {
	fprintf(fileptr, "both-end encryption\n");
    }

    mpz_t m, m2, c;
    uint64_t randSeed;
    uint8_t *key;
    int keyLength;

    TIMER_INIT(delay, nIters);
    TIMER_INIT(open, nIters);

    mpz_init2(m, mpz_sizeinbase(p, 2));
    mpz_init2(m2, mpz_sizeinbase(p, 2));
    mpz_init2(c, mpz_sizeinbase(p, 2));

    // using Thorp -> create key of R bits
    if (R != 0) keyLength = (R+7)/8; // ceil(R/8)
    // using both-ends encryption -> create key
    else keyLength = 32;
    key = (uint8_t*) malloc(keyLength*sizeof(uint8_t));
    assert(key);

    // create ceil(keyLength / 8) random words of 64 bits
    // and use them as the key
    for (int word=0; word < (keyLength+7)/8; ++word) {
	randSeed = xorshf64();
	memcpy(key + word*8, &randSeed, 8);
    }

    for(int i=0; i < nIters; ++i){

	randomMessage(m, p);

	TIMER_TIME(delay, delay(c, m, p, C, R, key), fileptr);

	TIMER_TIME(open, open(m2, c, p, b, C, R, key), fileptr);

	if(mpz_cmp(m, m2) != 0){
	    printf("ERROR!!!!!! delay open returned wrong message\n");
	    fprintf(fileptr, "ERROR!!!!!! delay open returned wrong message\n");
	    goto free;
	}
    }

    TIMER_REPORT(delay, fileptr);
    TIMER_REPORT(open, fileptr);

    writelineSep(fileptr);

 free:
    mpz_clears(m, m2, c, NULL);
    clearRandomness();
    free(key);
    if (R == 0) {
	cleanOpenSSL();
    }
}


void testTimesEnc(const mpz_t p, const int nIters, FILE * const fileptr) {

    writeTimestamp(fileptr);
    writeTimestamp(stdout);

    fprintf(fileptr, "Testing both-ends encryption with a prime of size %lu\n", mpz_sizeinbase(p, 2));

    mpz_t m, m2, c;

    TIMER_INIT(encryption, nIters);
    TIMER_INIT(decryption, nIters);

    mpz_init2(m, mpz_sizeinbase(p, 2));
    mpz_init2(m2, mpz_sizeinbase(p, 2));
    mpz_init2(c, mpz_sizeinbase(p, 2));

    if (0 != initialiseOpenSSL()){
	fprintf(stderr, "Failed to initialise OpenSSL\n");
	fprintf(fileptr, "Failed to initialise OpenSSL\n");
	goto free;
    }

    for (int i = 0; i < nIters; ++i) {
	randomMessage(m, p);

	// construct random key of 256 bits (32 bytes or 4 64-bits words)
	uint8_t key[32];
	for (int i = 0; i < 4; ++i) {
	    uint64_t t = xorshf64(); // get a random 64-bit word
	    memcpy(key + i*8, &t, 8);
	}

	TIMER_TIME(encryption, cycleEnc(c, m, p, key), fileptr);
	TIMER_TIME(decryption, cycleDec(m2, c, p, key), fileptr);

	if(mpz_cmp(m, m2) != 0) {
	    fprintf(stderr, "ERROR!!!!!! enc dec returned wrong message\n");
	    fprintf(fileptr, "ERROR!!!!!! enc dec returned wrong message\n");
	    goto free;
	}
    }

    TIMER_REPORT(encryption, fileptr);
    TIMER_REPORT(decryption, fileptr);

    writelineSep(fileptr);

 free:
    mpz_clears(m, m2, c, NULL);
    clearRandomness();
    cleanOpenSSL();
}
