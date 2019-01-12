/*
 *      Copyright (C) 2006-2009 Vincent Hanquez <tab@snarc.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * SHA1 implementation as describe in wikipedia.
 */

#include <string.h>
#include <stdio.h>

#include "sha1c.h"
#include "bitfn.h"

/**
 * sha1_init - Init SHA1 context
 */
void sha1_init(struct sha1_ctx *ctx)
{
  memset(ctx, 0, sizeof(*ctx));

  /* initialize H */
  ctx->h[0] = 0x67452301;
  ctx->h[1] = 0xEFCDAB89;
  ctx->h[2] = 0x98BADCFE;
  ctx->h[3] = 0x10325476;
  ctx->h[4] = 0xC3D2E1F0;
}

/**
 * sha1_copy - Copy SHA1 context
 */
void sha1_copy(struct sha1_ctx *dst, struct sha1_ctx *src)
{
  memcpy(dst, src, sizeof(*dst));
}

#define f1(x, y, z)   (z ^ (x & (y ^ z)))         /* x ? y : z */
#define f2(x, y, z)   (x ^ y ^ z)                 /* XOR */
#define f3(x, y, z)   ((x & y) + (z & (x ^ y)))   /* majority */
#define f4(x, y, z)   f2(x, y, z)

#define K1  0x5A827999L                 /* Rounds  0-19: sqrt(2) * 2^30 */
#define K2  0x6ED9EBA1L                 /* Rounds 20-39: sqrt(3) * 2^30 */
#define K3  0x8F1BBCDCL                 /* Rounds 40-59: sqrt(5) * 2^30 */
#define K4  0xCA62C1D6L                 /* Rounds 60-79: sqrt(10) * 2^30 */

#define R(a, b, c, d, e, f, k, w)  e += rol32(a, 5) + f(b, c, d) + k + w; \
  b = rol32(b, 30)

