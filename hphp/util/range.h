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

#ifndef RANGE_H_
#define RANGE_H_

#include "hphp/util/meta.h"

namespace HPHP {

/**
 * DefaultRangePolicy --
 * MappedRangePolicy --
 * KeyRangePolicy --
 *
 * Parameters to Range<...>.
 *
 * Type functions for pulling values out of types. Both a compile-time
 * function providing the type value_type, and a runtime function extract().
 */
template<typename Iter>
struct DefaultRangePolicy {
  typedef typename match_iterator<
    Iter,
    typename std::iterator_traits<Iter>::value_type
  >::type value_type;
  static value_type& extract(const Iter& i) {
    return *i;
  }
};

template<typename Iter>
struct MappedRangePolicy {
  typedef typename match_iterator<
    Iter,
    typename Iter::value_type::second_type
  >::type value_type;
  static value_type& extract(const Iter& i) {
    return i->second;
  }
};

template<typename Iter>
struct KeyRangePolicy {
  // The keys in a map cannot be mutable.
  typedef typename boost::add_const<typename Iter::value_type::first_type
  >::type value_type;
  value_type& extract(const Iter& i) {
    return i->first;
  }
};

/**
 * IterRange --
 *
 * A Range encapsulating forward iteration over typical iterators,
 * including pointers.
 *
 * You can control mutability and the iterator type, and the mapping from
 * iterators to range values, via It and RangePolicy. Mutation
 * happens in the underlying containers, assuming well-behaved iterators.
 *
 * E.g., mutable range of ints:
 *   Range<vector<int>::iterator> rng;
 *
 * Immutable range of the ints in a string->int map:
 *   typedef map<string, int> SIMap;
 *   Range<SIMap::const_iterator, MappedRangePolicy> rng;
 */
template<typename Iter,
         template<typename> class RangePolicy = DefaultRangePolicy>
class IterRange {
  typedef RangePolicy<Iter> Extractor;
public:
  typedef typename Extractor::value_type value_type;
  IterRange(const Iter& begin, const Iter& end) :
    m_next(begin), m_end(end) {}

  IterRange(const IterRange& r) :
    m_next(r.m_next), m_end(r.m_end) {}

  IterRange& operator=(const IterRange& r) {
    m_next = r.m_next;
    m_end = r.m_end;
    return *this;
  }

  value_type& front() const {
    return Extractor::extract(m_next);
  }
  value_type& popFront() {
    value_type& tmp = front();
    m_next++;
    return tmp;
  }

  bool empty() const { return m_next == m_end; }

  // Support begin() and end() so this can be used with range-based
  // for.
  Iter begin() const { return m_next; }
  Iter end() const { return m_end; }

private:
  Iter m_next, m_end;
};

/**
 * Range --
 *
 *   A range that spans an STL-like container. Just adds a convenience constructor
 *   for begin()/end() ranges.
 */
template<typename Cont,
         typename It = typename Cont::const_iterator,
         template<typename> class RangePolicy = DefaultRangePolicy>
struct Range : public IterRange<It, RangePolicy> {
  explicit Range(typename match_iterator<It, Cont>::type& c) :
     IterRange<It, RangePolicy>(c.begin(), c.end()) { }

  using IterRange<It,RangePolicy>::begin;
  using IterRange<It,RangePolicy>::end;
};

}

#endif /* RANGE_H_ */
