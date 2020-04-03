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
struct StringData;
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
  static tv_rval NvGetStr(const ArrayData*, const StringData*) {
    return nullptr;
  }

  static ssize_t NvGetIntPos(const ArrayData* ad, int64_t) {
    return ArrayCommon::ReturnInvalidIndex(ad);
  }
  static ssize_t NvGetStrPos(const ArrayData* ad, const StringData*) {
    return ArrayCommon::ReturnInvalidIndex(ad);
  }

  static TypedValue GetPosKey(const ArrayData*, ssize_t pos);
  static TypedValue GetPosVal(const ArrayData*, ssize_t pos);
  static ArrayData* SetInt(ArrayData*, int64_t k, TypedValue v);
  static ArrayData* SetIntMove(ArrayData*, int64_t k, TypedValue v);
  static ArrayData* SetStr(ArrayData*, StringData* k, TypedValue v);
  static ArrayData* SetStrMove(ArrayData*, StringData* k, TypedValue v);
  static ArrayData* RemoveInt(ArrayData* ad, int64_t);
  static ArrayData* RemoveStr(ArrayData* ad, const StringData*);
  static size_t Vsize(const ArrayData*);
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
  static arr_lval LvalStr(ArrayData*, StringData* k, bool copy);
  static constexpr auto IterBegin = &ArrayCommon::ReturnInvalidIndex;
  static constexpr auto IterLast = &ArrayCommon::ReturnInvalidIndex;
  static constexpr auto IterEnd = &ArrayCommon::ReturnInvalidIndex;
  static ssize_t IterAdvance(const ArrayData*, ssize_t prev);
  static ssize_t IterRewind(const ArrayData*, ssize_t prev);

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
  static ArrayData* Append(ArrayData*, TypedValue v);
  static ArrayData* PlusEq(ArrayData*, const ArrayData* elems);
  static ArrayData* Merge(ArrayData*, const ArrayData* elems);
  static ArrayData* Prepend(ArrayData*, TypedValue v);
  static ArrayData* ToPHPArray(ArrayData* ad, bool) {
    return ad;
  }
  static constexpr auto ToPHPArrayIntishCast = &ToPHPArray;
  static ArrayData* ToVArray(ArrayData*, bool) {
    return ArrayData::CreateVArray();
  }
  static ArrayData* ToDArray(ArrayData*, bool) {
    return ArrayData::CreateDArray();
  }
  static ArrayData* ToDict(ArrayData*, bool);
  static ArrayData* ToVec(ArrayData*, bool);
  static ArrayData* ToKeyset(ArrayData*, bool);
  static void Renumber(ArrayData*) {}
  static void OnSetEvalScalar(ArrayData*);

private:
  static arr_lval MakePacked(TypedValue);
  static arr_lval MakePackedInl(TypedValue);
  static arr_lval MakeMixed(StringData*, TypedValue);
  static arr_lval MakeMixed(int64_t, TypedValue);

  template<bool warn>
  static arr_lval LvalIntImpl(ArrayData*, int64_t k, bool);
  template<bool warn>
  static arr_lval LvalStrImpl(ArrayData*, StringData* k, bool);

private:
  struct Initializer;
  static Initializer s_initializer;
};

//////////////////////////////////////////////////////////////////////

}

#endif
