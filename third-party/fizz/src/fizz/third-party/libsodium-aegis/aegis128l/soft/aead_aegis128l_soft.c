#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sodium.h>

#include "fizz/third-party/libsodium-aegis/private/common.h"
#include "fizz/third-party/libsodium-aegis/private/softaes.h"

#include "aead_aegis128l_soft.h"

typedef SoftAesBlock aes_block_t;
#define STATE aes_block_t *const state = (aes_block_t *const)
#define STATE_SIZE 32U
#define TAG_LEN 16U
#ifndef _MSC_VER
_Static_assert(sizeof(SoftAesBlock) == sizeof(OpaqueSoftAesState), "Size is not correct");
#endif
#define AES_BLOCK_XOR(A, B)       softaes_block_xor((A), (B))
#define AES_BLOCK_AND(A, B)       softaes_block_and((A), (B))
#define AES_BLOCK_LOAD(A)         softaes_block_load(A)
#define AES_BLOCK_LOAD_64x2(A, B) softaes_block_load64x2((A), (B))
#define AES_BLOCK_STORE(A, B)     softaes_block_store((A), (B))
#define AES_ENC(A, B)             softaes_block_encrypt((A), (B))

static inline void
aegis128l_update(aes_block_t *const state, const aes_block_t d1, const aes_block_t d2)
{
    aes_block_t tmp;

    tmp      = state[7];
    state[7] = AES_ENC(state[6], state[7]);
    state[6] = AES_ENC(state[5], state[6]);
    state[5] = AES_ENC(state[4], state[5]);
    state[4] = AES_ENC(state[3], state[4]);
    state[3] = AES_ENC(state[2], state[3]);
    state[2] = AES_ENC(state[1], state[2]);
    state[1] = AES_ENC(state[0], state[1]);
    state[0] = AES_ENC(tmp, state[0]);

    state[0] = AES_BLOCK_XOR(state[0], d1);
    state[4] = AES_BLOCK_XOR(state[4], d2);
}

static void
aegis128l_init(const unsigned char *key, const unsigned char *nonce, aes_block_t *const state)
{
    static CRYPTO_ALIGN(16)
        const unsigned char c0_[] = { 0xdb, 0x3d, 0x18, 0x55, 0x6d, 0xc2, 0x2f, 0xf1,
                                      0x20, 0x11, 0x31, 0x42, 0x73, 0xb5, 0x28, 0xdd };
    static CRYPTO_ALIGN(16)
        const unsigned char c1_[] = { 0x00, 0x01, 0x01, 0x02, 0x03, 0x05, 0x08, 0x0d,
                                      0x15, 0x22, 0x37, 0x59, 0x90, 0xe9, 0x79, 0x62 };
    const aes_block_t       c0    = AES_BLOCK_LOAD(c0_);
    const aes_block_t       c1    = AES_BLOCK_LOAD(c1_);
    aes_block_t             k;
    aes_block_t             n;
    int                     i;

    k = AES_BLOCK_LOAD(key);
    n = AES_BLOCK_LOAD(nonce);

    state[0] = AES_BLOCK_XOR(k, n);
    state[1] = c0;
    state[2] = c1;
    state[3] = c0;
    state[4] = AES_BLOCK_XOR(k, n);
    state[5] = AES_BLOCK_XOR(k, c1);
    state[6] = AES_BLOCK_XOR(k, c0);
    state[7] = AES_BLOCK_XOR(k, c1);
    for (i = 0; i < 10; i++) {
        aegis128l_update(state, n, k);
    }
}

static void
aegis128l_mac(unsigned char *mac, unsigned long long adlen, unsigned long long mlen,
              aes_block_t *const state)
{
    aes_block_t tmp;
    int         i;

    tmp = AES_BLOCK_LOAD_64x2(mlen << 3, adlen << 3);
    tmp = AES_BLOCK_XOR(tmp, state[2]);

    for (i = 0; i < 7; i++) {
        aegis128l_update(state, tmp, tmp);
    }

    tmp = AES_BLOCK_XOR(state[6], state[5]);
    tmp = AES_BLOCK_XOR(tmp, state[4]);
    tmp = AES_BLOCK_XOR(tmp, state[3]);
    tmp = AES_BLOCK_XOR(tmp, state[2]);
    tmp = AES_BLOCK_XOR(tmp, state[1]);
    tmp = AES_BLOCK_XOR(tmp, state[0]);

    AES_BLOCK_STORE(mac, tmp);
}

