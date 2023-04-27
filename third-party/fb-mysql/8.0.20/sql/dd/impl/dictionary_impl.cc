/* Copyright (c) 2014, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/impl/dictionary_impl.h"

#include <string.h>
#include <memory>

#include "m_ctype.h"
#include "m_string.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysql/thread_type.h"
#include "mysql/udf_registration_types.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "sql/auth/auth_common.h"  // acl_init
#include "sql/auth/sql_security_ctx.h"
#include "sql/auto_thd.h"                        // Auto_thd
#include "sql/bootstrap.h"                       // bootstrap::bootstrap_functor
#include "sql/dd/cache/dictionary_client.h"      // dd::Dictionary_client
#include "sql/dd/dd.h"                           // enum_dd_init_type
#include "sql/dd/dd_schema.h"                    // dd::Schema_MDL_locker
#include "sql/dd/dd_version.h"                   // dd::DD_VERSION
#include "sql/dd/impl/bootstrap/bootstrapper.h"  // dd::Bootstrapper
#include "sql/dd/impl/cache/shared_dictionary_cache.h"  // Shared_dictionary_cache
#include "sql/dd/impl/system_registry.h"                // dd::System_tables
#include "sql/dd/impl/tables/columns.h"                 // dd::tables::Columns
#include "sql/dd/impl/tables/dd_properties.h"     // get_actual_dd_version()
#include "sql/dd/impl/tables/indexes.h"           // dd::tables::Indexes
#include "sql/dd/impl/tables/table_partitions.h"  // dd::tables::Table_partitions
#include "sql/dd/impl/tables/tables.h"            // dd::tables::Tables
#include "sql/dd/impl/tables/tablespaces.h"       // dd::tables::Tablespaces
#include "sql/dd/impl/utils.h"                    // dd::tables::Tablespaces
#include "sql/dd/info_schema/metadata.h"  // dd::info_schema::store_dynamic...
#include "sql/dd/types/abstract_table.h"  // dd::Abstract_table::DD_table
#include "sql/dd/types/column.h"          // dd::Column::DD_table
#include "sql/dd/types/index.h"           // dd::Index::DD_table
#include "sql/dd/types/object_table_definition.h"
#include "sql/dd/types/partition.h"  // dd::Partition::DD_table
#include "sql/dd/types/system_view.h"
#include "sql/dd/types/table.h"         // dd::Table::DD_table
#include "sql/dd/types/tablespace.h"    // dd::Tablespace::DD_table
#include "sql/dd/upgrade_57/upgrade.h"  // dd::upgrade
#include "sql/derror.h"
#include "sql/handler.h"
#include "sql/mdl.h"
#include "sql/opt_costconstantcache.h"  // init_optimizer_cost_module
#include "sql/plugin_table.h"
#include "sql/sql_base.h"   // close_cached_tables
#include "sql/sql_class.h"  // THD
#include "sql/system_variables.h"
#include "sql/thd_raii.h"                       // Disable_autocommit_guard
#include "sql/transaction.h"                    // trans_commit()
#include "storage/perfschema/pfs_dd_version.h"  // PFS_DD_VERSION

extern Cost_constant_cache *cost_constant_cache;  // defined in
                                                  // opt_costconstantcache.cc

///////////////////////////////////////////////////////////////////////////

namespace dd {

///////////////////////////////////////////////////////////////////////////
// Implementation details.
///////////////////////////////////////////////////////////////////////////

class Object_table;
class Table;

Dictionary_impl *Dictionary_impl::s_instance = nullptr;

Dictionary_impl *Dictionary_impl::instance() { return s_instance; }

Object_id Dictionary_impl::DEFAULT_CATALOG_ID = 1;
Object_id Dictionary_impl::DD_TABLESPACE_ID = 1;
const String_type Dictionary_impl::DEFAULT_CATALOG_NAME("def");

///////////////////////////////////////////////////////////////////////////

bool Dictionary_impl::init(enum_dd_init_type dd_init) {
  if (dd_init == enum_dd_init_type::DD_INITIALIZE ||
      dd_init == enum_dd_init_type::DD_RESTART_OR_UPGRADE) {
    DBUG_ASSERT(!Dictionary_impl::s_instance);

    if (Dictionary_impl::s_instance) return false; /* purecov: inspected */

    std::unique_ptr<Dictionary_impl> d(new Dictionary_impl());

    Dictionary_impl::s_instance = d.release();
  }

  /*
    Initialize the cost model, but delete it after the dd is initialized.
    This is because the cost model is needed for the dd initialization, but
    it must be re-initialized later after the plugins have been initialized.
    Upgrade process needs heap engine initialized, hence parameter 'true'
    is passed to the function.
  */
  bool cost_constant_inited = false;
  if (cost_constant_cache == nullptr) {
    init_optimizer_cost_module(true);
    cost_constant_inited = true;
  }

  // Disable table encryption privilege checks for system threads.
  bool saved_table_encryption_privilege_check =
      opt_table_encryption_privilege_check;
  opt_table_encryption_privilege_check = false;

  /*
    Install or start or upgrade the dictionary
    depending on bootstrapping option.
  */

  bool result = false;

  // Creation of Data Dictionary through current server
  if (dd_init == enum_dd_init_type::DD_INITIALIZE)
    result = ::bootstrap::run_bootstrap_thread(
        nullptr, nullptr, &bootstrap::initialize, SYSTEM_THREAD_DD_INITIALIZE);

  // Creation of INFORMATION_SCHEMA system views.
  else if (dd_init == enum_dd_init_type::DD_INITIALIZE_SYSTEM_VIEWS)
    result = ::bootstrap::run_bootstrap_thread(nullptr, nullptr,
                                               &dd::info_schema::initialize,
                                               SYSTEM_THREAD_DD_INITIALIZE);

  /*
    Creation of Dictionary Tables in old Data Directory
    This function also takes care of normal server restart.
  */
  else if (dd_init == enum_dd_init_type::DD_RESTART_OR_UPGRADE)
    result = ::bootstrap::run_bootstrap_thread(
        nullptr, nullptr, &upgrade_57::do_pre_checks_and_initialize_dd,
        SYSTEM_THREAD_DD_INITIALIZE);

  // Populate metadata in DD tables from old data directory and do cleanup.
  else if (dd_init == enum_dd_init_type::DD_POPULATE_UPGRADE)
    result = ::bootstrap::run_bootstrap_thread(
        nullptr, nullptr, &upgrade_57::fill_dd_and_finalize,
        SYSTEM_THREAD_DD_INITIALIZE);

  // Delete DD tables and do cleanup in case of error in upgrade
  else if (dd_init == enum_dd_init_type::DD_DELETE)
    result = ::bootstrap::run_bootstrap_thread(
        nullptr, nullptr, &upgrade_57::terminate, SYSTEM_THREAD_DD_INITIALIZE);

  // Update server and plugin I_S table metadata into DD tables.
  else if (dd_init == enum_dd_init_type::DD_UPDATE_I_S_METADATA)
    result = ::bootstrap::run_bootstrap_thread(
        nullptr, nullptr, &dd::info_schema::update_I_S_metadata,
        SYSTEM_THREAD_DD_INITIALIZE);

  // Creation of non-dd-based INFORMATION_SCHEMA system views.
  else if (dd_init ==
           enum_dd_init_type::DD_INITIALIZE_NON_DD_BASED_SYSTEM_VIEWS)
    result = ::bootstrap::run_bootstrap_thread(
        nullptr, nullptr, &dd::info_schema::init_non_dd_based_system_view,
        SYSTEM_THREAD_DD_INITIALIZE);

  // Restore the table_encryption_privilege_check.
  opt_table_encryption_privilege_check = saved_table_encryption_privilege_check;

  /* Now that the dd is initialized, delete the cost model. */
  if (cost_constant_inited) delete_optimizer_cost_module();

  return result;
}

