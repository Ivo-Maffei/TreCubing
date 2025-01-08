#include "constructPrimes.h"

#include <assert.h>
#include <stdbool.h> // for bool
#include "rand.h"
#include <openssl/bn.h>

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

// use openssl for generating safe primes
void findOpensslSafePrime(mpz_t p, const unsigned long Nbits) {
    // we must have Nbits < 2^31 to fit an int
    assert(Nbits < INT_MAX);

    mp_limb_t temp_limb;
    uint8_t temp_byte;
    int bytelen, nlimbs, bigendian;
    BIGNUM* ossl_num = BN_new();

    assert(ossl_num);

    // generate the safe prime
    // the NULLs are for callback and requirements on type of prime
    if (!BN_generate_prime_ex(ossl_num, Nbits, true, NULL, NULL, NULL)) {
	fprintf(stderr, "ERROR with openssl prime generation\n");
	mpz_set_ui(p, 0l);
    }

    // now convert BN to GMP
    // get "dump" BN number into a byte buffer with most significant byte first
    mp_limb_t* rawp = mpz_limbs_write(p, (Nbits+63)/64);
    bytelen = BN_bn2bin(ossl_num, (uint8_t*)rawp);
    nlimbs = (bytelen+7)/8;

    // we need to swap to little endian limbs
    // if each limb is in bigendian that concatenating 8 bytes will give you a correct limb
    // otherwise we need to reverse the 8 bytes
    temp_limb = 1l;
    bigendian = !(((uint8_t*)&temp_limb)[0] & 1);

    if (bigendian){ // reverse limbs
	for (int i = 0; i < nlimbs / 2; ++i){
	    // swap lowest ith limb with highest ith limb
	    temp_limb = rawp[i];
	    rawp[i] = rawp[nlimbs-1-i];
	    rawp[nlimbs-1-i] = temp_limb;
	}
	// now rawp is in little endian
	// however, we might have some extra zeros at the start
	// so we need to rightshift (i.e. divive by 2)
	if (nlimbs*64 - bytelen*8) mpn_rshift(rawp, rawp, nlimbs, nlimbs*64 - bytelen*8);
	// the if statement is needed as we cannot use rshift with argument 0
    } else { // revese bytes
	for (int i=0; i < bytelen / 2; ++i){
	    temp_byte = ((uint8_t*)rawp)[i];
	    ((uint8_t*)rawp)[i] = ((uint8_t*)rawp)[bytelen-1-i];
	    ((uint8_t*)rawp)[bytelen-1-i] = temp_byte;
	}
	// make sure the highest limb has zero in unused high bytes
	for (int i=bytelen; i < nlimbs*8; ++i)
	    ((uint8_t*)rawp)[i] = 0;
    }

    mpz_limbs_finish(p, nlimbs);

    BN_free(ossl_num);
}

// finds a safe prime of the specified bitsize (sort of)
// we assume such safe prime is larger than the 10 000th prime
void findPseudoSafePrime(mpz_t p, const unsigned long Nbits) {

    mpz_t q, t;
    bool isPrime;

    mpz_inits(q, t , NULL);

    mpz_set_ui(q, 1l);
    mpz_mul_2exp(q, q, Nbits-1); // q is 2^{Nbits-1}

    isPrime = false;
    while (!isPrime){
	mpz_nextprime(q, q); // finds the next prime

	// we will check if p=2q+1 is prime with the pocklington test
	// i.e. if there is an integer a such that a^{p-1} = 1 mod p and gcd(a^2-1, p) =1, then p is prime
	// we do the test for a=2 (we could pick a=3, so that a^2-1 is 8, hence we only need p to be odd, which it is!)
	// note that if we fail, then q is not prime (either 2^{p-1} != 1 or 3 divides p)

	// we check gcd(3, p)=1 by checking q%3 == 2
	// fdiv_ui returns the mod
	if( mpz_fdiv_ui(q, 3l) != 2l) continue; // not safe prime

	// now check 2^{p-1} = 1 mod p
	mpz_mul_2exp(p, q, 1l);
	mpz_add_ui(p, p, 1l);
	// p <- 2q+1

	// we compute 2^p mod p which must be 2
	mpz_set_ui(t, 2l);
	mpz_powm(t, t, p, p);
	if (mpz_cmp_ui(t, 2l) == 0l) { // found a safe prime
	    isPrime = true;
	}

    }

    mpz_clears(q, t, NULL);
}

// gets a prime of type mq+1 where m is quite small
// note that m=q mod 3 and m even
// so for q=2 mod 3 -> m = 6k + 2
// and for q=1 mod 3 -> m = 6k + 4
// we could just use m= 6k + 4(q%3), but 6k + 2(3-(q%3)) is perfect
void findAlmostSafePrime(mpz_t p, const unsigned long Nbits){
    mpz_t q;
    unsigned long m; // note that we expect m = O(log p) = O(N) hence in ulong
    bool isPrime;

    mpz_init(q);

    // get a prime of Nbits
    mpz_set_ui(q, 1l);
    mpz_mul_2exp(q, q, Nbits-1);
    mpz_nextprime(q, q);

    // now look for m = 6k + 2(3-(q%3)) = 6(k+1) - 2(q%3)
    // so we use 6k - 2(q%3) and start with k=1
    m = -2*mpz_fdiv_ui(q, 3l); //fdiv_ui returns the mod and we will add 6 later
    isPrime = false;
    while (!isPrime){
	m += 6;
	// compute p = mq+1
	mpz_mul_ui(p, q, m);
	mpz_add_ui(p, p, 1l);

	// check p prime using pocklinton's test
	// i.e. check a^{p-1} =1 mod p and gcd(a^(p-1/q)-1, p) = 1
	// we use a=2 for efficiency
	mpz_ui_pow_ui(q, 2, m); // q <- 2^m
	mpz_sub_ui(q, q, 1l); // q = 2^m -1

	mpz_gcd(q, p, q); // q <- gcd(p, q)
	if (mpz_cmp_ui(q, 1l) != 0) // q != 1
	    continue; // p not prime

	// we ceck 2^p = 2 mod p instead of 2^(p-1) = 1 mod p
	mpz_set_ui(q, 2l);
	mpz_powm(q, q, p, p);
	if (mpz_cmp_ui(q, 2l) == 0) // found a prime!
	    isPrime = true;
    }

    fprintf(stderr, "m value %lu\n", m);
    mpz_clear(q);
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
	if (N < 2500l){
	    findOpensslSafePrime(p, N);
	}
	else if (N < 5000l){
	    findAlmostSafePrime(p, N);
	}
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