static inline void
aegis128l_absorb(const unsigned char *const src, aes_block_t *const state)
{
    aes_block_t msg0, msg1;

    msg0 = AES_BLOCK_LOAD(src);
    msg1 = AES_BLOCK_LOAD(src + 16);
    aegis128l_update(state, msg0, msg1);
}

static void
aegis128l_enc(unsigned char *const dst, const unsigned char *const src, aes_block_t *const state)
{
    aes_block_t msg0, msg1;
    aes_block_t tmp0, tmp1;

    msg0 = AES_BLOCK_LOAD(src);
    msg1 = AES_BLOCK_LOAD(src + 16);
    tmp0 = AES_BLOCK_XOR(msg0, state[6]);
    tmp0 = AES_BLOCK_XOR(tmp0, state[1]);
    tmp1 = AES_BLOCK_XOR(msg1, state[5]);
    tmp1 = AES_BLOCK_XOR(tmp1, state[2]);
    tmp0 = AES_BLOCK_XOR(tmp0, AES_BLOCK_AND(state[2], state[3]));
    tmp1 = AES_BLOCK_XOR(tmp1, AES_BLOCK_AND(state[6], state[7]));
    AES_BLOCK_STORE(dst, tmp0);
    AES_BLOCK_STORE(dst + 16, tmp1);

    aegis128l_update(state, msg0, msg1);
}

static void
aegis128l_dec(unsigned char *const dst, const unsigned char *const src, aes_block_t *const state)
{
    aes_block_t msg0, msg1;

    msg0 = AES_BLOCK_LOAD(src);
    msg1 = AES_BLOCK_LOAD(src + 16);
    msg0 = AES_BLOCK_XOR(msg0, state[6]);
    msg0 = AES_BLOCK_XOR(msg0, state[1]);
    msg1 = AES_BLOCK_XOR(msg1, state[5]);
    msg1 = AES_BLOCK_XOR(msg1, state[2]);
    msg0 = AES_BLOCK_XOR(msg0, AES_BLOCK_AND(state[2], state[3]));
    msg1 = AES_BLOCK_XOR(msg1, AES_BLOCK_AND(state[6], state[7]));
    AES_BLOCK_STORE(dst, msg0);
    AES_BLOCK_STORE(dst + 16, msg1);

    aegis128l_update(state, msg0, msg1);
}

static int
aegis128l_encrypt_detached(unsigned char *c, unsigned char *mac, unsigned long long *maclen_p,
                           const unsigned char *m, unsigned long long mlen, const unsigned char *ad,
                           unsigned long long adlen, const unsigned char *nsec,
                           const unsigned char *npub, const unsigned char *k)
{
    aes_block_t                    state[8];
    CRYPTO_ALIGN(16) unsigned char src[32];
    CRYPTO_ALIGN(16) unsigned char dst[32];
    unsigned long long             i;

    (void) nsec;
    aegis128l_init(k, npub, state);

    for (i = 0ULL; i + 32ULL <= adlen; i += 32ULL) {
        aegis128l_absorb(ad + i, state);
    }
    if (adlen & 0x1f) {
        memset(src, 0, 32);
        memcpy(src, ad + i, adlen & 0x1f);
        aegis128l_absorb(src, state);
    }
    for (i = 0ULL; i + 32ULL <= mlen; i += 32ULL) {
        aegis128l_enc(c + i, m + i, state);
    }
    if (mlen & 0x1f) {
        memset(src, 0, 32);
        memcpy(src, m + i, mlen & 0x1f);
        aegis128l_enc(dst, src, state);
        memcpy(c + i, dst, mlen & 0x1f);
    }

    aegis128l_mac(mac, adlen, mlen, state);
    sodium_memzero(state, sizeof state);
    sodium_memzero(src, sizeof src);
    sodium_memzero(dst, sizeof dst);

    if (maclen_p != NULL) {
        *maclen_p = 16ULL;
    }
    return 0;
}

