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

#pragma once

#include <algorithm>
#include <memory>
#include <type_traits>

#include <boost/iterator/iterator_facade.hpp>
#include <folly/lang/Launder.h>
#include <folly/portability/Malloc.h>

#include "hphp/util/alloc.h"
#include "hphp/util/assertions.h"
#include "hphp/util/compact-tagged-ptrs.h"

namespace HPHP {

// Replace with std::is_nothrow_swappable in c++17
namespace tiny_vector_detail {
using std::swap;

template <typename U>
struct is_nothrow_swappable {
  static constexpr bool value =
    noexcept(swap(std::declval<U&>(), std::declval<U&>()));
};

}

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
 *    - The elements must be nothrow move constructible and nothrow swappable.
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

  T* allocate(std::size_t size) const {
    return reinterpret_cast<T*>(malloc(size));
  }
  void deallocate(T* ptr, size_t) const { free(ptr); }
  std::size_t usable_size(T* ptr, std::size_t) const {
    return malloc_usable_size(ptr);
  }
};

template<typename T,
         size_t InternalSize = 1,
         size_t MinHeapCapacity = 0,
         typename OrigAllocator = TinyVectorMallocAllocator<T>>
struct TinyVector {
  static_assert(std::is_nothrow_move_constructible<T>::value,
                "TinyVector only supports elements with "
                "non-throwing move constructors");
  static_assert(tiny_vector_detail::is_nothrow_swappable<T>::value,
                "TinyVector only supports elements with non-throwing swaps");
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
  using value_type = T;

  struct iterator;
  struct const_iterator;

  TinyVector() = default;
  ~TinyVector() { clear(); }

  TinyVector(const TinyVector& o) {
    reserve(o.size());
    for (auto const& elem : o) push_back(elem);
  }
  TinyVector(TinyVector&& o) noexcept { swap(o); }

  TinyVector& operator=(const TinyVector& o) {
    if (this != &o) {
      auto temp = o;
      swap(temp);
    }
    return *this;
  }
  TinyVector& operator=(TinyVector&& o) noexcept {
    if (this != &o) swap(o);
    return *this;
  }

  template <typename U>
  TinyVector(std::initializer_list<U> il) {
    reserve(il.size());
    for (auto const& elem : il) push_back(elem);
  }

  size_t size() const {
    if (auto const p = m_impl.m_data.ptr()) {
      assertx(!m_impl.m_data.size());
      return p->size;
    }
    return m_impl.m_data.size();
  }
  bool empty() const { return !size(); }

  void swap(TinyVector& o) noexcept {
    auto const size1 = size();
    auto const size2 = o.size();
    auto const internal1 = std::min<size_t>(size1, InternalSize);
    auto const internal2 = std::min<size_t>(size2, InternalSize);

    // Swap the prefix of the inline storage which both have initialized.
    auto const both = std::min(internal1, internal2);
    for (size_t i = 0; i < both; ++i) {
      using std::swap;
      swap(*location(i), *o.location(i));
    }

    // Move data out of the vector with more initialized inline data into the
    // one with less. Then destroy the moved from data.
    if (internal1 > internal2) {
      std::uninitialized_move_n(
        location(internal2),
        internal1 - internal2,
        o.location(internal2)
      );
      if (!std::is_trivially_destructible<T>::value) {
        for (size_t i = internal2; i < internal1; ++i) location(i)->~T();
      }
    } else {
      std::uninitialized_move_n(
        o.location(internal1),
        internal2 - internal1,
        location(internal1)
      );
      if (!std::is_trivially_destructible<T>::value) {
        for (size_t i = internal1; i < internal2; ++i) o.location(i)->~T();
      }
    }

    // Move the non-inline data. This is just a simple pointer swap.
    using std::swap;
    swap(m_impl.m_data, o.m_impl.m_data);
  }

  iterator begin() { return iterator(this, 0); }
  iterator end() { return iterator(this); }

  const_iterator begin() const { return const_iterator(this, 0); }
  const_iterator end()   const { return const_iterator(this); }

  T& operator[](size_t index) {
    assertx(index < size());
    return *location(index);
  }

  const T& operator[](size_t index) const {
    return const_cast<TinyVector*>(this)->operator[](index);
  }