#define M(i)  (w[i & 0x0f] = rol32(w[i & 0x0f] ^ w[(i - 14) & 0x0f]     \
                                   ^ w[(i - 8) & 0x0f] ^ w[(i - 3) & 0x0f], 1))

    static inline void sha1_do_chunk(unsigned char W[], unsigned int h[])
    {
      unsigned int a, b, c, d, e;
      unsigned int w[80];

#define CPY(i)  w[i] = be32_to_cpu(((unsigned int *) W)[i])
      CPY(0); CPY(1); CPY(2); CPY(3); CPY(4); CPY(5); CPY(6); CPY(7);
      CPY(8); CPY(9); CPY(10); CPY(11); CPY(12); CPY(13); CPY(14); CPY(15);
#undef CPY

      a = h[0];
      b = h[1];
      c = h[2];
      d = h[3];
      e = h[4];

      /* following unrolled from:
       *  for (i = 0; i < 20; i++) {
       *    t = f1(b, c, d) + K1 + rol32(a, 5) + e + M(i);
       *    e = d; d = c; c = rol32(b, 30); b = a; a = t;
       *  }
       */
      R(a, b, c, d, e, f1, K1, w[0]);
      R(e, a, b, c, d, f1, K1, w[1]);
      R(d, e, a, b, c, f1, K1, w[2]);
      R(c, d, e, a, b, f1, K1, w[3]);
      R(b, c, d, e, a, f1, K1, w[4]);
      R(a, b, c, d, e, f1, K1, w[5]);
      R(e, a, b, c, d, f1, K1, w[6]);
      R(d, e, a, b, c, f1, K1, w[7]);
      R(c, d, e, a, b, f1, K1, w[8]);
      R(b, c, d, e, a, f1, K1, w[9]);
      R(a, b, c, d, e, f1, K1, w[10]);
      R(e, a, b, c, d, f1, K1, w[11]);
      R(d, e, a, b, c, f1, K1, w[12]);
      R(c, d, e, a, b, f1, K1, w[13]);
      R(b, c, d, e, a, f1, K1, w[14]);
      R(a, b, c, d, e, f1, K1, w[15]);
      R(e, a, b, c, d, f1, K1, M(16));
      R(d, e, a, b, c, f1, K1, M(17));
      R(c, d, e, a, b, f1, K1, M(18));
      R(b, c, d, e, a, f1, K1, M(19));

      /* following unrolled from:
       *  for (i = 20; i < 40; i++) {
       *    t = f2(b, c, d) + K2 + rol32(a, 5) + e + M(i);
       *    e = d; d = c; c = rol32(b, 30); b = a; a = t;
       *  }
       */

      R(a, b, c, d, e, f2, K2, M(20));
      R(e, a, b, c, d, f2, K2, M(21));
      R(d, e, a, b, c, f2, K2, M(22));
      R(c, d, e, a, b, f2, K2, M(23));
      R(b, c, d, e, a, f2, K2, M(24));
      R(a, b, c, d, e, f2, K2, M(25));
      R(e, a, b, c, d, f2, K2, M(26));
      R(d, e, a, b, c, f2, K2, M(27));
      R(c, d, e, a, b, f2, K2, M(28));
      R(b, c, d, e, a, f2, K2, M(29));
      R(a, b, c, d, e, f2, K2, M(30));
      R(e, a, b, c, d, f2, K2, M(31));
      R(d, e, a, b, c, f2, K2, M(32));
      R(c, d, e, a, b, f2, K2, M(33));
      R(b, c, d, e, a, f2, K2, M(34));
      R(a, b, c, d, e, f2, K2, M(35));
      R(e, a, b, c, d, f2, K2, M(36));
      R(d, e, a, b, c, f2, K2, M(37));
      R(c, d, e, a, b, f2, K2, M(38));
      R(b, c, d, e, a, f2, K2, M(39));

      /* following unrolled from:
       *  for (i = 40; i < 60; i++) {
       *    t = f3(b, c, d) + K3 + rol32(a, 5) + e + M(i);
       *    e = d; d = c; c = rol32(b, 30); b = a; a = t;
       *  }
       */

      R(a, b, c, d, e, f3, K3, M(40));
      R(e, a, b, c, d, f3, K3, M(41));
      R(d, e, a, b, c, f3, K3, M(42));
      R(c, d, e, a, b, f3, K3, M(43));
      R(b, c, d, e, a, f3, K3, M(44));
      R(a, b, c, d, e, f3, K3, M(45));
      R(e, a, b, c, d, f3, K3, M(46));
      R(d, e, a, b, c, f3, K3, M(47));
      R(c, d, e, a, b, f3, K3, M(48));
      R(b, c, d, e, a, f3, K3, M(49));
      R(a, b, c, d, e, f3, K3, M(50));
      R(e, a, b, c, d, f3, K3, M(51));
      R(d, e, a, b, c, f3, K3, M(52));
      R(c, d, e, a, b, f3, K3, M(53));
      R(b, c, d, e, a, f3, K3, M(54));
      R(a, b, c, d, e, f3, K3, M(55));
      R(e, a, b, c, d, f3, K3, M(56));
      R(d, e, a, b, c, f3, K3, M(57));
      R(c, d, e, a, b, f3, K3, M(58));
      R(b, c, d, e, a, f3, K3, M(59));

      /* following unrolled from:
       *  for (i = 60; i < 80; i++) {
       *    t = f2(b, c, d) + K4 + rol32(a, 5) + e + M(i);
       *    e = d; d = c; c = rol32(b, 30); b = a; a = t;
       *  }
       */
      R(a, b, c, d, e, f4, K4, M(60));
      R(e, a, b, c, d, f4, K4, M(61));
      R(d, e, a, b, c, f4, K4, M(62));
      R(c, d, e, a, b, f4, K4, M(63));
      R(b, c, d, e, a, f4, K4, M(64));
      R(a, b, c, d, e, f4, K4, M(65));
      R(e, a, b, c, d, f4, K4, M(66));
      R(d, e, a, b, c, f4, K4, M(67));
      R(c, d, e, a, b, f4, K4, M(68));
      R(b, c, d, e, a, f4, K4, M(69));
      R(a, b, c, d, e, f4, K4, M(70));
      R(e, a, b, c, d, f4, K4, M(71));
      R(d, e, a, b, c, f4, K4, M(72));
      R(c, d, e, a, b, f4, K4, M(73));
      R(b, c, d, e, a, f4, K4, M(74));
      R(a, b, c, d, e, f4, K4, M(75));
      R(e, a, b, c, d, f4, K4, M(76));
      R(d, e, a, b, c, f4, K4, M(77));
      R(c, d, e, a, b, f4, K4, M(78));
      R(b, c, d, e, a, f4, K4, M(79));

      h[0] += a;
      h[1] += b;
      h[2] += c;
      h[3] += d;
      h[4] += e;
    }

