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
#ifndef incl_HPHP_RUNTIME_VM_GLOBALS_ARRAY_H
#define incl_HPHP_RUNTIME_VM_GLOBALS_ARRAY_H

#include "hphp/runtime/base/array-common.h"
#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/member-val.h"
#include "hphp/runtime/vm/name-value-table.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Wrapper to provide a KindOfArray interface to a NameValueTable.  This is
 * used to expose a NameValueTable to php code, particularly for $GLOBALS, but
 * also for some internal generated-bytecode initialization routines (86pinit,
 * 86sinit).
 *
 * Some differences compared to normal php arrays:
 *
 *   - Does not behave as if it has value semantics (i.e., no COW).
 *
 *   - Iteration order is not specified.
 *
 *   - Non-string keys are not really supported.  (Integers are converted to
 *     strings.)
 *
 *   - size() is an O(N) operation.  (This is because of kNamedLocalDataType
 *     support in the underlying NameValueTable.)
 *
 *   - Append/prepend operations are not supported.
 *
 *   - Strong iterators "past the end" are not updated when new elements are
 *     added.  (Since iteration order is unspecified, this semantic would seem
 *     weird anyway.)
 *
 * This holds a pointer to a NameValueTable whose lifetime must be guaranteed
 * to outlast the lifetime of the GlobalsArray.  (The wrapper is
 * refcounted, as required by ArrayData, but the table pointed to is not.)
 */
struct GlobalsArray final : ArrayData, type_scan::MarkCountable<GlobalsArray> {
  explicit GlobalsArray(NameValueTable* tab);
  ~GlobalsArray() = delete;

  // We only allow explicit conversions to ArrayData.  Generally you
  // should not be talking to the GlobalsArray directly (see
  // php-globals.h).
  ArrayData* asArrayData() { return this; }
  const ArrayData* asArrayData() const { return this; }

public:
  static void Release(ArrayData*) {}
  static ArrayData* Copy(const ArrayData* ad) {
    return const_cast<ArrayData*>(ad);
  }
  static size_t Vsize(const ArrayData*);
  static Cell NvGetKey(const ArrayData* ad, ssize_t pos);
  static member_rval::ptr_u GetValueRef(const ArrayData*, ssize_t pos);

  static bool ExistsInt(const ArrayData* ad, int64_t k);
  static bool ExistsStr(const ArrayData* ad, const StringData* k);

  static member_rval::ptr_u NvGetInt(const ArrayData*, int64_t k);
  static constexpr auto NvTryGetInt = &NvGetInt;
  static member_rval::ptr_u NvGetStr(const ArrayData*, const StringData* k);
  static constexpr auto NvTryGetStr = &NvGetStr;

  static member_lval LvalInt(ArrayData*, int64_t k, bool copy);
  static constexpr auto LvalIntRef = &LvalInt;
  static member_lval LvalStr(ArrayData*, StringData* k, bool copy);
  static constexpr auto LvalStrRef = &LvalStr;
  static member_lval LvalNew(ArrayData*, bool copy);
  static constexpr auto LvalNewRef = &LvalNew;

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
  static ArrayData* RemoveInt(ArrayData*, int64_t k, bool copy);
  static ArrayData* RemoveStr(ArrayData*, const StringData* k, bool copy);

  static ArrayData* Append(ArrayData*, Cell v, bool copy);
  static ArrayData* AppendRef(ArrayData*, Variant& v, bool copy);
  static ArrayData* AppendWithRef(ArrayData*, TypedValue v, bool copy);

  static ArrayData* PlusEq(ArrayData*, const ArrayData* elems);
  static ArrayData* Merge(ArrayData*, const ArrayData* elems);
  static ArrayData* Prepend(ArrayData*, Cell v, bool copy);

  static ssize_t IterBegin(const ArrayData*);
  static ssize_t IterLast(const ArrayData*);
  static ssize_t IterEnd(const ArrayData*);
  static ssize_t IterAdvance(const ArrayData*, ssize_t prev);
  static ssize_t IterRewind(const ArrayData*, ssize_t prev);

  static bool ValidMArrayIter(const ArrayData*, const MArrayIter & fp);
  static bool AdvanceMArrayIter(ArrayData*, MArrayIter&);
  static bool IsVectorData(const ArrayData*);
  static ArrayData* CopyStatic(const ArrayData*);
  static constexpr auto Pop = &ArrayCommon::Pop;
  static constexpr auto Dequeue = &ArrayCommon::Dequeue;
  static void Renumber(ArrayData*);
  static void OnSetEvalScalar(ArrayData*);

  static ArrayData* EscalateForSort(ArrayData*, SortFunction sf);
  static void Ksort(ArrayData*, int sort_flags, bool ascending);
  static void Sort(ArrayData*, int sort_flags, bool ascending);
  static void Asort(ArrayData*, int sort_flags, bool ascending);
  static bool Uksort(ArrayData*, const Variant& cmp_function);
  static bool Usort(ArrayData*, const Variant& cmp_function);
  static bool Uasort(ArrayData*, const Variant& cmp_function);

  static ArrayData* Escalate(const ArrayData* ad) {
    return const_cast<ArrayData*>(ad);
  }

  static ArrayData* ToPHPArray(ArrayData* ad, bool) {
    return ad;
  }
  static constexpr auto ToVec = &ArrayCommon::ToVec;
  static constexpr auto ToDict = &ArrayCommon::ToDict;
  static constexpr auto ToKeyset = &ArrayCommon::ToKeyset;
  static constexpr auto ToVArray = &ArrayCommon::ToVArray;

private:
  static GlobalsArray* asGlobals(ArrayData* ad);
  static const GlobalsArray* asGlobals(const ArrayData* ad);

public:
  void scan(type_scan::Scanner& scanner) const {
    scanner.scan(m_tab);
  }

private:
  NameValueTable* const m_tab;
};

//////////////////////////////////////////////////////////////////////

/*
 * Gets our request-local global variables array.
 */
GlobalsArray* get_global_variables();

}

#endif
