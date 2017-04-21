/*-
 * Copyright 2009 Colin Percival
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file was originally written by Colin Percival as part of the Tarsnap
 * online backup system.
 */
#include <folly/Portability.h>
#if FOLLY_SSE_PREREQ(2, 0)

#include <sys/types.h>
#include <folly/portability/SysMman.h>
#include <folly/Bits.h>

#include <emmintrin.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "sha256.h"

#include "crypto_scrypt.h"

static void blkcpy(void *, void *, size_t);
static void blkxor(void *, void *, size_t);
static void salsa20_8(__m128i *);
static void blockmix_salsa8(__m128i *, __m128i *, __m128i *, size_t);
static uint64_t integerify(void *, size_t);
static void smix(uint8_t *, size_t, uint64_t, void *, void *);

static void
blkcpy(void * dest, void * src, size_t len)
{
  __m128i * D = (__m128i *) dest;
  __m128i * S = (__m128i *) src;
  size_t L = len / 16;
  size_t i;

  for (i = 0; i < L; i++)
    D[i] = S[i];
}

static void
blkxor(void * dest, void * src, size_t len)
{
  __m128i * D = (__m128i *) dest;
  __m128i * S = (__m128i *) src;
  size_t L = len / 16;
  size_t i;

  for (i = 0; i < L; i++)
    D[i] = _mm_xor_si128(D[i], S[i]);
}

/**
 * salsa20_8(B):
 * Apply the salsa20/8 core to the provided block.
 */
static void
salsa20_8(__m128i B[4])
{
  __m128i X0, X1, X2, X3;
  __m128i T;
  size_t i;

  X0 = B[0];
  X1 = B[1];
  X2 = B[2];
  X3 = B[3];

  for (i = 0; i < 8; i += 2) {
    /* Operate on "columns". */
    T = _mm_add_epi32(X0, X3);
    X1 = _mm_xor_si128(X1, _mm_slli_epi32(T, 7));
    X1 = _mm_xor_si128(X1, _mm_srli_epi32(T, 25));
    T = _mm_add_epi32(X1, X0);
    X2 = _mm_xor_si128(X2, _mm_slli_epi32(T, 9));
    X2 = _mm_xor_si128(X2, _mm_srli_epi32(T, 23));
    T = _mm_add_epi32(X2, X1);
    X3 = _mm_xor_si128(X3, _mm_slli_epi32(T, 13));
    X3 = _mm_xor_si128(X3, _mm_srli_epi32(T, 19));
    T = _mm_add_epi32(X3, X2);
    X0 = _mm_xor_si128(X0, _mm_slli_epi32(T, 18));
    X0 = _mm_xor_si128(X0, _mm_srli_epi32(T, 14));

    /* Rearrange data. */
    X1 = _mm_shuffle_epi32(X1, 0x93);
    X2 = _mm_shuffle_epi32(X2, 0x4E);
    X3 = _mm_shuffle_epi32(X3, 0x39);

    /* Operate on "rows". */
    T = _mm_add_epi32(X0, X1);
    X3 = _mm_xor_si128(X3, _mm_slli_epi32(T, 7));
    X3 = _mm_xor_si128(X3, _mm_srli_epi32(T, 25));
    T = _mm_add_epi32(X3, X0);
    X2 = _mm_xor_si128(X2, _mm_slli_epi32(T, 9));
    X2 = _mm_xor_si128(X2, _mm_srli_epi32(T, 23));
    T = _mm_add_epi32(X2, X3);
    X1 = _mm_xor_si128(X1, _mm_slli_epi32(T, 13));
    X1 = _mm_xor_si128(X1, _mm_srli_epi32(T, 19));
    T = _mm_add_epi32(X1, X2);
    X0 = _mm_xor_si128(X0, _mm_slli_epi32(T, 18));
    X0 = _mm_xor_si128(X0, _mm_srli_epi32(T, 14));

    /* Rearrange data. */
    X1 = _mm_shuffle_epi32(X1, 0x39);
    X2 = _mm_shuffle_epi32(X2, 0x4E);
    X3 = _mm_shuffle_epi32(X3, 0x93);
  }

  B[0] = _mm_add_epi32(B[0], X0);
  B[1] = _mm_add_epi32(B[1], X1);
  B[2] = _mm_add_epi32(B[2], X2);
  B[3] = _mm_add_epi32(B[3], X3);
}

