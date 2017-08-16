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

#ifndef incl_HPHP_ARRAY_DATA_DEFS_H_
#define incl_HPHP_ARRAY_DATA_DEFS_H_

#include "hphp/runtime/base/array-data.h"

#include "hphp/runtime/base/member-val.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/type-variant.h"

#include <algorithm>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

extern const StaticString s_InvalidKeysetOperationMsg;
extern const StaticString s_VecUnsetMsg;

namespace detail {

inline bool isIntKey(Cell cell) {
  assert(isIntType(cell.m_type) || isStringType(cell.m_type));
  return isIntType(cell.m_type);
}

inline int64_t getIntKey(Cell cell) {
  assert(isIntType(cell.m_type));
  return cell.m_data.num;
}

inline StringData* getStringKey(Cell cell) {
  assert(isStringType(cell.m_type));
  return cell.m_data.pstr;
}

}

///////////////////////////////////////////////////////////////////////////////

inline ArrayData* ArrayData::Create(const Variant& value) {
  return Create(*value.asTypedValue());
}

inline ArrayData* ArrayData::Create(const Variant& name, TypedValue value) {
  return Create(*name.asTypedValue(), value);
}

inline ArrayData* ArrayData::Create(const Variant& name, const Variant& value) {
  return Create(*name.asTypedValue(), *value.asTypedValue());
}

inline ArrayData* ArrayData::CreateWithRef(const Variant& name,
                                           TypedValue value) {
  return CreateWithRef(*name.asTypedValue(), value);
}

inline ArrayData* ArrayData::CreateRef(const Variant& name, Variant& value) {
  return CreateRef(*name.asTypedValue(), value);
}

ALWAYS_INLINE bool ArrayData::convertKey(const StringData* key,
                                         int64_t& i,
                                         bool notice) const {
  auto const result = key->isStrictlyInteger(i) && useWeakKeys();
  if (UNLIKELY(result && notice)) raise_intish_index_cast();
  return result;
}

inline bool ArrayData::exists(Cell k) const {
  assert(IsValidKey(k));
  return detail::isIntKey(k) ? exists(detail::getIntKey(k))
                             : exists(detail::getStringKey(k));
}

inline bool ArrayData::exists(const String& k) const {
  assert(IsValidKey(k));
  return exists(k.get());
}

inline bool ArrayData::exists(const Variant& k) const {
  return exists(*k.asCell());
}

inline const Variant& ArrayData::get(Cell k, bool error) const {
  assert(IsValidKey(k));
  return detail::isIntKey(k) ? get(detail::getIntKey(k), error)
                             : get(detail::getStringKey(k), error);
}

inline const Variant& ArrayData::get(int64_t k, bool error) const {
  auto r = error ? rvalStrict(k) : rval(k);
  return r ? tvAsCVarRef(r.tv_ptr()) : getNotFound(k, error);
}

inline const Variant& ArrayData::get(const StringData* k, bool error) const {
  auto r = error ? rvalStrict(k) : rval(k);
  return r ? tvAsCVarRef(r.tv_ptr()) : getNotFound(k, error);
}

inline const Variant& ArrayData::get(const String& k, bool error) const {
  assert(IsValidKey(k));
  return get(k.get(), error);
}

inline const Variant& ArrayData::get(const Variant& k, bool error) const {
  return get(*k.asCell(), error);
}

inline member_lval ArrayData::lval(const String& k, bool copy) {
  assert(IsValidKey(k));
  return lval(k.get(), copy);
}

inline member_lval ArrayData::lval(Cell k, bool copy) {
  assert(IsValidKey(k));
  return detail::isIntKey(k) ? lval(detail::getIntKey(k), copy)
                             : lval(detail::getStringKey(k), copy);
}

inline member_lval ArrayData::lval(const Variant& k, bool copy) {
  return lval(*k.asCell(), copy);
}

inline member_lval ArrayData::lvalRef(const String& k, bool copy) {
  assert(IsValidKey(k));
  return lvalRef(k.get(), copy);
}

inline member_lval ArrayData::lvalRef(Cell k, bool copy) {
  assert(IsValidKey(k));
  return detail::isIntKey(k) ? lvalRef(detail::getIntKey(k), copy)
                             : lvalRef(detail::getStringKey(k), copy);
}

inline member_lval ArrayData::lvalRef(const Variant& k, bool copy) {
  return lvalRef(*k.asCell(), copy);
}

inline ArrayData* ArrayData::set(Cell k, Cell v, bool copy) {
  assertx(cellIsPlausible(k));
  assertx(cellIsPlausible(v));
  assertx(IsValidKey(k));

  return detail::isIntKey(k) ? set(detail::getIntKey(k), v, copy)
                             : set(detail::getStringKey(k), v, copy);
}

