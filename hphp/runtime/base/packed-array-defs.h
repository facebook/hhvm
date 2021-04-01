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

#include "hphp/runtime/base/packed-array.h"

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/hash-table.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/typed-value.h"

#include "hphp/util/type-scan.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

// This method does the actual logic for computing a size given a layout,
// using `stores_typed_values` to select between layout types.
constexpr size_t bytes2PackedArrayCapacity(size_t bytes) {
  if (PackedArray::stores_typed_values) {
    return bytes / sizeof(TypedValue);
  } else {
    static_assert(sizeof(Value) == 8);
    static_assert(sizeof(DataType) == 1);
    const size_t values = bytes / sizeof(Value);
    return values / 9 * 8 + std::max(int8_t(values % 9) - 1, 0);
  }
}

// This method is just to help compilers with types in the macro usage below.
// It also checks that the result of the arithmetic above fits in a uint32_t;
// if this check fails, it probably means that MaxSizeIndex must be adjusted.
constexpr uint32_t sizeClassParams2PackedArrayCapacity(
  size_t index,
  size_t lg_grp,
  size_t lg_delta,
  size_t ndelta
) {
  static_assert(sizeof(ArrayData) <= kSizeIndex2Size[0],
    "This math only works if ArrayData fits in the smallest size class.");
  if (index > PackedArray::MaxSizeIndex) return 0;
  const size_t total = ((size_t{1} << lg_grp) + (ndelta << lg_delta));
  const size_t capacity = bytes2PackedArrayCapacity(total - sizeof(ArrayData));
  return capacity <= std::numeric_limits<uint32_t>::max()
    ? capacity
    : throw std::length_error("capacity > uint32_t::max");
}

// We can use uint32_t safely because capacity is in units of array elements,
// and an ArrayData's m_size field is a uint32_t. See also the guard above.
alignas(64) constexpr uint32_t kSizeIndex2PackedArrayCapacity[] = {
#define SIZE_CLASS(index, lg_grp, lg_delta, ndelta, lg_delta_lookup, ncontig) \
  sizeClassParams2PackedArrayCapacity(index, lg_grp, lg_delta, ndelta),
  SIZE_CLASSES
#undef SIZE_CLASS
};

// This assertion lets us allocate by index and assume we know the capacity.
static_assert(
  kSizeIndex2PackedArrayCapacity[PackedArray::SmallSizeIndex]
    == PackedArray::SmallSize,
  "SmallSizeIndex must be MM size class index for array of SmallSize"
);

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE TypedValue* PackedArray::entries(ArrayData* arr) {
  assertx(stores_typed_values);
  return reinterpret_cast<TypedValue*>(arr + 1);
}

ALWAYS_INLINE ptrdiff_t PackedArray::entriesOffset() {
  return sizeof(ArrayData);
}

ALWAYS_INLINE
size_t PackedArray::capacityToSizeBytes(size_t capacity) {
  const auto base = sizeof(ArrayData);
  if constexpr (stores_typed_values) {
    return base + capacity * sizeof(TypedValue);
  }
  // When we use the PackedBlock layout, we can store each entry in 9 bytes
  // (1 byte for the type and 8 bytes for the value), but we must round up to
  // a multiple of 8 to handle the leftover types.
  //
  // We round to up to a multiple of 16 since our allocations are aligned.
  auto const size = base + capacity * (sizeof(DataType) + sizeof(Value));
  return (size + 15) & size_t(-16);
}

ALWAYS_INLINE
size_t PackedArray::capacityToSizeIndex(size_t capacity) {
  if (capacity <= PackedArray::SmallSize) {
    return PackedArray::SmallSizeIndex;
  }
  const auto index = MemoryManager::size2Index(capacityToSizeBytes(capacity));
  assertx(index <= PackedArray::MaxSizeIndex);
  return index;
}

ALWAYS_INLINE
uint32_t PackedArray::capacity(const ArrayData* ad) {
  return kSizeIndex2PackedArrayCapacity[sizeClass(ad)];
}

ALWAYS_INLINE
size_t PackedArray::heapSize(const ArrayData* ad) {
  return kSizeIndex2Size[sizeClass(ad)];
}

// Pack together the size class and the other aux bits into a single 16-bit
// number which can be stored in the HeapObject object. ArrayData requires the
// varray/darray state to be in the lower 8-bits, but we're free to use the
// upper.
ALWAYS_INLINE
uint16_t PackedArray::packSizeIndexAndAuxBits(uint8_t idx, uint8_t aux) {
  return (static_cast<uint16_t>(idx) << 8) | aux;
}

ALWAYS_INLINE
uint8_t PackedArray::sizeClass(const ArrayData* ad) {
  return ad->m_aux16 >> 8;
}

inline void PackedArray::scan(const ArrayData* a, type_scan::Scanner& scanner) {
  assertx(checkInvariants(a));
  if constexpr (stores_typed_values) {
    const auto* data = PackedArray::entries(const_cast<ArrayData*>(a));
    scanner.scan(*data, a->size() * sizeof(*data));
  } else {
    for (uint32_t i = 0; i < a->size(); ++i) {
      const auto lval = LvalUncheckedInt(const_cast<ArrayData*>(a), i);
      if (isRefcountedType(type(lval))) scanner.scan(val(lval).pcnt);
    }
  }
}

template <class F>
void PackedArray::IterateV(const ArrayData* arr, F fn) {
  assertx(checkInvariants(arr));
  auto const size = arr->m_size;
  for (uint32_t i = 0; i < size; ++i) {
    if (ArrayData::call_helper(fn, GetPosVal(arr, i))) break;
  }
}

template <class F>
void PackedArray::IterateKV(const ArrayData* arr, F fn) {
  assertx(checkInvariants(arr));
  auto const size = arr->m_size;
  for (auto k = make_tv<KindOfInt64>(0); val(k).num < size; ++val(k).num) {
    if (ArrayData::call_helper(fn, k, GetPosVal(arr, val(k).num))) break;
  }
}

//////////////////////////////////////////////////////////////////////

}

