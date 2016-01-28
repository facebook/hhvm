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

#ifndef incl_HPHP_DEFAULT_PTR_H_
#define incl_HPHP_DEFAULT_PTR_H_

#include <cstddef>
#include <utility>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * Pointer which can be safely const-dereferenced when null to yield a
 * default-constructed value.
 */
template <class T>
struct default_ptr {
  /*
   * Constructors.
   */
  default_ptr() = default;

  /* implicit */ default_ptr(std::nullptr_t) {}

  /* implicit */ default_ptr(T* p) : m_p{p ? p : fallback()} {}

  /*
   * Assignments.
   */
  default_ptr& operator=(std::nullptr_t p) {
    m_p = fallback();
    return *this;
  }
  default_ptr& operator=(T* p) {
    m_p = p ? p : fallback();
    return *this;
  }

  /*
   * Observers.
   */
  const T* get() const {
    return m_p;
  }
  const T& operator*() const {
    return *get();
  }
  const T* operator->() const {
    return get();
  }

  T* raw() const {
    return m_p == &s_default ? nullptr : m_p;
  }
  explicit operator bool() const {
    return raw();
  }

  /*
   * Modifiers.
   */
  void reset(T* p = nullptr) {
    operator=(p);
  }

private:
  /*
   * Internals.
   */
  static T* fallback() {
    return const_cast<T*>(&s_default);
  }

private:
  T* m_p{const_cast<T*>(&s_default)};
  static const T s_default;
};

template <class T>
const T default_ptr<T>::s_default;

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_DEFAULT_PTR_H_