inline ArrayData* ArrayData::set(const String& k, Cell v, bool copy) {
  assertx(cellIsPlausible(v));
  assertx(IsValidKey(k));
  return set(k.get(), v, copy);
}

inline ArrayData* ArrayData::set(const String& k, const Variant& v,
                                 bool copy) {
  return set(k, *v.asCell(), copy);
}

inline ArrayData* ArrayData::set(const Variant& k, const Variant& v,
                                 bool copy) {
  return set(*k.asCell(), *v.asCell(), copy);
}

inline ArrayData* ArrayData::setWithRef(Cell k, TypedValue v, bool copy) {
  assertx(cellIsPlausible(k));
  assertx(tvIsPlausible(v));
  assertx(IsValidKey(k));

  return detail::isIntKey(k) ? setWithRef(detail::getIntKey(k), v, copy)
                             : setWithRef(detail::getStringKey(k), v, copy);
}

inline ArrayData* ArrayData::setWithRef(const String& k,
                                        TypedValue v, bool copy) {
  assertx(tvIsPlausible(v));
  assertx(IsValidKey(k));
  return setWithRef(k.get(), v, copy);
}

inline ArrayData* ArrayData::setRef(Cell k, Variant& v, bool copy) {
  assert(IsValidKey(k));
  return detail::isIntKey(k) ? setRef(detail::getIntKey(k), v, copy)
                             : setRef(detail::getStringKey(k), v, copy);
}

inline ArrayData* ArrayData::setRef(const String& k, Variant& v, bool copy) {
  assert(IsValidKey(k));
  return setRef(k.get(), v, copy);
}

inline ArrayData* ArrayData::setRef(const Variant& k, Variant& v, bool copy) {
  return setRef(*k.asCell(), v, copy);
}

inline ArrayData* ArrayData::add(Cell k, Cell v, bool copy) {
  assertx(cellIsPlausible(k));
  assertx(cellIsPlausible(v));
  assertx(IsValidKey(k));

  return detail::isIntKey(k) ? add(detail::getIntKey(k), v, copy)
                             : add(detail::getStringKey(k), v, copy);
}

inline ArrayData* ArrayData::add(const String& k, Cell v, bool copy) {
  assertx(cellIsPlausible(v));
  assert(IsValidKey(k));
  return add(k.get(), v, copy);
}

inline ArrayData* ArrayData::add(const String& k, const Variant& v,
                                 bool copy) {
  return add(k, *v.asCell(), copy);
}

inline ArrayData* ArrayData::add(const Variant& k, const Variant& v,
                                 bool copy) {
  return add(*k.asCell(), *v.asCell(), copy);
}

inline ArrayData* ArrayData::remove(Cell k, bool copy) {
  assert(IsValidKey(k));
  return detail::isIntKey(k) ? remove(detail::getIntKey(k), copy)
                             : remove(detail::getStringKey(k), copy);
}

inline ArrayData* ArrayData::remove(const String& k, bool copy) {
  assert(IsValidKey(k));
  return remove(k.get(), copy);
}

inline ArrayData* ArrayData::remove(const Variant& k, bool copy) {
  return remove(*k.asCell(), copy);
}

inline Variant ArrayData::getValue(ssize_t pos) const {
  return tvAsCVarRef(rvalPos(pos).tv_ptr());
}

inline Variant ArrayData::getKey(ssize_t pos) const {
  auto key = nvGetKey(pos);
  return std::move(cellAsVariant(key));
}

///////////////////////////////////////////////////////////////////////////////

inline void ArrayData::release() noexcept {
  assert(hasExactlyOneRef());
  return g_array_funcs.release[kind()](this);
}

inline ArrayData* ArrayData::append(Cell v, bool copy) {
  assertx(v.m_type != KindOfUninit);
  return g_array_funcs.append[kind()](this, v, copy);
}

inline ArrayData* ArrayData::appendRef(Variant& v, bool copy) {
  return g_array_funcs.appendRef[kind()](this, v, copy);
}

inline ArrayData* ArrayData::appendWithRef(TypedValue v, bool copy) {
  return g_array_funcs.appendWithRef[kind()](this, v, copy);
}

inline ArrayData* ArrayData::appendWithRef(const Variant& v, bool copy) {
  return g_array_funcs.appendWithRef[kind()](this, *v.asTypedValue(), copy);
}

inline member_rval ArrayData::rval(int64_t k) const {
  return member_rval { this, g_array_funcs.nvGetInt[kind()](this, k) };
}

inline member_rval ArrayData::rvalStrict(int64_t k) const {
  return member_rval { this, g_array_funcs.nvTryGetInt[kind()](this, k) };
}

