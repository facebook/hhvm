/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

/* Heavily borrowed from md5.c & sha1.c of PHP archival fame
   Note that ripemd laughs in the face of logic and uses
   little endian byte ordering */

#include "hphp/runtime/ext/hash/hash_ripemd.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct PhpRipeMd128Ctx {
  unsigned int state[4];          /* state (ABCD) */
  unsigned int count[2];          /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];       /* input buffer */
};

struct PhpRipeMd160Ctx {
  unsigned int state[5];          /* state (ABCD) */
  unsigned int count[2];          /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];       /* input buffer */
};

struct PhpRipeMd256Ctx {
  unsigned int state[8];          /* state (ABCD) */
  unsigned int count[2];          /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];       /* input buffer */
};

struct PhpRipeMd320Ctx {
  unsigned int state[10];         /* state (ABCD) */
  unsigned int count[2];          /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];       /* input buffer */
};

hash_ripemd128::hash_ripemd128()
  : HashEngine(16, 64, sizeof(PhpRipeMd128Ctx)) {
}

hash_ripemd160::hash_ripemd160()
  : HashEngine(20, 64, sizeof(PhpRipeMd160Ctx)) {
}

hash_ripemd256::hash_ripemd256()
  : HashEngine(32, 64, sizeof(PhpRipeMd256Ctx)) {
}

hash_ripemd320::hash_ripemd320()
  : HashEngine(40, 64, sizeof(PhpRipeMd320Ctx)) {
}

/*
 * ripemd128 initialization. Begins a ripemd128 operation, writing
 * a new context.
 */
void hash_ripemd128::hash_init(void *context_) {
  PhpRipeMd128Ctx * context = (PhpRipeMd128Ctx*)context_;
  context->count[0] = context->count[1] = 0;
  /* Load magic initialization constants.
   */
  context->state[0] = 0x67452301;
  context->state[1] = 0xEFCDAB89;
  context->state[2] = 0x98BADCFE;
  context->state[3] = 0x10325476;
}

/*
 * ripemd256 initialization. Begins a ripemd256 operation,
 * writing a new context.
 */
void hash_ripemd256::hash_init(void *context_) {
  PhpRipeMd256Ctx * context = (PhpRipeMd256Ctx*)context_;
  context->count[0] = context->count[1] = 0;
  /* Load magic initialization constants.
   */
  context->state[0] = 0x67452301;
  context->state[1] = 0xEFCDAB89;
  context->state[2] = 0x98BADCFE;
  context->state[3] = 0x10325476;
  context->state[4] = 0x76543210;
  context->state[5] = 0xFEDCBA98;
  context->state[6] = 0x89ABCDEF;
  context->state[7] = 0x01234567;
}

/*
 * ripemd160 initialization. Begins a ripemd160 operation,
 * writing a new context.
 */
void hash_ripemd160::hash_init(void *context_) {
  PhpRipeMd160Ctx * context = (PhpRipeMd160Ctx*)context_;
  context->count[0] = context->count[1] = 0;
  /* Load magic initialization constants.
   */
  context->state[0] = 0x67452301;
  context->state[1] = 0xEFCDAB89;
  context->state[2] = 0x98BADCFE;
  context->state[3] = 0x10325476;
  context->state[4] = 0xC3D2E1F0;
}

/*
 * ripemd320 initialization. Begins a ripemd320 operation,
 *  writing a new context.
 */
void hash_ripemd320::hash_init(void *context_) {
  PhpRipeMd320Ctx * context = (PhpRipeMd320Ctx*)context_;
  context->count[0] = context->count[1] = 0;
  /* Load magic initialization constants.
   */
  context->state[0] = 0x67452301;
  context->state[1] = 0xEFCDAB89;
  context->state[2] = 0x98BADCFE;
  context->state[3] = 0x10325476;
  context->state[4] = 0xC3D2E1F0;
  context->state[5] = 0x76543210;
  context->state[6] = 0xFEDCBA98;
  context->state[7] = 0x89ABCDEF;
  context->state[8] = 0x01234567;
  context->state[9] = 0x3C2D1E0F;
}

