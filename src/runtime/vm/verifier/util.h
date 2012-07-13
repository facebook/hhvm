/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef incl_VM_VERIFIER_UTIL_H_
#define incl_VM_VERIFIER_UTIL_H_

#include <cstddef> // for size_t
#include <vector>
#include <stdint.h>
#include <boost/iterator/iterator_traits.hpp>
#include "util/assert.h"
#include "util/arena.h"

namespace HPHP {
namespace VM {
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

template <class C>
class StdRange {
 typedef typename C::value_type V;
 public:
  explicit StdRange(const C& c) : m_p(c.begin()), m_end(c.end()) {}
  bool empty() const { return m_p == m_end; }
  const V& front() const {
    ASSERT(!empty());
    return *m_p;
  }
  const V& popFront() {
    const V& f = front();
    ++m_p;
    return f;
  }
 private:
  typename C::const_iterator m_p, m_end;
};

// Range over a pair of ForwardIterators.
template <class Iterator>
struct IterRange {
  typedef typename boost::iterator_value<Iterator>::type value_type;

  IterRange(Iterator first, Iterator last)
    : m_first(first)
    , m_last(last)
  {}

  const value_type& front() const { return *m_first; }
  bool empty() const { return m_first == m_last; }
  value_type popFront() { return *m_first++; }

private:
  Iterator m_first;
  Iterator m_last;
};

/**
 * Return true if container c contains e, otherwise false.
 */
template<class C, class T>
inline bool contains(const C &c, T e) {
  return c.find(e) != c.end();
}

}}} // namespace HPHP::VM::Verifier

#endif // incl_VM_VERIFIER_UTIL_H
