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
#ifndef incl_HPHP_PACKED_ARRAY_H_
#define incl_HPHP_PACKED_ARRAY_H_

#include <cstddef>
#include <cstdint>
#include <sys/types.h>

#include "hphp/runtime/base/array-common.h"
#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/header-kind.h"
#include "hphp/runtime/base/member-val.h"
#include "hphp/runtime/base/sort-flags.h"
#include "hphp/runtime/base/typed-value.h"

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
struct APCHandle;

//////////////////////////////////////////////////////////////////////

/*
 * Packed arrays are a specialized array layout for vector-like data.  That is,
 * php arrays with zero-based contiguous integer keys, and values of mixed
 * types.  The TypedValue's are placed right after the array header.
 */
struct PackedArray final : type_scan::MarkCountable<PackedArray> {
  static constexpr uint32_t SmallSize = 3;
  // the smallest and largest MM size classes we use for allocating PackedArrays
  static constexpr size_t SmallSizeIndex = 3;
  static constexpr size_t MaxSizeIndex = 121;

  static void Release(ArrayData*);
  // Recursively register {allocation, rootAPCHandle} with APCGCManager
  static void RegisterUncountedAllocations(ArrayData* ad,
                                            APCHandle* rootAPCHandle);
  static void ReleaseUncounted(ArrayData*, size_t extra = 0);
  static member_rval::ptr_u NvGetInt(const ArrayData*, int64_t ki);
  static constexpr auto NvTryGetInt = &NvGetInt;
  static member_rval::ptr_u NvGetStr(const ArrayData*, const StringData*);
  static constexpr auto NvTryGetStr = &NvGetStr;
  static member_rval RvalInt(const ArrayData* ad, int64_t k) {
    return member_rval { ad, NvGetInt(ad, k) };
  }
  static member_rval RvalIntStrict(const ArrayData* ad, int64_t k) {
    return member_rval { ad, NvTryGetInt(ad, k) };
  }
  static member_rval RvalStr(const ArrayData* ad, const StringData* k) {
    return member_rval { ad, NvGetStr(ad, k) };
  }
  static member_rval RvalStrStrict(const ArrayData* ad, const StringData* k) {
    return member_rval { ad, NvTryGetStr(ad, k) };
  }
  static member_rval RvalAtPos(const ArrayData* ad, ssize_t pos) {
    return member_rval { ad, GetValueRef(ad, pos) };
  }
  static Cell NvGetKey(const ArrayData*, ssize_t pos);
  static ArrayData* SetInt(ArrayData*, int64_t k, Cell v, bool copy);
  static ArrayData* SetStr(ArrayData*, StringData* k, Cell v, bool copy);
  static size_t Vsize(const ArrayData*);
  static member_rval::ptr_u GetValueRef(const ArrayData* ad, ssize_t pos);
  static bool IsVectorData(const ArrayData*) {
    return true;
  }
  static bool ExistsInt(const ArrayData* ad, int64_t k);
  static bool ExistsStr(const ArrayData*, const StringData*);
  static member_lval LvalInt(ArrayData*, int64_t k, bool copy);
  static member_lval LvalIntRef(ArrayData*, int64_t k, bool copy);
  static member_lval LvalStr(ArrayData*, StringData* k, bool copy);
  static member_lval LvalStrRef(ArrayData*, StringData* k, bool copy);
  static member_lval LvalNew(ArrayData*, bool copy);
  static member_lval LvalNewRef(ArrayData*, bool copy);
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
  static ArrayData* Copy(const ArrayData* ad);
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
  static ArrayData* AppendWithRef(ArrayData*, TypedValue v, bool copy);
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
  static constexpr auto ToVArray = &ToPHPArray;

