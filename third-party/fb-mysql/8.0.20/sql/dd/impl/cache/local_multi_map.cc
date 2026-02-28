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

#include "sql/dd/cache/local_multi_map.h"

#include "my_dbug.h"
#include "sql/dd/cache/multi_map_base.h"
#include "sql/dd/impl/cache/cache_element.h"  // Cache_element
#include "sql/dd/impl/tables/character_sets.h"
#include "sql/dd/impl/tables/collations.h"
#include "sql/dd/impl/tables/column_statistics.h"
#include "sql/dd/impl/tables/events.h"
#include "sql/dd/impl/tables/resource_groups.h"
#include "sql/dd/impl/tables/routines.h"
#include "sql/dd/impl/tables/schemata.h"
#include "sql/dd/impl/tables/spatial_reference_systems.h"
#include "sql/dd/impl/tables/tables.h"
#include "sql/dd/impl/tables/tablespaces.h"

namespace dd {
class Abstract_table;
class Charset;
class Collation;
class Column_statistics;
class Event;
class Resource_group;
class Routine;
class Schema;
class Spatial_reference_system;
class Tablespace;
}  // namespace dd

namespace dd {
namespace cache {

// Put a new element into the map.
template <typename T>
void Local_multi_map<T>::put(Cache_element<T> *element) {
#ifndef DBUG_OFF
  // The new object instance may not be present in the map.
  Cache_element<T> *e = nullptr;
  m_map<const T *>()->get(element->object(), &e);
  DBUG_ASSERT(!e);

  // Get all keys that were created within the element.
  const typename T::Id_key *id_key = element->id_key();
  const typename T::Name_key *name_key = element->name_key();
  const typename T::Aux_key *aux_key = element->aux_key();

  // There must be at least one key.
  DBUG_ASSERT(id_key || name_key || aux_key);

  // None of the keys may exist.
  DBUG_ASSERT(
      (!id_key || !m_map<typename T::Id_key>()->is_present(*id_key)) &&
      (!name_key || !m_map<typename T::Name_key>()->is_present(*name_key)) &&
      (!aux_key || !m_map<typename T::Aux_key>()->is_present(*aux_key)));
#endif

  // Add the keys and the element to the maps.
  Multi_map_base<T>::add_single_element(element);
}

// Remove an element from the map.
template <typename T>
void Local_multi_map<T>::remove(Cache_element<T> *element) {
#ifndef DBUG_OFF
  // The object must be present.
  Cache_element<T> *e = nullptr;
  m_map<const T *>()->get(element->object(), &e);
  DBUG_ASSERT(e);

  // Get all keys that were created within the element.
  const typename T::Id_key *id_key = element->id_key();
  const typename T::Name_key *name_key = element->name_key();
  const typename T::Aux_key *aux_key = element->aux_key();

  // All non-null keys must exist.
  DBUG_ASSERT(
      (!id_key || m_map<typename T::Id_key>()->is_present(*id_key)) &&
      (!name_key || m_map<typename T::Name_key>()->is_present(*name_key)) &&
      (!aux_key || m_map<typename T::Aux_key>()->is_present(*aux_key)));
#endif

  // Remove the keys and the element from the maps.
  Multi_map_base<T>::remove_single_element(element);
}

// Remove and delete all elements and objects from the map.
template <typename T>
void Local_multi_map<T>::erase() {
  typename Multi_map_base<T>::Const_iterator it;
  for (it = begin(); it != end();) {
    DBUG_ASSERT(it->second);
    DBUG_ASSERT(it->second->object());

    // Make sure we handle iterator invalidation: Increment
    // before erasing.
    Cache_element<T> *element = it->second;
    ++it;

    // Remove the element from the multi map, delete the wrapped object.
    remove(element);
    delete element->object();
    delete element;
  }
}

/* purecov: begin inspected */
template <typename T>
void Local_multi_map<T>::dump() const {
#ifndef DBUG_OFF
  fprintf(stderr, "  --------------------------------\n");
  fprintf(stderr, "  Local multi map for '%s'\n",
          T::DD_table::instance().name().c_str());
  Multi_map_base<T>::dump();
  fprintf(stderr, "  --------------------------------\n");
#endif
}
/* purecov: end */

// Explicitly instantiate the types for the various usages.
template class Local_multi_map<Abstract_table>;
template class Local_multi_map<Charset>;
template class Local_multi_map<Collation>;
template class Local_multi_map<Column_statistics>;
template class Local_multi_map<Event>;
template class Local_multi_map<Resource_group>;
template class Local_multi_map<Routine>;
template class Local_multi_map<Schema>;
template class Local_multi_map<Spatial_reference_system>;
template class Local_multi_map<Tablespace>;

}  // namespace cache
}  // namespace dd
