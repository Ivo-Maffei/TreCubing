#include "fpe.h"

#include <stdlib.h>
#include <stdio.h>

#include "rand.h"

void thorp(mpz_t c, const mpz_t m, const mpz_t q, const unsigned long R, const uint8_t* key){
    uint64_t b;
    mpz_t N; // number of cards to shuffle
    // N must be even, so if q is odd, instead of shuffling [0..q)
    // we shuffle [0..q-1) and assume m is not q-1

    mpz_init_set(N, q);
    if (mpz_tstbit(N,0)) mpz_sub_ui(N, q, 1l); // N is odd

    mpz_set(c, m);
    for (unsigned long i=0; i<R; ++i){
	mpz_mul_2exp(c, c, 1l);

	// get ith bit of key
	b = (uint64_t)(key[i/8]>>(i%8))&1l;

	// if c >= N <--> c > y
	if(mpz_cmp(c, N)>=0) {
	    mpz_add_ui(c, c, 1l^b);
	    mpz_sub(c, c, N);
	    // c <- 2c + 1 - b mod N
	} else {
	    mpz_add_ui(c, c, b);
	    // c <- 2c +  b
	}
	// as a single formula: c -> 2c + (2c>=N) xor b - N *(2c>=N)
    }
    mpz_clear(N);
}


void inverse_thorp(mpz_t m, const mpz_t c, const mpz_t q, const unsigned long R, const uint8_t* key){
    uint64_t b;
    mpz_t N; // number of cards to shuffle. This must be even

    mpz_init_set(N, q);
    // if odd, subtract 1
    if (mpz_tstbit(N, 0)) mpz_sub_ui(N, q, 1l);

    mpz_set(m, c);
    for(unsigned long i=R; i > 0; /*do nothing*/){
	--i; // this way we execute when i=0 and stop after
	// to access the ith bit of the key
	b = (uint64_t)(key[i/8]>>(i%8))&1l;

	if (mpz_tstbit(m, 0) ^ b){
	    // m is the result of 2m+b - N : i.e. floor((m+N) / 2)
	    mpz_add(m, m, N);
	} // otherise m is the result of 2m + 1 - b

	mpz_tdiv_q_2exp(m, m, 1l);
	// as a single formula: y -> floor(y/2) + (y[0] xor b)*N
    }
    mpz_clear(N);
}


void swapOrNot(mpz_t x, const mpz_t N, const unsigned long R){

    uint64_t b;
    mpz_t K;
    gmp_randstate_t randState;

    mpz_init2(K, mpz_sizeinbase(N, 2));
    gmp_randinit_mt(randState);

    for (unsigned long i=0; i<R; ++i){
	b = nextRand();

	if (b) { // b==1
	    mpz_urandomm(K, randState, N);
	    mpz_sub(x, K, x);
	    if (mpz_sgn(x) < 0) mpz_add(x, x, N);
	    // x <- K-x mod N
	}
    }

    mpz_clears(K, NULL);
    gmp_randclear(randState);
}
