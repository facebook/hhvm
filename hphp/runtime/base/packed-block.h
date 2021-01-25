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
#ifndef incl_HPHP_PACKED_BLOCK_H_
#define incl_HPHP_PACKED_BLOCK_H_

#include <cstddef>
#include <cstdint>
#include <sys/types.h>

#include "hphp/runtime/base/array-common.h"
#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/data-walker.h"
#include "hphp/runtime/base/header-kind.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/sort-flags.h"
#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/base/typed-value.h"

#include "hphp/util/type-scan.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * If PackedArray::stores_typed_values is false, the entries of a PackedArray
 * will be stored in a PackedBlock format. Each block consists of 8 DataTypes
 * followed by 8 Values, for a total size of 72 bytes.
 *
 * The extra types and values in the last PackedBlock are uninitialized.
 *
 * The easiest way to access entries in this layout is to use LvalAt. However,
 * if you are iterating over the array, you may want to use a pair of nested
 * loops to iterate first over blocks and second over items in the block.
 */
struct PackedBlock final {
  static_assert(sizeof(Value) == 8);
  static_assert(sizeof(DataType) == 1);
  static constexpr size_t kNumItems = 8;
  static constexpr size_t kByteSize = kNumItems * 9;

  // Each block contains up to 8 items, so i must be less than 8 here.
  tv_lval operator[](size_t i);
  bool operator<(const PackedBlock& other);

  // Advance to the next block.
  PackedBlock& operator++();
  PackedBlock operator+(int64_t i);
  PackedBlock& operator+=(int64_t i);

  // For the fast type predicates, n must be the number of items in the block.
  // In particular, it must be greater than 0 and less than or equal to 8.
  bool AllTypesEqual(size_t n);
  bool AllTypesMatch(size_t n, DataType expected);
  bool AllTypesAreUncounted(size_t n);

  // Get a pointer to a given block by index in a packed array.
  static PackedBlock BlockAt(ArrayData* ad, size_t b);
  static tv_lval LvalAt(ArrayData* ad, size_t k);

  // Get the type and data offsets for an known index. Used by the JIT.
  static PackedArray::EntryOffset EntryOffset(size_t i);

  // Turn a pointer in an array into the index it points to. -1 on failure.
  static int64_t PointerToIndex(const ArrayData* ad, const void* ptr);

  // Always points to the 8-byte type block at the start of a block.
  Value* m_base;
};

//////////////////////////////////////////////////////////////////////

constexpr uint64_t kTypecheckEqualityMasks[8] = {
  0x0,
  0x11,
  0x1111,
  0x111111,
  0x11111111,
  0x1111111111,
  0x111111111111,
  0x11111111111111,
};

constexpr uint64_t kTypecheckRefcountMasks[8] = {
  0x01,
  0x0101,
  0x010101,
  0x01010101,
  0x0101010101,
  0x010101010101,
  0x01010101010101,
  0x0101010101010101,
};

ALWAYS_INLINE tv_lval PackedBlock::operator[](size_t i) {
  assertx(i < kNumItems);
  auto* type = reinterpret_cast<DataType*>(this->m_base);
  return tv_lval(type + i, this->m_base + i + 1);
}

ALWAYS_INLINE bool PackedBlock::operator<(const PackedBlock& other) {
  return this->m_base < other.m_base;
}

ALWAYS_INLINE PackedBlock& PackedBlock::operator++() {
  return *this += 1;
}

ALWAYS_INLINE PackedBlock PackedBlock::operator+(int64_t i) {
  return PackedBlock { this->m_base + 9 * i };
}

ALWAYS_INLINE PackedBlock& PackedBlock::operator+=(int64_t i) {
  this->m_base += 9 * i;
  return *this;
}

ALWAYS_INLINE bool PackedBlock::AllTypesEqual(size_t n) {
  assertx(0 < n && n <= kNumItems);
  auto const types = *reinterpret_cast<uint64_t*>(this->m_base);
  return ((types ^ (types >> 8)) & kTypecheckEqualityMasks[n - 1]) == 0;
}

