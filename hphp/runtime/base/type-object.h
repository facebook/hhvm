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

#ifndef incl_HPHP_OBJECT_H_
#define incl_HPHP_OBJECT_H_

#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/smart-ptr.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/types.h"

#include <algorithm>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#define null_object Object::s_nullObject

/**
 * Object type wrapping around ObjectData to implement reference count.
 */
class Object {
  SmartPtr<ObjectData> m_obj;

public:
  Object() {}

  static const Object s_nullObject;

  ObjectData* get() const { return m_obj.get(); }
  void reset() { m_obj.reset(); }

  ObjectData* operator->() const {
    return m_obj.get();
  }

  /**
   * Constructors
   */
  /* implicit */ Object(ObjectData *data) : m_obj(data) { }
  /* implicit */ Object(const Object& src) : m_obj(src.m_obj) { }

  template <typename T>
  explicit Object(const SmartPtr<T> &ptr) : m_obj(ptr) { }

  template <typename T>
  explicit Object(SmartPtr<T>&& ptr) : m_obj(std::move(ptr)) { }

  // Move ctor
  Object(Object&& src) noexcept : m_obj(std::move(src.m_obj)) { }

  // Regular assign
  Object& operator=(const Object& src) {
    m_obj = src.m_obj;
    return *this;
  }

  template <typename T>
  Object& operator=(const SmartPtr<T>& src) {
    m_obj = src;
    return *this;
  }

  // Move assign
  Object& operator=(Object&& src) {
    m_obj = std::move(src.m_obj);
    return *this;
  }

  template <typename T>
  Object& operator=(SmartPtr<T>&& src) {
    m_obj = std::move(src);
    return *this;
  }

  ~Object();

  /**
   * Informational
   */
  explicit operator bool() const { return (bool)m_obj; }

  bool isNull() const { return !m_obj; }
  bool instanceof(const String& s) const {
    return m_obj && m_obj->instanceof(s);
  }
  bool instanceof(const Class* cls) const {
    return m_obj && m_obj->instanceof(cls);
  }

  /**
   * getTyped() and is() are intended for use with C++ classes that derive
   * from ObjectData.
   *
   * Prefer using the following functions instead of getTyped:
   * o.getTyped<T>(false, false) -> cast<T>(o)
   * o.getTyped<T>(true,  false) -> cast_or_null<T>(o)
   * o.getTyped<T>(false, true) -> dyn_cast<T>(o)
   * o.getTyped<T>(true,  true) -> dyn_cast_or_null<T>(o)
   */
  template<typename T>
  T *getTyped(bool nullOkay = false, bool badTypeOkay = false) const {
    static_assert(std::is_base_of<ObjectData, T>::value, "");

    ObjectData *cur = get();
    if (!cur) {
      if (!nullOkay) {
        throw_null_pointer_exception();
      }
      return nullptr;
    }
    if (!cur->instanceof(T::classof())) {
      if (!badTypeOkay) {
        throw_invalid_object_type(classname_cstr());
      }
      return nullptr;
    }

    return static_cast<T*>(cur);
  }

  template<typename T>
  bool is() const {
    return m_obj && m_obj->instanceof(T::classof());
  }

  /**
   * Type conversions
   */
  bool    toBoolean() const { return m_obj ? m_obj->toBoolean() : false; }
  char    toByte   () const { return toInt64(); }
  int16_t toInt16  () const { return toInt64(); }
  int32_t toInt32  () const { return toInt64(); }
  int64_t toInt64  () const { return m_obj ? m_obj->toInt64() : 0; }
  double  toDouble () const { return m_obj ? m_obj->toDouble() : 0; }
  String  toString () const;
  Array   toArray  () const;

  int64_t toInt64ForCompare() const;
  double toDoubleForCompare() const;

  /**
   * Comparisons
   */
  bool same(const Object& v2) const { return m_obj == v2.m_obj; }
  bool equal(const Object& v2) const;
  bool less(const Object& v2) const;
  bool more(const Object& v2) const;

  Variant o_get(const String& propName, bool error = true,
                const String& context = null_string) const;
  Variant o_set(
    const String& s, const Variant& v, const String& context = null_string);

  /**
   * Input/Output
   */
  void serialize(VariableSerializer *serializer) const;

  void setToDefaultObject();

  // Transfer ownership of our reference to this object.
  ObjectData *detach() { return m_obj.detach(); }

  // Take ownership of a reference without touching the ref count
  static Object attach(ObjectData *object) {
    return Object(SmartPtr<ObjectData>::attach(object));
  }

private:
  static void compileTimeAssertions();

  const char* classname_cstr() const;
};

///////////////////////////////////////////////////////////////////////////////
// ObjNR

class ObjNR {
public:
  explicit ObjNR(ObjectData* data) {
    m_px = data;
  }

  Object& asObject() {
    return *reinterpret_cast<Object*>(this); // XXX
  }

  const Object& asObject() const {
    return const_cast<ObjNR*>(this)->asObject();
  }

private:
  ObjectData* m_px;

  static void compileTimeAssertions();
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_OBJECT_H_
