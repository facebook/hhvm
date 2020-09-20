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
 * SHA implementation low level operation
 */

#ifndef BITFN_H
#define BITFN_H
#include <stdint.h>

static inline unsigned int rol32(unsigned int word, unsigned int shift)
{
  return (word << shift) | (word >> (32 - shift));
}

static inline unsigned int ror32(unsigned int word, unsigned int shift)
{
  return (word >> shift) | (word << (32 - shift));
}

static inline uint64_t rol64(uint64_t word, unsigned int shift)
{
  return (word << shift) | (word >> (64 - shift));
}

static inline uint64_t ror64(uint64_t word, unsigned int shift)
{
  return (word >> shift) | (word << (64 - shift));
}

#if (defined(__i386__) || defined(__x86_64__)) && !defined(NO_INLINE_ASM)
static inline unsigned int swap32(unsigned int a)
{
  asm ("bswap %0" : "=r" (a) : "0" (a));
  return a;
}
#else
static inline unsigned int swap32(unsigned int a)
{
  return (a << 24) | ((a & 0xff00) << 8) | ((a >> 8) & 0xff00) | (a >> 24);
}
#endif

#if defined(__x86_64__) && !defined(NO_INLINE_ASM)
static inline uint64_t swap64(uint64_t a)
{
  asm ("bswap %0" : "=r" (a) : "0" (a));
  return a;
}
#else
static inline uint64_t swap64(uint64_t a)
{
  return ((uint64_t) swap32((unsigned int) (a >> 32))) |
    (((uint64_t) swap32((unsigned int) a)) << 32);
}
#endif

/* big endian to cpu */
#ifdef __APPLE__
#include <architecture/byte_order.h>
#elif defined(__FreeBSD__)
#include <sys/endian.h>
#elif WIN32
/* nothing */
#else
#include <endian.h>
#endif

#if LITTLE_ENDIAN == BYTE_ORDER
#define be32_to_cpu(a) swap32(a)
#define cpu_to_be32(a) swap32(a)
#define be64_to_cpu(a) swap64(a)
#define cpu_to_be64(a) swap64(a)
#elif BIG_ENDIAN == BYTE_ORDER
#define be32_to_cpu(a) (a)
#define cpu_to_be32(a) (a)
#define be64_to_cpu(a) (a)
#define cpu_to_be64(a) (a)
#else
#error "endian not supported"
#endif

#endif /* !BITFN_H */
