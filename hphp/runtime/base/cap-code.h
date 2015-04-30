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

  explicit operator uint16_t() const {
    return code;
  }
};

//////////////////////////////////////////////////////////////////////

}

#endif