/**
 * blockmix_salsa8(Bin, Bout, X, r):
 * Compute Bout = BlockMix_{salsa20/8, r}(Bin).  The input Bin must be 128r
 * bytes in length; the output Bout must also be the same size.  The
 * temporary space X must be 64 bytes.
 */
static void
blockmix_salsa8(__m128i * Bin, __m128i * Bout, __m128i * X, size_t r)
{
  size_t i;

  /* 1: X <-- B_{2r - 1} */
  blkcpy(X, &Bin[8 * r - 4], 64);

  /* 2: for i = 0 to 2r - 1 do */
  for (i = 0; i < r; i++) {
    /* 3: X <-- H(X \xor B_i) */
    blkxor(X, &Bin[i * 8], 64);
    salsa20_8(X);

    /* 4: Y_i <-- X */
    /* 6: B' <-- (Y_0, Y_2 ... Y_{2r-2}, Y_1, Y_3 ... Y_{2r-1}) */
    blkcpy(&Bout[i * 4], X, 64);

    /* 3: X <-- H(X \xor B_i) */
    blkxor(X, &Bin[i * 8 + 4], 64);
    salsa20_8(X);

    /* 4: Y_i <-- X */
    /* 6: B' <-- (Y_0, Y_2 ... Y_{2r-2}, Y_1, Y_3 ... Y_{2r-1}) */
    blkcpy(&Bout[(r + i) * 4], X, 64);
  }
}

/**
 * integerify(B, r):
 * Return the result of parsing B_{2r-1} as a little-endian integer.
 */
static uint64_t
integerify(void * B, size_t r)
{
  uint32_t * X = (uint32_t *)((uintptr_t)(B) + (2 * r - 1) * 64);

  return (((uint64_t)(X[13]) << 32) + X[0]);
}

/**
 * smix(B, r, N, V, XY):
 * Compute B = SMix_r(B, N).  The input B must be 128r bytes in length;
 * the temporary storage V must be 128rN bytes in length; the temporary
 * storage XY must be 256r + 64 bytes in length.  The value N must be a
 * power of 2 greater than 1.  The arrays B, V, and XY must be aligned to a
 * multiple of 64 bytes.
 */
static void
smix(uint8_t * B, size_t r, uint64_t N, void * V, void * XY)
{
  __m128i * X = (__m128i *) XY;
  __m128i * Y = (__m128i *) ((uintptr_t)(XY) + 128 * r);
  __m128i * Z = (__m128i *) ((uintptr_t)(XY) + 256 * r);
  uint32_t * X32 = (uint32_t *)X;
  uint64_t i, j;
  size_t k;

  /* 1: X <-- B */
  for (k = 0; k < 2 * r; k++) {
    for (i = 0; i < 16; i++) {
      X32[k * 16 + i] = folly::Endian::little32(
                          *((uint32_t *) &B[(k * 16 + (i * 5 % 16)) * 4]));
    }
  }

  /* 2: for i = 0 to N - 1 do */
  for (i = 0; i < N; i += 2) {
    /* 3: V_i <-- X */
    blkcpy((void *)((uintptr_t)(V) + i * 128 * r), X, 128 * r);

    /* 4: X <-- H(X) */
    blockmix_salsa8(X, Y, Z, r);

    /* 3: V_i <-- X */
    blkcpy((void *)((uintptr_t)(V) + (i + 1) * 128 * r),
        Y, 128 * r);

    /* 4: X <-- H(X) */
    blockmix_salsa8(Y, X, Z, r);
  }

  /* 6: for i = 0 to N - 1 do */
  for (i = 0; i < N; i += 2) {
    /* 7: j <-- Integerify(X) mod N */
    j = integerify(X, r) & (N - 1);

    /* 8: X <-- H(X \xor V_j) */
    blkxor(X, (void *)((uintptr_t)(V) + j * 128 * r), 128 * r);
    blockmix_salsa8(X, Y, Z, r);

    /* 7: j <-- Integerify(X) mod N */
    j = integerify(Y, r) & (N - 1);

    /* 8: X <-- H(X \xor V_j) */
    blkxor(Y, (void *)((uintptr_t)(V) + j * 128 * r), 128 * r);
    blockmix_salsa8(Y, X, Z, r);
  }

  /* 10: B' <-- X */
  for (k = 0; k < 2 * r; k++) {
    for (i = 0; i < 16; i++) {
      *((uint32_t *) &B[(k * 16 + (i * 5 % 16)) * 4]) =
          folly::Endian::little32(X32[k * 16 + i]);
    }
  }
}