///////////////////////////////////////////////////////////////////////////

bool Dictionary_impl::shutdown() {
  if (!Dictionary_impl::s_instance) return true;

  delete Dictionary_impl::s_instance;
  Dictionary_impl::s_instance = nullptr;

  return false;
}

///////////////////////////////////////////////////////////////////////////
// Implementation details.
///////////////////////////////////////////////////////////////////////////

uint Dictionary_impl::get_target_dd_version() { return dd::DD_VERSION; }

///////////////////////////////////////////////////////////////////////////

uint Dictionary_impl::get_actual_dd_version(THD *thd) {
  bool exists = false;
  uint version = 0;
  bool error MY_ATTRIBUTE((unused)) = tables::DD_properties::instance().get(
      thd, "DD_VERSION", &version, &exists);
  DBUG_ASSERT(!error);
  DBUG_ASSERT(exists);
  return version;
}

///////////////////////////////////////////////////////////////////////////

uint Dictionary_impl::get_target_I_S_version() {
  return dd::info_schema::IS_DD_VERSION;
}

///////////////////////////////////////////////////////////////////////////

uint Dictionary_impl::get_actual_I_S_version(THD *thd) {
  bool exists = false;
  uint version = 0;
  bool error MY_ATTRIBUTE((unused)) = tables::DD_properties::instance().get(
      thd, "IS_VERSION", &version, &exists);
  DBUG_ASSERT(!error);
  DBUG_ASSERT(exists);
  return version;
}