/* Basic ripemd function */
#define F0(x,y,z)		((x) ^ (y) ^ (z))
#define F1(x,y,z)		(((x) & (y)) | ((~(x)) & (z)))
#define F2(x,y,z)		(((x) | (~(y))) ^ (z))
#define F3(x,y,z)		(((x) & (z)) | ((y) & (~(z))))
#define F4(x,y,z)		((x) ^ ((y) | (~(z))))

static const unsigned int K_values[5]  = { 0x00000000, 0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xA953FD4E };    /* 128, 256, 160, 320 */
static const unsigned int KK_values[4] = { 0x50A28BE6, 0x5C4DD124, 0x6D703EF3, 0x00000000 };                /* 128 & 256 */
static const unsigned int KK160_values[5] = { 0x50A28BE6, 0x5C4DD124, 0x6D703EF3, 0x7A6D76E9, 0x00000000 }; /* 160 & 320 */

#define K(n)  K_values[ (n) >> 4]
#define KK(n) KK_values[(n) >> 4]
#define KK160(n) KK160_values[(n) >> 4]

static const unsigned char R[80] = {
  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
  7,  4, 13,  1, 10,  6, 15,  3, 12,  0,  9,  5,  2, 14, 11,  8,
  3, 10, 14,  4,  9, 15,  8,  1,  2,  7,  0,  6, 13, 11,  5, 12,
  1,  9, 11, 10,  0,  8, 12,  4, 13,  3,  7, 15, 14,  5,  6,  2,
  4,  0,  5,  9,  7, 12,  2, 10, 14,  1,  3,  8, 11,  6, 15, 13 };

static const unsigned char RR[80] = {
  5, 14,  7,  0,  9,  2, 11,  4, 13,  6, 15,  8,  1, 10,  3, 12,
  6, 11,  3,  7,  0, 13,  5, 10, 14, 15,  8, 12,  4,  9,  1,  2,
  15,  5,  1,  3,  7, 14,  6,  9, 11,  8, 12,  2, 10,  0,  4, 13,
  8,  6,  4,  1,  3, 11, 15,  0,  5, 12,  2, 13,  9,  7, 10, 14,
  12, 15, 10,  4,  1,  5,  8,  7,  6,  2, 13, 14,  0,  3,  9, 11 };

static const unsigned char S[80] = {
  11, 14, 15, 12,  5,  8,  7,  9, 11, 13, 14, 15,  6,  7,  9,  8,
  7,  6,  8, 13, 11,  9,  7, 15,  7, 12, 15,  9, 11,  7, 13, 12,
  11, 13,  6,  7, 14,  9, 13, 15, 14,  8, 13,  6,  5, 12,  7,  5,
  11, 12, 14, 15, 14, 15,  9,  8,  9, 14,  5,  6,  8,  6,  5, 12,
  9, 15,  5, 11,  6,  8, 13, 12,  5, 12, 13, 14, 11,  8,  5,  6 };

static const unsigned char SS[80] = {
  8,  9,  9, 11, 13, 15, 15,  5,  7,  7,  8, 11, 14, 14, 12,  6,
  9, 13, 15,  7, 12,  8,  9, 11,  7,  7, 12,  7,  6, 15, 13, 11,
  9,  7, 15, 11,  8,  6,  6, 14, 12, 13,  5, 14, 13, 13,  7,  5,
  15,  5,  8, 11, 14, 14,  6, 14,  6,  9, 12,  9, 12,  5, 15,  8,
  8,  5, 12,  9, 12,  5, 14,  6,  8, 13,  6,  5, 15, 13, 11, 11 };

#define ROLS(j, x)	(((x) << S[j])  | ((x) >> (32 - S[j])))
#define ROLSS(j, x)	(((x) << SS[j]) | ((x) >> (32 - SS[j])))
#define ROL(n, x)	(((x) << n) | ((x) >> (32 - n)))

