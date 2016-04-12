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
#ifndef incl_HPHP_COPY_PTR_H_
#define incl_HPHP_COPY_PTR_H_

#include <utility>
#include <memory>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * A smart pointer that does deep copies when you copy construct it.
 */
template<class T>
struct copy_ptr {
  copy_ptr() noexcept : m_p(nullptr) {}
  explicit copy_ptr(T* t) noexcept : m_p(t) {}

  copy_ptr(const copy_ptr& o) : m_p(o.m_p ? new T(*o.m_p) : nullptr) {}

  copy_ptr(copy_ptr&& o) noexcept
    : m_p(o.m_p)
  {
    o.m_p = nullptr;
  }

  copy_ptr& operator=(const copy_ptr& o) {
    if (!m_p) {
      m_p = o.m_p ? new T(*o.m_p) : nullptr;
      return *this;
    }

    if (o.m_p) {
      *m_p = *o.m_p;
      return *this;
    }

    delete m_p;
    m_p = nullptr;
    return *this;
  }

  copy_ptr& operator=(copy_ptr&& o) noexcept {
    delete m_p;
    m_p = o.m_p;
    o.m_p = nullptr;
    return *this;
  }

  ~copy_ptr() {
    delete m_p;
#ifdef DEBUG
    m_p = nullptr;
#endif
  }

  explicit operator bool() const { return !!m_p; }

  T& operator*() const { return *m_p; }
  T* operator->() const { return m_p; }
  T* get() const { return m_p; }

  void reset(T* p = nullptr) {
    std::unique_ptr<T> old(m_p);
    m_p = p;
  }

private:
  T* m_p;
};

template<class T, class... Args>
copy_ptr<T> make_copy_ptr(Args&&... t) {
  return copy_ptr<T>(new T(std::forward<Args>(t)...));
}

//////////////////////////////////////////////////////////////////////

}

#endif
