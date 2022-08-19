/* Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.

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

/*
  This file contains code that updates data dictionary tables
  with the metadata of INFORMATION_SCHEMA tables.

  There are two types of I_S tables.
  - I_S tables exposed by the server.
  - I_S tables exposed by the plugin.

*/

#include "sql/dd/info_schema/metadata.h"

#include <sys/types.h>
#include <algorithm>
#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include "field_types.h"  // enum_field_types
#include "lex_string.h"
#include "m_string.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_loglevel.h"
#include "my_sys.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/log_shared.h"
#include "mysql/plugin.h"
#include "sql/dd/cache/dictionary_client.h"  // dd::cache::Dictionary_client
#include "sql/dd/dd_schema.h"                // dd::Schema_MDL_locker
#include "sql/dd/dd_table.h"                 // dd::get_sql_type_by_field_info
#include "sql/dd/dd_utility.h"               // check_if_server_ddse_readonly
#include "sql/dd/impl/bootstrap/bootstrap_ctx.h"  // dd::bootstrap::DD_boot...
#include "sql/dd/impl/bootstrap/bootstrapper.h"   // dd::Column
#include "sql/dd/impl/dictionary_impl.h"          // dd::Dictionary_impl
#include "sql/dd/impl/system_registry.h"          // dd::System_views
#include "sql/dd/impl/utils.h"                    // dd::System_views
#include "sql/dd/properties.h"                    // dd::Properties
#include "sql/dd/types/abstract_table.h"
#include "sql/dd/types/column.h"  // dd::Column
#include "sql/dd/types/schema.h"
#include "sql/dd/types/system_view.h"
#include "sql/dd/types/system_view_definition.h"  // dd::System_view_definition
#include "sql/dd/types/view.h"
#include "sql/dd_sql_view.h"  // update_referencing_views_metadata
#include "sql/item_create.h"
#include "sql/mdl.h"
#include "sql/mysqld.h"     // opt_readonly
#include "sql/sql_class.h"  // THD
#include "sql/sql_lex.h"
#include "sql/sql_plugin.h"  // plugin_foreach
#include "sql/sql_plugin_ref.h"
#include "sql/sql_profile.h"
#include "sql/sql_show.h"
#include "sql/system_variables.h"
#include "sql/table.h"
#include "sql/thd_raii.h"

namespace {

unsigned int UNKNOWN_PLUGIN_VERSION = -1;

const dd::String_type PLUGIN_VERSION_STRING("plugin_version");
const dd::String_type SERVER_I_S_TABLE_STRING("server_i_s_table");

}  // namespace

namespace {

// Hold context during IS metadata update to DD.
class Update_context {
 public:
  Update_context(THD *thd, bool commit_gaurd)
      : m_thd(thd),
        m_saved_var_tx_read_only(thd->variables.transaction_read_only),
        m_saved_tx_read_only(thd->tx_read_only),
        m_autocommit_guard(commit_gaurd ? thd : nullptr),
        m_mdl_handler(thd),
        m_auto_releaser(thd->dd_client()),
        m_schema_obj(nullptr),
        m_plugin_names(nullptr) {
    /*
      Set tx_read_only to false to allow installing DD tables even
      if the server is started with --transaction-read-only=true.
    */
    m_thd->variables.transaction_read_only = false;
    m_thd->tx_read_only = false;

    if (m_mdl_handler.ensure_locked(INFORMATION_SCHEMA_NAME.str) ||
        m_thd->dd_client()->acquire(INFORMATION_SCHEMA_NAME.str, &m_schema_obj))
      m_schema_obj = nullptr;

    DBUG_ASSERT(m_schema_obj);
  }

  ~Update_context() {
    // Restore thd state.
    m_thd->variables.transaction_read_only = m_saved_var_tx_read_only;
    m_thd->tx_read_only = m_saved_tx_read_only;
  }

  const dd::Schema *info_schema() { return m_schema_obj; }

  void set_plugin_names(std::vector<dd::String_type> *names) {
    m_plugin_names = names;
  }

  std::vector<dd::String_type> *plugin_names() { return m_plugin_names; }

