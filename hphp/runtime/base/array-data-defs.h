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

#ifndef incl_HPHP_ARRAY_DEFS_H_
#define incl_HPHP_ARRAY_DEFS_H_

#include "hphp/runtime/base/array-data.h"

#include <algorithm>

#include "hphp/runtime/base/type-variant.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
inline bool isIntKey(const Cell* cell) {
  return IS_INT_KEY_TYPE(cell->m_type);
}

inline int64_t getIntKey(const Cell* cell) {
  assert(cell->m_type == KindOfInt64);
  return cell->m_data.num;
}

inline StringData* getStringKey(const Cell* cell) {
  assert(IS_STRING_TYPE(cell->m_type));
  return cell->m_data.pstr;
}
}

inline bool ArrayData::exists(const String& k) const {
  assert(IsValidKey(k));
  return exists(k.get());
}

inline bool ArrayData::exists(const Variant& k) const {
  assert(IsValidKey(k));
  auto const cell = k.asCell();
  return isIntKey(cell) ? exists(getIntKey(cell))
                        : exists(getStringKey(cell));
}

inline const Variant& ArrayData::get(const String& k, bool error) const {
  assert(IsValidKey(k));
  return get(k.get(), error);
}

inline const Variant& ArrayData::get(int64_t k, bool error) const {
  auto tv = nvGet(k);
  return tv ? tvAsCVarRef(tv) : getNotFound(k, error);
}

inline const Variant& ArrayData::get(const StringData* k, bool error) const {
  auto tv = nvGet(k);
  return tv ? tvAsCVarRef(tv) : getNotFound(k, error);
}

inline ArrayData* ArrayData::lval(const String& k, Variant *&ret, bool copy) {
  assert(IsValidKey(k));
  return lval(k.get(), ret, copy);
}

inline ArrayData* ArrayData::lval(const Variant& k, Variant *&ret, bool copy) {
  assert(IsValidKey(k));
  auto const cell = k.asCell();
  return isIntKey(cell) ? lval(getIntKey(cell), ret, copy)
                        : lval(getStringKey(cell), ret, copy);
}

inline ArrayData* ArrayData::set(const String& k, const Variant& v,
                                 bool copy) {
  assert(IsValidKey(k));
  return set(k.get(), v, copy);
}

inline ArrayData* ArrayData::set(const Variant& k, const Variant& v, bool copy) {
  assert(IsValidKey(k));
  auto const cell = k.asCell();
  return isIntKey(cell) ? set(getIntKey(cell), v, copy)
                        : set(getStringKey(cell), v, copy);
}

inline ArrayData* ArrayData::setRef(const String& k, Variant& v, bool copy) {
  assert(IsValidKey(k));
  return setRef(k.get(), v, copy);
}

inline ArrayData* ArrayData::setRef(const Variant& k, Variant& v, bool copy) {
  assert(IsValidKey(k));
  auto const cell = k.asCell();
  return isIntKey(cell) ? setRef(getIntKey(cell), v, copy)
                        : setRef(getStringKey(cell), v, copy);
}

inline ArrayData* ArrayData::add(const String& k, const Variant& v, bool copy) {
  assert(IsValidKey(k));
  return add(k.get(), v, copy);
}

inline ArrayData* ArrayData::add(const Variant& k, const Variant& v, bool copy) {
  assert(IsValidKey(k));
  auto const cell = k.asCell();
  return isIntKey(cell) ? add(getIntKey(cell), v, copy)
                        : add(getStringKey(cell), v, copy);
}

inline ArrayData* ArrayData::remove(const String& k, bool copy) {
  assert(IsValidKey(k));
  return remove(k.get(), copy);
}

inline ArrayData* ArrayData::remove(const Variant& k, bool copy) {
  assert(IsValidKey(k));
  auto const cell = k.asCell();
  return isIntKey(cell) ? remove(getIntKey(cell), copy)
                        : remove(getStringKey(cell), copy);
}

inline Variant ArrayData::getValue(ssize_t pos) const {
  return getValueRef(pos);
}

