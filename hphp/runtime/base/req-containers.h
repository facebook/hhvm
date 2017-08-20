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
#include <folly/Optional.h>

#include "hphp/runtime/base/req-malloc.h"

#include "hphp/util/fixed-vector.h"
#include "hphp/util/tiny-vector.h"
#include "hphp/util/type-scan.h"
#include "hphp/util/hash-map-typedefs.h"

namespace HPHP { namespace req {

//////////////////////////////////////////////////////////////////////

/*
 * Defines a family of types similar to std:: collections and pointers, except
 * using the request-local allocator and with GC type-scanner annotations.
 *
 * Replace std:: with req:: if you know the data is request-local.
 */

/*
 * Most of these replacements purposefully derive from standard types and
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
  template <class... Args>
  /* implicit */ unique_ptr(Args&&... args) :
    Base(std::forward<Args>(args)...) {}
  // Unlike the rest, we don't want to ignore the base. Its easy to type-scan
  // unique_ptr and more efficient to let it do it itself.
  TYPE_SCAN_SILENCE_FORBIDDEN_BASES(Base);
};

template<typename T, class... Args>
unique_ptr<T> make_unique(Args&&... args) {
  return folly::allocate_unique<T>(
    Allocator<T>(),
    std::forward<Args>(args)...
  );
}

template <typename T> struct weak_ptr;

/*
 * req::shared_ptr does not derive from std::shared_ptr.
 * 1) They have different scopes (req::shared_ptr is per-request)
 * 2) Guaranteeing req::shared_ptr's can only be instantiated via make_shared
 *    makes type scanning easier.
 */

template <typename T>
struct shared_ptr final  {
  shared_ptr() = default;
  explicit shared_ptr(std::nullptr_t) :
    m_std_ptr(nullptr) {
  }
  template<class Y>
  shared_ptr( const shared_ptr<Y>& r )
    : m_std_ptr(r.m_std_ptr),
      m_scan_size(r.m_scan_size),
      m_scan_index(r.m_scan_index) {
  }
  template <class Y>
  shared_ptr(shared_ptr<Y>&& r)
    : m_std_ptr(std::move(r.m_std_ptr)),
      m_scan_size(std::move(r.m_scan_size)),
      m_scan_index(std::move(r.m_scan_index)) {
  }
  template<class Y>
  explicit shared_ptr(const weak_ptr<Y>& r)
    : m_std_ptr(r),
      m_scan_size(r.m_scan_size),
      m_scan_index(r.m_scan_index) {
  }

  TYPE_SCAN_CUSTOM(T) {
    if (this->get() != nullptr) {
      scanner.scanByIndex(m_scan_index, this->get(), m_scan_size);
    }
  }

  T* get() const {
    return m_std_ptr.get();
  }
  T& operator*() const {
    return *get();
  }
  T* operator->() const {
    return get();
  }
  explicit operator bool() const {
    return get() != nullptr;
  }
  template <class U>
  bool operator==(const shared_ptr<U>& rhs) {
    return m_std_ptr == rhs.m_std_ptr;
  }
  template<class Y>
  shared_ptr& operator=(const shared_ptr<Y>& r) {
    m_std_ptr = r.m_std_ptr;
    m_scan_size = r.m_scan_size;
    m_scan_index = r.m_scan_index;
    return *this;
  }
  template<class Y>
  shared_ptr& operator=(shared_ptr<Y>&& r) {
    m_std_ptr = std::move(r.m_std_ptr);
    m_scan_size = std::move(r.m_scan_size);
    m_scan_index = std::move(r.m_scan_index);
    return *this;
  }
  void reset() {
    m_std_ptr.reset();
    m_scan_size = 0;
    m_scan_index = type_scan::kIndexUnknownNoPtrs;
  }
  void swap(shared_ptr& r)  {
    m_std_ptr.swap(r.m_std_ptr);
    std::swap(m_scan_index, r.m_scan_index);
    std::swap(m_scan_size, r.m_scan_size);
  }
  void swap(T& t)  {
    m_std_ptr.swap(t);
    m_scan_size = sizeof(t);
    m_scan_index = type_scan::getIndexForScan<T>();
  }

