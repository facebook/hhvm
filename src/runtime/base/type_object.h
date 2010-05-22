/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __INSIDE_HPHP_COMPLEX_TYPES_H__
#error Directly including 'type_object.h' is prohibited. \
       Include 'complex_types.h' instead.
#endif

#ifndef __HPHP_OBJECT_H__
#define __HPHP_OBJECT_H__

#include <runtime/base/util/smart_ptr.h>
#include <runtime/base/object_data.h>
#include <runtime/base/type_string.h>
#include <runtime/base/hphp_value.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#define null_object Object::s_nullObject

/**
 * Object type wrapping around ObjectData to implement reference count.
 */
class Object : protected Value {
 public:
  friend class Variant;

  Object() {
    m_data.pobj = NULL;
  }
  ~Object() {
    if (m_data.pobj && m_data.pobj->decRefCount() == 0) {
      m_data.pobj->release();
    }
  }

  static const Object s_nullObject;

  /**
   * Constructors
   */
  Object(ObjectData *data) {
    m_data.pobj = data;
    if (m_data.pobj) {
      m_data.pobj->incRefCount();
    }
  }

  template <typename T>
  Object(T *data) {
    // Performs a static_cast from T* to ObjectData*. This statement will
    // cause a compile time failure if T is not a descendent of ObjectData
    // in the inheritance hierarchy
    m_data.pobj = static_cast<ObjectData*>(data);
    // Assert that casting does not adjust the 'this' pointer
    ASSERT((void*)dynamic_cast<ObjectData*>(data) == (void*)data);
    if (m_data.pobj) m_data.pobj->incRefCount();
  }

  Object(CObjRef src) {
    m_data.pobj = src.m_data.pobj;
    if (m_data.pobj) {
      m_data.pobj->incRefCount();
    }
  }

  bool isNull() const {
    return m_data.pobj == NULL;
  }

  Object& set(ObjectData *px) {
    if (m_data.pobj != px) {
      if (m_data.pobj && m_data.pobj->decRefCount() == 0) {
        m_data.pobj->release();
      }
      m_data.pobj = px;
      if (m_data.pobj) {
        m_data.pobj->incRefCount();
      }
    }
    return *this;
  }

 private:
  Object& setPtr(ObjectData *px) {
    ASSERT(m_data.pobj != px);
    ASSERT(px != NULL);
    if (m_data.pobj && m_data.pobj->decRefCount() == 0) {
      m_data.pobj->release();
    }
    m_data.pobj = px;
    m_data.pobj->incRefCount();
    return *this;
  }

 public:
  Object& set(const Object& src) {
    return set(src.m_data.pobj);
  }

  void reset() {
    if (m_data.pobj && m_data.pobj->decRefCount() == 0) {
      m_data.pobj->release();
    }
    m_data.pobj = NULL;
  }

  /**
   * Magic delegation.
   */
  ObjectData *operator->() const {
    if (!m_data.pobj) throw NullPointerException();
    return m_data.pobj;
  }

  /**
   * Get the raw pointer.
   */
  ObjectData *get() const {
    return m_data.pobj;
  }

  /**
   * Operators
   */
  Object &operator=(ObjectData *data) {
    set(data);
    return *this;
  }

  Object &operator=(CObjRef obj) {
    set(obj.m_data.pobj);
    return *this;
  }

  Object &operator=(CVarRef var);

  /**
   * Informational
   */
  bool isResource() const {
    return m_data.pobj && m_data.pobj->isResource();
  }
  bool instanceof(const char *s) const {
    return m_data.pobj && m_data.pobj->o_instanceof(s);
  }

  /**
   * getTyped() and is() are intended for use with classes only. Using
   * these functions with an interface will cause a compile time error.
   * It is also worth noting that these functions have not been tested
   * with redeclared classes or classes that have a redeclared ancestor
   * in the inheritance hierarchy.
   */
  template<typename T>
  T *getTyped(bool nullOkay = false, bool badTypeOkay = false) const {
    CT_ASSERT_DESCENDENT_OF_OBJECTDATA(T);

    if (!m_data.pobj) {
      if (!nullOkay) {
        throw NullPointerException();
      }
      return NULL;
    }
    ASSERT(m_data.pobj);

    T *px = dynamic_cast<T*>(m_data.pobj);
    // Assert that casting does not adjust the 'this' pointer
    ASSERT(px == NULL || (void*)px == (void*)m_data.pobj);
    if (!px && !badTypeOkay) {
      throw InvalidObjectTypeException(m_data.pobj->o_getClassName());
    }

    return px;
  }
  template<typename T>
  bool is() const {
    return getTyped<T>(true, true) != NULL;
  }

  /**
   * Type conversions
   */
  bool   toBoolean() const { return m_data.pobj;}
  char   toByte   () const { return m_data.pobj ? m_data.pobj->o_toInt64() : 0;}
  short  toInt16  () const { return m_data.pobj ? m_data.pobj->o_toInt64() : 0;}
  int    toInt32  () const { return m_data.pobj ? m_data.pobj->o_toInt64() : 0;}
  int64  toInt64  () const { return m_data.pobj ? m_data.pobj->o_toInt64() : 0;}
  double toDouble () const { return m_data.pobj ? m_data.pobj->o_toInt64() : 0;}
  String toString () const {
    return m_data.pobj ? m_data.pobj->t___tostring() : String();
  }
  Array  toArray  () const;
  Variant toKey   () const;

  /**
   * Comparisons
   */
  bool same (CObjRef v2) const { return m_data.pobj == v2.m_data.pobj;}
  bool equal(CObjRef v2) const;
  bool less (CObjRef v2) const;
  bool more (CObjRef v2) const;

  /**
   * Unresolved objects will go through these two functions than the ones
   * on SmartObject<T>.
   */
  Variant o_get(CStrRef propName, int64 hash = -1, bool error = true) const;
  ObjectOffset o_lval(CStrRef propName, int64 hash = -1);

  /**
   * Input/Output
   */
  void serialize(VariableSerializer *serializer) const;
  bool unserialize(std::istream &in);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_OBJECT_H__