///////////////////////////////////////////////////////////////////////////

uint Dictionary_impl::set_I_S_version(THD *thd, uint version) {
  return tables::DD_properties::instance().set(thd, "IS_VERSION", version);
}

///////////////////////////////////////////////////////////////////////////

uint Dictionary_impl::get_target_P_S_version() { return PFS_DD_VERSION; }

///////////////////////////////////////////////////////////////////////////

uint Dictionary_impl::get_actual_P_S_version(THD *thd) {
  bool exists = false;
  uint version = 0;
  bool error MY_ATTRIBUTE((unused)) = tables::DD_properties::instance().get(
      thd, "PS_VERSION", &version, &exists);
  DBUG_ASSERT(!error);
  DBUG_ASSERT(exists);
  return version;
}

///////////////////////////////////////////////////////////////////////////

bool Dictionary_impl::get_actual_ndbinfo_schema_version(THD *thd, uint *ver) {
  bool exists = false;
  tables::DD_properties::instance().get(thd, "NDBINFO_VERSION", ver, &exists);
  return exists;
}

///////////////////////////////////////////////////////////////////////////

uint Dictionary_impl::set_ndbinfo_schema_version(THD *thd, uint version) {
  return tables::DD_properties::instance().set(thd, "NDBINFO_VERSION", version);
}

///////////////////////////////////////////////////////////////////////////

uint Dictionary_impl::set_P_S_version(THD *thd, uint version) {
  return tables::DD_properties::instance().set(thd, "PS_VERSION", version);
}

///////////////////////////////////////////////////////////////////////////

const Object_table *Dictionary_impl::get_dd_table(
    const String_type &schema_name, const String_type &table_name) const {
  if (!is_dd_schema_name(schema_name)) return nullptr;

  return System_tables::instance()->find_table(schema_name, table_name);
}

///////////////////////////////////////////////////////////////////////////

bool Dictionary_impl::is_dd_table_name(const String_type &schema_name,
                                       const String_type &table_name) const {
  if (!is_dd_schema_name(schema_name)) return false;

  const System_tables::Types *table_type =
      System_tables::instance()->find_type(schema_name, table_name);

  return (table_type != nullptr &&
          (*table_type == System_tables::Types::CORE ||
           *table_type == System_tables::Types::INERT ||
           *table_type == System_tables::Types::SECOND ||
           *table_type == System_tables::Types::DDSE_PRIVATE ||
           *table_type == System_tables::Types::DDSE_PROTECTED));
}

///////////////////////////////////////////////////////////////////////////

bool Dictionary_impl::is_system_table_name(
    const String_type &schema_name, const String_type &table_name) const {
  if (!is_dd_schema_name(schema_name)) return false;

  const System_tables::Types *table_type =
      System_tables::instance()->find_type(schema_name, table_name);

  return (table_type != nullptr &&
          (*table_type == System_tables::Types::SYSTEM));
}

