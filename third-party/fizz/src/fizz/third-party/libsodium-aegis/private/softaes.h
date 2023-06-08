#ifndef fizz_softaes_H
#define fizz_softaes_H 1

#include <stdint.h>

#include "common.h"

typedef struct SoftAesBlock {
    uint32_t w0;
    uint32_t w1;
    uint32_t w2;
    uint32_t w3;
} SoftAesBlock;

SoftAesBlock softaes_block_encrypt(const SoftAesBlock block, const SoftAesBlock rk);

static inline SoftAesBlock
softaes_block_load(const uint8_t in[16])
{
    const SoftAesBlock out = { LOAD32_LE(in + 0), LOAD32_LE(in + 4), LOAD32_LE(in + 8),
                               LOAD32_LE(in + 12) };
    return out;
}

static inline SoftAesBlock
softaes_block_load64x2(const uint64_t a, const uint64_t b)
{
    const SoftAesBlock out = { (uint32_t) b, (uint32_t) (b >> 32), (uint32_t) a,
                               (uint32_t) (a >> 32) };
    return out;
}

static inline void
softaes_block_store(uint8_t out[16], const SoftAesBlock in)
{
    STORE32_LE(out + 0, in.w0);
    STORE32_LE(out + 4, in.w1);
    STORE32_LE(out + 8, in.w2);
    STORE32_LE(out + 12, in.w3);
}

static inline SoftAesBlock
softaes_block_xor(const SoftAesBlock a, const SoftAesBlock b)
{
    const SoftAesBlock out = { a.w0 ^ b.w0, a.w1 ^ b.w1, a.w2 ^ b.w2, a.w3 ^ b.w3 };
    return out;
}

static inline SoftAesBlock
softaes_block_and(const SoftAesBlock a, const SoftAesBlock b)
{
    const SoftAesBlock out = { a.w0 & b.w0, a.w1 & b.w1, a.w2 & b.w2, a.w3 & b.w3 };
    return out;
}

#endif
