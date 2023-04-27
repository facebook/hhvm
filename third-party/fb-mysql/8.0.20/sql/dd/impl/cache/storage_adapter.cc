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

#include "sql/dd/impl/cache/storage_adapter.h"

#include <memory>
#include <string>

#include "mutex_lock.h"  // Mutex_lock
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysql/components/services/log_builtins.h"
#include "sql/dd/cache/dictionary_client.h"       // Dictionary_client
#include "sql/dd/impl/bootstrap/bootstrap_ctx.h"  // bootstrap::DD_bootstrap_ctx
#include "sql/dd/impl/cache/cache_element.h"
#include "sql/dd/impl/raw/object_keys.h"           // Primary_id_key
#include "sql/dd/impl/raw/raw_record.h"            // Raw_record
#include "sql/dd/impl/raw/raw_table.h"             // Raw_table
#include "sql/dd/impl/sdi.h"                       // sdi::store() sdi::drop()
#include "sql/dd/impl/tables/character_sets.h"     // dd::tables::Character_sets
#include "sql/dd/impl/tables/collations.h"         // dd::tables::Collations
#include "sql/dd/impl/tables/column_statistics.h"  // dd::tables::Column_stat...
#include "sql/dd/impl/tables/events.h"             // dd::tables::Events
#include "sql/dd/impl/tables/index_stats.h"        // dd::tables::Index_stats
#include "sql/dd/impl/tables/resource_groups.h"  // dd::tables::Resource_groups
#include "sql/dd/impl/tables/routines.h"         // dd::tables::Routines
#include "sql/dd/impl/tables/schemata.h"         // dd::tables::Schemata
#include "sql/dd/impl/tables/spatial_reference_systems.h"  // dd::tables::Spatial...
#include "sql/dd/impl/tables/table_stats.h"  // dd::tables::Table_stats
#include "sql/dd/impl/tables/tables.h"       // dd::tables::Tables
#include "sql/dd/impl/tables/tablespaces.h"  // dd::tables::Tablespaces
#include "sql/dd/impl/transaction_impl.h"    // Transaction_ro
#include "sql/dd/impl/types/entity_object_impl.h"
#include "sql/dd/types/abstract_table.h"            // Abstract_table
#include "sql/dd/types/charset.h"                   // Charset
#include "sql/dd/types/collation.h"                 // Collation
#include "sql/dd/types/column_statistics.h"         // Column_statistics
#include "sql/dd/types/entity_object_table.h"       // Entity_object_table
#include "sql/dd/types/event.h"                     // Event
#include "sql/dd/types/function.h"                  // Routine, Function
#include "sql/dd/types/index_stat.h"                // Index_stat
#include "sql/dd/types/procedure.h"                 // Procedure
#include "sql/dd/types/schema.h"                    // Schema
#include "sql/dd/types/spatial_reference_system.h"  // Spatial_reference_system
#include "sql/dd/types/table.h"                     // Table
#include "sql/dd/types/table_stat.h"                // Table_stat
#include "sql/dd/types/tablespace.h"                // Tablespace
#include "sql/dd/types/view.h"                      // View
#include "sql/dd/upgrade_57/upgrade.h"              // allow_sdi_creation
#include "sql/debug_sync.h"                         // DEBUG_SYNC
#include "sql/error_handler.h"                      // Internal_error_handler
#include "sql/log.h"
#include "sql/sql_class.h"  // THD

