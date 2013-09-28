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

#ifndef incl_HPHP_UTIL_TINYVECTOR_H_
#define incl_HPHP_UTIL_TINYVECTOR_H_

#include <stdlib.h>
#include <boost/noncopyable.hpp>
#include <boost/type_traits/has_trivial_destructor.hpp>
#include <boost/type_traits/has_trivial_copy.hpp>
#include <boost/type_traits/has_trivial_assign.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <algorithm>
#include "hphp/util/alloc.h"
#include "hphp/util/assertions.h"
#include "hphp/util/compact-sized-ptr.h"

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
template<class T, size_t InternalSize = 1, size_t MinHeapCapacity = 0>
struct TinyVector : private boost::noncopyable {
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

  ~TinyVector() { clear(); }

  size_t size() const { return m_data.size(); }
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
    if (HeapData* p = m_data.ptr()) {
      free(p);
    }
    m_data.set(0, 0);
  }

  void push_back(const T& t) {
    size_t current = size();
    reserve(current + 1);
    *location(current) = t;
    m_data.set(current + 1, m_data.ptr());
  }

  void pop_back() {
    assert(!empty());
    m_data.set(size() - 1, m_data.ptr());
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
    return m_vals[0];
  }

  const T& front() const {
    assert(!empty());
    return m_vals[0];
  }

  void reserve(size_t sz) {
    if (sz < InternalSize) return;

    const size_t currentHeap = m_data.ptr() ? m_data.ptr()->capacity : 0;
    const size_t neededHeap = sz - InternalSize;
    if (neededHeap <= currentHeap) {
      return;
    }

    const size_t newCapacity = std::max(
      currentHeap ? currentHeap * 4 / 3
                  : std::max(neededHeap, MinHeapCapacity),
      neededHeap);
    HeapData* newHeap = static_cast<HeapData*>(
      malloc(sizeof(HeapData) + sizeof(T) * newCapacity));
    newHeap->capacity = (malloc_usable_size(newHeap) -
                         offsetof(HeapData, vals)) / sizeof(T);

    std::copy(&m_data.ptr()->vals[0],
              &m_data.ptr()->vals[size() - InternalSize],
              &newHeap->vals[0]);
    free(m_data.ptr());
    m_data.set(size(), newHeap);
  }

private:
  struct HeapData {
    uint32_t capacity; // numbers of vals---excludes this capacity field
    T vals[0];
  };

  T* location(size_t index) {
    return index < InternalSize ? &m_vals[index]
                                : &m_data.ptr()->vals[index - InternalSize];
  }

private:
  CompactSizedPtr<HeapData> m_data;
  T m_vals[InternalSize];
};

//////////////////////////////////////////////////////////////////////

template<class T, size_t InternalSize, size_t MinHeapCapacity>
struct TinyVector<T,InternalSize,MinHeapCapacity>::const_iterator
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
