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
#include "hphp/runtime/base/member-val.h"
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
struct EmptyArray final : type_scan::MarkCountable<EmptyArray> {
  static void Release(ArrayData*);

  static member_rval::ptr_u NvGetInt(const ArrayData*, int64_t) {
    return nullptr;
  }
  static constexpr auto NvTryGetInt = &NvGetInt;

  static member_rval::ptr_u NvGetStr(const ArrayData*, const StringData*) {
    return nullptr;
  }
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
  static ArrayData* RemoveInt(ArrayData* ad, int64_t, bool) {
    return ad;
  }
  static ArrayData* RemoveStr(ArrayData* ad, const StringData*, bool) {
    return ad;
  }
  static size_t Vsize(const ArrayData*);
  static member_rval::ptr_u GetValueRef(const ArrayData* ad, ssize_t pos);
  static bool IsVectorData(const ArrayData*) {
    return true;
  }
  static bool ExistsInt(const ArrayData*, int64_t) {
    return false;
  }
  static bool ExistsStr(const ArrayData*, const StringData*) {
    return false;
  }
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
  static ArrayData* ZSetInt(ArrayData* ad, int64_t k, RefData* v);
  static ArrayData* ZSetStr(ArrayData* ad, StringData* k, RefData* v);
  static ArrayData* ZAppend(ArrayData* ad, RefData* v, int64_t* key_ptr);
  static ArrayData* Append(ArrayData*, Cell v, bool copy);
  static ArrayData* AppendRef(ArrayData*, Variant& v, bool copy);
  static ArrayData* AppendWithRef(ArrayData*, TypedValue v, bool copy);
  static ArrayData* PlusEq(ArrayData*, const ArrayData* elems);
  static ArrayData* Merge(ArrayData*, const ArrayData* elems);
  static ArrayData* Prepend(ArrayData*, Cell v, bool copy);
  static ArrayData* ToPHPArray(ArrayData* ad, bool) {
    return ad;
  }
  static ArrayData* ToDict(ArrayData*, bool);
  static ArrayData* ToVec(ArrayData*, bool);
  static ArrayData* ToKeyset(ArrayData*, bool);
  static void Renumber(ArrayData*) {}
  static void OnSetEvalScalar(ArrayData*);
  static ArrayData* Escalate(const ArrayData* ad) {
    return const_cast<ArrayData*>(ad);
  }

  static constexpr auto ToVArray = &ToPHPArray;

private:
  static member_lval MakePacked(TypedValue);
  static member_lval MakePackedInl(TypedValue);
  static member_lval MakeMixed(StringData*, TypedValue);
  static member_lval MakeMixed(int64_t, TypedValue);

private:
  struct Initializer;
  static Initializer s_initializer;
};

//////////////////////////////////////////////////////////////////////

}

#endif
