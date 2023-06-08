#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "softaes.h"

static const uint32_t LUT[256] = {
    0xa56363c6, 0x847c7cf8, 0x997777ee, 0x8d7b7bf6, 0x0df2f2ff, 0xbd6b6bd6, 0xb16f6fde, 0x54c5c591,
    0x50303060, 0x03010102, 0xa96767ce, 0x7d2b2b56, 0x19fefee7, 0x62d7d7b5, 0xe6abab4d, 0x9a7676ec,
    0x45caca8f, 0x9d82821f, 0x40c9c989, 0x877d7dfa, 0x15fafaef, 0xeb5959b2, 0xc947478e, 0x0bf0f0fb,
    0xecadad41, 0x67d4d4b3, 0xfda2a25f, 0xeaafaf45, 0xbf9c9c23, 0xf7a4a453, 0x967272e4, 0x5bc0c09b,
    0xc2b7b775, 0x1cfdfde1, 0xae93933d, 0x6a26264c, 0x5a36366c, 0x413f3f7e, 0x02f7f7f5, 0x4fcccc83,
    0x5c343468, 0xf4a5a551, 0x34e5e5d1, 0x08f1f1f9, 0x937171e2, 0x73d8d8ab, 0x53313162, 0x3f15152a,
    0x0c040408, 0x52c7c795, 0x65232346, 0x5ec3c39d, 0x28181830, 0xa1969637, 0x0f05050a, 0xb59a9a2f,
    0x0907070e, 0x36121224, 0x9b80801b, 0x3de2e2df, 0x26ebebcd, 0x6927274e, 0xcdb2b27f, 0x9f7575ea,
    0x1b090912, 0x9e83831d, 0x742c2c58, 0x2e1a1a34, 0x2d1b1b36, 0xb26e6edc, 0xee5a5ab4, 0xfba0a05b,
    0xf65252a4, 0x4d3b3b76, 0x61d6d6b7, 0xceb3b37d, 0x7b292952, 0x3ee3e3dd, 0x712f2f5e, 0x97848413,
    0xf55353a6, 0x68d1d1b9, 0x00000000, 0x2cededc1, 0x60202040, 0x1ffcfce3, 0xc8b1b179, 0xed5b5bb6,
    0xbe6a6ad4, 0x46cbcb8d, 0xd9bebe67, 0x4b393972, 0xde4a4a94, 0xd44c4c98, 0xe85858b0, 0x4acfcf85,
    0x6bd0d0bb, 0x2aefefc5, 0xe5aaaa4f, 0x16fbfbed, 0xc5434386, 0xd74d4d9a, 0x55333366, 0x94858511,
    0xcf45458a, 0x10f9f9e9, 0x06020204, 0x817f7ffe, 0xf05050a0, 0x443c3c78, 0xba9f9f25, 0xe3a8a84b,
    0xf35151a2, 0xfea3a35d, 0xc0404080, 0x8a8f8f05, 0xad92923f, 0xbc9d9d21, 0x48383870, 0x04f5f5f1,
    0xdfbcbc63, 0xc1b6b677, 0x75dadaaf, 0x63212142, 0x30101020, 0x1affffe5, 0x0ef3f3fd, 0x6dd2d2bf,
    0x4ccdcd81, 0x140c0c18, 0x35131326, 0x2fececc3, 0xe15f5fbe, 0xa2979735, 0xcc444488, 0x3917172e,
    0x57c4c493, 0xf2a7a755, 0x827e7efc, 0x473d3d7a, 0xac6464c8, 0xe75d5dba, 0x2b191932, 0x957373e6,
    0xa06060c0, 0x98818119, 0xd14f4f9e, 0x7fdcdca3, 0x66222244, 0x7e2a2a54, 0xab90903b, 0x8388880b,
    0xca46468c, 0x29eeeec7, 0xd3b8b86b, 0x3c141428, 0x79dedea7, 0xe25e5ebc, 0x1d0b0b16, 0x76dbdbad,
    0x3be0e0db, 0x56323264, 0x4e3a3a74, 0x1e0a0a14, 0xdb494992, 0x0a06060c, 0x6c242448, 0xe45c5cb8,
    0x5dc2c29f, 0x6ed3d3bd, 0xefacac43, 0xa66262c4, 0xa8919139, 0xa4959531, 0x37e4e4d3, 0x8b7979f2,
    0x32e7e7d5, 0x43c8c88b, 0x5937376e, 0xb76d6dda, 0x8c8d8d01, 0x64d5d5b1, 0xd24e4e9c, 0xe0a9a949,
    0xb46c6cd8, 0xfa5656ac, 0x07f4f4f3, 0x25eaeacf, 0xaf6565ca, 0x8e7a7af4, 0xe9aeae47, 0x18080810,
    0xd5baba6f, 0x887878f0, 0x6f25254a, 0x722e2e5c, 0x241c1c38, 0xf1a6a657, 0xc7b4b473, 0x51c6c697,
    0x23e8e8cb, 0x7cdddda1, 0x9c7474e8, 0x211f1f3e, 0xdd4b4b96, 0xdcbdbd61, 0x868b8b0d, 0x858a8a0f,
    0x907070e0, 0x423e3e7c, 0xc4b5b571, 0xaa6666cc, 0xd8484890, 0x05030306, 0x01f6f6f7, 0x120e0e1c,
    0xa36161c2, 0x5f35356a, 0xf95757ae, 0xd0b9b969, 0x91868617, 0x58c1c199, 0x271d1d3a, 0xb99e9e27,
    0x38e1e1d9, 0x13f8f8eb, 0xb398982b, 0x33111122, 0xbb6969d2, 0x70d9d9a9, 0x898e8e07, 0xa7949433,
    0xb69b9b2d, 0x221e1e3c, 0x92878715, 0x20e9e9c9, 0x49cece87, 0xff5555aa, 0x78282850, 0x7adfdfa5,
    0x8f8c8c03, 0xf8a1a159, 0x80898909, 0x170d0d1a, 0xdabfbf65, 0x31e6e6d7, 0xc6424284, 0xb86868d0,
    0xc3414182, 0xb0999929, 0x772d2d5a, 0x110f0f1e, 0xcbb0b07b, 0xfc5454a8, 0xd6bbbb6d, 0x3a16162c
};

