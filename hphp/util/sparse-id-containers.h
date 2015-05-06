/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_SPARSE_ID_CONTAINERS_H_
#define incl_HPHP_SPARSE_ID_CONTAINERS_H_

#include <cstdlib>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <type_traits>
#include <utility>

#include <folly/gen/String.h>

#include "hphp/util/compilation-flags.h"
#include "hphp/util/safe-cast.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Time-efficient representations for sparse sets and sparse maps keyed with
 * integers from a known universe (i.e. values from zero to some maximum).  It
 * also has the peculiar (but maybe occasionally useful) property of iteration
 * order matching insertion order as long as you haven't erased anything yet.
 *
 * Space consumption is O(universe), where universe is the maximum value the
 * set can hold.  See the constructor for more information on this.  The set
 * version has several member functions with preconditions relating to universe
 * size.
 *
 * The datastructure implemented here is from this:
 *
 *   http://dl.acm.org/citation.cfm?id=176484
 *
 * Some notes about this implementation:
 *
 *    o The set doesn't support set complement.  It's O(universe), so if you
 *      need it it's probably better to use a bitset of some sort.
 *
 *    o Iterators are invalidated on any call to a non-const member function.
 *
 *    o Moving from a set or map leaves it in an undetermined (but valid)
 *      state.  Notably this includes potentially changing its universe size.
 *
 *    o Set operations involving multiple sets are generally only legal if both
 *      sets have the same universe size.  Exceptions are expressions relating
 *      to EqualityComparable, Assignable, and swap(), for reasons relating to
 *      moving potentially changing universe size.
 *
 *    o Lookups and insertions can be done through a different type than the
 *      containers actually hold.  This is to ease using non-integer types as
 *      "keys" in these classes, as long as they can be mapped down to ids.  An
 *      'extractor' function object type can be provided as a template
 *      parameter to control how the mapping works.
 *
 * Also, note that for very small universes, even if the bits are sparse
 * there's a good chance you'll be better off with some kind of bitset than the
 * set version of this.
 *
 */

//////////////////////////////////////////////////////////////////////

namespace sparse_id_detail {

template<class T, class Lookup>
struct default_extract {
  T operator()(Lookup l) const {
    size_t convert = l;
    return safe_cast<T>(convert);
  }
};

}

//////////////////////////////////////////////////////////////////////

template<
  class T,
  class LookupT = T,
  class Extract = sparse_id_detail::default_extract<T,LookupT>
>
struct sparse_id_set {
  using value_type     = T;
  using size_type      = value_type;
  using const_iterator = const T*;

  static_assert(
    std::is_integral<T>::value && std::is_unsigned<T>::value,
    "sparse_id_set is intended for use with unsigned integer types"
  );

  /*
   * When constructing a sparse_id_set, you must provide a 'universe size' for
   * the ids.  This is one greater than the maximum value you'll insert into
   * the set.
   *
   * All functions dealing with values have a precondition that the values fit
   * in the universe size, and most functions involving multiple sparse_id_sets
   * (essentially everything except swap) will have a precondition that the two
   * sets have the same universe size.
   */
  explicit sparse_id_set(T universe_size)
    : m_universe_size{universe_size}
    , m_next{0}
    , m_mem{
        universe_size
          ? static_cast<T*>(std::malloc(sizeof(T) * universe_size * 2))
          : nullptr
      }
  {
    // Note: the sparse part of m_mem is deliberately uninitialized, but we do
    // it for valgrind or asan builds.
#if defined(FOLLY_SANITIZE_ADDRESS) || defined(VALGRIND)
    std::memset(m_mem, 0, sizeof(T) * universe_size);
#endif
  }
  ~sparse_id_set() { if (m_universe_size) std::free(m_mem); }

  /*
   * Copy this set from `o'.
   *
   * Post: operator==(o)
   */
  sparse_id_set(const sparse_id_set& o)
    : sparse_id_set(o.m_universe_size)
  {
    *this |= o;
  }

  /*
   * Move construct a set from `o'.
   *
   * The set `o' is left in an unspecified but valid state.  It's
   * universe_size() is not even guaranteed to be the same after it is
   * moved from.
   */
  sparse_id_set(sparse_id_set&& o) noexcept
    : m_universe_size{o.m_universe_size}
    , m_next{o.m_next}
    , m_mem{o.m_mem}
  {
    o.m_universe_size = 0;
    if (debug) {
      o.m_mem = nullptr;
      o.m_next = 0;
    }
  }

