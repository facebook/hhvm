#ifndef fizz_aead_aegis128l_H
#define fizz_aead_aegis128l_H

#include <fizz/third-party/libsodium-aegis/private/state.h>

typedef struct crypto_aead_aegis128l_implementation {
    int (*encrypt_detached)(unsigned char *c, unsigned char *mac, unsigned long long *maclen_p,
                            const unsigned char *m, unsigned long long mlen,
                            const unsigned char *ad, unsigned long long adlen,
                            const unsigned char *nsec, const unsigned char *npub,
                            const unsigned char *k);
    int (*decrypt_detached)(unsigned char *m, unsigned char *nsec, const unsigned char *c,
                            unsigned long long clen, const unsigned char *mac,
                            const unsigned char *ad, unsigned long long adlen,
                            const unsigned char *npub, const unsigned char *k);
} crypto_aead_aegis128l_implementation;

typedef struct aegis128l_evp {
  int (*init_state)(
      const unsigned char* key,
      const unsigned char* nonce,
      fizz_aegis_evp_ctx* ctx);
  int (*aad_update)(
      const unsigned char* ad,
      unsigned long long adlen,
      fizz_aegis_evp_ctx *ctx);
  int (*aad_final) (fizz_aegis_evp_ctx* ctx);
  int (*encrypt_update)(
      unsigned char* c,
      unsigned long long* c_writtenlen_p,
      const unsigned char* m,
      unsigned long long mlen,
      fizz_aegis_evp_ctx* ctx);
  int (*encrypt_final)(
      unsigned char* c,
      unsigned long long* c_writtenlen_p,
      unsigned long long mlen,
      unsigned long long adlen,
      fizz_aegis_evp_ctx* ctx);
} aegis128l_evp;

#endif