///////////////////////////////////////////////////////////////////////////

int Dictionary_impl::table_type_error_code(
    const String_type &schema_name, const String_type &table_name) const {
  const System_tables::Types *type =
      System_tables::instance()->find_type(schema_name, table_name);
  if (type != nullptr) return System_tables::type_name_error_code(*type);
  return ER_NO_SYSTEM_TABLE_ACCESS_FOR_TABLE;
}

///////////////////////////////////////////////////////////////////////////

bool Dictionary_impl::is_dd_table_access_allowed(bool is_dd_internal_thread,
                                                 bool is_ddl_statement,
                                                 const char *schema_name,
                                                 size_t schema_length,
                                                 const char *table_name) const {
  /*
    From WL#6391, we have the following matrix describing access:

    ---------+---------------------+
             | Dictionary internal |
    ---------+----------+----------+
             |   DDL    |   DML    |
    ---------+-----+----+-----+----+
             | IN  | EX | IN  | EX |
    ---------+-----+----+-----+----+
    Inert    |  X          X       |
    Core     |  X          X       |
    Second   |  X          X       |
    DDSE_priv|  X          X       |
    DDSE_prot|  X          X    X  |
    SYSTEM   |  X    X     X    X  |
    ---------+---------------------+

    For performance reasons, we first check the schema
    name to shortcut the evaluation. If the table is not in
    the 'mysql' schema, we don't need any further checks. Same for
    checking for internal threads - an internal thread has full
    access. We also allow access if the appropriate debug flag
    is set.
  */
  if (schema_length != MYSQL_SCHEMA_NAME.length ||
      strncmp(schema_name, MYSQL_SCHEMA_NAME.str, MYSQL_SCHEMA_NAME.length) ||
      is_dd_internal_thread ||
      DBUG_EVALUATE_IF("skip_dd_table_access_check", true, false))
    return true;

  // Now we need to get the table type.
  const String_type schema_str(schema_name);
  const String_type table_str(table_name);
  const System_tables::Types *table_type =
      System_tables::instance()->find_type(schema_str, table_str);

  /*
    Access allowed for external DD tables, for DML on protected DDSE tables,
    and for any operation on SYSTEM tables.
  */
  return (table_type == nullptr ||
          (*table_type == System_tables::Types::DDSE_PROTECTED &&
           !is_ddl_statement) ||
          *table_type == System_tables::Types::SYSTEM);
}

///////////////////////////////////////////////////////////////////////////

bool Dictionary_impl::is_system_view_name(const char *schema_name,
                                          const char *table_name,
                                          bool *hidden) const {
  /*
    TODO One possible improvement here could be to try and use the variant
    of is_infoschema_db() that takes length as a parameter. Then, if the
    schema name length is different, this can quickly be used to conclude
    that this is indeed not a system view, without having to do a strcmp at
    all.
  */
  if (schema_name == nullptr || table_name == nullptr ||
      is_infoschema_db(schema_name) == false)
    return false;

  // The System_views registry stores the view name in uppercase.
  // So convert the input to uppercase before search.
  char tab_name_buf[NAME_LEN + 1];
  my_stpcpy(tab_name_buf, table_name);
  my_caseup_str(system_charset_info, tab_name_buf);

  const system_views::System_view *s =
      System_views::instance()->find(INFORMATION_SCHEMA_NAME.str, tab_name_buf);

  if (s)
    *hidden = s->hidden();
  else
    *hidden = false;

  return s != nullptr;
}

///////////////////////////////////////////////////////////////////////////

/*
  Global interface methods at 'dd' namespace.
  Following are couple of API's that InnoDB needs to acquire MDL locks.
*/

