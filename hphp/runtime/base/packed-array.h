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

#include <cstddef>
#include <cstdint>
#include <sys/types.h>

#include "hphp/runtime/base/array-common.h"
#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/data-walker.h"
#include "hphp/runtime/base/header-kind.h"
#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/base/sort-flags.h"
#include "hphp/runtime/base/typed-value.h"

#include "hphp/util/type-scan.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct Variant;
struct ArrayData;
struct StringData;
struct MixedArray;
struct APCArray;
struct APCHandle;

//////////////////////////////////////////////////////////////////////

/*
 * Packed arrays are a specialized array layout for vector-like data.  That is,
 * php arrays with zero-based contiguous integer keys, and values of mixed
 * types.  The TypedValues are placed right after the array header.
 */
struct PackedArray final : type_scan::MarkCollectable<PackedArray> {
  static constexpr uint32_t SmallSize = 3;
  // the smallest and largest MM size classes we use for allocating PackedArrays
  static constexpr size_t SmallSizeIndex = 3;
  static constexpr size_t MaxSizeIndex = 121;

  // Used in static_asserts near code that will have to change if/when we
  // disaggregate the TypedValues in PackedArray.
  static constexpr bool stores_typed_values = true;

  static_assert(MaxSizeIndex <= std::numeric_limits<uint8_t>::max(),
                "Size index must fit into 8-bits");

  static void Release(ArrayData*);
  static void ReleaseUncounted(ArrayData*);
  static TypedValue NvGetInt(const ArrayData*, int64_t ki);
  static TypedValue NvGetStr(const ArrayData*, const StringData*);
  static ssize_t NvGetIntPos(const ArrayData*, int64_t k);
  static ssize_t NvGetStrPos(const ArrayData*, const StringData* k);
  static TypedValue GetPosKey(const ArrayData*, ssize_t pos);
  static TypedValue GetPosVal(const ArrayData*, ssize_t pos);
  static ArrayData* SetInt(ArrayData*, int64_t k, TypedValue v);
  static ArrayData* SetIntMove(ArrayData*, int64_t k, TypedValue v);
  static ArrayData* SetStr(ArrayData*, StringData* k, TypedValue v);
  static constexpr auto SetStrMove = &SetStr;
  static bool IsVectorData(const ArrayData*) { return true; }
  static bool ExistsInt(const ArrayData* ad, int64_t k);
  static bool ExistsStr(const ArrayData*, const StringData*);
  static arr_lval LvalInt(ArrayData*, int64_t k);
  static arr_lval LvalStr(ArrayData*, StringData* k);
  static ArrayData* RemoveInt(ArrayData*, int64_t k);
  static ArrayData* RemoveStr(ArrayData*, const StringData* k);
  static ssize_t IterBegin(const ArrayData*);
  static ssize_t IterLast(const ArrayData*);
  static ssize_t IterEnd(const ArrayData*);
  static ssize_t IterAdvance(const ArrayData*, ssize_t pos);
  static ssize_t IterRewind(const ArrayData*, ssize_t pos);
  static ArrayData* Copy(const ArrayData* ad);
  static ArrayData* CopyStatic(const ArrayData*);
  static ArrayData* EscalateForSort(ArrayData*, SortFunction);
  static void Ksort(ArrayData*, int, bool);
  static void Sort(ArrayData*, int, bool);
  static void Asort(ArrayData*, int, bool);
  static bool Uksort(ArrayData*, const Variant&);
  static bool Usort(ArrayData*, const Variant&);
  static bool Uasort(ArrayData*, const Variant&);
  static ArrayData* Append(ArrayData*, TypedValue v);
  static ArrayData* AppendMove(ArrayData*, TypedValue v);
  static ArrayData* Pop(ArrayData*, Variant& value);
  static ArrayData* Prepend(ArrayData*, TypedValue v);
  static ArrayData* ToDVArray(ArrayData*, bool copy);
  static ArrayData* ToHackArr(ArrayData*, bool copy);
  static void OnSetEvalScalar(ArrayData*);

  //////////////////////////////////////////////////////////////////////

  // Layout-specific helpers that work on PHP and Hack packed arrays.
  // We use these helpers in ArrayInit and a few other hot sites where
  // we can guarantee the layout of the array.

  // Exactly like Append, except that it skips the COW check. May grow.
  //  @precondition: !cowCheck
  static ArrayData* AppendInPlace(ArrayData*, TypedValue v);

  // Exactly like LvalInt, except that it skips the bounds check.
  //  @precondition: 0 <= i && i < size
  static tv_lval LvalUncheckedInt(ArrayData*, int64_t i);

