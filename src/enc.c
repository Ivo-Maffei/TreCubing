#include "enc.h"

#include <openssl/evp.h>
#include <string.h> // memcpy
#define NDEBUG
#include <assert.h>

static EVP_CIPHER* aes256 = NULL;
static EVP_CIPHER_CTX* ctx = NULL;

int initialiseOpenSSL() {
    ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL){ // failed
	fprintf(stderr, "AES context creation failed\n");
	return -1;
    }

    aes256 = EVP_CIPHER_fetch(NULL, "AES-256-ECB", NULL); // fetch any implementation for the default library context
    if (aes256 == NULL) { // something failed
	fprintf(stderr, "AES fetch failed\n");
	return -1; // exit
    }

    return 0;
}


int bothEndsEnc(mpz_t c, const mpz_t m, const uint64_t N, const uint8_t *key){

    int t;
    uint8_t * buffer;
    mp_limb_t* cptr;

    const int encBytes = (N >> 7)*16; // floor(N/128) * 16; -> number of bytes that we feed to AES in 1 go
    const int byteSize = (N+7)/8; // smallest byte size to fit all bits
    const int offsetBytes = byteSize - encBytes; // number of bytes that we leave untouched (rounding up)
    const int bytePadding = N%8 ? 8 - (N%8) : 0; // number of extra bit to fill up the highest byte.
    const int nLimbs = (N+63) / 64; // num limbs to contain p

    if (ctx == NULL) {
	if (initialiseOpenSSL() != 0){
	    fprintf(stderr, "Initialisation failed\n");
	    return -1;
	}
    }
        
    // pass key and cipher to the context
    if (1 != EVP_EncryptInit_ex2(ctx, aes256, key , NULL, NULL)){
	fprintf(stderr, "Failed to init context\n");
	return -1;
    }

    EVP_CIPHER_CTX_set_padding(ctx, 0); // disable padding

    // now do the actual encryption
    mpz_set(c, m); // set c to have the same value of m
    cptr = mpz_limbs_modify(c, nLimbs); // create read-write limbs array with ceil(N/64) libms
    buffer = malloc(nLimbs * sizeof(uint64_t)); // we need the size to be a limb multiple to use mpn_shift on buffer
    // we make sure that the top unused bytes are 0
    for(int i=byteSize; i < nLimbs*8; ++i) buffer[i] = 0x00; 

    // encrypt the lower encBytes
    if (1 != EVP_EncryptUpdate(ctx, buffer, &t, ((uint8_t*)cptr), encBytes)) {
	fprintf(stderr, "First enc failed\n");
	free(buffer);
	return -1;
    }
    assert(t == encBytes);

    // copy the remaing bytes to buffer
    // note that the top bytePadding bits of the highest byte in cptr are 0s
    memcpy(buffer+encBytes, ((uint8_t*)cptr)+encBytes, offsetBytes);

    // encrypt the higher encBytes
    // to do so we need to shift everything so that the highest byte is fully used
    // note that we add bytePadding 0 bits in the lowest byte
    mpn_lshift(((mp_limb_t*)buffer), ((mp_limb_t*)buffer), nLimbs, bytePadding);

    if (1 != EVP_EncryptUpdate(ctx, ((uint8_t*)cptr)+offsetBytes, &t, buffer + offsetBytes, encBytes)){
	fprintf(stderr, "Second enc failed\n");
	free(buffer);
	return -1;
    }
    assert(t == encBytes);

    // now we need to copy the offsetted bytes
    // here we copy the lowest bytePadding 0 bits which are not part of our number
    memcpy(((uint8_t*)cptr), buffer, offsetBytes);

    // now we need to shift everything right removing the bytePadding zeros
    // note that we introduced bytePadding new bits at the top, but these are 0
    mpn_rshift(cptr, cptr, nLimbs, bytePadding);

    // FINALISE EVERYTHING
    if (1 != EVP_EncryptFinal_ex(ctx, buffer, &t)){
	fprintf(stderr, "Finalisation failed\n");
	return -1;
    }
    assert(t == 0);

    mpz_limbs_finish(c, nLimbs);
    
    free(buffer);
    return 0; // done!
}


