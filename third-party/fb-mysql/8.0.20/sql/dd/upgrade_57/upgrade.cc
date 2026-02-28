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

#include "sql/dd/upgrade_57/upgrade.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string>
#include <vector>

#include "lex_string.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_dir.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_loglevel.h"
#include "my_sys.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/log_shared.h"
#include "mysql/plugin.h"
#include "mysql/psi/mysql_file.h"  // mysql_file_open
#include "mysql/udf_registration_types.h"
#include "mysql_version.h"  // MYSQL_VERSION_ID
#include "mysqld_error.h"
#include "sql/dd/cache/dictionary_client.h"  // dd::cache::Dictionary_client
#include "sql/dd/dd_schema.h"                // dd::schema_exists
#include "sql/dd/dd_tablespace.h"            // dd::fill_table_and_parts...
#include "sql/dd/impl/bootstrap/bootstrap_ctx.h"  // dd::DD_bootstrap_ctx
#include "sql/dd/impl/bootstrap/bootstrapper.h"   // execute_query
#include "sql/dd/impl/dictionary_impl.h"          // dd::Dictionary_impl
#include "sql/dd/impl/sdi.h"                      // sdi::store()
#include "sql/dd/impl/system_registry.h"          // dd::System_tables
#include "sql/dd/impl/utils.h"                    // execute_query
#include "sql/dd/info_schema/metadata.h"     // dd::info_schema::install_IS...
#include "sql/dd/performance_schema/init.h"  // create_pfs_schema
#include "sql/dd/sdi_file.h"                 // dd::sdi_file::EXT
#include "sql/dd/types/object_table.h"
#include "sql/dd/types/table.h"  // dd::Table
#include "sql/dd/types/tablespace.h"
#include "sql/dd/upgrade_57/event.h"
#include "sql/dd/upgrade_57/global.h"
#include "sql/dd/upgrade_57/routine.h"
#include "sql/dd/upgrade_57/schema.h"
#include "sql/dd/upgrade_57/table.h"
#include "sql/error_handler.h"  // Dummy_error_handler
#include "sql/handler.h"
#include "sql/lock.h"       // Tablespace_hash_set
#include "sql/log.h"        // sql_print_warning
#include "sql/mysqld.h"     // key_file_sdi
#include "sql/sd_notify.h"  // sysd::notify
#include "sql/sql_class.h"  // THD
#include "sql/sql_list.h"
#include "sql/sql_plugin.h"
#include "sql/sql_plugin_ref.h"
#include "sql/sql_prepare.h"
#include "sql/sql_table.h"  // build_tablename
#include "sql/stateless_allocator.h"
#include "sql/strfunc.h"  // lex_cstring_handle
#include "sql/table.h"
#include "sql/thd_raii.h"
#include "sql/transaction.h"  // trans_rollback

#include "sql/dd/impl/upgrade/server.h"
#include "sql/dd/upgrade/server.h"

