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

#ifndef __HPHP_SMART_OBJECT_H__
#define __HPHP_SMART_OBJECT_H__

#include <runtime/base/util/smart_ptr.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/object_offset.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * We need a wrapper to manage lifetime of user-defined object's pointers.
 * This template wraps around any user-defined class to allow direct property
 * and method access like this,
 *
 *   SmartObject<MyClass> obj;
 *   obj->method(obj->property);
 *
 * Note that Object class doesn't do this. Therefore we need SmartPtr<T> right
 * on a user-defined class. SmartObject<T> is the same as SmartPtr<T> with
 * extra object specific functions.
 *
 * Since we can't really make every place to take SmartObject<T>, we need to
 * be able to cast SmartObject<T> to Object as a generic fallback. With Object,
 * invoking methods and accessing properties will be hash-table based, so it's
 * slower. This is why when a function is written for objects, Object can be
 * used as a parameter's type, but if it can be specialized to SmartObject<T>,
 * it will almost always be better.
 */
template<typename T>
class SmartObject : public SmartPtr<T> {
 public:
  /**
   * Constructors
   */
  SmartObject()           { }
  SmartObject(Variant v)  { SmartPtr<T>::operator=(v.toObject().get());}
  SmartObject(T *data) : SmartPtr<T>(data) { }
  template<class Y>
  SmartObject(Y *data)    { SmartPtr<T>::operator=(data);}
  template<class Y>
  SmartObject(const SmartPtr<Y> &data) { SmartPtr<T>::operator=(data);}
  SmartObject(Object obj) { SmartPtr<T>::operator=(obj.get());}

  bool instanceof(const char *s) const {
    return SmartPtr<T>::m_px && SmartPtr<T>::m_px->o_instanceof(s);
  }

  /**
   * Assignment
   */
  SmartObject &operator=(CVarRef v) {
    SmartPtr<T>::operator=(v.toObject().get());
    return *this;
  }
  SmartObject &operator=(const SmartPtr<T> &src) {
    SmartPtr<T>::operator=(src);
    return *this;
  }
  SmartObject &operator=(T *src) {
    SmartPtr<T>::operator=(src);
    return *this;
  }
  template<class Y>
  SmartObject &operator=(const SmartPtr<Y> &src) {
    SmartPtr<T>::operator=(src.get());
    return *this;
  }

  /**
   * Conversions
   */
  operator Object() const { return SmartPtr<T>::m_px;}

  template<class Y>
  operator SmartObject<Y>() {
    return static_cast<Y *>(SmartPtr<T>::m_px);
  }

  /**
   * r-value and l-value of object properties.
   */
  Variant o_get(CStrRef propName, int64 hash) const {
    if (SmartPtr<T>::m_px == NULL) return null;
    return SmartPtr<T>::m_px->o_get(propName, hash);
  }
  ObjectOffset o_lval(CStrRef propName, int64 hash) {
    return ObjectOffset(SmartPtr<T>::m_px, propName, hash);
  }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_SMART_OBJET_H__
