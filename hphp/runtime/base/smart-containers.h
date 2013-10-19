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

#include <deque>
#include <queue>
#include <map>
#include <stack>
#include <vector>
#include <tr1/unordered_map>
#include <cstdlib>
#include <memory>

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
 * Shorthand to create a std::unique_ptr to a smart-allocated object.
 *
 * Usage:
 *
 *   auto ptr = smart::make_unique<Foo>(arg1, arg2);
 *
 * If you need to make a typedef to one, since we don't have type
 * aliases yet in our version of gcc you have to do:
 *
 *   typedef smart::unique_ptr<T>::type type;
 */

template<class T> struct unique_ptr
  : folly::AllocatorUniquePtr<T,Allocator<T>>
{};

template<class T, class... Args>
typename unique_ptr<T>::type make_unique(Args&&... args) {
  return folly::allocate_unique<T>(
    Allocator<T>(),
    std::forward<Args>(args)...
  );
}

#ifndef __APPLE__ // XXX: this affects codegen quality but not correctness
static_assert(
  sizeof(unique_ptr<int>::type) == sizeof(std::unique_ptr<int>),
  "smart::unique_ptr pointer should not be larger than std::unique_ptr"
);
#endif

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

template <class Key, class T, class Compare = std::less<Key>>
class map : public std::map<
  Key, T, Compare,
  Allocator<std::pair<const Key,T>>
> {};

template <class T, class Compare = std::less<T>>
class set : public std::set<
  T, Compare, Allocator<T>
> {};

template <class T>
class deque : public std::deque<T,Allocator<T>> {};

template <class T>
class vector : public std::vector<T,Allocator<T>> {
  typedef std::vector<T,Allocator<T>> Base;
public:
  template<class... A>
  explicit vector(A&&... args) : Base(std::forward<A>(args)...) {}
};

template <class T, class Container = deque<T>>
class stack : public std::stack<T, Container> {};

template <class T>
class list : public std::list<T,Allocator<T>> {
  typedef std::list<T,Allocator<T>> Base;
public:
  template<class... A>
  explicit list(A&&... args) : Base(std::forward<A>(args)...) {}
};

template <class T>
class queue : public std::queue<T, deque<T> > {};

template <class T,
          class U,
          class V = hphp_hash<T>,
          class W = std::equal_to<T>>
struct hash_map : std::tr1::unordered_map<
  T, U, V, W,
  Allocator<std::pair<T,U>>
> {
  hash_map()
    : std::tr1::unordered_map<
        T, U, V, W,
        Allocator<std::pair<T,U>>
      >(0)
  {}
};

template <class T,
          class V = hphp_hash<T>,
          class W = std::equal_to<T>>
struct hash_set : std::tr1::unordered_set<T,V,W,Allocator<T> > {
  hash_set()
    : std::tr1::unordered_set<T,V,W,Allocator<T>>(0)
  {}
};

template<class K,
         class V,
         class Pred = std::less<K>>
struct flat_map
  : boost::container::flat_map<K,V,Pred,Allocator<std::pair<K,V>>>
{};

template<class K, class Pred = std::less<K>>
struct flat_set
  : boost::container::flat_set<K, Pred, Allocator<K>>
{};

template<class K,
         class V,
         class Pred = std::less<K>>
struct flat_multimap
  : boost::container::flat_multimap<K,V,Pred,Allocator<std::pair<K,V>>>
{};

//////////////////////////////////////////////////////////////////////

}}

#endif
