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
#ifndef incl_HPHP_TLS_POD_BAG_H_
#define incl_HPHP_TLS_POD_BAG_H_

#include <cstdint>
#include <cstring>
#include <type_traits>
#include <cassert>

#include "hphp/util/assertions.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * This class implements a small vector that is suitable for direct
 * placement into thread local storage, to maintain small sets of POD
 * data.
 *
 * The motivating case for this was the strong iterator association in
 * tl_miter_table, so it's got a fairly specialized API.  It has a
 * concept of "unpopulated" slots of the vector, and has methods for
 * finding slots that are currently unpopulated.  This was pulled out
 * in a separate class to it testable, not really to make it reusable.
 *
 * To work with this class, T must be at least 8 bytes, and whatever
 * state the user of this class wants to put into T must involve
 * having a non-zero initial 8 bytes, since this is how we decide
 * whether a slot is populated.
 */
template<class T, class Allocator>
struct TlsPodBag {
  static_assert(
    sizeof(T) >= sizeof(uint64_t),
    "TlsPodBag expects at least 8 bytes per entry"
  );

  static constexpr uint32_t kInitialCap = 4;

  /*
   * No constructor or destructor.  This is a POD, intended to be
   * allocated by value in TLS.  You have to destruct its elements
   * yourself.
   *
   * Zeroing memory and then casting it to a TlsPodBag is guaranteed
   * to produce a TLS vec where empty() is true.
   */

  /*
   * Query the population of the TlsPodBag.
   */
  bool empty() const { return !m_population; }
  uint32_t population() const { return m_population; }

  /*
   * Return a pointer to an unpopulated slot, growing if necessary.
   * Before returning the pointer, this increments the population
   * count---the caller must make some change to this element to cause
   * its first uint64_t bytes to be non-zero.
   */
  T* find_unpopulated() {
    if (m_population == m_capacity) {
      return realloc_find();
    }
    for (auto i = uint32_t{0}; i < m_capacity; ++i) {
      if (unpopulated(&m_data[i])) {
        ++m_population;
        return &m_data[i];
      }
    }
    not_reached();
  }

  /*
   * Find a populated slot, call a function on it, and then remove it.
   */
  template<class Fn>
  void visit_to_remove(Fn fn) {
    assert(!empty());
    for (auto i = uint32_t{0}; i < m_capacity; ++i) {
      if (!unpopulated(&m_data[i])) {
        fn(m_data[i]);
        std::memset(&m_data[i], 0, sizeof m_data[i]);
        --m_population;
        if (!m_population) deallocate();
        return;
      }
    }
    not_reached();
  }

  /*
   * Call a function for each slot in the container, regardless of
   * whether it is populated or not.
   */
  template<class Fn>
  void for_each(Fn fn) {
    for (auto i = uint32_t{0}; i < m_capacity; ++i) {
      fn(m_data[i]);
    }
  }

  /*
   * Mark all slots that match a predicate as unpopulated.  If the
   * population count goes to zero after this, free the memory.
   *
   * Cond must not return true for an unpopulated slot, but it is
   * called for unpopulated slots.
   */
  template<class Cond>
  void release_if(Cond cond) {
    for (auto i = uint32_t{0}; i < m_capacity; ++i) {
      if (cond(m_data[i])) {
        std::memset(&m_data[i], 0, sizeof m_data[i]);
        --m_population;
      }
    }

    if (!m_population) deallocate();
  }

private:
  bool unpopulated(const T* t) const {
    return *reinterpret_cast<const uint64_t*>(t) == 0;
  }

  T* allocate() {
    assert(empty());
    m_data = Allocator{}.allocate(kInitialCap);
    std::memset(m_data, 0, sizeof *m_data * kInitialCap);
    m_capacity = kInitialCap;
    m_population = 1;
    return m_data;
  }

  void deallocate() {
    assert(m_population == 0 && m_data != 0);
    Allocator{}.deallocate(m_data, m_capacity);
    m_capacity = 0;
    m_data = nullptr;
  }

  T* realloc_find() {
    if (!m_capacity) return allocate();

    auto const oldcap = m_capacity;
    auto const newcap = oldcap * 2;
    auto const newdat = Allocator{}.allocate(newcap);
    std::copy(m_data, m_data + oldcap, newdat);
    std::memset(newdat + oldcap, 0, oldcap * sizeof *newdat);
    Allocator{}.deallocate(m_data, oldcap);
    m_data = newdat;
    m_capacity = newcap;
    ++m_population;
    return newdat + oldcap;
  }

private:
  T* m_data;
  uint32_t m_population;
  uint32_t m_capacity;
};

//////////////////////////////////////////////////////////////////////

}

#endif
