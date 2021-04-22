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

#pragma once

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
// ArrayFunction dispatch.

inline bool ArrayData::notCyclic(TypedValue v) const {
  return !tvIsArrayLike(v) || v.m_data.parr != this;
}

NO_PROFILING
inline void ArrayData::releaseUncounted() {
  g_array_funcs.releaseUncounted[kind()](this);
}

NO_PROFILING
inline ArrayData* ArrayData::copyStatic() const {
  auto ret = g_array_funcs.copyStatic[kind()](this);
  assertx(ret != this && ret->isStatic());
  return ret;
}

NO_PROFILING
inline bool ArrayData::isVectorData() const {
  return g_array_funcs.isVectorData[kind()](this);
}

NO_PROFILING
inline bool ArrayData::exists(int64_t k) const {
  return g_array_funcs.existsInt[kind()](this, k);
}

NO_PROFILING
inline bool ArrayData::exists(const StringData* k) const {
  return g_array_funcs.existsStr[kind()](this, k);
}

NO_PROFILING
inline arr_lval ArrayData::lval(int64_t k) {
  return g_array_funcs.lvalInt[kind()](this, k);
}

NO_PROFILING
inline arr_lval ArrayData::lval(StringData* k) {
  return g_array_funcs.lvalStr[kind()](this, k);
}

NO_PROFILING
inline TypedValue ArrayData::nvGetKey(ssize_t pos) const {
  return g_array_funcs.getPosKey[kind()](this, pos);
}

NO_PROFILING
inline TypedValue ArrayData::nvGetVal(ssize_t pos) const {
  return g_array_funcs.getPosVal[kind()](this, pos);
}

NO_PROFILING
inline Variant ArrayData::getKey(ssize_t pos) const {
  return Variant::wrap(nvGetKey(pos));
}

NO_PROFILING
inline Variant ArrayData::getValue(ssize_t pos) const {
  return Variant::wrap(nvGetVal(pos));
}

NO_PROFILING
inline ArrayData* ArrayData::setMove(int64_t k, TypedValue v) {
  assertx(tvIsPlausible(v));
  assertx(cowCheck() || notCyclic(v));
  return g_array_funcs.setIntMove[kind()](this, k, v);
}

NO_PROFILING
inline ArrayData* ArrayData::setMove(StringData* k, TypedValue v) {
  assertx(tvIsPlausible(v));
  assertx(cowCheck() || notCyclic(v));
  return g_array_funcs.setStrMove[kind()](this, k, v);
}

NO_PROFILING
inline ArrayData* ArrayData::remove(int64_t k) {
  return g_array_funcs.removeInt[kind()](this, k);
}

NO_PROFILING
inline ArrayData* ArrayData::remove(const StringData* k) {
  return g_array_funcs.removeStr[kind()](this, k);
}

NO_PROFILING
inline ArrayData* ArrayData::appendMove(TypedValue v) {
  assertx(v.m_type != KindOfUninit);
  assertx(cowCheck() || notCyclic(v));
  return g_array_funcs.appendMove[kind()](this, v);
}

NO_PROFILING
inline ssize_t ArrayData::iter_begin() const {
  return g_array_funcs.iterBegin[kind()](this);
}

NO_PROFILING
inline ssize_t ArrayData::iter_last() const {
  return g_array_funcs.iterLast[kind()](this);
}

NO_PROFILING
inline ssize_t ArrayData::iter_end() const {
  return g_array_funcs.iterEnd[kind()](this);
}

NO_PROFILING
inline ssize_t ArrayData::iter_advance(ssize_t pos) const {
  return g_array_funcs.iterAdvance[kind()](this, pos);
}

NO_PROFILING
inline ssize_t ArrayData::iter_rewind(ssize_t pos) const {
  return g_array_funcs.iterRewind[kind()](this, pos);
}

NO_PROFILING
inline ArrayData* ArrayData::escalateForSort(SortFunction sf) {
  return g_array_funcs.escalateForSort[kind()](this, sf);
}

NO_PROFILING
inline void ArrayData::ksort(int sort_flags, bool ascending) {
  return g_array_funcs.ksort[kind()](this, sort_flags, ascending);
}

NO_PROFILING
inline void ArrayData::sort(int sort_flags, bool ascending) {
  return g_array_funcs.sort[kind()](this, sort_flags, ascending);
}

NO_PROFILING
inline void ArrayData::asort(int sort_flags, bool ascending) {
  return g_array_funcs.asort[kind()](this, sort_flags, ascending);
}

NO_PROFILING
inline bool ArrayData::uksort(const Variant& compare) {
  return g_array_funcs.uksort[kind()](this, compare);
}

NO_PROFILING
inline bool ArrayData::usort(const Variant& compare) {
  return g_array_funcs.usort[kind()](this, compare);
}

