#include "delay.h"

#include "enc.h"
#include "fpe.h"
#include "rand.h"


// delay message m using cubing modulo p, chained C times with Thorp shuffle using R rounds of shuffling
// seed is the seed to use with Throp and key is the AES key to use
// if key is NULL, use Thorp, else use Both-Ends Encryption
void delay(mpz_t r, const mpz_t m, const mpz_t p, const unsigned long C, const unsigned long R, const uint64_t seed, const uint8_t *key){
    mpz_t N;

    if (key == NULL) {
	mpz_inits(N, NULL);
	mpz_sub_ui(N, p, 1l);
    } else {
	initialiseOpenSSL();
    }

    mpz_set(r, m);

    for (unsigned long chainRound=0; chainRound<C; ++chainRound){

	// delay the message
	mpz_powm_ui(r, r, 3l, p);

	if (key == NULL){// now the FPE
	    // first map to 0...p-2
	    mpz_sub_ui(r, r, 1l);
	
	    setSeed(seed);
	    thorp(r, N, R); // shuffle

	    // map back to 1 ... p-1
	    mpz_add_ui(r, r, 1l);
	} else {
	    // both-ends encryption
	    cycleEnc(r, r, p, key);
	}

    }
       
    if (key == NULL) {
	mpz_clears(N, NULL);
    } else {
	cleanOpenSSL();
    }
}

// opens the puzzle c and place the result in m
void open(mpz_t m, const mpz_t c, const mpz_t p, const unsigned long C, const unsigned long R, const uint64_t seed, const uint8_t *key){
    mpz_t N, b;

    mpz_inits(N, b, NULL);

    if (key == NULL){
	mpz_sub_ui(N, p, 1l);
    } else {
	initialiseOpenSSL();
    }

    // compute b = 1/3 (2p-1)
    mpz_mul_2exp(b, p, 1l);
    mpz_sub_ui(b, b, 1l);
    mpz_divexact_ui(b, b, 3l);

    mpz_set(m, c);

    for (unsigned long chainRound=0; chainRound<C; ++chainRound){
	
	if (key == NULL){ // first the FPE
	    // first map to 0...p-2
	    mpz_sub_ui(m, m, 1l);

	    setSeed(seed);
	    inverse_thorp(m, N, R); // shuffle

	    // map back to 1 ... p-1
	    mpz_add_ui(m, m, 1l);
	} else {
	    cycleDec(m, m, p, key);
	}

	// now cube root
	mpz_powm(m, m, b, p);
    }

    if (key != NULL) {
	cleanOpenSSL();
    }
    mpz_clears(N, b, NULL);
    
}
