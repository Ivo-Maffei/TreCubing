#include "hash.h"

#include <openssl/evp.h>
#include <assert.h>

static EVP_MD* sha256 = NULL;
static EVP_MD_CTX* ctx = NULL;

int initialiseHashing() {
    ctx = EVP_MD_CTX_new();
    if (ctx == NULL){ // failed
	fprintf(stderr, "SHA3-256 context creation failed\n");
	return -1;
    }

    sha256 = EVP_MD_fetch(NULL, "SHA3-256", NULL); // fetch any implementation for the default library context
    if (sha256 == NULL) { // something failed
	fprintf(stderr, "SHA3-256 fetch failed\n");
	return -1; // exit
    }

    return 0;
}

void cleanHashing() {
    if (sha256) EVP_MD_free(sha256);
    if (ctx)EVP_MD_CTX_free(ctx);
    sha256 = NULL;
    ctx = NULL;
}

size_t hash(uint8_t* digest, const mpz_t input){
    unsigned int digestLength;

    if (digest == NULL) { // allocate memory
	digest = (uint8_t*) malloc(32*sizeof(uint8_t)); // allocate 256 bits
	assert(digest);
    }

    // hash mpz_size(input)*8 bytes of inputs and save them in digest
    // digestLength will be the number of bytes of the hashed value (should be 32)
    // use sha256 with default implementation
    EVP_Digest(mpz_limbs_read(input), mpz_size(input)*8, digest, &digestLength, sha256, NULL);
    assert(digestLength == 32);

    return digestLength;
}