static int
aegis128l_decrypt_detached(unsigned char *m, unsigned char *nsec, const unsigned char *c,
                           unsigned long long clen, const unsigned char *mac,
                           const unsigned char *ad, unsigned long long adlen,
                           const unsigned char *npub, const unsigned char *k)
{
    aes_block_t                    state[8];
    CRYPTO_ALIGN(16) unsigned char src[32];
    CRYPTO_ALIGN(16) unsigned char dst[32];
    CRYPTO_ALIGN(16) unsigned char computed_mac[16];
    unsigned long long             i;
    unsigned long long             mlen;
    int                            ret;

    (void) nsec;
    mlen = clen;
    aegis128l_init(k, npub, state);

    for (i = 0ULL; i + 32ULL <= adlen; i += 32ULL) {
        aegis128l_absorb(ad + i, state);
    }
    if (adlen & 0x1f) {
        memset(src, 0, 32);
        memcpy(src, ad + i, adlen & 0x1f);
        aegis128l_absorb(src, state);
    }
    if (m != NULL) {
        for (i = 0ULL; i + 32ULL <= mlen; i += 32ULL) {
            aegis128l_dec(m + i, c + i, state);
        }
    } else {
        for (i = 0ULL; i + 32ULL <= mlen; i += 32ULL) {
            aegis128l_dec(dst, c + i, state);
        }
    }
    if (mlen & 0x1f) {
        memset(src, 0, 32);
        memcpy(src, c + i, mlen & 0x1f);
        aegis128l_dec(dst, src, state);
        if (m != NULL) {
            memcpy(m + i, dst, mlen & 0x1f);
        }
        memset(dst, 0, mlen & 0x1f);
        state[0] = AES_BLOCK_XOR(state[0], AES_BLOCK_LOAD(dst));
        state[4] = AES_BLOCK_XOR(state[4], AES_BLOCK_LOAD(dst + 16));
    }

    aegis128l_mac(computed_mac, adlen, mlen, state);
    sodium_memzero(state, sizeof state);
    sodium_memzero(src, sizeof src);
    sodium_memzero(dst, sizeof dst);
    ret = crypto_verify_16(computed_mac, mac);
    sodium_memzero(computed_mac, sizeof computed_mac);
    if (m == NULL) {
        return ret;
    }
    if (ret != 0) {
        memset(m, 0, mlen);
        return -1;
    }
    return 0;
}

static int aegis128l_init_state(
    const unsigned char* key,
    const unsigned char* nonce,
    fizz_aegis_evp_ctx *ctx) {
  STATE ctx->aegis128l.soft_state;
  aegis128l_init(key, nonce, state);
  ctx->aegis128l.buffer_size = 0;
  return 0;
}

static int aegis128l_aad_update(
    const unsigned char* ad,
    unsigned long long adlen,
    fizz_aegis_evp_ctx *ctx) {
  unsigned long long i;
  unsigned int buffer_size = ctx->aegis128l.buffer_size;
  unsigned int copy_size = 0;
  STATE ctx->aegis128l.soft_state;

  // If buffer has existing bytes, copy aad to buffer and update state if
  // buffer is full
  if (buffer_size > 0 && buffer_size < STATE_SIZE) {
    unsigned int rem_buffer_size = STATE_SIZE - buffer_size;
    copy_size = adlen <= rem_buffer_size ? adlen : rem_buffer_size;
    memcpy(ctx->aegis128l.buffer + buffer_size, ad, copy_size);
    buffer_size += copy_size;
    if (buffer_size == STATE_SIZE) {
      aegis128l_absorb(
          ctx->aegis128l.buffer, state);
      buffer_size = 0;
    }
  }

  // absorb full blocks (STATE_SIZE) worth of bytes of aad
  for (i = copy_size; i + STATE_SIZE <= adlen; i += STATE_SIZE) {
    aegis128l_absorb(ad + i, state);
  }

  // Copy remaining bytes from ad to buffer
  unsigned int leftover = (adlen - copy_size) & 0x1f;
  if (leftover) {
    memcpy(ctx->aegis128l.buffer, ad + i, leftover);
    buffer_size = leftover;
  }
  ctx->aegis128l.buffer_size = buffer_size;
  return 0;
}

