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

namespace HPHP {

//////////////////////////////////////////////////////////////////////

template <size_t ChunkSize>
void SparseBitset<ChunkSize>::add(uint64_t x) {
  auto const prefix = x / ChunkSize;
  auto it = chunks.lower_bound(prefix);
  if (it == end(chunks) || it->prefix != prefix) {
    it = chunks.emplace_hint(it, prefix);
  }
  it->bits.set(x % ChunkSize);
}

template <size_t ChunkSize>
void SparseBitset<ChunkSize>::remove(uint64_t x) {
  auto it = chunks.find(x / ChunkSize);
  if (it == end(chunks)) return;
  it->bits.reset(x % ChunkSize);
  // Remove empty chunks to maintain sparsity
  if (it->bits.none()) chunks.erase(it);
}

template <size_t ChunkSize>
bool SparseBitset<ChunkSize>::contains(uint64_t x) const {
  auto it = chunks.find(x / ChunkSize);
  return (it != end(chunks) && it->bits.test(x % ChunkSize));
}

template <size_t ChunkSize>
size_t SparseBitset<ChunkSize>::size() const {
  size_t s = 0;
  for (auto const& c : chunks) s += c.bits.count();
  return s;
}

template <size_t ChunkSize>
bool SparseBitset<ChunkSize>::any() const {
  return !chunks.empty();
}

template <size_t ChunkSize>
bool SparseBitset<ChunkSize>::none() const {
  return chunks.empty();
}

template <size_t ChunkSize>
bool SparseBitset<ChunkSize>::empty() const {
  return chunks.empty();
}

template <size_t ChunkSize>
void SparseBitset<ChunkSize>::clear() {
  chunks.clear();
}

template <size_t ChunkSize>
void SparseBitset<ChunkSize>::swap(SparseBitset& o) {
  chunks.swap(o.chunks);
}

template <size_t ChunkSize>
template <typename F>
void SparseBitset<ChunkSize>::forEach(F&& f) const {
  for (auto const& c : chunks) {
    bitset_for_each_set(
      c.bits,
      [&] (uint64_t b) { f(b + (c.prefix * ChunkSize)); }
    );
  }
}

template <size_t ChunkSize>
template <typename F>
void SparseBitset<ChunkSize>::forEachIsect(const SparseBitset& o, F&& f) const {
  // Two-pointer merge algorithm: advance through both sorted chunk lists,
  // processing only chunks with matching prefixes (intersection)
  auto it1 = begin(chunks);
  auto it2 = begin(o.chunks);
  auto const end1 = end(chunks);
  auto const end2 = end(o.chunks);

  while (it1 != end1 && it2 != end2) {
    auto const& c1 = *it1;
    auto const& c2 = *it2;
    if (c1.prefix < c2.prefix) {
      ++it1; // Chunk only in this set, skip it
    } else if (c1.prefix > c2.prefix) {
      ++it2; // Chunk only in other set, skip it
    } else {
      // Matching chunk: iterate bits in the intersection
      bitset_for_each_set(
        c1.bits & c2.bits,
        [&] (uint64_t b) { f(b + (c1.prefix * ChunkSize)); }
      );
      ++it1;
      ++it2;
    }
  }
}

template <size_t ChunkSize>
bool SparseBitset<ChunkSize>::isSubsetOf(const SparseBitset& o) const {
  // Two-pointer merge: verify every chunk in this set has a corresponding
  // superset chunk in o
  auto it1 = begin(chunks);
  auto it2 = begin(o.chunks);
  auto const end1 = end(chunks);
  auto const end2 = end(o.chunks);

  while (it1 != end1 && it2 != end2) {
    auto const& c1 = *it1;
    auto const& c2 = *it2;
    if (c1.prefix < c2.prefix) {
      // This chunk has no match in o, so this isn't a subset
      return false;
    } else if (c1.prefix > c2.prefix) {
      ++it2; // Skip chunks in o that we don't have
    } else {
      // Verify all bits in c1 are also set in c2
      if ((c1.bits & c2.bits) != c1.bits) return false;
      ++it1;
      ++it2;
    }
  }

  // If we have any remaining chunks, this isn't a subset
  return it1 == end1;
}

template <size_t ChunkSize>
SparseBitset<ChunkSize>
SparseBitset<ChunkSize>::operator&(const SparseBitset& o) const {
  // Two-pointer merge: build output containing only chunks that exist in both
  auto it1 = begin(chunks);
  auto it2 = begin(o.chunks);
  auto const end1 = end(chunks);
  auto const end2 = end(o.chunks);

  typename ChunkVector::container_type out;
  out.reserve(std::min(chunks.size(), o.chunks.size()));

  while (it1 != end1 && it2 != end2) {
    auto const& c1 = *it1;
    auto const& c2 = *it2;
    if (c1.prefix < c2.prefix) {
      ++it1;
    } else if (c1.prefix > c2.prefix) {
      ++it2;
    } else {
      // Matching chunks: compute bitwise AND
      out.emplace_back(c1.prefix);
      out.back().bits = c1.bits & c2.bits;
      // Remove the chunk if the intersection is empty
      if (out.back().bits.none()) out.pop_back();
      ++it1;
      ++it2;
    }
  }
  return SparseBitset{ChunkVector{folly::sorted_unique_t{}, std::move(out)}};
}

template <size_t ChunkSize>
SparseBitset<ChunkSize>
SparseBitset<ChunkSize>::operator|(const SparseBitset& o) const {
  // Two-pointer merge: build output containing all chunks from both sets
  auto it1 = begin(chunks);
  auto it2 = begin(o.chunks);
  auto const end1 = end(chunks);
  auto const end2 = end(o.chunks);

  typename ChunkVector::container_type out;
  out.reserve(std::max(chunks.size(), o.chunks.size()));

  while (it1 != end1 && it2 != end2) {
    auto const& c1 = *it1;
    auto const& c2 = *it2;
    if (c1.prefix < c2.prefix) {
      out.emplace_back(c1);
      ++it1;
    } else if (c1.prefix > c2.prefix) {
      out.emplace_back(c2);
      ++it2;
    } else {
      // Matching chunks: compute bitwise OR
      out.emplace_back(c1.prefix);
      out.back().bits = c1.bits | c2.bits;
      ++it1;
      ++it2;
    }
  }

  // Append any remaining chunks from either set
  if (it1 != end1) out.insert(end(out), it1, end1);
  if (it2 != end2) out.insert(end(out), it2, end2);
  return SparseBitset{ChunkVector{folly::sorted_unique_t{}, std::move(out)}};
}

template <size_t ChunkSize>
SparseBitset<ChunkSize>&
SparseBitset<ChunkSize>::operator&=(const SparseBitset& o) {
  if (this == &o) return *this;

  // In-place intersection using a two-pointer merge with compaction.
  // We maintain two iterators: 'it1' scans our chunks, and 'insert' marks
  // where to write the next non-empty result chunk. This allows us to
  // modify our chunk list in-place without allocating a new vector.
  auto it1 = begin(chunks);
  auto it2 = begin(o.chunks);
  auto const end1 = end(chunks);
  auto const end2 = end(o.chunks);

  auto insert = begin(chunks);
  while (it1 != end1 && it2 != end2) {
    auto& c1 = *it1;
    auto const& c2 = *it2;
    if (c1.prefix < c2.prefix) {
      ++it1; // Chunk only in this set, skip (not in intersection)
    } else if (c1.prefix > c2.prefix) {
      ++it2; // Chunk only in other set, skip
    } else {
      // Matching chunks: compute intersection
      if (insert == it1) {
        // Optimization: reuse current chunk in-place
        c1.bits &= c2.bits;
        if (c1.bits.any()) ++insert;
      } else {
        // Move result to earlier position in chunk list
        insert->prefix = c1.prefix;
        insert->bits = c1.bits & c2.bits;
        if (insert->bits.any()) ++insert;
      }
      ++it1;
      ++it2;
    }
  }
  // Erase any chunks past the insertion point
  chunks.erase(insert, end(chunks));

  return *this;
}

template <size_t ChunkSize>
SparseBitset<ChunkSize>&
SparseBitset<ChunkSize>::operator|=(const SparseBitset& o) {
  if (this == &o) return *this;
  *this = *this | o;
  return *this;
}

template <size_t ChunkSize>
bool SparseBitset<ChunkSize>::operator==(const SparseBitset& o) const {
  return chunks == o.chunks;
}

template <size_t ChunkSize>
bool SparseBitset<ChunkSize>::operator!=(const SparseBitset& o) const {
  return chunks != o.chunks;
}

//////////////////////////////////////////////////////////////////////

}

// Specialize std::swap for better performance
namespace std {
template <size_t ChunkSize>
void swap(HPHP::SparseBitset<ChunkSize>& a, HPHP::SparseBitset<ChunkSize>& b) {
  a.swap(b);
}
}
