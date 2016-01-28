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
#ifndef incl_HPHP_DATAFLOW_WORKLIST_H_
#define incl_HPHP_DATAFLOW_WORKLIST_H_

#include <cstdint>
#include <queue>
#include <vector>
#include <type_traits>
#include <algorithm>
#include <functional>
#include <utility>
#include <cassert>

#include <boost/dynamic_bitset.hpp>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Encapsulates a unique priority queue of an integer type, where elements in
 * the queue are ids less than some universe size.
 *
 * This is useful for dataflow worklists, where we want to visit blocks with
 * lower (or higher) ids before blocks with higher (or lower) ids, but also
 * don't want to schedule something that's already scheduled.  By default this
 * returns lower ids first (note that this is returning things that are larger
 * according to its Compare function, like std::priority_queue).
 */
template<class T, class Compare = std::greater<T>>
struct dataflow_worklist {
  static_assert(
    std::is_integral<T>::value && std::is_unsigned<T>::value,
    "dataflow_worklist requires an unsigned integer type"
  );

  explicit dataflow_worklist(T universe_size)
    : m_set(universe_size)
    , m_q(
        Compare{},
        [&] {
          auto r = std::vector<T>{};
          r.reserve(universe_size);
          return r;
        }()
      )
  {}

  // We have to manually implement move, because boost::dynamic_bitset doesn't
  // support it (at the time of this writing).
  dataflow_worklist(dataflow_worklist&& o) noexcept
    : dataflow_worklist(0)
  {
    swap(o);
  }

  dataflow_worklist& operator=(dataflow_worklist&& o) noexcept {
    dataflow_worklist tmp(std::move(o));
    tmp.swap(*this);
    return *this;
  }

  void swap(dataflow_worklist& o) {
    using std::swap;
    swap(m_set, o.m_set);
    swap(m_q, o.m_q);
  }

  bool empty() const { return m_q.empty(); }
  T top() { assert(!empty()); return m_q.top(); }

  T pop() {
    auto const t = top();
    assert(m_set.test(t));
    m_q.pop();
    m_set.reset(t);
    return t;
  }

  /*
   * Enqueue t. Returns true iff the item was newly inserted.
   */
  bool push(T t) {
    assert(t < m_set.size());
    if (m_set[t]) return false;
    m_q.push(t);
    m_set.set(t);
    return true;
  }

private:
  boost::dynamic_bitset<> m_set;
  std::priority_queue<T,std::vector<T>,Compare> m_q;
};

//////////////////////////////////////////////////////////////////////

}

#endif
