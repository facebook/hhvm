/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_UTIL_IMMED_H
#define incl_HPHP_UTIL_IMMED_H

#include "hphp/util/safe-cast.h"

namespace HPHP { namespace jit {

/*
 * When selecting encodings, we often need to assess a two's complement
 * distance to see if it fits in a shorter encoding.
 */
inline bool deltaFits(int64_t delta, int s) {
  // sz::qword is always true
  assert(s == sz::byte ||
         s == sz::word ||
         s == sz::dword);
  int64_t bits = s * 8;
  return delta < (1ll << (bits-1)) && delta >= -(1ll << (bits-1));
}

// The unsigned equivalent of deltaFits
inline bool magFits(uint64_t val, int s) {
  // sz::qword is always true
  assert(s == sz::byte ||
         s == sz::word ||
         s == sz::dword);
  uint64_t bits = s * 8;
  return (val & ((1ull << bits) - 1)) == val;
}

/*
 * Immediate wrappers for the assembler.
 *
 * Immed only allows 32-bit signed values.  Unsigned-32bit values and
 * larger values will not safely fit, we want the caller to deal with
 * it or explicitly downcast to int32_t.
 *
 * The Immed64 wrapper picks up whether the immediate argument was an integer
 * or a pointer type, so we don't have to cast pointers at callsites.
 *
 * Immediates are always treated as sign-extended values, but it's
 * often convenient to use unsigned types, so we allow it with an
 * implicit implementation-defined conversion.
 */
struct Immed {
  /* implicit */ Immed(int i) : m_int(i) {}
#ifdef _MSC_VER
  // MSVC prefers not changing the sign of the value when
  // resolving the overloads, which means that an unsigned
  // argument, even of a size smaller than int, will error
  // so we provide an unsigned overload to fix that.
  /* implicit */ Immed(unsigned int i) : m_int((int)i) {}
#else
  /* implicit */ Immed(unsigned i) = delete;
#endif
  /* implicit */ Immed(long i) = delete;
  /* implicit */ Immed(unsigned long i) = delete;
  /* implicit */ Immed(long long i) = delete;
  /* implicit */ Immed(unsigned long long i) = delete;

  int64_t q() const { return m_int; }
  int32_t l() const { return safe_cast<int32_t>(m_int); }
  int16_t w() const { return safe_cast<int16_t>(m_int); }
  int8_t  b() const { return safe_cast<int8_t>(m_int); }
  uint8_t ub() const { return safe_cast<uint8_t>(m_int); }

  bool fits(int sz) const { return deltaFits(m_int, sz); }

  Immed operator-() { return -this->m_int; }

private:
  int32_t m_int;
};

struct Immed64 {
  template<class T>
  /* implicit */ Immed64(T i,
                         typename std::enable_if<
                           std::is_integral<T>::value ||
                           std::is_enum<T>::value
                         >::type* = 0)
    : m_long(i)
  {}

  template<class T>
  /* implicit */ Immed64(T* p)
    : m_long(reinterpret_cast<uintptr_t>(p))
  {}

  int64_t q() const { return m_long; }
  int32_t l() const { return safe_cast<int32_t>(m_long); }
  int16_t w() const { return safe_cast<int16_t>(m_long); }
  int8_t  b() const { return safe_cast<int8_t>(m_long); }

  bool fits(int sz) const { return deltaFits(m_long, sz); }

private:
  int64_t m_long;
};

}}
#endif