  void clear() {
    if (!std::is_trivially_destructible<T>::value) {
      auto const current = size();
      for (size_t i = 0; i < current; ++i) location(i)->~T();
    }

    if (auto p = m_impl.m_data.ptr()) {
      alloc_traits<Impl>::deallocate(m_impl, reinterpret_cast<AllocPtr>(p),
                                     allocSize(p->capacity));
    }
    m_impl.m_data.set(0, nullptr);
  }

  template <typename U>
  T& push_back(const U& u) {
    auto const current = size();
    reserve(current + 1);
    auto ptr = location(current);
    ::new (ptr) T(u);
    setSize(current + 1);
    return *ptr;
  }

  template <typename U>
  T& push_back(U&& u) {
    auto const current = size();
    reserve(current + 1);
    auto ptr = location(current);
    ::new (ptr) T(std::move(u));
    setSize(current + 1);
    return *ptr;
  }

  template <typename... Args>
  T& emplace_back(Args&&... args) {
    auto const current = size();
    reserve(current + 1);
    auto ptr = location(current);
    ::new (ptr) T(std::forward<Args>(args)...);
    setSize(current + 1);
    return *ptr;
  }

  template <typename I>
  void insert(I begin, I end) {
    while (begin != end) emplace_back(*begin++);
  }

  void pop_back() {
    assertx(!empty());
    location(size() - 1)->~T();
    setSize(size() - 1);
  }

  T& back() {
    assertx(!empty());
    return (*this)[size() - 1];
  }

  const T& back() const {
    assertx(!empty());
    return (*this)[size() - 1];
  }

  T& front() {
    assertx(!empty());
    return (*this)[0];
  }

  const T& front() const {
    assertx(!empty());
    return (*this)[0];
  }

  void resize(size_t sz) {
    while (sz > size()) emplace_back();
    while (sz < size()) pop_back();
  }

  void reserve(size_t sz) {
    if (sz <= InternalSize) return;

    const size_t currentHeap =
      m_impl.m_data.ptr() ? m_impl.m_data.ptr()->capacity : 0;
    const size_t neededHeap = sz - InternalSize;
    if (neededHeap <= currentHeap) return;

    const size_t newCapacity = std::max(
      currentHeap ? currentHeap * 4 / 3
                  : std::max(neededHeap, MinHeapCapacity),
      neededHeap);
    const size_t requested = allocSize(newCapacity);
    auto newHeap = reinterpret_cast<HeapData*>(AT::allocate(m_impl, requested));
    const size_t usableSize = AT::usable_size(m_impl, newHeap, requested);
    newHeap->capacity = (usableSize - offsetof(HeapData, vals)) / sizeof(T);
    newHeap->size = size();

    if (auto p = m_impl.m_data.ptr()) {
      assertx(!m_impl.m_data.size());
      auto const current = newHeap->size;
      std::uninitialized_move_n(
        &m_impl.m_data.ptr()->vals[0],
        current - InternalSize,
        &newHeap->vals[0]
      );
      if (!std::is_trivially_destructible<T>::value) {
        for (size_t i = InternalSize; i < current; ++i) location(i)->~T();
      }
      AT::deallocate(
        m_impl,
        reinterpret_cast<AllocPtr>(p),
        allocSize(p->capacity)
      );
    }
    m_impl.m_data.set(0, newHeap);
  }

  template<size_t isize, size_t minheap, typename origalloc>
  bool operator==(const TinyVector<T, isize, minheap, origalloc>& tv) const {
    if (size() != tv.size()) return false;
    return std::equal(begin(), end(), tv.begin());
  }
  template<size_t isize, size_t minheap, typename origalloc>
  bool operator!=(const TinyVector<T, isize, minheap, origalloc>& tv) const {
    return !(*this == tv);
  }

