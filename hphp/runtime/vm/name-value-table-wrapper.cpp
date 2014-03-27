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
#include "hphp/runtime/vm/name-value-table-wrapper.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/hphp-array-defs.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/runtime-error.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

NameValueTableWrapper::~NameValueTableWrapper() {}

inline NameValueTableWrapper* NameValueTableWrapper::asNVTW(ArrayData* ad) {
  assert(ad->kind() == kNvtwKind);
  return static_cast<NameValueTableWrapper*>(ad);
}

inline const NameValueTableWrapper*
NameValueTableWrapper::asNVTW(const ArrayData* ad) {
  assert(ad->kind() == kNvtwKind);
  return static_cast<const NameValueTableWrapper*>(ad);
}

size_t NameValueTableWrapper::Vsize(const ArrayData* ad) {
  // We need to iterate to find out the actual size, since
  // KindOfIndirect elements in the array may have been set to
  // KindOfUninit.
  auto a = asNVTW(ad);
  size_t count = 0;
  for (auto iter = IterBegin(a);
      iter != invalid_index;
      iter = IterAdvance(a, iter)) {
    ++count;
  }
  return count;
}

void NameValueTableWrapper::NvGetKey(const ArrayData* ad, TypedValue* out,
                                     ssize_t pos) {
  auto a = asNVTW(ad);
  NameValueTable::Iterator iter(a->m_tab, pos);
  if (iter.valid()) {
    auto k = iter.curKey();
    out->m_data.pstr = const_cast<StringData*>(k);
    out->m_type = KindOfString;
    k->incRefCount();
  } else {
    out->m_type = KindOfUninit;
  }
}

const Variant& NameValueTableWrapper::GetValueRef(const ArrayData* ad, ssize_t pos) {
  auto a = asNVTW(ad);
  NameValueTable::Iterator iter(a->m_tab, pos);
  return iter.valid() ? tvAsCVarRef(iter.curVal()) : null_variant;
}

bool
NameValueTableWrapper::ExistsInt(const ArrayData* ad, int64_t k) {
  return ExistsStr(ad, String(k).get());
}

bool
NameValueTableWrapper::ExistsStr(const ArrayData* ad, const StringData* k) {
  return asNVTW(ad)->m_tab->lookup(k) != nullptr;
}

TypedValue*
NameValueTableWrapper::NvGetStr(const ArrayData* ad, const StringData* k) {
  return asNVTW(ad)->m_tab->lookup(k);
}

TypedValue* NameValueTableWrapper::NvGetInt(const ArrayData* ad, int64_t k) {
  return asNVTW(ad)->m_tab->lookup(String(k).get());
}

ArrayData*
NameValueTableWrapper::LvalInt(ArrayData* ad, int64_t k, Variant*& ret,
                               bool copy) {
  return LvalStr(ad, String(k).get(), ret, copy);
}

ArrayData*
NameValueTableWrapper::LvalStr(ArrayData* ad, StringData* k, Variant*& ret,
                               bool copy) {
  auto a = asNVTW(ad);
  TypedValue* tv = a->m_tab->lookup(k);
  if (!tv) {
    TypedValue nulVal;
    tvWriteNull(&nulVal);
    tv = a->m_tab->set(k, &nulVal);
  }
  ret = &tvAsVariant(tv);
  return a;
}

ArrayData*
NameValueTableWrapper::LvalNew(ArrayData* ad, Variant*& ret, bool copy) {
  ret = &Variant::lvalBlackHole();
  return ad;
}

ArrayData* NameValueTableWrapper::SetInt(ArrayData* ad, int64_t k,
                                         const Variant& v, bool copy) {
  return SetStr(ad, String(k).get(), v, copy);
}

ArrayData* NameValueTableWrapper::SetStr(ArrayData* ad, StringData* k,
                                         const Variant& v, bool copy) {
  auto a = asNVTW(ad);
  tvAsVariant(a->m_tab->lookupAdd(k)).assignVal(v);
  return a;
}

ArrayData* NameValueTableWrapper::SetRefInt(ArrayData* ad, int64_t k,
                                            const Variant& v, bool copy) {
  return asNVTW(ad)->setRef(String(k).get(), v, copy);
}

ArrayData* NameValueTableWrapper::SetRefStr(ArrayData* ad, StringData* k,
                                            const Variant& v, bool copy) {
  auto a = asNVTW(ad);
  tvAsVariant(a->m_tab->lookupAdd(k)).assignRef(v);
  return a;
}

ArrayData*
NameValueTableWrapper::RemoveInt(ArrayData* ad, int64_t k, bool copy) {
  return RemoveStr(ad, String(k).get(), copy);
}