int bothEndsDec(mpz_t m, const mpz_t c, const uint64_t N, const unsigned char *key){
    int t;
    uint8_t * buffer;
    mp_limb_t* mptr;

    const int encBytes = (N >> 7)*16; // floor(N/128) * 16; -> number of bytes that we feed to AES in 1 go
    const int byteSize = (N+7)/8; // smallest byte size to fit all bits
    const int offsetBytes = byteSize - encBytes; // number of bytes that we leave untouched (rounding up)
    const int bytePadding = N%8 ? 8 - (N%8) : 0; // number of extra bit to fill up the highest byte.
    const int nLimbs = (N+63) / 64; // num limbs to contain p

    if (ctx == NULL) {
	t = initialiseOpenSSL();
	if (t != 0){
	    fprintf(stderr, "Initialisation failed\n");
	    return -1;
	}
    }
       
    // pass key and cipher to the context
    if (1 != EVP_DecryptInit_ex2(ctx, aes256, key , NULL, NULL)){
	fprintf(stderr, "Failed to init context\n");
	return -1;
    }

    EVP_CIPHER_CTX_set_padding(ctx, 0); // disable padding

    // do actual decryption
    mpz_set(m, c);
    mptr = mpz_limbs_modify(m, nLimbs);
    buffer = malloc(nLimbs*sizeof(uint64_t)); // we need the size to be a limb multiple to use mpn_shift on buffer
    // we make sure that the top unused bytes are 0
    for(int i=byteSize; i < nLimbs*8; ++i) buffer[i] = 0; 

    // first decrypt the higher encBytes:

    // move up to fill the top byte
    mpn_lshift(mptr, mptr, nLimbs, bytePadding);

    memcpy(buffer, ((uint8_t*)mptr), offsetBytes); // copy lower bits
    
    if (1 != EVP_DecryptUpdate(ctx, buffer+offsetBytes, &t, ((uint8_t*)mptr)+offsetBytes, encBytes)){
	fprintf(stderr, "First dec failed\n");
	free(buffer);
	return -1;
    }
    assert(t == encBytes);

    // shift down
    mpn_rshift(((mp_limb_t*)buffer), ((mp_limb_t*)buffer), nLimbs, bytePadding);

    // note that we are copying also the bytePadding highest bytes of the top significant byte
    // however, these are 0s
    memcpy(((uint8_t*)mptr) + encBytes, buffer + encBytes, offsetBytes);
    

    if (1 != EVP_DecryptUpdate(ctx, ((uint8_t*)mptr), &t, buffer, encBytes)) {
	fprintf(stderr, "Second dec failed\n");
	free(buffer);
	return -1;
    }
    assert(t == encBytes);

    // FINALISE EVERYTHING
    if (1 != EVP_DecryptFinal_ex(ctx, buffer, &t)){
	fprintf(stderr, "Finalisation dec failed\n");
	return -1;
    }
    assert(t == 0);

    mpz_limbs_finish(m, nLimbs);
    
    free(buffer);
    return 0;
}


void cycleEnc(mpz_t c, const mpz_t m, const mpz_t p, const uint8_t *key) {
    const uint64_t N = mpz_sizeinbase(p, 2);
    mpz_set(c, m);
    do {
	if (0 != bothEndsEnc(c, c, N, key)) {
	    fprintf(stderr, "Both-Ends Enc failed\n");
	    return;
	}
    } while (mpz_cmp(c, p) >= 0 || mpz_sgn(c) == 0); // repeat if c >= p or c == 0
}


void cycleDec(mpz_t m, const mpz_t c, const mpz_t p, const uint8_t *key) {
    const uint64_t N = mpz_sizeinbase(p, 2);
    mpz_set(m, c);
    do {
	if ( 0 != bothEndsDec(m, m, N, key)) {
	    fprintf(stderr, "Both-Ends Dec failed\n");
	    return;
	}
    } while (mpz_cmp(m, p) >= 0 || mpz_sgn(m) == 0);
}


void cleanOpenSSL() {
    if (aes256) EVP_CIPHER_free(aes256);
    if (ctx)EVP_CIPHER_CTX_free(ctx);
    aes256 = NULL;
    ctx = NULL;
}

