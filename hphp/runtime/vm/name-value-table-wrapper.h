/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_RUNTIME_VM_NAME_VALUE_TABLE_WRAPPER_H
#define incl_HPHP_RUNTIME_VM_NAME_VALUE_TABLE_WRAPPER_H

#include "hphp/runtime/vm/name-value-table.h"
#include "hphp/runtime/base/array-data.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Wrapper to provide a KindOfArray interface to a NameValueTable.
 * This is used to expose a NameValueTable to php code, particularly
 * for $GLOBALS, but also for some internal generated-bytecode
 * initialization routines (86pinit, 86sinit).
 *
 * Some differences compared to normal php arrays:
 *
 *   - Does not behave as if it has value semantics.  (I.e., no COW.)
 *
 *   - Iteration order is not specified.
 *
 *   - Non-string keys are not really supported.  (Integers are
 *     converted to strings.)
 *
 *   - size() is an O(N) operation.  (This is because of KindOfIndirect
 *     support in the underlying NameValueTable.)
 *
 *   - Append/prepend operations are not supported.
 *
 *   - Strong iterators "past the end" are not updated when new
 *     elements are added.  (Since iteration order is unspecified,
 *     this semantic would seem weird anyway.)
 *
 * This holds a pointer to a NameValueTable whose lifetime must be
 * guaranteed to outlast the lifetime of the NameValueTableWrapper.
 * (The wrapper is refcounted, as required by ArrayData, but the table
 * pointed to is not.)
 */
struct NameValueTableWrapper : public ArrayData {
  explicit NameValueTableWrapper(NameValueTable* tab)
    : ArrayData(kNvtwKind)
    , m_tab(tab)
  {}

public: // ArrayData implementation
  static void Release(ArrayData*) {}

  // These using directives ensure the full set of overloaded functions
  // are visible in this class, to avoid triggering implicit conversions.
  using ArrayData::exists;
  using ArrayData::lval;
  using ArrayData::lvalNew;
  using ArrayData::set;
  using ArrayData::setRef;
  using ArrayData::add;
  using ArrayData::remove;
  using ArrayData::nvGet;

  Variant& getRef(const String& k) {
    return tvAsVariant(nvGet(k.get()));
  }

  static size_t Vsize(const ArrayData*);
  static void NvGetKey(const ArrayData* ad, TypedValue* out, ssize_t pos);
  static CVarRef GetValueRef(const ArrayData*, ssize_t pos);

  static bool ExistsInt(const ArrayData* ad, int64_t k);
  static bool ExistsStr(const ArrayData* ad, const StringData* k);

  static TypedValue* NvGetInt(const ArrayData*, int64_t k);
  static TypedValue* NvGetStr(const ArrayData*, const StringData* k);

  static ArrayData* LvalInt(ArrayData*, int64_t k, Variant*& ret, bool copy);
  static ArrayData* LvalStr(ArrayData*, StringData* k, Variant*& ret,
                            bool copy);
  static ArrayData* LvalNew(ArrayData*, Variant*& ret, bool copy);

  static ArrayData* SetInt(ArrayData*, int64_t k, CVarRef v, bool copy);
  static ArrayData* SetStr(ArrayData*, StringData* k, CVarRef v, bool copy);
  static ArrayData* SetRefInt(ArrayData*, int64_t k, CVarRef v, bool copy);
  static ArrayData* SetRefStr(ArrayData*, StringData* k, CVarRef v, bool copy);
  static ArrayData* RemoveInt(ArrayData*, int64_t k, bool copy);
  static ArrayData* RemoveStr(ArrayData*, const StringData* k, bool copy);

  static ArrayData* Copy(const ArrayData* ad) {
    return const_cast<ArrayData*>(ad);
  }

  static ArrayData* Append(ArrayData*, CVarRef v, bool copy);
  static ArrayData* AppendRef(ArrayData*, CVarRef v, bool copy);
  static ArrayData* AppendWithRef(ArrayData*, CVarRef v, bool copy);

  static ArrayData* PlusEq(ArrayData*, const ArrayData* elems);
  static ArrayData* Merge(ArrayData*, const ArrayData* elems);
  static ArrayData* Prepend(ArrayData*, CVarRef v, bool copy);

  static ssize_t IterBegin(const ArrayData*);
  static ssize_t IterEnd(const ArrayData*);
  static ssize_t IterAdvance(const ArrayData*, ssize_t prev);
  static ssize_t IterRewind(const ArrayData*, ssize_t prev);

  static bool ValidFullPos(const ArrayData*, const FullPos & fp);
  static bool AdvanceFullPos(ArrayData*, FullPos&);
  static bool IsVectorData(const ArrayData*);

  static ArrayData* EscalateForSort(ArrayData*);
  static void Ksort(ArrayData*, int sort_flags, bool ascending);
  static void Sort(ArrayData*, int sort_flags, bool ascending);
  static void Asort(ArrayData*, int sort_flags, bool ascending);
  static bool Uksort(ArrayData*, CVarRef cmp_function);
  static bool Usort(ArrayData*, CVarRef cmp_function);
  static bool Uasort(ArrayData*, CVarRef cmp_function);

private:
  static NameValueTableWrapper* asNVTW(ArrayData* ad);
  static const NameValueTableWrapper* asNVTW(const ArrayData* ad);

private:
  NameValueTable* const m_tab;
};

class GlobalNameValueTableWrapper : public NameValueTableWrapper {
 public:
  explicit GlobalNameValueTableWrapper(NameValueTable* tab);
};

//////////////////////////////////////////////////////////////////////

}

#endif