/**
 * sha1_update - Update the SHA1 context values with length bytes of data
 */
void sha1_update(struct sha1_ctx *ctx, unsigned char *data, int len)
{
  unsigned int index, to_fill;

  index = (unsigned int) (ctx->sz & 0x3f);
  to_fill = 64 - index;

  ctx->sz += len;

  /* process partial buffer if there's enough data to make a block */
  if (index && len >= to_fill) {
    memcpy(ctx->buf + index, data, to_fill);
    sha1_do_chunk(ctx->buf, ctx->h);
    len -= to_fill;
    data += to_fill;
    index = 0;
  }

  /* process as much 64-block as possible */
  for (; len >= 64; len -= 64, data += 64)
    sha1_do_chunk(data, ctx->h);

  /* append data into buf */
  if (len)
    memcpy(ctx->buf + index, data, len);
}

/**
 * sha1_finalize - Finalize the context and create the SHA1 digest
 */
void sha1_finalize(struct sha1_ctx *ctx, sha1_digest *out)
{
  static unsigned char padding[64] = { 0x80, };
  unsigned int bits[2];
  unsigned int index, padlen;

  /* add padding and update data with it */
  bits[0] = cpu_to_be32((unsigned int) (ctx->sz >> 29));
  bits[1] = cpu_to_be32((unsigned int) (ctx->sz << 3));

  /* pad out to 56 */
  index = (unsigned int) (ctx->sz & 0x3f);
  padlen = (index < 56) ? (56 - index) : ((64 + 56) - index);
  sha1_update(ctx, padding, padlen);

  /* append length */
  sha1_update(ctx, (unsigned char *) bits, sizeof(bits));

  /* output hash */
  out->digest[0] = cpu_to_be32(ctx->h[0]);
  out->digest[1] = cpu_to_be32(ctx->h[1]);
  out->digest[2] = cpu_to_be32(ctx->h[2]);
  out->digest[3] = cpu_to_be32(ctx->h[3]);
  out->digest[4] = cpu_to_be32(ctx->h[4]);
}

/**
 * sha1_to_bin - Transform the SHA1 digest into a binary data
 */
void sha1_to_bin(sha1_digest *digest, char *out)
{
  uint32_t *ptr = (uint32_t *) out;

  ptr[0] = digest->digest[0];
  ptr[1] = digest->digest[1];
  ptr[2] = digest->digest[2];
  ptr[3] = digest->digest[3];
  ptr[4] = digest->digest[4];
}

/**
 * sha1_to_hex - Transform the SHA1 digest into a readable data
 */
void sha1_to_hex(sha1_digest *digest, char *out)
{

#define D(i) (cpu_to_be32(digest->digest[i]))
  snprintf(out, 41, "%08x%08x%08x%08x%08x",
           D(0), D(1), D(2), D(3), D(4));
#undef D
}
