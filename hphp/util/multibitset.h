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

#ifndef incl_HPHP_UTIL_MULTIBITSET_H_
#define incl_HPHP_UTIL_MULTIBITSET_H_

#include <cstddef>
#include <cstdint>
#include <limits>
#include <unordered_map>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

/*
 * Like a bitset, but we have N bits at each position instead of 1.
 */
template<size_t N>
struct multibitset {
  /*
   * Create an empty multibitset with no bits allocated.
   */
  multibitset();

  /*
   * Allocate a multibitset with room for `nelms' N-bits, and zero it.
   */
  explicit multibitset(size_t nelms);

  /*
   * Free the allocated bits.
   */
  ~multibitset();

  /*
   * Make room for `nelms' N-bits in the set, shrinking it if `nelms' is
   * smaller than the current size.
   */
  void resize(size_t nelms);

  /*
   * Set all N-bits to zero.
   */
  void reset();

  /////////////////////////////////////////////////////////////////////////////

  /*
   * Reference to an N-bit.
   *
   * Behaves analogously to std::bitset::reference.
   */
  struct reference {
    /* implicit */ operator uint64_t() const;
    reference& operator=(uint64_t);

    friend struct multibitset<N>;

   private:
    reference(multibitset<N>& mbs, size_t pos);

   private:
    multibitset<N>& m_mbs;
    size_t m_word;
    uint8_t m_bit;
  };

  static constexpr size_t npos = std::numeric_limits<uint64_t>::max();

  /////////////////////////////////////////////////////////////////////////////

  /*
   * Access a specific N-bit.
   *
   * This asserts that `pos <= N'; as such, we do not implement a test().
   */
  uint64_t operator[](size_t pos) const;
  reference operator[](size_t pos);

  /*
   * Returns the number of N-bits the multibitset can hold.
   */
  size_t size() const;

  /*
   * Find the position of the first or last N-bit set to a nonzero value,
   * starting at `pos'.  Note that `pos' is an inclusive bound, so `pos' itself
   * may be returned.
   *
   * Returns `npos' if no set N-bit is found.
   */
  size_t ffs(size_t pos = 0) const;
  size_t fls(size_t pos = npos) const;

  /////////////////////////////////////////////////////////////////////////////

private:
  static_assert(N > 0 && N < 64, "must have 0 < N < 64");

private:
  uint64_t* m_bits{nullptr};
  size_t m_size{0};
  size_t m_nwords{0};
};

template<size_t N>
constexpr size_t multibitset<N>::npos;

///////////////////////////////////////////////////////////////////////////////

/*
 * An alternate multibitset interface intended for use patterns with sparse
 * clusters of marked bits.
 *
 * Has roughly the same interface as multibitset, with exceptions noted.
 */
template<size_t N>
struct chunked_multibitset {
  /*
   * Create an empty multibitset.
   *
   * chunked_multibitset automatically expands storage as bits are set.  Thus,
   * it does not accept a size restriction; instead, it takes a chunk size.
   */
  explicit chunked_multibitset(size_t chunk_sz);

  /////////////////////////////////////////////////////////////////////////////

  struct reference {
    /* implicit */ operator uint64_t() const;
    reference& operator=(uint64_t);

    friend struct chunked_multibitset<N>;

   private:
    reference(chunked_multibitset<N>& cmbs, size_t pos);

   private:
    chunked_multibitset<N>& m_cmbs;
    size_t m_pos;
  };

  static constexpr size_t npos = std::numeric_limits<uint64_t>::max();

  /////////////////////////////////////////////////////////////////////////////

  void reset();

  /*
   * Return a reference to an N-bit.
   *
   * Setting an N-bit may cause new chunks to be allocated.
   */
  uint64_t operator[](size_t pos) const;
  reference operator[](size_t pos);

  /*
   * Find set bit; see multibitset::f{f,l}s().
   *
   * The time complexity of these routines is linearly bounded by the number of
   * chunks in which bits have been touched during the lifetime of the object.
   * No other guarantees are made.
   */
  size_t ffs(size_t pos = 0) const;
  size_t fls(size_t pos = npos) const;

  /////////////////////////////////////////////////////////////////////////////

private:
  static_assert(N > 0 && N <= 64, "must have 0 < N <= 64");

  multibitset<N>& chunk_for(size_t pos);

private:
  std::unordered_map<size_t,multibitset<N>> m_chunks;
  const size_t m_chunk_sz;
  size_t m_highwater{0};
  size_t m_lowwater{npos};
};

template<size_t N>
constexpr size_t chunked_multibitset<N>::npos;

///////////////////////////////////////////////////////////////////////////////

}

#include "hphp/util/multibitset-inl.h"

#endif
