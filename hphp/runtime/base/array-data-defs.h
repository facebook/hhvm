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

inline ArrayData* ArrayData::CreateWithRef(const Variant& name,
                                           TypedValue value) {
  return CreateWithRef(*name.asTypedValue(), value);
}

inline ArrayData* ArrayData::CreateRef(const Variant& name, tv_lval value) {
  return CreateRef(*name.asTypedValue(), value);
}

///////////////////////////////////////////////////////////////////////////////
// ArrayFunction dispatch.

inline bool ArrayData::notCyclic(Cell v) const {
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

inline ArrayData* ArrayData::toShape(bool copy) {
  return g_array_funcs.toShape[kind()](this, copy);
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

inline ArrayData* ArrayData::escalate() const {
  return g_array_funcs.escalate[kind()](this);
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

inline arr_lval ArrayData::lvalRef(int64_t k, bool copy) {
  return g_array_funcs.lvalIntRef[kind()](this, k, copy);
}

inline arr_lval ArrayData::lvalRef(StringData* k, bool copy) {
  return g_array_funcs.lvalStrRef[kind()](this, k, copy);
}

inline arr_lval ArrayData::lvalNew(bool copy) {
  return g_array_funcs.lvalNew[kind()](this, copy);
}

inline arr_lval ArrayData::lvalNewRef(bool copy) {
  return g_array_funcs.lvalNewRef[kind()](this, copy);
}

inline tv_rval ArrayData::rval(int64_t k) const {
  return g_array_funcs.nvGetInt[kind()](this, k);
}

inline tv_rval ArrayData::rval(const StringData* k) const {
  return g_array_funcs.nvGetStr[kind()](this, k);
}

inline tv_rval ArrayData::rvalStrict(int64_t k) const {
  return g_array_funcs.nvTryGetInt[kind()](this, k);
}

inline tv_rval ArrayData::rvalStrict(const StringData* k) const {
  return g_array_funcs.nvTryGetStr[kind()](this, k);
}

inline tv_rval ArrayData::rvalPos(ssize_t pos) const {
  return g_array_funcs.nvGetPos[kind()](this, pos);
}

inline Cell ArrayData::nvGetKey(ssize_t pos) const {
  return g_array_funcs.nvGetKey[kind()](this, pos);
}

inline ArrayData* ArrayData::set(int64_t k, Cell v) {
  assertx(cellIsPlausible(v));
  assertx(cowCheck() || notCyclic(v));
  return g_array_funcs.setInt[kind()](this, k, v);
}

inline ArrayData* ArrayData::setInPlace(int64_t k, Cell v) {
  assertx(cellIsPlausible(v));
  assertx(notCyclic(v));
  return g_array_funcs.setIntInPlace[kind()](this, k, v);
}

inline ArrayData* ArrayData::set(StringData* k, Cell v) {
  assertx(cellIsPlausible(v));
  assertx(cowCheck() || notCyclic(v));
  return g_array_funcs.setStr[kind()](this, k, v);
}

inline ArrayData* ArrayData::setInPlace(StringData* k, Cell v) {
  assertx(cellIsPlausible(v));
  assertx(notCyclic(v));
  return g_array_funcs.setStrInPlace[kind()](this, k, v);
}

inline ArrayData* ArrayData::set(int64_t k, const Variant& v) {
  auto c = *v.toCell();
  assertx(cowCheck() || notCyclic(c));
  return g_array_funcs.setInt[kind()](this, k, c);
}

inline ArrayData* ArrayData::set(StringData* k, const Variant& v) {
  auto c = *v.toCell();
  assertx(cowCheck() || notCyclic(c));
  return g_array_funcs.setStr[kind()](this, k, c);
}

inline ArrayData* ArrayData::setInPlace(StringData* k, const Variant& v) {
  auto c = *v.toCell();
  assertx(notCyclic(c));
  return g_array_funcs.setStrInPlace[kind()](this, k, c);
}

inline ArrayData* ArrayData::setWithRef(int64_t k, TypedValue v) {
  assertx(tvIsPlausible(v));
  return g_array_funcs.setWithRefInt[kind()](this, k, v);
}

inline ArrayData* ArrayData::setWithRefInPlace(int64_t k, TypedValue v) {
  assertx(tvIsPlausible(v));
  return g_array_funcs.setWithRefIntInPlace[kind()](this, k, v);
}

inline ArrayData* ArrayData::setWithRef(StringData* k, TypedValue v) {
  assertx(tvIsPlausible(v));
  return g_array_funcs.setWithRefStr[kind()](this, k, v);
}

inline ArrayData* ArrayData::setWithRefInPlace(StringData* k, TypedValue v) {
  assertx(tvIsPlausible(v));
  return g_array_funcs.setWithRefStrInPlace[kind()](this, k, v);
}

inline ArrayData* ArrayData::remove(int64_t k) {
  return g_array_funcs.removeInt[kind()](this, k);
}

inline ArrayData* ArrayData::removeInPlace(int64_t k) {
  return g_array_funcs.removeIntInPlace[kind()](this, k);
}

inline ArrayData* ArrayData::remove(const StringData* k) {
  return g_array_funcs.removeStr[kind()](this, k);
}

inline ArrayData* ArrayData::removeInPlace(const StringData* k) {
  return g_array_funcs.removeStrInPlace[kind()](this, k);
}

inline ArrayData* ArrayData::append(Cell v) {
  assertx(v.m_type != KindOfUninit);
  assertx(cowCheck() || notCyclic(v));
  return g_array_funcs.append[kind()](this, v);
}

inline ArrayData* ArrayData::appendInPlace(Cell v) {
  assertx(v.m_type != KindOfUninit);
  assertx(notCyclic(v));
  return g_array_funcs.appendInPlace[kind()](this, v);
}

inline ArrayData* ArrayData::appendWithRef(TypedValue v) {
  return g_array_funcs.appendWithRef[kind()](this, v);
}

inline ArrayData* ArrayData::appendWithRefInPlace(TypedValue v) {
  return g_array_funcs.appendWithRefInPlace[kind()](this, v);
}

inline ArrayData* ArrayData::appendWithRef(const Variant& v) {
  return g_array_funcs.appendWithRef[kind()](this, *v.asTypedValue());
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
  assertx(ret->isPHPArray());
  assertx(ret->isNotDVArray());
  return ret;
}

inline ArrayData* ArrayData::pop(Variant& value) {
  return g_array_funcs.pop[kind()](this, value);
}

inline ArrayData* ArrayData::dequeue(Variant& value) {
  return g_array_funcs.dequeue[kind()](this, value);
}

inline ArrayData* ArrayData::prepend(Cell v) {
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

inline bool isIntKey(Cell cell) {
  assertx(isIntType(cell.m_type) || isStringType(cell.m_type));
  return isIntType(cell.m_type);
}

inline int64_t getIntKey(Cell cell) {
  assertx(isIntType(cell.m_type));
  return cell.m_data.num;
}

inline StringData* getStringKey(Cell cell) {
  assertx(isStringType(cell.m_type));
  return cell.m_data.pstr;
}

}

///////////////////////////////////////////////////////////////////////////////
// Element manipulation.

inline bool ArrayData::exists(Cell k) const {
  assertx(IsValidKey(k));
  return detail::isIntKey(k) ? exists(detail::getIntKey(k))
                             : exists(detail::getStringKey(k));
}

inline arr_lval ArrayData::lval(Cell k, bool copy) {
  assertx(IsValidKey(k));
  return detail::isIntKey(k) ? lval(detail::getIntKey(k), copy)
                             : lval(detail::getStringKey(k), copy);
}

inline arr_lval ArrayData::lvalRef(Cell k, bool copy) {
  assertx(IsValidKey(k));
  return detail::isIntKey(k) ? lvalRef(detail::getIntKey(k), copy)
                             : lvalRef(detail::getStringKey(k), copy);
}

inline tv_rval ArrayData::get(int64_t k, bool error) const {
  auto r = error ? rvalStrict(k) : rval(k);
  return r ? r : getNotFound(k, error);
}

inline tv_rval ArrayData::get(const StringData* k, bool error) const {
  auto r = error ? rvalStrict(k) : rval(k);
  return r ? r : getNotFound(k, error);
}

inline tv_rval ArrayData::get(Cell k, bool error) const {
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

inline TypedValue ArrayData::at(Cell k) const {
  assertx(IsValidKey(k));
  return detail::isIntKey(k) ? at(detail::getIntKey(k))
                             : at(detail::getStringKey(k));
}

inline TypedValue ArrayData::atPos(ssize_t pos) const {
  return rvalPos(pos).tv();
}

inline ArrayData* ArrayData::set(Cell k, Cell v) {
  assertx(cellIsPlausible(k));
  assertx(cellIsPlausible(v));
  assertx(IsValidKey(k));

  return detail::isIntKey(k) ? set(detail::getIntKey(k), v)
                             : set(detail::getStringKey(k), v);
}

inline ArrayData* ArrayData::setInPlace(Cell k, Cell v) {
  assertx(cellIsPlausible(k));
  assertx(cellIsPlausible(v));
  assertx(IsValidKey(k));

  return detail::isIntKey(k) ? setInPlace(detail::getIntKey(k), v)
                             : setInPlace(detail::getStringKey(k), v);
}

inline ArrayData* ArrayData::setWithRef(Cell k, TypedValue v) {
  assertx(cellIsPlausible(k));
  assertx(tvIsPlausible(v));
  assertx(IsValidKey(k));

  return detail::isIntKey(k) ? setWithRef(detail::getIntKey(k), v)
                             : setWithRef(detail::getStringKey(k), v);
}

inline ArrayData* ArrayData::setWithRefInPlace(Cell k, TypedValue v) {
  assertx(cellIsPlausible(k));
  assertx(tvIsPlausible(v));
  assertx(IsValidKey(k));

  return detail::isIntKey(k) ? setWithRefInPlace(detail::getIntKey(k), v)
                             : setWithRefInPlace(detail::getStringKey(k), v);
}

inline ArrayData* ArrayData::remove(Cell k) {
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
  return exists(*k.toCell());
}

inline arr_lval ArrayData::lval(const String& k, bool copy) {
  assertx(IsValidKey(k));
  return lval(k.get(), copy);
}

inline arr_lval ArrayData::lval(const Variant& k, bool copy) {
  return lval(*k.toCell(), copy);
}

inline arr_lval ArrayData::lvalRef(const String& k, bool copy) {
  assertx(IsValidKey(k));
  return lvalRef(k.get(), copy);
}

inline arr_lval ArrayData::lvalRef(const Variant& k, bool copy) {
  return lvalRef(*k.toCell(), copy);
}

inline tv_rval ArrayData::get(const String& k, bool error) const {
  assertx(IsValidKey(k));
  return get(k.get(), error);
}

inline tv_rval ArrayData::get(const Variant& k, bool error) const {
  return get(*k.toCell(), error);
}

inline ArrayData* ArrayData::set(const String& k, Cell v) {
  assertx(cellIsPlausible(v));
  assertx(IsValidKey(k));
  return set(k.get(), v);
}

inline ArrayData* ArrayData::setInPlace(const String& k, Cell v) {
  assertx(cellIsPlausible(v));
  assertx(IsValidKey(k));
  return setInPlace(k.get(), v);
}

inline ArrayData* ArrayData::set(const String& k, const Variant& v) {
  return set(k, *v.toCell());
}

inline ArrayData* ArrayData::setInPlace(const String& k, const Variant& v) {
  return setInPlace(k, *v.toCell());
}

inline ArrayData* ArrayData::set(const Variant& k, const Variant& v) {
  return set(*k.toCell(), *v.toCell());
}

inline ArrayData* ArrayData::setInPlace(const Variant& k, const Variant& v) {
  return setInPlace(*k.toCell(), *v.toCell());
}

inline ArrayData* ArrayData::setWithRef(const String& k, TypedValue v) {
  assertx(tvIsPlausible(v));
  assertx(IsValidKey(k));
  return setWithRef(k.get(), v);
}

inline ArrayData* ArrayData::setWithRefInPlace(const String& k, TypedValue v) {
  assertx(tvIsPlausible(v));
  assertx(IsValidKey(k));
  return setWithRefInPlace(k.get(), v);
}

inline ArrayData* ArrayData::remove(const String& k) {
  assertx(IsValidKey(k));
  return remove(k.get());
}

inline ArrayData* ArrayData::remove(const Variant& k) {
  return remove(*k.toCell());
}

inline Variant ArrayData::getValue(ssize_t pos) const {
  return Variant{const_variant_ref{rvalPos(pos)}};
}

inline Variant ArrayData::getKey(ssize_t pos) const {
  auto key = nvGetKey(pos);
  return std::move(cellAsVariant(key));
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