/*
   Decodes input (unsigned char) into output (unsigned int). Assumes len is
   a multiple of 4.
 */
static void RIPEMDDecode(unsigned int *output, const unsigned char *input,
                         unsigned int len) {
  unsigned int i, j;

  for (i = 0, j = 0; j < len; i++, j += 4)
    output[i] = ((unsigned int) input[j + 0]) |
      (((unsigned int) input[j + 1]) << 8) |
      (((unsigned int) input[j + 2]) << 16) |
      (((unsigned int) input[j + 3]) << 24);
}

/*
 * ripemd128 basic transformation. Transforms state based on block.
 */
static void RIPEMD128Transform(unsigned int state[4],
                               const unsigned char block[64]) {
  unsigned int a  = state[0], b  = state[1], c  = state[2], d  = state[3];
  unsigned int aa = state[0], bb = state[1], cc = state[2], dd = state[3];
  unsigned int tmp, x[16];
  int j;

  RIPEMDDecode(x, block, 64);

  for(j = 0; j < 16; j++) {
    tmp = ROLS( j, a  + F0(b,  c,  d)  + x[R[j]]  + K(j));
    a = d; d = c; c = b; b = tmp;
    tmp = ROLSS(j, aa + F3(bb, cc, dd) + x[RR[j]] + KK(j));
    aa = dd; dd = cc; cc = bb; bb = tmp;
  }

  for(j = 16; j < 32; j++) {
    tmp = ROLS( j, a  + F1(b,  c,  d)  + x[R[j]]  + K(j));
    a = d; d = c; c = b; b = tmp;
    tmp = ROLSS(j, aa + F2(bb, cc, dd) + x[RR[j]] + KK(j));
    aa = dd; dd = cc; cc = bb; bb = tmp;
  }

  for(j = 32; j < 48; j++) {
    tmp = ROLS( j, a  + F2(b,  c,  d)  + x[R[j]]  + K(j));
    a = d; d = c; c = b; b = tmp;
    tmp = ROLSS(j, aa + F1(bb, cc, dd) + x[RR[j]] + KK(j));
    aa = dd; dd = cc; cc = bb; bb = tmp;
  }

  for(j = 48; j < 64; j++) {
    tmp = ROLS( j, a  + F3(b,  c,  d)  + x[R[j]]  + K(j));
    a = d; d = c; c = b; b = tmp;
    tmp = ROLSS(j, aa + F0(bb, cc, dd) + x[RR[j]] + KK(j));
    aa = dd; dd = cc; cc = bb; bb = tmp;
  }

  tmp = state[1] + c + dd;
  state[1] = state[2] + d + aa;
  state[2] = state[3] + a + bb;
  state[3] = state[0] + b + cc;
  state[0] = tmp;

  tmp = 0;
  memset(x, 0, sizeof(x));
}

/*
  ripemd128 block update operation. Continues a ripemd128 message-digest
  operation, processing another message block, and updating the
  context.
 */
void hash_ripemd128::hash_update(void *context_, const unsigned char *input,
                                 unsigned int inputLen) {
  PhpRipeMd128Ctx * context = (PhpRipeMd128Ctx*)context_;
  unsigned int i, index, partLen;

  /* Compute number of bytes mod 64 */
  index = (unsigned int) ((context->count[0] >> 3) & 0x3F);

  /* Update number of bits */
  if ((context->count[0] +=
       ((unsigned int) inputLen << 3)) < ((unsigned int) inputLen << 3)) {
    context->count[1]++;
  }
  context->count[1] += ((unsigned int) inputLen >> 29);

  partLen = 64 - index;

  /* Transform as many times as possible.
   */
  if (inputLen >= partLen) {
    memcpy((unsigned char*) & context->buffer[index],
           (unsigned char*) input, partLen);
    RIPEMD128Transform(context->state, context->buffer);

    for (i = partLen; i + 63 < inputLen; i += 64) {
      RIPEMD128Transform(context->state, &input[i]);
    }

    index = 0;
  } else {
    i = 0;
  }

  /* Buffer remaining input */
  memcpy((unsigned char*) & context->buffer[index],
         (unsigned char*) & input[i], inputLen - i);
}

