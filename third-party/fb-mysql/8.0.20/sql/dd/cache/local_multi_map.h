/* Copyright (c) 2015, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD_CACHE__LOCAL_MULTI_MAP_INCLUDED
#define DD_CACHE__LOCAL_MULTI_MAP_INCLUDED

#include <stdio.h>

#include "multi_map_base.h"  // Multi_map_base
#include "my_dbug.h"
#include "sql/dd/types/entity_object_table.h"  // dd::Entity_object_table

namespace dd {
namespace cache {

template <typename K, typename E>
class Element_map;
template <typename T>
class Cache_element;

/**
  Implementation of a local set of maps for a given object type.

  The implementation is an extension of the multi map base, adding support
  for iteration. It is intended to be used in a single threaded context, and
  there is no support for tracking object usage, free list management,
  thread synchronization, etc.

  @tparam  T  Dictionary object type.
*/

template <typename T>
class Local_multi_map : public Multi_map_base<T> {
 private:
  /**
    Template helper function getting the element map.

    Const and non-const variants.

    @note   Slightly weird syntax is needed to help the parser
            to resolve this correctly.

    @tparam K  Key type.

    @return    The element map handling keys of type K.
   */

  template <typename K>
  Element_map<K, Cache_element<T>> *m_map() {
    return Multi_map_base<T>::template m_map<K>();
  }

  template <typename K>
  const Element_map<K, Cache_element<T>> *m_map() const {
    return Multi_map_base<T>::template m_map<K>();
  }

 public:
  /**
    Get an iterator to the beginning of the map.

    Const and non-const variants.

    @return Iterator to the beginning of the map.
  */

  /* purecov: begin inspected */
  typename Multi_map_base<T>::Const_iterator begin() const {
    return m_map<const T *>()->begin();
  }
  /* purecov: end */

  typename Multi_map_base<T>::Iterator begin() {
    return m_map<const T *>()->begin();
  }

  /**
    Get an iterator to one past the end of the map.

    Const and non-const variants.

    @return Iterator to one past the end of the map.
  */

  /* purecov: begin inspected */
  typename Multi_map_base<T>::Const_iterator end() const {
    return m_map<const T *>()->end();
  }
  /* purecov: end */

  typename Multi_map_base<T>::Iterator end() {
    return m_map<const T *>()->end();
  }

  /**
    Get an element from the map handling the given key type.

    If the element is present, return a pointer to it. Otherwise,
    return NULL.

    @tparam      K       Key type.
    @param       key     Key to use for looking up the element.
    @param [out] element Element pointer, if present, otherwise NULL.
  */

  template <typename K>
  void get(const K &key, Cache_element<T> **element) const {
    m_map<K>()->get(key, element);
  }

  /**
    Put a new element into the map.

    None of the keys may exist in advance, and the wrapped object may not
    be present in this map already.

    @param   element         New element to be added.
  */

  void put(Cache_element<T> *element);

  /**
    Remove an element from the map.

    This function will remove the element from the multi map. This means that
    all keys associated with the element will be removed from the maps, and
    the cache element wrapper will be removed, but not deleted. The object
    itself is not deleted. It is up to the outer layer to decide what to do
    with the element and object.

    @param element           Element to be removed.
  */

  void remove(Cache_element<T> *element);

  /**
    Remove and delete all objects from the map. This includes
    Cache_elements and the Dictionary objects themselves.
  */

  void erase();

  /**
    Get the number of elements in the map.

    @return  Number of elements.
  */

  size_t size() const { return m_map<const T *>()->size(); }

  /**
    Debug dump of the local multi map to stderr.
  */

  void dump() const;
};

}  // namespace cache
}  // namespace dd

#endif  // DD_CACHE__LOCAL_MULTI_MAP_INCLUDED
