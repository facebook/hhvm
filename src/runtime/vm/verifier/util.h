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
#include <util/assert.h>

#include <boost/iterator/iterator_traits.hpp>

namespace HPHP {
namespace VM {
namespace Verifier {

/**
 * Arena is an allocator that frees all memory when the arena instance
 * is destroyed.  No destructors of allocated objects will be called!
 * It is a bump-pointer allocator.
 *
 * Every allocation is rounded up to 16 bytes and is 16-byte aligned; this
 * mirrors the way stack alignment works in gcc, which should be good enough.
 *
 * If we're out of memory, allocation functions throw an Exception.
 * Blocks smaller than kMinBytes bytes are rounded up to kMinBytes, and
 * all blocks are kMinBytes-aligned.
 */
class Arena {
  static const size_t kMinBytes = 16;
  static const size_t kChunkBytes = 4096;
 public:
  Arena();
  ~Arena();
  void* alloc(size_t nbytes);
 private:
  // copying Arenas will end badly.
  Arena(const Arena&);
  Arena& operator=(const Arena&);
 private:
  void* alloc_slow(size_t nbytes);
  char* fill(size_t nbytes);
 private:
  char* m_next;
  char* m_limit;
  std::vector<char*> m_ptrs;
};

inline void* Arena::alloc(size_t nbytes) {
  nbytes = (nbytes + (kMinBytes - 1)) & ~(kMinBytes - 1); // round up
  char* ptr = m_next;
  char* next = ptr + nbytes;
  if (next <= m_limit) {
    m_next = next;
    return ptr;
  }
  return alloc_slow(nbytes);
}

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

// These global-operator-new declarations cannot be in a namespace,
// but since they take Arena arguments we won't overload anything else.

inline void* operator new(size_t nbytes, HPHP::VM::Verifier::Arena& a) {
  return a.alloc(nbytes);
}

inline void* operator new[](size_t nbytes, HPHP::VM::Verifier::Arena& a) {
  return a.alloc(nbytes);
}

#endif // incl_VM_VERIFIER_UTIL_H
