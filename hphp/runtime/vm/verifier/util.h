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

#pragma once

#include <cstddef> // for size_t
#include <stdint.h>

#include "hphp/runtime/base/string-data.h"

#include "hphp/util/arena.h"
#include "hphp/util/hash-map.h"

namespace HPHP {
namespace Verifier {

/**
 * Fixed-sized, arena-based bitset class (size determined on allocation)
 */
struct Bits {
  Bits(Arena& arena, int bit_cap);
  ~Bits() { /* words was arena-allocated */ }

  void set(int i) {
    word(i) |= mask(i);
  }

  bool get(int i) {
    return (word(i) & mask(i)) != 0;
  }

private:
  static const int BPW = 8 * sizeof(uintptr_t); // bits per word

  uintptr_t& word(int i) { return m_words[i / BPW]; }
  static uintptr_t mask(int i) { return uintptr_t(1) << (i % BPW); }

private:
  uintptr_t* m_words;
};

/**
 * Return true if container c contains e, otherwise false.
 */
template<class C, class T>
inline bool contains(const C &c, T e) {
  return c.find(e) != c.end();
}

using StringToStringTMap = hphp_fast_map<
  const StringData*,
  const StringData*,
  string_data_hash,
  string_data_tsame
>;

}} // namespace HPHP::Verifier
