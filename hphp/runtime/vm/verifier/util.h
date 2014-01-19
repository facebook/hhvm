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

#ifndef incl_HPHP_VM_VERIFIER_UTIL_H_
#define incl_HPHP_VM_VERIFIER_UTIL_H_

#include <cstddef> // for size_t
#include <vector>
#include <stdint.h>
#include <boost/iterator/iterator_traits.hpp>
#include "hphp/util/assertions.h"
#include "hphp/util/arena.h"

namespace HPHP {
namespace Verifier {

/**
 * Fixed-sized, arena-based bitset class (size determined on allocation)
 */
class Bits {
  static const int BPW = 8 * sizeof(uintptr_t); // bits per word
 public:
  Bits(Arena& arena, int bit_cap);
  ~Bits() { /* words was arena-allocated */ }
 public:
  void set(int i) {
    word(i) |= mask(i);
  }

  bool get(int i) {
    return (word(i) & mask(i)) != 0;
  }
 private:
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

}} // namespace HPHP::Verifier

#endif // incl_HPHP_VM_VERIFIER_UTIL_H
