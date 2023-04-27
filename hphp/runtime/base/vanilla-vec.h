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
#include "hphp/runtime/base/sort-flags.h"
#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/unaligned-typed-value.h"

#include "hphp/runtime/vm/jit/vasm-emit.h"
#include "hphp/util/type-scan.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct Variant;
struct ArrayData;
struct StringData;
struct VanillaDict;
struct APCArray;
struct APCHandle;

//////////////////////////////////////////////////////////////////////

/*
 * VanillaVec is the main array layout for vecs, backing a vector-like
 * array with arbitrary values. We support two layouts for VanillaVec:
 *
 *  - If "stores_unaligned_typed_values" is true, it's an array of UnalignedTypedValue.
 *    Each UnalignedTypedValue takes up 9 bytes, with the 8 byte value immediately
 *    followed by the associated 1 byte type; the next UnalignedTypedValue starts
 *    at the next byte, etc.
 *
 *  - Otherwise, it's stored as an array of PackedBlock "8-up" blocks.
 *    Each block stores 8 types, then 8 values, in a total of 72 bytes.
 *
 * Using PackedBlock saves memory but increases the cost, in instructions,
 * of accessing an array element: first we need to find the block, then
 * the element with the block. Layout choice is a compile-time switch.
 */
struct VanillaVec final : type_scan::MarkCollectable<VanillaVec> {
  // If false, use the "8 type bytes / 8 value words" chunked layout.
  // If true, stored 9 byte unaligned typed values.
  static constexpr bool stores_unaligned_typed_values =
    (arch() == Arch::X64 || arch() == Arch::ARM);

  // The default capacity of PackedLayout and unaligned type values, used if capacity = 0.
  static constexpr uint32_t SmallSize = 5;

  // The smallest and largest MM size classes we use for PackedLayouts.
  static constexpr size_t SmallSizeIndex = 3;
  static constexpr size_t MaxSizeIndex = 119;

  static_assert(MaxSizeIndex <= std::numeric_limits<uint8_t>::max(),
                "Size index must fit into 8-bits");

  static void Release(ArrayData*);
  static void ReleaseUncounted(ArrayData*);
  static TypedValue NvGetInt(const ArrayData*, int64_t ki);
  static TypedValue NvGetStr(const ArrayData*, const StringData*);
  static TypedValue GetPosKey(const ArrayData*, ssize_t pos);
  static TypedValue GetPosVal(const ArrayData*, ssize_t pos);
  static bool PosIsValid(const ArrayData*, ssize_t pos);
  static ArrayData* SetIntMove(ArrayData*, int64_t k, TypedValue v);
  static ArrayData* SetStrMove(ArrayData*, StringData* k, TypedValue v);
  static bool IsVectorData(const ArrayData*) { return true; }
  static bool ExistsInt(const ArrayData* ad, int64_t k);
  static bool ExistsStr(const ArrayData*, const StringData*);
  static arr_lval LvalInt(ArrayData*, int64_t k);
  static arr_lval LvalStr(ArrayData*, StringData* k);
  static ArrayData* RemoveIntMove(ArrayData*, int64_t k);
  static ArrayData* RemoveStrMove(ArrayData*, const StringData* k);
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
  static ArrayData* AppendMove(ArrayData*, TypedValue v);
  static ArrayData* PopMove(ArrayData*, Variant& value);
  static ArrayData* Prepend(ArrayData*, TypedValue v);
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

  // This method can only be called if `stores_unaligned_typed_values` is true.
  static UnalignedTypedValue* entries(ArrayData*);

  // This method can be called for any layout, to get a layout start offset.
  static ptrdiff_t entriesOffset();

  // This method can be called for any layout, to get diffs for a known index.
  struct EntryOffset {
    ptrdiff_t type_offset;
    ptrdiff_t data_offset;
  };
  static EntryOffset entryOffset(size_t i);

  static uint32_t capacity(const ArrayData*);
  static uint16_t packSizeIndexAndAuxBits(uint8_t, uint8_t);

  static void scan(const ArrayData*, type_scan::Scanner&);

  static ArrayData* MakeReserveVec(uint32_t capacity);

  /*
   * Allocate a VanillaVec containing `size' values, in the reverse order of
   * the `values' array. This function takes ownership of the input `values',
   * moving refcounts rather than doing refcounting ops.
   */
  static ArrayData* MakeVec(uint32_t size, const TypedValue* values);

  /*
   * Like MakePacked, but with `values' array in natural (not reversed) order.
   */
  static ArrayData* MakeVecNatural(uint32_t size, const TypedValue* values);

  static ArrayData* MakeUninitializedVec(uint32_t size);

  static ArrayData* MakeUncounted(
      ArrayData* array, const MakeUncountedEnv& env, bool hasApcTv);

  static ArrayData* MakeVecFromAPC(const APCArray* apc, bool pure, bool isLegacy = false);

  static bool VecEqual(const ArrayData* ad1, const ArrayData* ad2);
  static bool VecNotEqual(const ArrayData* ad1, const ArrayData* ad2);
  static bool VecSame(const ArrayData* ad1, const ArrayData* ad2);
  static bool VecNotSame(const ArrayData* ad1, const ArrayData* ad2);

  // Fast iteration
  template <class F>
  static void IterateV(const ArrayData* arr, F fn);
  template <class F>
  static void IterateKV(const ArrayData* arr, F fn);

  // Return a VanillaDict with the same elements as this VanillaVec..
  // The target type is based on the source: varray -> darray, vec -> dict.
  static VanillaDict* ToMixed(ArrayData*);
  static VanillaDict* ToMixedCopy(const ArrayData*);
  static VanillaDict* ToMixedCopyReserve(const ArrayData*, size_t);

  // Converts a pointer into the given array to an index at that array.
  // May fail, in which case the result will be negative. May be slow.
  static int64_t pointerToIndex(const ArrayData*, const void* ptr);

  static size_t capacityToSizeBytes(size_t);
  static size_t capacityToSizeIndex(size_t);

  static constexpr auto SizeIndexOffset = HeaderAuxOffset + 1;
private:
  static VanillaDict* ToMixedHeader(const ArrayData*, size_t);

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

  struct MarkedVecInitializer;
  static MarkedVecInitializer s_marked_vec_initializer;
};

//////////////////////////////////////////////////////////////////////

}
