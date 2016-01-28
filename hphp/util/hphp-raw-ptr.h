/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_HPHP_RAW_PTR_H_
#define incl_HPHP_HPHP_RAW_PTR_H_

#include <memory>

#include "hphp/util/assertions.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * This is a smart pointer that wraps a raw pointer, where the
 * underlying value is expected to be managed by a shared_ptr.
 *
 * This class is implicitly convertable to and from shared_ptrs
 * (assuming the underlying managed object has
 * enable_shared_from_this).
 *
 * The point of this class was to work around some cases where
 * shared_ptr's were being heavily passed by value in the compiler.
 * Switching to raw pointers in a few select cases was a major
 * compile-time perf win for hphp.
 */
template <class T> class hphp_raw_ptr {
public:
  hphp_raw_ptr() : px(0) {}
  explicit hphp_raw_ptr(T *p) : px(p) {}

  template <class S>
  /* implicit */ hphp_raw_ptr(const std::shared_ptr<S> &p) : px(p.get()) {}
  template <class S>
  /* implicit */ hphp_raw_ptr(const hphp_raw_ptr<S> &p) : px(p.get()) {}

  std::shared_ptr<T> lock() const {
    return px ? std::static_pointer_cast<T>(px->shared_from_this()) :
      std::shared_ptr<T>();
  }
  bool expired() const {
    return !px;
  }

  template <class S>
  /* implicit */ operator std::shared_ptr<S>() const {
    S *s = px; // just to verify the implicit conversion T->S
    return s ? std::static_pointer_cast<S>(px->shared_from_this()) :
      std::shared_ptr<S>();
  }

  T *operator->() const { assert(px); return px; }
  T *get() const { return px; }
  explicit operator bool() const { return !expired(); }
  void reset() { px = 0; }
private:
  T     *px;
};

#define IMPLEMENT_PTR_OPERATORS(A, B) \
  template <class T, class U> \
  inline bool operator==(const A<T> &p1, const B<U> &p2) { \
    return p1.get() == p2.get(); \
  } \
  template <class T, class U> \
  inline bool operator!=(const A<T> &p1, const B<U> &p2) { \
    return p1.get() != p2.get(); \
  } \
  template <class T, class U> \
  inline bool operator<(const A<T> &p1, const B<U> &p2) { \
    return intptr_t(p1.get()) < intptr_t(p2.get()); \
  }

IMPLEMENT_PTR_OPERATORS(hphp_raw_ptr, hphp_raw_ptr);
IMPLEMENT_PTR_OPERATORS(hphp_raw_ptr, std::shared_ptr);
IMPLEMENT_PTR_OPERATORS(std::shared_ptr, hphp_raw_ptr);

#undef IMPLEMENT_PTR_OPERATORS

template <typename T, typename U>
HPHP::hphp_raw_ptr<T> dynamic_pointer_cast(HPHP::hphp_raw_ptr<U> p) {
  return HPHP::hphp_raw_ptr<T>(dynamic_cast<T*>(p.get()));
}

template <typename T, typename U>
HPHP::hphp_raw_ptr<T> static_pointer_cast(HPHP::hphp_raw_ptr<U> p) {
  return HPHP::hphp_raw_ptr<T>(static_cast<T*>(p.get()));
}

//////////////////////////////////////////////////////////////////////

}


#endif