  template<class U, class... Args>
  friend shared_ptr<U> make_shared(Args&&... args);
  friend struct weak_ptr<T>;

  private:
    std::shared_ptr<T> m_std_ptr;
    std::size_t m_scan_size = 0;
    type_scan::Index m_scan_index = type_scan::kIndexUnknownNoPtrs;
    template<class Y>
    shared_ptr(const std::shared_ptr<Y>& r, std::size_t size,
        type_scan::Index index)
      : m_std_ptr(r), m_scan_size(size), m_scan_index(index) {
    }
};

template<class T, class... Args>
shared_ptr<T> make_shared(Args&&... args) {
  std::shared_ptr<T> r = std::make_shared<T>(std::forward<Args>(args)...);
  return shared_ptr<T>(r, sizeof(*r), type_scan::getIndexForScan<T>());
}

template <class T>
void swap(shared_ptr<T>& lhs, shared_ptr<T>& rhs) {
  lhs.swap(rhs);
}

template <typename T>
struct weak_ptr final : private std::weak_ptr<T> {
  using Base = std::weak_ptr<T>;
  template <class Y>
    explicit weak_ptr (const shared_ptr<Y>& r) :
    Base(r.m_std_ptr),
    m_scan_size(r.m_scan_size),
    m_scan_index(r.m_scan_index) {
  }

  shared_ptr<T> lock() const {
    std::shared_ptr<T> r = Base::lock();
    return shared_ptr<T>(r, sizeof(*r), type_scan::getIndexForScan<T>());
  }
  TYPE_SCAN_IGNORE_BASES(Base);
  TYPE_SCAN_IGNORE_ALL;

