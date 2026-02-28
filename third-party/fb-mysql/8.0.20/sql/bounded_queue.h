/* Copyright (c) 2010, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef BOUNDED_QUEUE_INCLUDED
#define BOUNDED_QUEUE_INCLUDED

#include "my_base.h"
#include "my_sys.h"
#include "mysys_err.h"
#include "priority_queue.h"
#include "sql/malloc_allocator.h"

/**
  A priority queue with a fixed, limited size.

  This is a wrapper on top of Priority_queue.
  It keeps the top-N elements which are inserted.

  Elements of type Element_type are pushed into the queue.
  For each element, we call a user-supplied Key_generator::make_sortkey(),
  to generate a key of type Key_type for the element.
  Instances of Key_type are compared with the user-supplied Key_compare.

  Pointers to the top-N elements are stored in the sort_keys array given
  to the init() function below. To access elements in sorted order,
  sort the array and access it sequentially.
 */
template <typename Element_type, typename Key_type, typename Key_generator,
          typename Key_compare = std::less<Key_type>>
class Bounded_queue {
 public:
  typedef Priority_queue<
      Key_type, std::vector<Key_type, Malloc_allocator<Key_type>>, Key_compare>
      Queue_type;

  typedef typename Queue_type::allocator_type allocator_type;

  Bounded_queue(
      size_t element_size = sizeof(Element_type),
      const allocator_type &alloc = allocator_type(PSI_NOT_INSTRUMENTED))
      : m_queue(Key_compare(), alloc),
        m_sort_keys(nullptr),
        m_sort_param(nullptr),
        m_element_size(element_size) {}

  /**
    Initialize the queue.

    @param max_elements   The size of the queue.
    @param sort_param     Sort parameters. We call sort_param->make_sortkey()
                          to generate keys for elements.
    @param[in,out] sort_keys Array of keys to sort.
                             Must be initialized by caller.
                             Will be filled with pointers to the top-N elements.

    @retval false OK, true Could not allocate memory.

    We do *not* take ownership of any of the input pointer arguments.
   */
  bool init(ha_rows max_elements, Key_generator *sort_param,
            Key_type *sort_keys) {
    m_sort_keys = sort_keys;
    m_sort_param = sort_param;
    DBUG_EXECUTE_IF("bounded_queue_init_fail",
                    my_error(EE_OUTOFMEMORY, MYF(ME_FATALERROR), 42);
                    return true;);

    // We allocate space for one extra element, for replace when queue is full.
    if (m_queue.reserve(max_elements + 1)) return true;
    // We cannot have packed keys in the queue.
    m_queue.m_compare_length = sort_param->max_compare_length();
    // We can have variable length keys though.
    if (sort_param->using_varlen_keys()) m_queue.m_param = sort_param;
    return false;
  }

  /**
    Pushes an element on the queue.
    If the queue is already full, we discard one element.
    Calls m_sort_param::make_sortkey() to generate a key for the element.

    @param element        The element to be pushed.
   */
  void push(Element_type element) {
    /*
      Add one extra byte to each key, so that sort-key generating functions
      won't be returning out-of-space. Since we know there's always room
      given a "m_element_size"-sized buffer even in the worst case (by
      definition), we could in principle make a special mode flag in
      Sort_param::make_sortkey() instead for the case of fixed-length records,
      but this is much simpler.
     */
    DBUG_ASSERT(m_element_size < 0xFFFFFFFF);
    const uint element_size = m_element_size + 1;

    if (m_queue.size() == m_queue.capacity()) {
      const Key_type &pq_top = m_queue.top();
      const uint MY_ATTRIBUTE((unused)) rec_sz =
          m_sort_param->make_sortkey(pq_top, element_size, element);
      DBUG_ASSERT(rec_sz <= m_element_size);
      m_queue.update_top();
    } else {
      const uint MY_ATTRIBUTE((unused)) rec_sz = m_sort_param->make_sortkey(
          m_sort_keys[m_queue.size()], element_size, element);
      DBUG_ASSERT(rec_sz <= m_element_size);
      m_queue.push(m_sort_keys[m_queue.size()]);
    }
  }

  /**
    The number of elements in the queue.
   */
  size_t num_elements() const { return m_queue.size(); }

 private:
  Queue_type m_queue;
  Key_type *m_sort_keys;
  Key_generator *m_sort_param;
  size_t m_element_size;
};

#endif  // BOUNDED_QUEUE_INCLUDED