namespace dd {

namespace upgrade_57 {
/*
  The variable is used to differentiate between a normal server restart
  and server upgrade.

  For the upgrade, before populating the DD tables, all the plugins needs
  to be initialized. Once the plugins are initialized, the server calls
  DD initialization function again to finish the upgrade process.

  In case of deleting dictionary tables, we need to delete mysql.ibd
  after innodb is shutdown. This flag is used in server shutdown  code
  to delete mysql.ibd after unsuccessful attempt of upgrade.
*/
static bool dd_upgrade_flag = false;

/*
  This variable is used to skip creation of SDI files for tables
  and InnoDB tablespaces when creating entry in dictionary. SDI entry
  These entries are created as last phase of upgrade.
*/
static bool create_sdi_flag = true;

bool in_progress() { return dd_upgrade_flag; }

void set_in_progress(bool param) { dd_upgrade_flag = param; }

bool allow_sdi_creation() { return create_sdi_flag; }

void set_allow_sdi_creation(bool param) { create_sdi_flag = param; }

/**
  Check if it is a file extension which should be moved
  to backup_metadata_57 folder upgrade upgrade is successful.
*/

static bool check_file_extension(const String_type &extn) {
  // Check for extensions
  if (extn.size() < 4) return false;

  return ((extn.compare(extn.size() - 4, 4, reg_ext) == 0) ||
          (extn.compare(extn.size() - 4, 4, TRG_EXT) == 0) ||
          (extn.compare(extn.size() - 4, 4, TRN_EXT) == 0) ||
          (extn.compare(extn.size() - 4, 4, PAR_EXT) == 0) ||
          (extn.compare(extn.size() - 4, 4, OPT_EXT) == 0) ||
          (extn.compare(extn.size() - 4, 4, NDB_EXT) == 0) ||
          (extn.compare(extn.size() - 4, 4, ISL_EXT) == 0));
}

/**
  Rename stats tables. Installation from 5.7 should contain
  mysql.innodb_table_stats and mysql.innodb_index_stats.
  Rename .frm and .ibd files for these tables here. These tables
  will be created from scratch with other dictionary tables. Data from
  5.7 stats table will be inserted in new created stats table
  via INSERT...SELECT statement.
*/

static void rename_stats_tables() {
  /*
    Rename mysql/innodb_index_stats.ibd and mysql/innodb_table_stats.ibd.
    Dictionary bootstrap will create these tables. Upgrade will copy
    data from 5.7 version of these tables using INSERT..SELECT
  */
  char to_path[FN_REFLEN];
  char from_path[FN_REFLEN];
  bool not_used;

  build_table_filename(to_path, sizeof(to_path) - 1, MYSQL_SCHEMA_NAME.str,
                       index_stats_backup.c_str(), IBD_EXT.c_str(), 0,
                       &not_used);

  build_table_filename(from_path, sizeof(from_path) - 1, MYSQL_SCHEMA_NAME.str,
                       index_stats.c_str(), IBD_EXT.c_str(), 0, &not_used);

  if (mysql_file_rename(key_file_misc, from_path, to_path, MYF(0))) {
    LogErr(WARNING_LEVEL, ER_DD_UPGRADE_RENAME_IDX_STATS_FILE_FAILED);
  }

  build_table_filename(to_path, sizeof(to_path) - 1, MYSQL_SCHEMA_NAME.str,
                       table_stats_backup.c_str(), IBD_EXT.c_str(), 0,
                       &not_used);

  build_table_filename(from_path, sizeof(from_path) - 1, MYSQL_SCHEMA_NAME.str,
                       table_stats.c_str(), IBD_EXT.c_str(), 0, &not_used);

  if (mysql_file_rename(key_file_misc, from_path, to_path, MYF(0))) {
    LogErr(WARNING_LEVEL, ER_DD_UPGRADE_RENAME_IDX_STATS_FILE_FAILED);
  }
}

/**
  Cleanup inside SE after upgrade for one SE.

  @param[in]    thd             Thread Handle
  @param[in]    plugin          Handlerton Plugin
  @param[in]    failed_upgrade  Flag to tell SE if cleanup is after failed
                                failed upgrade or successful upgrade.

  @retval false  ON SUCCESS
  @retval true   ON FAILURE
*/
static bool ha_finish_upgrade(THD *thd, plugin_ref plugin,
                              void *failed_upgrade) {
  handlerton *hton = plugin_data<handlerton *>(plugin);

  if (hton->finish_upgrade) {
    if (hton->finish_upgrade(thd, *(static_cast<bool *>(failed_upgrade))))
      return true;
  }
  return false;
}

// Cleanup inside SE after upgrade for all SE.
bool ha_finish_upgrade(THD *thd, bool failed_upgrade) {
  return (plugin_foreach(thd, ha_finish_upgrade, MYSQL_STORAGE_ENGINE_PLUGIN,
                         static_cast<void *>(&failed_upgrade)));
}

/**
  In case of successful upgrade, this function
  deletes all .frm, .TRG, .TRG, .par, .opt, .isl
  files from all databases.

  mysql.proc, mysql.event, statistics tables from 5.7
  will be deleted as part of cleanup.

  @param[in]  thd        Thread handle.
*/
bool finalize_upgrade(THD *thd) {
  uint i;
  MY_DIR *a = nullptr, *b = nullptr;
  String_type path;
  char from_path[FN_REFLEN];

  std::vector<String_type> db_name;

  Upgrade_status().remove();

  // Drop mysql.proc and mysql.event tables from 5.7
  (void)dd::execute_query(thd, "DROP TABLE IF EXISTS mysql.proc");
  (void)dd::execute_query(thd, "DROP TABLE IF EXISTS mysql.event");
  (void)dd::execute_query(thd,
                          "DROP TABLE IF EXISTS "
                          "mysql.innodb_table_stats_backup57");
  (void)dd::execute_query(thd,
                          "DROP TABLE IF EXISTS "
                          "mysql.innodb_index_stats_backup57");

  path.assign(mysql_real_data_home);
  if (!(a = my_dir(path.c_str(), MYF(MY_WANT_STAT)))) {
    LogErr(ERROR_LEVEL, ER_DD_UPGRADE_DD_OPEN_FAILED, path.c_str());
    return true;
  }

  // Scan all files and folders in data directory.
  for (i = 0; i < (uint)a->number_off_files; i++) {
    String_type file;

    file.assign(a->dir_entry[i].name);
    if (file.at(0) == '.') continue;

    // If its a folder, add it to the vector.
    if (MY_S_ISDIR(a->dir_entry[i].mystat->st_mode)) {
      db_name.push_back(a->dir_entry[i].name);
    } else {
      String_type file_ext;

      if (file.size() < 4) continue;

      file_ext.assign(file.c_str() + file.size() - 4);
      // Get the name without the file extension.
      if (check_file_extension(file_ext)) {
        if (fn_format(from_path, file.c_str(), mysql_real_data_home, "",
                      MYF(MY_UNPACK_FILENAME | MY_SAFE_PATH)) == nullptr)
          return true;

        (void)mysql_file_delete(key_file_misc, from_path, MYF(0));
      }
    }
  }

  // Iterate through the databases list
  for (String_type str : db_name) {
    String_type dir_name = str.c_str();
    char dir_path[FN_REFLEN];

    if (fn_format(dir_path, dir_name.c_str(), path.c_str(), "",
                  MYF(MY_UNPACK_FILENAME | MY_SAFE_PATH)) == nullptr)
      continue;

    if (!(b = my_dir(dir_path, MYF(MY_WANT_STAT)))) continue;

    // Scan all files and folders in data directory.
    for (i = 0; i < (uint)b->number_off_files; i++) {
      String_type file;
      file.assign(b->dir_entry[i].name);

      if ((file.at(0) == '.') || (file.size() < 4)) continue;

      String_type file_ext;
      file_ext.assign(file.c_str() + file.size() - 4);

      // Get the name without the file extension.
      if (check_file_extension(file_ext)) {
        if (fn_format(from_path, file.c_str(), dir_path, "",
                      MYF(MY_UNPACK_FILENAME | MY_SAFE_PATH)) == nullptr)
          continue;

        (void)mysql_file_delete(key_file_misc, from_path, MYF(0));
      }
    }
    my_dirend(b);
  }

  my_dirend(a);

  return ha_finish_upgrade(thd, false);
}

/**
  Function to scan mysql schema to check if any tables exist
  with the same name as DD tables to be created.

  This function checks existence of .frm files in mysql schema.

  @retval false  ON SUCCESS
  @retval true   ON FAILURE
*/
bool check_for_dd_tables() {
  // Iterate over DD tables, check .frm files
  for (System_tables::Const_iterator it = System_tables::instance()->begin();
       it != System_tables::instance()->end(); ++it) {
    if ((*it)->property() == System_tables::Types::SYSTEM) {
      continue;
    }
    String_type table_name = (*it)->entity()->name();
    String_type schema_name(MYSQL_SCHEMA_NAME.str);

    const System_tables::Types *table_type =
        System_tables::instance()->find_type(schema_name, table_name);

    bool is_innodb_stats_table =
        (table_type != nullptr) &&
        (*table_type == System_tables::Types::DDSE_PROTECTED);
    is_innodb_stats_table &= ((table_name == "innodb_table_stats") ||
                              (table_name == "innodb_index_stats"));

    if (is_innodb_stats_table) continue;

    char path[FN_REFLEN + 1];
    bool not_used;
    build_table_filename(path, sizeof(path) - 1, "mysql", table_name.c_str(),
                         reg_ext, 0, &not_used);

    if (!my_access(path, F_OK)) {
      LogErr(ERROR_LEVEL, ER_FILE_EXISTS_DURING_UPGRADE, path);
      return true;
    }
  }

  return false;
}

/**
  Rename back .ibd files for innodb stats table.
*/
void rename_back_stats_tables(THD *thd) {
  /*
    Ignore error of the statement execution as we are already in
    error handling code.
  */
  (void)dd::execute_query(thd, "SET GLOBAL INNODB_FAST_SHUTDOWN= 0");

  // Rename back mysql/innodb_index_stats.ibd and mysql/innodb_table_stats.ibd
  char to_path[FN_REFLEN];
  char from_path[FN_REFLEN];
  bool not_used;

  build_table_filename(to_path, sizeof(to_path) - 1, MYSQL_SCHEMA_NAME.str,
                       index_stats.c_str(), IBD_EXT.c_str(), 0, &not_used);

  build_table_filename(from_path, sizeof(from_path) - 1, MYSQL_SCHEMA_NAME.str,
                       index_stats_backup.c_str(), IBD_EXT.c_str(), 0,
                       &not_used);

  (void)mysql_file_rename(key_file_misc, from_path, to_path, MYF(0));

  build_table_filename(to_path, sizeof(to_path) - 1, MYSQL_SCHEMA_NAME.str,
                       table_stats.c_str(), IBD_EXT.c_str(), 0, &not_used);

  build_table_filename(from_path, sizeof(from_path) - 1, MYSQL_SCHEMA_NAME.str,
                       table_stats_backup.c_str(), IBD_EXT.c_str(), 0,
                       &not_used);

  (void)mysql_file_rename(key_file_misc, from_path, to_path, MYF(0));
}

/**
  Drop all .SDI files created during upgrade.
*/

static void drop_sdi_files() {
  uint i, j;
  // Iterate in data directory and delete all .SDI files
  MY_DIR *a, *b;
  String_type path;

  path.assign(mysql_real_data_home);

  if (!(a = my_dir(path.c_str(), MYF(MY_WANT_STAT)))) {
    LogErr(ERROR_LEVEL, ER_CANT_OPEN_DATADIR_AFTER_UPGRADE_FAILURE,
           path.c_str());
    return;
  }

  // Scan all files and folders in data directory.
  for (i = 0; i < (uint)a->number_off_files; i++) {
    String_type file;

    file.assign(a->dir_entry[i].name);
    if (file.at(0) == '.') continue;

    // If its a folder, iterate it to delete all .SDI files
    if (MY_S_ISDIR(a->dir_entry[i].mystat->st_mode)) {
      char dir_path[FN_REFLEN];
      if (fn_format(dir_path, file.c_str(), path.c_str(), "",
                    MYF(MY_UNPACK_FILENAME | MY_SAFE_PATH)) == nullptr) {
        LogErr(ERROR_LEVEL, ER_CANT_SET_PATH_FOR, file.c_str());
        continue;
      }

      if (!(b = my_dir(dir_path, MYF(MY_WANT_STAT)))) {
        LogErr(ERROR_LEVEL, ER_CANT_OPEN_DIR, dir_path);
        continue;
      }

      // Scan all files and folders in data directory.
      for (j = 0; j < (uint)b->number_off_files; j++) {
        String_type file2;
        file2.assign(b->dir_entry[j].name);

        if ((file2.at(0) == '.') || (file2.size() < 4)) continue;

        String_type file_ext;
        file_ext.assign(file2.c_str() + file2.size() - 4);
        if (file_ext.compare(0, 4, dd::sdi_file::EXT) == 0) {
          char to_path[FN_REFLEN];
          if (fn_format(to_path, file2.c_str(), dir_path, "",
                        MYF(MY_UNPACK_FILENAME | MY_SAFE_PATH)) == nullptr) {
            LogErr(ERROR_LEVEL, ER_CANT_SET_PATH_FOR, file2.c_str());
            continue;
          }

          (void)mysql_file_delete(key_file_sdi, to_path, MYF(MY_WME));
        }
      }
      my_dirend(b);
    } else {
      // Delete .SDI files in data directory created for schema.
      String_type file_ext;
      if (file.size() < 4) continue;
      file_ext.assign(file.c_str() + file.size() - 4);
      // Get the name without the file extension.
      if (file_ext.compare(0, 4, dd::sdi_file::EXT) == 0) {
        char to_path[FN_REFLEN];
        if (fn_format(to_path, file.c_str(), path.c_str(), "",
                      MYF(MY_UNPACK_FILENAME | MY_SAFE_PATH)) == nullptr) {
          LogErr(ERROR_LEVEL, ER_CANT_SET_PATH_FOR, file.c_str());
          continue;
        }
        (void)mysql_file_delete(key_file_sdi, to_path, MYF(MY_WME));
      }
    }
  }

  my_dirend(a);
}  // drop_dd_table

/**
  Create SDI information for all tablespaces and tables.

  @param[in] thd         Thread handle.

  @retval false  ON SUCCESS
  @retval true   ON FAILURE
*/
bool add_sdi_info(THD *thd) {
  // Fetch list of tablespaces. We will ignore error in storing SDI info
  // as upgrade can only roll forward in this stage. Use Error handler to avoid
  // any error calls in dd::sdi::store()
  std::vector<dd::String_type> tablespace_names;
  Dummy_error_handler error_handler;
  MEM_ROOT mem_root(PSI_NOT_INSTRUMENTED, MEM_ROOT_BLOCK_SIZE);
  Thd_mem_root_guard root_guard(thd, &mem_root);

  if (thd->dd_client()->fetch_global_component_names<dd::Tablespace>(
          &tablespace_names)) {
    LogErr(ERROR_LEVEL, ER_DD_UPGRADE_FAILED_TO_FETCH_TABLESPACES);
    return (true);
  }

  // Add sdi info
  thd->push_internal_handler(&error_handler);
  for (dd::String_type &tsc : tablespace_names) {
    dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
    Disable_autocommit_guard autocommit_guard(thd);
    dd::Tablespace *ts = nullptr;

    if (thd->dd_client()->acquire_for_modification<dd::Tablespace>(tsc, &ts)) {
      // In case of error, we will continue with upgrade.
      LogErr(ERROR_LEVEL, ER_DD_UPGRADE_FAILED_TO_ACQUIRE_TABLESPACE,
             tsc.c_str());
      continue;
    }

    plugin_ref pr =
        ha_resolve_by_name_raw(thd, lex_cstring_handle(ts->engine()));
    handlerton *hton = nullptr;

    if (pr)
      hton = plugin_data<handlerton *>(pr);
    else
      LogErr(ERROR_LEVEL, ER_DD_UPGRADE_FAILED_TO_RESOLVE_TABLESPACE_ENGINE,
             ts->name().c_str(), ts->engine().c_str());

    // In case of error, we will continue with upgrade.
    if (hton && hton->upgrade_space_version(ts))
      LogErr(ERROR_LEVEL, ER_DD_UPGRADE_FAILED_TO_UPDATE_VER_NO_IN_TABLESPACE,
             ts->name().c_str());

    if (hton && hton->sdi_create) {
      // Error handling not possible at this stage, upgrade should complete.
      if (hton->sdi_create(ts))
        LogErr(ERROR_LEVEL, ER_FAILED_TO_STORE_SDI_FOR_TABLESPACE,
               ts->name().c_str());

      // Write changes to dictionary.
      if (thd->dd_client()->update(ts)) {
        trans_rollback_stmt(thd);
        LogErr(ERROR_LEVEL, ER_FAILED_TO_STORE_SDI_FOR_TABLESPACE,
               ts->name().c_str());
      }
      trans_commit_stmt(thd);
      trans_commit(thd);
    }
    mem_root.ClearForReuse();
  }
  thd->pop_internal_handler();

  // Fetch list of tables from dictionary
  std::vector<dd::Object_id> table_ids;
  if (thd->dd_client()->fetch_global_component_ids<dd::Table>(&table_ids)) {
    LogErr(ERROR_LEVEL, ER_DD_UPGRADE_FAILED_TO_FETCH_TABLES);
    return (true);
  }

  // Add sdi info
  thd->push_internal_handler(&error_handler);
  for (dd::Object_id &table_id : table_ids) {
    dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
    const dd::Table *table = nullptr;
    if (thd->dd_client()->acquire(table_id, &table) || !table) {
      mem_root.ClearForReuse();
      continue;
    }

    if (dd::sdi::store(thd, table)) {
      LogErr(ERROR_LEVEL, ER_UNKNOWN_TABLE_IN_UPGRADE, table->name().c_str());
      trans_rollback_stmt(thd);
    }
    trans_commit_stmt(thd);
    trans_commit(thd);
    mem_root.ClearForReuse();
  }
  thd->pop_internal_handler();

  // Add status to mark tablespace modification complete.
  if (Upgrade_status().update(Upgrade_status::enum_stage::SDI_INFO_UPDATED))
    return true;

  LogErr(SYSTEM_LEVEL, ER_DD_UPGRADE_DD_POPULATED);
  log_sink_buffer_check_timeout();
  sysd::notify("STATUS=Data Dictionary upgrade from MySQL 5.7 complete\n");

  return false;
}  // add_sdi_info

//
// Methods of class Update_status.
//

// Get update status.
Upgrade_status::enum_stage Upgrade_status::get() {
  if (open(O_RDONLY)) return enum_stage::NONE;

  enum_stage status = read();

  if (close()) return enum_stage::NONE;

  return status;
}

// Update upgrade status.
bool Upgrade_status::update(Upgrade_status::enum_stage stage) {
  if (open(O_TRUNC | O_WRONLY)) return true;

  write(stage);

  if (close()) return true;

  return false;
}

// Constructor initialization.
Upgrade_status::Upgrade_status()
    : m_file(nullptr), m_filename("mysql_dd_upgrade_info") {}

// Create status file.
bool Upgrade_status::create() {
  if (open(O_TRUNC | O_WRONLY)) return true;

  write(enum_stage::STARTED);

  if (exists() == false || close()) return true;

  return false;
}

// Open status file.
bool Upgrade_status::open(int flags) {
  DBUG_ASSERT(m_file == nullptr);

  if (!(m_file = my_fopen(m_filename.c_str(), flags, MYF(0)))) {
    LogErr(ERROR_LEVEL, ER_DD_UPGRADE_INFO_FILE_OPEN_FAILED, m_filename.c_str(),
           errno);
    return true;
  }

  return false;
}

// Read status from file.
Upgrade_status::enum_stage Upgrade_status::read() {
  DBUG_ASSERT(m_file);

  enum_stage stage = enum_stage::NONE;
  size_t items_read MY_ATTRIBUTE((unused));

  if (!feof(m_file)) items_read = fread(&stage, sizeof(int), 1, m_file);

  return stage;
}

// Write status to file.
bool Upgrade_status::write(Upgrade_status::enum_stage stage) {
  DBUG_ASSERT(m_file);

  fwrite(&stage, sizeof(int), 1, m_file);
  fflush(m_file);
  return false;
}

bool Upgrade_status::exists() {
  //  Check if the upgrade_info_file was properly created/updated
  return !my_access(m_filename.c_str(), F_OK);
}

// Close status file.
bool Upgrade_status::close() {
  DBUG_ASSERT(m_file);

  if (my_fclose(m_file, MYF(0))) {
    LogErr(ERROR_LEVEL, ER_DD_UPGRADE_INFO_FILE_CLOSE_FAILED,
           m_filename.c_str(), errno);
    return true;
  }

  m_file = nullptr;

  return false;
}

// Delete status file.
bool Upgrade_status::remove() {
  DBUG_ASSERT(!m_file);
  (void)mysql_file_delete(key_file_misc, m_filename.c_str(), MYF(MY_WME));
  return false;
}

// Delete dictionary tables
bool terminate(THD *thd) {
  // Function call to SEs for cleanup after failed upgrade.
  ha_finish_upgrade(thd, true);

  // RAII to handle error messages.
  dd::upgrade::Bootstrap_error_handler bootstrap_error_handler;

  // Set flag true to delete mysql.ibd after innodb is shutdown.
  set_in_progress(true);

  // Rename back stats tables
  rename_back_stats_tables(thd);

  // Drop SDI files.
  drop_sdi_files();

  return false;
}

/**
  create data dictionary entry for tablespaces for one SE.

  @param[in]    thd         Thread Handle
  @param[in]    plugin      Handlerton Plugin

  @retval false  ON SUCCESS
  @retval true   ON FAILURE
*/
static bool ha_migrate_tablespaces(THD *thd, plugin_ref plugin, void *) {
  handlerton *hton = plugin_data<handlerton *>(plugin);

  int error = 0;
  if (hton->upgrade_tablespace) {
    error = hton->upgrade_tablespace(thd);

    // Commit or Rollback dictionary change.
    if (error) {
      trans_rollback_stmt(thd);
      // Full rollback in case we have THD::transaction_rollback_request.
      trans_rollback(thd);
    } else {
      error = (trans_commit_stmt(thd) || trans_commit(thd));
    }

    if (error) {
      LogErr(ERROR_LEVEL, ER_DD_UPGRADE_TABLESPACE_MIGRATION_FAILED, error);
      return true;
    }
  }
  return false;
}

/**
  create data dictionary entry for tablespaces for one SE.

  @param[in]    thd         Thread Handle

  @retval false  ON SUCCESS
  @retval true   ON FAILURE
*/
static bool ha_migrate_tablespaces(THD *thd) {
  return (plugin_foreach(thd, ha_migrate_tablespaces,
                         MYSQL_STORAGE_ENGINE_PLUGIN, nullptr));
}

/**
  Migrate statistics from 5.7 stats tables.
  Ignore error here as stats table data can be regenerated by
  user using ANALYZE command.
*/
static bool migrate_stats(THD *thd) {
  dd::upgrade::Bootstrap_error_handler error_handler;
  error_handler.set_log_error(false);
  if (dd::execute_query(thd,
                        "INSERT IGNORE INTO mysql.innodb_table_stats "
                        "SELECT * FROM mysql.innodb_table_stats_backup57 "
                        "WHERE table_name not like '%#P#%'"))
    LogErr(WARNING_LEVEL, ER_DD_UPGRADE_FAILED_TO_CREATE_TABLE_STATS);
  else
    LogErr(INFORMATION_LEVEL, ER_DD_UPGRADE_TABLE_STATS_MIGRATE_COMPLETED);

  if (dd::execute_query(thd,
                        "INSERT IGNORE INTO mysql.innodb_index_stats "
                        "SELECT * FROM mysql.innodb_index_stats_backup57 "
                        "WHERE table_name not like '%#P#%'"))
    LogErr(WARNING_LEVEL, ER_DD_UPGRADE_FAILED_TO_CREATE_INDEX_STATS);
  else
    LogErr(INFORMATION_LEVEL, ER_DD_UPGRADE_TABLE_STATS_MIGRATE_COMPLETED);

  // Reset error logging
  error_handler.set_log_error(true);

  return false;
}

// Initialize dictionary in case of server restart.
static bool restart_dictionary(THD *thd) {
  // RAII to handle error messages.
  dd::upgrade::Bootstrap_error_handler bootstrap_error_handler;

  // RAII to handle error in execution of CREATE TABLE.
  Key_length_error_handler key_error_handler;
  /*
    Ignore ER_TOO_LONG_KEY for dictionary tables during restart.
    Do not print the error in error log as we are creating only the
    cached objects and not physical tables.
    TODO: Workaround due to bug#20629014. Remove when the bug is fixed.
  */
  bool error = false;
  thd->push_internal_handler(&key_error_handler);
  bootstrap_error_handler.set_log_error(false);
  error = bootstrap::restart(thd);
  bootstrap_error_handler.set_log_error(true);
  thd->pop_internal_handler();
  return error;
}

/**
  Upgrade logs inside one SE.

  @param[in]    thd         Thread Handle
  @param[in]    plugin      Handlerton Plugin

  @retval false  ON SUCCESS
  @retval true   ON FAILURE
*/
static bool upgrade_logs(THD *thd, plugin_ref plugin, void *) {
  handlerton *hton = plugin_data<handlerton *>(plugin);

  if (hton->upgrade_logs) {
    if (hton->upgrade_logs(thd)) return true;
  }
  return false;
}

/**
  Upgrade logs inside all SE.

  @param[in]    thd         Thread Handle

  @retval false  ON SUCCESS
  @retval true   ON FAILURE
*/
static bool ha_upgrade_engine_logs(THD *thd) {
  if (plugin_foreach(thd, upgrade_logs, MYSQL_STORAGE_ENGINE_PLUGIN, nullptr))
    return true;

  return false;
}

// Initialize DD in case of upgrade.
bool do_pre_checks_and_initialize_dd(THD *thd) {
  // Set both variables false in the beginning
  set_in_progress(false);
  opt_initialize = false;
  set_allow_sdi_creation(true);

  Disable_autocommit_guard autocommit_guard(thd);
  Dictionary_impl *d = dd::Dictionary_impl::instance();
  DBUG_ASSERT(d);
  cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());