inline member_rval ArrayData::rval(const StringData* k) const {
  return member_rval { this, g_array_funcs.nvGetStr[kind()](this, k) };
}

inline member_rval ArrayData::rvalStrict(const StringData* k) const {
  return member_rval { this, g_array_funcs.nvTryGetStr[kind()](this, k) };
}

inline member_rval ArrayData::rvalPos(ssize_t pos) const {
  return member_rval { this, g_array_funcs.nvGetPos[kind()](this, pos) };
}

inline TypedValue ArrayData::at(int64_t k) const {
  return rval(k).tv();
}

inline TypedValue ArrayData::at(const StringData* k) const {
  return rval(k).tv();
}

inline TypedValue ArrayData::atPos(ssize_t pos) const {
  return rvalPos(pos).tv();
}

inline Cell ArrayData::nvGetKey(ssize_t pos) const {
  return g_array_funcs.nvGetKey[kind()](this, pos);
}

inline ArrayData* ArrayData::set(int64_t k, const Variant& v, bool copy) {
  return g_array_funcs.setInt[kind()](this, k, *v.asCell(), copy);
}

inline ArrayData* ArrayData::set(StringData* k, const Variant& v, bool copy) {
  return g_array_funcs.setStr[kind()](this, k, *v.asCell(), copy);
}

inline ArrayData* ArrayData::set(int64_t k, Cell v, bool copy) {
  assertx(cellIsPlausible(v));
  return g_array_funcs.setInt[kind()](this, k, v, copy);
}

inline ArrayData* ArrayData::set(StringData* k, Cell v, bool copy) {
  assertx(cellIsPlausible(v));
  return g_array_funcs.setStr[kind()](this, k, v, copy);
}

inline ArrayData* ArrayData::setWithRef(int64_t k, TypedValue v, bool copy) {
  assertx(tvIsPlausible(v));
  return g_array_funcs.setWithRefInt[kind()](this, k, v, copy);
}

inline ArrayData* ArrayData::setWithRef(StringData* k,
                                        TypedValue v, bool copy) {
  assertx(tvIsPlausible(v));
  return g_array_funcs.setWithRefStr[kind()](this, k, v, copy);
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

inline member_lval ArrayData::lval(int64_t k, bool copy) {
  return g_array_funcs.lvalInt[kind()](this, k, copy);
}

inline member_lval ArrayData::lvalRef(int64_t k, bool copy) {
  return g_array_funcs.lvalIntRef[kind()](this, k, copy);
}

inline member_lval ArrayData::lval(StringData* k, bool copy) {
  return g_array_funcs.lvalStr[kind()](this, k, copy);
}

inline member_lval ArrayData::lvalRef(StringData* k, bool copy) {
  return g_array_funcs.lvalStrRef[kind()](this, k, copy);
}

inline member_lval ArrayData::lvalNew(bool copy) {
  return g_array_funcs.lvalNew[kind()](this, copy);
}

inline member_lval ArrayData::lvalNewRef(bool copy) {
  return g_array_funcs.lvalNewRef[kind()](this, copy);
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

inline ArrayData* ArrayData::add(int64_t k, Cell v, bool copy) {
  assertx(cellIsPlausible(v));
  return g_array_funcs.addInt[kind()](this, k, v, copy);
}

inline ArrayData* ArrayData::add(StringData* k, Cell v, bool copy) {
  assertx(cellIsPlausible(v));
  return g_array_funcs.addStr[kind()](this, k, v, copy);
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

inline ArrayData* ArrayData::copyStatic() const {
  auto ret = g_array_funcs.copyStatic[kind()](this);
  assert(ret != this && ret->isStatic());
  return ret;
}

inline ArrayData* ArrayData::pop(Variant& value) {
  return g_array_funcs.pop[kind()](this, value);
}

inline ArrayData* ArrayData::dequeue(Variant& value) {
  return g_array_funcs.dequeue[kind()](this, value);
}

inline ArrayData* ArrayData::prepend(Cell v, bool copy) {
  assertx(v.m_type != KindOfUninit);
  return g_array_funcs.prepend[kind()](this, v, copy);
}

inline ArrayData* ArrayData::toPHPArray(bool copy) {
  return g_array_funcs.toPHPArray[kind()](this, copy);
}

inline ArrayData* ArrayData::toDict(bool copy) {
  return g_array_funcs.toDict[kind()](this, copy);
}

inline ArrayData* ArrayData::toVec(bool copy) {
  return g_array_funcs.toVec[kind()](this, copy);
}

inline ArrayData* ArrayData::toKeyset(bool copy) {
  return g_array_funcs.toKeyset[kind()](this, copy);
}

inline ArrayData* ArrayData::toVArray(bool copy) {
  return g_array_funcs.toVArray[kind()](this, copy);
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

#endif
