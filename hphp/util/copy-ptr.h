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
#ifndef incl_HPHP_COPY_PTR_H_
#define incl_HPHP_COPY_PTR_H_

#include <utility>
#include <memory>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * A refcounted smart pointer that does deep copies if you ask for a
 * mutable copy when the ref-count is greater than one.
 */
template<class T>
struct copy_ptr {
  copy_ptr() noexcept {}
  explicit copy_ptr(T* t) : m_p(t) {}
  copy_ptr(const copy_ptr& o) = default;
  copy_ptr(copy_ptr&& o) noexcept = default;
  copy_ptr& operator=(const copy_ptr& o) = default;
  copy_ptr& operator=(copy_ptr&& o) noexcept = default;

  explicit operator bool() const { return !!m_p; }

  const T& operator*() const { return *m_p; }
  const T* operator->() const { return m_p.get(); }
  const T* get() const { return m_p.get(); }
  T* mutate() {
    if (m_p.use_count() != 1) {
      emplace(*m_p);
    }
    return m_p.get();
  }

  void reset(T* p = nullptr) {
    m_p.reset(p);
  }

  template <typename... Args> void emplace(Args&&... args) {
    m_p = std::make_shared<T>(std::forward<Args>(args)...);
  }
private:
  std::shared_ptr<T> m_p;
};

//////////////////////////////////////////////////////////////////////

}

#endif