  char path[FN_REFLEN + 1];
  bool not_used;
  build_table_filename(path, sizeof(path) - 1, "", "mysql", ".ibd", 0,
                       &not_used);
  bool exists_mysql_tablespace = (!my_access(path, F_OK));

  // Check existence of mysql/plugin.frm
  build_table_filename(path, sizeof(path) - 1, "mysql", "plugin", ".frm", 0,
                       &not_used);
  bool exists_plugin_frm = (!my_access(path, F_OK));

  /*
    If mysql.ibd and mysql/plugin.frm do not exist,
    this is neither a restart nor an in-place upgrade case.
    Upgrade process has dependency on mysql.plugin table.
    Server restart is not possible without mysql.ibd.
    Exit with an error.
  */
  if (!exists_mysql_tablespace && !exists_plugin_frm) {
    LogErr(ERROR_LEVEL, ER_DD_UPGRADE_FAILED_FIND_VALID_DATA_DIR);
    return true;
  }

  // Read stage of upgrade from the file.
  Upgrade_status upgrade_status;
  bool upgrade_status_exists = upgrade_status.exists();
  Upgrade_status::enum_stage upgrade_stage = Upgrade_status::enum_stage::NONE;
  if (upgrade_status_exists) {
    upgrade_stage = upgrade_status.get();

    DBUG_EXECUTE_IF("dd_upgrade_debug_info",
                    sql_print_information("Status of upgrade is %d",
                                          static_cast<int>(upgrade_stage)););
  }