  /*
   * Copy assignment.
   *
   * Post: operator==(o)
   */
  sparse_id_set& operator=(const sparse_id_set& o) {
    if (m_universe_size == o.m_universe_size) {
      clear();
      *this |= o;
      return *this;
    }
    sparse_id_set tmp(o);
    swap(o);
    return *this;
  }

  /*
   * Move assignment.
   *
   * Leaves `o' in an unspecified, but valid state.  It may not have the same
   * universe size that it had before being moved from.
   */
  sparse_id_set& operator=(sparse_id_set&& o) noexcept {
    swap(o);
    if (debug) {
      // Make sure no one relies on the universe staying the same.
      sparse_id_set tmp(0);
      tmp.swap(o);
    }
    return *this;
  }

  /*
   * Returns the universe size of this sparse_id_set.  Once created, a set's
   * universe size can not change unless you move-construct or move-assign from
   * it.
   */
  size_type universe_size() const { return m_universe_size; }

  /*
   * Iteration.  Make sure you don't mutate the set while you're using its
   * iterators.
   *
   * The order of elements in the set is guaranteed to be the same as the order
   * of insertion.
   */
  const_iterator begin() const  { return const_iterator(dense()); }
  const_iterator end() const    { return const_iterator(dense() + m_next); }
  const_iterator cbegin() const { return const_iterator(dense()); }
  const_iterator cend() const   { return const_iterator(dense() + m_next); }

  /*
   * Since we iterate in insertion order, it might be convenient to ask what's
   * at the front or the back.  This class is definitely not a full model of
   * Sequence, however.
   */
  T front() const { assert(!empty()); return dense()[0]; }
  T back() const { assert(!empty()); return dense()[m_next - 1]; }

  /*
   * Number of elements in this set.
   */
  size_type size() const { return m_next; }

  /*
   * Returns: size() != 0
   */
  bool empty() const { return !size(); }

  /*
   * Clear all members from the set.  O(1).
   *
   * Post: empty()
   */
  void clear() { m_next = 0; }

  /*
   * Returns: whether this sparse_id_set contains a particular value.  O(1).
   */
  bool contains(LookupT lt) const {
    return containsImpl(Extract()(lt));
  }

  /*
   * Returns: whether this sparse_id_set contains a particular value.  O(1).
   * Does not require that the id is in range.
   */
  bool contains_safe(LookupT lt) const {
    auto const t = Extract()(lt);
    return t < m_universe_size && containsImpl(t);
  }

  /*
   * Insert a new value into the set.  O(1)
   *
   * Post: contains an element with the id of `lt'
   */
  void insert(LookupT lt) {
    auto const t = Extract()(lt);
    assert(t < m_universe_size);
    if (containsImpl(t)) return;
    dense()[m_next] = t;
    sparse()[t] = m_next;
    ++m_next;
  }

  /*
   * Remove an element from the set, if it is a member.  (Does not assume that
   * it is.)
   *
   * Post: !contains(lt)
   */
  void erase(LookupT lt) {
    auto const t = Extract()(lt);
    assert(t < m_universe_size);
    // Swap with back element and update sparse ptrs.
    auto const didx = sparse()[t];  // possibly reads uninitialized mem
    if (didx >= m_next || dense()[didx] != t) return;
    auto const moving = dense()[m_next - 1];
    sparse()[moving] = didx;
    dense()[didx] = moving;
    --m_next;
    // No need to write to sparse()[t].  If it's read, next and dense are
    // rechecked to ensure it's actually relevant.
  }

  /*
   * These sets are EqualityComparable, even if they aren't from the save
   * universe.  (Rationale: if you move construct from something it's nice that
   * it is still legal to compare to other things it was legally comparable to
   * before that.)
   *
   * This returns whether the two sets have the same elements, regardless of
   * the order they were inserted.
   *
   * But note that it's O(size()) worst case.  You probably should just never
   * use this function.
   */
  bool operator==(const sparse_id_set& o) const {
    if (universe_size() != o.universe_size()) return false;
    if (size() != o.size()) return false;
    for (auto v : *this) if (!o.containsImpl(v)) return false;
    return true;
  }
  bool operator!=(const sparse_id_set& o) const { return !(*this == o); }

