/* Copyright (c) 2019, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/dd/ndbinfo_schema/init.h"

#include "lex_string.h"
#include "m_ctype.h"
#include "my_dbug.h"
#include "mysql/components/services/log_builtins.h"  // LogErr
#include "mysql/thread_type.h"
#include "mysql_version.h"
#include "sql/bootstrap.h"                   // bootstrap::run_bootstrap_thread
#include "sql/dd/cache/dictionary_client.h"  // dd::cache::Dictionary_client
#include "sql/dd/dd_schema.h"                // dd::schema_exists
#include "sql/dd/dd_utility.h"               // check_if_server_ddse_readonly
#include "sql/dd/impl/dictionary_impl.h"     // dd::Dictionary_impl
#include "sql/dd/impl/utils.h"               // execute_query
#include "sql/dd/properties.h"               // dd::Properties
#include "sql/dd/string_type.h"              // dd::String_type
#include "sql/dd/upgrade/server.h"           // UPGRADE_FORCE
#include "sql/handler.h"                     // handlerton
#include "sql/opt_costconstantcache.h"       // init_optimizer_cost_model()
#include "sql/plugin_table.h"                // Plugin_table
#include "sql/sql_class.h"                   // THD
#include "sql/sql_list.h"                    // List<T>
#include "sql/thd_raii.h"                    // Disable_autocommit_guard

static bool forced_upgrade{false};

static bool create_schema(THD *thd, const char *schema_name) {
  bool exists = false;
  if (dd::schema_exists(thd, schema_name, &exists)) return true;

  if (exists) return false;

  dd::String_type query("CREATE SCHEMA ");
  query.append(schema_name);

  return dd::execute_query(thd, query);
}

/* WL#11563

   Starting from MySQL 8.0.20, the installed version of the ndbinfo schema
   is stored in dd_properties.

   If the property does not exist, or the stored schema version does not match
   the current version number, this indicates that the server has been upgraded
   (or downgraded), and all existing ndbinfo tables and views should be dropped
   and re-created using the current definitions.
*/
static bool check_ndbinfo_schema_has_correct_version(THD *thd) {
  uint version;

  dd::Dictionary_impl *d = dd::Dictionary_impl::instance();
  bool tag_exists = d->get_actual_ndbinfo_schema_version(thd, &version);

  return tag_exists ? (version == MYSQL_VERSION_ID) : false;
}

static bool drop_and_create_table(THD *thd, const Plugin_table &table) {
  dd::String_type drop_sql("DROP ");
  drop_sql.append(table.get_object_type());  // TABLE or VIEW
  drop_sql.append(" IF EXISTS ").append(table.get_qualified_name());

  if (dd::execute_query(thd, drop_sql)) return true;

  if (table.get_table_definition()) {
    /* Create schema if needed */
    if (create_schema(thd, table.get_schema_name())) return true;

    /* Create table */
    if (dd::execute_query(thd, table.get_ddl())) return true;
  }

  return false;  // success
}

static bool initialize_ndbinfo(THD *thd) {
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  Disable_autocommit_guard autocommit_guard(thd);

  // Fetch the ndbinfo handler.
  plugin_ref plugin = ha_resolve_by_name_raw(thd, LEX_CSTRING{"ndbinfo", 7});

  // If the plugin was not found, the server is running without ndbinfo
  if (plugin == nullptr) {
    return false;
  }

  // If ndbinfo is present, it should have been initialized
  handlerton *hton = plugin_data<handlerton *>(plugin);
  if (!(hton && hton->dict_init)) {
    DBUG_ASSERT(false);
    return true;
  }

  // Also check that ndbcluster itself is enabled
  if (!ha_storage_engine_is_enabled(
          ha_resolve_by_legacy_type(thd, DB_TYPE_NDBCLUSTER))) {
    return false;
  }

  // If no upgrade is needed, just write the "not upgrading" log message
  if (!forced_upgrade) {
    if (check_ndbinfo_schema_has_correct_version(thd)) {
      LogErr(INFORMATION_LEVEL, ER_NDBINFO_NOT_UPGRADING_SCHEMA);
      return false;
    }
  }

  // Abort if the data dictionary is in read-only mode
  if (dd::check_if_server_ddse_readonly(thd, "ndbinfo")) return true;

  // Upgrade of ndbinfo schema begins here. Write the "upgrading" log message.
  LogErr(INFORMATION_LEVEL, ER_NDBINFO_UPGRADING_SCHEMA, MYSQL_SERVER_VERSION);

  // Call into hton->dict_init() to fetch all ndbinfo tables and views
  List<const Plugin_table> ndbinfo_tables;
  if (hton->dict_init(DICT_INIT_CREATE_FILES,
                      dd::Dictionary_impl::instance()->get_target_dd_version(),
                      &ndbinfo_tables, nullptr))
    return true;

  // Create and use the ndbinfo schema
  if (create_schema(thd, "ndbinfo")) return true;
  if (dd::execute_query(thd, dd::String_type("USE ndbinfo"))) return true;

  // Create each table or view defined in the list
  bool failed = false;
  for (const Plugin_table &table : ndbinfo_tables) {
    if (!failed) {
      failed = drop_and_create_table(thd, table);
    }
  }

  // Update the stored version number
  if (!failed) {
    dd::Dictionary_impl *d = dd::Dictionary_impl::instance();
    failed = d->set_ndbinfo_schema_version(thd, MYSQL_VERSION_ID);
  }

  ndbinfo_tables.delete_elements();

  // Commit the DD transaction
  return dd::end_transaction(thd, failed);
}

/* Public interface */

namespace dd {
namespace ndbinfo {

bool init_schema_and_tables(long upgrade_mode) {
  forced_upgrade = (upgrade_mode == UPGRADE_FORCE);

  // Creating views will require the cost module to be initialized
  init_optimizer_cost_module(false);

  bool r = ::bootstrap::run_bootstrap_thread(
      nullptr, nullptr, &initialize_ndbinfo, SYSTEM_THREAD_DD_RESTART);

  // Shut down the temporary cost module
  delete_optimizer_cost_module();

  return r;
}

}  // namespace ndbinfo
}  // namespace dd