  // Upgrade data directory from mysql-5.7
  if (!exists_mysql_tablespace && !upgrade_status_exists) {
    if (opt_upgrade_mode == UPGRADE_NONE) {
      LogErr(ERROR_LEVEL, ER_SERVER_UPGRADE_OFF);
      return true;
    }

    // Create the file to track stages of upgrade.
    if (upgrade_status.create()) return true;

    /*
      If mysql.idb does not exist and updgrade stage tracking file
      does not exist, we are in upgrade mode.
    */
    LogErr(SYSTEM_LEVEL, ER_DD_UPGRADE_START);
    log_sink_buffer_check_timeout();
    sysd::notify("STATUS=Data Dictionary upgrade from MySQL 5.7 in progress\n");
  }

  /*
    Initialize InnoDB in restart mode if mysql.ibd is present.
    Else, initialize InnoDB in upgrade mode to create mysql tablespace
    and upgrade redo and undo logs.
    If mysql.ibd does not exist but upgrade stage tracking file exist
    This can happen in rare scenario when server detects it needs to upgrade.
    Server creates mysql_dd_upgrade_info file but crashes/killed before
    creating mysql.ibd. In this case, innodb was initialized above in upgrade
    mode. It would create mysql tablespace. Do nothing here, we will treat this
    as upgrade.
  */

