/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/ext/hash/hash_keccak.h"

#include <algorithm>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct KeccakContext {
  uint8_t state[5 * 5 * sizeof(uint64_t)]; // 200 bytes
  uint32_t pos;
};

hash_keccak::hash_keccak(uint32_t capacity,
                         uint32_t digestlen)
  : HashEngine(digestlen, (1600 - capacity) >> 3, sizeof(KeccakContext)) {
  assert(capacity > 0);
  assert(capacity < 1600);
  assert((capacity % 8) == 0);
}

void hash_keccak::hash_init(void *context) {
  memset(context, 0, sizeof(KeccakContext));
}

namespace keccak {
///////////////////////////////////////////////////////////////////////////////

#if (defined(__APPLE__) || defined(__APPLE_CC__)) && \
    (defined(__BIG_ENDIAN__) || defined(__LITTLE_ENDIAN__))
# if defined(__LITTLE_ENDIAN__)
#  undef WORDS_BIGENDIAN
# else
#  if defined(__BIG_ENDIAN__)
#   define WORDS_BIGENDIAN
#  endif
# endif
#endif

inline uint64_t rol64(uint64_t v, uint8_t b) {
  return (v << b) | (v >> (64 - b));
}
inline uint8_t idx(uint8_t x, uint8_t y) {
  return x + (5 * y);
}

#ifdef WORDS_BIGENDIAN
inline uint64_t load64(const uint8_t* x) {
  uint64_t ret = 0;
  for (uint8_t i = 7; i >= 0; --i) {
    ret <<= 8;
    ret |= x[i];
  }
  return ret;
}
inline void store64(const uint8_t* x, uint64_t val) {
  for (uint8_t i = 0; i < 8; ++i) {
    x[i] = val & 0xFF;
    val >>= 8;
  }
}
inline void xor64(const uint8_t* x, uint64_t val) {
  for (uint8_t i = 0; i < 8; ++i) {
    x[i] ^= val & 0xFF;
    val >>= 8;
  }
}
# define readLane(x, y)     load64(ctx->state+sizeof(uint64_t)*idx(x, y))
# define writeLane(x, y, v) store64(ctx->state+sizeof(uint64_t)*idx(x, y), v)
# define XORLane(x, y, v)   xor64(ctx->state+sizeof(uint64_t)*idx(x, y), v)
#else
# define readLane(x, y)     (((uint64_t*)ctx->state)[idx(x,y)])
# define writeLane(x, y, v) (((uint64_t*)ctx->state)[idx(x,y)] = v)
# define XORLane(x, y, v)   (((uint64_t*)ctx->state)[idx(x,y)] ^= v)
#endif

inline bool LFSR86540(uint8_t& LFSR)
{
    bool result = LFSR & 0x01;
    if (LFSR & 0x80) {
        // Primitive polynomial over GF(2): x^8+x^6+x^5+x^4+1
        LFSR = (LFSR << 1) ^ 0x71;
    } else {
        LFSR <<= 1;
    }
    return result;
}

static void permute(KeccakContext* ctx) {
  uint8_t LFSRstate = 0x01;

  for (uint8_t round = 0; round < 24; ++round) {
    { // Theta step (see [Keccak Reference, Section 2.3.2])
      uint64_t C[5], D;
      for (uint8_t x = 0; x < 5; ++x) {
        C[x] = readLane(x, 0) ^ readLane(x, 1) ^
               readLane(x, 2) ^ readLane(x, 3) ^ readLane(x, 4);
      }
      for (uint8_t x = 0; x < 5; ++x) {
        D = C[(x+4)%5] ^ rol64(C[(x+1)%5], 1);
        for (uint8_t y = 0; y < 5; ++y) {
          XORLane(x, y, D);
        }
      }
    }

    { // p and Pi steps (see [Keccak Reference, Sections 2.3.3 and 2.3.4])
      uint8_t x = 1, y = 0;
      uint64_t current = readLane(x, y);
      for (uint8_t t = 0; t < 24; ++t) {
        uint8_t r = ((t + 1) * (t + 2) / 2) % 64;
        uint8_t Y = (2*x + 3*y) % 5;
        x = y; y = Y;
        uint64_t temp = readLane(x, y);
        writeLane(x, y, rol64(current, r));
        current = temp;
      }
    }

    { // X step (see [Keccak Reference, Section 2.3.1])
      for (uint8_t y = 0; y < 5; ++y) {
        uint64_t temp[5];
        for (uint8_t x = 0; x < 5; ++x) {
          temp[x] = readLane(x, y);
        }
        for (uint8_t x = 0; x < 5; ++x) {
          writeLane(x, y, temp[x] ^((~temp[(x+1)%5]) & temp[(x+2)%5]));
        }
      }
    }

    { // i step (see [Keccak Reference, Section 2.3.5])
      for (uint8_t j = 0; j < 7; ++j) {
        if (LFSR86540(LFSRstate)) {
          uint64_t bitPos = (1<<j) - 1;
          XORLane(0, 0, (uint64_t)1 << bitPos);
        }
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
} // namespace keccak

void hash_keccak::hash_update(void *context, const unsigned char *buf,
                              unsigned int count) {
  auto ctx = (KeccakContext*)context;
  while (count > 0) {
    auto len = std::min(block_size - ctx->pos, count);
    count -= len;
    while (len-- > 0) {
      assert(ctx->pos < block_size);
      ctx->state[ctx->pos++] ^= *(buf++);
    }
    if (ctx->pos >= block_size) {
      keccak::permute(ctx);
      ctx->pos = 0;
    }
  }
}

void hash_keccak::hash_final(unsigned char *digest, void *context) {
  auto ctx = (KeccakContext*)context;

  // Pad state to finalize
  ctx->state[ctx->pos++] ^= SUFFIX;
  if ((SUFFIX & 0x80) && (ctx->pos >= block_size)) {
    keccak::permute(ctx);
  }
  ctx->state[block_size-1] ^= 0x80;
  keccak::permute(ctx);

  // Square output for digest
  auto len = digest_size;
  for(;;) {
    auto bs = std::min(len, block_size);
    memcpy(digest, ctx->state, bs);
    digest += bs;
    len -= bs;
    if (!len) break;
    keccak::permute(ctx);
  }

  // Zero out context
  memset(ctx, 0, sizeof(KeccakContext));
}

///////////////////////////////////////////////////////////////////////////////
}