 private:
  THD *m_thd;
  bool m_saved_var_tx_read_only;
  bool m_saved_tx_read_only;
  Disable_autocommit_guard m_autocommit_guard;
  dd::Schema_MDL_locker m_mdl_handler;
  dd::cache::Dictionary_client::Auto_releaser m_auto_releaser;

  // DD object for INFORMATION_SCHEMA.
  const dd::Schema *m_schema_obj;

  // List of plugins whose I_S tables are already present in DD.
  std::vector<dd::String_type> *m_plugin_names;
};

/**
  Holds context during I_S table referencing view's status/column metadata
  update.
*/
class View_metadata_update_ctx {
 public:
  View_metadata_update_ctx(THD *thd, bool is_drop_tbl_op)
      : m_thd(thd), m_saved_sql_command(thd->lex->sql_command) {
    m_thd->lex->sql_command =
        is_drop_tbl_op ? SQLCOM_UNINSTALL_PLUGIN : SQLCOM_INSTALL_PLUGIN;
  }

  ~View_metadata_update_ctx() { m_thd->lex->sql_command = m_saved_sql_command; }

 private:
  // Thread Handle.
  THD *m_thd;

  // Saved SQL command.
  enum_sql_command m_saved_sql_command;
};

/**
  Store metadata from ST_SCHEMA_TABLE into DD tables.
  Store option plugin_version=version, if it is not UNKNOWN_PLUGIN_VERSION,
  otherwise store server_i_s_table=true.

  @param thd           Thread
  @param ctx           Update context.
  @param schema_table  Pointer to ST_SCHEMA_TABLE contain
                       metadata of IS table.
  @param version       Plugin version.

  @returns false on success, else true.
*/
bool store_in_dd(THD *thd, Update_context *ctx, ST_SCHEMA_TABLE *schema_table,
                 unsigned int version) {
  // Skip I_S tables that are hidden from users.
  if (schema_table->hidden) return false;

  std::unique_ptr<dd::View> view_obj(
      ctx->info_schema()->create_system_view(thd));

  // Set view properties
  view_obj->set_client_collation_id(ctx->info_schema()->default_collation_id());

  view_obj->set_connection_collation_id(
      ctx->info_schema()->default_collation_id());

  view_obj->set_name(schema_table->table_name);

  dd::Properties *view_options = &view_obj->options();
  if (version != UNKNOWN_PLUGIN_VERSION)
    view_options->set(PLUGIN_VERSION_STRING, version);
  else
    view_options->set(SERVER_I_S_TABLE_STRING, true);

  /*
    Fill columns details
  */
  ST_FIELD_INFO *fields_info = schema_table->fields_info;
  const CHARSET_INFO *cs = get_charset(system_charset_info->number, MYF(0));
  for (; fields_info->field_name; fields_info++) {
    dd::Column *col_obj = view_obj->add_column();

    col_obj->set_name(fields_info->field_name);

    /*
      The 5.7 create_schema_table() creates Item_empty_string() item for
      MYSQL_TYPE_STRING. Item_empty_string->field_type() maps to
      MYSQL_TYPE_VARCHAR. So, we map MYSQL_TYPE_STRING to
      MYSQL_TYPE_VARCHAR when storing metadata into DD.
    */
    enum_field_types ft = fields_info->field_type;
    uint32 fl = fields_info->field_length;
    if (fields_info->field_type == MYSQL_TYPE_STRING) {
      ft = MYSQL_TYPE_VARCHAR;
    }

    col_obj->set_type(dd::get_new_field_type(ft));

    col_obj->set_char_length(fields_info->field_length);

    col_obj->set_nullable(fields_info->field_flags & MY_I_S_MAYBE_NULL);

    col_obj->set_unsigned(fields_info->field_flags & MY_I_S_UNSIGNED);

    col_obj->set_zerofill(false);

    // Collation ID
    col_obj->set_collation_id(system_charset_info->number);

    col_obj->set_column_type_utf8(dd::get_sql_type_by_field_info(
        thd, ft, fl, 0, col_obj->is_nullable(), col_obj->is_unsigned(), cs));

    col_obj->set_default_value_utf8(dd::String_type(STRING_WITH_LEN("")));
  }

  /*
    Acquire MDL on the view name, if not yet acquired.

    In case of INSTALL PLUGIN, the lock would be already taken. We acquire
    lock here during server restart and bootstrap.
  */
  if (thd->mdl_context.owns_equal_or_stronger_lock(
          MDL_key::TABLE, ctx->info_schema()->name().c_str(),
          view_obj->name().c_str(), MDL_EXCLUSIVE) == false) {
    MDL_request mdl_request;
    MDL_REQUEST_INIT(&mdl_request, MDL_key::TABLE,
                     ctx->info_schema()->name().c_str(),
                     view_obj->name().c_str(), MDL_EXCLUSIVE, MDL_TRANSACTION);
    if (thd->mdl_context.acquire_lock_nsec(
            &mdl_request, thd->variables.lock_wait_timeout_nsec))
      return true;
  }

  // Store the metadata into DD
  if (thd->dd_client()->store(view_obj.get())) {
    DBUG_ASSERT(thd->is_system_thread() || thd->killed || thd->is_error());
    return (true);
  }

  return false;
}

/**
  Helper method to store plugin IS table metadata into DD.
  Skip storing, if the I_S name is already present in
  Update_context plugin names.

  @param      thd            Thread ID
  @param      plugin         Reference to a plugin.
  @param      ctx            Pointer to Context for I_S update.

  @retval false on success
  @retval true when fails to store the metadata.
*/

static bool store_plugin_metadata(THD *thd, plugin_ref plugin,
                                  Update_context *ctx) {
  DBUG_ASSERT(plugin && ctx);

  // Store in DD tables.
  st_plugin_int *pi = plugin_ref_to_int(plugin);
  ST_SCHEMA_TABLE *schema_table = plugin_data<ST_SCHEMA_TABLE *>(plugin);
  return store_in_dd(thd, ctx, schema_table, pi->plugin->version);
}

/**
  Store plugin IS table metadata into DD.
  Update column metadata of views referencing I_S plugin table.
  Skip storing, if the I_S name is already present in
  Update_context plugin names.

  @param thd    Thread ID
  @param plugin Reference to a plugin.
  @param arg    Pointer to Context for I_S update.

  @return
    false on success
    true when fails to store the metadata.
*/

bool store_plugin_and_referencing_views_metadata(THD *thd, plugin_ref plugin,
                                                 void *arg) {
  st_plugin_int *pi = plugin_ref_to_int(plugin);
  Update_context *ctx = static_cast<Update_context *>(arg);

  // Stop if we already have IS metadata in DD.
  if (ctx->plugin_names()) {
    dd::String_type name(pi->name.str, pi->name.length);
    if (std::find(ctx->plugin_names()->begin(), ctx->plugin_names()->end(),
                  name) != ctx->plugin_names()->end()) {
      return false;
    }
  }

  View_metadata_update_ctx vw_update_ctx(thd, false);
  // Guard for uncommitted tables while updating views column metadata.
  Uncommitted_tables_guard uncommitted_tables(thd);

  return store_plugin_metadata(thd, plugin, ctx) ||
         update_referencing_views_metadata(
             thd, INFORMATION_SCHEMA_NAME.str,
             (plugin_data<ST_SCHEMA_TABLE *>(plugin))->table_name, false,
             &uncommitted_tables);
}

/**
  Iterate through dynamic I_S plugins, and store I_S table
  metadata into dictionary, during MySQL server startup. These
  are plugins that are loaded using server command line options.

  Does following,
  - Remove I_S table metadata for plugins that are not loaded.
  - Store I_S table metadata of plugins that are newly loaded.

  @param thd  Thread

  @returns  false on success, otherwise true.
*/
bool update_plugins_I_S_metadata(THD *thd) {
  //  Warn if we have DDSE in read only mode and continue server startup.
  if (dd::check_if_server_ddse_readonly(thd, INFORMATION_SCHEMA_NAME.str))
    return false;

  /*
    Stage 1:
    Remove I_S table metadata for plugins that are not loaded.
  */

  /*
    Create std::vector to hold 'plugin_name' of I_S tables that are
    persistent in DD and if the plugin is loaded.
  */
  Update_context ctx(thd, true);
  std::vector<dd::String_type> plugin_names;
  ctx.set_plugin_names(&plugin_names);
  std::vector<const dd::View *> sch_views;
  if (thd->dd_client()->fetch_schema_components(ctx.info_schema(), &sch_views))
    return true;

  bool error = false;
  for (const dd::View *view : sch_views) {
    // Skip if this is not a plugin I_S metadata.
    const dd::Properties *view_options = &view->options();
    if (!view_options->exists(PLUGIN_VERSION_STRING)) continue;

    // Lookup if plugin is loaded during this server startup.
    LEX_CSTRING plugin_name = {
        view->name().c_str(), static_cast<unsigned int>(view->name().length())};
    plugin_ref plugin = my_plugin_lock_by_name(thd, plugin_name,
                                               MYSQL_INFORMATION_SCHEMA_PLUGIN);
    if (plugin != nullptr) {
      unsigned int plugin_version = 0;
      st_plugin_int *plugin_int = plugin_ref_to_int(plugin);
      view_options->get(PLUGIN_VERSION_STRING, &plugin_version);

      // Testing to make sure we update plugins when version changes.
      DBUG_EXECUTE_IF("test_i_s_metadata_version",
                      { plugin_version = UNKNOWN_PLUGIN_VERSION; });

      if (plugin_int->plugin->version == plugin_version &&
          plugin_int->state == PLUGIN_IS_READY) {
        plugin_names.push_back(view->name());
        continue;
      }
    }

    // Remove metadata from DD if version mismatch.
    if (dd::info_schema::remove_I_S_view_metadata(thd, view->name())) break;

    // Update status of referencing views.
    {
      View_metadata_update_ctx vw_update_ctx(thd, true);
      // Guard for uncommitted tables while updating views column metadata.
      Uncommitted_tables_guard uncommitted_tables(thd);

      if (update_referencing_views_metadata(thd, INFORMATION_SCHEMA_NAME.str,
                                            view->name().c_str(), false,
                                            &uncommitted_tables))
        break;
    }
  }

  /*
    Stage 2:
    Store I_S table metadata of plugins that are newly loaded.
  */
  error = error || plugin_foreach_with_mask(
                       thd, store_plugin_and_referencing_views_metadata,
                       MYSQL_INFORMATION_SCHEMA_PLUGIN, PLUGIN_IS_READY, &ctx);

  return dd::end_transaction(thd, error);
}

/*
  Create INFORMATION_SCHEMA system views.
*/
bool create_system_views(THD *thd, bool is_non_dd_based) {
  // Force use of utf8 charset.
  const CHARSET_INFO *client_cs = thd->variables.character_set_client;
  const CHARSET_INFO *cs = thd->variables.collation_connection;
  const CHARSET_INFO *m_client_cs, *m_connection_cl;
  Disable_binlog_guard binlog_guard(thd);
  Implicit_substatement_state_guard substatement_guard(thd);

  resolve_charset("utf8", system_charset_info, &m_client_cs);
  resolve_collation("utf8_general_ci", system_charset_info, &m_connection_cl);

  thd->variables.character_set_client = m_client_cs;
  thd->variables.collation_connection = m_connection_cl;
  thd->update_charset();

  dd::System_views::Types sv_type =
      is_non_dd_based ? dd::System_views::Types::NON_DD_BASED_INFORMATION_SCHEMA
                      : dd::System_views::Types::INFORMATION_SCHEMA;

  // Iterate over system view definitions.
  bool error = false;
  for (dd::System_views::Const_iterator it =
           dd::System_views::instance()->begin(sv_type);
       it != dd::System_views::instance()->end();
       it = dd::System_views::instance()->next(it, sv_type)) {
    const dd::system_views::System_view_definition *view_def =
        (*it)->entity()->view_definition();

    // Build the CREATE VIEW DDL statement and execute it.
    if (view_def == nullptr ||
        dd::execute_query(thd, view_def->build_ddl_create_view())) {
      error = true;
      break;
    }
  }

  // Store the target I_S version.
  if (!error && !is_non_dd_based) {
    dd::Dictionary_impl *d = dd::Dictionary_impl::instance();
    if (!opt_initialize)
      dd::bootstrap::DD_bootstrap_ctx::instance().set_actual_I_S_version(
          d->get_actual_I_S_version(thd));
    error = d->set_I_S_version(thd, d->get_target_I_S_version());
    dd::bootstrap::DD_bootstrap_ctx::instance().set_I_S_upgrade_done();
  }

  // Restore the original character set.
  thd->variables.character_set_client = client_cs;
  thd->variables.collation_connection = cs;
  thd->update_charset();

  return error;
}

bool create_non_dd_based_system_views(THD *thd) {
  return create_system_views(thd, true);
}

/**
  Does following during server restart.

  - If I_S version is not changed, do nothing.
  - Else, remove all I_S metadata that belongs to server.
  - Store new server I_S metadata.
  - Recreate I_S system views, which will update old I_S metadata
    because we execute CREATE OR REPLACE VIEW.

  @param thd   Thread

  @returns false on success, otherwise true.
*/
bool update_server_I_S_metadata(THD *thd) {
  bool error = false;
  dd::Dictionary_impl *d = dd::Dictionary_impl::instance();

  // Stop if I_S version is same and no DD upgrade was done.
  uint actual_version = d->get_actual_I_S_version(thd);

  // Testing to make sure we update plugins when version changes.
  DBUG_EXECUTE_IF("test_i_s_metadata_version",
                  { actual_version = UNKNOWN_PLUGIN_VERSION; });

  if (d->get_target_I_S_version() == actual_version &&
      !dd::bootstrap::DD_bootstrap_ctx::instance().dd_upgrade_done())
    return false;

  /*
    Stop server restart if I_S version is changed and the server is
    started with DDSE in read-only mode.
  */
  if (dd::check_if_server_ddse_readonly(thd, INFORMATION_SCHEMA_NAME.str))
    return true;

  Update_context ctx(thd, true);
  const dd::Schema *sch_obj = ctx.info_schema();
  if (sch_obj == nullptr) return true;

  /*
    Stage 1:
    Remove all server I_S metadata from DD tables.
  */

  std::vector<const dd::View *> sch_views;
  if (thd->dd_client()->fetch_schema_components(sch_obj, &sch_views))
    return true;

  for (const dd::View *view : sch_views) {
    // Skip if this is not a server I_S table.
    const dd::Properties *view_options = &view->options();

    if (!view_options->exists(PLUGIN_VERSION_STRING)) {
      // Remove metadata from DD as I_S version is changed.
      error = dd::info_schema::remove_I_S_view_metadata(thd, view->name());

      if (error) break;

      // Update status of views referencing a system view.
      {
        View_metadata_update_ctx vw_update_ctx(thd, true);
        // Guard for uncommitted tables while updating views column metadata.
        Uncommitted_tables_guard uncommitted_tables(thd);

        error = update_referencing_views_metadata(
            thd, INFORMATION_SCHEMA_NAME.str, view->name().c_str(), false,
            &uncommitted_tables);
        if (error) break;
      }
    }
  }

  /*
    Stage 2:
    1) Store server I_S tables
    2) Recreate system view.
    3) Update the target IS version in DD.
  */
  error = error || dd::info_schema::store_server_I_S_metadata(thd) ||
          dd::info_schema::create_system_views(thd);

  return dd::end_transaction(thd, error);
}

bool initialize(THD *thd, bool non_dd_based_system_view) {
  /*
    Set tx_read_only to false to allow installing system views even
    if the server is started with --transaction-read-only=true.
  */
  thd->variables.transaction_read_only = false;
  thd->tx_read_only = false;

  Disable_autocommit_guard autocommit_guard(thd);

  dd::Dictionary_impl *d = dd::Dictionary_impl::instance();
  DBUG_ASSERT(d);

  if (!non_dd_based_system_view) {
    if (dd::info_schema::create_system_views(thd) ||
        dd::info_schema::store_server_I_S_metadata(thd))
      return true;
  } else {
    bool error = create_non_dd_based_system_views(thd);
    return dd::end_transaction(thd, error);
  }

  LogErr(INFORMATION_LEVEL, ER_CREATED_SYSTEM_WITH_VERSION,
         (int)d->get_target_dd_version());
  return false;
}

}  // namespace

