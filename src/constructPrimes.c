#include "constructPrimes.h"

#include <assert.h>
#include <stdbool.h> // for bool
#include <stdio.h>
#include "rand.h"

const unsigned long availablePrimeSizes[] = {
    512,
    1024,
    2048,
    3072,
    5000,
    10000,
    //    20000,
    30000,
    // 40000,
    50000,
    // 60000,
    70000,
    // 80000,
    90000,
    100000
};

const unsigned long numAvailablePrimes = 11;




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

    mpz_t q, t;
    bool isPrime;
    unsigned long count = 0;

    mpz_inits(q, t , NULL);

    mpz_set_ui(q, 1l);
    mpz_mul_2exp(q, q, Nbits-1); // q is 2^{Nbits-1}

    isPrime = false;
    while (!isPrime){
	++count;
	mpz_nextprime(q, q); // finds the next prime

	// we will check if p=2q+1 is prime with the pocklington test
	// i.e. if there is an integer a such that a^{p-1} = 1 mod p and gcd(a^2-1, p) =1, then p is prime
	// we do the test for a=2 (we could pick a=3, so that a^2-1 is 8, hence we only need p to be odd, which it is!)
	// note that if we fail, then q is not prime (either 2^{p-1} != 1 or 3 divides p)

	// we check gcd(3, p)=1 by checking q%3 == 2
	mpz_mod_ui(t, q, 3l); // compute t = q % 3
	if( mpz_cmp_ui(t, 2l) != 0) continue; // not safe prime

	// now check 2^{p-1} = 1 mod p
	// note that t is already 2
	mpz_mul_2exp(p, q, 1l);
	mpz_add_ui(p, p, 1l);
	// p <- 2q+1

	// we compute 2^p mod p which must be 2
	mpz_powm(t, t, p, p);
	if (mpz_cmp_ui(t, 2l) == 0l) { // found a safe prime
	    isPrime = true;
	}

	if ( (count << 54) == 0) fprintf(stderr, ".");
    }
    fprintf(stderr, "tested %lu primes\n", count);
    mpz_clears(q, t, NULL);
}


// construct a prime and stores it in p
// p should already be initialised with the correct size
void constructPrime(mpz_t p, const unsigned long N) {
    switch (N) {
    case 5000l:
	constructPrime5k(p);
	break;
    case 10000l:
	constructPrime10k(p);
	break;
    case 20000l:
	constructPrime20k(p);
	break;
    case 30000l:
	constructPrime30k(p);
	break;
    case 40000l:
	constructPrime40k(p);
	break;
    case 50000l:
	constructPrime50k(p);
	break;
    case 60000l:
	constructPrime60k(p);
	break;
    case 70000l:
	constructPrime70k(p);
	break;
    case 80000l:
	constructPrime80k(p);
	break;
    case 90000l:
	constructPrime90k(p);
	break;
    case 100000l:
	constructPrime100k(p);
	break;
    default:
	if (N < 5000l)
	    findPseudoSafePrime(p, N);
	else {
	    fprintf(stderr, "constructing primes of size %lu is not supported\n", N);
	    assert(0);
	}
    }
}


// assuming secpar is not crazt high, we just compute a random number of secpar bits and find the next prime
// use this as the basis for q
// MUST ENSURE p=2 mod 3 otherwise cubing is not invertible in ZZ_q^*
void constructPrimePower(mpz_t q, mpz_t p, const unsigned long secpar, const unsigned long N){

    unsigned long k;

    // get a random number of exactly secpar bits
    randomNumber(p, secpar);
    do {
	mpz_nextprime(p, p); // p is now prime
    } while (mpz_fdiv_ui(p, 3l) != 2l); // try again untill we get a prime congruent 2 modulo 3

    k = mpz_sizeinbase(p, 2); // actual bitsize of p
    k = (N + k -1) / k; // ceil (N/k)

    assert(k>1 && "Prime power is a trivial power");

    mpz_pow_ui(q, p, k); // q = p^k
}
