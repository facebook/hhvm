#ifndef fizz_aegis_evp_ctx_H
#define fizz_aegis_evp_ctx_H

#include "fizz/third-party/libsodium-aegis/private/config.h"

#if FIZZ_LIBSODIUM_HAS_AESNI
#include <wmmintrin.h>
#endif

typedef unsigned char OpaqueSoftAesState[16];
typedef struct aegis128l_state {
  unsigned char buffer[32];
  unsigned int buffer_size;
  union {
    OpaqueSoftAesState soft_state[8];
#if FIZZ_LIBSODIUM_HAS_AESNI
    __m128i aesni_state[8];
#endif
  };
} aegis128l_state;

typedef struct aegis256_state {
  unsigned char buffer[16];
  unsigned int buffer_size;
  union {
    OpaqueSoftAesState soft_state[6];
#if FIZZ_LIBSODIUM_HAS_AESNI
    __m128i aesni_state[6];
#endif
  };
} aegis256_state;

typedef struct fizz_aegis_evp_ctx {
  union {
    aegis128l_state aegis128l;
    aegis256_state aegis256;
  };
} fizz_aegis_evp_ctx;

#endif
