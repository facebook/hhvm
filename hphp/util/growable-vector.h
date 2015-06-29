/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_GROWABLE_VECTOR_H_
#define incl_HPHP_GROWABLE_VECTOR_H_

#include <cstdlib>
#include <cstdint>

#include "hphp/util/assertions.h"

namespace HPHP { namespace jit {
////////////////////////////////////////////////////////////////////////////////

/*
 * A vector of elements that will increase its capacity when it fills up.  It
 * differs from std::vector in that it only takes up 8 bytes of space, its
 * elements must be trivially destructible, and it cannot erase individual
 * elements.
 */
template<typename T>
struct GrowableVector {
  explicit GrowableVector() {}

  GrowableVector(GrowableVector&& other) noexcept : m_vec(other.m_vec) {
    other.m_vec = nullptr;
  }

  GrowableVector& operator=(GrowableVector&& other) {
    m_vec = other.m_vec;
    other.m_vec = nullptr;
    return *this;
  }

  GrowableVector(const GrowableVector&) = delete;
  GrowableVector& operator=(const GrowableVector&) = delete;

  //////////////////////////////////////////////////////////////////////////////

  bool empty() const {
    return !m_vec;
  }

  size_t size() const {
    return m_vec ? m_vec->m_size : 0;
  }

  void clear() {
    free(m_vec);
    m_vec = nullptr;
  }

  T& operator[](const size_t idx) {
    assertx(idx < size());
    return m_vec->m_data[idx];
  }

  const T& operator[](const size_t idx) const {
    assertx(idx < size());
    return m_vec->m_data[idx];
  }

  void push_back(const T& datum) {
    if (!m_vec) {
      m_vec = Impl::make();
    }
    m_vec = m_vec->push_back(datum);
  }

  void swap(GrowableVector<T>& other) {
    std::swap(m_vec, other.m_vec);
  }

  T* begin() const {
    return m_vec ? m_vec->m_data : (T*)this;
  }

  T* end() const {
    return m_vec ? &m_vec->m_data[m_vec->m_size] : (T*)this;
  }

  void setEnd(T* newEnd) {
    if (newEnd == begin()) {
      free(m_vec);
      m_vec = nullptr;
      return;
    }
    m_vec->m_size = newEnd - m_vec->m_data;
  }

private:
  struct Impl {
    static Impl* make() {
      static_assert(
        std::is_trivially_destructible<T>::value,
        "GrowableVector can only hold trivially destructible types"
      );
      auto const mem = malloc(sizeof(Impl));
      return new (mem) Impl();
    }

    Impl* push_back(const T& datum) {
      // m_data always has room for at least one element due to the m_data[1]
      // declaration, so the realloc() code first has to kick in when a second
      // element is about to be pushed.
      auto gv = [&] {
        if (!folly::isPowTwo(m_size)) return this;
        return (Impl*)realloc(
          this,
          offsetof(Impl, m_data) + 2 * m_size * sizeof(T)
        );
      }();

      gv->m_data[gv->m_size++] = datum;
      return gv;
    }

    ////////////////////////////////////////////////////////////////////////////

    uint32_t m_size{0};
    /* Actually variable length. */
    T m_data[1];
  };

  //////////////////////////////////////////////////////////////////////////////

  Impl* m_vec{nullptr};
};

////////////////////////////////////////////////////////////////////////////////
}}

#endif
