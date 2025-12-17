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

#include <bitset>
#include <cstddef>
#include <cstdint>

#include <folly/sorted_vector_types.h>

#include "hphp/util/bitset-utils.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/**
 * SparseBitset - A memory-efficient bitset for sparse data
 *
 * This is a space-optimized bitset implementation designed for storing large,
 * sparse sets of integers. Instead of allocating a full bitset for the entire
 * range, it divides the space into chunks and only allocates storage for chunks
 * that contain at least one set bit.
 *
 * The implementation uses a sorted vector of chunks, where each chunk represents
 * a fixed-size range of integers (default 256). This provides good performance
 * characteristics:
 * - O(log n) lookup, insertion, and deletion where n is the number of chunks
 * - O(1) amortized insertion when adding consecutive values
 * - Memory usage proportional to the number of set bits, not the range
 *
 * Performance considerations:
 * - For best cache locality, organize your data so frequently-accessed elements
 *   have the lowest indices. Keeping hot elements clustered in lower index
 *   ranges can improve performance by reducing the working set of chunks.
 * - The chunk vector is stored sequentially, so earlier chunks (lower indices)
 *   have slightly better cache performance during iteration.
 *
 * Template Parameters:
 *   ChunkSize - The number of bits per chunk (default 256). Must be > 0.
 *               Larger chunks use less memory overhead but may waste more space
 *               for very sparse data.
 *
 * Example usage:
 *   SparseBitset<> bitset;
 *   bitset.add(100);
 *   bitset.add(1000000);  // No need to allocate ~1M bits
 *   if (bitset.contains(100)) { ... }
 *   bitset.forEach([](uint64_t x) { std::cout << x << "\n"; });
 */
template <size_t ChunkSize = 256>
struct SparseBitset {
  static_assert(ChunkSize > 0);

  SparseBitset() = default;
  SparseBitset(const SparseBitset&) = default;
  SparseBitset(SparseBitset&&) = default;

  SparseBitset& operator=(const SparseBitset&) = default;
  SparseBitset& operator=(SparseBitset&&) = default;

  /**
   * Add a value to the bitset.
   * Time complexity: O(log n) where n is the number of chunks.
   */
  void add(uint64_t x);

  /**
   * Remove a value from the bitset.
   * If the value isn't present, this is a no-op.
   * Time complexity: O(log n) where n is the number of chunks.
   */
  void remove(uint64_t x);

  /**
   * Check if a value is in the bitset.
   * Time complexity: O(log n) where n is the number of chunks.
   */
  bool contains(uint64_t x) const;

  /**
   * Return the number of set bits in the bitset.
   * Time complexity: O(n * ChunkSize) where n is the number of chunks.
   */
  size_t size() const;

  /**
   * Check if any bits are set.
   * Time complexity: O(1)
   */
  bool any() const;

  /**
   * Check if no bits are set.
   * Time complexity: O(1)
   */
  bool none() const;

  /**
   * Check if no bits are set (STL-style alias for none()).
   * Time complexity: O(1)
   */
  bool empty() const;

  /**
   * Remove all bits from the bitset.
   * Time complexity: O(n) where n is the number of chunks.
   */
  void clear();

  /**
   * Swap contents with another bitset.
   * Time complexity: O(1)
   */
  void swap(SparseBitset& o);

  /**
   * Iterate over all set bits, calling f(x) for each.
   * Bits are visited in ascending order.
   * Time complexity: O(k) where k is the number of set bits.
   */
  template <typename F>
  void forEach(F&& f) const;

  /**
   * Iterate over the intersection of this bitset and another,
   * calling f(x) for each bit set in both.
   * Bits are visited in ascending order.
   * Time complexity: O(n + m) where n and m are the number of chunks.
   */
  template <typename F>
  void forEachIsect(const SparseBitset& o, F&& f) const;

  /**
   * Check if this bitset is a subset of another.
   * Returns true if every bit set in this bitset is also set in o.
   * Time complexity: O(n + m) where n and m are the number of chunks.
   */
  bool isSubsetOf(const SparseBitset& o) const;

  /**
   * Compute the intersection (bitwise AND) of two bitsets.
   * Time complexity: O(n + m) where n and m are the number of chunks.
   */
  SparseBitset operator&(const SparseBitset& o) const;

  /**
   * Compute the union (bitwise OR) of two bitsets.
   * Time complexity: O(n + m) where n and m are the number of chunks.
   */
  SparseBitset operator|(const SparseBitset& o) const;

  /**
   * Compute the intersection in-place.
   * Time complexity: O(n + m) where n and m are the number of chunks.
   */
  SparseBitset& operator&=(const SparseBitset& o);

  /**
   * Compute the union in-place.
   * Time complexity: O(n + m) where n and m are the number of chunks.
   */
  SparseBitset& operator|=(const SparseBitset& o);

  /**
   * Equality comparison.
   */
  bool operator==(const SparseBitset& o) const;

  /**
   * Inequality comparison.
   */
  bool operator!=(const SparseBitset& o) const;

private:
  // A chunk represents a contiguous range of ChunkSize bits.
  // The prefix identifies which range (prefix * ChunkSize to
  // (prefix + 1) * ChunkSize - 1), and bits stores which values
  // in that range are set.
  struct Chunk {
    uint64_t prefix;
    std::bitset<ChunkSize> bits;
    bool operator==(const Chunk& o) const {
      return
        prefix == o.prefix &&
        bits == o.bits;
    }
  };

  // Comparator for chunks that supports transparent lookup by prefix.
  struct ChunkLT {
    using is_transparent = void;
    bool operator()(const Chunk& c1, const Chunk& c2) const {
      return c1.prefix < c2.prefix;
    }
    bool operator()(const Chunk& c, uint64_t p) const { return c.prefix < p; }
    bool operator()(uint64_t p, const Chunk& c) const { return p < c.prefix; }
  };

  using ChunkVector = folly::sorted_vector_set<Chunk, ChunkLT>;

  explicit SparseBitset(ChunkVector v) : chunks{std::move(v)} {}

  ChunkVector chunks;
};

//////////////////////////////////////////////////////////////////////

}

#include "hphp/util/sparse-bitset-inl.h"
