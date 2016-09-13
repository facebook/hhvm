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
#ifndef incl_HPHP_PACKED_ARRAY_H_
#define incl_HPHP_PACKED_ARRAY_H_

#include <cstddef>
#include <cstdint>
#include <sys/types.h>

#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/array-common.h"
#include "hphp/runtime/base/sort-flags.h"
#include "hphp/runtime/base/header-kind.h"

#include "hphp/util/type-scan.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct Variant;
struct RefData;
struct ArrayData;
struct StringData;
struct MArrayIter;
struct MixedArray;
struct APCArray;

//////////////////////////////////////////////////////////////////////

/*
 * Packed arrays are a specialized array layout for vector-like data.  That is,
 * php arrays with zero-based contiguous integer keys, and values of mixed
 * types.  The TypedValue's are placed right after the array header.
 */
struct PackedArray final: type_scan::MarkCountable<PackedArray> {
  static constexpr uint32_t MaxSize = 0xFFFFFFFFul;
  static constexpr uint32_t SmallSize = 3;

  static void Release(ArrayData*);
  static void ReleaseUncounted(ArrayData*, size_t extra = 0);
  static const TypedValue* NvGetInt(const ArrayData*, int64_t ki);
  static constexpr auto NvTryGetInt = &NvGetInt;
  static const TypedValue* NvGetStr(const ArrayData*, const StringData*);
  static constexpr auto NvTryGetStr = &NvGetStr;
  static void NvGetKey(const ArrayData*, TypedValue* out, ssize_t pos);
  static ArrayData* SetInt(ArrayData*, int64_t k, Cell v, bool copy);
  static ArrayData* SetStr(ArrayData*, StringData* k, Cell v, bool copy);
  static size_t Vsize(const ArrayData*);
  static const Variant& GetValueRef(const ArrayData* ad, ssize_t pos);
  static bool IsVectorData(const ArrayData*) {
    return true;
  }
  static bool ExistsInt(const ArrayData* ad, int64_t k);
  static bool ExistsStr(const ArrayData*, const StringData*);
  static ArrayData* LvalInt(ArrayData*, int64_t k, Variant*& ret, bool copy);
  static constexpr auto LvalIntRef = &LvalInt;
  static ArrayData* LvalStr(ArrayData*, StringData* k, Variant*& ret,
                            bool copy);
  static constexpr auto LvalStrRef = &LvalStr;
  static ArrayData* LvalNew(ArrayData*, Variant*& ret, bool copy);
  static constexpr auto LvalNewRef = &LvalNew;
  static ArrayData* SetRefInt(ArrayData*, int64_t k, Variant& v, bool copy);
  static ArrayData* SetRefStr(ArrayData*, StringData* k, Variant& v,
    bool copy);
  static constexpr auto AddInt = &SetInt;
  static constexpr auto AddStr = &SetStr;
  static ArrayData* RemoveInt(ArrayData*, int64_t k, bool copy);
  static ArrayData* RemoveStr(ArrayData*, const StringData* k, bool copy);
  static ssize_t IterBegin(const ArrayData*);
  static ssize_t IterLast(const ArrayData*);
  static ssize_t IterEnd(const ArrayData*);
  static ssize_t IterAdvance(const ArrayData*, ssize_t pos);
  static ssize_t IterRewind(const ArrayData*, ssize_t pos);
  static constexpr auto ValidMArrayIter = &ArrayCommon::ValidMArrayIter;
  static bool AdvanceMArrayIter(ArrayData*, MArrayIter& fp);
  static void CopyPackedHelper(const ArrayData* adIn, ArrayData* ad,
                               RefCount initial_count, HeaderKind dest_hk);
  static ArrayData* Copy(const ArrayData* ad);
  static ArrayData* CopyWithStrongIterators(const ArrayData*);
  static ArrayData* CopyStatic(const ArrayData*);
  static ArrayData* EscalateForSort(ArrayData*, SortFunction);
  static void Ksort(ArrayData*, int, bool);
  static void Sort(ArrayData*, int, bool);
  static void Asort(ArrayData*, int, bool);
  static bool Uksort(ArrayData*, const Variant&);
  static bool Usort(ArrayData*, const Variant&);
  static bool Uasort(ArrayData*, const Variant&);
  static ArrayData* ZSetInt(ArrayData*, int64_t k, RefData* v);
  static ArrayData* ZSetStr(ArrayData*, StringData* k, RefData* v);
  static ArrayData* ZAppend(ArrayData*, RefData* v, int64_t* key_ptr);
  static ArrayData* Append(ArrayData*, Cell v, bool copy);
  static ArrayData* AppendRef(ArrayData*, Variant& v, bool copy);
  static ArrayData* AppendWithRef(ArrayData*, const Variant& v, bool copy);
  static ArrayData* PlusEq(ArrayData*, const ArrayData* elems);
  static ArrayData* Merge(ArrayData*, const ArrayData* elems);
  static ArrayData* Pop(ArrayData*, Variant& value);
  static ArrayData* Dequeue(ArrayData*, Variant& value);
  static ArrayData* Prepend(ArrayData*, Cell v, bool copy);
  static ArrayData* ToPHPArray(ArrayData*, bool);
  static ArrayData* ToDict(ArrayData*, bool);
  static ArrayData* ToVec(ArrayData*, bool);
  static void Renumber(ArrayData*) {}
  static void OnSetEvalScalar(ArrayData*);
  static ArrayData* Escalate(const ArrayData* ad) {
    return const_cast<ArrayData*>(ad);
  }

