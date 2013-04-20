/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/base/array/array_data.h>
#include <runtime/base/complex_types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace {
typedef Variant::TypedValueAccessor TypedValueAccessor;

inline static bool isIntKey(TypedValueAccessor tva) {
    return Variant::GetAccessorType(tva) <= KindOfInt64;
}

inline static int64_t getIntKey(TypedValueAccessor tva) {
    return Variant::GetInt64(tva);
}

inline static StringData *getStringKey(TypedValueAccessor tva) {
    return Variant::GetStringData(tva);
}
}

inline bool ArrayData::exists(CStrRef k) const {
  assert(IsValidKey(k));
  return exists(k.get());
}

inline bool ArrayData::exists(CVarRef k) const {
  assert(IsValidKey(k));
  TypedValueAccessor tvk = k.getTypedAccessor();
  return isIntKey(tvk) ? exists(getIntKey(tvk)) :
         exists(getStringKey(tvk));
}

inline CVarRef ArrayData::get(CStrRef k, bool error) const {
  assert(IsValidKey(k));
  return get(k.get(), error);
}

inline CVarRef ArrayData::get(CVarRef k, bool error) const {
  assert(IsValidKey(k));
  TypedValueAccessor tvk = k.getTypedAccessor();
  return isIntKey(tvk) ? get(getIntKey(tvk), error) :
         get(getStringKey(tvk), error);
}

inline ssize_t ArrayData::getIndex(CStrRef k) const {
  assert(IsValidKey(k));
  return getIndex(k.get());
}

inline ssize_t ArrayData::getIndex(CVarRef k) const {
  assert(IsValidKey(k));
  TypedValueAccessor tvk = k.getTypedAccessor();
  return isIntKey(tvk) ? getIndex(getIntKey(tvk)) :
         getIndex(getStringKey(tvk));
}

inline ArrayData* ArrayData::lval(CStrRef k, Variant *&ret, bool copy,
                                  bool checkExist) {
  assert(IsValidKey(k));
  return lval(k.get(), ret, copy, checkExist);
}

inline ArrayData* ArrayData::lval(CVarRef k, Variant *&ret, bool copy,
                                  bool checkExist) {
  assert(IsValidKey(k));
  TypedValueAccessor tvk = k.getTypedAccessor();
  return isIntKey(tvk) ? lval(getIntKey(tvk), ret, copy, checkExist) :
         lval(getStringKey(tvk), ret, copy, checkExist);
}

inline ArrayData *ArrayData::lvalPtr(CStrRef k, Variant *&ret, bool copy,
                                     bool create) {
  assert(IsValidKey(k));
  return lvalPtr(k.get(), ret, copy, create);
}

inline ArrayData* ArrayData::set(CStrRef k, CVarRef v, bool copy) {
  assert(IsValidKey(k));
  return set(k.get(), v, copy);
}

inline ArrayData* ArrayData::set(CVarRef k, CVarRef v, bool copy) {
  assert(IsValidKey(k));
  TypedValueAccessor tvk = k.getTypedAccessor();
  return isIntKey(tvk) ? set(getIntKey(tvk), v, copy) :
         set(getStringKey(tvk), v, copy);
}

inline ArrayData* ArrayData::nvSet(int64_t ki, const TypedValue* v, bool copy) {
  return set(ki, tvAsCVarRef(v), copy);
}

inline ArrayData* ArrayData::nvSet(StringData* k, const TypedValue* v,
                                   bool copy) {
  return set(k, tvAsCVarRef(v), copy);
}

inline ArrayData* ArrayData::setRef(CStrRef k, CVarRef v, bool copy) {
  assert(IsValidKey(k));
  return setRef(k.get(), v, copy);
}

inline ArrayData* ArrayData::setRef(CVarRef k, CVarRef v, bool copy) {
  assert(IsValidKey(k));
  TypedValueAccessor tvk = k.getTypedAccessor();
  return isIntKey(tvk) ? setRef(getIntKey(tvk), v, copy) :
         setRef(getStringKey(tvk), v, copy);
}

inline ArrayData* ArrayData::add(CStrRef k, CVarRef v, bool copy) {
  assert(IsValidKey(k));
  return add(k.get(), v, copy);
}

inline ArrayData* ArrayData::add(CVarRef k, CVarRef v, bool copy) {
  assert(IsValidKey(k));
  TypedValueAccessor tvk = k.getTypedAccessor();
  return isIntKey(tvk) ? add(getIntKey(tvk), v, copy) :
         add(getStringKey(tvk), v, copy);
}

inline ArrayData* ArrayData::addLval(CStrRef k, Variant *&ret, bool copy) {
  assert(IsValidKey(k));
  return addLval(k.get(), ret, copy);
}

inline ArrayData* ArrayData::addLval(CVarRef k, Variant *&ret, bool copy) {
  assert(IsValidKey(k));
  TypedValueAccessor tvk = k.getTypedAccessor();
  return isIntKey(tvk) ? addLval(getIntKey(tvk), ret, copy) :
         addLval(getStringKey(tvk), ret, copy);
}

inline ArrayData* ArrayData::remove(CStrRef k, bool copy) {
  assert(IsValidKey(k));
  return remove(k.get(), copy);
}

inline ArrayData* ArrayData::remove(CVarRef k, bool copy) {
  assert(IsValidKey(k));
  TypedValueAccessor tvk = k.getTypedAccessor();
  return isIntKey(tvk) ? remove(getIntKey(tvk), copy) :
         remove(getStringKey(tvk), copy);
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_ARRAY_INLINE_H_
