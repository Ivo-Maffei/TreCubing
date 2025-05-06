#include <argp.h>
#include <assert.h>
#include <errno.h> // error codes
#include <gmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h> // convert strings to int/longs
#include <string.h> // to parse strings


#include "constructPrimes.h"
#include "testTimes.h"
#include "rand.h"
#include "enc.h"
#include "hash.h"

#define DEFAULTITERS 100
#define STRINGIFY(x) STRINGIFY2(x) // we need all this bloatware to make it work
#define STRINGIFY2(x) #x
#define BOOLSTR(bool) bool ? "yes" : "no"

// setup global variables for the argument parsing
const char * argp_program_version = "trecubing v2.0";
const char * argp_program_bug_address = "ivo.maffei@uni.lu";
static char args_doc[] = "[FILENAME|stdout]"; // mandatory arguments
static char doc[] = "Test different primitives for Time-Lock Puzzles via Cubing and outputs the test results in the file provided.";

static struct argp_option options[] = {
    // { <long name>, <ascii code for short name>, <name of argument>, <flags>, <documentation>, <group id>}
    { "iterations", 'n', "nIters", 0, "Specify the number of indipendent iterations to run (default: " STRINGIFY(DEFAULTITERS) ")" },
    { "securityParam", 's', "secpar", 0, "If non-zero, this specifies the bit-size of the based used for moduli using prime powers or product of primes powers" },
    { "numberPrimes", 'k', "nprimes", 0, "If non-zero, this specifies the number of primes to use for the product-of-primes moduli" },
    { "primesize", 'p', "pSize", 0, "Specify the (approximate) size in bits for the modolus to use (default: test all valid sizes)" },
    { 0, 0, 0, 0, "Select one or more of the following 5 if you don't want to test all methods:", 1}, // this is a header for the next group
    { "cubing", 'c', 0, 0, "Test the cubing/cube root performance"},
    { "encryption", 'e', 0, 0, "Test the stream cipher encryption performance"},
    { "hashing", 'x', 0, 0, "Test the hashing performance"},
    { "moduli", 'm', 0, 0, "Test the performance of prime power modulo creations" },
    { "clean", -1, 0, 0, "Clean the output file before writing to it", 1}, // we use -1 to avoid allowing a short version
    { 0 } // termination of this "vector"
};

// this encapsulates all inputs received by the user
// this is what will be used in the main function
struct input {
    char *filename;
    unsigned long nIters;
    unsigned long pSize;
    unsigned long nprimes;
    unsigned long secpar;
    bool cubing;
    bool enc;
    bool hashing;
    bool moduli;
    bool clean;
};

// this is the function that handle the actual parsing
error_t parser_fun(int key, char *arg, struct argp_state *state) {

    struct input *input = state->input; // state->input is a pointer to my struct input

    switch(key){
    case 'n': {// handle iterations
	if (arg == 0) { // no value is given
	    argp_error(state, "If --iterations is specified, then a number must follow");
	    return EINVAL;
	}
	input->nIters = strtoul(arg, (char**) NULL, 10); // convert arg to unsigned long base 10 and ingnore other outputs [using (char**) NULL]
	break;
    }
    case 's': {// handle security paramter
	if (arg == 0) { // no value is given
	    argp_error(state, "If --securityParam is specified, then a number must follow");
	    return EINVAL;
	}
	input->secpar = strtoul(arg, (char**) NULL, 10);
	break;
    }
    case 'k': { // handle number of primes
	if (arg == 0){ // no value is given
	    argp_error(state, "If --securityParam is specified, then a number must follow");
	    return EINVAL;
	}
	input->nprimes = strtoul(arg, (char**) NULL, 10);
	break;
    }
    case 'p': {// handle primesize
	if (arg == 0) { // no value is given
	    argp_error(state, "If --primesize is specified, then a number must follow");
	    return EINVAL;
	}
	input->pSize = strtoul(arg, (char**) NULL, 10);
	break;
    }
    case 'm': { // handle test of moduli
	input->moduli = true;
	break;
    }
    case 'c': {// handle cubing
	input->cubing = true;
	break;
    }
    case 'e': { // handle encryption
	input->enc = true;
	break;
    }
    case 'x': { // handle hashing
	input->hashing = true;
	break;
    }
    case -1: { // clean option specified
	input->clean = true;
    }
    case ARGP_KEY_ARG: {// handle non-optional argument
	if (state->arg_num != 0) { // we have already parsed a non-optional argument (hence we already have a filenema)
	    argp_error(state, "Only one output file can be specified"); // output error message and terminate the program
	    return EINVAL; // invalid argument error; this should be unreachable as the above function terminates the program
	}
	input->filename = arg;
	break;
    }
    case ARGP_KEY_END: {// all command parsed -> check that mandatory ones have been read and set -cdf if none were provided
	if (state->arg_num == 0) {
	    argp_error(state, "No output file specified");
	    return EINVAL;
	}
	if (!(input->cubing || input->enc || input->moduli || input->hashing)) { // no specific test set
	    // set all tests to true
	    input->cubing = input->enc = input->moduli = input->hashing = true;
	}
	if (input->nprimes && !input->secpar) {
	    argp_error(state, "Specified a number of primes for the modulo, but not their size");
	    return EINVAL;
	}
    }
    default:
	return ARGP_ERR_UNKNOWN;
    }
    return 0; // exit successfully
}