  static constexpr auto ToKeyset = &ArrayCommon::ToKeyset;

  static const TypedValue* NvTryGetIntVec(const ArrayData*, int64_t);
  static const TypedValue* NvTryGetStrVec(const ArrayData*, const StringData*);
  static ArrayData* SetIntVec(ArrayData*, int64_t, Cell, bool);
  static ArrayData* SetStrVec(ArrayData*, StringData*, Cell, bool);
  static ArrayData* RemoveIntVec(ArrayData*, int64_t, bool);
  static ArrayData* LvalIntVec(ArrayData*, int64_t, Variant*&, bool);
  static ArrayData* LvalStrVec(ArrayData*, StringData*, Variant*&, bool);
  static ArrayData* LvalIntRefVec(ArrayData*, int64_t, Variant*&, bool);
  static ArrayData* LvalStrRefVec(ArrayData*, StringData*, Variant*&, bool);
  static ArrayData* LvalNewRefVec(ArrayData*, Variant*&, bool);
  static ArrayData* SetRefIntVec(ArrayData*, int64_t, Variant&, bool);
  static ArrayData* SetRefStrVec(ArrayData*, StringData*, Variant&, bool);
  static ArrayData* AppendRefVec(ArrayData*, Variant&, bool);
  static ArrayData* AppendWithRefVec(ArrayData*, const Variant&, bool);
  static ArrayData* PlusEqVec(ArrayData*, const ArrayData*);
  static ArrayData* ToPHPArrayVec(ArrayData*, bool);
  static ArrayData* ToDictVec(ArrayData*, bool);
  static ArrayData* ToVecVec(ArrayData*, bool);

  static constexpr auto MergeVec = &Merge;
  static constexpr auto ReleaseVec = &Release;
  static constexpr auto NvGetIntVec = &NvGetInt;
  static constexpr auto NvGetStrVec = &NvGetStr;
  static constexpr auto NvGetKeyVec = &NvGetKey;
  static constexpr auto VsizeVec = &Vsize;
  static constexpr auto GetValueRefVec = &GetValueRef;
  static constexpr auto IsVectorDataVec = &IsVectorData;
  static constexpr auto ExistsIntVec = &ExistsInt;
  static constexpr auto ExistsStrVec = &ExistsStr;
  static constexpr auto LvalNewVec = &LvalNew;
  static constexpr auto RemoveStrVec = &RemoveStr;
  static constexpr auto IterBeginVec = &IterBegin;
  static constexpr auto IterLastVec = &IterLast;
  static constexpr auto IterEndVec = &IterEnd;
  static constexpr auto IterAdvanceVec = &IterAdvance;
  static constexpr auto IterRewindVec = &IterRewind;
  static constexpr auto ValidMArrayIterVec = ValidMArrayIter;
  static constexpr auto AdvanceMArrayIterVec = &AdvanceMArrayIter;
  static constexpr auto EscalateForSortVec = &EscalateForSort;
  static constexpr auto KsortVec = &Ksort;
  static constexpr auto SortVec = &Sort;
  static constexpr auto AsortVec = &Asort;
  static constexpr auto UksortVec = &Uksort;
  static constexpr auto UsortVec = &Usort;
  static constexpr auto UasortVec = &Uasort;
  static constexpr auto CopyVec = &Copy;
  static constexpr auto CopyStaticVec = &CopyStatic;
  static constexpr auto CopyWithStrongIteratorsVec = &CopyWithStrongIterators;
  static constexpr auto AppendVec = &Append;
  static constexpr auto PopVec = &Pop;
  static constexpr auto DequeueVec = &Dequeue;
  static constexpr auto PrependVec = &Prepend;
  static constexpr auto RenumberVec = &Renumber;
  static constexpr auto OnSetEvalScalarVec = &OnSetEvalScalar;
  static constexpr auto EscalateVec = &Escalate;
  static constexpr auto ToKeysetVec = &ArrayCommon::ToKeyset;

