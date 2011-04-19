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

#ifndef __HPHP_SMART_OBJECT_H__
#define __HPHP_SMART_OBJECT_H__

#include <runtime/base/util/smart_ptr.h>
#include <runtime/base/complex_types.h>

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
template <typename T>
class SmartObject : public Object {
public:
  SmartObject()           { }
  SmartObject(CVarRef v) : Object(v.toObject().get()) { }
  SmartObject(T *data) : Object(data) { }
  template<class Y>
  SmartObject(Y *data) : Object(data) { }
  template<class Y>
  SmartObject(const SmartPtr<Y> &data) : Object(data.get()) { }
  SmartObject(CObjRef src) : Object(src) { }

  /**
   * Assignment
   */
  SmartObject &operator=(CVarRef v) {
    Object::operator=(v.toObject().get());
    return *this;
  }
  SmartObject &operator=(const SmartPtr<T> &src) {
    Object::operator=(src);
    return *this;
  }
  SmartObject &operator=(T *src) {
    Object::operator=(src);
    return *this;
  }
  template<class Y>
  SmartObject &operator=(const SmartPtr<Y> &src) {
    Object::operator=(src.get());
    return *this;
  }

  T *operator->() const {
    return static_cast<T*>(static_cast<void*>((Object::operator->())));
  }

  T *get() const {
    return static_cast<T*>(static_cast<void*>(Object::get()));
  }

  template<class Y>
  operator SmartObject<Y>() {
    /*
      Note the cast to T* is intentional. We want to allow this
      conversion only if T is derived from Y, so we are using an
      implicit cast from T* to Y* to achieve that
    */
    return static_cast<T*>(this->m_px);
  }
};

template <typename T>
class SmartInterface : public Object {
public:
  SmartInterface() {}
  SmartInterface(CObjRef obj) : Object(obj) { intf = init(m_px); }
  SmartInterface(ObjectData *obj) : Object(obj) { intf = init(m_px); }
  SmartInterface(CVarRef obj) : Object(obj.toObject().get()) {
    intf = init(m_px);
  }
  T* operator->() const { return intf; }
  CObjRef object() const { return *this; }
private:
  T *init(ObjectData *obj) const {
    T *t = dynamic_cast<T*>(obj);
    if (!t) {
      if (ObjectData *parent = obj->getRedeclaredParent()) {
        return init(parent);
      }
      throw InvalidObjectTypeException(m_px->o_getClassName());
    }
    return t;
  }

  T     *intf;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_SMART_OBJET_H__
