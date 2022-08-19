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

#ifndef DD_CACHE__MULTI_MAP_BASE_INCLUDED
#define DD_CACHE__MULTI_MAP_BASE_INCLUDED

#include <stdio.h>

#include "element_map.h"  // Element_map
#include "sql/dd/types/abstract_table.h"
#include "sql/dd/types/charset.h"
#include "sql/dd/types/collation.h"
#include "sql/dd/types/column_statistics.h"
#include "sql/dd/types/event.h"
#include "sql/dd/types/resource_group.h"
#include "sql/dd/types/routine.h"
#include "sql/dd/types/schema.h"
#include "sql/dd/types/spatial_reference_system.h"
#include "sql/dd/types/tablespace.h"

namespace dd {
namespace cache {

template <typename T>
class Cache_element;

/**
  Implementation of a set of maps for a given object type.

  The class declares a set of maps, each of which maps from a key type
  to an element type. The element type wraps the template object type
  parameter into a wrapper instance.

  The implementation is intended to be used as a base to be extended for
  usage in a specific context. There is support for adding and removing
  elements in all maps with one operation (but not necessarily atomically),
  and for retrieving a single map. There is no support for tracking object
  usage, free list management, thread synchronization, etc.

  @tparam  T  Dictionary object type.
*/

template <typename T>
class Multi_map_base {
 private:
  Element_map<const T *, Cache_element<T>> m_rev_map;  // Reverse element map.

  Element_map<typename T::Id_key, Cache_element<T>>
      m_id_map;  // Id map instance.
  Element_map<typename T::Name_key, Cache_element<T>>
      m_name_map;  // Name map instance.
  Element_map<typename T::Aux_key, Cache_element<T>>
      m_aux_map;  // Aux map instance.

  template <typename K>
  struct Type_selector {};  // Dummy type to use for
                            // selecting map instance.

  /**
    Overloaded functions to use for selecting an element list instance
    based on a key type. Const and non-const variants.
  */

  Element_map<const T *, Cache_element<T>> *m_map(Type_selector<const T *>) {
    return &m_rev_map;
  }

  const Element_map<const T *, Cache_element<T>> *m_map(
      Type_selector<const T *>) const {
    return &m_rev_map;
  }

  Element_map<typename T::Id_key, Cache_element<T>> *m_map(
      Type_selector<typename T::Id_key>) {
    return &m_id_map;
  }

  const Element_map<typename T::Id_key, Cache_element<T>> *m_map(
      Type_selector<typename T::Id_key>) const {
    return &m_id_map;
  }

  Element_map<typename T::Name_key, Cache_element<T>> *m_map(
      Type_selector<typename T::Name_key>) {
    return &m_name_map;
  }

  const Element_map<typename T::Name_key, Cache_element<T>> *m_map(
      Type_selector<typename T::Name_key>) const {
    return &m_name_map;
  }

  Element_map<typename T::Aux_key, Cache_element<T>> *m_map(
      Type_selector<typename T::Aux_key>) {
    return &m_aux_map;
  }

  const Element_map<typename T::Aux_key, Cache_element<T>> *m_map(
      Type_selector<typename T::Aux_key>) const {
    return &m_aux_map;
  }

 public:
  // Iterate based on the reverse map where all elements must be present.
  typedef typename Element_map<const T *, Cache_element<T>>::Const_iterator
      Const_iterator;

  typedef typename Element_map<const T *, Cache_element<T>>::Iterator Iterator;

 protected:
  /**
    Template function to get an element map.

    To support generic code, the element map instances are available
    through template function instances. This allows looking up the
    appropriate instance based on the key type. We must use overloading
    to accomplish this (see above). Const and non-const variants.

    @tparam K Key type.

    @return The element map handling keys of type K.
   */

  template <typename K>
  Element_map<K, Cache_element<T>> *m_map() {
    return m_map(Type_selector<K>());
  }

  template <typename K>
  const Element_map<K, Cache_element<T>> *m_map() const {
    return m_map(Type_selector<K>());
  }

  /**
    Helper function to remove the mapping of a single element, without
    deleting the element itself. This function assumes that checking for
    key and element presence has already been done.

    @param element  Element to be removed and deleted.
  */

  void remove_single_element(Cache_element<T> *element);

  /**
    Helper function to add a single element.

    This function assumes that checking for key and element presence
    has already been done, that the object has been assigned, and that the
    keys have been generated.

    @param element  Element to be added.
  */

  void add_single_element(Cache_element<T> *element);

  /**
    Debug dump of the multi map base to stderr.
  */

  /* purecov: begin inspected */
  void dump() const {
#ifndef DBUG_OFF
    fprintf(stderr, "    Reverse element map:\n");
    m_map<const T *>()->dump();
    fprintf(stderr, "    Id map:\n");
    m_map<typename T::Id_key>()->dump();
    fprintf(stderr, "    Name map:\n");
    m_map<typename T::Name_key>()->dump();
    fprintf(stderr, "    Aux map:\n");
    m_map<typename T::Aux_key>()->dump();
#endif
  }
  /* purecov: end */
};

}  // namespace cache
}  // namespace dd

#endif  // DD_CACHE__MULTI_MAP_BASE_INCLUDED
