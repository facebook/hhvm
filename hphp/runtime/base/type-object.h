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
class Object : protected SmartPtr<ObjectData> {
  typedef SmartPtr<ObjectData> ObjectBase;

public:
  Object() {}

  static const Object s_nullObject;

  ObjectData* get() const { return m_px; }
  void reset() { ObjectBase::reset(); }

  ObjectData* operator->() const {
    if (!m_px) throw_null_pointer_exception();
    return m_px;
  }

  /**
   * Constructors
   */
  /* implicit */ Object(ObjectData *data) : ObjectBase(data) { }
  /* implicit */ Object(const Object& src) : ObjectBase(src.m_px) { }

  // Move ctor
  Object(Object&& src) : ObjectBase(std::move(src)) { }

  // Regular assign
  Object& operator=(const Object& src) {
    ObjectBase::operator=(src);
    return *this;
  }
  // Move assign
  Object& operator=(Object&& src) {
    ObjectBase::operator=(std::move(src));
    return *this;
  }

  ~Object();

  /**
   * Informational
   */
  bool isNull() const {
    return m_px == nullptr;
  }
  bool instanceof(const String& s) const {
    return m_px && m_px->o_instanceof(s);
  }
  bool instanceof(const Class* cls) const {
    return m_px && m_px->instanceof(cls);
  }

  template <class T> T& cast() { return *static_cast<T*>(this); }
  template <class T> const T& cast() const {
    return *static_cast<const T*>(this);
  }

  /**
   * getTyped() and is() are intended for use with C++ classes that derive
   * from ObjectData.
   */
  template<typename T>
  T *getTyped(bool nullOkay = false, bool badTypeOkay = false) const {
    static_assert(std::is_base_of<ObjectData, T>::value, "");

    ObjectData *cur = m_px;
    if (!cur) {
      if (!nullOkay) {
        throw_null_pointer_exception();
      }
      return nullptr;
    }
    if (!cur->instanceof(T::classof())) {
      if (!badTypeOkay) {
        throw InvalidObjectTypeException(classname_cstr());
      }
      return nullptr;
    }

    return static_cast<T*>(cur);
  }

  template<typename T>
  bool is() const {
    return getTyped<T>(true, true) != nullptr;
  }

  template<typename T>
  T *cast() const {
    return getTyped<T>();
  }

  /**
   * Type conversions
   */
  bool    toBoolean() const { return m_px ? m_px->o_toBoolean() : false;}
  char    toByte   () const { return m_px ? m_px->o_toInt64() : 0;}
  short   toInt16  () const { return m_px ? m_px->o_toInt64() : 0;}
  int     toInt32  () const { return m_px ? m_px->o_toInt64() : 0;}
  int64_t toInt64  () const { return m_px ? m_px->o_toInt64() : 0;}
  double  toDouble () const { return m_px ? m_px->o_toDouble() : 0;}
  String  toString () const;
  Array   toArray  () const;

  int64_t toInt64ForCompare() const;
  double toDoubleForCompare() const;

  /**
   * Comparisons
   */
  bool same (const Object& v2) const { return m_px == v2.get();}
  bool equal(const Object& v2) const;
  bool less (const Object& v2) const;
  bool more (const Object& v2) const;

  Variant o_get(const String& propName, bool error = true,
                const String& context = null_string) const;
  Variant o_set(
    const String& s, const Variant& v, const String& context = null_string);
  Variant o_set(
    const String& s, RefResult v, const String& context = null_string);
  Variant o_setRef(
    const String& s, const Variant& v, const String& context = null_string);

  /**
   * Input/Output
   */
  void serialize(VariableSerializer *serializer) const;
  bool unserialize(std::istream &in);

  void setToDefaultObject();

  // Transfer ownership of our reference to this object.
  ObjectData *detach() {
    ObjectData *ret = m_px;
    m_px = nullptr;
    return ret;
  }

  // Take ownership of a reference without touching the ref count
  static Object attach(ObjectData *object) {
    Object o;
    o.m_px = object;
    return o;
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
