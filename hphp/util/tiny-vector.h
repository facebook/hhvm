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

#ifndef incl_HPHP_UTIL_TINYVECTOR_H_
#define incl_HPHP_UTIL_TINYVECTOR_H_

#include <stdlib.h>
#include <boost/type_traits/has_trivial_destructor.hpp>
#include <boost/type_traits/has_trivial_copy.hpp>
#include <boost/type_traits/has_trivial_assign.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <algorithm>
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

// Allocator interface to control how TinyVector allocates memory. It would be
// nice if it could use the standard allocator interface. However, it expects to
// allocate raw memory of N bytes, while the standard allocator interface
// allocates N instances of type T.
template <typename T> struct TinyVectorMallocAllocator {
  template <typename U> struct rebind {
    using type = TinyVectorMallocAllocator<U>;
  };

  void* allocate(std::size_t size) const { return malloc(size); }
  void deallocate(void* ptr) const { free(ptr); }
  std::size_t usable_size(void* ptr, std::size_t size) const {
    return malloc_usable_size(ptr);
  }
};

template<class T,
         size_t InternalSize = 1,
         size_t MinHeapCapacity = 0,
         typename OrigAllocator = TinyVectorMallocAllocator<T>>
struct TinyVector {
  struct const_iterator;

#ifndef __INTEL_COMPILER
  static_assert(boost::has_trivial_destructor<T>::value,
                "TinyVector only supports elements with trivial destructors");
  static_assert(boost::has_trivial_copy<T>::value &&
                boost::has_trivial_assign<T>::value,
                "TinyVector only supports elements with trivial copy "
                "constructors and trivial assignment operators");
#endif
  static_assert(InternalSize >= 1,
                "TinyVector assumes that the internal size is at least 1");

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
      m_impl.deallocate(p);
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
    const size_t requested = sizeof(HeapData) + sizeof(T) * newCapacity;
    HeapData* newHeap = static_cast<HeapData*>(m_impl.allocate(requested));
    newHeap->capacity = (m_impl.usable_size(newHeap, requested) -
                         offsetof(HeapData, vals)) / sizeof(T);

    std::copy(&m_impl.m_data.ptr()->vals[0],
              &m_impl.m_data.ptr()->vals[size() - InternalSize],
              &newHeap->vals[0]);
    m_impl.deallocate(m_impl.m_data.ptr());
    m_impl.m_data.set(size(), newHeap);
  }

private:
  struct HeapData {
    uint32_t capacity; // numbers of vals---excludes this capacity field
    T vals[0];
  };

  T* location(size_t index) {
    return index < InternalSize ?
                   &m_impl.m_vals[index]
                   : &m_impl.m_data.ptr()->vals[index - InternalSize];
  }

private:
  using Allocator = typename OrigAllocator::template rebind<HeapData>::type;
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
