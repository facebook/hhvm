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

#ifndef __HPHP_OBJECT_H__
#define __HPHP_OBJECT_H__

#include <cpp/base/util/smart_ptr.h>
#include <cpp/base/object_data.h>
#include <cpp/base/type_string.h>
#include <cpp/base/type_array.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#define null_object Object::s_nullObject

/**
 * Object type wrapping around ObjectData to implement reference count.
 */
class Object : public SmartPtr<ObjectData> {
 public:
  Object() {}

  static const Object s_nullObject;

  /**
   * Constructors
   */
  Object(ObjectData *data) {
    SmartPtr<ObjectData>::operator=(data);
  }
  Object(CObjRef src) {
    SmartPtr<ObjectData>::operator=(src.m_px);
  }

  /**
   * Informational
   */
  bool isNull() const {
    return m_px == NULL;
  }
  bool isResource() const {
    return m_px && m_px->isResource();
  }
  bool instanceof(const char *s) const {
    return m_px && m_px->o_instanceof(s);
  }

  template<typename T>
  T *getTyped(bool nullOkay = false, bool badTypeOkay = false) const {
    if (!m_px) {
      if (!nullOkay) {
        throw NullPointerException();
      }
      return NULL;
    }

    T *px = dynamic_cast<T*>(m_px);
    if (!px && !badTypeOkay) {
      throw InvalidObjectTypeException(m_px->o_getClassName());
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
  bool   toBoolean() const { return m_px != NULL;}
  char   toByte   () const { return m_px ? m_px->o_toInt64() : 0;}
  short  toInt16  () const { return m_px ? m_px->o_toInt64() : 0;}
  int    toInt32  () const { return m_px ? m_px->o_toInt64() : 0;}
  int64  toInt64  () const { return m_px ? m_px->o_toInt64() : 0;}
  double toDouble () const { return m_px ? m_px->o_toInt64() : 0;}
  String toString () const { return m_px ? m_px->t___tostring() : String();}
  Array  toArray  () const { return m_px ? m_px->o_toArray() : Array();}
  Variant toKey() const {
    return m_px ? (isResource() ? m_px->o_toInt64() : m_px->t___tostring())
                : String();
  }

  /**
   * Comparisons
   */
  bool same (CObjRef v2) const { return m_px == v2.get();}
  bool equal(CObjRef v2) const {
    if (m_px == v2.get())
      return true;
    if (!m_px || !v2.get())
      return false;
    if (isResource() || v2.isResource())
      return false;
    return (v2.get()->o_isClass(m_px->o_getClassName()) &&
            toArray().equal(v2.toArray()));
  }
  bool less (CObjRef v2) const { return false;}
  bool more (CObjRef v2) const { return false;}

  /**
   * Unresolved objects will go through these two functions than the ones
   * on SmartObject<T>.
   */
  Variant o_get(CStrRef propName, int64 hash = -1) const;
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
