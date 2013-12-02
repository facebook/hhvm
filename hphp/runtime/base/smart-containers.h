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
#ifndef incl_HPHP_RUNTIME_BASE_MEMORY_SMART_CONTAINERS_H_
#define incl_HPHP_RUNTIME_BASE_MEMORY_SMART_CONTAINERS_H_

#include <cstdlib>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <queue>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <boost/container/flat_map.hpp>

#include "hphp/runtime/base/memory-manager.h"

namespace HPHP { namespace smart {

//////////////////////////////////////////////////////////////////////

/*
 * Defines a family of types similar to std:: collections and
 * pointers, except using the request-local allocator, which we
 * consider smart.
 *
 * Replace std:: with smart:: if you know the data is request-local.
 *
 * You can also use smart::Allocator as a model of folly's
 * SimpleAllocator where appropriate.
 */

//////////////////////////////////////////////////////////////////////

// STL-style allocator for the smart allocator.  (Unfortunately we
// can't use allocator_traits yet.)

template <class T>
struct Allocator {
  typedef T              value_type;
  typedef T*             pointer;
  typedef const T*       const_pointer;
  typedef T&             reference;
  typedef const T&       const_reference;
  typedef std::size_t    size_type;
  typedef std::ptrdiff_t difference_type;

  template <class U>
  struct rebind {
    typedef Allocator<U> other;
  };

  pointer address(reference value) const {
    return &value;
  }
  const_pointer address(const_reference value) const {
    return &value;
  }

  Allocator() noexcept {}
  Allocator(const Allocator&) noexcept {}
  template<class U> Allocator(const Allocator<U>&) noexcept {}
  ~Allocator() noexcept {}

  size_type max_size() const {
    return std::numeric_limits<std::size_t>::max() / sizeof(T);
  }

  pointer allocate(size_type num, const void* = 0) {
    pointer ret = (pointer)smart_malloc(num * sizeof(T));
    return ret;
  }

  template<class... Args>
  void construct(pointer p, Args&&... args) {
    new ((void*)p) T(std::forward<Args>(args)...);
  }

  void destroy(pointer p) {
    p->~T();
  }

  void deallocate(pointer p, size_type num) {
    smart_free(p);
  }

  template<class U> bool operator==(const Allocator<U>&) const {
    return true;
  }

  template<class U> bool operator!=(const Allocator<U>&) const {
    return false;
  }
};

/*
 * Shorthand to create an std::unique_ptr to a smart-allocated object.
 *
 * Usage:
 *
 *   auto ptr = smart::make_unique<Foo>(arg1, arg2);
 */

template<class T>
using unique_ptr = typename folly::AllocatorUniquePtr<T,Allocator<T>>::type;

template<class T, class... Args>
unique_ptr<T> make_unique(Args&&... args) {
  return folly::allocate_unique<T>(
    Allocator<T>(),
    std::forward<Args>(args)...
  );
}

#ifndef __APPLE__ // XXX: this affects codegen quality but not correctness
static_assert(
  sizeof(unique_ptr<int>) == sizeof(std::unique_ptr<int>),
  "smart::unique_ptr pointer should not be larger than std::unique_ptr"
);
#endif

template <class Key, class T, class Compare = std::less<Key>>
using map = std::map<Key, T, Compare, Allocator<std::pair<const Key,T>>>;

template <class Key, class T, class Compare = std::less<Key>>
using multimap = std::multimap<Key,T,Compare,Allocator<std::pair<const Key,T>>>;

template <class T, class Compare = std::less<T>>
using set = std::set<T, Compare, Allocator<T>>;

template <class T, class Compare = std::less<T>>
using multiset = std::set<T, Compare, Allocator<T>>;

template <class T>
using deque = std::deque<T,Allocator<T>>;

template <class T>
using vector = std::vector<T,Allocator<T>>;

template <class T, class Container = deque<T>>
using stack = std::stack<T, Container>;

template <class T>
using list = std::list<T,Allocator<T>>;

template <class T>
using queue = std::queue<T, deque<T>>;

template <class T, class Compare = std::less<T>>
using priority_queue = std::priority_queue<T, vector<T>, Compare>;

template<class K, class V, class Pred = std::less<K>>
using flat_map = boost::container::flat_map<K,V,Pred,Allocator<std::pair<K,V>>>;

template<class K, class V, class Pred = std::less<K>>
using flat_multimap =
  boost::container::flat_multimap<K,V,Pred,Allocator<std::pair<K,V>>>;

template<class K, class Pred = std::less<K>>
using flat_set = boost::container::flat_set<K, Pred, Allocator<K>>;

template<class K, class Pref = std::less<K>>
using flat_multiset = boost::container::flat_multiset<K, Pref, Allocator<K>>;

/*
 * We are deriving from the std::collection classes to get
 * smart::collection classes that use smart allocation. To avoid the
 * various issues involved with deriving from value types, we want to
 * make sure that there are no references to the base classes here
 * other than the ones below. That way we know that a pointer to a
 * smart::collection can never decay to a pointer to a
 * std::collection.
 *
 * Derivation from value types is generally bad. We also add no
 * functionality to the derived class. Your code will not get past
 * code review if you try to do so.
 *
 * When we upgrade compilers we can change these to C++11 type
 * aliases.
 */

template <class T,
          class U,
          class V = hphp_hash<T>,
          class W = std::equal_to<T>>
struct hash_map : std::unordered_map<
  T, U, V, W,
  Allocator<std::pair<T,U>>
> {
  hash_map()
    : std::unordered_map<
        T, U, V, W,
        Allocator<std::pair<T,U>>
      >(0)
  {}
};

template <class T,
          class U,
          class V = hphp_hash<T>,
          class W = std::equal_to<T>>
struct hash_multimap : std::unordered_multimap<
  T, U, V, W,
  Allocator<std::pair<T,U>>
> {
  hash_multimap()
    : std::unordered_multimap<
        T, U, V, W,
        Allocator<std::pair<T,U>>
      >(0)
  {}
};

template <class T,
          class V = hphp_hash<T>,
          class W = std::equal_to<T>>
struct hash_set : std::unordered_set<T,V,W,Allocator<T> > {
  hash_set()
    : std::unordered_set<T,V,W,Allocator<T>>(0)
  {}
};

//////////////////////////////////////////////////////////////////////

}}

#endif
