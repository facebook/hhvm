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

#ifndef incl_HPHP_SMART_OBJECT_H_
#define incl_HPHP_SMART_OBJECT_H_

#include "hphp/runtime/base/smart-ptr.h"
#include "hphp/runtime/base/complex-types.h"

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
  /* implicit */ SmartObject(const Variant& v) : Object(v.toObject().get()) { }
  /* implicit */ SmartObject(T *data) : Object(data) { }
  template<class Y>
  /* implicit */ SmartObject(Y *data) : Object(data) { }
  /* implicit */ SmartObject(const Object& src) : Object(src) { }

  /**
   * Assignment
   */
  SmartObject &operator=(const Variant& v) {
    Object::operator=(v.toObject().get());
    return *this;
  }
  SmartObject &operator=(const SmartPtr<T> &src) {
    Object::operator=(src);
    return *this;
  }
  SmartObject &operator=(const Object& src) {
    Object::operator=(src);
    return *this;
  }
  SmartObject &operator=(T *src) {
    Object::operator=(src);
    return *this;
  }

  T *operator->() const {
    return static_cast<T*>(Object::operator->());
  }

  T *get() const {
    return static_cast<T*>(Object::get());
  }

  static SmartObject attach(T* object) {
    SmartObject o;
    o.m_px = object;
    return o;
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

///////////////////////////////////////////////////////////////////////////////

template <typename T>
class SmartResource : public Resource {
public:
  SmartResource()           { }
  explicit SmartResource(const Variant& v) : Resource(v.toResource().get()) { }
  /* implicit */ SmartResource(T *data) : Resource(data) { }
  template<class Y>
  explicit SmartResource(Y *data) : Resource(data) { }
  explicit SmartResource(const Resource& src) : Resource(src) { }

  /**
   * Assignment
   */
  SmartResource &operator=(const Variant& v) {
    Resource::operator=(v.toObject().get());
    return *this;
  }
  SmartResource &operator=(const SmartPtr<T> &src) {
    Resource::operator=(src);
    return *this;
  }
  SmartResource &operator=(const Resource& src) {
    Resource::operator=(src);
    return *this;
  }
  SmartResource &operator=(T *src) {
    Resource::operator=(src);
    return *this;
  }

  T *operator->() const {
    return static_cast<T*>(Resource::operator->());
  }

  T *get() const {
    return static_cast<T*>(Resource::get());
  }

  template<class Y>
  explicit operator SmartResource<Y>() {
    /*
      Note the cast to T* is intentional. We want to allow this
      conversion only if T is derived from Y, so we are using an
      implicit cast from T* to Y* to achieve that
    */
    return static_cast<T*>(this->m_px);
  }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_SMART_OBJECT_H__