inline Variant ArrayData::getKey(ssize_t pos) const {
  TypedValue tv;
  nvGetKey(&tv, pos);
  return std::move(tvAsVariant(&tv));
}

///////////////////////////////////////////////////////////////////////////////

inline void ArrayData::release() noexcept {
  assert(isArrayKind(m_hdr.kind));
  assert(hasExactlyOneRef());
  return g_array_funcs.release[kind()](this);
}

inline ArrayData* ArrayData::append(const Variant& v, bool copy) {
  return g_array_funcs.append[kind()](this, v, copy);
}

inline ArrayData* ArrayData::appendRef(Variant& v, bool copy) {
  return g_array_funcs.appendRef[kind()](this, v, copy);
}

inline ArrayData* ArrayData::appendWithRef(const Variant& v, bool copy) {
  return g_array_funcs.appendWithRef[kind()](this, v, copy);
}

inline const TypedValue* ArrayData::nvGet(int64_t ikey) const {
  return g_array_funcs.nvGetInt[kind()](this, ikey);
}

inline const TypedValue* ArrayData::nvGet(const StringData* skey) const {
  return g_array_funcs.nvGetStr[kind()](this, skey);
}

inline void ArrayData::nvGetKey(TypedValue* out, ssize_t pos) const {
  g_array_funcs.nvGetKey[kind()](this, out, pos);
}

inline ArrayData* ArrayData::set(int64_t k, const Variant& v, bool copy) {
  return g_array_funcs.setInt[kind()](this, k, *v.asCell(), copy);
}

inline ArrayData* ArrayData::set(StringData* k, const Variant& v, bool copy) {
  return g_array_funcs.setStr[kind()](this, k, *v.asCell(), copy);
}

inline ArrayData* ArrayData::zSet(int64_t k, RefData* v) {
  return g_array_funcs.zSetInt[kind()](this, k, v);
}

inline ArrayData* ArrayData::zSet(StringData* k, RefData* v) {
  return g_array_funcs.zSetStr[kind()](this, k, v);
}

inline ArrayData* ArrayData::zAppend(RefData* v, int64_t* key_ptr) {
  return g_array_funcs.zAppend[kind()](this, v, key_ptr);
}

inline size_t ArrayData::vsize() const {
  return g_array_funcs.vsize[kind()](this);
}

inline const Variant& ArrayData::getValueRef(ssize_t pos) const {
  return g_array_funcs.getValueRef[kind()](this, pos);
}

inline bool ArrayData::noCopyOnWrite() const {
  // GlobalsArray doesn't support COW.
  return kind() == kGlobalsKind;
}

inline bool ArrayData::isVectorData() const {
  return g_array_funcs.isVectorData[kind()](this);
}

inline bool ArrayData::exists(int64_t k) const {
  return g_array_funcs.existsInt[kind()](this, k);
}

inline bool ArrayData::exists(const StringData* k) const {
  return g_array_funcs.existsStr[kind()](this, k);
}

inline ArrayData* ArrayData::lval(int64_t k, Variant*& ret, bool copy) {
  return g_array_funcs.lvalInt[kind()](this, k, ret, copy);
}

inline ArrayData* ArrayData::lval(StringData* k, Variant*& ret, bool copy) {
  return g_array_funcs.lvalStr[kind()](this, k, ret, copy);
}

inline ArrayData* ArrayData::lvalNew(Variant*& ret, bool copy) {
  return g_array_funcs.lvalNew[kind()](this, ret, copy);
}

inline ArrayData* ArrayData::lvalNewRef(Variant*& ret, bool copy) {
  return g_array_funcs.lvalNewRef[kind()](this, ret, copy);
}

inline ArrayData* ArrayData::setRef(int64_t k, Variant& v, bool copy) {
  return g_array_funcs.setRefInt[kind()](this, k, v, copy);
}

inline ArrayData* ArrayData::setRef(StringData* k, Variant& v, bool copy) {
  return g_array_funcs.setRefStr[kind()](this, k, v, copy);
}

