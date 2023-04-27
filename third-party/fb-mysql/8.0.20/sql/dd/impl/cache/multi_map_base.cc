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

#include "sql/dd/cache/multi_map_base.h"

#include "my_dbug.h"
#include "sql/dd/impl/cache/cache_element.h"        // Cache_element
#include "sql/dd/types/abstract_table.h"            // Abstract_table
#include "sql/dd/types/charset.h"                   // Charset
#include "sql/dd/types/collation.h"                 // Collation
#include "sql/dd/types/column_statistics.h"         // Column_statistics
#include "sql/dd/types/event.h"                     // Event
#include "sql/dd/types/resource_group.h"            // Resource_group
#include "sql/dd/types/routine.h"                   // Routine
#include "sql/dd/types/schema.h"                    // Schema
#include "sql/dd/types/spatial_reference_system.h"  // Spatial_reference_system
#include "sql/dd/types/tablespace.h"                // Tablespace

namespace dd {
namespace cache {

// Helper function to remove the mapping of a single element.
template <typename T>
void Multi_map_base<T>::remove_single_element(Cache_element<T> *element) {
  // Remove the element from all maps.
  DBUG_ASSERT(element->object());
  if (element->object()) m_map<const T *>()->remove(element->object());
  if (element->id_key())
    m_map<typename T::Id_key>()->remove(*element->id_key());
  if (element->name_key())
    m_map<typename T::Name_key>()->remove(*element->name_key());
  if (element->aux_key())
    m_map<typename T::Aux_key>()->remove(*element->aux_key());
}

// Helper function to add a single element.
template <typename T>
void Multi_map_base<T>::add_single_element(Cache_element<T> *element) {
  // Add the element to all maps.
  DBUG_ASSERT(element->object());
  if (element->object()) m_map<const T *>()->put(element->object(), element);
  if (element->id_key())
    m_map<typename T::Id_key>()->put(*element->id_key(), element);
  if (element->name_key())
    m_map<typename T::Name_key>()->put(*element->name_key(), element);
  if (element->aux_key())
    m_map<typename T::Aux_key>()->put(*element->aux_key(), element);
}

// Explicitly instantiate the types for the various usages.
template class Multi_map_base<Abstract_table>;
template class Multi_map_base<Charset>;
template class Multi_map_base<Collation>;
template class Multi_map_base<Column_statistics>;
template class Multi_map_base<Event>;
template class Multi_map_base<Resource_group>;
template class Multi_map_base<Routine>;
template class Multi_map_base<Schema>;
template class Multi_map_base<Spatial_reference_system>;
template class Multi_map_base<Tablespace>;

}  // namespace cache
}  // namespace dd