  // Appends a new null element and returns an lval to it.
  //  @precondition: !cowCheck
  //  @precondition: size < capacity
  static tv_lval LvalNewInPlace(ArrayData*);

  /////////////////////////////////////////////////////////////////////

  static bool checkInvariants(const ArrayData*);

  static ptrdiff_t entriesOffset();

  static uint32_t capacity(const ArrayData*);
  static size_t heapSize(const ArrayData*);
  static uint16_t packSizeIndexAndAuxBits(uint8_t, uint8_t);

  static void scan(const ArrayData*, type_scan::Scanner&);

  static ArrayData* MakeReserveVArray(uint32_t capacity);
  static ArrayData* MakeReserveVec(uint32_t capacity);

  /*
   * Allocate a PackedArray containing `size' values, in the reverse order of
   * the `values' array. This can only be used to populate the array with cells,
   * not refs.
   *
   * This function takes ownership of the Cells in `values'.
   */
  static ArrayData* MakeVArray(uint32_t size, const TypedValue* values);
  static ArrayData* MakeVec(uint32_t size, const TypedValue* values);

  /*
   * Like MakePacked, but with `values' array in natural (not reversed) order.
   */
  static ArrayData* MakeVArrayNatural(uint32_t size, const TypedValue* values);
  static ArrayData* MakeVecNatural(uint32_t size, const TypedValue* values);

  static ArrayData* MakeUninitializedVArray(uint32_t size);
  static ArrayData* MakeUninitializedVec(uint32_t size);

  static ArrayData* MakeUncounted(
      ArrayData* array, bool withApcTypedValue = false,
      DataWalker::PointerMap* seen = nullptr
  );
  static ArrayData* MakeUncounted(
      ArrayData* array, int, DataWalker::PointerMap* seen = nullptr
  ) = delete;
  static ArrayData* MakeUncounted(
      ArrayData* array, size_t extra, DataWalker::PointerMap* seen = nullptr
  ) = delete;
  static ArrayData* MakeUncountedHelper(ArrayData* array, size_t extra);

  static ArrayData* MakeVecFromAPC(const APCArray* apc, bool isLegacy = false);
  static ArrayData* MakeVArrayFromAPC(const APCArray* apc,
                                      bool isMarked = false);

  static bool VecEqual(const ArrayData* ad1, const ArrayData* ad2);
  static bool VecNotEqual(const ArrayData* ad1, const ArrayData* ad2);
  static bool VecSame(const ArrayData* ad1, const ArrayData* ad2);
  static bool VecNotSame(const ArrayData* ad1, const ArrayData* ad2);

  // Fast iteration
  template <class F, bool inc = true>
  static void IterateV(const ArrayData* arr, F fn);
  template <class F, bool inc = true>
  static void IterateKV(const ArrayData* arr, F fn);
  template <class F>
  static void IterateVNoInc(const ArrayData* arr, F fn);

  // Return a MixedArray with the same elements as this PackedArray.
  // The target type is based on the source: varray -> darray, vec -> dict.
  static MixedArray* ToMixed(ArrayData*);
  static MixedArray* ToMixedCopy(const ArrayData*);
  static MixedArray* ToMixedCopyReserve(const ArrayData*, size_t);

  static size_t capacityToSizeIndex(size_t);

  static constexpr auto SizeIndexOffset = HeaderAuxOffset + 1;
private:
  static uint8_t sizeClass(const ArrayData*);

  static MixedArray* ToMixedHeader(const ArrayData*, size_t);

  static ArrayData* Grow(ArrayData*, bool);
  static ArrayData* PrepareForInsert(ArrayData*, bool);
  static SortFlavor preSort(ArrayData*);

  static ArrayData* MakeReserveImpl(uint32_t, HeaderKind);

  template<bool reverse>
  static ArrayData* MakePackedImpl(uint32_t, const TypedValue*, HeaderKind);

  static void CopyPackedHelper(const ArrayData* adIn, ArrayData* ad);

  static bool VecEqualHelper(const ArrayData*, const ArrayData*, bool);
  static int64_t VecCmpHelper(const ArrayData*, const ArrayData*);

  // By default, this method will inc-ref the value being inserted. If move is
  // true, no refcounting operations will be performed.
  static ArrayData* AppendImpl(ArrayData*, TypedValue v, bool copy,
                               bool move = false);

  struct VecInitializer;
  static VecInitializer s_vec_initializer;

  struct VArrayInitializer;
  static VArrayInitializer s_varr_initializer;

  struct MarkedVecInitializer;
  static MarkedVecInitializer s_marked_vec_initializer;

  struct MarkedVArrayInitializer;
  static MarkedVArrayInitializer s_marked_varr_initializer;
};

//////////////////////////////////////////////////////////////////////

}

