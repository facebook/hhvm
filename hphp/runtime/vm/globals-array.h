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
#include "hphp/runtime/base/tv-val.h"
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
 *   - keys(),size() are O(N) operation. (This is because of kNamedLocalDataType
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
struct GlobalsArray final : ArrayData,
                            type_scan::MarkCollectable<GlobalsArray> {
  explicit GlobalsArray(NameValueTable* tab);
  ~GlobalsArray() = delete;

public:
  ArrayData* keys();
  bool keyExists(const StringData*);

  static void Release(ArrayData*) {}
  static ArrayData* Copy(const ArrayData* ad) {
    return const_cast<ArrayData*>(ad);
  }
  static size_t Vsize(const ArrayData*);
  static TypedValue GetPosKey(const ArrayData* ad, ssize_t pos);
  static TypedValue GetPosVal(const ArrayData* ad, ssize_t pos);

  static bool ExistsInt(const ArrayData* ad, int64_t k);
  static bool ExistsStr(const ArrayData* ad, const StringData* k);

  static tv_rval NvGetInt(const ArrayData*, int64_t k);
  static tv_rval NvGetStr(const ArrayData*, const StringData* k);

  static ssize_t NvGetIntPos(const ArrayData*, int64_t k);
  static ssize_t NvGetStrPos(const ArrayData*, const StringData* k);

  static arr_lval LvalInt(ArrayData*, int64_t k, bool copy);
  static arr_lval LvalStr(ArrayData*, StringData* k, bool copy);

  static ArrayData* SetInt(ArrayData*, int64_t k, TypedValue v);
  static ArrayData* SetIntMove(ArrayData*, int64_t k, TypedValue v);
  static ArrayData* SetStr(ArrayData*, StringData* k, TypedValue v);
  static ArrayData* SetStrMove(ArrayData*, StringData* k, TypedValue v);
  static ArrayData* RemoveInt(ArrayData*, int64_t k);
  static ArrayData* RemoveStr(ArrayData*, const StringData* k);
  static ArrayData* Append(ArrayData*, TypedValue v);
  static ArrayData* PlusEq(ArrayData*, const ArrayData* elems);
  static ArrayData* Merge(ArrayData*, const ArrayData* elems);
  static ArrayData* Prepend(ArrayData*, TypedValue v);

  static ssize_t IterBegin(const ArrayData*);
  static ssize_t IterLast(const ArrayData*);
  static ssize_t IterEnd(const ArrayData*);
  static ssize_t IterAdvance(const ArrayData*, ssize_t prev);
  static ssize_t IterRewind(const ArrayData*, ssize_t prev);

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

  static ArrayData* ToPHPArray(ArrayData* ad, bool) {
    return ad;
  }
  static constexpr auto ToPHPArrayIntishCast = &ToPHPArray;
  static constexpr auto ToVec = &ArrayCommon::ToVec;
  static constexpr auto ToDict = &ArrayCommon::ToDict;
  static constexpr auto ToKeyset = &ArrayCommon::ToKeyset;
  static constexpr auto ToVArray = &ArrayCommon::ToVArray;
  static constexpr auto ToDArray = &ArrayCommon::ToDArray;

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
