#include "lowLevelSquaring.h"

#include <stdlib.h> // malloc
#include <gmp.h>

// low-level repeated squaring function usnig Montgomery reduction
void lowLevelSquaring(mpz_t m, mpz_t p, const unsigned long b, const unsigned long Rlimbs) {
    // We need R > p
    // so we pick R = 2^{Rlimbs *64} assuming that Rlimbs is the number of limbs of p    
    mpz_t R, p2;
    mp_limb_t *t1, *t2, *ml;
    const mp_limb_t *pl, *p2l;
    const unsigned long Rsize = Rlimbs * GMP_LIMB_BITS + 1l; // because R = 2^{Rlimbs * 64}
    unsigned long i;
    

    mpz_inits(R, p2, NULL);

    mpz_set_ui(R, 1l);
    mpz_mul_2exp(R, R, Rsize -1l); // R= 2^(Rsize-1) = 2^{Rlimbs *64}

    mpz_neg(p2, p);
    mpz_invert(p2, p2,  R); // p2*p = -1 mod R
    
    // prepare montgomery form
    mpz_mul_2exp(m, m, Rsize- 1l); // m <- m * R
    mpz_mod(m, m, p); // m <- m mod p   
    
    pl = mpz_limbs_read(p); // we assume that mpz_size(p) is Rlimbs
    p2l = mpz_limbs_read(p2); // we assume that mpz_size(p2) is Rlimbs
    ml = mpz_limbs_modify(m, 2*Rlimbs);
    t1 = (mp_limb_t*) malloc(2*Rlimbs*GMP_LIMB_BITS);
    t2 = (mp_limb_t*) malloc(2*Rlimbs*GMP_LIMB_BITS);

    for (i=0; i < b; ++i) { // square m
	mpn_sqr(t1, ml, Rlimbs); // m <- m^2
	//mpn_copyi(ml, t1, 2*Rlimbs); // this because we can't have same destination and source in mpn_sqr

	// now REDC
	mpn_mul_n(t2, t1, p2l, Rlimbs); // lowest Rlibs of (m^2) * p2 -> (m^2 mod R) * p2
	mpn_mul_n(ml, t2, pl, Rlimbs); // lowest Rlimbs of t2 * p -> (t2 mod R) * p -> ((m^2 mod R) * p2  mod R) * p
	mpn_add_n(t2, t1, ml, 2*Rlimbs); // ml <- m^2 + ml = m^2 + ((m^2 mod R) * p2  mod R) * p
	mpn_copyi(ml, t2+Rlimbs, Rlimbs);
	
       	if (mpn_cmp(ml, pl, Rlimbs) > 0) // ml > p
	    mpn_sub_n(ml, ml, pl, Rlimbs); // ml <- ml-p
    }

    // finalise the changes on m
    mpz_limbs_finish(m, Rlimbs); // only the first Rlimbs are ok, the rest might contain garbage

    // before finishing we must remove the montgomery form
    mpz_invert(p2, R, p); // p2*R = 1 mod p
    mpz_mul(m, m, p2); // m <- m*p2
    mpz_mod(m, m, p); // m <- m mod p
    
    free(t1);
    free(t2);
    mpz_clears(R, p2, NULL);
}

