#include "enc.h"

#include <openssl/evp.h>
#include <string.h> // memcpy
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

    int t = 1; // set to 1 so we can check endianess
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

    buffer = (uint8_t*) malloc(nLimbs*sizeof(uint64_t));

    modptr = mpz_limbs_read(M);

    // to encrypt m, we first copy it to cptr
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

	// now we need to repeat if the resulting ciphertext is not in ZZ^*_M
	// since M = p^k, then the probabilty that x in ZZ_M is not in ZZ^*_M is negligible (1/p)
	// hence we just check that the ciphertext is less than M
    } while (mpn_cmp(cptr, modptr, nLimbs) >= 0);

    // FINALISE EVERYTHING
    if (!EVP_EncryptFinal_ex(ctx, ((uint8_t*)cptr), &t)){
	fprintf(stderr, "Finalisation failed\n");
	return -1;
    }
    assert(t==0);

    free(buffer);
    return 0; // done!
}


void cleanOpenSSL() {
    if (aes256) EVP_CIPHER_free(aes256);
    if (aes256ofb) EVP_CIPHER_free(aes256ofb);
    if (ctx)EVP_CIPHER_CTX_free(ctx);
    aes256 = NULL;
    aes256ofb = NULL;
    ctx = NULL;
}