/**
 * crypto_scrypt(passwd, passwdlen, salt, saltlen, N, r, p, buf, buflen):
 * Compute scrypt(passwd[0 .. passwdlen - 1], salt[0 .. saltlen - 1], N, r,
 * p, buflen) and write the result into buf.  The parameters r, p, and buflen
 * must satisfy r * p < 2^30 and buflen <= (2^32 - 1) * 32.  The parameter N
 * must be a power of 2 greater than 1.
 *
 * Return 0 on success; or -1 on error.
 */
int
crypto_scrypt(const uint8_t * passwd, size_t passwdlen,
    const uint8_t * salt, size_t saltlen, uint64_t N, uint32_t r, uint32_t p,
    uint8_t * buf, size_t buflen)
{
  void * B0, * V0, * XY0;
  uint8_t * B;
  uint32_t * V;
  uint32_t * XY;
  uint32_t i;

  /* Sanity-check parameters. */
  if (buflen > (((uint64_t)(1) << 32) - 1) * 32) {
    errno = EFBIG;
    goto err0;
  }
  if ((uint64_t)(r) * (uint64_t)(p) >= (1 << 30)) {
    errno = EFBIG;
    goto err0;
  }
  if (((N & (N - 1)) != 0) || (N == 0)) {
    errno = EINVAL;
    goto err0;
  }
  if ((r > SIZE_MAX / 128 / p) ||
      (N > SIZE_MAX / 128 / r)) {
    errno = ENOMEM;
    goto err0;
  }

  /* Allocate memory. */
  B0 = folly_ext::aligned_malloc(128 * r * p, 64);
  if (!B0) {
    goto err0;
  }

  XY0 = folly_ext::aligned_malloc(256 * r + 64, 64);
  if (!XY0) {
    goto err1;
  }

  B = (uint8_t *)(B0);
  XY = (uint32_t *)(XY0);

  if ((V0 = mmap(NULL, 128 * r * N, PROT_READ | PROT_WRITE,
#ifdef MAP_NOCORE
      MAP_ANON | MAP_PRIVATE | MAP_NOCORE,
#else
      MAP_ANON | MAP_PRIVATE,
#endif
      -1, 0)) == MAP_FAILED)
    goto err2;
  V = (uint32_t *)(V0);

  /* 1: (B_0 ... B_{p-1}) <-- PBKDF2(P, S, 1, p * MFLen) */
  PBKDF2_SHA256(passwd, passwdlen, salt, saltlen, 1, B, p * 128 * r);

  /* 2: for i = 0 to p - 1 do */
  for (i = 0; i < p; i++) {
    /* 3: B_i <-- MF(B_i, N) */
    smix(&B[i * 128 * r], r, N, V, XY);
  }
  /* 5: DK <-- PBKDF2(P, B, 1, dkLen) */
  PBKDF2_SHA256(passwd, passwdlen, B, p * 128 * r, 1, buf, buflen);

  /* Free memory. */
  if (munmap(V0, 128 * r * N))
    goto err2;
  folly_ext::aligned_free(XY0);
  folly_ext::aligned_free(B0);

  /* Success! */
  return (0);

err2:
  folly_ext::aligned_free(XY0);
err1:
  folly_ext::aligned_free(B0);
err0:
  /* Failure! */
  return (-1);
}
#endif
