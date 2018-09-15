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
#ifndef incl_HPHP_EMPTY_ARRAY_H_
#define incl_HPHP_EMPTY_ARRAY_H_

#include <cstddef>
#include <cstdint>
#include <utility>
#include <sys/types.h>

#include "hphp/runtime/base/array-common.h"
#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/header-kind.h"
#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/base/sort-flags.h"
#include "hphp/runtime/base/typed-value.h"

#include "hphp/util/type-scan.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct Variant;
struct RefData;
struct StringData;
struct MArrayIter;
struct MixedArray;

//////////////////////////////////////////////////////////////////////

/*
 * Functions relating to the "empty array" kind.  These implement
 * entries in the array dispatch table for the global empty array.
 * Other arrays may also be empty in the sense that size() == 0, but
 * this one is dealt with commonly enough to deserve special handlers.
 */
struct EmptyArray final : type_scan::MarkCollectable<EmptyArray> {
  static void Release(ArrayData*);

  static tv_rval NvGetInt(const ArrayData*, int64_t) {
    return nullptr;
  }
  static constexpr auto NvTryGetInt = &NvGetInt;

  static tv_rval NvGetStr(const ArrayData*, const StringData*) {
    return nullptr;
  }
  static constexpr auto NvTryGetStr = &NvGetStr;

  static tv_rval RvalInt(const ArrayData* ad, int64_t k) {
    return NvGetInt(ad, k);
  }
  static tv_rval RvalIntStrict(const ArrayData* ad, int64_t k) {
    return NvTryGetInt(ad, k);
  }
  static tv_rval RvalStr(const ArrayData* ad, const StringData* k) {
    return NvGetStr(ad, k);
  }
  static tv_rval RvalStrStrict(const ArrayData* ad, const StringData* k) {
    return NvTryGetStr(ad, k);
  }
  static tv_rval RvalAtPos(const ArrayData* ad, ssize_t pos) {
    return GetValueRef(ad, pos);
  }

  static Cell NvGetKey(const ArrayData*, ssize_t pos);
  static ArrayData* SetInt(ArrayData*, int64_t k, Cell v, bool copy);
  static ArrayData* SetStr(ArrayData*, StringData* k, Cell v, bool copy);
  static ArrayData* SetWithRefInt(ArrayData*, int64_t k,
                                  TypedValue v, bool copy);
  static ArrayData* SetWithRefStr(ArrayData*, StringData* k,
                                  TypedValue v, bool copy);
  static ArrayData* RemoveInt(ArrayData* ad, int64_t, bool) {
    return ad;
  }
  static ArrayData* RemoveStr(ArrayData* ad, const StringData*, bool) {
    return ad;
  }
  static size_t Vsize(const ArrayData*);
  static tv_rval GetValueRef(const ArrayData* ad, ssize_t pos);
  static bool IsVectorData(const ArrayData*) {
    return true;
  }
  static bool ExistsInt(const ArrayData*, int64_t) {
    return false;
  }
  static bool ExistsStr(const ArrayData*, const StringData*) {
    return false;
  }
  static arr_lval LvalInt(ArrayData*, int64_t k, bool copy);
  static arr_lval LvalIntRef(ArrayData*, int64_t k, bool copy);
  static arr_lval LvalStr(ArrayData*, StringData* k, bool copy);
  static arr_lval LvalStrRef(ArrayData*, StringData* k, bool copy);
  static arr_lval LvalNew(ArrayData*, bool copy);
  static arr_lval LvalNewRef(ArrayData*, bool copy);
  static ArrayData* SetRefInt(ArrayData*, int64_t k,
                              tv_lval v, bool copy);
  static ArrayData* SetRefStr(ArrayData*, StringData* k,
                              tv_lval v, bool copy);
  static constexpr auto IterBegin = &ArrayCommon::ReturnInvalidIndex;
  static constexpr auto IterLast = &ArrayCommon::ReturnInvalidIndex;
  static constexpr auto IterEnd = &ArrayCommon::ReturnInvalidIndex;
  static ssize_t IterAdvance(const ArrayData*, ssize_t prev);
  static ssize_t IterRewind(const ArrayData*, ssize_t prev);

  // ValidMArrayIter may be called on this array kind, because Escalate is a
  // no-op.
  static bool ValidMArrayIter(const ArrayData*, const MArrayIter&) {
    return false;
  }
  static bool AdvanceMArrayIter(ArrayData*, MArrayIter& fp);
  static ArrayData* EscalateForSort(ArrayData* ad, SortFunction /*sf*/) {
    return ad;
  }
  static void Ksort(ArrayData*, int, bool) {}
  static void Sort(ArrayData*, int, bool) {}
  static void Asort(ArrayData*, int, bool) {}
  static bool Uksort(ArrayData*, const Variant&) {
    return true;
  }
  static bool Usort(ArrayData*, const Variant&) {
    return true;
  }
  static bool Uasort(ArrayData*, const Variant&) {
    return true;
  }
  static ArrayData* PopOrDequeue(ArrayData*, Variant&);
  static constexpr auto Pop = &PopOrDequeue;
  static constexpr auto Dequeue = &PopOrDequeue;
  static ArrayData* Copy(const ArrayData* ad);
  static ArrayData* CopyStatic(const ArrayData*);
  static ArrayData* Append(ArrayData*, Cell v, bool copy);
  static ArrayData* AppendRef(ArrayData*, tv_lval v, bool copy);
  static ArrayData* AppendWithRef(ArrayData*, TypedValue v, bool copy);
  static ArrayData* PlusEq(ArrayData*, const ArrayData* elems);
  static ArrayData* Merge(ArrayData*, const ArrayData* elems);
  static ArrayData* Prepend(ArrayData*, Cell v);
  static ArrayData* ToPHPArray(ArrayData* ad, bool) {
    return ad;
  }
  static ArrayData* ToVArray(ArrayData*, bool) {
    return staticEmptyVArray();
  }
  static ArrayData* ToDArray(ArrayData*, bool) {
    return staticEmptyDArray();
  }
  static ArrayData* ToDict(ArrayData*, bool);
  static ArrayData* ToShape(ArrayData*, bool);
  static ArrayData* ToVec(ArrayData*, bool);
  static ArrayData* ToKeyset(ArrayData*, bool);
  static void Renumber(ArrayData*) {}
  static void OnSetEvalScalar(ArrayData*);
  static ArrayData* Escalate(const ArrayData* ad) {
    return const_cast<ArrayData*>(ad);
  }

private:
  static arr_lval MakePacked(TypedValue);
  static arr_lval MakePackedInl(TypedValue);
  static arr_lval MakeMixed(StringData*, TypedValue);
  static arr_lval MakeMixed(int64_t, TypedValue);

private:
  struct Initializer;
  static Initializer s_initializer;
};

//////////////////////////////////////////////////////////////////////

}

#endif