  friend struct shared_ptr<T>;
  private:
    std::size_t m_scan_size = 0;
    type_scan::Index m_scan_index = type_scan::kIndexUnknownNoPtrs;
};

#ifndef __APPLE__ // XXX: this affects codegen quality but not correctness
static_assert(
  sizeof(unique_ptr<int>) == sizeof(std::unique_ptr<int>),
  "req::unique_ptr pointer should not be larger than std::unique_ptr"
);
#endif

/*
 * Container replacements.
 *
 * For all the containers, instruct the type-scanning machinery to ignore the
 * base class (the actual container). This is because the container internals
 * typically use something like std::aligned_storage<> for the actual value, so
 * the scanner will not find it on its own. Instead, ignore the base, and
 * provide a custom scanner which uses the public interface to get at the
 * values. The actual container nodes are allocated with a conservative scan
 * action, so if the container lies on the stack or anywhere else we're
 * conservative scanning, we'll conservative scan that as well.
 */

template <typename Key,
          typename T,
          typename Compare = std::less<Key>>
struct map final : std::map<Key, T, Compare,
                            ConservativeAllocator<std::pair<const Key,T>>
                            > {
  using Base = std::map<Key, T, Compare,
                        ConservativeAllocator<std::pair<const Key, T>>>;
  using Base::Base;
  TYPE_SCAN_IGNORE_BASES(Base);
  TYPE_SCAN_CUSTOM(Key, T) {
    for (const auto& pair : *this) scanner.scan(pair);
  }
};

template <typename Key,
          typename T,
          typename Compare = std::less<Key>>
struct multimap final : std::multimap<Key, T, Compare,
                                      ConservativeAllocator<
                                        std::pair<const Key,T>>> {
  using Base = std::multimap<Key, T, Compare,
                             ConservativeAllocator<std::pair<const Key, T>>>;
  using Base::Base;
  TYPE_SCAN_IGNORE_BASES(Base);
  TYPE_SCAN_CUSTOM(Key, T) {
    for (const auto& pair : *this) scanner.scan(pair);
  }
};

template <typename T, typename Compare = std::less<T>>
struct set final : std::set<T, Compare, ConservativeAllocator<T>> {
  using Base = std::set<T, Compare, ConservativeAllocator<T>>;
  using Base::Base;
  TYPE_SCAN_IGNORE_BASES(Base);
  TYPE_SCAN_CUSTOM(T) {
    for (const auto& v : *this) scanner.scan(v);
  }
};

template <typename T, typename Compare = std::less<T>>
struct multiset final : std::multiset<T, Compare, ConservativeAllocator<T>> {
  using Base = std::multiset<T, Compare, ConservativeAllocator<T>>;
  using Base::Base;
  TYPE_SCAN_IGNORE_BASES(Base);
  TYPE_SCAN_CUSTOM(T) {
    for (const auto& v : *this) scanner.scan(v);
  }
};

template <typename T>
struct deque final : std::deque<T, ConservativeAllocator<T>> {
  using Base = std::deque<T, ConservativeAllocator<T>>;
  using Base::Base;
  TYPE_SCAN_IGNORE_BASES(Base);
  TYPE_SCAN_CUSTOM(T) {
    for (const auto& v : *this) scanner.scan(v);
  }
};

template <typename T>
struct vector final : std::vector<T, ConservativeAllocator<T>> {
  using Base = std::vector<T, ConservativeAllocator<T>>;
  using Base::Base;
  TYPE_SCAN_IGNORE_BASES(Base);
  TYPE_SCAN_CUSTOM(T) {
    for (const auto& v : *this) scanner.scan(v);
  }
};

template <typename T>
struct list final : std::list<T, ConservativeAllocator<T>> {
  using Base = std::list<T, ConservativeAllocator<T>>;
  using Base::Base;
  TYPE_SCAN_IGNORE_BASES(Base);
  TYPE_SCAN_CUSTOM(T) {
    for (const auto& v : *this) scanner.scan(v);
  }
};

template <typename T>
struct forward_list final : std::forward_list<T, ConservativeAllocator<T>> {
  using Base = std::forward_list<T, ConservativeAllocator<T>>;
  using Base::Base;
  TYPE_SCAN_IGNORE_BASES(Base);
  TYPE_SCAN_CUSTOM(T) {
    for (const auto& v : *this) scanner.scan(v);
  }
};

template <typename T, typename Container = req::deque<T>>
using stack = std::stack<T, Container>;

template <typename T, typename Container = req::deque<T>>
using queue = std::queue<T, Container>;

template <typename T,
          typename Container = req::vector<T>,
          typename Compare = std::less<T>>
using priority_queue = std::priority_queue<T, Container, Compare>;

template<typename K, typename V, typename Pred = std::less<K>>
struct flat_map final : boost::container::flat_map<
  K, V, Pred, ConservativeAllocator<std::pair<K,V>>
> {
  using Base =
    boost::container::flat_map<K, V, Pred,
                               ConservativeAllocator<std::pair<K,V>>>;
  using Base::Base;
  TYPE_SCAN_IGNORE_BASES(Base);
  TYPE_SCAN_CUSTOM(K, V) {
    for (const auto& pair : *this) scanner.scan(pair);
  }
};

template<typename K, typename V, typename Pred = std::less<K>>
struct flat_multimap final : boost::container::flat_multimap<
  K, V, Pred, ConservativeAllocator<std::pair<K,V>>
> {
  using Base = boost::container::flat_multimap<
    K, V, Pred, ConservativeAllocator<std::pair<K,V>>
  >;
  using Base::Base;
  TYPE_SCAN_IGNORE_BASES(Base);
  TYPE_SCAN_CUSTOM(K, V) {
    for (const auto& pair : *this) scanner.scan(pair);
  }
};

template<typename K, typename Compare = std::less<K>>
struct flat_set final : boost::container::flat_set<K, Compare,
                                                   ConservativeAllocator<K>> {
  using Base = boost::container::flat_set<K, Compare, ConservativeAllocator<K>>;
  using Base::Base;
  TYPE_SCAN_IGNORE_BASES(Base);
  TYPE_SCAN_CUSTOM(K) {
    for (const auto& v : *this) scanner.scan(v);
  }
};

template<typename K, typename Compare = std::less<K>>
struct flat_multiset final : boost::container::flat_multiset<
  K, Compare, ConservativeAllocator<K>
> {
  using Base =
    boost::container::flat_multiset<K, Compare, ConservativeAllocator<K>>;
  using Base::Base;
  TYPE_SCAN_IGNORE_BASES(Base);
  TYPE_SCAN_CUSTOM(K) {
    for (const auto& v : *this) scanner.scan(v);
  }
};

template <class T,
          class U,
          class V = std::hash<T>,
          class W = std::equal_to<T>>
struct hash_map final : std::unordered_map<
  T, U, V, W,
  ConservativeAllocator<std::pair<const T,U>>
> {
  hash_map()
    : std::unordered_map<T, U, V, W,
      ConservativeAllocator<std::pair<const T,U>>>
      GOOD_UNORDERED_CTOR
  {}

  using Base = std::unordered_map<
    T, U, V, W, ConservativeAllocator<std::pair<const T, U>>
  >;

  TYPE_SCAN_IGNORE_BASES(Base);
  TYPE_SCAN_CUSTOM(T, U) {
   for (const auto& pair : *this) scanner.scan(pair);
  }
};

template <class T,
          class U,
          class V = std::hash<T>,
          class W = std::equal_to<T>>
struct hash_multimap final : std::unordered_multimap<
  T, U, V, W,
  ConservativeAllocator<std::pair<const T,U>>
> {
  hash_multimap()
    : std::unordered_multimap<T, U, V, W,
      ConservativeAllocator<std::pair<const T,U>>>
      GOOD_UNORDERED_CTOR
  {}

  using Base = std::unordered_multimap<
    T, U, V, W, ConservativeAllocator<std::pair<const T, U>>
  >;

  TYPE_SCAN_IGNORE_BASES(Base);
  TYPE_SCAN_CUSTOM(T, U) {
   for (const auto& pair : *this) scanner.scan(pair);
  }
};

template <class T,
          class V = std::hash<T>,
          class W = std::equal_to<T>>
struct hash_set final : std::unordered_set<T,V,W,ConservativeAllocator<T> > {
  hash_set()
    : std::unordered_set<T,V,W,ConservativeAllocator<T>>
      GOOD_UNORDERED_CTOR
  {}