  //////////////////////////////////////////////////////////////////////

  // Like LvalInt, but silently does nothing if the element doesn't exist. Not
  // part of the ArrayData interface, but used in member operations.
  static ArrayData* LvalSilentInt(ArrayData*, int64_t, Variant*&, bool);

  static constexpr auto LvalSilentIntVec = &LvalSilentInt;

  /////////////////////////////////////////////////////////////////////

  static bool checkInvariants(const ArrayData*);

  /*
   * Accepts any array of any kind satisfying isVectorData() and makes a
   * static packed copy, like CopyStatic().
   */
  static ArrayData* ConvertStatic(const ArrayData*);
  static ArrayData* ConvertStaticHelper(const ArrayData*);

  static ptrdiff_t entriesOffset();
  static uint32_t getMaxCapInPlaceFast(uint32_t cap);

  static size_t heapSize(const ArrayData*);
  template<class Marker> static void scan(const ArrayData*, Marker&);

  static ArrayData* MakeReserve(uint32_t capacity);
  static ArrayData* MakeReserveVec(uint32_t capacity);

  /*
   * Allocate a PackedArray containing `size' values, in the reverse order of
   * the `values' array.
   *
   * This function takes ownership of the TypedValues in `values'.
   */
  static ArrayData* MakePacked(uint32_t size, const TypedValue* values);
  static ArrayData* MakeVec(uint32_t size, const TypedValue* values);

  static ArrayData* MakeUninitialized(uint32_t size);
  static ArrayData* MakeUninitializedVec(uint32_t size);

  static ArrayData* MakeUncounted(ArrayData* array, size_t extra = 0);
  static ArrayData* MakeUncountedHelper(ArrayData* array, size_t extra);

  static ArrayData* MakeVecFromAPC(const APCArray* apc);

  static bool VecEqual(const ArrayData* ad1, const ArrayData* ad2);
  static bool VecNotEqual(const ArrayData* ad1, const ArrayData* ad2);
  static bool VecSame(const ArrayData* ad1, const ArrayData* ad2);
  static bool VecNotSame(const ArrayData* ad1, const ArrayData* ad2);
  static bool VecLt(const ArrayData* ad1, const ArrayData* ad2);
  static bool VecLte(const ArrayData* ad1, const ArrayData* ad2);
  static bool VecGt(const ArrayData* ad1, const ArrayData* ad2);
  static bool VecGte(const ArrayData* ad1, const ArrayData* ad2);
  static int64_t VecCmp(const ArrayData* ad1, const ArrayData* ad2);

  // Fast iteration
  template <class F, bool inc = true>
  static void IterateV(const ArrayData* arr, F fn);
  template <class F, bool inc = true>
  static void IterateKV(const ArrayData* arr, F fn);

private:
  static ArrayData* Grow(ArrayData*);
  static ArrayData* GrowHelper(ArrayData*);
  static MixedArray* ToMixedHeader(const ArrayData*, size_t);
  static MixedArray* ToMixed(ArrayData*);
  static MixedArray* ToMixedCopy(const ArrayData*);
  static MixedArray* ToMixedCopyReserve(const ArrayData*, size_t);
  static ArrayData* CopyAndResizeIfNeededSlow(const ArrayData*);
  static ArrayData* CopyAndResizeIfNeeded(const ArrayData*);
  static ArrayData* ResizeIfNeeded(ArrayData*);
  static SortFlavor preSort(ArrayData*);

  static ArrayData* MakeReserveImpl(uint32_t, HeaderKind);
  static ArrayData* MakeReserveSlow(uint32_t, HeaderKind);

  static ArrayData* MakePackedImpl(uint32_t, const TypedValue*, HeaderKind);

  static ArrayData* MakeUninitializedImpl(uint32_t, HeaderKind);

  static ArrayData* CopyStaticHelper(const ArrayData*);

  static bool VecEqualHelper(const ArrayData*, const ArrayData*, bool);
  static int64_t VecCmpHelper(const ArrayData*, const ArrayData*);

  struct VecInitializer;
  static VecInitializer s_initializer;
};

//////////////////////////////////////////////////////////////////////

}

#endif
