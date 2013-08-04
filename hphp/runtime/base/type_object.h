/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_OBJECT_H_
#define incl_HPHP_OBJECT_H_

#ifndef incl_HPHP_INSIDE_HPHP_COMPLEX_TYPES_H_
#error Directly including 'type_object.h' is prohibited. \
       Include 'complex_types.h' instead.
#endif

#include "hphp/runtime/base/smart_ptr.h"
#include "hphp/runtime/base/object_data.h"
#include "hphp/runtime/base/type_string.h"
#include "hphp/runtime/base/hphp_value.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#define null_object Object::s_nullObject

class ArrayIter;
class MutableArrayIter;

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
  /* implicit */ Object(CObjRef src) : ObjectBase(src.m_px) { }

  // Move ctor
  Object(Object&& src) : ObjectBase(std::move(src)) {
    static_assert(sizeof(Object) == sizeof(ObjectBase), "Fix this.");
  }
  // Regular assign
  Object& operator=(const Object& src) {
    static_assert(sizeof(Object) == sizeof(ObjectBase), "Fix this.");
    ObjectBase::operator=(src);
    return *this;
  }
  // Move assign
  Object& operator=(Object&& src) {
    static_assert(sizeof(Object) == sizeof(ObjectBase), "Fix this.");
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
  bool instanceof(CStrRef s) const {
    return m_px && m_px->o_instanceof(s);
  }
  bool instanceof(const Class* cls) const {
    return m_px && m_px->instanceof(cls);
  }

  template <class T> T& cast() { return *static_cast<T*>(this); }
  template <class T> const T& cast() const {
    return *static_cast<const T*>(this);
  }
  ArrayIter begin(CStrRef context = null_string) const;

  MutableArrayIter begin(Variant *key, Variant &val,
                         CStrRef context = null_string) const;

  /**
   * getTyped() and is() are intended for use with C++ classes that derive
   * from ObjectData.
   */
  template<typename T>
  T *getTyped(bool nullOkay = false, bool badTypeOkay = false) const {
    CT_ASSERT_DESCENDENT_OF_OBJECTDATA(T);

    ObjectData *cur = m_px;
    if (!cur) {
      if (!nullOkay) {
        throw_null_pointer_exception();
      }
      return nullptr;
    }
    T *px = dynamic_cast<T*>(cur);
    if (!px) {
      if (!badTypeOkay) {
        throw InvalidObjectTypeException(m_px->o_getClassName().c_str());
      }
      return nullptr;
    }

    // Assert that casting does not adjust the 'this' pointer
    assert((void*)px == (void*)cur);
    return px;
  }
  template<typename T>
  bool is() const {
    return getTyped<T>(true, true) != nullptr;
  }
  template<typename T>
  T *cast() const {
    return getTyped<T>();
  }

  ObjectData *objectForCall() const {
    if (m_px) return m_px;
    throw_call_non_object();
  }

  /**
   * Type conversions
   */
  bool   toBoolean() const { return m_px ? m_px->o_toBoolean() : false;}
  char   toByte   () const { return m_px ? m_px->o_toInt64() : 0;}
  short  toInt16  () const { return m_px ? m_px->o_toInt64() : 0;}
  int    toInt32  () const { return m_px ? m_px->o_toInt64() : 0;}
  int64_t  toInt64  () const { return m_px ? m_px->o_toInt64() : 0;}
  double toDouble () const { return m_px ? m_px->o_toDouble() : 0;}
  String toString () const { return m_px ? m_px->invokeToString() : String();}
  Array  toArray  () const;

  int64_t toInt64ForCompare() const;
  double toDoubleForCompare() const;

  /**
   * Comparisons
   */
  bool same (CObjRef v2) const { return m_px == v2.get();}
  bool equal(CObjRef v2) const;
  bool less (CObjRef v2) const;
  bool more (CObjRef v2) const;

  Variant o_get(CStrRef propName, bool error = true,
                CStrRef context = null_string) const;
  Variant o_set(CStrRef s, CVarRef v, CStrRef context = null_string);
  Variant o_set(CStrRef s, RefResult v, CStrRef context = null_string);
  Variant o_setRef(CStrRef s, CVarRef v, CStrRef context = null_string);

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
};

///////////////////////////////////////////////////////////////////////////////
// ObjNR

class ObjNR {
public:
  explicit ObjNR(ObjectData *data) {
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

  static void compileTimeAssertions() {
    static_assert(offsetof(ObjNR, m_px) == kExpectedMPxOffset, "");
  }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_OBJECT_H_
