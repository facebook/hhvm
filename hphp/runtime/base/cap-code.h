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
//
// Note that the internal representation used here is able to represent numbers
// larger than std::numeric_limits<uint32_t>::max(), but we intentionally limit
// the range so that we can always decode into a uint32_t.

struct CapCode {
  static auto constexpr B = 11u;
  static auto constexpr M = (1 << B) - 1; // mask with B low-order 1s
  static auto constexpr Threshold = M;
  static auto constexpr MaxExp = (1 << (16 - B)) - 1;
  // Don't pass numbers larger than this to the encoder, after rounding up, the
  // decoded number won't fit in 32 bits.
  static uint32_t constexpr Max = 0xffe00000;

  uint16_t code;

  // return the exponent value to use, in the range 0..21
  static ALWAYS_INLINE uint32_t calc_exp(uint32_t c) {
    assert(c > Threshold / 2);
#ifdef __x86_64
    uint32_t i;
    __asm("bsr %1,%0\n" : "=r"(i) : "r"(c));
    assert(i >= B - 1);
    return i - (B - 1);
#else
    return (32 - B) - __builtin_clz(c);
#endif
  }

  // return the lowest encodable value >= n
  static ALWAYS_INLINE CapCode ceil(uint32_t n) {
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
  static ALWAYS_INLINE CapCode floor(uint32_t n) {
    assert(n <= Max);
    if (n <= Threshold) return {static_cast<uint16_t>(n)};
    auto e = calc_exp(n);
    auto m = n >> e; // discard low bits
    assert(m <= M && e <= MaxExp);
    return {static_cast<uint16_t>(e << B | m)};
  }

  // return the exact value n (must be small)
  static ALWAYS_INLINE CapCode exact(uint32_t n) {
    assert(n <= Threshold);
    return CapCode{static_cast<uint16_t>(n)};
  }

  // return the decoded value
  ALWAYS_INLINE uint32_t decode() const {
    uint32_t m = code & M;
    uint32_t e = code >> B;
    return m << e;
  }

  // return ceil(n).decode()
  static ALWAYS_INLINE uint32_t roundUp(uint32_t n) {
    assert(n <= Max);
    // Following the natural way of doing it, we would compute `mask' from `n',
    // but instead, we compute from `m = n - 1' here as a little trick to save
    // an instruction or two.  It still works because
    // (1) when `n <= Threashold + 1', we just return `n';
    // (2) when `bsr(m) == bsr(n)', `mask' will be the same for `m' and `n', so
    //     it doesn't matter;
    // (3) otherwise, we have `n == 1 << K', and  `m' is just `K' 1's in its
    //     binary form (K > 10).  In this case `(m | mask) == m', and we
    //     eventually return `n' as desired.  Note that the computed `e',
    //     and thus `mask', are *wrong* here, but they don't matter to the
    //     final result.
    //
    // Depending on situations at call sites, the `-1' on `n' and `+1' on the
    // return value can often get absorbed into neighboring instructions, this
    // hopefully saves us an instruction or two.
    auto m = n - 1;
    if (m <= Threshold) return m + 1;
    auto e = calc_exp(m);
    auto mask = (1 << e) - 1;
    return (m | mask) + 1;
  }

  // true if c is encodable exactly, without rounding
  static ALWAYS_INLINE bool encodable(size_t c) {
    if (c > Max) return false;
    auto const n = static_cast<uint32_t>(c);
    if (n <= Threshold + 1) return true;
    auto e = calc_exp(n);
    auto mask = (1 << e) - 1;
    return !(n & mask);
  }

  ALWAYS_INLINE explicit operator uint16_t() const {
    return code;
  }

  ALWAYS_INLINE bool operator==(const CapCode& other) const {
    return code == other.code;
  }

  ALWAYS_INLINE bool operator!=(const CapCode& other) const {
    return code != other.code;
  }
};

//////////////////////////////////////////////////////////////////////

}

#endif
