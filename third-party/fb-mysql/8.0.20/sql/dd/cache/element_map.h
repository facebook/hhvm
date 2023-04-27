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

#ifndef DD_CACHE__ELEMENT_MAP_INCLUDED
#define DD_CACHE__ELEMENT_MAP_INCLUDED

#include <cstddef>  // size_t
#include <map>      // std::map
#include <set>      // std::set

#include "my_dbug.h"
#include "sql/malloc_allocator.h"  // Malloc_allocator.

namespace dd {
namespace cache {

/**
  Implementation of a map between a key type and an element type.

  The map supports storing associations between instances of the key
  type K and pointers to the element type E. Additionally, the map provides
  support for registering keys that have been searched for in the map
  without being present. This allows registering a cache miss to avoid
  situations where several similar cache misses are handled simultaneously.
  Instead, only the first cache miss needs to be processed, while the
  subsequent ones can just wait for the first one to complete.

  There are no expectations as far as the element type E is concerned.
  Tracking object usage, memory management, loading and evicting elements,
  locking and synchronization, as well as registering cache misses are all
  issues that are handled by users of this interface.

  Basic assumptions regarding correct usage is implemented in terms of
  asserts to verify that, e.g., a key does not already exist in the map
  when it is added. There is, however, no assumptions regarding the
  relationship between the set of missed keys and the state of the map.

  There is support for representing missed keys, but the usage of this
  feature is left to the outer layer; e.g., this class does not assert
  that an element has been registered as missing before it is added to
  the map.

  @note The element pointer in a key/value pair may not be NULL.

  @tparam  K  Key type.
  @tparam  E  Element type (a Cache_element wrapping some dictionary
              object type).
*/

template <typename K, typename E>
class Element_map {
 public:
  typedef std::map<K, E *, std::less<K>,
                   Malloc_allocator<std::pair<const K, E *>>>
      Element_map_type;  // Real map type.
  typedef typename Element_map_type::const_iterator
      Const_iterator;                                    // Const iterator type.
  typedef typename Element_map_type::iterator Iterator;  // Iterator type.

 private:
  Element_map_type m_map;  // The real map instance.
  std::set<K, std::less<K>,
           Malloc_allocator<K>>
      m_missed;  // Cache misses being handled.

 public:
  Element_map()
      : m_map(std::less<K>(),
              Malloc_allocator<std::pair<const K, E *>>(PSI_INSTRUMENT_ME)),
        m_missed(std::less<K>(), Malloc_allocator<K>(PSI_INSTRUMENT_ME)) {
  } /* purecov: tested */

  /**
    Get an iterator to the beginning of the map.

    Const and non-const variants.

    @return Iterator to the beginning of the map.
  */

  /* purecov: begin inspected */
  Const_iterator begin() const { return m_map.begin(); }
  /* purecov: end */

  Iterator begin() { return m_map.begin(); }

  /**
    Get an iterator to one past the end of the map.

    Const and non-const variants.

    @return Iterator to one past the end of the map.
  */

  /* purecov: begin inspected */
  Const_iterator end() const { return m_map.end(); }
  /* purecov: end */

  Iterator end() { return m_map.end(); }

  /**
    Return the number of elements in the map.

    @return   Number of elements in the map.
  */

  size_t size() const { return m_map.size(); }

  /**
    Check if the given key is present in the map.

    If the key is present in the map, return true, otherwise false.

    @param   key   The key to look for.

    @return        true if present, otherwise false.
  */

  bool is_present(const K &key) const { return m_map.find(key) != m_map.end(); }

  /**
    Get the element associated with the given key.

    If the element is present in the map, return a pointer to it. If the
    element is not present, return NULL as the element pointer.

    @param       key      The key to use for looking up the element.
    @param [out] element  The element associated with the key (if present),
                          or NULL (if not present).
  */

  void get(const K &key, E **element) const {
    DBUG_ASSERT(element);
    typename Element_map_type::const_iterator it = m_map.find(key);
    if (it == m_map.end())
      *element = nullptr;
    else {
      DBUG_ASSERT(it->second);
      *element = it->second;
    }
  }

  /**
    Put the element into the map and associate it with the given key.

    The element may not be NULL, and the key may not be already present.

    @param       key      The key to be associated with the element.
    @param       element  The element to be associated with the key.
  */

  void put(const K &key, E *element) {
    DBUG_ASSERT(element);
    DBUG_ASSERT(m_map.find(key) == m_map.end());
    m_map.insert(typename Element_map_type::value_type(key, element));
  }

  /**
    Remove an element from the map.

    The key/value pair, as indicated by the key, is removed from the map.
    The element being pointed to is not deleted. The key must be present in
    the map.

    @param       key      The key to be removed.
  */

  void remove(const K &key) {
    DBUG_ASSERT(m_map.find(key) != m_map.end());
    m_map.erase(key);
  }

  /**
    Check if the given key has been missed in the cache.

    @param  key  Key representing element to check for recent cache miss.
    @return true if the element has been missed, otherwise false.
  */

  bool is_missed(const K &key) const {
    return m_missed.find(key) != m_missed.end();
  }

  /**
    Register the given key as being missed in the cache.

    The key cannot already be reported as missing.

    @param  key  Key representing element missed in the cache.
  */

  void set_missed(const K &key) {
    DBUG_ASSERT(m_missed.find(key) == m_missed.end());
    m_missed.insert(key);
  }

  /**
    Register that the miss of a key has been handled.

    The key must have been reported as missing.

    @param  key  Key representing element previously missed in the cache.
  */

  void set_miss_handled(const K &key) {
    DBUG_ASSERT(m_missed.find(key) != m_missed.end());
    m_missed.erase(key);
  }

  /**
    Debug dump of the element map to stderr.

    Iterates over all elements and dumps debug information for each
    of them.
  */

  /* purecov: begin inspected */
  void dump() const {
#ifndef DBUG_OFF
    Const_iterator it;
    for (it = m_map.begin(); it != m_map.end(); it++) it->second->dump();
#endif
  }
  /* purecov: end */
};

}  // namespace cache
}  // namespace dd

#endif  // DD_CACHE__ELEMENT_MAP_INCLUDED
