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
#ifndef SHA1_C_H
#define SHA1_C_H

struct sha1_ctx
{
  unsigned int h[5];
  unsigned char buf[64];
  unsigned long long sz;
};

typedef struct { unsigned int digest[5]; } sha1_digest;

void sha1_init(struct sha1_ctx *ctx);
void sha1_copy(struct sha1_ctx *dst, struct sha1_ctx *src);
void sha1_update(struct sha1_ctx *ctx, unsigned char *data, int len);
void sha1_finalize(struct sha1_ctx *ctx, sha1_digest *out);
void sha1_to_bin(sha1_digest *digest, char *out);
void sha1_to_hex(sha1_digest *digest, char *out);

#endif // SHA1_C_H