  if (exists_mysql_tablespace) {
    if (bootstrap::DDSE_dict_init(thd, DICT_INIT_CHECK_FILES,
                                  d->get_target_dd_version())) {
      LogErr(ERROR_LEVEL, ER_DD_SE_INIT_FAILED);
      return true;
    }
  } else {
    if (bootstrap::DDSE_dict_init(thd, DICT_INIT_UPGRADE_57_FILES,
                                  d->get_target_dd_version())) {
      LogErr(ERROR_LEVEL, ER_DD_UPGRADE_FAILED_INIT_DD_SE);
      Upgrade_status().remove();
      return true;
    }
  }

  /*
    Add status to mark initialization of InnoDB.
    This indicates undo and redo logs are upgraded and mysql.ibd exists.
  */
  if (!exists_mysql_tablespace && !upgrade_status_exists) {
    if (upgrade_status.update(Upgrade_status::enum_stage::DICT_SPACE_CREATED))
      return true;

    DBUG_EXECUTE_IF("dd_upgrade_stage_1",
                    /*
                      Server will crash will upgrading 5.7 data directory.
                      This will leave server is an inconsistent state.
                      File tracking upgrade will have Stage 1 written in it.
                      Next restart of server on same data directory should
                      revert all changes done by upgrade and data directory
                      should be reusable by 5.7 server.
                    */
                    DBUG_SUICIDE(););
  }

