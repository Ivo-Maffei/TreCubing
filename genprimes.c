#include <gmp.h>
#include <stdio.h>
#include <stdlib.h> // convert strings to int/longs
#include <string.h> // to parse strings

#include <time.h> // TODO: remove this

#define N_PRIMES 203280220
#define N_REFINED_PRIMES 101642128
#define N_BEST_PRIMES 49091941


int refineprimes(){

    FILE *db, *newdb;
    db = fopen("primes.32b", "rb");
    if (!db) return 1;

    newdb = fopen("bestprimes.32b", "wb");
    if (!newdb) return 1;

    unsigned long count = 0;
    uint32_t prime;


    while (!feof(db)) {// more bytes to read

	if (fread(&prime, sizeof(prime), 1, db) != 1) {
	    if(ferror(db)) fprintf(stderr, "Error while reading file\n");
	    else if(feof(db)) printf("EOF reached\n");
	    else printf("some error\n");
	    break;
	}

	if (prime > (1l<<31) && prime%3 == 2) { // good for new database
	    if (fwrite(&prime, sizeof(prime), 1, newdb) != 1){
		fprintf(stderr, "Error while writing file\n");
		break;
	    }
	    ++count;
	}
    }

    printf("Written %lu primes\n", count);
    fclose(db);
    fclose(newdb);
    return 0;
}

int getprimes(const int numprimes){

    FILE *db;
    db = fopen("refinedprimes.32b", "rb");
    if (!db) return 1;

    uint32_t place;
    uint32_t* dbprimes = malloc(N_BEST_PRIMES*sizeof(uint32_t));
    uint32_t *primes = (uint32_t *) malloc(numprimes*sizeof(uint32_t));

    size_t e = fread(dbprimes, sizeof(*dbprimes), N_BEST_PRIMES, db);
    if (e != N_BEST_PRIMES) {
	fprintf(stderr, "ERROR read less primes: %lu instead of %lu\n", e, N_BEST_PRIMES);
	return 1;
    }

    for (int i=0; i<numprimes; ++i){
	fprintf(stderr, ".");
	place = rand() % N_BEST_PRIMES;

	primes[i] = dbprimes[place];
	if (primes[i]%3 != 2) fprintf(stderr, "found a BAD prime\n");
    }
    
    fclose(db);
    return 0;
}


int main(int argc, char **argv) {

    /* refineprimes(); */
    /* return 0; */

    time_t start, end;
    
    start =time(NULL);
    if(getprimes(1000000)) fprintf(stderr, "error!\n");
    end = time(NULL);
    
    fprintf(stderr, "it took %lu s in total\n", (end-start));
    
    return 0;
    
    // see how long to generate tons of 32 bit primes using GMP

    mpz_t p;
    
    mpz_init(p);

    mpz_set_ui(p, (1l<<31)); // p is smallest 32-bit prime
    printf("p has size %lu\n", mpz_sizeinbase(p, 2));

    unsigned long count = 0;

    start = time(NULL);
    while (mpz_sizeinbase(p, 2) == 32) {
	mpz_nextprime(p, p);
	if (count % 100 == 0) {
	    //fprintf(stderr, "%lu th prime found: %lu\n", count, mpz_get_ui(p));
	    fprintf(stderr, ".");
	}
	++count;
    }
    end = time(NULL);

    fprintf(stderr, "computed %lu (pseudo) primes in %lu seconds\n", count, end-start);
    mpz_clear(p);
    return 0;
}
