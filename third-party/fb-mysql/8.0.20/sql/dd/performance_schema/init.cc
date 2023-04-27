/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/performance_schema/init.h"

#include <sys/types.h>
#include <list>
#include <new>
#include <ostream>
#include <string>
#include <vector>

#include "lex_string.h"
#include "m_ctype.h"
#include "my_dbug.h"
#include "mysql/thread_type.h"
#include "sql/auth/sql_security_ctx.h"
#include "sql/bootstrap.h"                   // bootstrap::run_bootstrap_thread
#include "sql/dd/cache/dictionary_client.h"  // dd::cache::Dictionary_client
#include "sql/dd/dd.h"                       // enum_dd_init_type
#include "sql/dd/dd_schema.h"                // dd::schema_exists
#include "sql/dd/dd_table.h"                 // dd::table_exists
#include "sql/dd/dd_utility.h"               // check_if_server_ddse_readonly
#include "sql/dd/impl/dictionary_impl.h"     // dd::Dictionary_impl
#include "sql/dd/impl/system_registry.h"     // dd::System_tables
#include "sql/dd/impl/tables/dd_properties.h"  // dd::tables::UNKNOWN_P_S_VERSION
#include "sql/dd/impl/utils.h"                 // execute_query
#include "sql/dd/properties.h"                 // dd::Properties
#include "sql/dd/string_type.h"
#include "sql/dd/types/object_table.h"
#include "sql/dd/types/object_table_definition.h"
#include "sql/dd/types/table.h"
#include "sql/derror.h"
#include "sql/handler.h"
#include "sql/plugin_table.h"
#include "sql/set_var.h"
#include "sql/sql_class.h"  // THD
#include "sql/sql_list.h"
#include "sql/stateless_allocator.h"
#include "sql/system_variables.h"
#include "sql/table.h"
#include "sql/thd_raii.h"

namespace dd {
class Schema;
}  // namespace dd

using namespace dd;

namespace {

/**
  Create and use the performance schema.

  @param thd    Thread context.

  @return       Upon failure, return true, otherwise false.
*/

bool create_and_use_pfs_schema(THD *thd) {
  return dd::performance_schema::create_pfs_schema(thd) ||
         dd::execute_query(thd,
                           dd::String_type("USE ") +
                               dd::String_type(PERFORMANCE_SCHEMA_DB_NAME.str));
}

/**
  Check that the actual version of performance schema supported by server
  and the version of performance schema stored in the Data Dictionary
  are the same.

  @param thd    Thread context.

  @return       true in case versions are matched, else false
*/

bool check_perf_schema_has_correct_version(THD *thd) {
  dd::Dictionary_impl *d = dd::Dictionary_impl::instance();

  // Stop if P_S version is same.
  uint actual_version = d->get_actual_P_S_version(thd);

#ifndef DBUG_OFF
  // Unknown version of the current server PS schema. It is used for tests.
  const uint UNKNOWN_P_S_VERSION = -1;
#endif
  // Testing to make sure we update plugins when version changes.
  DBUG_EXECUTE_IF("test_p_s_metadata_version",
                  { actual_version = UNKNOWN_P_S_VERSION; });

  return d->get_target_P_S_version() == actual_version;
}

/**
  Produce a statement DROP TABLE for a table name specified by the argument.

  @param table_name  A name of table to drop

  @return  a string representation of a statement DROP TABLE
*/

String_type build_ddl_drop_ps_table(const String_type &table_name) {
  dd::Stringstream_type drop_stmt;
  drop_stmt << "DROP TABLE " << table_name << ';';

  return drop_stmt.str();
}

/**
  Create performance schema tables.

  @param thd             Thread context.

  @return                Upon failure, return true, otherwise false.
*/

bool create_pfs_tables(THD *thd) {
  sql_mode_t sql_mode_saved = thd->variables.sql_mode;
  thd->variables.sql_mode = 0;

  bool ret = false;
  for (System_tables::Const_iterator it =
           System_tables::instance()->begin(System_tables::Types::PFS);
       it != System_tables::instance()->end();
       it = System_tables::instance()->next(it, System_tables::Types::PFS)) {
    bool exists = false;

    if (dd::table_exists(thd->dd_client(), PERFORMANCE_SCHEMA_DB_NAME.str,
                         (*it)->entity()->name().c_str(), &exists)) {
      ret = true;
      break;
    }

    DBUG_ASSERT(!exists);

    const Object_table_definition *table_def = nullptr;
    if (exists ||
        (table_def = (*it)->entity()->target_table_definition()) == nullptr ||
        dd::execute_query(thd, table_def->get_ddl())) {
      ret = true;
      break;
    }
  }

  if (!ret) {
    dd::Dictionary_impl *d = dd::Dictionary_impl::instance();

    ret = d->set_P_S_version(thd, d->get_target_P_S_version());
  }

  ret = dd::end_transaction(thd, ret);

  thd->variables.sql_mode = sql_mode_saved;

  return ret;
}

const dd::String_type SERVER_PS_TABLE_PROPERTY_NAME("server_p_s_table");

/**
  Drop all P_S tables that have the property PS_version

  @param thd  Thread context.

  @return      Upon failure, return true, otherwise false.
*/

bool drop_old_pfs_tables(THD *thd) {
  const dd::Schema *sch_obj = nullptr;

  dd::cache::Dictionary_client::Auto_releaser auto_releaser(thd->dd_client());
  dd::Schema_MDL_locker mdl_handler(thd);

  if (mdl_handler.ensure_locked(PERFORMANCE_SCHEMA_DB_NAME.str) ||
      thd->dd_client()->acquire(PERFORMANCE_SCHEMA_DB_NAME.str, &sch_obj))
    return true;

  std::vector<const dd::Table *> pfs_tables_in_dd;
  if (thd->dd_client()->fetch_schema_components(sch_obj, &pfs_tables_in_dd))
    return true;

  if (pfs_tables_in_dd.empty()) return false;

  std::list<dd::String_type> pfs_table_names_to_drop;
  for (const dd::Table *table : pfs_tables_in_dd) {
    if (table->options().exists(SERVER_PS_TABLE_PROPERTY_NAME))
      pfs_table_names_to_drop.push_back(table->name());
  }

  bool error = false;
  for (const dd::String_type &pfs_table_name : pfs_table_names_to_drop) {
    error = dd::execute_query(thd, build_ddl_drop_ps_table(pfs_table_name));
    if (error) break;
  }

  return dd::end_transaction(thd, error);
}

/**
  Produce sql statement to create a P_S table, add a pair
  table_name/sql_statement into the list of P_S tables to create
  and register a name of P_S table in the register of known P_S tables.

  @param table                        P_S table definition
*/

void add_pfs_definition(const Plugin_table *table) {
  DBUG_EXECUTE_IF("test_p_s_metadata_version", {
    if (!my_strcasecmp(system_charset_info, table->get_name(),
                       "cond_instances"))
      return;
  });

  Object_table_impl *plugin_table = new (std::nothrow) Object_table_impl(
      table->get_schema_name(), table->get_name(), table->get_ddl());
  System_tables::instance()->add(PERFORMANCE_SCHEMA_DB_NAME.str,
                                 table->get_name(), System_tables::Types::PFS,
                                 plugin_table);
}

/**
  Creates the database performance_schema and
  tables that the perf_schema consists of.

  @param thd    Thread context.

  @return       Upon failure, return true, otherwise false.
*/

bool initialize_pfs(THD *thd) {
  /*
    Set tx_read_only to false to allow installing DD tables even
    if the server is started with --transaction-read-only=true.
  */
  thd->variables.transaction_read_only = false;
  thd->tx_read_only = false;

  Disable_autocommit_guard autocommit_guard(thd);

  cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());

