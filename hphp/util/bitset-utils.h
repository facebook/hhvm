/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_BITSET_UTILS_H_
#define incl_HPHP_BITSET_UTILS_H_

#include <bitset>
#include "hphp/util/assertions.h"

namespace HPHP {

// Return the index of the first set bit in a bitset, or bitset.size() if none.
template <size_t N>
inline size_t bitset_find_first(const std::bitset<N>& bitset) {
#if defined(__GNUC__) && !defined(__APPLE__)
  // GNU provides non-standard (its a hold over from the original SGI
  // implementation) _Find_first(), which efficiently returns the index of the
  // first set bit.
  return bitset._Find_first();
#else
  for (size_t i = 0; i < bitset.size(); ++i) {
    if (bitset[i]) return i;
  }
  return bitset.size();
#endif
}

// Return the index of the first set bit in a bitset after the given index, or
// bitset.size() if none.
template <size_t N>
inline size_t bitset_find_next(const std::bitset<N>& bitset, size_t prev) {
  assertx(prev < bitset.size());
#if defined(__GNUC__) && !defined(__APPLE__)
  // GNU provides non-standard (its a hold over from the original SGI
  // implementation) _Find_next(), which given an index, efficiently returns
  // the index of the first set bit after the index.
  return bitset._Find_next(prev);
#else
  for (size_t i = prev+1; i < bitset.size(); ++i) {
    if (bitset[i]) return i;
  }
  return bitset.size();
#endif
}

// Invoke the given callable on the indices of all the set bits in a bitset.
template <typename F, size_t N>
inline void bitset_for_each_set(const std::bitset<N>& bitset, F f) {
  for (auto i = bitset_find_first(bitset);
       i < bitset.size();
       i = bitset_find_next(bitset, i)) {
    f(i);
  }
}

template <size_t N>
inline size_t findFirst1(const std::bitset<N>& bitset) {
  return bitset_find_first(bitset);
}

// range from [s, e), if not found, return e
template <size_t N>
inline size_t findFirst1(const std::bitset<N>& bitset, size_t s, size_t e) {
  assert(s <= e);
  assert(e <= bitset.size());
  for (size_t i = s; i < e; ++i) {
    if (bitset[i]) return i;
  }
  return e;
}

template <size_t N>
inline size_t findFirst0(const std::bitset<N>& bitset) {
  for (size_t i = 0; i < bitset.size(); ++i) {
    if (!bitset[i]) return i;
  }
  return bitset.size();
}

// range from [s, e), if not found, return e; if s == e, return e
template <size_t N>
inline size_t findFirst0(const std::bitset<N>& bitset, size_t s, size_t e) {
  assert(s <= e);
  assert(e <= bitset.size());
  for (size_t i = s; i < e; ++i) {
    if (!bitset[i]) return i;
  }
  return e;
}

template <size_t N>
inline size_t findLast1(const std::bitset<N>& bitset) {
  for (size_t i = bitset.size(); i-- > 0;) {
    if (bitset[i]) return i;
  }
  return bitset.size();
}

// range from [s, e), if not found, return e; if s == e, return e
template <size_t N>
inline size_t findLast1(const std::bitset<N>& bitset, size_t s, size_t e) {
  assert(s <= e);
  assert(e <= bitset.size());
  for (size_t i = e; i-- > s;) {
    if (bitset[i]) return i;
  }
  return e;
}

template <size_t N>
inline size_t findLast0(const std::bitset<N>& bitset) {
  for (size_t i = bitset.size(); i-- > 0;) {
    if (!bitset[i]) return i;
  }
  return bitset.size();
}

// range from [s, e), if not found, return e; if s == e, return e
template <size_t N>
inline size_t findLast0(const std::bitset<N>& bitset, size_t s, size_t e) {
  assert(s <= e);
  assert(e <= bitset.size());
  for (size_t i = e; i-- > s;) {
    if (!bitset[i]) return i;
  }
  return e;
}


template <size_t N>
inline std::bitset<N>& fill1(std::bitset<N>& bitset) {
  return bitset.set();
}

// range from [s, e)
template <size_t N>
inline std::bitset<N>& fill1(std::bitset<N>& bitset,
                      size_t s, size_t e) {
  assert(s < e);
  assert(e <= bitset.size());
  for(size_t i = s; i < e; ++i){
    bitset.set(i);
  }
  return bitset;
}

template <size_t N>
inline std::bitset<N>& fill0(std::bitset<N>& bitset) {
  return bitset.reset();
}

// range from [s, e)
template <size_t N>
inline std::bitset<N>& fill0(std::bitset<N>& bitset,
                      size_t s, size_t e) {
  assert(s < e);
  assert(e <= bitset.size());
  for(size_t i = s; i < e; ++i){
    bitset.reset(i);
  }
  return bitset;
}

} // HPHP

#endif
