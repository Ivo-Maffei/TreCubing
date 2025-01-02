#include "delay.h"

#include "enc.h"
#include "fpe.h"
#include "rand.h"

// delay message m using cubing modulo p, chained C times with:
// - Thorp shuffle using R rounds of shuffling if R != 0
// - Both-Ends Encryption, if R == 0
void delay(mpz_t r, const mpz_t m, const mpz_t p, const unsigned long C, const unsigned long R,  const uint8_t *key){
    if (R == 0) {
	initialiseOpenSSL();
    }

    mpz_set(r, m);

    for (unsigned long chainRound=0; chainRound<C; ++chainRound){

	if (R != 0){// use Thorp
	    thorp(r, r, p, R, key); // shuffle
	} else {
	    // both-ends encryption
	    cycleEnc(r, r, p, key);
	}

	// delay the message
	mpz_powm_ui(r, r, 3l, p);
    }

    if (R == 0) {
	cleanOpenSSL();
    }
}

// opens the puzzle c and place the result in m
void open(mpz_t m, const mpz_t c, const mpz_t p, const mpz_t b, const unsigned long C, const unsigned long R, const uint8_t *key){

    if (R == 0) {
	initialiseOpenSSL();
    }

    mpz_set(m, c);

    for (unsigned long chainRound=0; chainRound<C; ++chainRound){
	// now cube root
	mpz_powm(m, m, b, p);
	if (R != 0){ // use Thorp
	    inverse_thorp(m, m, p, R, key); // shuffle
	} else { // use BEE
	    cycleDec(m, m, p, key);
	}
    }

    if (R == 0) {
	cleanOpenSSL();
    }
}