  if (thd->system_thread == SYSTEM_THREAD_DD_RESTART &&
      check_perf_schema_has_correct_version(thd))
    return false;

  /*
    Stop server restart if P_S version is changed and the server is
    started with DDSE in read-only mode.
  */
  if (dd::check_if_server_ddse_readonly(thd, PERFORMANCE_SCHEMA_DB_NAME.str))
    return true;

  handlerton *pfs_se =
      ha_resolve_by_legacy_type(thd, DB_TYPE_PERFORMANCE_SCHEMA);

  /*
    The lists with element wrappers are mem root allocated.
    The wrapped instances are static allocated.
  */
  List<const Plugin_table> pfs_server_tables;
  if (pfs_se->dict_init == nullptr ||
      pfs_se->dict_init(
          DICT_INIT_CREATE_FILES,
          dd::Dictionary_impl::instance()->get_target_dd_version(),
          &pfs_server_tables, nullptr))
    return true;

  /*
    Iterate over the list of Plugin_table objects and
    produce strings containing sql ddl statements to create pfs
    tables.
  */
  List_iterator<const Plugin_table> table_it(pfs_server_tables);
  const Plugin_table *table = nullptr;

  while ((table = table_it++)) {
    add_pfs_definition(table);
  }

  return create_and_use_pfs_schema(thd) || drop_old_pfs_tables(thd) ||
         create_pfs_tables(thd);
}

}  // anonymous namespace

namespace dd {
namespace performance_schema {

bool create_pfs_schema(THD *thd) {
  bool exists = false;
  if (dd::schema_exists(thd, PERFORMANCE_SCHEMA_DB_NAME.str, &exists))
    return true;

  if (exists) return false;

  return dd::execute_query(thd,
                           dd::String_type("CREATE SCHEMA ") +
                               dd::String_type(PERFORMANCE_SCHEMA_DB_NAME.str) +
                               dd::String_type(" CHARACTER SET utf8mb4") +
                               dd::String_type(" COLLATE utf8mb4_0900_ai_ci"));
}

bool init_pfs_tables(enum_dd_init_type init_type) {
  if (init_type == dd::enum_dd_init_type::DD_INITIALIZE)
    return ::bootstrap::run_bootstrap_thread(nullptr, nullptr, &initialize_pfs,
                                             SYSTEM_THREAD_DD_INITIALIZE);
  else if (init_type == dd::enum_dd_init_type::DD_RESTART_OR_UPGRADE)
    return ::bootstrap::run_bootstrap_thread(nullptr, nullptr, &initialize_pfs,
                                             SYSTEM_THREAD_DD_RESTART);
  else {
    DBUG_ASSERT(false);
    return true;
  }
}

void set_PS_version_for_table(dd::Properties *table_options) {
  table_options->set(SERVER_PS_TABLE_PROPERTY_NAME, true);
}

}  // namespace performance_schema
}  // namespace dd
