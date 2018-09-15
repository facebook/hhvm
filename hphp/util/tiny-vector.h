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

#ifndef incl_HPHP_UTIL_TINYVECTOR_H_
#define incl_HPHP_UTIL_TINYVECTOR_H_

#include <algorithm>
#include <memory>
#include <type_traits>

#include <boost/iterator/iterator_facade.hpp>
#include <folly/portability/Malloc.h>

#include "hphp/util/alloc.h"
#include "hphp/util/assertions.h"
#include "hphp/util/compact-tagged-ptrs.h"


namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Simple, small buffer-optimized sequence of T.
 *
 * Notes:
 *
 *    - Unlike std::vector, storage is not guaranteed to be contiguous.
 *
 *    - Insertion and removal can only happen at the end of the
 *      sequence (although you can modify elements anywhere in the
 *      sequence).
 *
 *    - There is no non-const iterator support, and we have forward
 *      iterators only.
 *
 *    - The elements must be trivially copyable/assignable and have
 *      trivial destructors.
 *
 * Currently, does not invalidate pointers or references to elements
 * at indexes less than InternalSize, but we don't want to depend on
 * this without documenting it here.  Iterators should be considered
 * invalidated after any mutation.
 *
 * Also: if you have a T*, and the list almost always contains at most
 * one element, PointerList is a smaller object so may be preferable
 * in that case.  (Unless you want the first element to remain
 * accessible inline instead of moved to the heap.)
 */

// Allocator interface with an additional `usable_size()` API.
template <typename T>
struct TinyVectorMallocAllocator {
  using size_type = std::size_t;
  using value_type = T;
  template <typename U> struct rebind {
    using other = TinyVectorMallocAllocator<U>;
  };

  void* allocate(std::size_t size) const { return malloc(size); }
  void deallocate(void* ptr, size_t) const { free(ptr); }
  std::size_t usable_size(void* ptr, std::size_t /*size*/) const {
    return malloc_usable_size(ptr);
  }
};

template<class T,
         size_t InternalSize = 1,
         size_t MinHeapCapacity = 0,
         typename OrigAllocator = std::allocator<char>>
struct TinyVector {
  static_assert(std::is_trivially_destructible<T>::value,
                "TinyVector only supports elements with trivial destructors");
  static_assert(std::is_trivially_copy_constructible<T>::value &&
                std::is_trivially_copy_assignable<T>::value,
                "TinyVector only supports elements with trivial copy "
                "constructors and trivial assignment operators");
  static_assert(InternalSize >= 1,
                "TinyVector assumes that the internal size is at least 1");

  /*
   * We need to see if the allocator implements a member named `usable_size`.
   * If it does, we assume it is a method like
   *   size_type Alloc::usable_size(pointer, size_type);
   */
  template <typename Alloc> struct alloc_traits : std::allocator_traits<Alloc> {
    using typename std::allocator_traits<Alloc>::size_type;

   private:
    template <typename AT, decltype(&AT::usable_size)* = nullptr>
    struct valid_type;
    template <typename AT = Alloc>
    static std::true_type helper(valid_type<AT>*);
    template <typename AT = Alloc> static std::false_type helper(...);
    static constexpr bool has_usable_size =
      std::is_same<std::true_type, decltype(helper(nullptr))>::value;

    template<typename AT = Alloc> static size_type usable_size_impl(
      typename std::enable_if<alloc_traits<AT>::has_usable_size, AT>::type& a,
      void* ptr, size_type size) {
      return a.usable_size(ptr, size);
    }
    template<typename AT = Alloc> static size_type usable_size_impl(
      typename std::enable_if<!alloc_traits<AT>::has_usable_size, AT>::type&,
      void*, size_type size) {
      return size;
    }
   public:
    static size_type usable_size(Alloc& a, void* ptr, size_type size) {
      return usable_size_impl(a, ptr, size);
    }
  };

  using Allocator = typename OrigAllocator::template rebind<uint8_t>::other;
  using AT = alloc_traits<Allocator>;
  using AllocPtr = typename AT::pointer;

  struct const_iterator;

  TinyVector() {}
  ~TinyVector() { clear(); }

  TinyVector(const TinyVector&) = delete;
  TinyVector& operator=(const TinyVector&) = delete;

  size_t size() const { return m_impl.m_data.size(); }
  bool empty() const { return !size(); }

  const_iterator begin() const { return const_iterator(this, 0); }
  const_iterator end()   const { return const_iterator(this); }

  T& operator[](size_t index) {
    assert(index < size());
    return *location(index);
  }

