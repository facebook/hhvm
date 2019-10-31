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

#ifndef incl_HPHP_OBJECT_H_
#define incl_HPHP_OBJECT_H_

#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/req-ptr.h"
#include "hphp/runtime/base/typed-value.h"

#include <algorithm>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// Forward declare to avoid including tv-conversions.h and creating a cycle.
ObjectData* tvCastToObjectData(TypedValue tv);

/**
 * Object type wrapping around ObjectData to implement reference count.
 */
struct Object {
private:
  req::ptr<ObjectData> m_obj;

  using NoIncRef = req::ptr<ObjectData>::NoIncRef;

public:
  Object() {}

  ObjectData* get() const { return m_obj.get(); }
  void reset(ObjectData* obj = nullptr) { m_obj.reset(obj); }

  ObjectData* operator->() const {
    return m_obj.get();
  }

  /**
   * Constructors
   */
  explicit Object(ObjectData *data) : m_obj(data) {
    // The object must have at least two refs here. One pre-existing ref, and
    // one caused by placing it under m_obj's control.
    assertx(!data || data->hasMultipleRefs());
  }
  /* implicit */ Object(const Object& src) : m_obj(src.m_obj) {
    assertx(!m_obj || m_obj->hasMultipleRefs());
  }

  template <typename T>
  explicit Object(const req::ptr<T> &ptr) : m_obj(ptr) {
    assertx(!m_obj || m_obj->hasMultipleRefs());
  }

  template <typename T>
  explicit Object(req::ptr<T>&& ptr) : m_obj(std::move(ptr)) {
    assertx(!m_obj || m_obj->checkCount());
  }

  explicit Object(Class* cls)
    : m_obj(ObjectData::newInstance(cls), NoIncRef{}) {
    // References to the object can escape inside newInstance, so we only know
    // that the ref-count is at least 1 here.
    assertx(!m_obj || m_obj->checkCount());
  }

  explicit Object(Class* cls, ArrayData* reifiedTypes)
    : m_obj(ObjectData::newInstanceReified(cls, reifiedTypes), NoIncRef{}) {
    // References to the object can escape inside newInstance, so we only know
    // that the ref-count is at least 1 here.
    assertx(!m_obj || m_obj->checkCount());
  }

  // Move ctor
  Object(Object&& src) noexcept : m_obj(std::move(src.m_obj)) {
    assertx(!m_obj || m_obj->checkCount());
  }

  // Regular assign
  Object& operator=(const Object& src) {
    m_obj = src.m_obj;
    assertx(!m_obj || m_obj->hasMultipleRefs());
    return *this;
  }

  template <typename T>
  Object& operator=(const req::ptr<T>& src) {
    m_obj = src;
    assertx(!m_obj || m_obj->hasMultipleRefs());
    return *this;
  }

  // Move assign
  Object& operator=(Object&& src) {
    m_obj = std::move(src.m_obj);
    assertx(!m_obj || m_obj->checkCount());
    return *this;
  }

  template <typename T>
  Object& operator=(req::ptr<T>&& src) {
    m_obj = std::move(src);
    assertx(!m_obj || m_obj->checkCount());
    return *this;
  }

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

  // Transfer ownership of our reference to this object.
  ObjectData *detach() { return m_obj.detach(); }

  // Take ownership of a reference without touching the ref count
  static Object attach(ObjectData *object) {
    assertx(!object || object->checkCount());
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

extern const Object null_object;

///////////////////////////////////////////////////////////////////////////////
// ObjNR

struct ObjNR {
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

ALWAYS_INLINE const Object& asCObjRef(tv_rval tv) {
  assertx(tvIsObject(tv));
  return reinterpret_cast<const Object&>(val(tv).pobj);
}

ALWAYS_INLINE Object toObject(tv_rval tv) {
  if (tvIsObject(tv)) return Object{assert_not_null(val(tv).pobj)};
  return Object::attach(tvCastToObjectData(*tv));
}

}

#endif // incl_HPHP_OBJECT_H_
