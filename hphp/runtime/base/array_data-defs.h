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

#ifndef incl_HPHP_ARRAY_INLINE_H_
#define incl_HPHP_ARRAY_INLINE_H_

#include "hphp/runtime/base/array_data.h"
#include "hphp/runtime/base/complex_types.h"

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

inline bool ArrayData::exists(CStrRef k) const {
  assert(IsValidKey(k));
  return exists(k.get());
}

inline bool ArrayData::exists(CVarRef k) const {
  assert(IsValidKey(k));
  auto const cell = k.asCell();
  return isIntKey(cell) ? exists(getIntKey(cell))
                        : exists(getStringKey(cell));
}

inline CVarRef ArrayData::get(CStrRef k, bool error) const {
  assert(IsValidKey(k));
  return get(k.get(), error);
}

inline CVarRef ArrayData::get(int64_t k, bool error) const {
  auto tv = nvGet(k);
  return tv ? tvAsCVarRef(tv) : getNotFound(k, error);
}

inline CVarRef ArrayData::get(const StringData* k, bool error) const {
  auto tv = nvGet(k);
  return tv ? tvAsCVarRef(tv) : getNotFound(k, error);
}

inline ArrayData* ArrayData::lval(CStrRef k, Variant *&ret, bool copy,
                                  bool checkExist) {
  assert(IsValidKey(k));
  return lval(k.get(), ret, copy, checkExist);
}

inline ArrayData* ArrayData::lval(CVarRef k, Variant *&ret, bool copy,
                                  bool checkExist) {
  assert(IsValidKey(k));
  auto const cell = k.asCell();
  return isIntKey(cell) ? lval(getIntKey(cell), ret, copy, checkExist)
                        : lval(getStringKey(cell), ret, copy, checkExist);
}

inline
ArrayData *ArrayData::createLvalPtr(CStrRef k, Variant *&ret, bool copy) {
  assert(IsValidKey(k));
  return createLvalPtr(k.get(), ret, copy);
}

inline ArrayData *ArrayData::getLvalPtr(CStrRef k, Variant *&ret, bool copy) {
  assert(IsValidKey(k));
  return getLvalPtr(k.get(), ret, copy);
}

inline ArrayData* ArrayData::set(CStrRef k, CVarRef v, bool copy) {
  assert(IsValidKey(k));
  return set(k.get(), v, copy);
}

inline ArrayData* ArrayData::set(CVarRef k, CVarRef v, bool copy) {
  assert(IsValidKey(k));
  auto const cell = k.asCell();
  return isIntKey(cell) ? set(getIntKey(cell), v, copy)
                        : set(getStringKey(cell), v, copy);
}

inline ArrayData* ArrayData::setRef(CStrRef k, CVarRef v, bool copy) {
  assert(IsValidKey(k));
  return setRef(k.get(), v, copy);
}

inline ArrayData* ArrayData::setRef(CVarRef k, CVarRef v, bool copy) {
  assert(IsValidKey(k));
  auto const cell = k.asCell();
  return isIntKey(cell) ? setRef(getIntKey(cell), v, copy)
                        : setRef(getStringKey(cell), v, copy);
}

inline ArrayData* ArrayData::add(CStrRef k, CVarRef v, bool copy) {
  assert(IsValidKey(k));
  return add(k.get(), v, copy);
}

inline ArrayData* ArrayData::add(CVarRef k, CVarRef v, bool copy) {
  assert(IsValidKey(k));
  auto const cell = k.asCell();
  return isIntKey(cell) ? add(getIntKey(cell), v, copy)
                        : add(getStringKey(cell), v, copy);
}

inline ArrayData* ArrayData::addLval(CStrRef k, Variant *&ret, bool copy) {
  assert(IsValidKey(k));
  return addLval(k.get(), ret, copy);
}

inline ArrayData* ArrayData::addLval(CVarRef k, Variant *&ret, bool copy) {
  assert(IsValidKey(k));
  auto const cell = k.asCell();
  return isIntKey(cell) ? addLval(getIntKey(cell), ret, copy)
                        : addLval(getStringKey(cell), ret, copy);
}

inline ArrayData* ArrayData::remove(CStrRef k, bool copy) {
  assert(IsValidKey(k));
  return remove(k.get(), copy);
}

inline ArrayData* ArrayData::remove(CVarRef k, bool copy) {
  assert(IsValidKey(k));
  auto const cell = k.asCell();
  return isIntKey(cell) ? remove(getIntKey(cell), copy)
                        : remove(getStringKey(cell), copy);
}

inline Variant ArrayData::getValue(ssize_t pos) const {
  return getValueRef(pos);
}

inline TypedValue* ArrayData::nvGetValueRef(ssize_t pos) {
  return const_cast<TypedValue*>(getValueRef(pos).asTypedValue());
}

inline Variant ArrayData::getKey(ssize_t pos) const {
  TypedValue tv;
  nvGetKey(&tv, pos);
  return std::move(tvAsVariant(&tv));
}

///////////////////////////////////////////////////////////////////////////////

inline unsigned ArrayData::index() const {
  assert(unsigned(m_kind) < unsigned(ArrayKind::kNumKinds));
  return unsigned(m_kind);
}

inline void ArrayData::release() {
  return g_array_funcs.release[index()](this);
}

inline ArrayData* ArrayData::append(CVarRef v, bool copy) {
  return g_array_funcs.append[index()](this, v, copy);
}

inline TypedValue* ArrayData::nvGet(int64_t ikey) const {
  return g_array_funcs.nvGetInt[index()](this, ikey);
}

inline TypedValue* ArrayData::nvGet(const StringData* skey) const {
  return g_array_funcs.nvGetStr[index()](this, skey);
}

inline void ArrayData::nvGetKey(TypedValue* out, ssize_t pos) const {
  g_array_funcs.nvGetKey[index()](this, out, pos);
}

inline ArrayData* ArrayData::set(int64_t k, CVarRef v, bool copy) {
  return g_array_funcs.setInt[index()](this, k, v, copy);
}

inline ArrayData* ArrayData::set(StringData* k, CVarRef v, bool copy) {
  return g_array_funcs.setStr[index()](this, k, v, copy);
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_ARRAY_INLINE_H_