/*
 * ripemd256 basic transformation. Transforms state based on block.
 */
static void RIPEMD256Transform(unsigned int state[8],
                               const unsigned char block[64]) {
  unsigned int a  = state[0], b  = state[1], c  = state[2], d  = state[3];
  unsigned int aa = state[4], bb = state[5], cc = state[6], dd = state[7];
  unsigned int tmp, x[16];
  int j;

  RIPEMDDecode(x, block, 64);

  for(j = 0; j < 16; j++) {
    tmp = ROLS( j, a  + F0(b,  c,  d)  + x[R[j]]  + K(j));
    a = d; d = c; c = b; b = tmp;
    tmp = ROLSS(j, aa + F3(bb, cc, dd) + x[RR[j]] + KK(j));
    aa = dd; dd = cc; cc = bb; bb = tmp;
  }
  tmp = a; a = aa; aa = tmp;

  for(j = 16; j < 32; j++) {
    tmp = ROLS( j, a  + F1(b,  c,  d)  + x[R[j]]  + K(j));
    a = d; d = c; c = b; b = tmp;
    tmp = ROLSS(j, aa + F2(bb, cc, dd) + x[RR[j]] + KK(j));
    aa = dd; dd = cc; cc = bb; bb = tmp;
  }
  tmp = b; b = bb; bb = tmp;

  for(j = 32; j < 48; j++) {
    tmp = ROLS( j, a  + F2(b,  c,  d)  + x[R[j]]  + K(j));
    a = d; d = c; c = b; b = tmp;
    tmp = ROLSS(j, aa + F1(bb, cc, dd) + x[RR[j]] + KK(j));
    aa = dd; dd = cc; cc = bb; bb = tmp;
  }
  tmp = c; c = cc; cc = tmp;

  for(j = 48; j < 64; j++) {
    tmp = ROLS( j, a  + F3(b,  c,  d)  + x[R[j]]  + K(j));
    a = d; d = c; c = b; b = tmp;
    tmp = ROLSS(j, aa + F0(bb, cc, dd) + x[RR[j]] + KK(j));
    aa = dd; dd = cc; cc = bb; bb = tmp;
  }
  tmp = d; d = dd; dd = tmp;

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
  state[4] += aa;
  state[5] += bb;
  state[6] += cc;
  state[7] += dd;

  tmp = 0;
  memset(x, 0, sizeof(x));
}

/*
   ripemd256 block update operation. Continues a ripemd256 message-digest
   operation, processing another message block, and updating the
   context.
 */
void hash_ripemd256::hash_update(void *context_, const unsigned char *input,
                                 unsigned int inputLen) {
  PhpRipeMd256Ctx * context = (PhpRipeMd256Ctx*)context_;
  unsigned int i, index, partLen;

  /* Compute number of bytes mod 64 */
  index = (unsigned int) ((context->count[0] >> 3) & 0x3F);

  /* Update number of bits */
  if ((context->count[0] +=
       ((unsigned int) inputLen << 3)) < ((unsigned int) inputLen << 3)) {
    context->count[1]++;
  }
  context->count[1] += ((unsigned int) inputLen >> 29);

  partLen = 64 - index;

  /* Transform as many times as possible.
   */
  if (inputLen >= partLen) {
    memcpy((unsigned char*) & context->buffer[index],
           (unsigned char*) input, partLen);
    RIPEMD256Transform(context->state, context->buffer);

    for (i = partLen; i + 63 < inputLen; i += 64) {
      RIPEMD256Transform(context->state, &input[i]);
    }

    index = 0;
  } else {
    i = 0;
  }

  /* Buffer remaining input */
  memcpy((unsigned char*) & context->buffer[index],
         (unsigned char*) & input[i], inputLen - i);
}

/*
 * ripemd160 basic transformation. Transforms state based on block.
 */