  /*
   * Union, difference and intersection operators.
   *
   * All of these operators are only provided as versions that modify the lhs
   * in place.  Idiomatic uses are going to involve updating id sets that
   * already exist, so even with our move-construction support it will tend to
   * involve allocations compared to mutation-based usage-styles.
   *
   * Union (|=) and difference (-=) are O(o.size()), while intersection (&=) is
   * O(this->size()).
   *
   * Pre: universe_size() == o.universe_size()
   */
  sparse_id_set& operator|=(const sparse_id_set& o) {
    assert(m_universe_size == o.m_universe_size);
    for (auto t : o) insert(t);
    return *this;
  }
  sparse_id_set& operator-=(const sparse_id_set& o) {
    assert(m_universe_size == o.m_universe_size);
    for (auto t : o) erase(t);
    return *this;
  }
  sparse_id_set& operator&=(const sparse_id_set& o) {
    assert(m_universe_size == o.m_universe_size);
    auto fwd = T{0};
    auto back = m_next;
    while (fwd != back) {
      assert(fwd < back);
      if (!o.containsImpl(dense()[fwd])) {
        auto const val = dense()[--back];
        sparse()[val] = fwd;
        dense()[fwd] = val;
      } else {
        ++fwd;
      }
    }
    m_next = back;
    return *this;
  }

  /*
   * Swap the contents of two sets.
   *
   * This function is unusual in that it does not have any preconditions about
   * universe sizes matching.
   */
  void swap(sparse_id_set& o) noexcept {
    std::swap(m_universe_size, o.m_universe_size);
    std::swap(m_mem, o.m_mem);
    std::swap(m_next, o.m_next);
  }

  /*
   * Convert a sparse id set to a std::string, intended for debug printing.
   */
  friend std::string show(const sparse_id_set& set) {
    using namespace folly::gen;
    return from(set)
      | eachTo<std::string>()
      | unsplit<std::string>(" ")
      ;
  }

private:
  bool containsImpl(T t) const {
    assert(t < m_universe_size);
    auto const didx = sparse()[t];  // may read uninitialized memory
    return didx < m_next && dense()[didx] == t;
  }

private:
  T* sparse()             { return m_mem; }
  T* dense()              { return m_mem + m_universe_size; }
  const T* sparse() const { return m_mem; }
  const T* dense() const  { return m_mem + m_universe_size; }

private:
  T m_universe_size;
  T m_next;
  T* m_mem;
};

//////////////////////////////////////////////////////////////////////

template<
  class K,
  class V,
  class LookupKey = K,
  class KExtract = sparse_id_detail::default_extract<K,LookupKey>
>
struct sparse_id_map {
  using value_type     = std::pair<const K,V>;
  using size_type      = K;
  using const_iterator = const value_type*;

  static_assert(
    std::is_integral<K>::value && std::is_unsigned<K>::value,
    "sparse_id_set is intended for use with unsigned integer types"
  );

  /*
   * When constructing a sparse_id_map, you must provide a 'universe size' for
   * the ids.  This is one greater than the maximum key you'll insert into the
   * map.
   */
  explicit sparse_id_map(K universe_size)
    : m_universe_size(universe_size)
    , m_next{0}
    , m_mem{
        universe_size
          ? std::malloc(sizeof(K) * universe_size +
              sizeof(value_type) * universe_size)
          : nullptr
      }
  {
    // Note: the sparse part of m_mem is deliberately uninitialized, but we do
    // it for valgrind or asan builds.
#if defined(FOLLY_SANITIZE_ADDRESS) || defined(VALGRIND)
    std::memset(m_mem, 0, sizeof(K) * universe_size);
#endif
  }
  ~sparse_id_map() {
    if (!m_universe_size) return;
    if (!std::is_trivially_destructible<V>::value) {
      for (auto& kv : *this) {
        kv.~value_type();
      }
    }
    std::free(m_mem);
  }

  /*
   * Copy this map from `o'.  Nothrow as long as V has a nothrow copy
   * constructor.
   *
   * Post: operator==(o)
   */
  sparse_id_map(const sparse_id_map& o)
    : m_universe_size{o.m_universe_size}
    , m_next{o.m_next}
    , m_mem{
        m_universe_size
          ? std::malloc(sizeof(K) * m_universe_size +
              sizeof(value_type) * m_universe_size)
          : nullptr
      }
  {
    auto idx = K{0};
    auto initialize = [&] {
      for (; idx < m_next; ++idx) {
        new (&dense()[idx]) value_type(o.dense()[idx]);
        sparse()[o.dense()[idx].first] = idx;
      }
    };
    if (std::is_trivially_destructible<V>::value ||
        std::is_nothrow_copy_constructible<V>::value) {
      initialize();
      return;
    }

    try {
      initialize();
    } catch (...) {
      while (idx-- > 0) {
        dense()[idx].~value_type();
      }
      throw;
    }
  }