  const T& operator[](size_t index) const {
    return const_cast<TinyVector*>(this)->operator[](index);
  }

  void clear() {
    if (HeapData* p = m_impl.m_data.ptr()) {
      alloc_traits<Impl>::deallocate(m_impl, reinterpret_cast<AllocPtr>(p),
                                     allocSize(p->capacity));
    }
    m_impl.m_data.set(0, 0);
  }

  void push_back(const T& t) {
    alloc_back() = t;
  }

  /*
   * Increase the size of this TinyVector by 1 and return a reference to the
   * new object, which will be uninitialized.
   */
  T& alloc_back() {
    size_t current = size();
    reserve(current + 1);
    m_impl.m_data.set(current + 1, m_impl.m_data.ptr());
    return back();
  }

  void pop_back() {
    assert(!empty());
    m_impl.m_data.set(size() - 1, m_impl.m_data.ptr());
  }

  T& back() {
    assert(!empty());
    return (*this)[size() - 1];
  }

  const T& back() const {
    assert(!empty());
    return (*this)[size() - 1];
  }

  T& front() {
    assert(!empty());
    return m_impl.m_vals[0];
  }

  const T& front() const {
    assert(!empty());
    return m_impl.m_vals[0];
  }

  void reserve(size_t sz) {
    if (sz < InternalSize) return;

    const size_t currentHeap =
      m_impl.m_data.ptr() ? m_impl.m_data.ptr()->capacity : 0;
    const size_t neededHeap = sz - InternalSize;
    if (neededHeap <= currentHeap) {
      return;
    }

    const size_t newCapacity = std::max(
      currentHeap ? currentHeap * 4 / 3
                  : std::max(neededHeap, MinHeapCapacity),
      neededHeap);
    const size_t requested = allocSize(newCapacity);
    auto newHeap = reinterpret_cast<HeapData*>(AT::allocate(m_impl, requested));
    const size_t usableSize = AT::usable_size(m_impl, newHeap, requested);
    newHeap->capacity = (usableSize - offsetof(HeapData, vals)) / sizeof(T);

    if (HeapData* p = m_impl.m_data.ptr()) {
      std::copy(&m_impl.m_data.ptr()->vals[0],
                &m_impl.m_data.ptr()->vals[size() - InternalSize],
                &newHeap->vals[0]);
      AT::deallocate(m_impl, reinterpret_cast<AllocPtr>(p),
                     allocSize(p->capacity));
    }
    m_impl.m_data.set(size(), newHeap);
  }

  template<size_t isize, size_t minheap, typename origalloc>
  bool operator==(const TinyVector<T, isize, minheap, origalloc>& tv) const {
    if (size() != tv.size()) return false;

    for (size_t i = 0; i < size(); ++i) {
      if ((*this)[i] != tv[i]) return false;
    }
    return true;
  }

private:
  struct HeapData {
    uint32_t capacity; // numbers of vals---excludes this capacity field
    T vals[0];
  };

  static constexpr std::size_t allocSize(uint32_t capacity) {
    return sizeof(HeapData) + sizeof(T) * capacity;
  }

  T* location(size_t index) {
    return index < InternalSize
                   ? &m_impl.m_vals[index]
                   : &m_impl.m_data.ptr()->vals[index - InternalSize];
  }

private:
  struct Impl : Allocator {
    CompactSizedPtr<HeapData> m_data;
    T m_vals[InternalSize];
  };
  Impl m_impl;
};

//////////////////////////////////////////////////////////////////////

template<class T,
         size_t InternalSize,
         size_t MinHeapCapacity,
         typename Allocator>
struct TinyVector<T,InternalSize,MinHeapCapacity,Allocator>::const_iterator
  : boost::iterator_facade<const_iterator,
                           const T,
                           boost::forward_traversal_tag>
{
  explicit const_iterator(const TinyVector* p, uint32_t idx)
    : m_p(p)
    , m_idx(idx)
  {}

  explicit const_iterator(const TinyVector* p)
    : m_p(p)
    , m_idx(m_p->size())
  {}

private:
  friend class boost::iterator_core_access;

  void increment() {
    ++m_idx;
  }

  bool equal(const const_iterator& o) const {
    assert(m_p == o.m_p);
    return m_idx == o.m_idx;
  }

  const T& dereference() const {
    return (*m_p)[m_idx];
  }

private:
  const TinyVector* m_p;
  uint32_t m_idx;
};

//////////////////////////////////////////////////////////////////////

}

#endif
