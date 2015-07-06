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
#include "hphp/runtime/base/req-ptr.h"
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
  req::ptr<ObjectData> m_obj;

  using NoIncRef = req::ptr<ObjectData>::NoIncRef;
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
  /* implicit */ Object(ObjectData *data) : m_obj(data) {
    // The object must have at least two refs here. One pre-existing ref, and
    // one caused by placing it under m_obj's control.
    assert(!data || data->hasMultipleRefs());
  }
  /* implicit */ Object(const Object& src) : m_obj(src.m_obj) {
    assert(!m_obj || m_obj->hasMultipleRefs());
  }

  template <typename T>
  explicit Object(const req::ptr<T> &ptr) : m_obj(ptr) {
    assert(!m_obj || m_obj->hasMultipleRefs());
  }

  template <typename T>
  explicit Object(req::ptr<T>&& ptr) : m_obj(std::move(ptr)) {
    assert(!m_obj || m_obj->getCount() > 0);
  }

  explicit Object(Class* cls)
    : m_obj(ObjectData::newInstance(cls), NoIncRef{}) {
    // References to the object can escape inside newInstance, so we only know
    // that the ref-count is at least 1 here.
    assert(!m_obj || m_obj->getCount() > 0);
  }

  // Move ctor
  Object(Object&& src) noexcept : m_obj(std::move(src.m_obj)) {
    assert(!m_obj || m_obj->getCount() > 0);
  }

  // Regular assign
  Object& operator=(const Object& src) {
    m_obj = src.m_obj;
    assert(!m_obj || m_obj->hasMultipleRefs());
    return *this;
  }

  template <typename T>
  Object& operator=(const req::ptr<T>& src) {
    m_obj = src;
    assert(!m_obj || m_obj->hasMultipleRefs());
    return *this;
  }

  // Move assign
  Object& operator=(Object&& src) {
    m_obj = std::move(src.m_obj);
    assert(!m_obj || m_obj->getCount() > 0);
    return *this;
  }

  template <typename T>
  Object& operator=(req::ptr<T>&& src) {
    m_obj = std::move(src);
    assert(!m_obj || m_obj->getCount() > 0);
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
  [[deprecated("Please use one of the cast family of functions instead.")]]
  req::ptr<T> getTyped(bool nullOkay = false, bool badTypeOkay = false) const {
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

    return req::ptr<T>(static_cast<T*>(cur));
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
    assert(!object || object->getCount() > 0);
    return Object{req::ptr<ObjectData>::attach(object)};
  }

private:
  template <typename T>
  friend typename std::enable_if<
    std::is_base_of<ObjectData,T>::value,
    ObjectData*
  >::type deref(const Object& o) { return o.get(); }

  template <typename T>
  friend typename std::enable_if<
    std::is_base_of<ObjectData,T>::value,
    ObjectData*
  >::type detach(Object&& o) { return o.detach(); }

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