static int aegis128l_aad_final(
    fizz_aegis_evp_ctx *ctx) {
  if (ctx->aegis128l.buffer_size > 0) {
    STATE ctx->aegis128l.soft_state;
    CRYPTO_ALIGN(16) unsigned char src[STATE_SIZE];
    memset(src, 0, STATE_SIZE);
    memcpy(src, ctx->aegis128l.buffer, ctx->aegis128l.buffer_size);
    aegis128l_absorb(src, state);
    sodium_memzero(src, sizeof src);
    ctx->aegis128l.buffer_size = 0;
  }
  return 0;
}

static int aegis128l_encrypt_update(
    unsigned char* c,
    unsigned long long* outlen,
    const unsigned char* m,
    unsigned long long mlen,
    fizz_aegis_evp_ctx *ctx) {
  unsigned long long i;
  unsigned long long writtenlen = 0;
  unsigned int buffer_size = ctx->aegis128l.buffer_size;
  unsigned int copy_size = 0;
  STATE ctx->aegis128l.soft_state;

  // If buffer has existing bytes, copy m to buffer and update state if
  // buffer is full
  if (buffer_size > 0 && buffer_size < STATE_SIZE) {
    unsigned int rem_buffer_size = STATE_SIZE - buffer_size;
    copy_size = mlen <= rem_buffer_size ? mlen : rem_buffer_size;
    memcpy(ctx->aegis128l.buffer + buffer_size, m, copy_size);
    buffer_size += copy_size;
    if (buffer_size == STATE_SIZE) {
      aegis128l_enc(c, ctx->aegis128l.buffer, state);
      buffer_size = 0;
      writtenlen += STATE_SIZE;
      memset(ctx->aegis128l.buffer, 0, STATE_SIZE);
    }
  }

  // encrypt full blocks (STATE_SIZE) worth of bytes
  for (i = copy_size; i + STATE_SIZE <= mlen; i += STATE_SIZE) {
    aegis128l_enc(c + writtenlen, m + i, state);
    writtenlen += STATE_SIZE;
  }

  // Copy remaining bytes from m to buffer
  unsigned int leftover = (mlen - copy_size) & 0x1f;
  if (leftover) {
    memcpy(ctx->aegis128l.buffer, m + i, leftover);
    buffer_size  = leftover;
  }

  if (outlen != NULL) {
    *outlen = writtenlen;
  }
  ctx->aegis128l.buffer_size = buffer_size;
  return 0;
}

static int aegis128l_encrypt_final(
    unsigned char* c,
    unsigned long long* outlen,
    unsigned long long mlen,
    unsigned long long adlen,
    fizz_aegis_evp_ctx *ctx) {
  unsigned int buffer_size = ctx->aegis128l.buffer_size;
  STATE ctx->aegis128l.soft_state;

  if (buffer_size > 0) {
    CRYPTO_ALIGN(16) unsigned char src[STATE_SIZE];
    CRYPTO_ALIGN(16) unsigned char dst[STATE_SIZE];
    memset(src, 0, STATE_SIZE);
    memcpy(src, ctx->aegis128l.buffer, buffer_size);
    aegis128l_enc(dst, src, state);
    memcpy(c, dst, buffer_size);
    sodium_memzero(dst, sizeof dst);
    sodium_memzero(src, sizeof src);
    sodium_memzero(ctx->aegis128l.buffer, sizeof ctx->aegis128l.buffer);
    ctx->aegis128l.buffer_size = 0;
  }

  aegis128l_mac(c + buffer_size, adlen, mlen, state);
  sodium_memzero(state, sizeof state);
  // total final written length is the buffer length plus tag length
  if (outlen != NULL) {
    *outlen = (buffer_size + TAG_LEN);
  }

  return 0;
}

