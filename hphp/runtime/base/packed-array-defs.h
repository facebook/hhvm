/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/cap-code.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

constexpr uint32_t kPackedSmallSize = 3; // same as mixed-array for now

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
ArrayData* getArrayFromPackedData(const TypedValue* tv) {
  return const_cast<ArrayData*>(
    reinterpret_cast<const ArrayData*>(tv) - 1
  );
}

ALWAYS_INLINE
ptrdiff_t PackedArray::entriesOffset() {
  return reinterpret_cast<ptrdiff_t>(
    packedData(reinterpret_cast<ArrayData*>(0x0)));
}

ALWAYS_INLINE
size_t PackedArray::heapSize(const ArrayData* ad) {
  return sizeof(ArrayData) + sizeof(TypedValue) * ad->cap();
}

template<class Marker>
void PackedArray::scan(const ArrayData* a, Marker& mark) {
  assert(checkInvariants(a));
  auto data = packedData(a);
  for (unsigned i = 0, n = a->getSize(); i < n; ++i) {
    mark(data[i]);
  }
}

template <class F, bool inc>
void PackedArray::IterateV(const ArrayData* arr, F fn) {
  assert(checkInvariants(arr));
  auto elm = packedData(arr);
  if (inc) arr->incRefCount();
  SCOPE_EXIT { if (inc) decRefArr(const_cast<ArrayData*>(arr)); };
  for (auto i = arr->m_size; i--; elm++) {
    if (ArrayData::call_helper(fn, elm)) break;
  }
}

template <class F, bool inc>
void PackedArray::IterateKV(const ArrayData* arr, F fn) {
  assert(checkInvariants(arr));
  auto elm = packedData(arr);
  if (inc) arr->incRefCount();
  SCOPE_EXIT { if (inc) decRefArr(const_cast<ArrayData*>(arr)); };
  TypedValue key;
  key.m_data.num = 0;
  key.m_type = KindOfInt64;
  for (auto i = arr->m_size; i--; key.m_data.num++, elm++) {
    if (ArrayData::call_helper(fn, &key, elm)) break;
  }
}

//////////////////////////////////////////////////////////////////////

}

#endif
