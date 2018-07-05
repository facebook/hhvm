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
#ifndef incl_HPHP_RUNTIME_BASE_REQ_MEMORY_H_
#define incl_HPHP_RUNTIME_BASE_REQ_MEMORY_H_

#include "hphp/runtime/base/req-malloc.h"
#include "hphp/util/type-scan.h"
#include <folly/Memory.h>
#include <memory>

namespace HPHP { namespace req {

//////////////////////////////////////////////////////////////////////

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
struct unique_ptr final
    : std::unique_ptr<T, folly::allocator_delete<Allocator<T>>> {
  using Base = std::unique_ptr<T, folly::allocator_delete<Allocator<T>>>;
  template <class... Args>
  /* implicit */ unique_ptr(Args&&... args)
      : Base(std::forward<Args>(args)...) {}
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
  ConservativeAllocator<T> alloc;
  std::shared_ptr<T> r = std::allocate_shared<T>(alloc,
      std::forward<Args>(args)...);
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

}}

#endif
