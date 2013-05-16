/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "hphp/runtime/ext/hash/hash_snefru.h"
#include "hphp/runtime/ext/hash/php_hash_snefru_tables.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

typedef struct {
  unsigned int state[16];
  unsigned int count[2];
  unsigned char length;
  unsigned char buffer[32];
} PHP_SNEFRU_CTX;

hash_snefru::hash_snefru() : HashEngine(32, 32, sizeof(PHP_SNEFRU_CTX)) {
}

#define round(L, C, N, SB)  \
  SBE = SB[C & 0xff];       \
  L ^= SBE;                 \
  N ^= SBE

static inline void Snefru(unsigned int input[16]) {
  static int shifts[4] = {16, 8, 16, 24};
  int b, index, rshift, lshift;
  const unsigned int *t0,*t1;
  unsigned int SBE,B00,B01,B02,B03,B04,B05,B06,B07,B08,B09,B10,
    B11,B12,B13,B14,B15;

  B00 = input[0];
  B01 = input[1];
  B02 = input[2];
  B03 = input[3];
  B04 = input[4];
  B05 = input[5];
  B06 = input[6];
  B07 = input[7];
  B08 = input[8];
  B09 = input[9];
  B10 = input[10];
  B11 = input[11];
  B12 = input[12];
  B13 = input[13];
  B14 = input[14];
  B15 = input[15];

  for (index = 0; index < 8; index++) {
    t0 = tables[2*index+0];
    t1 = tables[2*index+1];
    for (b = 0; b < 4; b++) {
      round(B15, B00, B01, t0);
      round(B00, B01, B02, t0);
      round(B01, B02, B03, t1);
      round(B02, B03, B04, t1);
      round(B03, B04, B05, t0);
      round(B04, B05, B06, t0);
      round(B05, B06, B07, t1);
      round(B06, B07, B08, t1);
      round(B07, B08, B09, t0);
      round(B08, B09, B10, t0);
      round(B09, B10, B11, t1);
      round(B10, B11, B12, t1);
      round(B11, B12, B13, t0);
      round(B12, B13, B14, t0);
      round(B13, B14, B15, t1);
      round(B14, B15, B00, t1);

      rshift = shifts[b];
      lshift = 32-rshift;

      B00 = (B00 >> rshift) | (B00 << lshift);
      B01 = (B01 >> rshift) | (B01 << lshift);
      B02 = (B02 >> rshift) | (B02 << lshift);
      B03 = (B03 >> rshift) | (B03 << lshift);
      B04 = (B04 >> rshift) | (B04 << lshift);
      B05 = (B05 >> rshift) | (B05 << lshift);
      B06 = (B06 >> rshift) | (B06 << lshift);
      B07 = (B07 >> rshift) | (B07 << lshift);
      B08 = (B08 >> rshift) | (B08 << lshift);
      B09 = (B09 >> rshift) | (B09 << lshift);
      B10 = (B10 >> rshift) | (B10 << lshift);
      B11 = (B11 >> rshift) | (B11 << lshift);
      B12 = (B12 >> rshift) | (B12 << lshift);
      B13 = (B13 >> rshift) | (B13 << lshift);
      B14 = (B14 >> rshift) | (B14 << lshift);
      B15 = (B15 >> rshift) | (B15 << lshift);
    }
  }
  input[0] ^= B15;
  input[1] ^= B14;
  input[2] ^= B13;
  input[3] ^= B12;
  input[4] ^= B11;
  input[5] ^= B10;
  input[6] ^= B09;
  input[7] ^= B08;
}

static inline void SnefruTransform(PHP_SNEFRU_CTX *context,
                                   const unsigned char input[32]) {
  int i, j;

  for (i = 0, j = 0; i < 32; i += 4, ++j) {
    context->state[8+j] =
      ((input[i] & 0xff) << 24) | ((input[i+1] & 0xff) << 16) |
      ((input[i+2] & 0xff) << 8) | (input[i+3] & 0xff);
  }
  Snefru(context->state);
  memset(&context->state[8], 0, sizeof(unsigned int) * 8);
}

void hash_snefru::hash_init(void *context_) {
  PHP_SNEFRU_CTX *context = (PHP_SNEFRU_CTX*)context_;
  memset(context, 0, sizeof(*context));
}

static const unsigned int MAX32 = 0xffffffffLU;

void hash_snefru::hash_update(void *context_, const unsigned char *input,
                              unsigned int len) {
  PHP_SNEFRU_CTX *context = (PHP_SNEFRU_CTX*)context_;
  if ((MAX32 - context->count[1]) < (len * 8)) {
    context->count[0]++;
    context->count[1] = MAX32 - context->count[1];
    context->count[1] = (len * 8) - context->count[1];
  } else {
    context->count[1] += len * 8;
  }

  if (context->length + len < 32) {
    memcpy(&context->buffer[context->length], input, len);
    context->length += len;
  } else {
    size_t i = 0, r = (context->length + len) % 32;

    if (context->length) {
      i = 32 - context->length;
      memcpy(&context->buffer[context->length], input, i);
      SnefruTransform(context, context->buffer);
    }

    for (; i + 32 <= len; i += 32) {
      SnefruTransform(context, input + i);
    }

    memcpy(context->buffer, input + i, r);
    memset(&context->buffer[r], 0, 32 - r);
    context->length = r;
  }
}

void hash_snefru::hash_final(unsigned char *digest, void *context_) {
  PHP_SNEFRU_CTX *context = (PHP_SNEFRU_CTX*)context_;
  unsigned int i, j;

  if (context->length) {
    SnefruTransform(context, context->buffer);
  }

  context->state[14] = context->count[0];
  context->state[15] = context->count[1];
  Snefru(context->state);

  for (i = 0, j = 0; j < 32; i++, j += 4) {
    digest[j] = (unsigned char) ((context->state[i] >> 24) & 0xff);
    digest[j + 1] = (unsigned char) ((context->state[i] >> 16) & 0xff);
    digest[j + 2] = (unsigned char) ((context->state[i] >> 8) & 0xff);
    digest[j + 3] = (unsigned char) (context->state[i] & 0xff);
  }

  memset(context, 0, sizeof(*context));
}

///////////////////////////////////////////////////////////////////////////////
}
