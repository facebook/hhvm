/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD_CACHE__FREE_LIST_INCLUDED
#define DD_CACHE__FREE_LIST_INCLUDED

#include <vector>  // vector

#include "my_dbug.h"
#include "sql/malloc_allocator.h"  // Malloc_allocator.

namespace dd {
namespace cache {

/**
  Template for management of a free list based on a std::vector.

  The free list template defines functions mostly wrapping the std::vector
  functions, but additionally doing some asserts to ensure correct usage.

  The first element in the free list is the least recently used element.
  When a new element becomes unused, it is added to the end of the free list.
  An element may also be removed from within the middle of the free list
  when the element is being acquired. This use case means iterating through
  the vector to find the correct element.

  @tparam  E  Element type (a Cache_element wrapping some dictionary
              object type).
*/

template <typename E>
class Free_list {
 private:
  typedef std::vector<E *, Malloc_allocator<E *>> List_type;
  List_type m_list;  // The actual list.

 public:
  Free_list() : m_list(Malloc_allocator<E *>(PSI_INSTRUMENT_ME)) {}

  // Return the actual free list length.
  size_t length() const { return m_list.size(); }

  /**
    Add an element to the end of the free list.

    @param   element Element to add to the end of the list.
  */

  void add_last(E *element) {
    DBUG_ASSERT(element != nullptr && element->usage() == 0);
    m_list.push_back(element);
  }

  /**
    Remove an element from the free list.

    @param   element Element to be removed from the list.
  */

  void remove(E *element) {
    DBUG_ASSERT(element != nullptr && element->usage() == 0);
    DBUG_ASSERT(!m_list.empty());

    for (typename List_type::iterator it = m_list.begin(); it != m_list.end();
         ++it)
      if (*it == element) {
        m_list.erase(it);
        return;
      }

    DBUG_ASSERT(false); /* purecov: deadcode */
  }

  /**
    Get the least recently used element in the list, i.e., the first element.

    @return  The least recently used element in the list.
  */

  E *get_lru() const {
    DBUG_ASSERT(!m_list.empty());
    if (!m_list.empty()) return m_list.front();
    return nullptr;
  }

  /**
    Debug dump of the free list to stderr.
  */
  /* purecov: begin inspected */
  void dump() const {
#ifndef DBUG_OFF
    if (m_list.empty()) {
      fprintf(stderr, "    lru-> NULL\n");
      return;
    }
    fprintf(stderr, "    lru-> ");
    for (typename List_type::const_iterator it = m_list.begin();
         it != m_list.end(); ++it)
      fprintf(stderr, "%llu ", (*it)->object()->id());
    fprintf(stderr, "\n");
#endif
  }
  /* purecov: end */
};

}  // namespace cache
}  // namespace dd

#endif  // DD_CACHE__FREE_LIST_INCLUDED
