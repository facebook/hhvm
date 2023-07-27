#ifndef fizz_crypto_aead_aegis128l_H
#define fizz_crypto_aead_aegis128l_H

#include <stddef.h>
#include <sodium.h>

#include <fizz/third-party/libsodium-aegis/private/state.h>

#ifdef __cplusplus
# ifdef __GNUC__
#  pragma GCC diagnostic ignored "-Wlong-long"
# endif
extern "C" {
#endif

#define fizz_aegis128l_KEYBYTES 16U
SODIUM_EXPORT
size_t fizz_aegis128l_keybytes(void);

#define fizz_aegis128l_NSECBYTES 0U
SODIUM_EXPORT
size_t fizz_aegis128l_nsecbytes(void);

#define fizz_aegis128l_NPUBBYTES 16U
SODIUM_EXPORT
size_t fizz_aegis128l_npubbytes(void);

#define fizz_aegis128l_ABYTES    16U
SODIUM_EXPORT
size_t fizz_aegis128l_abytes(void);

#define fizz_aegis128l_MESSAGEBYTES_MAX \
    SODIUM_MIN(SODIUM_SIZE_MAX - fizz_aegis128l_ABYTES, \
               (1ULL << 61) - 1)
SODIUM_EXPORT
size_t fizz_aegis128l_messagebytes_max(void);

SODIUM_EXPORT
int fizz_aegis128l_encrypt(unsigned char *c,
                                  unsigned long long *clen_p,
                                  const unsigned char *m,
                                  unsigned long long mlen,
                                  const unsigned char *ad,
                                  unsigned long long adlen,
                                  const unsigned char *nsec,
                                  const unsigned char *npub,
                                  const unsigned char *k)
            __attribute__ ((nonnull(1, 8, 9)));

SODIUM_EXPORT
int fizz_aegis128l_decrypt(unsigned char *m,
                                  unsigned long long *mlen_p,
                                  unsigned char *nsec,
                                  const unsigned char *c,
                                  unsigned long long clen,
                                  const unsigned char *ad,
                                  unsigned long long adlen,
                                  const unsigned char *npub,
                                  const unsigned char *k)
            __attribute__ ((warn_unused_result)) __attribute__ ((nonnull(4, 8, 9)));

SODIUM_EXPORT
int fizz_aegis128l_encrypt_detached(unsigned char *c,
                                           unsigned char *mac,
                                           unsigned long long *maclen_p,
                                           const unsigned char *m,
                                           unsigned long long mlen,
                                           const unsigned char *ad,
                                           unsigned long long adlen,
                                           const unsigned char *nsec,
                                           const unsigned char *npub,
                                           const unsigned char *k)
            __attribute__ ((nonnull(1, 2, 9, 10)));

SODIUM_EXPORT
int fizz_aegis128l_decrypt_detached(unsigned char *m,
                                           unsigned char *nsec,
                                           const unsigned char *c,
                                           unsigned long long clen,
                                           const unsigned char *mac,
                                           const unsigned char *ad,
                                           unsigned long long adlen,
                                           const unsigned char *npub,
                                           const unsigned char *k)
            __attribute__ ((warn_unused_result)) __attribute__ ((nonnull(3, 5, 8, 9)));

SODIUM_EXPORT
void fizz_aegis128l_keygen(unsigned char k[fizz_aegis128l_KEYBYTES])
            __attribute__ ((nonnull));

SODIUM_EXPORT
int aegis128l_init_state(
    const unsigned char* key,
    const unsigned char* nonce,
    fizz_aegis_evp_ctx *ctx) __attribute__((nonnull(1, 2, 3)));

SODIUM_EXPORT
int aegis128l_aad_update(
    const unsigned char* ad,
    unsigned long long adlen,
    fizz_aegis_evp_ctx *ctx) __attribute__((nonnull(1, 3)));

SODIUM_EXPORT
int aegis128l_aad_final(fizz_aegis_evp_ctx *ctx) __attribute__((nonnull(1)));

SODIUM_EXPORT
int aegis128l_encrypt_update(
    unsigned char* c,
    unsigned long long* c_writtenlen_p,
    const unsigned char* m,
    unsigned long long mlen,
    fizz_aegis_evp_ctx *ctx) __attribute__((nonnull(1, 3, 5)));

SODIUM_EXPORT
int aegis128l_encrypt_final(
    unsigned char* c,
    unsigned long long* c_writtenlen_p,
    unsigned char *mac,
    fizz_aegis_evp_ctx *ctx) __attribute__((nonnull(1, 3, 4)));

SODIUM_EXPORT
int aegis128l_decrypt_update(
      unsigned char* m,
      unsigned long long* outlen,
      const unsigned char* c,
      unsigned long long clen,
      fizz_aegis_evp_ctx* ctx) __attribute__((nonnull(1, 3, 5)));

SODIUM_EXPORT
int aegis128l_decrypt_final(
      unsigned char* m,
      unsigned long long* outlen,
      const unsigned char* mac,
      fizz_aegis_evp_ctx* ctx) __attribute__((nonnull(1, 3, 4)));

SODIUM_EXPORT
int fizz_aegis128l_pick_best_implementation();

#ifdef __cplusplus
}
#endif

#endif
