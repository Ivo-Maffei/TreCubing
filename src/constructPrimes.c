#include "constructPrimes.h"

#include <stdbool.h> // for bool
#include <stdio.h>


const unsigned long availablePrimeSizes[] = {
    512,
    1000,
    1024,
    2000,
    5000,
    10000,
    20000,
    30000,
    40000,
    50000,
    60000,
    70000,
    80000,
    90000,
    100000
};

const unsigned long numAvailablePrimes = 13;




// p = 4133481*2^{5002} - 1
void constructPrime5k(mpz_t p) {
    mpz_set_ui(p, 1l);
    mpz_mul_2exp(p, p, 5002l);
    mpz_mul_ui(p, p, 4133481l);
    mpz_sub_ui(p, p, 1l);
}

// p = 9402702309*10^{3000} - 1
// bits: 9999
void constructPrime10k(mpz_t p) {
    mpz_set_ui(p, 10l);
    mpz_pow_ui(p, p, 3000l);
    mpz_mul_ui(p, p, 9402702309l);
    mpz_sub_ui(p, p, 1l);
}

// p = 29553033*2^{19991} - 1
// bits: 20016
void constructPrime20k(mpz_t p) {
    mpz_set_ui(p, 1l);
    mpz_mul_2exp(p, p, 19991l);
    mpz_mul_ui(p, p, 29553033l);
    mpz_sub_ui(p, p, 1l);
}

// p = 415365* 2^{30053} - 1
// bits: 30072
void constructPrime30k(mpz_t p) {
    mpz_set_ui(p, 1l);
    mpz_mul_2exp(p, p, 30053l);
    mpz_mul_ui(p, p, 415365l);
    mpz_sub_ui(p, p, 1l);
}

// p = 774951567* 2^{40961} - 1
// bits: 40991
void constructPrime40k(mpz_t p) {
    mpz_set_ui(p, 1l);
    mpz_mul_2exp(p, p, 40961l);
    mpz_mul_ui(p, p, 774951567l);
    mpz_sub_ui(p, p, 1l);
}

// p = 4127632557* 2^{50002} - 1
// bits: 50034
void constructPrime50k(mpz_t p) {
    mpz_set_ui(p, 1l);
    mpz_mul_2exp(p, p, 50002l);
    mpz_mul_ui(p, p, 4127632557l);
    mpz_sub_ui(p, p, 1l);
}

// p = 3714089895285* 2^{60001} - 1
// bits: 60043
void constructPrime60k(mpz_t p) {
    mpz_set_ui(p, 1l);
    mpz_mul_2exp(p, p, 60001l);
    mpz_mul_ui(p, p, 3714089895285l);
    mpz_sub_ui(p, p, 1l);
}

// p = 256685167*2^{70002} - 1
// bits: 70034
void constructPrime70k(mpz_t p) {
    mpz_set_ui(p, 1l);
    mpz_mul_2exp(p, p, 70002l);
    mpz_mul_ui(p, p, 2566851867l);
    mpz_sub_ui(p, p, 1l);
}

// p = 1213822389 * 2^{81132} - 1
// bits: 
void constructPrime80k(mpz_t p) {
    mpz_set_ui(p, 1l);
    mpz_mul_2exp(p, p, 81132l);
    mpz_mul_ui(p, p, 1213822389l);
    mpz_sub_ui(p, p, 1l);
}

// p = 3364553235* 2^{88889} - 1
// bits: 
void constructPrime90k(mpz_t p) {
    mpz_set_ui(p, 1l);
    mpz_mul_2exp(p, p, 88889l);
    mpz_mul_ui(p, p, 3364553235l);
    mpz_sub_ui(p, p, 1l);
}

// p = 35909079387* 2^{100001} - 1
// bits: 
void constructPrime100k(mpz_t p) {
    mpz_set_ui(p, 1l);
    mpz_mul_2exp(p, p, 100001l);
    mpz_mul_ui(p, p, 35909079387l);
    mpz_sub_ui(p, p, 1l);
}

// finds a safe prime of the specified bitsize (sort of)
// we assume such safe prime is larger than the 10 000th prime
void findPseudoSafePrime(mpz_t p, const unsigned long Nbits) {

    mpz_t q; // candidate prime
    bool isPrime = false;
    int primeCheck = -1;
    const unsigned long precision = 100l;
 
    mpz_inits(q, NULL);


    mpz_set_ui(p, 1l);
    mpz_mul_2exp(p, p, Nbits-1); // p is 2^{Nbits-1}

    isPrime = false;
    while (!isPrime){
	
	mpz_nextprime(p, p); // supposedly finds the next prime
        mpz_mod_ui(q, p, 3l); // compute q = p % 3
	// both p and q must be 2 modulo 3
	if( mpz_cmp_ui(q, 2l) != 0) continue; // not safe prime

	// check if p is a prime with high prob
	primeCheck = mpz_probab_prime_p(p, precision);

	// assume p = 2q+1 and compute q
	mpz_sub_ui(q, p, 1l);
	mpz_divexact_ui(q, q, 2l);

	// check if q is prime
	primeCheck = mpz_probab_prime_p(q, precision);
	if (primeCheck  != 0) {
	    isPrime = true;
	    //printf("Found p prime %i\n", primeCheck);
	}
	else {
	    // perhaps 2p+1 is prime
	    // set q  to 2p+1
	    mpz_mul_2exp(q, p, 1l);
	    mpz_add_ui(q, q, 1l);

	    // check if q is prime
	    primeCheck = mpz_probab_prime_p(q, precision);
	    if(primeCheck != 0) {
		isPrime = true;
		mpz_set(p, q);
		//printf("Found p prime %i\n", primeCheck);
	    }
	} 
	
    }

    //printf("Size of prime found %lu\n", mpz_sizeinbase(p, 2));

    mpz_clears(q, NULL);
}


// construct a prime and stores it in p
// p should already be initialised with the correct size
void constructPrime(mpz_t p, const unsigned long N) {
    switch (N) {
    case 512:
    case 1024:
    case 1000l: // fall trough to 2000l
    case 2000l:
	return findPseudoSafePrime(p, N);
    case 5000l:
	return constructPrime5k(p);
    case 10000l:
	return constructPrime10k(p);
    case 20000l:
	return constructPrime20k(p);
    case 30000l:
	return constructPrime30k(p);
    case 40000l:
	return constructPrime40k(p);
    case 50000l:
	return constructPrime50k(p);
    case 60000l:
	return constructPrime60k(p);
    case 70000l:
	return constructPrime70k(p);
    case 80000l:
	return constructPrime80k(p);
    case 90000l:
	return constructPrime90k(p);
    case 100000l:
	return constructPrime100k(p);
    }
}