  /*
   * Move construct a map from `o'.  Leaves `o' in an unspecified but valid
   * state.  (Notably the universe size may be changed.)
   *
   * Nothrow guarantee.
   */
  sparse_id_map(sparse_id_map&& o) noexcept
    : m_universe_size{o.m_universe_size}
    , m_next{o.m_next}
    , m_mem{o.m_mem}
  {
    o.m_universe_size = 0;
    if (debug) {
      o.m_mem = nullptr;
      o.m_next = 0;
    }
  }

  /*
   * Copy assignment.  Make this map equivalent to `o'.
   *
   * Strong exception guarantee.
   */
  sparse_id_map& operator=(const sparse_id_map& o) {
    sparse_id_map tmp(o);
    swap(tmp);
    return *this;
  }

  /*
   * Move assign from `o'.
   *
   * Leaves `o' in an unspecified but valid state.  Notably the universe size
   * may be changed.
   *
   * Nothrow guarantee.
   */
  sparse_id_map& operator=(sparse_id_map&& o) noexcept {
    swap(o);
    return *this;
  }

  /*
   * Returns the universe size of this sparse_id_map.  Once created, a map's
   * universe size can not change unless you move-construct or move-assign from
   * it.
   */
  size_type universe_size() const { return m_universe_size; }

  /*
   * Iteration.  Make sure you don't mutate the map while you're using its
   * iterators.
   *
   * The order of elements in the map is guaranteed to be the same as the order
   * of insertion.
   */
  const_iterator begin() const  { return const_iterator(dense()); }
  const_iterator end() const    { return const_iterator(dense() + m_next); }
  const_iterator cbegin() const { return const_iterator(dense()); }
  const_iterator cend() const   { return const_iterator(dense() + m_next); }

  /*
   * Since we iterate in insertion order, it might be convenient to ask what's
   * at the front or the back.  This class is definitely not a full model of
   * Sequence, however.
   */
  const value_type& front() const {
    assert(!empty());
    return dense()[0];
  }
  const value_type& back() const {
    assert(!empty());
    return dense()[m_next - 1];
  }

  /*
   * Number of elements in this map.
   */
  size_type size() const { return m_next; }

  /*
   * Returns: size() != 0
   */
  bool empty() const { return !size(); }

  /*
   * Clear all members from the map.  O(1) if V is trivially destructable,
   * O(size()) if not.
   *
   * Post: empty()
   */
  void clear() {
    if (!std::is_trivially_destructible<V>::value) {
      for (auto& kv : *this) {
        kv.~value_type();
      }
    }
    m_next = 0;
  }

  /*
   * Returns: whether this sparse_id_map contains a particular key.  O(1).
   */
  bool contains(LookupKey lk) const {
    return containsImpl(KExtract()(lk));
  }

  /*
   * Returns: whether this sparse_id_map contains a particular value.  O(1).
   * Does not require that the id is in range.
   */
  bool contains_safe(LookupKey lk) const {
    auto const k = KExtract()(lk);
    return k < m_universe_size && containsImpl(k);
  }

  /*
   * Get a reference to the value for key `k', inserting it with a default
   * constructed value if it doesn't exist.  Strong guarantee.
   */
  V& operator[](LookupKey lk) {
    auto const k = KExtract()(lk);
    if (!containsImpl(k)) insert(std::make_pair(k, V{}));
    return dense()[sparse()[k]].second;
  }

  /*
   * Insert a new value into the set.  O(1).  Strong exception guarantee.
   *
   * Post: contains an element with id v.first
   */
  void insert(const value_type& v) {
    assert(v.first < m_universe_size);
    if (containsImpl(v.first)) return;
    new (&dense()[m_next]) value_type(v);
    sparse()[v.first] = m_next;
    ++m_next;
  }

  /*
   * Insert a new value into the set, moving it if we need it.  O(1).  Strong
   * exception guarantee.
   *
   * Post: contains an element with id v.first
   */
  void insert(value_type&& v) {
    assert(v.first < m_universe_size);
    if (containsImpl(v.first)) return;
    new (&dense()[m_next]) value_type(std::move(v));
    sparse()[v.first] = m_next;
    ++m_next;
  }

