/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef PRIORITY_QUEUE_INCLUDED
#define PRIORITY_QUEUE_INCLUDED

/**
  @file include/priority_queue.h
*/

#include <functional>
#include <new>
#include <utility>
#include <vector>

#include "my_compiler.h"
#include "my_dbug.h"
#include "template_utils.h"

#if defined(EXTRA_CODE_FOR_UNIT_TESTING)
#include <iostream>
#include <sstream>
#endif

namespace priority_queue_unittest {
class PriorityQueueTest;
}  // namespace priority_queue_unittest

/**
  Implements a priority queue using a vector-based max-heap.

  A priority queue is a container specifically designed such that its first
  element is always the greatest of the elements it contains, according to
  some strict weak ordering criterion.

  For object locality, the implementation is vector-based, rather than
  node-based.

  The priority queue is mutable, which means that the priority of an element
  can be changed. See increase/decrease/update member functions.
  The typical use case is to change the value/priority of the root node.

  We provide iterators, which can be used to visit all elements.
  Iterators do not visit queue elements in priority order.
  Iterators should not be used to change the priority of elements.

  The underlying container must be
  constructible from an iterator range, should provide const and
  non-const random access iterators to access its elements, as well as
  the following operations:
  - size()
  - empty()
  - push_back()
  - pop_back()
  - swap()
  - clear()
  - capacity()
  - reserve()
  - max_size()

  @tparam T         Type of the elements of the priority queue.
  @tparam Container Type of the underlying container object where elements
                    are stored. Its value_type shall be T.
  @tparam Less      A binary predicate that takes two elements (of type T)
                    and returns a bool. The expression less(a,b), where
                    less is an object of this type and a and b are elements
                    in the container, shall return true if a is considered
                    to go before b in the strict weak ordering the
                    function defines.
 */
template <typename T, typename Container = std::vector<T>,
          typename Less = std::less<typename Container::value_type>>
class Priority_queue : public Less {
 public:
  typedef Container container_type;
  typedef Less less_type;
  typedef typename container_type::value_type value_type;
  typedef typename container_type::size_type size_type;
  typedef typename container_type::iterator iterator;
  typedef typename container_type::const_iterator const_iterator;
  typedef typename container_type::allocator_type allocator_type;

  friend class priority_queue_unittest::PriorityQueueTest;

 private:
  // Deriving from Less allows empty base-class optimization in some cases.
  typedef Less Base;

  // Returns the index of the parent node of node i.
  static size_type parent(size_type i) {
    DBUG_ASSERT(i != 0);
    return (--i) >> 1;  // (i - 1) / 2
  }

  // Returns the index of the left child of node i.
  static size_type left(size_type i) {
    return (i << 1) | 1;  // 2 * i + 1
  }

  // Returns the index of the right child of node i.
  static size_type right(size_type i) {
    return (++i) << 1;  // 2 * i + 2
  }

  void heapify(size_type i, size_type last) {
    DBUG_ASSERT(i < size());
    size_type largest = i;

    do {
      i = largest;
      size_type l = left(i);
      size_type r = right(i);

      if (l < last && Base::operator()(m_container[i], m_container[l])) {
        largest = l;
      }

      if (r < last && Base::operator()(m_container[largest], m_container[r])) {
        largest = r;
      }

      if (largest != i) {
        std::swap(m_container[i], m_container[largest]);
      }
    } while (largest != i);
  }

  void heapify(size_type i) { heapify(i, m_container.size()); }

  void reverse_heapify(size_type i) {
    DBUG_ASSERT(i < size());
    while (i > 0 && !Base::operator()(m_container[i], m_container[parent(i)])) {
      std::swap(m_container[parent(i)], m_container[i]);
      i = parent(i);
    }
  }

  // Sets the value of element i, and rebuilds the priority queue.
  void decrease_key(size_type i, value_type const &x) {
    m_container[i] = x;
    heapify(i);
  }

  // Sets the value of element i, and rebuilds the priority queue.
  void increase_key(size_type i, value_type const &x) {
    m_container[i] = x;
    reverse_heapify(i);
  }

 public:
  /// Constructs an empty priority queue.
  Priority_queue(Less const &less = Less(),
                 const allocator_type &alloc = allocator_type())
      : Base(less), m_container(alloc) {}

  /// Constructs a heap of the objects between first and beyond.
  template <typename Input_iterator>
  Priority_queue(Input_iterator first, Input_iterator beyond,
                 Less const &less = Less(),
                 const allocator_type &alloc = allocator_type())
      : Base(less), m_container(first, beyond, alloc) {
    build_heap();
  }

  /// Constructs a heap based on input argument.
  void assign(const container_type &container) {
    m_container = container;
    build_heap();
  }

  /**
    Constructs a heap based on container contents.
    Can also be used when many elements have changed.
  */
  void build_heap() {
    if (m_container.size() > 1) {
      for (size_type i = parent(m_container.size() - 1); i > 0; --i) {
        heapify(i);
      }
      heapify(0);
    }
  }

  /// Returns a const reference to the top element of the priority queue.
  value_type const &top() const {
    DBUG_ASSERT(!empty());
    return m_container[0];
  }

  /// Returns a reference to the top element of the priority queue.
  value_type &top() {
    DBUG_ASSERT(!empty());
    return m_container[0];
  }

  /**
    Inserts an element in the priority queue.

    @param  x value to be pushed.
    @retval true if out-of-memory, false otherwise.
  */
  bool push(value_type const &x) {
    try {
      m_container.push_back(x);
    } catch (std::bad_alloc const &) {
      return true;
    }

    reverse_heapify(m_container.size() - 1);
    return false;
  }