namespace dd {
namespace info_schema {

bool create_system_views(THD *thd) { return ::create_system_views(thd, false); }

/*
  Store the server I_S table metadata into dictionary, once during MySQL
  server bootstrap.
*/
bool store_server_I_S_metadata(THD *thd) {
  bool error = false;
  Update_context ctx(thd, false);
  ST_SCHEMA_TABLE *schema_tables = get_schema_table(SCH_FIRST);
  {
    View_metadata_update_ctx vw_update_ctx(thd, false);
    // Guard for uncommitted tables while updating views column metadata.
    Uncommitted_tables_guard uncommitted_tables(thd);

    for (; schema_tables->table_name && !error; schema_tables++) {
      error = store_in_dd(thd, &ctx, schema_tables, UNKNOWN_PLUGIN_VERSION);
      if (!error) {
        // Update column metadata of views referencing I_S plugin table.
        error = update_referencing_views_metadata(
            thd, INFORMATION_SCHEMA_NAME.str, schema_tables->table_name, false,
            &uncommitted_tables);
      }
    }
  }

  return dd::end_transaction(thd, error);
}

// Update server and plugin I_S table metadata on server restart.
bool update_I_S_metadata(THD *thd) {
  return update_server_I_S_metadata(thd) || update_plugins_I_S_metadata(thd);
}

/*
  Store dynamic I_S plugin table metadata into dictionary, during INSTALL
  command execution.
*/
bool store_dynamic_plugin_I_S_metadata(THD *thd, st_plugin_int *plugin_int) {
  plugin_ref plugin = plugin_int_to_ref(plugin_int);
  Update_context ctx(thd, false);
  DBUG_ASSERT(plugin_int->plugin->type == MYSQL_INFORMATION_SCHEMA_PLUGIN);

  return store_plugin_metadata(thd, plugin, &ctx);
}

/*
  Remove the given plugin I_S table metadata from dictionary table.
*/
bool remove_I_S_view_metadata(THD *thd, const dd::String_type &view_name) {
  // Make sure you have lock on I_S schema.
  Schema_MDL_locker mdl_locker(thd);
  if (mdl_locker.ensure_locked(INFORMATION_SCHEMA_NAME.str)) return true;

  // Acquire exclusive lock on it before dropping.
  MDL_request mdl_request;
  MDL_REQUEST_INIT(&mdl_request, MDL_key::TABLE, INFORMATION_SCHEMA_NAME.str,
                   view_name.c_str(), MDL_EXCLUSIVE, MDL_TRANSACTION);
  if (thd->mdl_context.acquire_lock_nsec(&mdl_request,
                                         thd->variables.lock_wait_timeout_nsec))
    return (true);

  // Acquire the object.
  const dd::Abstract_table *at = nullptr;
  if (thd->dd_client()->acquire(INFORMATION_SCHEMA_NAME.str, view_name.c_str(),
                                &at))
    return (true);

  DBUG_ASSERT(at->type() == dd::enum_table_type::SYSTEM_VIEW);

  // Remove view from DD tables.
  Implicit_substatement_state_guard substatement_guard(thd);
  if (thd->dd_client()->drop(at)) {
    DBUG_ASSERT(thd->is_system_thread() || thd->killed || thd->is_error());
    return (true);
  }

  return (false);
}

bool initialize(THD *thd) { return ::initialize(thd, false); }

bool init_non_dd_based_system_view(THD *thd) { return ::initialize(thd, true); }

/*
  Get create view definition for the given I_S system view.
*/
bool get_I_S_view_definition(const dd::String_type &schema_name,
                             const dd::String_type &view_name,
                             dd::String_type *definition) {
  definition->clear();
  const dd::system_views::System_view *sys_view =
      dd::System_views::instance()->find(schema_name.c_str(),
                                         view_name.c_str());
  if (sys_view == nullptr) return true;

  const dd::system_views::System_view_definition *view_def =
      sys_view->view_definition();

  if (view_def == nullptr) return true;

  *definition = view_def->build_ddl_create_view();

  return (false);
}

}  // namespace info_schema
}  // namespace dd