  /*
   * Remove an element from the set, if it is a member.  (Does not assume that
   * it is.)  No throw as long as V has a nothrow move assignment operator.
   * Strong guarantee otherwise.
   *
   * Post: !contains(lk)
   */
  void erase(LookupKey lk) {
    auto const key = KExtract()(lk);
    assert(key < m_universe_size);
    // Move in back element and update sparse ptrs.
    auto const didx = sparse()[key];  // possibly reads uninitialized mem
    if (didx >= m_next || dense()[didx].first != key) return;
    auto& moving = dense()[m_next - 1];
    auto const moved_key = moving.first;
    if (didx < m_next - 1) {
      dense()[didx].second = std::move(moving.second);
      const_cast<K&>(dense()[didx].first) = moved_key;
    }
    sparse()[moved_key] = didx;
    dense()[m_next - 1].~value_type();
    --m_next;
    // No need to write to sparse()[t].  If it's read, next and dense are
    // rechecked to ensure it's actually relevant.
  }

  /*
   * Model EqualityComparable, as long as the value is EqualityComparable.
   * Note that it's O(size()) worst case.
   *
   * This returns whether the two maps have equivalent key value pairs,
   * regardless of the order they were inserted.
   */
  bool operator==(const sparse_id_map& o) const {
    if (universe_size() != o.universe_size()) return false;
    if (size() != o.size()) return false;
    for (auto& kv : *this) {
      if (!o.containsImpl(kv.first)) return false;
      if (!(o.dense()[o.sparse()[kv.first]].second == kv.second)) {
        return false;
      }
    }
    return true;
  }
  bool operator!=(const sparse_id_map& o) const { return !(*this == o); }

  /*
   * Merge a map into this one by intersecting the keys, and using a
   * user-defined function to merge values that were present in both this and
   * `o'.  The user-defined value merge function should return a bool,
   * indicating whether the value should be considered changed.
   *
   * Basic guarantee only.  Nothrow if V has a nothrow move constructor or a
   * nothrow copy constructor.
   *
   * Complexity: O(size()).
   *
   * Returns: true if this map changed keys, or if the user-supplied function
   * returned true for any pair of values.
   *
   * Pre: universe_size() == o.universe_size()
   */
  template<class Fun>
  bool merge(const sparse_id_map& o, Fun val_merge) {
    assert(m_universe_size == o.m_universe_size);
    auto fwd = K{0};
    auto changed = false;
    while (fwd != m_next) {
      assert(fwd < m_next);
      auto const k = dense()[fwd].first;
      if (!o.containsImpl(k)) {
        changed = true;
        if (fwd == m_next - 1) {  // Avoid self-move assigning values.
          --m_next;
          continue;
        }
        // Order here is important for exception safety: we can't decrement
        // m_next until we've moved-from and then destroyed the old value.
        auto& tomove = dense()[m_next - 1];
        sparse()[tomove.first] = fwd;
        dense()[fwd].second = std::move(tomove.second);
        const_cast<K&>(dense()[fwd].first) = tomove.first;
        tomove.~value_type();
        --m_next;
        continue;
      }
      if (val_merge(dense()[fwd].second, o.dense()[o.sparse()[k]].second)) {
        changed = true;
      }
      ++fwd;
    }
    return changed;
  }

  /*
   * Swap the contents of two maps.
   */
  void swap(sparse_id_map& o) noexcept {
    std::swap(m_universe_size, o.m_universe_size);
    std::swap(m_mem, o.m_mem);
    std::swap(m_next, o.m_next);
  }

private:
  bool containsImpl(K k) const {
    assert(k < m_universe_size);
    auto const didx = sparse()[k];  // may read uninitialized memory
    return didx < m_next && dense()[didx].first == k;
  }

private:
  K* sparse() { return static_cast<K*>(m_mem); }
  const K* sparse() const { return static_cast<K*>(m_mem); }
  value_type* dense() {
    void* vpDense = sparse() + m_universe_size;
    return static_cast<value_type*>(vpDense);
  }
  const value_type* dense() const {
    return const_cast<sparse_id_map*>(this)->dense();
  }

private:
  K m_universe_size;
  K m_next;
  void* m_mem;
};

//////////////////////////////////////////////////////////////////////

// Non-member swaps for ADL swap idiom.

template<class T, class LT, class EX>
void swap(sparse_id_set<T,LT,EX>& a, sparse_id_set<T,LT,EX>& b) {
  a.swap(b);
}

template<class K, class V, class LK, class LKE>
void swap(sparse_id_map<K,V,LK,LKE>& a, sparse_id_map<K,V,LK,LKE>& b) {
  a.swap(b);
}

//////////////////////////////////////////////////////////////////////

}

#endif
