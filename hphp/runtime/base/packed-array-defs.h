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
#include "hphp/runtime/base/typed-value.h"

#include "hphp/util/type-scan.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Return the payload from a ArrayData* that is kPacked/VecKind.
 */
ALWAYS_INLINE
TypedValue* packedData(const ArrayData* arr) {
  return const_cast<TypedValue*>(
    reinterpret_cast<const TypedValue*>(arr + 1)
  );
}

ALWAYS_INLINE
ptrdiff_t PackedArray::entriesOffset() {
  // The JIT calls entriesOffset in code that depends on the TypeValue* layout.
  // There may be other places where the JIT depends on this layout, too.
  static_assert(PackedArray::stores_typed_values);
  return reinterpret_cast<ptrdiff_t>(
    packedData(reinterpret_cast<ArrayData*>(0x0)));
}

/* this only exists to make compilers happy about types in the below macro */
inline constexpr uint32_t sizeClassParams2PackedArrayCapacity(
  size_t index,
  size_t lg_grp,
  size_t lg_delta,
  size_t ndelta
) {
  static_assert(sizeof(ArrayData) <= kSizeIndex2Size[0],
    "this math only works if ArrayData fits in the smallest size class");
  return index <= PackedArray::MaxSizeIndex
    ? (((size_t{1} << lg_grp) + (ndelta << lg_delta)) - sizeof(ArrayData))
      / sizeof(TypedValue)
    : 0;
}

/* We can use uint32_t safely because capacity is in units of array elements,
 * and arrays can't have more than 32 bits worth of elements.
 */
alignas(64) constexpr uint32_t kSizeIndex2PackedArrayCapacity[] = {
#define SIZE_CLASS(index, lg_grp, lg_delta, ndelta, lg_delta_lookup, ncontig) \
  sizeClassParams2PackedArrayCapacity(index, lg_grp, lg_delta, ndelta),
  SIZE_CLASSES
#undef SIZE_CLASS
};

static_assert(
  kSizeIndex2PackedArrayCapacity[PackedArray::SmallSizeIndex]
    == PackedArray::SmallSize,
  "SmallSizeIndex must be MM size class index for array of SmallSize"
);

// MaxSizeIndex corresponds to HashTableCommon::MaxSize - 1 (which is the same
// as MixedArray::MaxSize - 1) because HashTableCommon::MaxSize - 1 exactly fits
// into a MM size class, PackedArray::capacity is a function of MM size class,
// and we can't allow a capacity > MaxSize since most operations only check
// that size <= capacity stays true (and don't explicitly check size <= MaxSize)
static_assert(
  kSizeIndex2PackedArrayCapacity[PackedArray::MaxSizeIndex]
    == array::HashTableCommon::MaxSize - 1,
  "MaxSizeIndex must be the largest possible size class index for PackedArrays"
);

ALWAYS_INLINE
size_t PackedArray::capacityToSizeIndex(size_t cap) {
  if (cap <= PackedArray::SmallSize) {
    return PackedArray::SmallSizeIndex;
  }
  auto const sizeIndex = MemoryManager::size2Index(
    sizeof(ArrayData) + cap * sizeof(TypedValue)
  );
  assertx(sizeIndex <= PackedArray::MaxSizeIndex);
  return sizeIndex;
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
  auto data = packedData(a);
  scanner.scan(*data, a->size() * sizeof(*data));
}

template <class F, bool inc>
void PackedArray::IterateV(const ArrayData* arr, F fn) {
  assertx(checkInvariants(arr));
  if (inc) arr->incRefCount();
  SCOPE_EXIT { if (inc) decRefArr(const_cast<ArrayData*>(arr)); };
  auto const size = arr->m_size;
  for (uint32_t i = 0; i < size; ++i) {
    if (ArrayData::call_helper(fn, GetPosVal(arr, i))) break;
  }
}

template <class F, bool inc>
void PackedArray::IterateKV(const ArrayData* arr, F fn) {
  assertx(checkInvariants(arr));
  if (inc) arr->incRefCount();
  SCOPE_EXIT { if (inc) decRefArr(const_cast<ArrayData*>(arr)); };
  auto const size = arr->m_size;
  for (auto k = make_tv<KindOfInt64>(0); val(k).num < size; ++val(k).num) {
    if (ArrayData::call_helper(fn, k, GetPosVal(arr, val(k).num))) break;
  }
}

template <class F>
ALWAYS_INLINE void PackedArray::IterateVNoInc(const ArrayData* arr, F fn) {
  PackedArray::IterateV<F, false>(arr, std::move(fn));
}

//////////////////////////////////////////////////////////////////////

}

