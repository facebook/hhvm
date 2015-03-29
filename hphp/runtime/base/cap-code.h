/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_CAP_CODE_H_
#define incl_HPHP_CAP_CODE_H_

namespace HPHP {

/**
 * This is an encoding scheme for a 24-bit capacity field so that
 * capacities up to almost 2^32 can be supported using only 3 bytes:
 *   cap = capCode < 0x10000 ? capCode : (capCode - 0xFF00) * 0x100
 *
 * The encoding breaks 3-byte capacity codes (capCodes) into two ranges.
 * Codes 0 - 65535 are mapped to capacities 0 - 65535 in increments of 1.
 * Codes 65536 - 16777215 are mapped to capacities 65536 - 4278255360 in
 * increments of 256. This scheme works out well in a couple ways:
 *   - No meaningful loss of granularity for the capacity field.
 *   - cap == capCode in the common case, which comes in handy.
 *   - No multiplication or division needed.
 *
 * Encodable values over 65536 are all multiples of 256, i.e. all have 0 in
 * the low 8 bits. Allocator size classes in this range are likely to also
 * be multiples of 256 as well, however if the store capacity is adjusted
 * (e.g. cap-sizeof(T)) then care must be taken to round down to an
 * encodable value.
 */

constexpr uint32_t kMaxPackedCap = 0xFF00FF00ul; // max encodable value
constexpr uint32_t kPackedCapCodeThreshold = 0x10000ul;

inline bool isEncodableCap(uint32_t cap) {
  return cap <= kPackedCapCodeThreshold ||
         (cap <= kMaxPackedCap && (cap & 0xff) == 0);
}

// convert raw capacity to an encoded capcode.
ALWAYS_INLINE
uint32_t packedCapToCode(uint32_t cap) {
  assert(isEncodableCap(cap));
  if (UNLIKELY(cap > kPackedCapCodeThreshold)) {
    auto const capCode = (cap + 0xFF00FFul) >> 8;
    assert(capCode <= 0xFFFFFFul && capCode <= cap);
    assert(((capCode - 0xFF00ul) << 8) == cap); // don't lose precision.
    return capCode;
  }
  return cap;
}

// convert encoded capcode to raw capacity
ALWAYS_INLINE
uint32_t packedCodeToCap(uint32_t capCode) {
  capCode = (capCode & 0xFFFFFFul);
  if (UNLIKELY(capCode > kPackedCapCodeThreshold)) {
    auto const cap = (capCode - 0xFF00ul) << 8;
    assert(cap <= kMaxPackedCap && capCode <= cap);
    return cap;
  }
  return capCode;
}

// round cap up to an encodable value
ALWAYS_INLINE
uint32_t roundUpPackedCap(uint32_t cap) {
  assert(cap <= kMaxPackedCap);
  if (UNLIKELY(cap > kPackedCapCodeThreshold)) {
    cap = (cap + 0xFFlu) & ~0xFFul;
  }
  // The capacity should not change if it round trips into
  // encoded form and back
  assert(isEncodableCap(cap));
  return cap;
}

// 16-bit Floating-point capacity encoding:
// [exp:5][mantissa:11]
// cap = mantissa << exp
//
// This is similar to half-precision floating point, but no sign bit,
// no exponent bias, and no implicit 1 added to the fraction.
//
// Values of 0..2047 have the same encoded and decoded bit pattern,
// since exp==0 (analogous to subnormal/denormal float point encodings).
// The max encodable value is 0xffe00000, which is larger than necessary
// for any packed array or string, whose sizes are limited to signed int32
// values (max-int32 = 0x7fffffff).

struct CapCode {
  static auto constexpr B = 11u;
  static auto constexpr M = (1 << B) - 1; // mask with B low-order 1s
  static auto constexpr Threshold = M;
  static auto constexpr Max = M << (32 - B);
  static auto constexpr MaxExp = (1 << (16 - B)) - 1;

  uint16_t code;

  // return the exponent value to use, in the range 1..21
  static uint32_t calc_exp(uint32_t c) {
    assert(c > Threshold);
#ifdef __x86_64
    uint32_t i;
    __asm("bsr %1,%0\n" : "=r"(i) : "r"(c));
    return i - (B - 1);
#else
    return (32 - B) - __builtin_clz(c);
#endif
  }

  // return the lowest encodable value >= n
  static CapCode ceil(uint32_t n) {
    assert(n <= Max);
    if (n <= Threshold) return {static_cast<uint16_t>(n)};
    auto e = calc_exp(n);
    auto m = (n + (1 << e) - 1) >> e; // round up before discarding low bits
    auto c = m >> B; // carry bit from the roundup: 0 or 1
    auto adjusted_exp = e + c;
    auto adjusted_mantissa = m >> c;
    assert(adjusted_mantissa <= M && adjusted_exp <= MaxExp);
    return {static_cast<uint16_t>(adjusted_exp << B | adjusted_mantissa)};
  }

  // return the highest encodable value <= n
  static CapCode floor(uint32_t n) {
    assert(n <= Max);
    if (n <= Threshold) return {static_cast<uint16_t>(n)};
    auto e = calc_exp(n);
    auto m = n >> e; // discard low bits
    assert(m <= M && e <= MaxExp);
    return {static_cast<uint16_t>(e << B | m)};
  }

  // return the decoded value
  uint32_t decode() const {
    uint32_t m = code & M;
    uint32_t e = code >> B;
    return m << e;
  }

  // true if c is encodable exactly, without rounding
  static bool encodable(size_t c) {
    return c <= Max && c == floor(c).decode();
  }
};

//////////////////////////////////////////////////////////////////////

}

#endif
