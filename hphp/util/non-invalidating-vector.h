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
#ifndef incl_NON_INVALIDATING_VECTOR_H_
#define incl_NON_INVALIDATING_VECTOR_H_

#include "hphp/util/portability.h"

#include <vector>

namespace HPHP {

/*
 * A vector-like container which can resize without invalidating
 * references to its contents while maintaining O(1) access (unlike
 * deque).
 *
 * It accomplishes this, by instead of resizing the vector, it creates
 * a new (larger) vector, copies the contents into it, then stores the
 * original vector away (to keep it from being destroyed). Since the
 * original vector never resizes, it never moves any of its contents
 * around, so references remain valid. Likewise, the vectors are kept
 * alive until everything is destroyed.
 *
 * The downsides are:
 *
 * - T must be copyable (and we cannot leverage move constructors)
 *
 * - T must have value identity. That is, two Ts should be
 *   semantically identical if they have the same contents. Their
 *   memory address should not be meaningful. This is because
 *   operator[] can return different references to the "same" T at
 *   different times.
 *
 * - Once inserted, you can only obtain a const reference to the
 *   T. Since there may be multiple of the "same" T, it's not clear
 *   which one you'd be modifying.
 *
 * - You can only insert into the container, not remove
 *   anything. Removing could invalidate references.
 *
 * - Since we may make multiple copies of the same value, this will
 *   use more memory than the equivalent std::vector. One can avoid
 *   this by using reserve() to pre-size the vector to avoid too many
 *   copies.
 */

template <typename T>
struct NonInvalidatingVector {
  NonInvalidatingVector() = default;

  // We don't need to make a copy of m_saved, since nothing can have a
  // reference to the copy.
  NonInvalidatingVector(NonInvalidatingVector& o) : m_data{o.m_data} {}
  NonInvalidatingVector& operator=(NonInvalidatingVector& o) {
    if (this == &o) return *this;
    m_data = o.m_data;
    m_saved.clear();
    return *this;
  }

  NonInvalidatingVector(NonInvalidatingVector&&) = default;
  NonInvalidatingVector& operator=(NonInvalidatingVector&&) = default;

  size_t size() const { return m_data.size(); }

  const T& operator[](size_t i) const { return m_data[i]; }

  template <typename ...Args>
  void emplace_back(Args&&... args) {
    if (UNLIKELY(m_data.size() == m_data.capacity() &&
                 !m_data.empty())) {
      auto const newCapacity =
        m_data.capacity() > 0 ? m_data.capacity() * 2 : 1;
      save(newCapacity);
    }
    m_data.emplace_back(std::forward<Args>(args)...);
  }

  void reserve(size_t capacity) {
    if (!m_data.empty() && capacity > m_data.capacity()) {
      save(capacity);
    } else {
      m_data.reserve(capacity);
    }
  }

private:
  void save(size_t capacity) {
    decltype(m_data) newData;
    newData.reserve(capacity);
    newData.insert(newData.end(), m_data.begin(), m_data.end());
    m_saved.emplace_back(std::move(m_data));
    m_data = std::move(newData);
  }

  std::vector<T> m_data;
  std::vector<std::vector<T>> m_saved;
};

}

#endif