static void RIPEMD160Transform(unsigned int state[5],
                               const unsigned char block[64]) {
  unsigned int a  = state[0], b  = state[1], c  = state[2],
    d  = state[3], e  = state[4];
  unsigned int aa = state[0], bb = state[1], cc = state[2],
    dd = state[3], ee = state[4];
  unsigned int tmp, x[16];
  int j;

  RIPEMDDecode(x, block, 64);

  for(j = 0; j < 16; j++) {
    tmp = ROLS( j, a  + F0(b,  c,  d)  + x[R[j]]  + K(j)) + e;
    a = e; e = d; d = ROL(10, c); c = b; b = tmp;
    tmp = ROLSS(j, aa + F4(bb, cc, dd) + x[RR[j]] + KK160(j)) + ee;
    aa = ee; ee = dd; dd = ROL(10, cc); cc = bb; bb = tmp;
  }

  for(j = 16; j < 32; j++) {
    tmp = ROLS( j, a  + F1(b,  c,  d)  + x[R[j]]  + K(j)) + e;
    a = e; e = d; d = ROL(10, c); c = b; b = tmp;
    tmp = ROLSS(j, aa + F3(bb, cc, dd) + x[RR[j]] + KK160(j)) + ee;
    aa = ee; ee = dd; dd = ROL(10, cc); cc = bb; bb = tmp;
  }

  for(j = 32; j < 48; j++) {
    tmp = ROLS( j, a  + F2(b,  c,  d)  + x[R[j]]  + K(j)) + e;
    a = e; e = d; d = ROL(10, c); c = b; b = tmp;
    tmp = ROLSS(j, aa + F2(bb, cc, dd) + x[RR[j]] + KK160(j)) + ee;
    aa = ee; ee = dd; dd = ROL(10, cc); cc = bb; bb = tmp;
  }

  for(j = 48; j < 64; j++) {
    tmp = ROLS( j, a  + F3(b,  c,  d)  + x[R[j]]  + K(j)) + e;
    a = e; e = d; d = ROL(10, c); c = b; b = tmp;
    tmp = ROLSS(j, aa + F1(bb, cc, dd) + x[RR[j]] + KK160(j)) + ee;
    aa = ee; ee = dd; dd = ROL(10, cc); cc = bb; bb = tmp;
  }

  for(j = 64; j < 80; j++) {
    tmp = ROLS( j, a  + F4(b,  c,  d)  + x[R[j]]  + K(j)) + e;
    a = e; e = d; d = ROL(10, c); c = b; b = tmp;
    tmp = ROLSS(j, aa + F0(bb, cc, dd) + x[RR[j]] + KK160(j)) + ee;
    aa = ee; ee = dd; dd = ROL(10, cc); cc = bb; bb = tmp;
  }

  tmp = state[1] + c + dd;
  state[1] = state[2] + d + ee;
  state[2] = state[3] + e + aa;
  state[3] = state[4] + a + bb;
  state[4] = state[0] + b + cc;
  state[0] = tmp;

  tmp = 0;
  memset(x, 0, sizeof(x));
}

/*
   ripemd160 block update operation. Continues a ripemd160 message-digest
   operation, processing another message block, and updating the
   context.
 */
void hash_ripemd160::hash_update(void *context_, const unsigned char *input,
                                 unsigned int inputLen) {
  PhpRipeMd160Ctx * context = (PhpRipeMd160Ctx*)context_;
  unsigned int i, index, partLen;

  /* Compute number of bytes mod 64 */
  index = (unsigned int) ((context->count[0] >> 3) & 0x3F);

  /* Update number of bits */
  if ((context->count[0] +=
       ((unsigned int) inputLen << 3)) < ((unsigned int) inputLen << 3)) {
    context->count[1]++;
  }
  context->count[1] += ((unsigned int) inputLen >> 29);

  partLen = 64 - index;

  /* Transform as many times as possible.
   */
  if (inputLen >= partLen) {
    memcpy((unsigned char*) & context->buffer[index],
           (unsigned char*) input, partLen);
    RIPEMD160Transform(context->state, context->buffer);

    for (i = partLen; i + 63 < inputLen; i += 64) {
      RIPEMD160Transform(context->state, &input[i]);
    }

    index = 0;
  } else {
    i = 0;
  }

  /* Buffer remaining input */
  memcpy((unsigned char*) & context->buffer[index],
         (unsigned char*) & input[i], inputLen - i);
}

