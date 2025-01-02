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


#define TIMER_INIT(name, iters)	  \
    clock_t start_ ## name, end_ ## name; \
    unsigned long* const allTime_ ## name = (unsigned long*) malloc(iters*sizeof(unsigned long)); \
    double avgTime_ ## name, stdTime_ ## name; \
    const unsigned long nIters_ ## name = iters; \
    unsigned long current_iter_ ## name = 0l;

#define TIMER_TIME(name, work, fp)			\
    start_ ## name = clock();\
    work; \
    end_ ## name = clock();\
    assert(current_iter_ ## name < nIters_ ## name); \
    allTime_ ## name[current_iter_ ## name] = end_ ## name - start_ ## name; \
    fprintf(fp, #name " took %.3fms\n", allTime_ ## name[current_iter_ ## name]/(double)CLOCKS_PER_SEC*1000.0); \
    ++current_iter_ ## name;

#define TIMER_REPORT(name, fp) \
    avgTime_ ## name = 0.0; \
    for (size_t i =0; i < nIters_ ## name; ++i) avgTime_ ## name += allTime_ ## name[i]; \
    avgTime_ ## name /= (double)(nIters_ ## name); \
    stdTime_ ## name = 0.0; \
    for (size_t i=0; i < nIters_ ## name; ++i) stdTime_ ## name += (allTime_ ## name[i] - avgTime_ ## name)*(allTime_ ## name[i] - avgTime_ ## name); \
    stdTime_ ## name /= (double)(nIters_ ## name); \
    stdTime_ ## name = sqrt(stdTime_ ## name); \
    fprintf(fp, "mean and std " #name " time %.9fms (%.9fms)\n", avgTime_ ## name/(double)CLOCKS_PER_SEC*1000.0, stdTime_ ## name/(double)CLOCKS_PER_SEC*1000.0); \
    free(allTime_ ## name);


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

// picks a random message m, computes m^3 mod p and then (m^3)^b mod p
// prints out the time taken and stores the time as well
// repeate for nIters and outputs means and std
void testTimesSq(mpz_t p, const mpz_t b, const unsigned long N, const int nIters, FILE * const fileptr) {

    writeTimestamp(fileptr);
    writeTimestamp(stdout);
    fprintf(fileptr, "Testing cubing using a prime of %lu bits\n", N);

    // variable for timing

    TIMER_INIT(Enc, nIters);
    TIMER_INIT(Dec, nIters);
    TIMER_INIT(SqGMP, nIters);

    // variables for the computation
    mpz_t m, m2, c, fexp;


    // initialse variables
    mpz_init2(m, N+1);
    mpz_init2(m2, N+1);
    mpz_init2(c, N+1);
    mpz_init2(fexp, N+1);

    const unsigned long nSquarings = mpz_sizeinbase(b, 2) - 1l;

    // fast exponent
    mpz_set_ui(fexp, 1l);
    mpz_mul_2exp(fexp, fexp, nSquarings);

    for (int i=0; i < nIters; ++i){

	randomMessage(m, p);

	// encryption
	TIMER_TIME(Enc,	mpz_powm_ui(c, m, 3l, p), fileptr);

        // decryption
	TIMER_TIME(Dec, mpz_powm(m2, c, b, p), fileptr);

	//check correctness
        if (mpz_cmp(m2, m) != 0){
            fprintf(fileptr, "ERROR: decryption is wrong!!!!");
            fprintf(fileptr, "-> %i\n", mpz_cmp(m2, m));
	    fprintf(stderr, "ERROR: repeated squaring failed\n");
        }

	// repeated squarings only
	TIMER_TIME(SqGMP, mpz_powm(m2, c, fexp, p), fileptr);

    }// end for loop

    TIMER_REPORT(Enc, fileptr);
    TIMER_REPORT(Dec, fileptr);
    TIMER_REPORT(SqGMP, fileptr);

    writelineSep(fileptr);

    mpz_clears(m, m2, c, fexp, NULL);
    clearRandomness();
}


void testTimesFpe(const mpz_t N, const unsigned long R, const bool t, const bool son, const bool sr, const int nIters, FILE * const fileptr){

    writeTimestamp(fileptr);
    writeTimestamp(stdout);

    fprintf(fileptr, "Testing FPE methods assuming a modulus of %lu bits and with %lu rounds\n", mpz_sizeinbase(N, 2), R);

     // variable for timing
    TIMER_INIT(Th, nIters);
    TIMER_INIT(Sw, nIters);
    TIMER_INIT(SR, nIters);

    mpz_t m;

    mpz_init2(m, mpz_sizeinbase(N, 2));


    for (int i=0; i<nIters; ++i){


	randomMessage(m, N);

	if(t){
	    TIMER_TIME(Th, thorp(m, N, R), fileptr);
	}

	if(son){
	    TIMER_TIME(Sw, swapOrNot(m, N, R), fileptr);
	}

	if(sr){
	    TIMER_TIME(SR, sometimesRecurse(m, N, R), fileptr);
	}
    }


    TIMER_REPORT(Th, fileptr);
    TIMER_REPORT(Sw, fileptr);
    TIMER_REPORT(SR, fileptr);

    writelineSep(fileptr);

    mpz_clears(m, NULL);
    clearRandomness();
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

    // create ceil(keyLength*8 / 64) random words of 64 bits
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