  using Base = std::unordered_set<T,V,W,ConservativeAllocator<T>>;

  TYPE_SCAN_IGNORE_BASES(Base);
  TYPE_SCAN_CUSTOM(T) {
   for (const auto& v : *this) scanner.scan(v);
  }
};

template <typename T>
struct FixedVector final : HPHP::FixedVector<T, ConservativeAllocator<T>> {
  using Base = HPHP::FixedVector<T, ConservativeAllocator<T>>;
  using Base::Base;
  TYPE_SCAN_IGNORE_BASES(Base);
  TYPE_SCAN_CUSTOM(T) {
    for (const auto& v : *this) scanner.scan(v);
  }
};

// Special allocator for TinyVector using request heap allocation.
template <typename T, typename Element = T> struct TinyVectorReqAllocator {
  template <typename U> struct rebind {
    using type = TinyVectorReqAllocator<U, Element>;
  };

  void* allocate(std::size_t size) const {
    return req::malloc(
      size,
      type_scan::getIndexForMalloc<
        T, type_scan::Action::Conservative<Element>
      >()
    );
  }
  void deallocate(void* ptr) const { req::free(ptr); }
  std::size_t usable_size(void* /*ptr*/, std::size_t size) const {
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
  TYPE_SCAN_IGNORE_BASES(Base);
  TYPE_SCAN_CUSTOM(T) {
    for (const auto& v : *this) scanner.scan(v);
  }
};

//////////////////////////////////////////////////////////////////////

/*
 * Like folly::Optional, but exactly scans T
 */
template<typename T>
struct Optional: folly::Optional<T> {
  using Base = folly::Optional<T>;
  TYPE_SCAN_IGNORE_BASES(Base);
  TYPE_SCAN_CUSTOM(T) {
    if (this->hasValue()) scanner.scan(*this->get_pointer());
  }
};

}}

#endif