  /*
    If mysql.ibd exists and upgrade stage tracking does not exist, restart
    the server.

    For ordinary restart of an 8.0 server, and for upgrades post 8.0,
    this code path will be taken.
  */
  if (exists_mysql_tablespace && !upgrade_status_exists) {
    return (restart_dictionary(thd));
  }

  if (exists_mysql_tablespace && upgrade_status_exists) {
    /*
      If mysql.idb exist and upgrade stage tracking file exist,
      get stage of upgrade. This is happen only in extreme cases like
      Server crash or server kill.
    */
    switch (upgrade_stage) {
      case Upgrade_status::enum_stage::STARTED:
      case Upgrade_status::enum_stage::DICT_SPACE_CREATED: {
        /*
          Upgrade Stage 0 DD_UPGRADE_STARTED:
          File tracking upgrade stage was created but InnoDB was not completely
          initialized but mysql.ibd was created. No new undo logs by InnoDB is
          created.

          Upgrade Stage 1 DD_UPGRADE_DICT_SPACE_CREATED:
          Dictionary tables were not created completely, InnoDB undo log should
          not have any new data.

          Error out, delete mysql.ibd, downgrade innodb undo and redo logs.
        */
        LogErr(ERROR_LEVEL, ER_DD_ABORTING_PARTIAL_UPGRADE);
        terminate(thd);
        return true;
      }
      case Upgrade_status::enum_stage::DICT_TABLES_CREATED:
      case Upgrade_status::enum_stage::DICTIONARY_CREATED: {
        /*
          Upgrade Stage 2 DD_UPGRADE_DICT_TABLES_CREATED:
          It indicates that dictionary tables were created but dictionary
          was not completely initialized. This is a rare condition.

          Upgrade Stage 3 DD_UPGRADE_DICTIONARY_CREATED:
          It indicates that dictionary tables inititlization was complete but
          user tables were not completely upgraded.
          InnoDB will have undo logs in these both stages.

          Inititialize dictionary, start InnoDB recovery to empty undo logs,
          then error out and delete mysql.ibd, downgrade innodb undo
          and redo logs.
        */

        LogErr(ERROR_LEVEL, ER_DD_UPGRADE_FOUND_PARTIALLY_UPGRADED_DD_ABORT);

        // Try to Initialize dictionary to empty undo log.
        bootstrap::recover_innodb_upon_upgrade(thd);

        terminate(thd);
        return true;
      }
      case Upgrade_status::enum_stage::USER_TABLE_UPGRADED: {
        /*
          It indicates that user tables were upgraded but SDI information was
          not updated in tablespaces.

          Restart dictionary, then update SDI information.
        */
        LogErr(INFORMATION_LEVEL,
               ER_DD_UPGRADE_FOUND_PARTIALLY_UPGRADED_DD_CONTINUE);
        if (restart_dictionary(thd)) return true;

        // Ignore error in this stage and continue with server restart.
        (void)add_sdi_info(thd);

        (void)migrate_stats(thd);

        // Cleanup after successful upgrade.
        finalize_upgrade(thd);

        return false;
      }
      case Upgrade_status::enum_stage::SDI_INFO_UPDATED: {
        /*
          It indicates that SDI information was created but stats migration
          was not complete. Ignore and continue with server restart.
        */

        LogErr(INFORMATION_LEVEL,
               ER_DD_UPGRADE_FOUND_PARTIALLY_UPGRADED_DD_CONTINUE);

        if (restart_dictionary(thd)) return true;

        // Cleanup after successful upgrade.
        finalize_upgrade(thd);

        return false;
      }
      case Upgrade_status::enum_stage::NONE:
        // Try to restart server in case this impossible scenario hits
        return restart_dictionary(thd);

    }  // End of switch
  }

