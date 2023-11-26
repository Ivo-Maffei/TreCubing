#include <argp.h>
#include <assert.h>
#include <errno.h> // error codes
#include <gmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h> // convert strings to int/longs
#include <string.h> // to parse strings

#include <omp.h>

#include "constructPrimes.h"
#include "testTimes.h"

#define DEFAULTITERS 100
#define STRINGIFY(x) STRINGIFY2(x) // we need all this bloatware to make it work
#define STRINGIFY2(x) #x
#define BOOLSTR(bool) bool ? "yes" : "no"

// setup global variables for the argument parsing
const char * argp_program_version = "trecubing v1.0";
const char * argp_program_bug_address = "ivo.maffei@cs.ox.ac.uk";
static char args_doc[] = "[FILENAME|stdout]"; // mandatory arguments
static char doc[] = "Test different primitives for TRE by Cubing and outputs the test results in the file provided.";

static struct argp_option options[] = {
    // { <long name>, <ascii code for short name>, <name of argument>, <flags>, <documentation>, <group id>}
    { "iterations", 'n', "nIters", 0, "Specify the number of indipendent iterations to run (default: " STRINGIFY(DEFAULTITERS) ")" },
    { "primesize", 'p', "pSize", 0, "Specify the (approximate) size in bits for the modolus to use (default: test all valid sizes)" },
    { 0, 0, 0, 0, "Select one or more of the following 5 if you don't want to test all methods:", 1}, // this is a header for the next group
    { "cubing", 'c', 0, 0, "Test the cubing/cube root performance"},
    { "delay", 'd', 0, 0, "Test the performance of the whole delay/open method (using both-ends encryption)" },
    { "delayThorp", -2, 0, 0, "Test the performance of the whole delay/open method using Thorp as FPE"},
    { "encryption", 'e', 0, 0, "Test the both-ends encryption performance"},
    { "fpe", 'f', "fpeScheme",  OPTION_ARG_OPTIONAL, "Test the performance of the specified FPE methods as a comma separated list (if no list is provided, all methods are tested). Valid FPE schemes are: 'thorp', 'swapornot', 'sometimesrecurse'" },
    { "rounds", 'R', "nRounds", 0, "Specify the number of round to use when testing FPE schemes. If this option is omitted, we test the values: 100, 0.5*primesize and primesize", 2 },
    { "chain", 'C', "nChain", 0, "Specify how many delays to chain together when testing the whole delay process. If this option is omitted, we test values: 10 and 100" },
    { "clean", -1, 0, 0, "Clean the output file before writing to it", 1}, // we use -1 to avoid allowing a short version
    { 0 } // termination of this "vector"
};

// this encapsulates all inputs received by the user
// this is what will be used in the main function
struct input {
    char *filename;
    unsigned long nIters;
    unsigned long pSize;
    bool cubing;
    bool delay;
    bool delayThorp;
    bool fpeThorp;
    bool fpeSoN;
    bool fpeSR;
    bool enc;
    unsigned long nRounds;
    unsigned long nChain;
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
    case 'p': {// handle primesize
	if (arg == 0) { // no value is given
	    argp_error(state, "If --primesize is specified, then a number must follow");
	    return EINVAL;
	}
	input->pSize = strtoul(arg, (char**) NULL, 10);
	break;
    }
    case 'c': {// handle cubing
	input->cubing = true;
	break;
    }
    case 'd': {// handle delay
	input->delay = true;
	break;
    }
    case -2: { // handle delayThorp
	input->delayThorp = true;
	break;
    }
    case 'e': { // handel encryption
	input->enc = true;
	break;
    }
    case 'f': {// handle fpe
	if (arg == 0){ // no value is given -> meaing test all
	    input->fpeThorp = true;
	    input->fpeSoN = true;
	    input->fpeSR = true;
	    return 0;
	}
	// other we need to handle one or more arguments passed

	// first we need to copy the read-only arg to a temp local string
	char string[strlen(arg) + 1]; // create temp string
	strcpy(string, arg);

	// we use strtok to separate the string into token separated by commas
	char *ptr = strtok(string, ",");
	while (ptr != NULL) {
	    if (strcmp(ptr, "thorp") == 0) input->fpeThorp = true;
	    else if (strcmp(ptr, "swapornot") == 0) input->fpeSoN = true;
	    else if (strcmp(ptr, "sometimesrecurse") == 0) input->fpeSR = true;
	    ptr = strtok(NULL, ","); // go to next token
	}
	break;
    }
    case 'R': {// handle rounds
	if (arg == 0) { // no value is given
	    argp_error(state, "If --rounds is specified, then a number must follow");
	    return EINVAL;
	}
	input->nRounds = strtoul(arg, (char**) NULL, 10);
	break;
    }
    case 'C': { // handle chain length
	if (arg == 0) { // no value is given
	    argp_error(state, "If --chain is specified, then a number must follow");
	    return EINVAL;
	}
	input->nChain = strtoul(arg, (char**) NULL, 10);
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
	if (!(input->cubing || input->delay || input->delayThorp || input->enc || input->fpeThorp || input->fpeSoN || input->fpeSR)) { // no specific test set
	    // set all tests to true
	    input->cubing = input->delay = input->delayThorp = input->enc = input->fpeThorp = input->fpeSoN = input->fpeSR = true;
	}
    }
    default:
	return ARGP_ERR_UNKNOWN;
    }
    return 0; // exit successfully
}

