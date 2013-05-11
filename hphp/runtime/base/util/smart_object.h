/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
  SmartObject &operator=(CObjRef src) {
    Object::operator=(src);
    return *this;
  }
  SmartObject &operator=(T *src) {
    Object::operator=(src);
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

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_SMART_OBJET_H__