static bool acquire_mdl_nsec(THD *thd,
                             MDL_key::enum_mdl_namespace lock_namespace,
                             const char *schema_name, const char *table_name,
                             bool no_wait, ulonglong lock_wait_timeout_nsec,
                             enum_mdl_type lock_type,
                             enum_mdl_duration lock_duration,
                             MDL_ticket **out_mdl_ticket) {
  DBUG_TRACE;

  MDL_request mdl_request;
  MDL_REQUEST_INIT(&mdl_request, lock_namespace, schema_name, table_name,
                   lock_type, lock_duration);

  /*
    If there is a request for an exclusive lock, we also need to acquire
    a transactional intention exclusive backup lock and global read lock
    (this is not done to get a lock, but rather to protect against others
    setting the backup- or global read lock).
  */
  MDL_request_list mdl_requests;
  mdl_requests.push_front(&mdl_request);

  MDL_request *grl_request = nullptr;
  MDL_request *bl_request = nullptr;
  if (lock_type == MDL_EXCLUSIVE) {
    // If we cannot acquire protection against GRL, err out.
    if (thd->global_read_lock.can_acquire_protection()) return true;

    grl_request = new (thd->mem_root) MDL_request;
    MDL_REQUEST_INIT(grl_request, MDL_key::GLOBAL, "", "",
                     MDL_INTENTION_EXCLUSIVE, MDL_TRANSACTION);
    mdl_requests.push_front(grl_request);

    bl_request = new (thd->mem_root) MDL_request;
    MDL_REQUEST_INIT(bl_request, MDL_key::BACKUP_LOCK, "", "",
                     MDL_INTENTION_EXCLUSIVE, MDL_TRANSACTION);
    mdl_requests.push_front(bl_request);
  }

  /*
    With no_wait, we acquire the locks one by one. When waiting,
    we use the lock request list to get either all or none in
    the same acquisition.
  */
  if (no_wait) {
    if (thd->mdl_context.try_acquire_lock(&mdl_request) ||
        (grl_request != nullptr &&
         thd->mdl_context.try_acquire_lock(bl_request)) ||
        (bl_request != nullptr &&
         thd->mdl_context.try_acquire_lock(grl_request))) {
      return true;
    }
  } else if (thd->mdl_context.acquire_locks_nsec(&mdl_requests,
                                                 lock_wait_timeout_nsec))
    return true;

  /*
    Unlike in other places where we acquire protection against global read
    lock, the read_only state is not checked here since it is handled by
    the caller or extra steps are taken to correctly ignore it. Also checking
    read_only state can be problematic for background threads like drop table
    thread and purge thread which can be initiated on behalf of statements
    executed by replication thread where the read_only state does not apply.
  */

  if (out_mdl_ticket) *out_mdl_ticket = mdl_request.ticket;

  return false;
}

bool acquire_shared_table_mdl(THD *thd, const char *schema_name,
                              const char *table_name, bool no_wait,
                              MDL_ticket **out_mdl_ticket) {
  return acquire_mdl_nsec(thd, MDL_key::TABLE, schema_name, table_name, no_wait,
                          thd->variables.lock_wait_timeout_nsec, MDL_SHARED,
                          MDL_EXPLICIT, out_mdl_ticket);
}

bool has_shared_table_mdl(THD *thd, const char *schema_name,
                          const char *table_name) {
  return thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, schema_name, table_name, MDL_SHARED);
}

bool has_exclusive_table_mdl(THD *thd, const char *schema_name,
                             const char *table_name) {
  return thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLE, schema_name, table_name, MDL_EXCLUSIVE);
}

bool acquire_exclusive_tablespace_mdl(THD *thd, const char *tablespace_name,
                                      bool no_wait, MDL_ticket **ticket,
                                      bool for_trx) {
  enum_mdl_duration duration = (for_trx ? MDL_TRANSACTION : MDL_EXPLICIT);
  return acquire_mdl_nsec(thd, MDL_key::TABLESPACE, "", tablespace_name,
                          no_wait, thd->variables.lock_wait_timeout_nsec,
                          MDL_EXCLUSIVE, duration, ticket);
}

bool acquire_shared_tablespace_mdl(THD *thd, const char *tablespace_name,
                                   bool no_wait, MDL_ticket **ticket,
                                   bool for_trx) {
  // When requesting a tablespace name lock, we leave the schema name empty.
  enum_mdl_duration duration = (for_trx ? MDL_TRANSACTION : MDL_EXPLICIT);
  return acquire_mdl_nsec(thd, MDL_key::TABLESPACE, "", tablespace_name,
                          no_wait, thd->variables.lock_wait_timeout_nsec,
                          MDL_SHARED, duration, ticket);
}

