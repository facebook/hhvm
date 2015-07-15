/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/sort-flags.h"
#include "hphp/runtime/base/header-kind.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct Variant;
struct ArrayData;
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
struct EmptyArray {
  static void Release(ArrayData*);
  static const TypedValue* NvGetInt(const ArrayData*, int64_t) {
    return nullptr;
  }
  static const TypedValue* NvGetStr(const ArrayData*, const StringData*) {
    return nullptr;
  }
  static void NvGetKey(const ArrayData*, TypedValue* out, ssize_t pos);
  static ArrayData* SetInt(ArrayData*, int64_t k, Cell v, bool copy);
  static ArrayData* SetStr(ArrayData*, StringData* k, Cell v, bool copy);
  static ArrayData* RemoveInt(ArrayData* ad, int64_t, bool) {
    return ad;
  }
  static ArrayData* RemoveStr(ArrayData* ad, const StringData*, bool) {
    return ad;
  }
  static size_t Vsize(const ArrayData*);
  static const Variant& GetValueRef(const ArrayData* ad, ssize_t pos);
  static bool IsVectorData(const ArrayData*) {
    return true;
  }
  static bool ExistsInt(const ArrayData*, int64_t) {
    return false;
  }
  static bool ExistsStr(const ArrayData*, const StringData*) {
    return false;
  }
  static ArrayData* LvalInt(ArrayData*, int64_t k, Variant*& ret, bool copy);
  static ArrayData* LvalStr(ArrayData*, StringData* k, Variant*& ret,
                            bool copy);
  static ArrayData* LvalNew(ArrayData*, Variant*& ret, bool copy);
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
  static ArrayData* EscalateForSort(ArrayData* ad, SortFunction sf) {
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
  static ArrayData* CopyWithStrongIterators(const ArrayData*);
  static ArrayData* CopyStatic(const ArrayData*);
  static ArrayData* ZSetInt(ArrayData* ad, int64_t k, RefData* v);
  static ArrayData* ZSetStr(ArrayData* ad, StringData* k, RefData* v);
  static ArrayData* ZAppend(ArrayData* ad, RefData* v, int64_t* key_ptr);
  static ArrayData* Append(ArrayData*, const Variant& v, bool copy);
  static ArrayData* AppendRef(ArrayData*, Variant& v, bool copy);
  static ArrayData* AppendWithRef(ArrayData*, const Variant& v, bool copy);
  static ArrayData* PlusEq(ArrayData*, const ArrayData* elems);
  static ArrayData* Merge(ArrayData*, const ArrayData* elems);
  static ArrayData* Prepend(ArrayData*, const Variant& v, bool copy);
  static void Renumber(ArrayData*) {}
  static void OnSetEvalScalar(ArrayData*);
  static ArrayData* Escalate(const ArrayData* ad) {
    return const_cast<ArrayData*>(ad);
  }

private:
  static std::pair<ArrayData*,TypedValue*> MakePacked(TypedValue);
  static std::pair<ArrayData*,TypedValue*> MakePackedInl(TypedValue);
  static std::pair<ArrayData*,TypedValue*> MakeMixed(StringData*, TypedValue);
  static std::pair<ArrayData*,TypedValue*> MakeMixed(int64_t, TypedValue);

private:
  struct Initializer;
  static Initializer s_initializer;
};

//////////////////////////////////////////////////////////////////////

}

#endif
