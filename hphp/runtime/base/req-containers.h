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
#ifndef incl_HPHP_RUNTIME_BASE_REQ_CONTAINERS_H_
#define incl_HPHP_RUNTIME_BASE_REQ_CONTAINERS_H_

#include <cstdlib>
#include <deque>
#include <forward_list>
#include <functional>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <boost/container/flat_map.hpp>

#include <folly/Memory.h>

#include "hphp/runtime/base/req-malloc.h"

#include "hphp/util/fixed-vector.h"
#include "hphp/util/tiny-vector.h"

namespace HPHP { namespace req {

//////////////////////////////////////////////////////////////////////

/*
 * Defines a family of types similar to std:: collections and
 * pointers, except using the request-local allocator.
 *
 * Replace std:: with req:: if you know the data is request-local.
 */

/*
 * All these replacements purposefully derive from standard types and
 * containers, which is normally not recommended. This is needed so we can add
 * GC type-scanner annotations. This is safe because we don't add any members,
 * nor change any functionality.
 */

/*
 * Shorthands to create std::unique_ptr or std::shared_ptr to a
 * heap-allocated object. Memory will be request-scoped; the pointer
 * wrapper remembers how to properly delete the object with req::Allocator.
 *
 * Usage:
 *   auto ptr = req::make_unique<Foo>(...);
 *   auto ptr = req::make_shared<Foo>(...);
 */

template <typename T>
struct unique_ptr final : folly::AllocatorUniquePtr<T, Allocator<T>>::type {
  using Base = typename folly::AllocatorUniquePtr<T,Allocator<T>>::type;
  using Base::Base;
};

template<typename T, class... Args>
unique_ptr<T> make_unique(Args&&... args) {
  return folly::allocate_unique<T>(
    Allocator<T>(),
    std::forward<Args>(args)...
  );
}

template <typename T>
struct shared_ptr final : std::shared_ptr<T> {
  using Base = std::shared_ptr<T>;
  using Base::Base;
};

template<class T, class... Args>
shared_ptr<T> make_shared(Args&&... args) {
  return static_cast<shared_ptr<T>>(
    std::allocate_shared<T>(
      Allocator<T>(),
      std::forward<Args>(args)...
    )
  );
}

#ifndef __APPLE__ // XXX: this affects codegen quality but not correctness
static_assert(
  sizeof(unique_ptr<int>) == sizeof(std::unique_ptr<int>),
  "req::unique_ptr pointer should not be larger than std::unique_ptr"
);
#endif

/*
 * Container replacements.
 */

template <typename Key,
          typename T,
          typename Compare = std::less<Key>>
struct map final : std::map<Key, T, Compare,
                            Allocator<std::pair<const Key,T>>
                            > {
  using Base = std::map<Key, T, Compare,
                        Allocator<std::pair<const Key, T>>>;
  using Base::Base;
};

template <typename Key,
          typename T,
          typename Compare = std::less<Key>>
struct multimap final : std::multimap<Key, T, Compare,
                                      Allocator<std::pair<const Key,T>>
                                      > {
  using Base = std::multimap<Key, T, Compare,
                             Allocator<std::pair<const Key, T>>>;
  using Base::Base;
};

template <typename T, typename Compare = std::less<T>>
struct set final : std::set<T, Compare, Allocator<T>> {
  using Base = std::set<T, Compare, Allocator<T>>;
  using Base::Base;
};

template <typename T, typename Compare = std::less<T>>
struct multiset final : std::multiset<T, Compare, Allocator<T>> {
  using Base = std::multiset<T, Compare, Allocator<T>>;
  using Base::Base;
};

template <typename T>
struct deque final : std::deque<T, Allocator<T>> {
  using Base = std::deque<T, Allocator<T>>;
  using Base::Base;
};

template <typename T>
struct vector final : std::vector<T, Allocator<T>> {
  using Base = std::vector<T, Allocator<T>>;
  using Base::Base;
};

template <typename T>
struct list final : std::list<T, Allocator<T>> {
  using Base = std::list<T, Allocator<T>>;
  using Base::Base;
};

template <typename T>
struct forward_list final : std::forward_list<T, Allocator<T>> {
  using Base = std::forward_list<T, Allocator<T>>;
  using Base::Base;
};

template <typename T, typename Container = req::deque<T>>
using stack = std::stack<T, Container>;

template <typename T, typename Container = req::deque<T>>
using queue = std::queue<T, Container>;

template <typename T,
          typename Container = req::vector<T>,
          typename Compare = std::less<T>>
using priority_queue = std::priority_queue<T, Container, Compare>;

#ifdef HAVE_BOOST1_49
// These classes are oddly broken in older boost versions.

template<typename K, typename V, typename Pred = std::less<K>>
struct flat_map final : boost::container::flat_map<
  K, V, Pred, Allocator<std::pair<K,V>>
> {
  using Base =
    boost::container::flat_map<K, V, Pred, Allocator<std::pair<K,V>>>;
  using Base::Base;
};

template<typename K, typename V, typename Pred = std::less<K>>
struct flat_multimap final : boost::container::flat_multimap<
  K, V, Pred, Allocator<std::pair<K,V>>
> {
  using Base = boost::container::flat_multimap<
    K, V, Pred, Allocator<std::pair<K,V>>
  >;
  using Base::Base;
};

#else
template <typename Key, typename T, typename Compare = std::less<Key>>
using flat_map = req::map<Key, T, Compare>;

template <typename Key, typename T, typename Compare = std::less<Key>>
using flat_multimap = req::multimap<Key,T,Compare>;
#endif

template<typename K, typename Compare = std::less<K>>
struct flat_set final : boost::container::flat_set<K, Compare,
                                                   Allocator<K>> {
  using Base = boost::container::flat_set<K, Compare, Allocator<K>>;
  using Base::Base;
};

template<typename K, typename Compare = std::less<K>>
struct flat_multiset final : boost::container::flat_multiset<
  K, Compare, Allocator<K>
> {
  using Base =
    boost::container::flat_multiset<K, Compare, Allocator<K>>;
  using Base::Base;
};

template <class T,
          class U,
          class V = std::hash<T>,
          class W = std::equal_to<T>>
struct hash_map final : std::unordered_map<
  T, U, V, W,
  Allocator<std::pair<const T,U>>
> {
  hash_map()
    : std::unordered_map<
        T, U, V, W,
        Allocator<std::pair<const T,U>>
      >(0)
  {}

  using Base = std::unordered_map<
    T, U, V, W, Allocator<std::pair<const T, U>>
  >;
};

template <class T,
          class U,
          class V = std::hash<T>,
          class W = std::equal_to<T>>
struct hash_multimap final : std::unordered_multimap<
  T, U, V, W,
  Allocator<std::pair<const T,U>>
> {
  hash_multimap()
    : std::unordered_multimap<
        T, U, V, W,
        Allocator<std::pair<const T,U>>
      >(0)
  {}

  using Base = std::unordered_multimap<
    T, U, V, W, Allocator<std::pair<const T, U>>
  >;
};

template <class T,
          class V = std::hash<T>,
          class W = std::equal_to<T>>
struct hash_set : std::unordered_set<T,V,W,Allocator<T> > {
  hash_set()
    : std::unordered_set<T,V,W,Allocator<T>>(0)
  {}

  using Base = std::unordered_set<T,V,W,Allocator<T>>;
};

template <typename T>
struct FixedVector final : HPHP::FixedVector<T, Allocator<T>> {
  using Base = HPHP::FixedVector<T, Allocator<T>>;
  using Base::Base;
};

// Special allocator for TinyVector using request heap allocation.
template <typename T, typename Element = T> struct TinyVectorReqAllocator {
  template <typename U> struct rebind {
    using type = TinyVectorReqAllocator<U, Element>;
  };

  void* allocate(std::size_t size) const {
    return req::malloc(size);
  }
  void deallocate(void* ptr) const { req::free(ptr); }
  std::size_t usable_size(void* ptr, std::size_t size) const {
    return size;
  }
};

template<typename T,
         std::size_t Internal = 1,
         std::size_t MinHeap = 0>
struct TinyVector final : HPHP::TinyVector<T,
                                           Internal,
                                           MinHeap,
                                           TinyVectorReqAllocator<T>> {
  using Base = HPHP::TinyVector<T,Internal,MinHeap,TinyVectorReqAllocator<T>>;
  using Base::Base;
};

//////////////////////////////////////////////////////////////////////

}}

#endif