  template<size_t isize, size_t minheap, typename origalloc>
  bool operator<(const TinyVector<T, isize, minheap, origalloc>& tv) const {
    return std::lexicographical_compare(begin(), end(), tv.begin(), tv.end());
  }
  template<size_t isize, size_t minheap, typename origalloc>
  bool operator<=(const TinyVector<T, isize, minheap, origalloc>& tv) const {
    return (*this < tv) || (*this == tv);
  }
  template<size_t isize, size_t minheap, typename origalloc>
  bool operator>(const TinyVector<T, isize, minheap, origalloc>& tv) const {
    return !(*this <= tv);
  }
  template<size_t isize, size_t minheap, typename origalloc>
  bool operator>=(const TinyVector<T, isize, minheap, origalloc>& tv) const {
    return !(*this < tv);
  }

private:
  struct HeapData {
    uint32_t size; // complete size, including InternalSize
    uint32_t capacity; // numbers of vals---excludes this capacity field
    T vals[0];
  };

  static constexpr std::size_t allocSize(uint32_t capacity) {
    return sizeof(HeapData) + sizeof(T) * capacity;
  }

  void setSize(size_t s) {
    if (auto p = m_impl.m_data.ptr()) {
      assertx(!m_impl.m_data.size());
      assertx(s <= p->capacity + InternalSize);
      p->size = s;
    } else {
      assertx(s <= InternalSize);
      m_impl.m_data.set(s, m_impl.m_data.ptr());
    }
  }

  T* location(size_t index) {
    return (index < InternalSize)
      ? folly::launder(reinterpret_cast<T*>(&m_impl.m_vals[index]))
      : &m_impl.m_data.ptr()->vals[index - InternalSize];
  }

  using RawBuffer = typename std::aligned_storage<sizeof(T), alignof(T)>::type;
  using InternalStorage = RawBuffer[InternalSize];

  struct Impl : Allocator {
    // If m_data.ptr() is null, the size of m_data.size(). Otherwise
    // the size is stored within HeapData.
    CompactSizedPtr<HeapData> m_data;
    InternalStorage m_vals;
  };
  Impl m_impl;

  static_assert(InternalSize <= CompactSizedPtr<HeapData>::kMaxSize,
                "TinyVector does not support such a large internal size");
};

//////////////////////////////////////////////////////////////////////

template<typename T,
         size_t InternalSize,
         size_t MinHeapCapacity,
         typename Allocator>
struct TinyVector<T,InternalSize,MinHeapCapacity,Allocator>::iterator
  : boost::iterator_facade<iterator,
                           T,
                           boost::random_access_traversal_tag>
{
  explicit iterator(TinyVector* p, uint32_t idx)
    : m_p(p)
    , m_idx(idx)
  {}

  explicit iterator(TinyVector* p)
    : m_p(p)
    , m_idx(m_p->size())
  {}

private:
  friend class boost::iterator_core_access;

  using size_type = uint32_t;

  void increment() {
    ++m_idx;
  }

  void decrement() {
    --m_idx;
  }

  void advance(int n) {
    m_idx += n;
  }

  int distance_to(const iterator& o) const {
    return o.m_idx - m_idx;
  }

  bool equal(const iterator& o) const {
    assertx(m_p == o.m_p);
    return m_idx == o.m_idx;
  }

  T& dereference() const {
    return (*m_p)[m_idx];
  }

private:
  TinyVector* m_p;
  size_type m_idx;
};

template<typename T,
         size_t InternalSize,
         size_t MinHeapCapacity,
         typename Allocator>
struct TinyVector<T,InternalSize,MinHeapCapacity,Allocator>::const_iterator
  : boost::iterator_facade<const_iterator,
                           const T,
                           boost::random_access_traversal_tag>
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

  using size_type = uint32_t;

  void increment() {
    ++m_idx;
  }

  void decrement() {
    --m_idx;
  }

  void advance(int n) {
    m_idx += n;
  }

  int distance_to(const const_iterator& o) const {
    return o.m_idx - m_idx;
  }

  bool equal(const const_iterator& o) const {
    assertx(m_p == o.m_p);
    return m_idx == o.m_idx;
  }

  const T& dereference() const {
    return (*m_p)[m_idx];
  }

private:
  const TinyVector* m_p;
  size_type m_idx;
};

//////////////////////////////////////////////////////////////////////

template<typename T,
         size_t InternalSize,
         size_t MinHeapCapacity,
         typename Allocator>
void swap(TinyVector<T, InternalSize, MinHeapCapacity, Allocator>& v1,
          TinyVector<T, InternalSize, MinHeapCapacity, Allocator>& v2)
  noexcept {
  v1.swap(v2);
}

//////////////////////////////////////////////////////////////////////

}
