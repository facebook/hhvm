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

#include "sql/dd/impl/cache/shared_dictionary_cache.h"

#include <atomic>

#include "my_dbug.h"
#include "sql/dd/impl/cache/shared_multi_map.h"
#include "sql/dd/impl/cache/storage_adapter.h"  // Storage_adapter
#include "sql/mysqld.h"
#include "sql/sql_class.h"  // THD::is_error()

namespace dd {
namespace cache {

template <typename T>
class Cache_element;

Shared_dictionary_cache *Shared_dictionary_cache::instance() {
  static Shared_dictionary_cache s_cache;
  return &s_cache;
}

void Shared_dictionary_cache::init() {
  instance()->m_map<Collation>()->set_capacity(collation_capacity);
  instance()->m_map<Charset>()->set_capacity(charset_capacity);

  // Set capacity to have room for all connections to leave an element
  // unused in the cache to avoid frequent cache misses while e.g.
  // opening a table.
  instance()->m_map<Abstract_table>()->set_capacity(max_connections);
  instance()->m_map<Event>()->set_capacity(event_capacity);
  instance()->m_map<Routine>()->set_capacity(stored_program_def_size);
  instance()->m_map<Schema>()->set_capacity(schema_def_size);
  instance()->m_map<Column_statistics>()->set_capacity(
      column_statistics_capacity);
  instance()->m_map<Spatial_reference_system>()->set_capacity(
      spatial_reference_system_capacity);
  instance()->m_map<Tablespace>()->set_capacity(tablespace_def_size);
  instance()->m_map<Resource_group>()->set_capacity(resource_group_capacity);
}

void Shared_dictionary_cache::shutdown() {
  instance()->m_map<Abstract_table>()->shutdown();
  instance()->m_map<Collation>()->shutdown();
  instance()->m_map<Column_statistics>()->shutdown();
  instance()->m_map<Charset>()->shutdown();
  instance()->m_map<Event>()->shutdown();
  instance()->m_map<Routine>()->shutdown();
  instance()->m_map<Schema>()->shutdown();
  instance()->m_map<Spatial_reference_system>()->shutdown();
  instance()->m_map<Tablespace>()->shutdown();
  instance()->m_map<Resource_group>()->shutdown();
}

// Don't call this function anywhere except upgrade scenario.
void Shared_dictionary_cache::reset(bool keep_dd_entities) {
  shutdown();
  if (!keep_dd_entities) Storage_adapter::instance()->erase_all();
  init();
}

// Workaround to be used during recovery at server restart.
bool Shared_dictionary_cache::reset_tables_and_tablespaces(THD *thd) {
  return (instance()->m_map<Abstract_table>()->reset(thd) ||
          instance()->m_map<Tablespace>()->reset(thd));
}

// Get an element from the cache, given the key.
template <typename K, typename T>
bool Shared_dictionary_cache::get(THD *thd, const K &key,
                                  Cache_element<T> **element) {
  bool error = false;
  DBUG_ASSERT(element);
  if (m_map<T>()->get(key, element)) {
    // Handle cache miss.
    const T *new_object = nullptr;
    error = get_uncached(thd, key, ISO_READ_COMMITTED, &new_object);

    // Add the new object, and assign the output element, even in the case of
    // a miss error (needed to remove the missed key).
    m_map<T>()->put(&key, new_object, element);
  }
  return error;
}

// Read an object directly from disk, given the key.
template <typename K, typename T>
bool Shared_dictionary_cache::get_uncached(THD *thd, const K &key,
                                           enum_tx_isolation isolation,
                                           const T **object) const {
  DBUG_ASSERT(object);
  bool error = Storage_adapter::get(thd, key, isolation, false, object);
  DBUG_ASSERT(!error || thd->is_system_thread() || thd->killed ||
              thd->is_error());

  return error;
}

// Add an object to the shared cache.
template <typename T>
void Shared_dictionary_cache::put(const T *object, Cache_element<T> **element) {
  DBUG_ASSERT(object);
  // Cast needed to help the compiler choose the correct template instance..
  m_map<T>()->put(static_cast<const typename T::Id_key *>(nullptr), object,
                  element);
}

// Explicitly instantiate the types for the various usages.
template bool
Shared_dictionary_cache::get<Abstract_table::Id_key, Abstract_table>(
    THD *thd, const Abstract_table::Id_key &, Cache_element<Abstract_table> **);
template bool
Shared_dictionary_cache::get<Abstract_table::Name_key, Abstract_table>(
    THD *thd, const Abstract_table::Name_key &,
    Cache_element<Abstract_table> **);
template bool
Shared_dictionary_cache::get<Abstract_table::Aux_key, Abstract_table>(
    THD *thd, const Abstract_table::Aux_key &,
    Cache_element<Abstract_table> **);
template bool
Shared_dictionary_cache::get_uncached<Abstract_table::Id_key, Abstract_table>(
    THD *thd, const Abstract_table::Id_key &, enum_tx_isolation,
    const Abstract_table **) const;
template bool
Shared_dictionary_cache::get_uncached<Abstract_table::Name_key, Abstract_table>(
    THD *thd, const Abstract_table::Name_key &, enum_tx_isolation,
    const Abstract_table **) const;
template bool
Shared_dictionary_cache::get_uncached<Abstract_table::Aux_key, Abstract_table>(
    THD *thd, const Abstract_table::Aux_key &, enum_tx_isolation,
    const Abstract_table **) const;
template void Shared_dictionary_cache::put<Abstract_table>(
    const Abstract_table *, Cache_element<Abstract_table> **);

template bool Shared_dictionary_cache::get<Charset::Id_key, Charset>(
    THD *thd, const Charset::Id_key &, Cache_element<Charset> **);
template bool Shared_dictionary_cache::get<Charset::Name_key, Charset>(
    THD *thd, const Charset::Name_key &, Cache_element<Charset> **);
template bool Shared_dictionary_cache::get<Charset::Aux_key, Charset>(
    THD *thd, const Charset::Aux_key &, Cache_element<Charset> **);
template bool Shared_dictionary_cache::get_uncached<Charset::Id_key, Charset>(
    THD *thd, const Charset::Id_key &, enum_tx_isolation,
    const Charset **) const;
template bool Shared_dictionary_cache::get_uncached<Charset::Name_key, Charset>(
    THD *thd, const Charset::Name_key &, enum_tx_isolation,
    const Charset **) const;
template bool Shared_dictionary_cache::get_uncached<Charset::Aux_key, Charset>(
    THD *thd, const Charset::Aux_key &, enum_tx_isolation,
    const Charset **) const;
template void Shared_dictionary_cache::put<Charset>(const Charset *,
                                                    Cache_element<Charset> **);

template bool Shared_dictionary_cache::get<Collation::Id_key, Collation>(
    THD *thd, const Collation::Id_key &, Cache_element<Collation> **);
template bool Shared_dictionary_cache::get<Collation::Name_key, Collation>(
    THD *thd, const Collation::Name_key &, Cache_element<Collation> **);
template bool Shared_dictionary_cache::get<Collation::Aux_key, Collation>(
    THD *thd, const Collation::Aux_key &, Cache_element<Collation> **);
template bool Shared_dictionary_cache::get_uncached<
    Collation::Id_key, Collation>(THD *thd, const Collation::Id_key &,
                                  enum_tx_isolation, const Collation **) const;
template bool
Shared_dictionary_cache::get_uncached<Collation::Name_key, Collation>(
    THD *thd, const Collation::Name_key &, enum_tx_isolation,
    const Collation **) const;
template bool Shared_dictionary_cache::get_uncached<
    Collation::Aux_key, Collation>(THD *thd, const Collation::Aux_key &,
                                   enum_tx_isolation, const Collation **) const;
template void Shared_dictionary_cache::put<Collation>(
    const Collation *, Cache_element<Collation> **);

template bool Shared_dictionary_cache::get<Event::Id_key, Event>(
    THD *thd, const Event::Id_key &, Cache_element<Event> **);
template bool Shared_dictionary_cache::get<Event::Name_key, Event>(
    THD *thd, const Event::Name_key &, Cache_element<Event> **);
template bool Shared_dictionary_cache::get<Event::Aux_key, Event>(
    THD *thd, const Event::Aux_key &, Cache_element<Event> **);
template bool Shared_dictionary_cache::get_uncached<Event::Id_key, Event>(
    THD *thd, const Event::Id_key &, enum_tx_isolation, const Event **) const;
template bool Shared_dictionary_cache::get_uncached<Event::Name_key, Event>(
    THD *thd, const Event::Name_key &, enum_tx_isolation, const Event **) const;
template bool Shared_dictionary_cache::get_uncached<Event::Aux_key, Event>(
    THD *thd, const Event::Aux_key &, enum_tx_isolation, const Event **) const;
template void Shared_dictionary_cache::put<Event>(const Event *,
                                                  Cache_element<Event> **);

template bool Shared_dictionary_cache::get<Routine::Id_key, Routine>(
    THD *thd, const Routine::Id_key &, Cache_element<Routine> **);
template bool Shared_dictionary_cache::get<Routine::Name_key, Routine>(
    THD *thd, const Routine::Name_key &, Cache_element<Routine> **);
template bool Shared_dictionary_cache::get<Routine::Aux_key, Routine>(
    THD *thd, const Routine::Aux_key &, Cache_element<Routine> **);
template bool Shared_dictionary_cache::get_uncached<Routine::Id_key, Routine>(
    THD *thd, const Routine::Id_key &, enum_tx_isolation,
    const Routine **) const;
template bool Shared_dictionary_cache::get_uncached<Routine::Name_key, Routine>(
    THD *thd, const Routine::Name_key &, enum_tx_isolation,
    const Routine **) const;
template bool Shared_dictionary_cache::get_uncached<Routine::Aux_key, Routine>(
    THD *thd, const Routine::Aux_key &, enum_tx_isolation,
    const Routine **) const;
template void Shared_dictionary_cache::put<Routine>(const Routine *,
                                                    Cache_element<Routine> **);

template bool Shared_dictionary_cache::get<Schema::Id_key, Schema>(
    THD *thd, const Schema::Id_key &, Cache_element<Schema> **);
template bool Shared_dictionary_cache::get<Schema::Name_key, Schema>(
    THD *thd, const Schema::Name_key &, Cache_element<Schema> **);
template bool Shared_dictionary_cache::get<Schema::Aux_key, Schema>(
    THD *thd, const Schema::Aux_key &, Cache_element<Schema> **);
template bool Shared_dictionary_cache::get_uncached<Schema::Id_key, Schema>(
    THD *thd, const Schema::Id_key &, enum_tx_isolation, const Schema **) const;
template bool Shared_dictionary_cache::get_uncached<Schema::Name_key, Schema>(
    THD *thd, const Schema::Name_key &, enum_tx_isolation,
    const Schema **) const;
template bool Shared_dictionary_cache::get_uncached<Schema::Aux_key, Schema>(
    THD *thd, const Schema::Aux_key &, enum_tx_isolation,
    const Schema **) const;
template void Shared_dictionary_cache::put<Schema>(const Schema *,
                                                   Cache_element<Schema> **);

template bool Shared_dictionary_cache::get<Spatial_reference_system::Id_key,
                                           Spatial_reference_system>(
    THD *thd, const Spatial_reference_system::Id_key &,
    Cache_element<Spatial_reference_system> **);
template bool Shared_dictionary_cache::get<Spatial_reference_system::Name_key,
                                           Spatial_reference_system>(
    THD *thd, const Spatial_reference_system::Name_key &,
    Cache_element<Spatial_reference_system> **);
template bool Shared_dictionary_cache::get<Spatial_reference_system::Aux_key,
                                           Spatial_reference_system>(
    THD *thd, const Spatial_reference_system::Aux_key &,
    Cache_element<Spatial_reference_system> **);
template bool Shared_dictionary_cache::get_uncached<
    Spatial_reference_system::Id_key, Spatial_reference_system>(
    THD *thd, const Spatial_reference_system::Id_key &, enum_tx_isolation,
    const Spatial_reference_system **) const;
template bool Shared_dictionary_cache::get_uncached<
    Spatial_reference_system::Name_key, Spatial_reference_system>(
    THD *thd, const Spatial_reference_system::Name_key &, enum_tx_isolation,
    const Spatial_reference_system **) const;
template bool Shared_dictionary_cache::get_uncached<
    Spatial_reference_system::Aux_key, Spatial_reference_system>(
    THD *thd, const Spatial_reference_system::Aux_key &, enum_tx_isolation,
    const Spatial_reference_system **) const;
template void Shared_dictionary_cache::put<Spatial_reference_system>(
    const Spatial_reference_system *,
    Cache_element<Spatial_reference_system> **);

template bool
Shared_dictionary_cache::get<Column_statistics::Id_key, Column_statistics>(
    THD *thd, const Column_statistics::Id_key &,
    Cache_element<Column_statistics> **);
template bool
Shared_dictionary_cache::get<Column_statistics::Name_key, Column_statistics>(
    THD *thd, const Column_statistics::Name_key &,
    Cache_element<Column_statistics> **);
template bool
Shared_dictionary_cache::get<Column_statistics::Aux_key, Column_statistics>(
    THD *thd, const Column_statistics::Aux_key &,
    Cache_element<Column_statistics> **);
template bool Shared_dictionary_cache::get_uncached<Column_statistics::Id_key,
                                                    Column_statistics>(
    THD *thd, const Column_statistics::Id_key &, enum_tx_isolation,
    const Column_statistics **) const;
template bool Shared_dictionary_cache::get_uncached<Column_statistics::Name_key,
                                                    Column_statistics>(
    THD *thd, const Column_statistics::Name_key &, enum_tx_isolation,
    const Column_statistics **) const;
template bool Shared_dictionary_cache::get_uncached<Column_statistics::Aux_key,
                                                    Column_statistics>(
    THD *thd, const Column_statistics::Aux_key &, enum_tx_isolation,
    const Column_statistics **) const;
template void Shared_dictionary_cache::put<Column_statistics>(
    const Column_statistics *, Cache_element<Column_statistics> **);

template bool Shared_dictionary_cache::get<Tablespace::Id_key, Tablespace>(
    THD *thd, const Tablespace::Id_key &, Cache_element<Tablespace> **);
template bool Shared_dictionary_cache::get<Tablespace::Name_key, Tablespace>(
    THD *thd, const Tablespace::Name_key &, Cache_element<Tablespace> **);
template bool Shared_dictionary_cache::get<Tablespace::Aux_key, Tablespace>(
    THD *thd, const Tablespace::Aux_key &, Cache_element<Tablespace> **);
template bool
Shared_dictionary_cache::get_uncached<Tablespace::Id_key, Tablespace>(
    THD *thd, const Tablespace::Id_key &, enum_tx_isolation,
    const Tablespace **) const;
template bool
Shared_dictionary_cache::get_uncached<Tablespace::Name_key, Tablespace>(
    THD *thd, const Tablespace::Name_key &, enum_tx_isolation,
    const Tablespace **) const;
template bool
Shared_dictionary_cache::get_uncached<Tablespace::Aux_key, Tablespace>(
    THD *thd, const Tablespace::Aux_key &, enum_tx_isolation,
    const Tablespace **) const;
template void Shared_dictionary_cache::put<Tablespace>(
    const Tablespace *, Cache_element<Tablespace> **);

template bool
Shared_dictionary_cache::get<Resource_group::Id_key, Resource_group>(
    THD *thd, const Resource_group::Id_key &, Cache_element<Resource_group> **);
template bool
Shared_dictionary_cache::get<Resource_group::Name_key, Resource_group>(
    THD *thd, const Resource_group::Name_key &,
    Cache_element<Resource_group> **);
template bool
Shared_dictionary_cache::get<Resource_group::Aux_key, Resource_group>(
    THD *thd, const Resource_group::Aux_key &,
    Cache_element<Resource_group> **);
template bool
Shared_dictionary_cache::get_uncached<Resource_group::Id_key, Resource_group>(
    THD *thd, const Resource_group::Id_key &, enum_tx_isolation,
    const Resource_group **) const;
template bool
Shared_dictionary_cache::get_uncached<Resource_group::Name_key, Resource_group>(
    THD *thd, const Resource_group::Name_key &, enum_tx_isolation,
    const Resource_group **) const;
template bool
Shared_dictionary_cache::get_uncached<Resource_group::Aux_key, Resource_group>(
    THD *thd, const Resource_group::Aux_key &, enum_tx_isolation,
    const Resource_group **) const;
template void Shared_dictionary_cache::put<Resource_group>(
    const Resource_group *, Cache_element<Resource_group> **);
}  // namespace cache
}  // namespace dd
