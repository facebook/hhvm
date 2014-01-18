/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

namespace HPHP { namespace JIT {

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
 * Immediate wrapper for the assembler.
 *
 * This wrapper picks up whether the immediate argument was an integer
 * or a pointer type, so we don't have to cast pointers at callsites.
 *
 * Immediates are always treated as sign-extended values, but it's
 * often convenient to use unsigned types, so we allow it with an
 * implicit implementation-defined conversion.
 */
struct Immed {
  template<class T>
  /* implicit */ Immed(T i,
                       typename std::enable_if<
                         std::is_integral<T>::value ||
                         std::is_enum<T>::value
                       >::type* = 0)
    : m_int(i)
  {}

  template<class T>
  /* implicit */ Immed(T* p)
    : m_int(reinterpret_cast<uintptr_t>(p))
  {}

  int64_t q() const { return m_int; }
  int32_t l() const { return safe_cast<int32_t>(m_int); }
  int16_t w() const { return safe_cast<int16_t>(m_int); }
  int8_t  b() const { return safe_cast<int8_t>(m_int); }

  bool fits(int sz) const { return deltaFits(m_int, sz); }

private:
  intptr_t m_int;
};

}}
#endif
