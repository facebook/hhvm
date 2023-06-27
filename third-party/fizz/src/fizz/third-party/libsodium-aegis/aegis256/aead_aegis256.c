
#include <errno.h>
#include <stdlib.h>

#include "crypto_aead_aegis256.h"
#include <sodium.h>

#include "aead_aegis256.h"

#include "soft/aead_aegis256_soft.h"

#include <fizz/third-party/libsodium-aegis/private/config.h>

#if FIZZ_LIBSODIUM_HAS_AESNI
#include "aesni/aead_aegis256_aesni.h"
#endif

static const crypto_aead_aegis256_implementation *implementation =
    &fizz_crypto_aead_aegis256_soft_implementation;

size_t
fizz_aegis256_keybytes(void)
{
    return fizz_aegis256_KEYBYTES;
}

size_t
fizz_aegis256_nsecbytes(void)
{
    return fizz_aegis256_NSECBYTES;
}

size_t
fizz_aegis256_npubbytes(void)
{
    return fizz_aegis256_NPUBBYTES;
}

size_t
fizz_aegis256_abytes(void)
{
    return fizz_aegis256_ABYTES;
}

size_t
fizz_aegis256_messagebytes_max(void)
{
    return fizz_aegis256_MESSAGEBYTES_MAX;
}

void
fizz_aegis256_keygen(unsigned char k[fizz_aegis256_KEYBYTES])
{
    randombytes_buf(k, fizz_aegis256_KEYBYTES);
}

int
fizz_aegis256_encrypt(unsigned char *c, unsigned long long *clen_p, const unsigned char *m,
                             unsigned long long mlen, const unsigned char *ad,
                             unsigned long long adlen, const unsigned char *nsec,
                             const unsigned char *npub, const unsigned char *k)
{
    unsigned long long clen = 0ULL;
    int                ret;

    if (mlen > fizz_aegis256_MESSAGEBYTES_MAX) {
        sodium_misuse();
    }
    ret =
        fizz_aegis256_encrypt_detached(c, c + mlen, NULL, m, mlen, ad, adlen, nsec, npub, k);
    if (clen_p != NULL) {
        if (ret == 0) {
            clen = mlen + 16ULL;
        }
        *clen_p = clen;
    }
    return ret;
}

int
fizz_aegis256_decrypt(unsigned char *m, unsigned long long *mlen_p, unsigned char *nsec,
                             const unsigned char *c, unsigned long long clen,
                             const unsigned char *ad, unsigned long long adlen,
                             const unsigned char *npub, const unsigned char *k)
{
    unsigned long long mlen = 0ULL;
    int                ret  = -1;

    if (clen >= 16ULL) {
        ret = fizz_aegis256_decrypt_detached(m, nsec, c, clen - 16ULL, c + clen - 16ULL, ad,
                                                    adlen, npub, k);
    }
    if (mlen_p != NULL) {
        if (ret == 0) {
            mlen = clen - 16ULL;
        }
        *mlen_p = mlen;
    }
    return ret;
}

int
fizz_aegis256_encrypt_detached(unsigned char *c, unsigned char *mac,
                                      unsigned long long *maclen_p, const unsigned char *m,
                                      unsigned long long mlen, const unsigned char *ad,
                                      unsigned long long adlen, const unsigned char *nsec,
                                      const unsigned char *npub, const unsigned char *k)
{
    return implementation->encrypt_detached(c, mac, maclen_p, m, mlen, ad, adlen, nsec, npub, k);
}

int
fizz_aegis256_decrypt_detached(unsigned char *m, unsigned char *nsec, const unsigned char *c,
                                      unsigned long long clen, const unsigned char *mac,
                                      const unsigned char *ad, unsigned long long adlen,
                                      const unsigned char *npub, const unsigned char *k)
{
    return implementation->decrypt_detached(m, nsec, c, clen, mac, ad, adlen, npub, k);
}

int
fizz_aegis256_pick_best_implementation(void)
{
    implementation = &fizz_crypto_aead_aegis256_soft_implementation;

#if FIZZ_LIBSODIUM_HAS_AESNI
    if (sodium_runtime_has_aesni()) {
        implementation = &fizz_crypto_aead_aegis256_aesni_implementation;
        return 0;
    }
#endif
    return 0; /* LCOV_EXCL_LINE */
}