ArrayData*
NameValueTableWrapper::RemoveStr(ArrayData* ad, const StringData* k,
                                 bool copy) {
  auto a = asNVTW(ad);
  a->m_tab->unset(k);
  return a;
}

/*
 * The messages in the user-visible exceptions below claim we are
 * $GLOBALS, because the only user-visible NameValueTableWrapper array
 * is currently $GLOBALS.
 */

ArrayData*
NameValueTableWrapper::Append(ArrayData*, const Variant& v, bool copy) {
  throw NotImplementedException("append on $GLOBALS");
}

ArrayData*
NameValueTableWrapper::AppendRef(ArrayData*, const Variant& v, bool copy) {
  throw NotImplementedException("appendRef on $GLOBALS");
}

ArrayData*
NameValueTableWrapper::AppendWithRef(ArrayData*, const Variant& v, bool copy) {
  throw NotImplementedException("appendWithRef on $GLOBALS");
}

ArrayData*
NameValueTableWrapper::PlusEq(ArrayData*, const ArrayData* elems) {
  throw NotImplementedException("plus on $GLOBALS");
}

ArrayData*
NameValueTableWrapper::Merge(ArrayData*, const ArrayData* elems) {
  throw NotImplementedException("merge on $GLOBALS");
}

ArrayData*
NameValueTableWrapper::Prepend(ArrayData*, const Variant& v, bool copy) {
  throw NotImplementedException("prepend on $GLOBALS");
}

ssize_t NameValueTableWrapper::IterBegin(const ArrayData* ad) {
  auto a = asNVTW(ad);
  NameValueTable::Iterator iter(a->m_tab);
  return iter.toInteger();
}

ssize_t NameValueTableWrapper::IterEnd(const ArrayData* ad) {
  auto a = asNVTW(ad);
  return NameValueTable::Iterator::getEnd(a->m_tab).toInteger();
}

ssize_t NameValueTableWrapper::IterAdvance(const ArrayData* ad, ssize_t prev) {
  auto a = asNVTW(ad);
  NameValueTable::Iterator iter(a->m_tab, prev);
  iter.next();
  return iter.toInteger();
}

ssize_t NameValueTableWrapper::IterRewind(const ArrayData* ad, ssize_t prev) {
  auto a = asNVTW(ad);
  NameValueTable::Iterator iter(a->m_tab, prev);
  iter.prev();
  return iter.toInteger();
}

bool
NameValueTableWrapper::ValidMArrayIter(const ArrayData* ad,
                                       const MArrayIter & fp) {
  assert(fp.getContainer() == ad);
  auto a = asNVTW(ad);
  if (fp.getResetFlag()) return false;
  if (fp.m_pos == invalid_index) return false;
  NameValueTable::Iterator iter(a->m_tab, fp.m_pos);
  return iter.valid();
}

bool NameValueTableWrapper::AdvanceMArrayIter(ArrayData* ad, MArrayIter& fp) {
  auto a = asNVTW(ad);
  bool reset = fp.getResetFlag();
  NameValueTable::Iterator iter = reset ?
    NameValueTable::Iterator(a->m_tab) :
    NameValueTable::Iterator(a->m_tab, fp.m_pos);
  if (reset) {
    fp.setResetFlag(false);
  } else {
    if (!iter.valid()) {
      return false;
    }
    iter.next();
  }
  fp.m_pos = iter.toInteger();
  if (!iter.valid()) return false;
  // To conform to PHP behavior, we need to set the internal
  // cursor to point to the next element.
  iter.next();
  a->m_pos = iter.toInteger();
  return true;
}

ArrayData* NameValueTableWrapper::EscalateForSort(ArrayData* ad) {
  raise_warning("Sorting the $GLOBALS array is not supported");
  return ad;
}
void NameValueTableWrapper::Ksort(ArrayData*, int sort_flags, bool ascending) {}
void NameValueTableWrapper::Sort(ArrayData*, int sort_flags, bool ascending) {}
void NameValueTableWrapper::Asort(ArrayData*, int sort_flags, bool ascending) {}
bool NameValueTableWrapper::Uksort(ArrayData*, const Variant& cmp_function) {
  return true;
}
bool NameValueTableWrapper::Usort(ArrayData*, const Variant& cmp_function) {
  return true;
}
bool NameValueTableWrapper::Uasort(ArrayData*, const Variant& cmp_function) {
  return true;
}

bool NameValueTableWrapper::IsVectorData(const ArrayData*) {
  return false;
}

//////////////////////////////////////////////////////////////////////

}
