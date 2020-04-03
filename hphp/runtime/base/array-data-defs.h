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

#include "hphp/runtime/base/tv-val.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/type-variant.h"

#include <algorithm>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

extern const StaticString s_InvalidKeysetOperationMsg;
extern const StaticString s_VecUnsetMsg;

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

///////////////////////////////////////////////////////////////////////////////
// ArrayFunction dispatch.

inline bool ArrayData::notCyclic(TypedValue v) const {
  return !tvIsArrayLike(v) || v.m_data.parr != this;
}

inline ArrayData* ArrayData::copy() const {
  return g_array_funcs.copy[kind()](this);
}

inline ArrayData* ArrayData::copyStatic() const {
  auto ret = g_array_funcs.copyStatic[kind()](this);
  assertx(ret != this && ret->isStatic());
  return ret;
}

inline ArrayData* ArrayData::toPHPArray(bool copy) {
  return g_array_funcs.toPHPArray[kind()](this, copy);
}

inline ArrayData* ArrayData::toPHPArrayIntishCast(bool copy) {
  return g_array_funcs.toPHPArrayIntishCast[kind()](this, copy);
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

inline ArrayData* ArrayData::toDArray(bool copy) {
  return g_array_funcs.toDArray[kind()](this, copy);
}

inline bool ArrayData::isVectorData() const {
  return g_array_funcs.isVectorData[kind()](this);
}

inline void ArrayData::release() noexcept {
  assertx(!hasMultipleRefs());
  g_array_funcs.release[kind()](this);
  AARCH64_WALKABLE_FRAME();
}

inline bool ArrayData::exists(int64_t k) const {
  return g_array_funcs.existsInt[kind()](this, k);
}

inline bool ArrayData::exists(const StringData* k) const {
  return g_array_funcs.existsStr[kind()](this, k);
}

inline arr_lval ArrayData::lval(int64_t k, bool copy) {
  return g_array_funcs.lvalInt[kind()](this, k, copy);
}

inline arr_lval ArrayData::lval(StringData* k, bool copy) {
  return g_array_funcs.lvalStr[kind()](this, k, copy);
}

inline tv_rval ArrayData::rval(int64_t k) const {
  return g_array_funcs.nvGetInt[kind()](this, k);
}

inline tv_rval ArrayData::rval(const StringData* k) const {
  return g_array_funcs.nvGetStr[kind()](this, k);
}

inline ssize_t ArrayData::nvGetIntPos(int64_t k) const {
  return g_array_funcs.nvGetIntPos[kind()](this, k);
}

inline ssize_t ArrayData::nvGetStrPos(const StringData* k) const {
  return g_array_funcs.nvGetStrPos[kind()](this, k);
}

inline TypedValue ArrayData::nvGetKey(ssize_t pos) const {
  return g_array_funcs.getPosKey[kind()](this, pos);
}

inline TypedValue ArrayData::nvGetVal(ssize_t pos) const {
  return g_array_funcs.getPosVal[kind()](this, pos);
}

inline Variant ArrayData::getKey(ssize_t pos) const {
  return Variant::wrap(nvGetKey(pos));
}

inline Variant ArrayData::getValue(ssize_t pos) const {
  return Variant::wrap(nvGetVal(pos));
}

inline ArrayData* ArrayData::set(int64_t k, TypedValue v) {
  assertx(tvIsPlausible(v));
  assertx(cowCheck() || notCyclic(v));
  return g_array_funcs.setInt[kind()](this, k, v);
}

inline ArrayData* ArrayData::setMove(int64_t k, TypedValue v) {
  assertx(tvIsPlausible(v));
  assertx(cowCheck() || notCyclic(v));
  return g_array_funcs.setIntMove[kind()](this, k, v);
}

inline ArrayData* ArrayData::set(StringData* k, TypedValue v) {
  assertx(tvIsPlausible(v));
  assertx(cowCheck() || notCyclic(v));
  return g_array_funcs.setStr[kind()](this, k, v);
}

inline ArrayData* ArrayData::setMove(StringData* k, TypedValue v) {
  assertx(tvIsPlausible(v));
  assertx(cowCheck() || notCyclic(v));
  return g_array_funcs.setStrMove[kind()](this, k, v);
}

inline ArrayData* ArrayData::set(int64_t k, const Variant& v) {
  auto c = *v.asTypedValue();
  assertx(cowCheck() || notCyclic(c));
  return g_array_funcs.setInt[kind()](this, k, c);
}

inline ArrayData* ArrayData::set(StringData* k, const Variant& v) {
  auto c = *v.asTypedValue();
  assertx(cowCheck() || notCyclic(c));
  return g_array_funcs.setStr[kind()](this, k, c);
}

inline ArrayData* ArrayData::remove(int64_t k) {
  return g_array_funcs.removeInt[kind()](this, k);
}

inline ArrayData* ArrayData::remove(const StringData* k) {
  return g_array_funcs.removeStr[kind()](this, k);
}

inline ArrayData* ArrayData::append(TypedValue v) {
  assertx(v.m_type != KindOfUninit);
  assertx(cowCheck() || notCyclic(v));
  return g_array_funcs.append[kind()](this, v);
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

inline ArrayData* ArrayData::plusEq(const ArrayData* elms) {
  return g_array_funcs.plusEq[kind()](this, elms);
}

inline ArrayData* ArrayData::merge(const ArrayData* elms) {
  auto ret = g_array_funcs.merge[kind()](this, elms);
  assertx(ret->isPHPArrayType());
  assertx(ret->isNotDVArray());
  return ret;
}

inline ArrayData* ArrayData::pop(Variant& value) {
  return g_array_funcs.pop[kind()](this, value);
}

inline ArrayData* ArrayData::dequeue(Variant& value) {
  return g_array_funcs.dequeue[kind()](this, value);
}

inline ArrayData* ArrayData::prepend(TypedValue v) {
  assertx(v.m_type != KindOfUninit);
  return g_array_funcs.prepend[kind()](this, v);
}

inline void ArrayData::onSetEvalScalar() {
  return g_array_funcs.onSetEvalScalar[kind()](this);
}

inline void ArrayData::renumber() {
  return g_array_funcs.renumber[kind()](this);
}

///////////////////////////////////////////////////////////////////////////////

namespace detail {

inline bool isIntKey(TypedValue cell) {
  assertx(isIntType(cell.m_type) || isStringType(cell.m_type));
  return isIntType(cell.m_type);
}

inline int64_t getIntKey(TypedValue cell) {
  assertx(isIntType(cell.m_type));
  return cell.m_data.num;
}

inline StringData* getStringKey(TypedValue cell) {
  assertx(isStringType(cell.m_type));
  return cell.m_data.pstr;
}

}

///////////////////////////////////////////////////////////////////////////////
// Element manipulation.

inline bool ArrayData::exists(TypedValue k) const {
  assertx(IsValidKey(k));
  return detail::isIntKey(k) ? exists(detail::getIntKey(k))
                             : exists(detail::getStringKey(k));
}

inline arr_lval ArrayData::lval(TypedValue k, bool copy) {
  assertx(IsValidKey(k));
  return detail::isIntKey(k) ? lval(detail::getIntKey(k), copy)
                             : lval(detail::getStringKey(k), copy);
}

inline tv_rval ArrayData::get(int64_t k, bool error) const {
  auto const r = rval(k);
  return r ? r : getNotFound(k, error);
}

inline tv_rval ArrayData::get(const StringData* k, bool error) const {
  auto const r = rval(k);
  return r ? r : getNotFound(k, error);
}

inline tv_rval ArrayData::get(TypedValue k, bool error) const {
  assertx(IsValidKey(k));
  return detail::isIntKey(k) ? get(detail::getIntKey(k), error)
                             : get(detail::getStringKey(k), error);
}

inline TypedValue ArrayData::at(int64_t k) const {
  return rval(k).tv();
}

inline TypedValue ArrayData::at(const StringData* k) const {
  return rval(k).tv();
}

inline TypedValue ArrayData::at(TypedValue k) const {
  assertx(IsValidKey(k));
  return detail::isIntKey(k) ? at(detail::getIntKey(k))
                             : at(detail::getStringKey(k));
}

inline ArrayData* ArrayData::set(TypedValue k, TypedValue v) {
  assertx(tvIsPlausible(k));
  assertx(tvIsPlausible(v));
  assertx(IsValidKey(k));

  return detail::isIntKey(k) ? set(detail::getIntKey(k), v)
                             : set(detail::getStringKey(k), v);
}

inline ArrayData* ArrayData::remove(TypedValue k) {
  assertx(IsValidKey(k));
  return detail::isIntKey(k) ? remove(detail::getIntKey(k))
                             : remove(detail::getStringKey(k));
}

///////////////////////////////////////////////////////////////////////////////

inline bool ArrayData::exists(const String& k) const {
  assertx(IsValidKey(k));
  return exists(k.get());
}

inline bool ArrayData::exists(const Variant& k) const {
  return exists(*k.asTypedValue());
}

inline arr_lval ArrayData::lval(const String& k, bool copy) {
  assertx(IsValidKey(k));
  return lval(k.get(), copy);
}

inline arr_lval ArrayData::lval(const Variant& k, bool copy) {
  return lval(*k.asTypedValue(), copy);
}

inline tv_rval ArrayData::get(const String& k, bool error) const {
  assertx(IsValidKey(k));
  return get(k.get(), error);
}

inline tv_rval ArrayData::get(const Variant& k, bool error) const {
  return get(*k.asTypedValue(), error);
}

inline ArrayData* ArrayData::set(const String& k, TypedValue v) {
  assertx(tvIsPlausible(v));
  assertx(IsValidKey(k));
  return set(k.get(), v);
}

inline ArrayData* ArrayData::set(const String& k, const Variant& v) {
  return set(k, *v.asTypedValue());
}

inline ArrayData* ArrayData::set(const Variant& k, const Variant& v) {
  return set(*k.asTypedValue(), *v.asTypedValue());
}

inline ArrayData* ArrayData::remove(const String& k) {
  assertx(IsValidKey(k));
  return remove(k.get());
}

inline ArrayData* ArrayData::remove(const Variant& k) {
  return remove(*k.asTypedValue());
}

///////////////////////////////////////////////////////////////////////////////

template <IntishCast IC>
ALWAYS_INLINE bool ArrayData::convertKey(const StringData* key,
                                         int64_t& i) const {
  return IC == IntishCast::Cast &&
         key->isStrictlyInteger(i) &&
         useWeakKeys();
}

template <IntishCast IC>
ALWAYS_INLINE
folly::Optional<int64_t> tryIntishCast(const StringData* key) {
  int64_t i;
  if (UNLIKELY(IC == IntishCast::Cast &&
               key->isStrictlyInteger(i))) {
    return i;
  }
  return {};
}

///////////////////////////////////////////////////////////////////////////////

}

#endif