NO_PROFILING
inline bool ArrayData::uasort(const Variant& compare) {
  return g_array_funcs.uasort[kind()](this, compare);
}

NO_PROFILING
inline ArrayData* ArrayData::pop(Variant& value) {
  return g_array_funcs.pop[kind()](this, value);
}

NO_PROFILING
inline void ArrayData::onSetEvalScalar() {
  if (isStatic()) return;
  return g_array_funcs.onSetEvalScalar[kind()](this);
}

NO_PROFILING
inline ArrayData* ArrayData::makeUncounted(
    const MakeUncountedEnv& env, bool hasApcTv) {
  return g_array_funcs.makeUncounted[kind()](this, env, hasApcTv);
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

inline arr_lval ArrayData::lval(TypedValue k) {
  assertx(IsValidKey(k));
  return detail::isIntKey(k) ? lval(detail::getIntKey(k))
                             : lval(detail::getStringKey(k));
}

NO_PROFILING
inline TypedValue ArrayData::get(int64_t k) const {
  return g_array_funcs.nvGetInt[kind()](this, k);
}

NO_PROFILING
inline TypedValue ArrayData::get(const StringData* k) const {
  return g_array_funcs.nvGetStr[kind()](this, k);
}

NO_PROFILING
inline TypedValue ArrayData::getThrow(int64_t k) const {
  auto const res = get(k);
  if (!res.is_init()) throw kDummyException;
  return res;
}

NO_PROFILING
inline TypedValue ArrayData::getThrow(const StringData* k) const {
  auto const res = get(k);
  if (!res.is_init()) throw kDummyException;
  return res;
}

NO_PROFILING
inline TypedValue ArrayData::get(int64_t k, bool error) const {
  auto const result = get(k);
  if (error && !result.is_init()) getNotFound(k);
  return result;
}

NO_PROFILING
inline TypedValue ArrayData::get(const StringData* k, bool error) const {
  auto const result = get(k);
  if (error && !result.is_init()) getNotFound(k);
  return result;
}

NO_PROFILING
inline TypedValue ArrayData::get(TypedValue k, bool error) const {
  assertx(IsValidKey(k));
  return detail::isIntKey(k) ? get(detail::getIntKey(k), error)
                             : get(detail::getStringKey(k), error);
}

inline TypedValue ArrayData::at(int64_t k) const {
  auto const result = get(k);
  assertx(result.is_init());
  return result;
}

inline TypedValue ArrayData::at(const StringData* k) const {
  auto const result = get(k);
  assertx(result.is_init());
  return result;
}

inline TypedValue ArrayData::at(TypedValue k) const {
  assertx(IsValidKey(k));
  return detail::isIntKey(k) ? at(detail::getIntKey(k))
                             : at(detail::getStringKey(k));
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

inline arr_lval ArrayData::lval(const String& k) {
  assertx(IsValidKey(k));
  return lval(k.get());
}

inline arr_lval ArrayData::lval(const Variant& k) {
  return lval(*k.asTypedValue());
}

inline TypedValue ArrayData::get(const String& k, bool error) const {
  assertx(IsValidKey(k));
  return get(k.get(), error);
}

inline TypedValue ArrayData::get(const Variant& k, bool error) const {
  return get(*k.asTypedValue(), error);
}

inline ArrayData* ArrayData::setMove(TypedValue k, TypedValue v) {
  assertx(tvIsPlausible(k));
  assertx(tvIsPlausible(v));
  assertx(IsValidKey(k));

  return detail::isIntKey(k) ? setMove(detail::getIntKey(k), v)
                             : setMove(detail::getStringKey(k), v);
}

inline ArrayData* ArrayData::setMove(const String& k, TypedValue v) {
  assertx(tvIsPlausible(v));
  assertx(IsValidKey(k));

  return setMove(k.get(), v);
}

inline ArrayData* ArrayData::setMove(int64_t k, const Variant& v) {
  return setMove(k, *v.asTypedValue());
}

inline ArrayData* ArrayData::setMove(StringData* k, const Variant& v) {
  assertx(IsValidKey(k));
  return setMove(k, *v.asTypedValue());
}

inline ArrayData* ArrayData::setMove(const String& k, const Variant& v) {
  assertx(IsValidKey(k));
  return setMove(k.get(), *v.asTypedValue());
}

inline ArrayData* ArrayData::setMove(const Variant& k, const Variant& v) {
  return setMove(*k.asTypedValue(), *v.asTypedValue());
}

inline ArrayData* ArrayData::remove(const String& k) {
  assertx(IsValidKey(k));
  return remove(k.get());
}

inline ArrayData* ArrayData::remove(const Variant& k) {
  return remove(*k.asTypedValue());
}

///////////////////////////////////////////////////////////////////////////////

}
