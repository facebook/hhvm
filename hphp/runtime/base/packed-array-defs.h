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
#ifndef incl_HPHP_PACKED_ARRAY_DEFS_H_
#define incl_HPHP_PACKED_ARRAY_DEFS_H_

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
uint32_t PackedArray::capacity(const ArrayData* ad) {
  return kSizeIndex2PackedArrayCapacity[ad->m_aux16];
}

ALWAYS_INLINE
size_t PackedArray::heapSize(const ArrayData* ad) {
  return kSizeIndex2Size[ad->m_aux16];
}

inline void PackedArray::scan(const ArrayData* a, type_scan::Scanner& scanner) {
  assert(checkInvariants(a));
  auto data = packedData(a);
  scanner.scan(*data, a->getSize() * sizeof(*data));
}

template <class F, bool inc>
void PackedArray::IterateV(const ArrayData* arr, F fn) {
  assert(checkInvariants(arr));
  auto elm = packedData(arr);
  if (inc) arr->incRefCount();
  SCOPE_EXIT { if (inc) decRefArr(const_cast<ArrayData*>(arr)); };
  for (auto i = arr->m_size; i--; elm++) {
    if (ArrayData::call_helper(fn, *elm)) break;
  }
}

template <class F, bool inc>
void PackedArray::IterateKV(const ArrayData* arr, F fn) {
  assert(checkInvariants(arr));
  auto elm = packedData(arr);
  if (inc) arr->incRefCount();
  SCOPE_EXIT { if (inc) decRefArr(const_cast<ArrayData*>(arr)); };
  auto key = make_tv<KindOfInt64>(0);
  for (auto i = arr->m_size; i--; key.m_data.num++, elm++) {
    if (ArrayData::call_helper(fn, key, *elm)) break;
  }
}

//////////////////////////////////////////////////////////////////////

}

#endif
