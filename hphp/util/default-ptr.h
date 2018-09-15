/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
  default_ptr() : m_p{fallback()} {}

  /* implicit */ default_ptr(std::nullptr_t) {}

  /* implicit */ default_ptr(T* p) : m_p{p ? p : fallback()} {}

  /*
   * Thread-safe allocation.
   */
  T* ensureAllocated() {
    if (auto p = raw()) return p;
    auto ptr = new T();
    T* expected = fallback();
    if (!m_p.compare_exchange_strong(
          expected, ptr, std::memory_order_relaxed)) {
      // Already set by someone else, use theirs.
      delete ptr;
      return expected;
    } else {
      return ptr;
    }
  }

  /*
   * Assignments.
   */
  default_ptr& operator=(std::nullptr_t /*p*/) {
    m_p.store(fallback(), std::memory_order_relaxed);
    return *this;
  }
  default_ptr& operator=(T* p) {
    m_p.store(p ? p : fallback(), std::memory_order_relaxed);
    return *this;
  }

  /*
   * Observers.
   */
  const T* get() const {
    return m_p.load(std::memory_order_relaxed);
  }
  const T& operator*() const {
    return *get();
  }
  const T* operator->() const {
    return get();
  }

  T* raw() const {
    auto p = m_p.load(std::memory_order_relaxed);
    return p == &s_default ? nullptr : p;
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
  std::atomic<T*> m_p{const_cast<T*>(&s_default)};
  static const T s_default;
};

template <class T>
const T default_ptr<T>::s_default;

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_DEFAULT_PTR_H_
