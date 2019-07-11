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

#ifndef incl_HPHP_JIT_ID_SET_
#define incl_HPHP_JIT_ID_SET_

#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/vm/jit/ir-unit.h"

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////

/*
 * IdSet implements a bitset over the range of Key's ids, optimized for
 * a relatively low number of bits.  If we only need one word of storage,
 * this doesn't allocate any extra memory.  Otherwise allocate an array
 * of words.
 */
template<class Key>
struct IdSet {
  IdSet() : m_bits{0}, m_cap{0}, m_base{0} {}

  explicit IdSet(const Key* k) : IdSet() { add(k); }
  explicit IdSet(const Key& k) : IdSet() { add(k); }

  IdSet(const IdSet& o) {
    if (o.m_cap <= 1) {
      m_cap = o.m_cap;
      m_base = o.m_base;
      m_bits = o.m_bits;
      return;
    }

    // "Trim" the other set by finding the first and last block index
    // with bits set. These are the only blocks we actually need to
    // copy. This avoids expanding the current set by more than
    // necessary.
    size_t begin = 0;
    size_t end = 0;
    auto foundBegin = false;
    for (size_t i = 0; i < o.m_cap; ++i) {
      if (!o.m_bitsptr[i]) continue;
      if (!foundBegin) {
        begin = i;
        foundBegin = true;
      }
      end = i + 1;
    }

    if (begin == end) {
      // Other set is empty, so we are as well.
      m_cap = 0;
      m_base = 0;
      m_bits = 0;
    } else if (begin + 1 == end) {
      // Other set has just a single block, so we can avoid allocating
      // memory.
      m_cap = 1;
      m_base = begin * kBlockSize + o.m_base;
      m_bits = o.m_bitsptr[begin];
    } else {
      // Otherwise we need to allocate and copy.
      m_cap = end - begin;
      m_base = begin * kBlockSize + o.m_base;
      m_bitsptr = (uint64_t*)malloc(m_cap * sizeof(uint64_t));
      for (size_t i = begin; i < end; ++i) {
        m_bitsptr[i - begin] = o.m_bitsptr[i];
      }
    }
  }

  IdSet(IdSet&& o) noexcept
    : m_bits{o.m_bits}
    , m_cap{o.m_cap}
    , m_base{o.m_base}
  {
    o.m_bits = 0;
    o.m_cap = 0;
    o.m_base = 0;
  }

  IdSet& operator=(const IdSet& o) {
    if (this == &o) return *this;

    auto b2 = o.bits();

    // "Trim" the other set by finding the first and last block index
    // with bits set. These are the only blocks we actually need to
    // copy. This avoids expanding the current set by more than
    // necessary.
    size_t begin = 0;
    size_t end = 0;
    auto foundBegin = false;
    for (size_t i = 0; i < o.m_cap; ++i) {
      if (!b2[i]) continue;
      if (!foundBegin) {
        begin = i;
        foundBegin = true;
      }
      end = i + 1;
    }

    if (begin == end) {
      // Other set is actually empty. Just clear our bits.
      clear();
      return *this;
    }

    // Ensure we have enough space.
    auto b1 =
      resize(begin * kBlockSize + o.m_base, end * kBlockSize + o.m_base);

    assertx(m_base <= (begin * kBlockSize + o.m_base));
    assertx((m_cap * kBlockSize + m_base) >= (end * kBlockSize + o.m_base));

    auto const prefix = (o.m_base / kBlockSize + begin) - m_base / kBlockSize;
    for (size_t i = 0; i < prefix; ++i) b1[i] = 0;
    for (size_t i = begin; i < end; ++i) b1[i - begin + prefix] = b2[i];
    for (size_t i = end - begin + prefix; i < m_cap; ++i) b1[i] = 0;

    return *this;
  }

  IdSet& operator=(IdSet&& other) noexcept {
    this->swap(other);
    return *this;
  }

  ~IdSet() {
    if (m_cap > 1) free(m_bitsptr);
  }

  // Add an id
  void add(uint32_t id) {
    auto ptr = resize(id, id + 1);
    assertx(id >= m_base &&
            id < ((m_cap * kBlockSize) + m_base));
    bitvec_set(ptr, id - m_base);
  }
  void add(const Key* k) { add(k->id()); }
  void add(const Key& k) { add(k.id()); }

  // Remove id
  void erase(uint32_t id) {
    if (id < m_base || id >= ((m_cap * kBlockSize) + m_base)) return;
    bitvec_clear(bits(), id - m_base);
  }
  void erase(const Key* k) { erase(k->id()); }
  void erase(const Key& k) { erase(k.id()); }

  // Remove all ids
  void clear() {
    auto b = bits();
    for (size_t i = 0; i < m_cap; ++i) b[i] = 0;
  }

