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
#ifndef incl_HPHP_APC_LOCAL_ARRAY_H_
#define incl_HPHP_APC_LOCAL_ARRAY_H_

#include <vector>
#include <utility>
#include <cstdint>
#include <sys/types.h>

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/apc-array.h"
#include "hphp/runtime/base/member-val.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct Variant;
struct TypedValue;
struct StringData;
struct MArrayIter;

//////////////////////////////////////////////////////////////////////

/*
 * Wrapper for APCArray. It is what gets returned when an array is fetched
 * via APC. It has a pointer to the APCArray that it represents and it may
 * cache values locally depending on the type accessed and/or the operation.
 */
struct APCLocalArray final : ArrayData,
                             type_scan::MarkCountable<APCLocalArray> {
  static APCLocalArray* Make(const APCArray*);

  static size_t Vsize(const ArrayData*);
  static member_rval::ptr_u GetValueRef(const ArrayData* ad, ssize_t pos);
  static bool ExistsInt(const ArrayData* ad, int64_t k);
  static bool ExistsStr(const ArrayData* ad, const StringData* k);
  static member_lval LvalInt(ArrayData*, int64_t k, bool copy);
  static member_lval LvalIntRef(ArrayData*, int64_t k, bool copy);
  static member_lval LvalStr(ArrayData*, StringData* k, bool copy);
  static member_lval LvalStrRef(ArrayData*, StringData* k, bool copy);
  static member_lval LvalNew(ArrayData*, bool copy);
  static member_lval LvalNewRef(ArrayData*, bool copy);
  static ArrayData* SetInt(ArrayData*, int64_t k, Cell v, bool copy);
  static ArrayData* SetStr(ArrayData*, StringData* k, Cell v, bool copy);
  static ArrayData* SetWithRefInt(ArrayData*, int64_t k,
                                  TypedValue v, bool copy);
  static ArrayData* SetWithRefStr(ArrayData*, StringData* k,
                                  TypedValue v, bool copy);
  static ArrayData* SetRefInt(ArrayData*, int64_t k, Variant& v, bool copy);
  static ArrayData* SetRefStr(ArrayData*, StringData* k, Variant& v,
                              bool copy);
  static constexpr auto AddInt = &SetInt;
  static constexpr auto AddStr = &SetStr;
  static ArrayData *RemoveInt(ArrayData* ad, int64_t k, bool copy);
  static ArrayData *RemoveStr(ArrayData* ad, const StringData* k, bool copy);
  static ArrayData* Copy(const ArrayData*);
  static ArrayData* Append(ArrayData* a, Cell v, bool copy);
  static ArrayData* AppendRef(ArrayData*, Variant& v, bool copy);
  static ArrayData* AppendWithRef(ArrayData*, TypedValue v, bool copy);
  static ArrayData* PlusEq(ArrayData*, const ArrayData *elems);
  static ArrayData* Merge(ArrayData*, const ArrayData *elems);
  static ArrayData* Prepend(ArrayData*, Cell v, bool copy);
  static member_rval::ptr_u NvGetInt(const ArrayData*, int64_t k);
  static constexpr auto NvTryGetInt = &NvGetInt;
  static member_rval::ptr_u NvGetStr(const ArrayData*, const StringData* k);
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
  static bool IsVectorData(const ArrayData* ad);
  static ssize_t IterBegin(const ArrayData*);
  static ssize_t IterLast(const ArrayData*);
  static ssize_t IterEnd(const ArrayData*);
  static ssize_t IterAdvance(const ArrayData*, ssize_t prev);
  static ssize_t IterRewind(const ArrayData*, ssize_t prev);
  static bool ValidMArrayIter(const ArrayData*, const MArrayIter& fp);
  static bool AdvanceMArrayIter(ArrayData*, MArrayIter& fp);
  static ArrayData* CopyStatic(const ArrayData*);
  static constexpr auto Pop = &ArrayCommon::Pop;
  static constexpr auto Dequeue = &ArrayCommon::Dequeue;
  static void Renumber(ArrayData*);
  static void OnSetEvalScalar(ArrayData*);
  static void Release(ArrayData*);
  static ArrayData* Escalate(const ArrayData*);
  static ArrayData* EscalateForSort(ArrayData*, SortFunction);
  static void Ksort(ArrayData*, int, bool);
  static void Sort(ArrayData*, int, bool);
  static void Asort(ArrayData*, int, bool);
  static bool Uksort(ArrayData*, const Variant&);
  static bool Usort(ArrayData*, const Variant&);
  static bool Uasort(ArrayData*, const Variant&);
  static ArrayData* ToPHPArray(ArrayData* ad, bool) {
    return ad;
  }
  static constexpr auto ToVec = &ArrayCommon::ToVec;
  static constexpr auto ToDict = &ArrayCommon::ToDict;
  static constexpr auto ToKeyset = &ArrayCommon::ToKeyset;
  static constexpr auto ToVArray = &ArrayCommon::ToVArray;

public:
  using ArrayData::decRefCount;
  using ArrayData::hasMultipleRefs;
  using ArrayData::hasExactlyOneRef;
  using ArrayData::decWillRelease;
  using ArrayData::incRefCount;

  ssize_t iterAdvanceImpl(ssize_t prev) const {
    assert(prev >= 0 && prev < m_size);
    ssize_t next = prev + 1;
    return next < m_size ? next : m_size;
  }

  // Only explicit conversions are allowed to and from ArrayData*.
  ArrayData* asArrayData() { return this; }
  const ArrayData* asArrayData() const { return this; }

  // Pre: ad->isApcArray()
  static APCLocalArray* asApcArray(ArrayData*);
  static const APCLocalArray* asApcArray(const ArrayData*);

  void scan(type_scan::Scanner& scanner) const;
  size_t heapSize() const;

private:
  explicit APCLocalArray(const APCArray* source);
  ~APCLocalArray() = delete;

  static bool checkInvariants(const ArrayData*);
  ssize_t getIndex(int64_t k) const;
  ssize_t getIndex(const StringData* k) const;
  ArrayData* loadElems() const;
  Variant getKey(ssize_t pos) const;
  void sweep();
  TypedValue* localCache() const;

private:
  const APCArray* m_arr;
  unsigned m_sweep_index;
  friend struct MemoryManager; // access to m_sweep_index
};

//////////////////////////////////////////////////////////////////////

}

#endif