inline ArrayData* ArrayData::add(int64_t k, const Variant& v, bool copy) {
  return g_array_funcs.addInt[kind()](this, k, *v.asCell(), copy);
}

inline ArrayData* ArrayData::add(StringData* k, const Variant& v, bool copy) {
  return g_array_funcs.addStr[kind()](this, k, *v.asCell(), copy);
}

inline ArrayData* ArrayData::remove(int64_t k, bool copy) {
  return g_array_funcs.removeInt[kind()](this, k, copy);
}

inline ArrayData* ArrayData::remove(const StringData* k, bool copy) {
  return g_array_funcs.removeStr[kind()](this, k, copy);
}

inline ssize_t ArrayData::iter_begin() const {
  return g_array_funcs.iterBegin[kind()](this);
}

inline ssize_t ArrayData::iter_last() const {
  return g_array_funcs.iterLast[kind()](this);
}

inline ssize_t ArrayData::iter_end() const {
  return g_array_funcs.iterEnd[kind()](this);
}

inline ssize_t ArrayData::iter_advance(ssize_t pos) const {
  return g_array_funcs.iterAdvance[kind()](this, pos);
}

inline ssize_t ArrayData::iter_rewind(ssize_t pos) const {
  return g_array_funcs.iterRewind[kind()](this, pos);
}

inline bool ArrayData::validMArrayIter(const MArrayIter& fp) const {
  return g_array_funcs.validMArrayIter[kind()](this, fp);
}

inline bool ArrayData::advanceMArrayIter(MArrayIter& fp) {
  return g_array_funcs.advanceMArrayIter[kind()](this, fp);
}

inline ArrayData* ArrayData::escalateForSort(SortFunction sf) {
  return g_array_funcs.escalateForSort[kind()](this, sf);
}

inline void ArrayData::ksort(int sort_flags, bool ascending) {
  return g_array_funcs.ksort[kind()](this, sort_flags, ascending);
}

inline void ArrayData::sort(int sort_flags, bool ascending) {
  return g_array_funcs.sort[kind()](this, sort_flags, ascending);
}

inline void ArrayData::asort(int sort_flags, bool ascending) {
  return g_array_funcs.asort[kind()](this, sort_flags, ascending);
}

inline bool ArrayData::uksort(const Variant& compare) {
  return g_array_funcs.uksort[kind()](this, compare);
}

inline bool ArrayData::usort(const Variant& compare) {
  return g_array_funcs.usort[kind()](this, compare);
}

inline bool ArrayData::uasort(const Variant& compare) {
  return g_array_funcs.uasort[kind()](this, compare);
}

inline ArrayData* ArrayData::copy() const {
  return g_array_funcs.copy[kind()](this);
}

inline ArrayData* ArrayData::copyWithStrongIterators() const {
  return g_array_funcs.copyWithStrongIterators[kind()](this);
}

inline ArrayData* ArrayData::copyStatic() const {
  auto ret = g_array_funcs.copyStatic[kind()](this);
  assert(ret != this && ret->hasExactlyOneRef());
  return ret;
}

inline ArrayData* ArrayData::pop(Variant& value) {
  return g_array_funcs.pop[kind()](this, value);
}

inline ArrayData* ArrayData::dequeue(Variant& value) {
  return g_array_funcs.dequeue[kind()](this, value);
}

inline ArrayData* ArrayData::prepend(const Variant& value, bool copy) {
  return g_array_funcs.prepend[kind()](this, value, copy);
}

inline void ArrayData::renumber() {
  return g_array_funcs.renumber[kind()](this);
}

inline void ArrayData::onSetEvalScalar() {
  return g_array_funcs.onSetEvalScalar[kind()](this);
}

inline ArrayData* ArrayData::escalate() const {
  return g_array_funcs.escalate[kind()](this);
}

inline ArrayData* ArrayData::plusEq(const ArrayData* elms) {
  return g_array_funcs.plusEq[kind()](this, elms);
}

inline ArrayData* ArrayData::merge(const ArrayData* elms) {
  return g_array_funcs.merge[kind()](this, elms);
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_ARRAY_DEFS_H_