static int aegis128l_decrypt_update(
    unsigned char* m,
    unsigned long long* outlen,
    const unsigned char* c,
    unsigned long long clen,
    fizz_aegis_evp_ctx* ctx) {
  unsigned long long i;
  unsigned long long writtenlen = 0;
  unsigned int buffer_size = ctx->aegis128l.buffer_size;
  unsigned int copy_size = 0;
  STATE ctx->aegis128l.soft_state;

  // If buffer has existing bytes, copy m to buffer and update state if
  // buffer is full
  if (buffer_size > 0 && buffer_size < STATE_SIZE) {
    unsigned int rem_buffer_size = STATE_SIZE - buffer_size;
    copy_size = clen <= rem_buffer_size ? clen : rem_buffer_size;
    memcpy(ctx->aegis128l.buffer + buffer_size, c, copy_size);
    buffer_size += copy_size;
    if (buffer_size == STATE_SIZE) {
      aegis128l_dec(m, ctx->aegis128l.buffer, state);
      buffer_size = 0;
      writtenlen += STATE_SIZE;
      memset(ctx->aegis128l.buffer, 0, STATE_SIZE);
    }
  }

  // decrypt full blocks (STATE_SIZE) worth of bytes
  for (i = copy_size; i + STATE_SIZE <= clen; i += STATE_SIZE) {
    aegis128l_dec(m + writtenlen, c + i, state);
    writtenlen += STATE_SIZE;
  }

  // Copy remaining bytes from c to buffer
  unsigned int leftover = (clen - copy_size) & 0x1f;
  if (leftover) {
    memcpy(ctx->aegis128l.buffer, c + i, leftover);
    buffer_size  = leftover;
  }

  if (outlen != NULL) {
    *outlen = writtenlen;
  }
  ctx->aegis128l.buffer_size = buffer_size;
  return 0;
}

static int aegis128l_decrypt_final(
    unsigned char* m,
    unsigned long long* outlen,
    unsigned long long mlen,
    unsigned long long adlen,
    const unsigned char* mac,
    fizz_aegis_evp_ctx* ctx) {
  unsigned int buffer_size = ctx->aegis128l.buffer_size;
  CRYPTO_ALIGN(16) unsigned char computed_mac[16];
  STATE ctx->aegis128l.soft_state;
  int ret;

  if (buffer_size > 0) {
    CRYPTO_ALIGN(16) unsigned char src[STATE_SIZE];
    CRYPTO_ALIGN(16) unsigned char dst[STATE_SIZE];
    memset(src, 0, STATE_SIZE);
    memcpy(src, ctx->aegis128l.buffer, buffer_size);
    aegis128l_dec(dst, src, state);
    memcpy(m, dst, buffer_size);
    memset(dst, 0, buffer_size);
    state[0] = AES_BLOCK_XOR(state[0], AES_BLOCK_LOAD(dst));
    state[4] = AES_BLOCK_XOR(state[4], AES_BLOCK_LOAD(dst + 16));
    sodium_memzero(dst, sizeof dst);
    sodium_memzero(src, sizeof src);
    sodium_memzero(ctx->aegis128l.buffer, sizeof ctx->aegis128l.buffer);
    ctx->aegis128l.buffer_size = 0;
  }

  if (outlen != NULL) {
    *outlen = buffer_size;
  }

  aegis128l_mac(computed_mac, adlen, mlen, state);
  ret = crypto_verify_16(computed_mac, mac);
  sodium_memzero(computed_mac, sizeof computed_mac);
  sodium_memzero(state, sizeof state);
  return ret;
}

struct crypto_aead_aegis128l_implementation fizz_crypto_aead_aegis128l_soft_implementation = {
    SODIUM_C99(.encrypt_detached =) aegis128l_encrypt_detached,
    SODIUM_C99(.decrypt_detached =) aegis128l_decrypt_detached
};

struct aegis128l_evp aegis128l_soft_evp = {
    SODIUM_C99(.init_state =) aegis128l_init_state,
    SODIUM_C99(.aad_update =) aegis128l_aad_update,
    SODIUM_C99(.aad_final =) aegis128l_aad_final,
    SODIUM_C99(.encrypt_update =) aegis128l_encrypt_update,
    SODIUM_C99(.encrypt_final =) aegis128l_encrypt_final,
    SODIUM_C99(.decrypt_update =) aegis128l_decrypt_update,
    SODIUM_C99(.decrypt_final =) aegis128l_decrypt_final,
};
