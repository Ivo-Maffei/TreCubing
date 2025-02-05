#include "enc.h"

#include <openssl/evp.h>
#include <string.h> // memcpy
#include <stdbool.h>
//#define NDEBUG
#include <assert.h>

static EVP_CIPHER* aes256 = NULL;
static EVP_CIPHER* aes256ofb = NULL;
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

    aes256ofb = EVP_CIPHER_fetch(NULL, "AES-256-OFB", NULL); // fetch any implementation for the default library context
    if (aes256 == NULL) { // something failed
	fprintf(stderr, "AES-OFB fetch failed\n");
	return -1; // exit
    }

    return 0;
}

// N is the bitsize of the modulo (i.e. largest bitsize of m)
int bothEndsEnc(mpz_t c, const mpz_t m, const uint64_t N, const uint8_t *key){

    int t;
    uint8_t * buffer;
    mp_limb_t* cptr;

    const int encBytes = (N >> 7)*16; // floor(N/128) * 16; -> number of bytes that we feed to AES in 1 go
    const int byteSize = (N+7)/8; // smallest byte size to fit all bits
    const int offsetBytes = byteSize - encBytes; // number of bytes that we leave untouched (rounding up)
    const int bytePadding = N%8 ? 8 - (N%8) : 0; // number of extra bit to fill up the highest byte.
    const int nLimbs = (N+63) / 64; // num limbs to contain p
    const int bufferSize = nLimbs * 8; // size in bytes of the buffer of nLimbs limbs

    if (ctx == NULL) {
	if (initialiseOpenSSL() != 0){
	    fprintf(stderr, "Initialisation failed\n");
	    return -1;
	}
    }

    // pass key and cipher to the context
    if (1 != EVP_EncryptInit_ex2(ctx, aes256, key , NULL, NULL)){ // the 2 nulls are iv and params
	fprintf(stderr, "Failed to init context\n");
	return -1;
    }

    EVP_CIPHER_CTX_set_padding(ctx, 0); // disable padding

    // now do the actual encryption
    mpz_set(c, m); // set c to have the same value of m
    cptr = mpz_limbs_modify(c, nLimbs); // create read-write limbs array with ceil(N/64) libms

    // zero new limbs
    for (int i=mpz_size(m); i < nLimbs; ++i) cptr[i] = 0;

    buffer = malloc(nLimbs * sizeof(uint64_t)); // we need the size to be a limb multiple to use mpn_shift on buffer

    // we make sure that the top unused bytes are 0
    for(int i=0; i < bufferSize; ++i) buffer[i] = 0x00;

    // encrypt the lower encBytes
    if (!EVP_EncryptUpdate(ctx, buffer, &t, ((uint8_t*)cptr), encBytes)) {
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
    if (bytePadding > 0) { //lshift is not supported if bytePadding is 0
	mpn_lshift(((mp_limb_t*)buffer), ((mp_limb_t*)buffer), nLimbs, bytePadding);
    }

    if (!EVP_EncryptUpdate(ctx, ((uint8_t*)cptr)+offsetBytes, &t, buffer + offsetBytes, encBytes)){
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
    if (bytePadding > 0) {// rshift not supported for bytePadding == 0
	mpn_rshift(cptr, cptr, nLimbs, bytePadding);
    }

    // FINALISE EVERYTHING
    if (!EVP_EncryptFinal_ex(ctx, buffer, &t)){
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
    const int bufferSize = nLimbs * 8; // size in bytes of the buffer of nLimbs limbs

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

    // zero new limbs
    for (int i=mpz_size(c); i < nLimbs; ++i) mptr[i] = 0;

    buffer = malloc(nLimbs*sizeof(uint64_t)); // we need the size to be a limb multiple to use mpn_shift on buffer
    // we make sure that the top unused bytes are 0
    for(int i=byteSize; i < bufferSize; ++i) buffer[i] = 0;

    // first decrypt the higher encBytes:

    // move up to fill the top byte
    // gmp does not support shifting by 0 bytes
    if (bytePadding >0) mpn_lshift(mptr, mptr, nLimbs, bytePadding);

    memcpy(buffer, ((uint8_t*)mptr), offsetBytes); // copy lower bits

    if (!EVP_DecryptUpdate(ctx, buffer+offsetBytes, &t, ((uint8_t*)mptr)+offsetBytes, encBytes)){
	fprintf(stderr, "First dec failed\n");
	free(buffer);
	return -1;
    }
    assert(t == encBytes);

    // shift down
    // gmp assumings we are shifting by at least 1 bit
    if (bytePadding >0) mpn_rshift(((mp_limb_t*)buffer), ((mp_limb_t*)buffer), nLimbs, bytePadding);

    // note that we are copying also the bytePadding highest bytes of the top significant byte
    // however, these are 0s
    memcpy(((uint8_t*)mptr) + encBytes, buffer + encBytes, offsetBytes);

    if (!EVP_DecryptUpdate(ctx, ((uint8_t*)mptr), &t, buffer, encBytes)) {
	fprintf(stderr, "Second dec failed\n");
	free(buffer);
	return -1;
    }
    assert(t == encBytes);

    // FINALISE EVERYTHING
    if (!EVP_DecryptFinal_ex(ctx, buffer, &t)){
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
    } while (mpz_cmp(c, p) >= 0); // repeat if c >= p
}


void cycleDec(mpz_t m, const mpz_t c, const mpz_t p, const uint8_t *key) {
    const uint64_t N = mpz_sizeinbase(p, 2);
    mpz_set(m, c);
    do {
	if ( 0 != bothEndsDec(m, m, N, key)) {
	    fprintf(stderr, "Both-Ends Dec failed\n");
	    return;
	}
    } while (mpz_cmp(m, p) >= 0);
}

// encrypt m with a cycle walking AES-OFB where the IV is the first 128-bits of the key
int streamCipher(mpz_t c, const mpz_t m, const mpz_t M, const uint8_t *key) {

    const uint64_t N = mpz_sizeinbase(M, 2);
    const int nbytes = (N+7)/8;
    const int bitsPadding = nbytes*8 - N;
    const int nLimbs = mpz_size(M); // num limbs to contain the modulo
    const int bufferSize = nLimbs * 8;
    mp_limb_t* cptr;
    const mp_limb_t* modptr;
    uint8_t * buffer;
    int t = 1;
    bool lendian = ((uint8_t*)&t)[0] & 1; // check if int is stored LSB first or MSB
    assert(lendian); // no need to handle big endian as all my machines are little endian

    if (ctx == NULL) {
	if (initialiseOpenSSL() != 0){
	    fprintf(stderr, "Initialisation failed\n");
	    return -1;
	}
    }

    // pass key and cipher to the context
    // the first 16 bytes (128-bits) of the key are the IV!
    if (1 != EVP_EncryptInit_ex2(ctx, aes256ofb, key+16 , key, NULL)){ // the null is params
	fprintf(stderr, "Failed to init context\n");
	return -1;
    }
    EVP_CIPHER_CTX_set_padding(ctx, 0); // disable padding

    // to encrypt m, we first copy it to a buffer of the correct size
    // since m might be shorter
    buffer = (uint8_t*) malloc(nLimbs*sizeof(uint64_t));
    modptr = mpz_limbs_read(M);

    // copy over m to c
    mpz_set(c, m);
    cptr = mpz_limbs_modify(c, nLimbs);
    // now zero everything else
    for (size_t i=mpz_size(m); i < nLimbs; ++i) {
	cptr[i]=0;
    }
    // now cptr does contain only m

    do {
	memcpy(buffer, cptr, bufferSize); // copy bufferSize bytes (nLimbs limbs) to buffer

	// we only need to encrpt N bits, so encrypt nbytes and zero the top bits
	// note that this is possible since we are storing 64-bits words in little endian
	// otherwise the first byte of the last limb is the top-most byte!
	if (!EVP_EncryptUpdate(ctx, ((uint8_t*)cptr), &t, buffer, nbytes)) {
	    fprintf(stderr, "Enc failed\n");
	    free(buffer);
	    return -1;
	}
	assert(t==nbytes);

	// we encrypted nbytes, but only need N bits
	// so we zero the top nbytes*8-N bits
	((uint8_t*)cptr)[nbytes-1] &= (0xff>>(bitsPadding));
    } while (mpn_cmp(cptr, modptr, nLimbs) >= 0); // cptr >= modptr

    // FINALISE EVERYTHING
    if (!EVP_EncryptFinal_ex(ctx, ((uint8_t*)cptr), &t)){
	fprintf(stderr, "Finalisation failed\n");
	return -1;
    }
    assert(t==0);

    mpz_limbs_finish(c, nLimbs);
    free(buffer);
    return 0; // done!
}


void cleanOpenSSL() {
    if (aes256) EVP_CIPHER_free(aes256);
    if (aes256ofb) EVP_CIPHER_free(aes256ofb);
    if (ctx)EVP_CIPHER_CTX_free(ctx);
    aes256 = NULL;
    ctx = NULL;
}