  /// Pops the top-most element in the priority queue.
  void pop() { remove(0); }

  /// Removes the element at position i from the priority queue.
  void remove(size_type i) {
    DBUG_ASSERT(i < size());

    if (i == m_container.size() - 1) {
      m_container.pop_back();
      return;
    }

    m_container[i] = m_container[m_container.size() - 1];
    m_container.pop_back();
    update(i);
  }

  /**
    Decreases the priority of the element at position i, where the
    new priority is x.
  */
  void decrease(size_type i, value_type const &x) {
    DBUG_ASSERT(i < size());
    DBUG_ASSERT(!Base::operator()(m_container[i], x));
    decrease_key(i, x);
  }

  /**
    Increases the priority of the element at position i, where the
    new priority is x.
  */
  void increase(size_type i, value_type const &x) {
    DBUG_ASSERT(i < size());
    DBUG_ASSERT(!Base::operator()(x, m_container[i]));
    increase_key(i, x);
  }

  /**
    Changes the priority of the element at position i, where the
    new priority is x.
  */
  void update(size_type i, value_type const &x) {
    DBUG_ASSERT(i < size());
    if (Base::operator()(x, m_container[i])) {
      decrease_key(i, x);
    } else {
      increase_key(i, x);
    }
  }

  /**
    Assumes that the i-th element's value has increased
    and rebuilds the priority queue.
  */
  void increase(size_type i) { reverse_heapify(i); }

  /**
    Assumes that the i-th element's value has decreased
    and rebuilds the priority queue.
  */
  void decrease(size_type i) { heapify(i); }

  /**
    Assumes that the i-th element's value has changed
    and rebuilds the priority queue.
  */
  void update(size_type i) {
    DBUG_ASSERT(i < size());
    if (i == 0 || Base::operator()(m_container[i], m_container[parent(i)])) {
      heapify(i);
    } else {
      reverse_heapify(i);
    }
  }

  /**
    Assumes that the top element's value has changed
    and rebuilds the priority queue.
  */
  void update_top() {
    DBUG_ASSERT(!empty());
    heapify(0);
  }

  /// Returns the number of elements of the priority queue
  size_type size() const { return m_container.size(); }

  /// Returns true if the priority queue is empty
  bool empty() const { return m_container.empty(); }

  /// Returns a const reference to the i-th element in the underlying container.
  value_type const &operator[](size_type i) const {
    DBUG_ASSERT(i < size());
    return m_container[i];
  }

  /// Returns a reference to the i-th element in the underlying container.
  value_type &operator[](size_type i) {
    DBUG_ASSERT(i < size());
    return m_container[i];
  }

  /// Returns a const iterator to the first element of the underlying container.
  const_iterator begin() const { return m_container.begin(); }

  /// Returns a const iterator to the end element of the underlying container.
  const_iterator end() const { return m_container.end(); }

  /// Returns an iterator to the first element of the underlying container.
  iterator begin() { return m_container.begin(); }

  /// Returns an iterator to the end element of the underlying container.
  iterator end() { return m_container.end(); }

  /// Swaps the contents of two priority queues.
  void swap(Priority_queue &other) {
    std::swap(static_cast<Base &>(*this), static_cast<Base &>(other));
    m_container.swap(other.m_container);
  }

  /// Returns true if the priority queue has the heap property.
  bool is_valid() const {
    for (size_type i = 1; i < m_container.size(); ++i) {
      if (Base::operator()(m_container[parent(i)], m_container[i])) {
        return false;
      }
    }
    return true;
  }

  /**
    Sorts the elements of the priority queue according to the strict
    partial ordering defined by the object of type Less passed to
    the priority queue.

    The heap property of the priority queue is invalidated by this
    operation.
  */
  void sort() {
    if (!m_container.empty()) {
      for (size_type i = m_container.size() - 1; i > 0; --i) {
        std::swap(m_container[i], m_container[0]);
        heapify(0, i);
      }
    }
  }

  /// Clears the priority queue.
  void clear() { m_container.clear(); }

  /// Clears the priority queue, but deletes all elements first.
  void delete_elements() { delete_container_pointers(m_container); }

  /// Returns the capacity of the internal container.
  size_type capacity() const { return m_container.capacity(); }

  /**
    Reserves space for array elements.

    @param  n number of elements.
    @retval true if out-of-memory, false otherwise.
  */
  MY_ATTRIBUTE((warn_unused_result))
  bool reserve(size_type n) {
    DBUG_ASSERT(n <= m_container.max_size());
    try {
      m_container.reserve(n);
    } catch (std::bad_alloc const &) {
      return true;
    }
    return false;
  }

 private:
  container_type m_container;
};

#if defined(EXTRA_CODE_FOR_UNIT_TESTING)
template <class T, class Container, class Less>
inline std::ostream &operator<<(std::ostream &os,
                                Priority_queue<T, Container, Less> const &pq) {
  typedef typename Priority_queue<T, Container, Less>::size_type size_type;

  for (size_type i = 0; i < pq.size(); i++) {
    os << pq[i] << " " << std::flush;
  }

  return os;
}

template <class T, class Container, class Less>
inline std::stringstream &operator<<(
    std::stringstream &ss, Priority_queue<T, Container, Less> const &pq) {
  typedef typename Priority_queue<T, Container, Less>::size_type size_type;

  for (size_type i = 0; i < pq.size(); i++) {
    ss << pq[i] << " ";
    ;
  }

  return ss;
}
#endif  // EXTRA_CODE_FOR_UNIT_TESTING

#endif  // PRIORITY_QUEUE_INCLUDED
