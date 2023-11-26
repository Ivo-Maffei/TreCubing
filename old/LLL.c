#include "LLL.h"

#include <flint/fmpz.h>
#include <flint/fmpz_vec.h>

#include <stdio.h>


// create a random lattice basis matrix using the same structure as the output of
// the quantum algorithm (with the exception that the values j are sampled randomly
// result stored in L
void LLLReduction_mat(fmpz_mat_t L, const unsigned long m, const unsigned long l, const unsigned long n) {

    flint_rand_t rand_state;
    fmpz_t twolm; // a integers

    // initialise everything
    flint_randinit(rand_state);
    fmpz_init(twolm);

    // set my constant
    // twolm = 2^{l+m}
    fmpz_set_d_2exp(twolm, 1.0, l+m);

    // set up the matrix
    fmpz_mat_zero(L);

    // generate the random js and diagonals
    for (unsigned long i = 0; i < n; ++i){
	fmpz_randm(fmpz_mat_entry(L, 0, i), rand_state, twolm);
	fmpz_set_d_2exp(fmpz_mat_entry(L, i+1l, i), 1.0, l+m);
    }
    fmpz_set_ui(fmpz_mat_entry(L, 0, n), 1l);
    

    // clear all memory
    fmpz_clear(twolm);
    flint_randclear(rand_state);
}

void cvp_vector(fmpz* vec, const unsigned long m, const unsigned long l, const unsigned long n) {

    flint_rand_t rand_state;
    fmpz_t twolm, twolm1, k; // a integers
    const unsigned long vecLen = n+1l;

    // initialisation
    flint_randinit(rand_state);
    fmpz_init(twolm);
    fmpz_init(twolm1);
    fmpz_init(k);

    // set my constant
    // twolm = 2^{l+m}
    // twolm1 = 2^{l+m-1}
    fmpz_set_d_2exp(twolm, 1.0, l+m);
    fmpz_set_d_2exp(twolm1, 1.0, l+m-1l);
    

    _fmpz_vec_zero(vec, n+1l);

    // compute entries of the vector
    // we sample k uniformly
    for(unsigned long i=0; i < n; ++i) {
	fmpz_randm(k, rand_state, twolm);
	fmpz_mul_2exp(vec + i , k, m); // vec[i] <- k2^m
	fmpz_neg(k, vec +  i); // k <- -k2^m ( x in my thesis)

	fmpz_mod(vec + i, k, twolm); // vec[i] <- x mod 2^{m+l}
	fmpz_fdiv_q(k, vec + i, twolm1); // k <- floor( (x mod 2^{m+l}) / (2^{m+l-1}) )
	fmpz_submul(vec + i, twolm, k); // vec[i] <- vec[i] - twoml * k = (x mod 2^{m+l}) - 2^{m+l} * floor((x mod 2^{m+l}) / (2^{m+l-1}))
    }
    
    // clear all memory
    fmpz_clear(twolm);
    fmpz_clear(twolm1);
    fmpz_clear(k);
    flint_randclear(rand_state);
}

void LLL_mat_to_file(const char* filename, const mpz_t p, const unsigned long s, const unsigned long n) {
    fmpz_mat_t M;
    const unsigned long m = mpz_sizeinbase(p, 2) + 1l;
    const unsigned long l = (m+s-1l) / s; //ceil(m / s)
    FILE* fileptr;

    fmpz_mat_init(M, n+1l, n+1l);

    LLLReduction_mat(M, m, l, n);

    fileptr = fopen(filename, "w");

    fmpz_mat_fprint_pretty(fileptr, M);

    fclose(fileptr);
    
    fmpz_mat_clear(M);
}


void cvp_to_file(const char* filename, const mpz_t p, const unsigned long s, const unsigned long n) {
    fmpz_mat_t M;
    fmpz* vec;
    const unsigned long m = mpz_sizeinbase(p, 2) + 1l;
    const unsigned long l = (m+s-1l) / s; //ceil(m / s)
    FILE* fileptr;

    // initialisation
    fmpz_mat_init(M, n+1l, n+1l);
    vec = _fmpz_vec_init(n+1l);

    // create matrix
    LLLReduction_mat(M, m, l, n);

    // create vector
    cvp_vector(vec, m, l, n);


    // write to file
    fileptr = fopen(filename, "w");

    fmpz_mat_fprint_pretty(fileptr, M);

    // write the vector
    fprintf(fileptr, "[");
    fmpz_fprint(fileptr, vec);
    for (unsigned long i=1; i < n+1l; ++i){
	fprintf(fileptr, " ");
	fmpz_fprint(fileptr, vec + i);
    }
    fprintf(fileptr, "]");

    fclose(fileptr);
    
    fmpz_mat_clear(M);
    _fmpz_vec_clear(vec, n+1l);
}