static struct argp argp_struct = { options, parser_fun, args_doc , doc };








// ENTRYPOINT
int main(int argc, char **argv) {

    #ifdef _OMP
    omp_set_num_threads(20);
    omp_set_dynamic(1);
    omp_set_nested(1);
    omp_set_max_active_levels(5);

    printf("Max number of threads: %d\n", omp_get_max_threads());
    #endif
    
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

    unsigned long Rs[] = {100, 0, 0};
    unsigned long Cs[] = {10, 100};
    int numR = 3;
    int numC = 2;
    if (input.nRounds != 0) { // user specified something != 0
	Rs[0] = input.nRounds;
	numR = 1;
    }
    if (input.nChain != 0) {
	Cs[0] = input.nChain;
	numC = 1;
    }
    
    mpz_t p;
    unsigned long N; // bitsize of p

    mpz_init(p);



    printf("Output test results to file: %s\n", input.filename);
    printf("Testing %lu iterations of:\n", input.nIters);
    printf("cubing: %s\nboth-end encryption: %s\ndelay: %s ", BOOLSTR(input.cubing), BOOLSTR(input.enc), BOOLSTR(input.delay || input.delayThorp));

    if(input.delayThorp || input.delay) {
	printf("(using chain lengths:");
	for (int i=0; i< numC; ++i) printf(" %lu", Cs[i]);
	printf(")");
    }
    printf("\n");

    printf("delay uses both-ends encryption: %s\ndelay uses Thorp: %s\n", BOOLSTR(input.delay), BOOLSTR(input.delayThorp));
    
    printf("FPE methods: %s%s%s",
	   input.fpeThorp ? "Thorp " : "",
	   input.fpeSoN ? "Swap-or-Not " : "",
	   input.fpeSR ? "SometimesRecurse" : "");
    
    if (input.fpeThorp || input.fpeSoN || input.fpeSR) {
	printf(" (using round lengths: %lu", Rs[0]);
	if (numR > 1) printf(" primesize/2 primesize");
	printf(")\n");
    } else printf("none\n");
    
    printf("Prime sizes selected: ");
    if (input.pSize) printf("%lu\n", input.pSize);
    else {
	for (int i=0; i < numAvailablePrimes; ++i) printf("%lu ", availablePrimeSizes[i]);
	printf("\n");
    }

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


    
    
    for(unsigned long i=0; i < nPrimes; ++i) {
	constructPrime(p, primeSizes[i]);
	N = mpz_sizeinbase(p, 2);

	printf("Testing a prime of size %lu\n", N);

	if (input.cubing) { // test repeated squarings
	    testTimesSq(p, N, input.nIters, fileptr);
	    fflush(fileptr);
	    printf("Tested cubing\n");
	}
	
	if (input.enc) { // test both-end ecnryptions
	    testTimesEnc(p, input.nIters, fileptr);
	    fflush(fileptr);
	    printf("Tested both-ends encryption\n");
	}

	if (input.delay) {
	    for (int ci = 0; ci < numC; ++ci) {
		testTimesAll(p, 0, Cs[ci], input.nIters, fileptr);
		fflush(fileptr);
	    }
	    printf("Tested delay\n");
	}


	if (numR > 1){ // populate N/2 and N in rounds
	    Rs[1] = N / 2l;
	    Rs[2] = N;
	}

	if (input.delayThorp) { // test overall delay
	    for (int ri = 0; ri < numR; ++ri) {
		for (int ci = 0; ci < numC; ++ci) {
		    testTimesAll(p, Rs[ri], Cs[ci], input.nIters, fileptr);
		    fflush(fileptr);
		}
	    }
	    printf("Tested delay Thorp\n");
	}

	if (input.fpeThorp || input.fpeSoN || input.fpeSR) {
	    for (int ri = 0; ri < numR; ++ri) {
		testTimesFpe(p, Rs[ri], input.fpeThorp, input.fpeSoN, input.fpeSR, input.nIters, fileptr);
		fflush(fileptr);
	    }
	    printf("Tested FPEs\n");
	}

    }

    fprintf(fileptr, "\n\nEND TEST\n");
    for(int i=0; i<50; ++i) fprintf(fileptr, "=");
    fclose(fileptr);
    
    
    
    mpz_clear(p);
    return 0;
}