  /*
    Create New DD tables in DD and storage engine. Mark
    dd_upgrade_flag to true to indicate that we are upgrading.
  */
  set_allow_sdi_creation(true);
  set_in_progress(true);
  opt_initialize = true;

  if (check_for_dd_tables()) {
    LogErr(ERROR_LEVEL, ER_DD_FRM_EXISTS_FOR_TABLE);
    return true;
  }

  /*
    Ignore ER_TOO_LONG_KEY for dictionary tables creation.
    TODO: Workaround due to bug#20629014. Remove when the bug is fixed.
  */
  // RAII to handle error in execution of CREATE TABLE.
  Key_length_error_handler key_error_handler;
  thd->push_internal_handler(&key_error_handler);

  if (bootstrap::initialize_dictionary(thd, in_progress(), d) ||
      dd::info_schema::create_system_views(thd) ||
      dd::info_schema::store_server_I_S_metadata(thd)) {
    thd->pop_internal_handler();
    terminate(thd);
    return true;
  }

  // Add status to mark creation and initialization of dictionary.
  if (Upgrade_status().update(Upgrade_status::enum_stage::DICTIONARY_CREATED))
    return true;

  thd->pop_internal_handler();

  LogErr(INFORMATION_LEVEL, ER_DD_CREATED_FOR_UPGRADE);

  // Rename .ibd files for innodb stats tables
  rename_stats_tables();

  // Mark opt_initiazlize false after creating dictionary tables.
  opt_initialize = false;

  // Mark flag true to skip creation of SDI information in tablespaces.
  set_allow_sdi_creation(false);

