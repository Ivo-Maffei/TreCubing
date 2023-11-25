#include "fpe.h"

#include <stdlib.h>

#include "rand.h"

void thorp(mpz_t x, const mpz_t N, const unsigned long R){
    uint64_t b;
    
    for (unsigned long i=0; i<R; ++i){
	
	b = nextRand();
	mpz_mul_2exp(x, x, 1l);
	
	// if x >= N <--> x > y
	if(mpz_cmp(x, N)>=0) {
	    mpz_add_ui(x, x, 1^b);
	    mpz_sub(x, x, N);
	    // x <- 2x + 1 - b mod N
	} else {
	    mpz_add_ui(x, x, b);
	    // x <- 2x +  b
	}

	// as a single formula: x -> 2x + (2x>=N) xor b - N *(2x>=N)
    }
}


void inverse_thorp(mpz_t x, const mpz_t N, const unsigned long R){
      
    // first we need to generate all the bits used
    const unsigned long Nwords = (R+63l) / 64l;
    uint64_t *b = (uint64_t*) malloc(Nwords * sizeof(uint64_t));
    
    for (unsigned long i=0; i < Nwords; ++i){
	b[i] = xorshf64();
    }

    uint64_t bit;

    for(unsigned long i=R; i > 0; --i){
	 
	// to access the ith bit we need the bit in position i-1
	bit = (b[(i-1l)/64] >> ((i-1l)%64)) & 1l;

	if (mpz_tstbit(x, 0) ^ bit){
	    // x is the result of 2x+b - N : i.e. floor((x+N) / 2)
	    mpz_add(x, x, N);    
	} // otherise x is the result of 2x + 1 - b
	
	mpz_tdiv_q_2exp(x, x, 1l); 
    }

    // as a single formula: y -> floor(y/2) + (y[0] xor bit)*
    free(b);
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