/*
 * ripemd320 basic transformation. Transforms state based on block.
 */
static void RIPEMD320Transform(unsigned int state[10],
                               const unsigned char block[64]) {
  unsigned int a  = state[0], b  = state[1], c  = state[2],
    d  = state[3], e  = state[4];
  unsigned int aa = state[5], bb = state[6], cc = state[7],
    dd = state[8], ee = state[9];
  unsigned int tmp, x[16];
  int j;

  RIPEMDDecode(x, block, 64);

  for(j = 0; j < 16; j++) {
    tmp = ROLS( j, a  + F0(b,  c,  d)  + x[R[j]]  + K(j)) + e;
    a = e; e = d; d = ROL(10, c); c = b; b = tmp;
    tmp = ROLSS(j, aa + F4(bb, cc, dd) + x[RR[j]] + KK160(j)) + ee;
    aa = ee; ee = dd; dd = ROL(10, cc); cc = bb; bb = tmp;
  }
  tmp = b; b = bb; bb = tmp;

  for(j = 16; j < 32; j++) {
    tmp = ROLS( j, a  + F1(b,  c,  d)  + x[R[j]]  + K(j)) + e;
    a = e; e = d; d = ROL(10, c); c = b; b = tmp;
    tmp = ROLSS(j, aa + F3(bb, cc, dd) + x[RR[j]] + KK160(j)) + ee;
    aa = ee; ee = dd; dd = ROL(10, cc); cc = bb; bb = tmp;
  }
  tmp = d; d = dd; dd = tmp;

  for(j = 32; j < 48; j++) {
    tmp = ROLS( j, a  + F2(b,  c,  d)  + x[R[j]]  + K(j)) + e;
    a = e; e = d; d = ROL(10, c); c = b; b = tmp;
    tmp = ROLSS(j, aa + F2(bb, cc, dd) + x[RR[j]] + KK160(j)) + ee;
    aa = ee; ee = dd; dd = ROL(10, cc); cc = bb; bb = tmp;
  }
  tmp = a; a = aa; aa = tmp;

  for(j = 48; j < 64; j++) {
    tmp = ROLS( j, a  + F3(b,  c,  d)  + x[R[j]]  + K(j)) + e;
    a = e; e = d; d = ROL(10, c); c = b; b = tmp;
    tmp = ROLSS(j, aa + F1(bb, cc, dd) + x[RR[j]] + KK160(j)) + ee;
    aa = ee; ee = dd; dd = ROL(10, cc); cc = bb; bb = tmp;
  }
  tmp = c; c = cc; cc = tmp;

  for(j = 64; j < 80; j++) {
    tmp = ROLS( j, a  + F4(b,  c,  d)  + x[R[j]]  + K(j)) + e;
    a = e; e = d; d = ROL(10, c); c = b; b = tmp;
    tmp = ROLSS(j, aa + F0(bb, cc, dd) + x[RR[j]] + KK160(j)) + ee;
    aa = ee; ee = dd; dd = ROL(10, cc); cc = bb; bb = tmp;
  }
  tmp = e; e = ee; ee = tmp;

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
  state[4] += e;
  state[5] += aa;
  state[6] += bb;
  state[7] += cc;
  state[8] += dd;
  state[9] += ee;

  tmp = 0;
  memset(x, 0, sizeof(x));
}

/*
   ripemd320 block update operation. Continues a ripemd320 message-digest
   operation, processing another message block, and updating the
   context.
 */