bool has_shared_tablespace_mdl(THD *thd, const char *tablespace_name) {
  // When checking a tablespace name lock, we leave the schema name empty.
  return thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLESPACE, "", tablespace_name, MDL_SHARED);
}

bool has_exclusive_tablespace_mdl(THD *thd, const char *tablespace_name) {
  // When checking a tablespace name lock, we leave the schema name empty.
  return thd->mdl_context.owns_equal_or_stronger_lock(
      MDL_key::TABLESPACE, "", tablespace_name, MDL_EXCLUSIVE);
}

bool acquire_exclusive_table_mdl(THD *thd, const char *schema_name,
                                 const char *table_name, bool no_wait,
                                 MDL_ticket **out_mdl_ticket) {
  return acquire_mdl_nsec(thd, MDL_key::TABLE, schema_name, table_name, no_wait,
                          thd->variables.lock_wait_timeout_nsec, MDL_EXCLUSIVE,
                          MDL_TRANSACTION, out_mdl_ticket);
}

bool acquire_exclusive_table_mdl(THD *thd, const char *schema_name,
                                 const char *table_name,
                                 unsigned long int lock_wait_timeout_nsec,
                                 MDL_ticket **out_mdl_ticket) {
  return acquire_mdl_nsec(thd, MDL_key::TABLE, schema_name, table_name, false,
                          lock_wait_timeout_nsec, MDL_EXCLUSIVE,
                          MDL_TRANSACTION, out_mdl_ticket);
}

bool acquire_exclusive_schema_mdl(THD *thd, const char *schema_name,
                                  bool no_wait, MDL_ticket **out_mdl_ticket) {
  return acquire_mdl_nsec(thd, MDL_key::SCHEMA, schema_name, "", no_wait,
                          thd->variables.lock_wait_timeout_nsec, MDL_EXCLUSIVE,
                          MDL_EXPLICIT, out_mdl_ticket);
}

void release_mdl(THD *thd, MDL_ticket *mdl_ticket) {
  DBUG_TRACE;

  thd->mdl_context.release_lock(mdl_ticket);
}

/* purecov: begin deadcode */
cache::Dictionary_client *get_dd_client(THD *thd) { return thd->dd_client(); }
/* purecov: end */

bool create_native_table(THD *thd, const Plugin_table *pt) {
  if (dd::get_dictionary()->is_dd_table_name(pt->get_schema_name(),
                                             pt->get_name())) {
    my_error(ER_NO_SYSTEM_TABLE_ACCESS, MYF(0),
             ER_THD_NONCONST(thd, dd::get_dictionary()->table_type_error_code(
                                      pt->get_schema_name(), pt->get_name())),
             pt->get_schema_name(), pt->get_name());

    return true;
  }

  // Acquire MDL on new native table that we would create.
  bool error = false;
  MDL_request mdl_request;
  MDL_REQUEST_INIT(&mdl_request, MDL_key::TABLE, pt->get_schema_name(),
                   pt->get_name(), MDL_EXCLUSIVE, MDL_TRANSACTION);
  dd::Schema_MDL_locker mdl_locker(thd);
  if (mdl_locker.ensure_locked(pt->get_schema_name()) ||
      thd->mdl_context.acquire_lock_nsec(&mdl_request,
                                         thd->variables.lock_wait_timeout_nsec))
    return true;

  /*
    1. Mark that we are executing a special DDL during
    plugin initialization. This will enable DDL to not be
    committed or binlogged. The called of this API would commit
    the transaction.

    2. Remove metadata of native table if already exists. This could
    happen if server was crashed and restarted.

    3. Create native table.

    4. Undo 1.
  */
  dd::cache::Dictionary_client *client = thd->dd_client();
  const dd::Table *table_def = nullptr;
  if (client->acquire(pt->get_schema_name(), pt->get_name(), &table_def))
    return true;

  thd->mark_plugin_fake_ddl(true);
  ulong master_access = thd->security_context()->master_access();
  thd->security_context()->set_master_access(~(ulong)0);
  {
    Disable_binlog_guard guard(thd);

    // Drop the table and related dynamic statistics too.
    if (table_def) {
      error =
          client->drop(table_def) || client->remove_table_dynamic_statistics(
                                         pt->get_schema_name(), pt->get_name());
    }

    if (!error) error = dd::execute_query(thd, pt->get_ddl());
  }

  thd->security_context()->set_master_access(master_access);
  thd->mark_plugin_fake_ddl(false);

  return error;
}