static struct argp argp_struct = { options, parser_fun, args_doc , doc };





// tell user what we are going to test
void printReceivedInput(struct input input) {
    printf("Output test results to file: %s\n", input.filename);
    printf("Testing %lu iterations of:\n", input.nIters);
    printf("modulo: %s\ncubing: %s\nstream encryption: %s\nhashing: %s\n", BOOLSTR(input.moduli), BOOLSTR(input.cubing), BOOLSTR(input.enc), BOOLSTR(input.hashing));

    printf("Prime sizes selected: ");
    if (input.pSize) printf("%lu\n", input.pSize);
    else {
	for (int i=0; i < numAvailablePrimes; ++i) printf("%lu ", availablePrimeSizes[i]);
	printf("\n");
    }
    if (input.nprimes)
	printf("Using product of prime powers with %lu primes and security paramter %lu\n", input.nprimes, input.secpar);
    else if (input.secpar)
	printf("Using prime powers with security parameter %lu\n", input.secpar);
    else
	printf("Using safe primes\n");
}

// ENTRYPOINT
int main(int argc, char **argv) {

    assert(GMP_NUMB_BITS == 64);

    // create object to encapsulate all inputs
    struct input input = { .nIters = DEFAULTITERS };  // we give a default value of 30 to nIters; everything else deafaults to 0 (NULL, false)

    error_t errorcode = argp_parse(&argp_struct, argc, argv, 0, NULL, &input); // first 0 are the optional flags. the NULL is for unparsed argumets

    // if any error found, stop
    if (errorcode) {
	return  errorcode;
    }

    // instantiate the primes, chain lengths and rounds to test
    const unsigned long *primeSizes; // pointer to const
    unsigned long nPrimes;
    
    if (input.pSize == 0) {
	primeSizes = availablePrimeSizes;
	nPrimes = numAvailablePrimes;
    } else {
	primeSizes = &input.pSize;
	nPrimes = 1;
    }

    // tell use what we are going to do
    printReceivedInput(input);

    // OPEN OUTPUT FILE
    FILE* fileptr = NULL;
    if (strcmp(input.filename, "stdout") == 0) fileptr=stdout;
    else if(input.clean) fileptr = fopen(input.filename, "w");
    else fileptr = fopen(input.filename, "a");

    if (!fileptr) {
	printf("Cannot open file, aborting...\n");
	return -2;
    }

    fprintf(fileptr, "\n\n");
    for(int i=0; i<50; ++i) fprintf(fileptr, "=");
    fprintf(fileptr, "\nSTART TESTS using %lu iterations\n\n", input.nIters);
    fflush(fileptr);

    // ACTUALLY DO THE TESTS
    mpz_t q, b;
    unsigned long N; // bitsize of q

    mpz_inits(q, b, NULL);

    // initialise xorshf64
    //    setSeed(rand());

    for(unsigned long i=0; i < nPrimes; ++i) {

	if (input.moduli){
	    testModuloConstruction(primeSizes[i], input.nprimes, input.secpar, input.nIters, fileptr);
	    fflush(fileptr);
	    printf("Tested modulo creation\n");
	}

	// combpute modulo and exponent for the cubing
	if (input.nprimes)
	    constructmPower(q, b, input.nprimes, primeSizes[i]);
	else if (input.secpar)
	    constructPrimePower(q, b, input.secpar, primeSizes[i]);
	else constructSafePrime(q, b, primeSizes[i]);

	N = mpz_sizeinbase(q, 2);
	printf("Using a prime with exactly %lu bits\n", N);


	if (input.cubing) { // test repeated squarings
	    testTimesSq(q, b, N, input.nIters, fileptr);
	    fflush(fileptr);
	    printf("Tested cubing\n");
	}

	if (input.enc) { // test AES256-OFB ecnryptions
	    if (!input.secpar) {
		fprintf(stderr, "Cannot test encryption without a security parameter\n");
	    } else {
		testTimesEnc(primeSizes[i], input.secpar, input.nIters, fileptr);
		fflush(fileptr);
		printf("Tested AES256-OFB encryption\n");
	    }
	}

	if (input.hashing) {
	    testTimesHash(q, input.nIters, fileptr);
	    fflush(fileptr);
	    printf("Testing hahsing\n");
	}

    }

    fprintf(fileptr, "\n\nEND TEST\n");
    for(int i=0; i<50; ++i) fprintf(fileptr, "=");
    fclose(fileptr);

    mpz_clears(q, b, NULL);
    cleanOpenSSL();
    cleanHashing();
    clearRandomness();
    clearPrimesDB();
    return 0;
}