ALWAYS_INLINE bool PackedBlock::AllTypesMatch(size_t n, DataType type) {
  assertx(0 < n && n <= kNumItems);
  return AllTypesEqual(n) && *reinterpret_cast<DataType*>(m_base) == type;
}

ALWAYS_INLINE bool PackedBlock::AllTypesAreUncounted(size_t n) {
  assertx(0 < n && n <= kNumItems);
  auto const types = *reinterpret_cast<uint64_t*>(this->m_base);
  return (types & kTypecheckRefcountMasks[n - 1]) == 0;
}

ALWAYS_INLINE PackedBlock PackedBlock::BlockAt(ArrayData* ad, size_t b) {
  assertx(!PackedArray::stores_typed_values);
  return PackedBlock { reinterpret_cast<Value*>(ad + 1) } + b;
}

ALWAYS_INLINE tv_lval PackedBlock::LvalAt(ArrayData* ad, size_t k) {
  // Given an index k = x + y, where y = k % 8, we want to index into ad at:
  //   type: ad + PackedArray::entriesOffset + 9 * x + y
  //   data: ad + PackedArray::entriesOffset + 9 * x + 8 * y + 8
  //
  // One way to compute these offsets is to compute the "base block pointer"
  // at ad + PackedArray::entriesOffset + 9 * x, then use that to compute the
  // two pointers. This approach uses 6 arithmetic instructions:
  //
  //   x = k & -8          # AND
  //   y = k & 7           # AND
  //   z = x + 8 * x       # LEA
  //   w = ad + z + eo     # LEA
  //   t = w + y           # ADD
  //   d = w + 8 * y + 8   # LEA
  //
  // We can save an instruction by avoiding computing y, as follows:
  //
  //   x = k & -8          # AND
  //   a = ad + 8 * x + eo # LEA
  //   b = ad + 8 * k + eo # LEA
  //   t = a + k           # ADD
  //   d = b + x + 8       # LEA
  //
  // This method implements the optimization above and checks it against the
  // simpler computation using the PackedBlock API.
  //
  assertx(!PackedArray::stores_typed_values);
  auto const x = k & size_t(-8);
  auto const a = reinterpret_cast<char*>(ad + 1) + 8 * x;
  auto const b = reinterpret_cast<char*>(ad + 1) + 8 * k;
  auto const lval = tv_lval(
    reinterpret_cast<DataType*>(a + k),
    reinterpret_cast<Value*>(b + x + 8)
  );
  assertx(lval == PackedBlock::BlockAt(ad, k / 8)[k & 7]);
  return lval;
}

PackedArray::EntryOffset PackedBlock::EntryOffset(size_t i) {
  assertx(!PackedArray::stores_typed_values);
  auto const div = i / kNumItems;
  auto const mod = i % kNumItems;
  auto const base = sizeof(ArrayData) + kByteSize * div;
  return {ptrdiff_t(base + sizeof(DataType) * mod),
          ptrdiff_t(base + sizeof(DataType) * kNumItems + sizeof(Value) * mod)};
}

int64_t PackedBlock::PointerToIndex(const ArrayData* ad, const void* ptr) {
  // We have three cases to handle here:
  //  1. ptr is before the first array entry.
  //  2. ptr points into some block's 8-byte DataType prefix.
  //  2. ptr points into some block's 64-byte Value suffix.
  assertx(!PackedArray::stores_typed_values);
  auto const base = reinterpret_cast<const char*>(ad + 1);
  auto const diff = reinterpret_cast<const char*>(ptr) - base;
  auto const div = diff / kByteSize;
  auto const mod = diff % kByteSize;
  return diff < 0 ? -1 : mod < 8 ? 8 * div + mod : 8 * div + mod / 8 - 1;
}

} // namespace HPHP

#endif