  static member_rval::ptr_u NvTryGetIntVec(const ArrayData*, int64_t);
  static member_rval::ptr_u NvTryGetStrVec(const ArrayData*, const StringData*);
  static ArrayData* SetIntVec(ArrayData*, int64_t, Cell, bool);
  static ArrayData* SetStrVec(ArrayData*, StringData*, Cell, bool);
  static ArrayData* RemoveIntVec(ArrayData*, int64_t, bool);
  static member_lval LvalIntVec(ArrayData*, int64_t, bool);
  static member_lval LvalStrVec(ArrayData*, StringData*, bool);
  static member_lval LvalIntRefVec(ArrayData*, int64_t, bool);
  static member_lval LvalStrRefVec(ArrayData*, StringData*, bool);
  static member_lval LvalNewRefVec(ArrayData*, bool);
  static ArrayData* SetRefIntVec(ArrayData*, int64_t, Variant&, bool);
  static ArrayData* SetRefStrVec(ArrayData*, StringData*, Variant&, bool);
  static ArrayData* AppendRefVec(ArrayData*, Variant&, bool);
  static ArrayData* AppendWithRefVec(ArrayData*, TypedValue, bool);
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
  static constexpr auto AppendVec = &Append;
  static constexpr auto PopVec = &Pop;
  static constexpr auto DequeueVec = &Dequeue;
  static constexpr auto PrependVec = &Prepend;
  static constexpr auto RenumberVec = &Renumber;
  static constexpr auto OnSetEvalScalarVec = &OnSetEvalScalar;
  static constexpr auto EscalateVec = &Escalate;
  static constexpr auto ToKeysetVec = &ArrayCommon::ToKeyset;
  static constexpr auto ToVArrayVec = &ToPHPArrayVec;

  static member_rval RvalIntVec(const ArrayData* ad, int64_t k) {
    return member_rval { ad, NvGetIntVec(ad, k) };
  }
  static member_rval RvalIntStrictVec(const ArrayData* ad, int64_t k) {
    return member_rval { ad, NvTryGetIntVec(ad, k) };
  }

  //////////////////////////////////////////////////////////////////////

  // Like LvalInt, but silently does nothing if the element doesn't exist. Not
  // part of the ArrayData interface, but used in member operations.
  static member_lval LvalSilentInt(ArrayData*, int64_t, bool);
  static constexpr auto LvalSilentIntVec = &LvalSilentInt;

  /////////////////////////////////////////////////////////////////////

  static bool checkInvariants(const ArrayData*);

  /*
   * Accepts any array of any kind satisfying isVectorData() and makes a
   * static packed copy, like CopyStatic().
   */
  static ArrayData* ConvertStatic(const ArrayData*);

  static ptrdiff_t entriesOffset();

  static uint32_t capacity(const ArrayData*);
  static size_t heapSize(const ArrayData*);
  static void scan(const ArrayData*, type_scan::Scanner&);

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
  /*
   * Like MakePacked, but with `values' array in natural (not reversed) order.
   */
  static ArrayData* MakePackedNatural(uint32_t size, const TypedValue* values);

  static ArrayData* MakeUninitialized(uint32_t size);
  static ArrayData* MakeUninitializedVec(uint32_t size);

  static ArrayData* MakeUncounted(ArrayData* array, size_t extra = 0);

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
  static MixedArray* ToMixedHeader(const ArrayData*, size_t);
  static MixedArray* ToMixed(ArrayData*);
  static MixedArray* ToMixedCopy(const ArrayData*);
  static MixedArray* ToMixedCopyReserve(const ArrayData*, size_t);
  static ArrayData* Grow(ArrayData*, bool);
  static ArrayData* PrepareForInsert(ArrayData*, bool);
  static SortFlavor preSort(ArrayData*);

  static ArrayData* MakeReserveImpl(uint32_t, HeaderKind);

  template<bool reverse>
  static ArrayData* MakePackedImpl(uint32_t, const TypedValue*, HeaderKind);

  template<bool convertingPackedToVec>
  static bool CopyPackedHelper(const ArrayData* adIn, ArrayData* ad);

  static bool VecEqualHelper(const ArrayData*, const ArrayData*, bool);
  static int64_t VecCmpHelper(const ArrayData*, const ArrayData*);

  struct VecInitializer;
  static VecInitializer s_initializer;
};

//////////////////////////////////////////////////////////////////////

}

#endif