// Remove metadata of native table from DD tables.
bool drop_native_table(THD *thd, const char *schema_name,
                       const char *table_name) {
  if (dd::get_dictionary()->is_dd_table_name(schema_name, table_name)) {
    my_error(ER_NO_SYSTEM_TABLE_ACCESS, MYF(0),
             ER_THD_NONCONST(thd, dd::get_dictionary()->table_type_error_code(
                                      schema_name, table_name)),
             schema_name, table_name);

    return true;
  }

  // Acquire MDL on schema and table.
  MDL_request mdl_request;
  MDL_REQUEST_INIT(&mdl_request, MDL_key::TABLE, schema_name, table_name,
                   MDL_EXCLUSIVE, MDL_TRANSACTION);
  dd::Schema_MDL_locker mdl_locker(thd);
  if (mdl_locker.ensure_locked(schema_name) ||
      thd->mdl_context.acquire_lock_nsec(&mdl_request,
                                         thd->variables.lock_wait_timeout_nsec))
    return true;

  dd::cache::Dictionary_client *client = thd->dd_client();
  const dd::Table *table_def = nullptr;
  if (client->acquire(schema_name, table_name, &table_def)) {
    // Error is reported by the dictionary subsystem.
    return true;
  }

  // Not error is reported if table is not present.
  if (!table_def) return false;

  // Drop the table and related dynamic statistics too.
  return client->drop(table_def) ||
         client->remove_table_dynamic_statistics(schema_name, table_name);
}

bool reset_tables_and_tablespaces() {
  Auto_THD thd;
  handlerton *ddse = ha_resolve_by_legacy_type(thd.thd, DB_TYPE_INNODB);

  // Acquire transactional metadata locks and evict all cached objects.
  if (dd::cache::Shared_dictionary_cache::reset_tables_and_tablespaces(thd.thd))
    return true;

  // Evict all cached objects in the DD cache in the DDSE.
  if (ddse->dict_cache_reset_tables_and_tablespaces != nullptr)
    ddse->dict_cache_reset_tables_and_tablespaces();

  bool ret =
      close_cached_tables_nsec(nullptr, nullptr, false, LONG_TIMEOUT_NSEC);

  // Release transactional metadata locks.
  thd.thd->mdl_context.release_transactional_locks();

  return ret;
}

bool commit_or_rollback_tablespace_change(THD *thd, dd::Tablespace *space,
                                          bool error,
                                          bool release_mdl_on_commit_only) {
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  Disable_autocommit_guard autocommit_guard(thd);

  if (!error && space != nullptr) {
    error = thd->dd_client()->update(space);
  }

  if (error) {
    trans_rollback_stmt(thd);
    trans_rollback(thd);
  } else {
    error = trans_commit_stmt(thd) || trans_commit(thd);
  }

  if (!error || !release_mdl_on_commit_only) {
    thd->mdl_context.release_transactional_locks();
  }
  return error;
}

template <typename Entity_object_type>
const Object_table &get_dd_table() {
  return Entity_object_type::DD_table::instance();
}

template const Object_table &get_dd_table<dd::Column>();
template const Object_table &get_dd_table<dd::Index>();
template const Object_table &get_dd_table<dd::Partition>();
template const Object_table &get_dd_table<dd::Table>();
template const Object_table &get_dd_table<dd::Tablespace>();

void rename_tablespace_mdl_hook(THD *thd, MDL_ticket *src, MDL_ticket *dst) {
  if (!thd->locked_tables_mode) {
    return;
  }
  thd->locked_tables_list.add_rename_tablespace_mdls(src, dst);
}

}  // namespace dd
