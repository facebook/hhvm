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

#ifndef incl_HPHP_JIT_ID_SET_
#define incl_HPHP_JIT_ID_SET_

#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/vm/jit/ir-unit.h"

namespace HPHP { namespace JIT {

//////////////////////////////////////////////////////////////////////

/*
 * IdSet implements a bitset over the range of Key's ids, optimized for
 * a relatively low number of bits.  If we only need one word of storage,
 * this doesn't allocate any extra memory.  Otherwise allocate an array
 * of words using smart_malloc.
 */
template<class Key>
struct IdSet {
  enum { kBits = 8 * sizeof(size_t) };

  IdSet() : m_bits(0), m_cap(1) {}

  IdSet(const IdSet& other) : m_cap(1) {
    if (other.m_cap == 1) {
      m_bits = other.m_bits;
    } else {
      m_bits = 0;
      (*this) |= other;
    }
  }

  ~IdSet() {
    if (m_cap > 1) smart_free(m_bitsptr);
  }

  // Add an id to the bitset
  void add(uint32_t id) { resize(id + 1)[id / kBits] |= mask(id); }
  void add(const Key* k) { add(k->id()); }
  void add(const Key& k) { add(k->id()); }

  // Remove id from this bitset
  void erase(uint32_t id) {
    if (id < m_cap * kBits) bits()[id / kBits] &= ~mask(id);
  }
  void erase(const Key* k) { erase(k->id()); }
  void erase(const Key& k) { erase(k.id()); }

  // Add all of the ids in other to this bitset
  void operator|=(const IdSet& other) {
    auto b1 = resize(other.m_cap * kBits);
    auto b2 = other.bits();
    for (size_t i = 0, n = other.m_cap; i < n; ++i) {
      b1[i] |= b2[i];
    }
  }

  // Assign other to this bitset.  If other is shorter, do not shrink,
  // just clear the extra bits.
  IdSet& operator=(const IdSet& other) {
    auto b1 = resize(std::max(m_cap, other.m_cap) * kBits);
    auto b2 = other.bits();
    size_t i = 0;
    for (auto n = other.m_cap; i < n; ++i) b1[i] = b2[i];
    for (auto n = m_cap; i < n; ++i) b1[i] = 0;
    return *this;
  }

  // Read-only access to bits using [id]
  bool operator[](uint32_t id) const {
    return id < m_cap * kBits && (bits()[id / kBits] & mask(id)) != 0;
  }
  bool operator[](const Key& k) const { return (*this)[k.id()]; }
  bool operator[](const Key* k) const { return (*this)[k->id()]; }

  // invoke fun(id) for each id bit set
  template <class Fun> void forEach(Fun f) const {
    auto b = bits();
    for (size_t i = 0, n = m_cap; i < n; ++i) {
      for (auto m = b[i]; m != 0;) {
        auto j = __builtin_ffsl(m) - 1;
        f(i * kBits + j);
        m &= ~mask(j);
      }
    }
  }

private:
  // Return a mask with 1 bit set
  static size_t mask(size_t id) { return size_t(1) << (id % kBits); }

  // Return a bitvector pointer.
  size_t* bits() { return m_cap > 1 ? m_bitsptr : &m_bits; }
  const size_t* bits() const { return m_cap > 1 ? m_bitsptr : &m_bits; }

  // Grow this bitset, reallocating if necessary.
  size_t* resize(size_t n) {
    if (n <= m_cap * kBits) return bits();
    auto cap = (n + kBits - 1) / kBits;
    if (m_cap == 1) {
      auto ptr = (size_t*)smart_malloc(cap * sizeof(size_t));
      ptr[0] = m_bits;
      m_bitsptr = ptr;
    } else {
      m_bitsptr = (size_t*)smart_realloc(m_bitsptr, cap * sizeof(size_t));
    }
    for (size_t i = m_cap; i < cap; ++i) m_bitsptr[i] = 0;
    m_cap = cap;
    return m_bitsptr;
  }

private:
  static constexpr Key* nullKey { nullptr };
  union {
    size_t m_bits;
    size_t* m_bitsptr;
  };
  size_t m_cap; // in words
};

//////////////////////////////////////////////////////////////////////

}}

#endif