  // Union another IdSet with this
  void operator|=(const IdSet& o) {
    if (this == &o) return;

    auto b2 = o.bits();

    // "Trim" the other set by finding the first and last block index
    // with bits set. These are the only blocks we actually need to
    // copy. This avoids expanding the current set by more than
    // necessary.
    size_t begin = 0;
    size_t end = 0;
    auto foundBegin = false;
    for (size_t i = 0; i < o.m_cap; ++i) {
      if (!b2[i]) continue;
      if (!foundBegin) {
        begin = i;
        foundBegin = true;
      }
      end = i + 1;
    }

    // Other set is actually empty
    if (begin == end) return;

    // Resize and do actual union
    auto b1 =
      resize(begin * kBlockSize + o.m_base, end * kBlockSize + o.m_base);

    assertx(m_base <= (begin * kBlockSize + o.m_base));
    assertx((m_cap * kBlockSize + m_base) >= (end * kBlockSize + o.m_base));

    auto const prefix = (o.m_base / kBlockSize + begin) - m_base / kBlockSize;
    for (size_t i = begin; i < end; ++i) b1[i - begin + prefix] |= b2[i];
  }

  void swap(IdSet& o) noexcept {
    using std::swap;
    swap(m_bits, o.m_bits);
    swap(m_cap, o.m_cap);
    swap(m_base, o.m_base);
  }

  // Check if a specific id is present.
  bool operator[](uint32_t id) const {
    return
      id >= m_base &&
      id < ((m_cap * kBlockSize) + m_base) &&
      bitvec_test(bits(), id - m_base);
  }
  bool operator[](const Key& k) const { return (*this)[k.id()]; }
  bool operator[](const Key* k) const { return (*this)[k->id()]; }

  // Check if no ids are present.
  bool none() const {
    if (LIKELY(m_cap <= 1)) return !m_bits;
    for (size_t i = 0; i < m_cap; ++i) {
      if (m_bitsptr[i] != 0) return false;
    }
    return true;
  }

  // Invoke f(id) for each id present
  template <class Fun> void forEach(Fun f) const {
    auto b = bits();
    for (size_t i = 0, n = m_cap; i < n; ++i) {
      uint64_t word = b[i];
      uint64_t out;
      while (ffs64(word, out)) {
        assertx(0 <= out && out < 64);
        word &= ~(uint64_t{1} << out);
        f(i * kBlockSize + out + m_base);
      }
    }
  }

  static const constexpr size_t kBlockSize = 64;

private:
  uint64_t* bits() { return m_cap > 1 ? m_bitsptr : &m_bits; }
  const uint64_t* bits() const { return m_cap > 1 ? m_bitsptr : &m_bits; }

  // Ensure that this bitset has enough storage to represent all bits
  // between [min and max), returning the proper storage. The new bits
  // will be initialized to zero.
  uint64_t* resize(uint32_t min, uint32_t max) {
    assertx(max > 0);
    assertx(min < max);

    // Common case: everything already fits
    if (min >= m_base && max <= (m_cap * kBlockSize + m_base)) return bits();

    // Calculate the new base and capacity
    auto const roundedMin = min - (min % kBlockSize);
    auto const base =
      (m_cap > 0) ? std::min<uint32_t>(roundedMin, m_base) : roundedMin;
    auto const newMax = std::max<uint32_t>(max, m_cap * kBlockSize + m_base);
    auto const cap =
      ((newMax + kBlockSize - 1) / kBlockSize) - (base / kBlockSize);

    // The common case check should have already caught the case where
    // we don't need to expand.
    assertx(cap > m_cap);
    if (cap == 1) {
      // We've gone from empty to a single block. Just initialize the
      // storage.
      m_bits = 0;
    } else {
      // Otherwise we need to allocate memory off the heap.
      auto const prefix = (m_cap > 0)
        ? (m_base / kBlockSize - base / kBlockSize)
        : 0;

      if (m_cap <= 1 || prefix > 0) {
        // Allocate new memory, and copy the existing bits over to
        // it. Initialize the new prefix and new suffix to zero.
        auto ptr = (uint64_t*)malloc(cap * sizeof(uint64_t));

        auto oldbits = bits();
        auto const copy = prefix + m_cap;
        for (size_t i = 0; i < prefix; ++i) ptr[i] = 0;
        for (size_t i = prefix; i < copy; ++i) ptr[i] = oldbits[i - prefix];
        for (size_t i = copy; i < cap; ++i) ptr[i] = 0;

        if (m_cap > 1) free(m_bitsptr);
        m_bitsptr = ptr;
      } else {
        // If we already have heap allocated storage, and we're not
        // expanding the front, we can use realloc.
        m_bitsptr = (uint64_t*)realloc(m_bitsptr, cap * sizeof(uint64_t));
        for (size_t i = m_cap; i < cap; ++i) m_bitsptr[i] = 0;
      }
    }

    assertx(base % kBlockSize == 0);
    m_cap = cap;
    m_base = base;
    return bits();
  }

private:
  // Storage. If the bits fit in a single word, we use m_bits,
  // m_bitsptr pointing to malloced memory otherwise.
  union {
    uint64_t m_bits;
    uint64_t* m_bitsptr;
  };
  // Number of allocated blocks. If <= 1, then m_bits is active,
  // m_bitsptr otherwise.
  uint32_t m_cap;
  // Logical start index of the bits. This lets us avoid allocating
  // memory if the ids are clustered but not near zero. Must be a
  // multiple of kBlockSize.
  uint32_t m_base;
};

//////////////////////////////////////////////////////////////////////

template<typename K>
void swap(IdSet<K>& a, IdSet<K>& b) noexcept {
  a.swap(b);
}

//////////////////////////////////////////////////////////////////////

}}

#endif