#ifndef SOFTAES_STRIDE
#define SOFTAES_STRIDE 16
#endif

static SoftAesBlock
_encrypt(const uint8_t ix0[4], const uint8_t ix1[4], const uint8_t ix2[4], const uint8_t ix3[4])
{
    CRYPTO_ALIGN(64) uint32_t     t[4][4][256 / SOFTAES_STRIDE];
    CRYPTO_ALIGN(64) uint8_t      of[4][4];
    CRYPTO_ALIGN(64) SoftAesBlock out;
    size_t                        i;
    size_t                        j;

    for (j = 0; j < 4; j++) {
        of[j][0] = ix0[j] % SOFTAES_STRIDE;
        of[j][1] = ix1[j] % SOFTAES_STRIDE;
        of[j][2] = ix2[j] % SOFTAES_STRIDE;
        of[j][3] = ix3[j] % SOFTAES_STRIDE;
    }
    for (i = 0; i < 256 / SOFTAES_STRIDE; i++) {
        for (j = 0; j < 4; j++) {
            t[j][0][i] = LUT[(i * SOFTAES_STRIDE) | of[j][0]];
            t[j][1][i] = LUT[(i * SOFTAES_STRIDE) | of[j][1]];
            t[j][2][i] = LUT[(i * SOFTAES_STRIDE) | of[j][2]];
            t[j][3][i] = LUT[(i * SOFTAES_STRIDE) | of[j][3]];
        }
    }

#ifdef HAVE_INLINE_ASM
    __asm__ __volatile__("" : : "r"(t) : "memory");
#endif

    out.w0 = t[0][0][ix0[0] / SOFTAES_STRIDE];
    out.w0 ^= ROTL32(t[0][1][ix1[0] / SOFTAES_STRIDE], 8);
    out.w0 ^= ROTL32(t[0][2][ix2[0] / SOFTAES_STRIDE], 16);
    out.w0 ^= ROTL32(t[0][3][ix3[0] / SOFTAES_STRIDE], 24);

    out.w1 = t[1][0][ix0[1] / SOFTAES_STRIDE];
    out.w1 ^= ROTL32(t[1][1][ix1[1] / SOFTAES_STRIDE], 8);
    out.w1 ^= ROTL32(t[1][2][ix2[1] / SOFTAES_STRIDE], 16);
    out.w1 ^= ROTL32(t[1][3][ix3[1] / SOFTAES_STRIDE], 24);

    out.w2 = t[2][0][ix0[2] / SOFTAES_STRIDE];
    out.w2 ^= ROTL32(t[2][1][ix1[2] / SOFTAES_STRIDE], 8);
    out.w2 ^= ROTL32(t[2][2][ix2[2] / SOFTAES_STRIDE], 16);
    out.w2 ^= ROTL32(t[2][3][ix3[2] / SOFTAES_STRIDE], 24);

    out.w3 = t[3][0][ix0[3] / SOFTAES_STRIDE];
    out.w3 ^= ROTL32(t[3][1][ix1[3] / SOFTAES_STRIDE], 8);
    out.w3 ^= ROTL32(t[3][2][ix2[3] / SOFTAES_STRIDE], 16);
    out.w3 ^= ROTL32(t[3][3][ix3[3] / SOFTAES_STRIDE], 24);

    return out;
}

SoftAesBlock
softaes_block_encrypt(const SoftAesBlock block, const SoftAesBlock rk)
{
    CRYPTO_ALIGN(64) SoftAesBlock out;
    CRYPTO_ALIGN(64) uint8_t      ix0[4], ix1[4], ix2[4], ix3[4];
    const uint32_t                s0 = block.w0;
    const uint32_t                s1 = block.w1;
    const uint32_t                s2 = block.w2;
    const uint32_t                s3 = block.w3;

    ix0[0] = (uint8_t) s0;
    ix0[1] = (uint8_t) s1;
    ix0[2] = (uint8_t) s2;
    ix0[3] = (uint8_t) s3;

    ix1[0] = (uint8_t) (s1 >> 8);
    ix1[1] = (uint8_t) (s2 >> 8);
    ix1[2] = (uint8_t) (s3 >> 8);
    ix1[3] = (uint8_t) (s0 >> 8);

    ix2[0] = (uint8_t) (s2 >> 16);
    ix2[1] = (uint8_t) (s3 >> 16);
    ix2[2] = (uint8_t) (s0 >> 16);
    ix2[3] = (uint8_t) (s1 >> 16);

    ix3[0] = (uint8_t) (s3 >> 24);
    ix3[1] = (uint8_t) (s0 >> 24);
    ix3[2] = (uint8_t) (s1 >> 24);
    ix3[3] = (uint8_t) (s2 >> 24);

    out = _encrypt(ix0, ix1, ix2, ix3);

    out.w0 ^= rk.w0;
    out.w1 ^= rk.w1;
    out.w2 ^= rk.w2;
    out.w3 ^= rk.w3;

    return out;
}