  // Migrate tablespaces from SE to dictionary.
  if (ha_migrate_tablespaces(thd)) {
    terminate(thd);
    return true;
  }

  /*
    Migrate meta data of plugin table to DD.
    It is used in plugin initialization.
  */
  if (migrate_plugin_table_to_dd(thd)) {
    terminate(thd);
    return true;
  }

  /*
    Plugins may need to create performance schema tables. During upgrade from
    5.7, we do not yet have an entry in mysql.schemata for performance schema,
    so creation of such tables will fail. To avoid this, we migrate the entry
    here if the schema was present in 5.7. If the performance schema was not
    present in 5.7, then we create the schema explicitly, if the server is
    configured to use the performance schema.
  */
  size_t path_len = build_table_filename(
      path, sizeof(path) - 1, PERFORMANCE_SCHEMA_DB_NAME.str, "", "", 0);
  path[path_len - 1] = 0;  // Remove last '/' from path
  MY_STAT stat_info;

  // RAII to handle error messages.
  dd::upgrade::Bootstrap_error_handler bootstrap_error_handler;

  if (mysql_file_stat(key_file_misc, path, &stat_info, MYF(0)) != nullptr) {
    if (migrate_schema_to_dd(thd, PERFORMANCE_SCHEMA_DB_NAME.str)) {
      terminate(thd);
      return true;
    }
  }
#ifdef WITH_PERFSCHEMA_STORAGE_ENGINE
  else if (dd::performance_schema::create_pfs_schema(thd)) {
    terminate(thd);
    return true;
  }
#endif

  // Reset flag
  set_allow_sdi_creation(true);

  return false;
}

// Server Upgrade from 5.7
bool fill_dd_and_finalize(THD *thd) {
  bool error = false;

  // RAII to handle error messages.
  dd::upgrade::Bootstrap_error_handler bootstrap_error_handler;

  /*
    While migrating tables, mysql_prepare_create_table() is called which checks
    for duplicated value in SET data type. Error is reported for duplicated
    values only in strict sql mode. Reset the value of sql_mode to zero while
    migrating data to dictionary.
  */
  thd->variables.sql_mode = 0;

  std::vector<dd::String_type> db_name;
  std::vector<dd::String_type>::iterator it;

  if (find_schema_from_datadir(&db_name)) {
    terminate(thd);
    return true;
  }

  dd::upgrade::Syntax_error_handler error_handler;
  thd->push_internal_handler(&error_handler);
  // Upgrade schema and tables, create view without resolving dependency
  for (it = db_name.begin(); it != db_name.end(); it++) {
    bool exists = false;
    dd::schema_exists(thd, it->c_str(), &exists);

    if (!exists && migrate_schema_to_dd(thd, it->c_str())) {
      thd->pop_internal_handler();
      terminate(thd);
      return true;
    }

    // Mark flag false to skip creation of SDI information.
    set_allow_sdi_creation(false);
    if (migrate_all_frm_to_dd(thd, it->c_str(), false)) {
      // Don't return from here, we want to print all error to error log
      error |= true;
    }
    // Reset flag
    set_allow_sdi_creation(true);
  }

  /*
    Do not print error while resolving routine or view dependency from
    my_error(). Function resolving routine/view dependency will print warning
    if it is not from sys schema. Fatal errors will result in termination
    of upgrade.
  */
  bootstrap_error_handler.set_log_error(false);

  error |= migrate_events_to_dd(thd);
  error |= migrate_routines_to_dd(thd);

  // We will not get error in this step unless its a fatal error.
  for (it = db_name.begin(); it != db_name.end(); it++) {
    // Upgrade view resolving dependency
    if (migrate_all_frm_to_dd(thd, it->c_str(), true)) {
      // Don't return from here, we want to print all error to error log.
      error = true;
    }
  }

  // Reset error log output behavior.
  bootstrap_error_handler.set_log_error(true);
  thd->pop_internal_handler();

  DBUG_EXECUTE_IF("dd_upgrade_stage_3",
                  /*
                    Server will crash will upgrading 5.7 data directory.
                    This will leave server is an inconsistent state.
                    File tracking upgrade will have Stage 3 written in it.
                    Next restart of server on same data directory should
                    revert all changes done by upgrade and data directory
                    should be reusable by 5.7 server.
                  */
                  DBUG_SUICIDE(););

  if (error) {
    terminate(thd);
    return true;
  }

  // Upgrade logs in storage engine
  if (ha_upgrade_engine_logs(thd)) {
    LogErr(ERROR_LEVEL, ER_DD_UPGRADE_SE_LOGS_FAILED);
    terminate(thd);
    return true;
  }

  /*
    Add status to mark creation and initialization of dictionary.
    We will be modifying innodb tablespaces now.
    After this step, upgrade process can only roll forward.
  */
  if (Upgrade_status().update(Upgrade_status::enum_stage::USER_TABLE_UPGRADED))
    return true;

  log_sink_buffer_check_timeout();

  // Add SDI information to all tablespaces
  if (add_sdi_info(thd))
    LogErr(ERROR_LEVEL, ER_DD_UPGRADE_SDI_INFO_UPDATE_FAILED);

  // Migrate Statistics tables
  (void)migrate_stats(thd);

  // Finalize upgrade process.
  if (finalize_upgrade(thd)) {
    terminate(thd);
    return true;
  }

  // Mark upgrade flag false
  set_in_progress(false);

  if (bootstrap::setup_dd_objects_and_collations(thd)) {
    terminate(thd);
    return true;
  }

  return false;
}

}  // namespace upgrade_57
}  // namespace dd
