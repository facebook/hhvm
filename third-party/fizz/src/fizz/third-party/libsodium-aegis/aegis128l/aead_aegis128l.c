
#include <errno.h>
#include <stdlib.h>

#include "crypto_aead_aegis128l.h"
#include <sodium.h>

#include "soft/aead_aegis128l_soft.h"

#include <fizz/third-party/libsodium-aegis/private/config.h>

#if FIZZ_LIBSODIUM_HAS_AESNI
#include "aesni/aead_aegis128l_aesni.h"
#endif

static const crypto_aead_aegis128l_implementation *implementation =
    &fizz_crypto_aead_aegis128l_soft_implementation;
static const aegis128l_evp* aegis_evp =
    &aegis128l_soft_evp;

size_t
fizz_aegis128l_keybytes(void)
{
    return fizz_aegis128l_KEYBYTES;
}

size_t
fizz_aegis128l_nsecbytes(void)
{
    return fizz_aegis128l_NSECBYTES;
}

size_t
fizz_aegis128l_npubbytes(void)
{
    return fizz_aegis128l_NPUBBYTES;
}

size_t
fizz_aegis128l_abytes(void)
{
    return fizz_aegis128l_ABYTES;
}

size_t
fizz_aegis128l_messagebytes_max(void)
{
    return fizz_aegis128l_MESSAGEBYTES_MAX;
}

void
fizz_aegis128l_keygen(unsigned char k[fizz_aegis128l_KEYBYTES])
{
    randombytes_buf(k, fizz_aegis128l_KEYBYTES);
}

int
fizz_aegis128l_encrypt(unsigned char *c, unsigned long long *clen_p, const unsigned char *m,
                              unsigned long long mlen, const unsigned char *ad,
                              unsigned long long adlen, const unsigned char *nsec,
                              const unsigned char *npub, const unsigned char *k)
{
    unsigned long long clen = 0ULL;
    int                ret;

    if (mlen > fizz_aegis128l_MESSAGEBYTES_MAX) {
        sodium_misuse();
    }
    ret = fizz_aegis128l_encrypt_detached(c, c + mlen, NULL, m, mlen, ad, adlen, nsec, npub,
                                                 k);
    if (clen_p != NULL) {
        if (ret == 0) {
            clen = mlen + 16ULL;
        }
        *clen_p = clen;
    }
    return ret;
}

int
fizz_aegis128l_decrypt(unsigned char *m, unsigned long long *mlen_p, unsigned char *nsec,
                              const unsigned char *c, unsigned long long clen,
                              const unsigned char *ad, unsigned long long adlen,
                              const unsigned char *npub, const unsigned char *k)
{
    unsigned long long mlen = 0ULL;
    int                ret  = -1;

    if (clen >= 16ULL) {
        ret = fizz_aegis128l_decrypt_detached(m, nsec, c, clen - 16ULL, c + clen - 16ULL, ad,
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
fizz_aegis128l_encrypt_detached(unsigned char *c, unsigned char *mac,
                                       unsigned long long *maclen_p, const unsigned char *m,
                                       unsigned long long mlen, const unsigned char *ad,
                                       unsigned long long adlen, const unsigned char *nsec,
                                       const unsigned char *npub, const unsigned char *k)
{
    return implementation->encrypt_detached(c, mac, maclen_p, m, mlen, ad, adlen, nsec, npub, k);
}

int
fizz_aegis128l_decrypt_detached(unsigned char *m, unsigned char *nsec,
                                       const unsigned char *c, unsigned long long clen,
                                       const unsigned char *mac, const unsigned char *ad,
                                       unsigned long long adlen, const unsigned char *npub,
                                       const unsigned char *k)
{
    return implementation->decrypt_detached(m, nsec, c, clen, mac, ad, adlen, npub, k);
}

int aegis128l_init_state(
    const unsigned char* key,
    const unsigned char* nonce,
    fizz_aegis_evp_ctx* ctx) {
  return aegis_evp->init_state(key, nonce, ctx);
}

int aegis128l_aad_update(
    const unsigned char* ad,
    unsigned long long adlen,
    fizz_aegis_evp_ctx* ctx) {
  return aegis_evp->aad_update(ad, adlen, ctx);
}

int aegis128l_aad_final(fizz_aegis_evp_ctx* ctx) {
  return aegis_evp->aad_final(ctx);
}

int aegis128l_encrypt_update(
    unsigned char* c,
    unsigned long long* c_writtenlen_p,
    const unsigned char* m,
    unsigned long long mlen,
    fizz_aegis_evp_ctx* ctx) {
  return aegis_evp->encrypt_update(c, c_writtenlen_p, m, mlen, ctx);
}

int aegis128l_encrypt_final(
    unsigned char* c,
    unsigned long long* c_writtenlen_p,
    unsigned long long mlen,
    unsigned long long adlen,
    fizz_aegis_evp_ctx* ctx) {
  return aegis_evp->encrypt_final(c, c_writtenlen_p, mlen, adlen, ctx);
}

int
fizz_aegis128l_pick_best_implementation(void)
{
    implementation = &fizz_crypto_aead_aegis128l_soft_implementation;
    aegis_evp = &aegis128l_soft_evp;

#if FIZZ_LIBSODIUM_HAS_AESNI
    if (sodium_runtime_has_aesni()) {
        implementation = &fizz_crypto_aead_aegis128l_aesni_implementation;
        aegis_evp = &aegis128l_aesni_evp;
        return 0;
    }
#endif
    return 0; /* LCOV_EXCL_LINE */
}