namespace dd {
namespace cache {

Storage_adapter *Storage_adapter::instance() {
  static Storage_adapter s_instance;
  return &s_instance;
}

bool Storage_adapter::s_use_fake_storage = false;

// Generate a new object id for a registry partition.
template <typename T>
Object_id Storage_adapter::next_oid() {
  static Object_id next_oid = FIRST_OID;
  return next_oid++;
}

// Get the number of core objects in a registry partition.
template <typename T>
size_t Storage_adapter::core_size() {
  MUTEX_LOCK(lock, &m_lock);
  return m_core_registry.size<typename T::Cache_partition>();
}

// Get a dictionary object id from core storage.
template <typename T>
Object_id Storage_adapter::core_get_id(const typename T::Name_key &key) {
  Cache_element<typename T::Cache_partition> *element = nullptr;
  MUTEX_LOCK(lock, &m_lock);
  m_core_registry.get(key, &element);
  if (element) {
    DBUG_ASSERT(element->object());
    return element->object()->id();
  }
  return INVALID_OBJECT_ID;
}

// Get a dictionary object from core storage.
template <typename K, typename T>
void Storage_adapter::core_get(const K &key, const T **object) {
  DBUG_ASSERT(object);
  *object = nullptr;
  Cache_element<typename T::Cache_partition> *element = nullptr;
  MUTEX_LOCK(lock, &m_lock);
  m_core_registry.get(key, &element);
  if (element) {
    // Must clone the object here, otherwise evicting the object from
    // the shared cache will also make it vanish from the core storage.
    *object = dynamic_cast<const T *>(element->object())->clone();
  }
}

// Update the dictionary object for a dd entity in the core registry.
void Storage_adapter::core_update(const dd::Tablespace *new_tsp) {
  if (new_tsp->id() != MYSQL_TABLESPACE_DD_ID) {
    return;
  }

  Cache_element<typename dd::Tablespace::Cache_partition> *element = nullptr;
  typename dd::Tablespace::Id_key key(new_tsp->id());
  MUTEX_LOCK(lock, &m_lock);
  m_core_registry.get(key, &element);
  DBUG_ASSERT(element != nullptr);
  m_core_registry.remove(element);
  std::unique_ptr<const dd::Tablespace> old{element->object()};

  element->set_object(new_tsp->clone());
  element->recreate_keys();
  m_core_registry.put(element);
}

// Get a dictionary object from persistent storage.
template <typename K, typename T>
bool Storage_adapter::get(THD *thd, const K &key, enum_tx_isolation isolation,
                          bool bypass_core_registry, const T **object) {
  DBUG_ASSERT(object);
  *object = nullptr;

  if (!bypass_core_registry) {
    instance()->core_get(key, object);
    if (*object || s_use_fake_storage) return false;
  }

  // We may have a cache miss while checking for existing tables during
  // server start. At this stage, the object will be considered not existing.
  if (bootstrap::DD_bootstrap_ctx::instance().get_stage() <
      bootstrap::Stage::CREATED_TABLES)
    return false;

  // Start a DD transaction to get the object.
  Transaction_ro trx(thd, isolation);
  trx.otx.register_tables<T>();

  if (trx.otx.open_tables()) {
    DBUG_ASSERT(thd->is_system_thread() || thd->killed || thd->is_error());
    return true;
  }

  const Entity_object_table &table = T::DD_table::instance();
  // Get main object table.
  Raw_table *t = trx.otx.get_table(table.name());

  // Find record by the object-id.
  std::unique_ptr<Raw_record> r;
  if (t->find_record(key, r)) {
    DBUG_ASSERT(thd->is_system_thread() || thd->killed || thd->is_error());
    return true;
  }

  // Restore the object from the record.
  Entity_object *new_object = nullptr;
  if (r.get() &&
      table.restore_object_from_record(&trx.otx, *r.get(), &new_object)) {
    DBUG_ASSERT(thd->is_system_thread() || thd->killed || thd->is_error());
    return true;
  }

  // Delete the new object if dynamic cast fails.
  if (new_object) {
    // Here, a failing dynamic cast is not a legitimate situation.
    // In production, we report an error.
    *object = dynamic_cast<T *>(new_object);
    if (!*object) {
      /* purecov: begin inspected */
      my_error(ER_INVALID_DD_OBJECT, MYF(0), new_object->name().c_str());
      delete new_object;
      DBUG_ASSERT(false);
      return true;
      /* purecov: end */
    }
  }

  return false;
}

// Drop a dictionary object from core storage.
template <typename T>
void Storage_adapter::core_drop(THD *thd MY_ATTRIBUTE((unused)),
                                const T *object) {
  DBUG_ASSERT(s_use_fake_storage || thd->is_dd_system_thread());
  DBUG_ASSERT(bootstrap::DD_bootstrap_ctx::instance().get_stage() <=
              bootstrap::Stage::CREATED_TABLES);
  Cache_element<typename T::Cache_partition> *element = nullptr;
  MUTEX_LOCK(lock, &m_lock);

  // For unit tests, drop based on id to simulate behavior of persistent tables.
  // For storing core objects during bootstrap, drop based on names since id may
  // differ between scaffolding objects and persisted objects.
  if (s_use_fake_storage) {
    typename T::Id_key key;
    object->update_id_key(&key);
    m_core_registry.get(key, &element);
  } else {
    typename T::Name_key key;
    object->update_name_key(&key);
    m_core_registry.get(key, &element);
  }
  if (element) {
    m_core_registry.remove(element);
    delete element->object();
    delete element;
  }
}

// Drop a dictionary object from persistent storage.
template <typename T>
bool Storage_adapter::drop(THD *thd, const T *object) {
  if (s_use_fake_storage ||
      bootstrap::DD_bootstrap_ctx::instance().get_stage() <
          bootstrap::Stage::CREATED_TABLES) {
    instance()->core_drop(thd, object);
    return false;
  }

  if (object->impl()->validate()) {
    DBUG_ASSERT(thd->is_system_thread() || thd->killed || thd->is_error());
    return true;
  }

  if (sdi::drop(thd, object)) {
    return true;
  }

  // Drop the object from the dd tables. We need to switch transaction ctx to do
  // this.
  Update_dictionary_tables_ctx ctx(thd);
  ctx.otx.register_tables<T>();

  if (ctx.otx.open_tables() || object->impl()->drop(&ctx.otx)) {
    DBUG_ASSERT(thd->is_system_thread() || thd->killed || thd->is_error());
    return true;
  }

  return false;
}

// Store a dictionary object to core storage.
template <typename T>
void Storage_adapter::core_store(THD *thd, T *object) {
  DBUG_ASSERT(s_use_fake_storage || thd->is_dd_system_thread());
  DBUG_ASSERT(bootstrap::DD_bootstrap_ctx::instance().get_stage() <=
              bootstrap::Stage::CREATED_TABLES);
  Cache_element<typename T::Cache_partition> *element =
      new Cache_element<typename T::Cache_partition>();

  if (object->id() != INVALID_OBJECT_ID) {
    // For unit tests, drop old object (based on id) to simulate update.
    if (s_use_fake_storage) core_drop(thd, object);
  } else {
    dynamic_cast<dd::Entity_object_impl *>(object)->set_id(next_oid<T>());
  }

  // Need to clone since core registry takes ownership
  element->set_object(object->clone());
  element->recreate_keys();
  MUTEX_LOCK(lock, &m_lock);
  m_core_registry.put(element);
}

// Re-map error messages emitted during DDL with innodb-read-only == 1.
class Open_dictionary_tables_error_handler : public Internal_error_handler {
 public:
  virtual bool handle_condition(THD *, uint sql_errno, const char *,
                                Sql_condition::enum_severity_level *,
                                const char *) {
    if (sql_errno == ER_OPEN_AS_READONLY) {
      my_error(ER_READ_ONLY_MODE, MYF(0));
      return true;
    }
    return false;
  }
};

// Store a dictionary object to persistent storage.
template <typename T>
bool Storage_adapter::store(THD *thd, T *object) {
  if (s_use_fake_storage ||
      bootstrap::DD_bootstrap_ctx::instance().get_stage() <
          bootstrap::Stage::CREATED_TABLES) {
    instance()->core_store(thd, object);
    return false;
  }

  if (object->impl()->validate()) {
    DBUG_ASSERT(thd->is_system_thread() || thd->killed || thd->is_error());
    return true;
  }

  // Store the object into the dd tables. We need to switch transaction
  // ctx to do this.
  Update_dictionary_tables_ctx ctx(thd);
  ctx.otx.register_tables<T>();
  DEBUG_SYNC(thd, "before_storing_dd_object");

  Open_dictionary_tables_error_handler error_handler;
  thd->push_internal_handler(&error_handler);
  if (ctx.otx.open_tables() || object->impl()->store(&ctx.otx)) {
    DBUG_ASSERT(thd->is_system_thread() || thd->killed || thd->is_error());
    thd->pop_internal_handler();
    return true;
  }
  thd->pop_internal_handler();

  // Do not create SDIs for tablespaces and tables while creating
  // dictionary entry during upgrade.
  if (bootstrap::DD_bootstrap_ctx::instance().get_stage() >
          bootstrap::Stage::CREATED_TABLES &&
      dd::upgrade_57::allow_sdi_creation() && sdi::store(thd, object))
    return true;

  return false;
}

// Sync a dictionary object from persistent to core storage.
template <typename T>
bool Storage_adapter::core_sync(THD *thd, const typename T::Name_key &key,
                                const T *object) {
  DBUG_ASSERT(thd->is_dd_system_thread());
  DBUG_ASSERT(bootstrap::DD_bootstrap_ctx::instance().get_stage() <=
              bootstrap::Stage::CREATED_TABLES);

  // Copy the name, needed for error output. The object has to be
  // dropped before get().
  String_type name(object->name());
  core_drop(thd, object);
  const typename T::Cache_partition *new_obj = nullptr;

  /*
    Fetch the object from persistent tables. The object was dropped
    from the core registry above, so we know get() will fetch it
    from the tables.

    There is a theoretical possibility of get() failing or sending
    back a nullptr if there has been a corruption or wrong usage
    (e.g. dropping a DD table), leaving one or more DD tables
    inaccessible. Assume, e.g., that the 'mysql.tables' table has
    been dropped. Then, the following will happen during restart:

    1. After creating the scaffolding, the meta data representing
       the DD tables is kept in the shared cache, secured by a
       scoped auto releaser in 'sync_meta_data()' in the bootstrapper
       (this is to make sure the meta data is not evicted during
       synchronization).
    2. We sync the DD tables, starting with 'mysql.character_sets'
       (because it is the first entry in the System_table_registry).
    3. Here in core_sync(), the entry in the core registry is
       removed. Then, we call get(), which will read the meta data
       from the persistent DD tables.
    4. While trying to fetch the meta data for the first table to
       be synced (i.e., 'mysql.character_sets'), we first open
       the tables that are needed to read the meta data for a table
       (i.e., we open the core tables). One of these tables is the
       'mysql.tables' table.
    5. While opening these tables, the server will fetch the meta
       data for them. The meta data for 'mysql.tables' is indeed
       found (because it was created as part of the scaffolding
       with the meta data now being in the shared cache), however,
       when opening the table in the storage engine, we get a
       failure because the SE knows nothing about this table, and
       is unable to open it.
  */
  if (get(thd, key, ISO_READ_COMMITTED, false, &new_obj) ||
      new_obj == nullptr) {
    LogErr(ERROR_LEVEL, ER_DD_METADATA_NOT_FOUND, name.c_str());
    return true;
  }

  Cache_element<typename T::Cache_partition> *element =
      new Cache_element<typename T::Cache_partition>();
  element->set_object(new_obj);
  element->recreate_keys();
  MUTEX_LOCK(lock, &m_lock);
  m_core_registry.put(element);
  return false;
}

// Remove and delete all elements and objects from core storage.
void Storage_adapter::erase_all() {
  MUTEX_LOCK(lock, &m_lock);
  instance()->m_core_registry.erase_all();
}

// Dump the contents of the core storage.
void Storage_adapter::dump() {
#ifndef DBUG_OFF
  MUTEX_LOCK(lock, &m_lock);
  fprintf(stderr, "================================\n");
  fprintf(stderr, "Storage adapter\n");
  m_core_registry.dump<dd::Tablespace>();
  m_core_registry.dump<dd::Schema>();
  m_core_registry.dump<dd::Abstract_table>();
  fprintf(stderr, "================================\n");
#endif
}

// Explicitly instantiate the type for the various usages.
template bool Storage_adapter::core_sync(THD *, const Table::Name_key &,
                                         const Table *);
template bool Storage_adapter::core_sync(THD *, const Tablespace::Name_key &,
                                         const Tablespace *);
template bool Storage_adapter::core_sync(THD *, const Schema::Name_key &,
                                         const Schema *);

template Object_id Storage_adapter::core_get_id<Table>(const Table::Name_key &);
template Object_id Storage_adapter::core_get_id<Schema>(
    const Schema::Name_key &);
template Object_id Storage_adapter::core_get_id<Tablespace>(
    const Tablespace::Name_key &);

template void Storage_adapter::core_get(dd::Item_name_key const &,
                                        const dd::Schema **);
template void Storage_adapter::core_get<dd::Item_name_key, dd::Abstract_table>(
    dd::Item_name_key const &, const dd::Abstract_table **);
template void Storage_adapter::core_get<dd::Global_name_key, dd::Tablespace>(
    dd::Global_name_key const &, const dd::Tablespace **);

template Object_id Storage_adapter::next_oid<Abstract_table>();
template Object_id Storage_adapter::next_oid<Table>();
template Object_id Storage_adapter::next_oid<View>();
template Object_id Storage_adapter::next_oid<Charset>();
template Object_id Storage_adapter::next_oid<Collation>();
template Object_id Storage_adapter::next_oid<Column_statistics>();
template Object_id Storage_adapter::next_oid<Event>();
template Object_id Storage_adapter::next_oid<Routine>();
template Object_id Storage_adapter::next_oid<Function>();
template Object_id Storage_adapter::next_oid<Procedure>();
template Object_id Storage_adapter::next_oid<Schema>();
template Object_id Storage_adapter::next_oid<Spatial_reference_system>();
template Object_id Storage_adapter::next_oid<Tablespace>();

template size_t Storage_adapter::core_size<Abstract_table>();
template size_t Storage_adapter::core_size<Table>();
template size_t Storage_adapter::core_size<Schema>();
template size_t Storage_adapter::core_size<Tablespace>();

template bool Storage_adapter::get<Abstract_table::Id_key, Abstract_table>(
    THD *, const Abstract_table::Id_key &, enum_tx_isolation, bool,
    const Abstract_table **);
template bool Storage_adapter::get<Abstract_table::Name_key, Abstract_table>(
    THD *, const Abstract_table::Name_key &, enum_tx_isolation, bool,
    const Abstract_table **);
template bool Storage_adapter::get<Abstract_table::Aux_key, Abstract_table>(
    THD *, const Abstract_table::Aux_key &, enum_tx_isolation, bool,
    const Abstract_table **);
template bool Storage_adapter::drop(THD *, const Abstract_table *);
template bool Storage_adapter::store(THD *, Abstract_table *);
template bool Storage_adapter::drop(THD *, const Table *);
template bool Storage_adapter::store(THD *, Table *);
template bool Storage_adapter::drop(THD *, const View *);
template bool Storage_adapter::store(THD *, View *);

template bool Storage_adapter::get<Charset::Id_key, Charset>(
    THD *, const Charset::Id_key &, enum_tx_isolation, bool, const Charset **);
template bool Storage_adapter::get<Charset::Name_key, Charset>(
    THD *, const Charset::Name_key &, enum_tx_isolation, bool,
    const Charset **);
template bool Storage_adapter::get<Charset::Aux_key, Charset>(
    THD *, const Charset::Aux_key &, enum_tx_isolation, bool, const Charset **);
template bool Storage_adapter::drop(THD *, const Charset *);
template bool Storage_adapter::store(THD *, Charset *);

template bool Storage_adapter::get<Collation::Id_key, Collation>(
    THD *, const Collation::Id_key &, enum_tx_isolation, bool,
    const Collation **);
template bool Storage_adapter::get<Collation::Name_key, Collation>(
    THD *, const Collation::Name_key &, enum_tx_isolation, bool,
    const Collation **);
template bool Storage_adapter::get<Collation::Aux_key, Collation>(
    THD *, const Collation::Aux_key &, enum_tx_isolation, bool,
    const Collation **);
template bool Storage_adapter::drop(THD *, const Collation *);
template bool Storage_adapter::store(THD *, Collation *);

template bool
Storage_adapter::get<Column_statistics::Id_key, Column_statistics>(
    THD *, const Column_statistics::Id_key &, enum_tx_isolation, bool,
    const Column_statistics **);
template bool
Storage_adapter::get<Column_statistics::Name_key, Column_statistics>(
    THD *, const Column_statistics::Name_key &, enum_tx_isolation, bool,
    const Column_statistics **);
template bool
Storage_adapter::get<Column_statistics::Aux_key, Column_statistics>(
    THD *, const Column_statistics::Aux_key &, enum_tx_isolation, bool,
    const Column_statistics **);
template bool Storage_adapter::drop(THD *, const Column_statistics *);
template bool Storage_adapter::store(THD *, Column_statistics *);

template bool Storage_adapter::get<Event::Id_key, Event>(THD *,
                                                         const Event::Id_key &,
                                                         enum_tx_isolation,
                                                         bool, const Event **);
template bool Storage_adapter::get<Event::Name_key, Event>(
    THD *, const Event::Name_key &, enum_tx_isolation, bool, const Event **);
template bool Storage_adapter::get<Event::Aux_key, Event>(
    THD *, const Event::Aux_key &, enum_tx_isolation, bool, const Event **);
template bool Storage_adapter::drop(THD *, const Event *);
template bool Storage_adapter::store(THD *, Event *);

template bool Storage_adapter::get<Resource_group::Id_key, Resource_group>(
    THD *, const Resource_group::Id_key &, enum_tx_isolation, bool,
    const Resource_group **);
template bool Storage_adapter::get<Resource_group::Name_key, Resource_group>(
    THD *, const Resource_group::Name_key &, enum_tx_isolation, bool,
    const Resource_group **);
template bool Storage_adapter::get<Resource_group::Aux_key, Resource_group>(
    THD *, const Resource_group::Aux_key &, enum_tx_isolation, bool,
    const Resource_group **);
template bool Storage_adapter::drop(THD *, const Resource_group *);
template bool Storage_adapter::store(THD *, Resource_group *);

template bool Storage_adapter::get<Routine::Id_key, Routine>(
    THD *, const Routine::Id_key &, enum_tx_isolation, bool, const Routine **);
template bool Storage_adapter::get<Routine::Name_key, Routine>(
    THD *, const Routine::Name_key &, enum_tx_isolation, bool,
    const Routine **);
template bool Storage_adapter::get<Routine::Aux_key, Routine>(
    THD *, const Routine::Aux_key &, enum_tx_isolation, bool, const Routine **);
template bool Storage_adapter::drop(THD *, const Routine *);
template bool Storage_adapter::store(THD *, Routine *);
template bool Storage_adapter::drop(THD *, const Function *);
template bool Storage_adapter::store(THD *, Function *);
template bool Storage_adapter::drop(THD *, const Procedure *);
template bool Storage_adapter::store(THD *, Procedure *);

template bool Storage_adapter::get<Schema::Id_key, Schema>(
    THD *, const Schema::Id_key &, enum_tx_isolation, bool, const Schema **);
template bool Storage_adapter::get<Schema::Name_key, Schema>(
    THD *, const Schema::Name_key &, enum_tx_isolation, bool, const Schema **);
template bool Storage_adapter::get<Schema::Aux_key, Schema>(
    THD *, const Schema::Aux_key &, enum_tx_isolation, bool, const Schema **);
template bool Storage_adapter::drop(THD *, const Schema *);
template bool Storage_adapter::store(THD *, Schema *);

template bool Storage_adapter::get<Spatial_reference_system::Id_key,
                                   Spatial_reference_system>(
    THD *, const Spatial_reference_system::Id_key &, enum_tx_isolation, bool,
    const Spatial_reference_system **);
template bool Storage_adapter::get<Spatial_reference_system::Name_key,
                                   Spatial_reference_system>(
    THD *, const Spatial_reference_system::Name_key &, enum_tx_isolation, bool,
    const Spatial_reference_system **);
template bool Storage_adapter::get<Spatial_reference_system::Aux_key,
                                   Spatial_reference_system>(
    THD *, const Spatial_reference_system::Aux_key &, enum_tx_isolation, bool,
    const Spatial_reference_system **);
template bool Storage_adapter::drop(THD *, const Spatial_reference_system *);
template bool Storage_adapter::store(THD *, Spatial_reference_system *);

template bool Storage_adapter::get<Tablespace::Id_key, Tablespace>(
    THD *, const Tablespace::Id_key &, enum_tx_isolation, bool,
    const Tablespace **);
template bool Storage_adapter::get<Tablespace::Name_key, Tablespace>(
    THD *, const Tablespace::Name_key &, enum_tx_isolation, bool,
    const Tablespace **);
template bool Storage_adapter::get<Tablespace::Aux_key, Tablespace>(
    THD *, const Tablespace::Aux_key &, enum_tx_isolation, bool,
    const Tablespace **);
template bool Storage_adapter::drop(THD *, const Tablespace *);
template bool Storage_adapter::store(THD *, Tablespace *);

/*
  DD objects dd::Table_stat and dd::Index_stat are not cached,
  because these objects are only updated and never read by DD
  API's. Information schema system views use these DD tables
  to project table/index statistics. As these objects are
  not in DD cache, it cannot make it to core storage.
*/

template <>
void Storage_adapter::core_get(const Table_stat::Name_key &,
                               const Table_stat **) {}

template <>
void Storage_adapter::core_get(const Index_stat::Name_key &,
                               const Index_stat **) {}

template <>
void Storage_adapter::core_drop(THD *, const Table_stat *) {}

template <>
void Storage_adapter::core_drop(THD *, const Index_stat *) {}

template <>
void Storage_adapter::core_store(THD *, Table_stat *) {}

template <>
void Storage_adapter::core_store(THD *, Index_stat *) {}

template bool Storage_adapter::get<Table_stat::Name_key, Table_stat>(
    THD *, const Table_stat::Name_key &, enum_tx_isolation, bool,
    const Table_stat **);
template bool Storage_adapter::store(THD *, Table_stat *);
template bool Storage_adapter::drop(THD *, const Table_stat *);
template bool Storage_adapter::get<Index_stat::Name_key, Index_stat>(
    THD *, const Index_stat::Name_key &, enum_tx_isolation, bool,
    const Index_stat **);
template bool Storage_adapter::store(THD *, Index_stat *);
template bool Storage_adapter::drop(THD *, const Index_stat *);

// Doxygen doesn't understand this explicit template instantiation.
#ifndef IN_DOXYGEN
template void Storage_adapter::core_drop<Schema>(THD *, const Schema *);
template void Storage_adapter::core_drop<Table>(THD *, const Table *);
template void Storage_adapter::core_drop<Tablespace>(THD *, const Tablespace *);

template void Storage_adapter::core_store<Schema>(THD *, Schema *);
template void Storage_adapter::core_store<Table>(THD *, Table *);
template void Storage_adapter::core_store<Tablespace>(THD *, Tablespace *);
#endif

}  // namespace cache
}  // namespace dd