void hash_ripemd320::hash_update(void *context_, const unsigned char *input,
                                 unsigned int inputLen) {
  PhpRipeMd320Ctx * context = (PhpRipeMd320Ctx*)context_;
  unsigned int i, index, partLen;

  /* Compute number of bytes mod 64 */
  index = (unsigned int) ((context->count[0] >> 3) & 0x3F);

  /* Update number of bits */
  if ((context->count[0] +=
       ((unsigned int) inputLen << 3)) < ((unsigned int) inputLen << 3)) {
    context->count[1]++;
  }
  context->count[1] += ((unsigned int) inputLen >> 29);

  partLen = 64 - index;

  /* Transform as many times as possible.
   */
  if (inputLen >= partLen) {
    memcpy((unsigned char*) & context->buffer[index],
           (unsigned char*) input, partLen);
    RIPEMD320Transform(context->state, context->buffer);

    for (i = partLen; i + 63 < inputLen; i += 64) {
      RIPEMD320Transform(context->state, &input[i]);
    }

    index = 0;
  } else {
    i = 0;
  }

  /* Buffer remaining input */
  memcpy((unsigned char*) & context->buffer[index],
         (unsigned char*) & input[i], inputLen - i);
}

static const unsigned char PADDING[64] = {
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*
   Encodes input (unsigned int) into output (unsigned char). Assumes len is
   a multiple of 4.
 */
static void RIPEMDEncode(unsigned char *output, unsigned int *input,
                         unsigned int len) {
  unsigned int i, j;

  for (i = 0, j = 0; j < len; i++, j += 4) {
    output[j + 3] = (unsigned char) ((input[i] >> 24) & 0xff);
    output[j + 2] = (unsigned char) ((input[i] >> 16) & 0xff);
    output[j + 1] = (unsigned char) ((input[i] >> 8) & 0xff);
    output[j + 0] = (unsigned char) (input[i] & 0xff);
  }
}

/*
   ripemd128 finalization. Ends a ripemd128 message-digest operation,
   writing the message digest and zeroizing the context.
 */
void hash_ripemd128::hash_final(unsigned char *digest, void *context_) {
  PhpRipeMd128Ctx * context = (PhpRipeMd128Ctx*)context_;
  unsigned char bits[8];
  unsigned int index, padLen;

  /* Save number of bits */
  bits[0] = (unsigned char) (context->count[0] & 0xFF);
  bits[1] = (unsigned char) ((context->count[0] >> 8) & 0xFF);
  bits[2] = (unsigned char) ((context->count[0] >> 16) & 0xFF);
  bits[3] = (unsigned char) ((context->count[0] >> 24) & 0xFF);
  bits[4] = (unsigned char) (context->count[1] & 0xFF);
  bits[5] = (unsigned char) ((context->count[1] >> 8) & 0xFF);
  bits[6] = (unsigned char) ((context->count[1] >> 16) & 0xFF);
  bits[7] = (unsigned char) ((context->count[1] >> 24) & 0xFF);

  /* Pad out to 56 mod 64.
   */
  index = (unsigned int) ((context->count[0] >> 3) & 0x3f);
  padLen = (index < 56) ? (56 - index) : (120 - index);
  hash_update(context, PADDING, padLen);

  /* Append length (before padding) */
  hash_update(context, bits, 8);

  /* Store state in digest */
  RIPEMDEncode(digest, context->state, 16);

  /* Zeroize sensitive information.
   */
  memset((unsigned char*) context, 0, sizeof(*context));
}

/*
   ripemd256 finalization. Ends a ripemd256 message-digest operation,
   writing the message digest and zeroizing the context.
 */
void hash_ripemd256::hash_final(unsigned char *digest, void *context_) {
  PhpRipeMd256Ctx * context = (PhpRipeMd256Ctx*)context_;
  unsigned char bits[8];
  unsigned int index, padLen;

  /* Save number of bits */
  bits[0] = (unsigned char) (context->count[0] & 0xFF);
  bits[1] = (unsigned char) ((context->count[0] >> 8) & 0xFF);
  bits[2] = (unsigned char) ((context->count[0] >> 16) & 0xFF);
  bits[3] = (unsigned char) ((context->count[0] >> 24) & 0xFF);
  bits[4] = (unsigned char) (context->count[1] & 0xFF);
  bits[5] = (unsigned char) ((context->count[1] >> 8) & 0xFF);
  bits[6] = (unsigned char) ((context->count[1] >> 16) & 0xFF);
  bits[7] = (unsigned char) ((context->count[1] >> 24) & 0xFF);

  /* Pad out to 56 mod 64.
   */
  index = (unsigned int) ((context->count[0] >> 3) & 0x3f);
  padLen = (index < 56) ? (56 - index) : (120 - index);
  hash_update(context, PADDING, padLen);

  /* Append length (before padding) */
  hash_update(context, bits, 8);

  /* Store state in digest */
  RIPEMDEncode(digest, context->state, 32);

  /* Zeroize sensitive information.
   */
  memset((unsigned char*) context, 0, sizeof(*context));
}

/*
   ripemd160 finalization. Ends a ripemd160 message-digest operation,
   writing the message digest and zeroizing the context.
 */
void hash_ripemd160::hash_final(unsigned char *digest, void *context_) {
  PhpRipeMd160Ctx * context = (PhpRipeMd160Ctx*)context_;
  unsigned char bits[8];
  unsigned int index, padLen;

  /* Save number of bits */
  bits[0] = (unsigned char) (context->count[0] & 0xFF);
  bits[1] = (unsigned char) ((context->count[0] >> 8) & 0xFF);
  bits[2] = (unsigned char) ((context->count[0] >> 16) & 0xFF);
  bits[3] = (unsigned char) ((context->count[0] >> 24) & 0xFF);
  bits[4] = (unsigned char) (context->count[1] & 0xFF);
  bits[5] = (unsigned char) ((context->count[1] >> 8) & 0xFF);
  bits[6] = (unsigned char) ((context->count[1] >> 16) & 0xFF);
  bits[7] = (unsigned char) ((context->count[1] >> 24) & 0xFF);

  /* Pad out to 56 mod 64.
   */
  index = (unsigned int) ((context->count[0] >> 3) & 0x3f);
  padLen = (index < 56) ? (56 - index) : (120 - index);
  hash_update(context, PADDING, padLen);

  /* Append length (before padding) */
  hash_update(context, bits, 8);

  /* Store state in digest */
  RIPEMDEncode(digest, context->state, 20);

  /* Zeroize sensitive information.
   */
  memset((unsigned char*) context, 0, sizeof(*context));
}

/*
   ripemd320 finalization. Ends a ripemd320 message-digest operation,
   writing the message digest and zeroizing the context.
 */
void hash_ripemd320::hash_final(unsigned char *digest, void *context_) {
  PhpRipeMd320Ctx * context = (PhpRipeMd320Ctx*)context_;
  unsigned char bits[8];
  unsigned int index, padLen;

  /* Save number of bits */
  bits[0] = (unsigned char) (context->count[0] & 0xFF);
  bits[1] = (unsigned char) ((context->count[0] >> 8) & 0xFF);
  bits[2] = (unsigned char) ((context->count[0] >> 16) & 0xFF);
  bits[3] = (unsigned char) ((context->count[0] >> 24) & 0xFF);
  bits[4] = (unsigned char) (context->count[1] & 0xFF);
  bits[5] = (unsigned char) ((context->count[1] >> 8) & 0xFF);
  bits[6] = (unsigned char) ((context->count[1] >> 16) & 0xFF);
  bits[7] = (unsigned char) ((context->count[1] >> 24) & 0xFF);

  /* Pad out to 56 mod 64.
   */
  index = (unsigned int) ((context->count[0] >> 3) & 0x3f);
  padLen = (index < 56) ? (56 - index) : (120 - index);
  hash_update(context, PADDING, padLen);

  /* Append length (before padding) */
  hash_update(context, bits, 8);

  /* Store state in digest */
  RIPEMDEncode(digest, context->state, 40);

  /* Zeroize sensitive information.
   */
  memset((unsigned char*) context, 0, sizeof(*context));
}

///////////////////////////////////////////////////////////////////////////////
}
