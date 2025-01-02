#include "fpe.h"

#include <stdlib.h>

#include "rand.h"

void thorp(mpz_t c, const mpz_t m, const mpz_t N, const unsigned long R, const uint8_t* key){
    uint64_t b;

    mpz_set(c, m);
    for (unsigned long i=0; i<R; ++i){
	mpz_mul_2exp(c, c, 1l);

	// get ith bit of key
	b = (uint64_t)(key[i/8+i%8]);

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
}


void inverse_thorp(mpz_t m, const mpz_t c, const mpz_t N, const unsigned long R, const uint8_t* key){
    uint64_t b;

    mpz_set(m, c);
    for(unsigned long i=R; i > 0; --i){

	// to access the ith bit of the key
	b = (uint64_t)(key[i/8+i%8]);

	if (mpz_tstbit(m, 0) ^ b){
	    // m is the result of 2m+b - N : i.e. floor((m+N) / 2)
	    mpz_add(m, m, N);
	} // otherise m is the result of 2m + 1 - b

	mpz_tdiv_q_2exp(m, m, 1l);
	// as a single formula: y -> floor(y/2) + (y[0] xor b)*N
    }
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


void sometimesRecurse(mpz_t x, const mpz_t N, const unsigned long R) {

    mpz_t N2;

    mpz_init2(N2, mpz_sizeinbase(N, 2));

    mpz_tdiv_q_2exp(N2, N, 1l); // N2 <- N/2 truncated

    swapOrNot(x, N, R);

    while (mpz_cmp(x, N2) < 0  && mpz_cmp_ui(N2, 1l) != 0){
	swapOrNot(x, N2, R);
	mpz_tdiv_q_2exp(N2, N2, 1l); // N2 <- N/2 truncated
    }

    if (mpz_cmp_ui(N2, 1l) == 0)
	mpz_set(x, 0l);


    mpz_clears(N2, NULL);

}
