#include "delay.h"

#include "enc.h"
#include "fpe.h"
#include "rand.h"

// delay message m using cubing modulo p, chained C times with Thorp shuffle using R rounds of shuffling
// seed is the seed to use with Throp and key is the AES key to use
// if key is NULL, use Thorp, else use Both-Ends Encryption
void delay(mpz_t r, const mpz_t m, const mpz_t p, const unsigned long C, const unsigned long R, const uint64_t seed, const uint8_t *key){
    if (key != NULL) {
	initialiseOpenSSL();
    }

    mpz_set(r, m);

    for (unsigned long chainRound=0; chainRound<C; ++chainRound){

	if (key == NULL){// use Thorp
	    setSeed(seed);
	    thorp(r, p, R); // shuffle
	} else {
	    // both-ends encryption
	    cycleEnc(r, r, p, key);
	}

	// delay the message
	mpz_powm_ui(r, r, 3l, p);
    }

    if (key != NULL) {
	cleanOpenSSL();
    }
}

// opens the puzzle c and place the result in m
void open(mpz_t m, const mpz_t c, const mpz_t p, const mpz_t b, const unsigned long C, const unsigned long R, const uint64_t seed, const uint8_t *key){

    if (key != NULL) {
	initialiseOpenSSL();
    }

    mpz_set(m, c);

    for (unsigned long chainRound=0; chainRound<C; ++chainRound){
	// now cube root
	mpz_powm(m, m, b, p);
	if (key == NULL){ // use Thorp
	    setSeed(seed);
	    inverse_thorp(m, p, R); // shuffle
	} else { // use BEE
	    cycleDec(m, m, p, key);
	}
    }

    if (key != NULL) {
	cleanOpenSSL();
    }
}
