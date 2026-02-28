/*
   Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

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

/* create and drop of databases */

#include "sql/sql_db.h"

#include "my_config.h"

#include <errno.h>
#ifdef _WIN32
#include <direct.h>
#endif
#include <string.h>
#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <atomic>
#include <set>
#include <string>
#include <vector>

#include "lex_string.h"
#include "m_ctype.h"
#include "m_string.h"
#include "my_command.h"
#include "my_dbug.h"
#include "my_dir.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_macros.h"
#include "my_sys.h"
#include "my_thread_local.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/log_shared.h"
#include "mysql/psi/mysql_file.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/service_mysql_alloc.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "mysys_err.h"  // EE_*
#include "sql/auth/auth_acls.h"
#include "sql/auth/auth_common.h"  // SELECT_ACL
#include "sql/auth/sql_security_ctx.h"
#include "sql/binlog.h"                      // mysql_bin_log
#include "sql/dd/cache/dictionary_client.h"  // Dictionary_client
#include "sql/dd/dd.h"                       // dd::get_dictionary()
#include "sql/dd/dd_schema.h"                // dd::create_schema
#include "sql/dd/dd_table.h"                 // is_encrypted()
#include "sql/dd/dictionary.h"               // dd::Dictionary
#include "sql/dd/string_type.h"
#include "sql/dd/types/abstract_table.h"
#include "sql/dd/types/schema.h"
#include "sql/dd/upgrade_57/upgrade.h"  // dd::upgrade::in_progress
#include "sql/debug_sync.h"             // DEBUG_SYNC
#include "sql/derror.h"                 // ER_THD
#include "sql/error_handler.h"          // Drop_table_error_handler
#include "sql/events.h"                 // Events
#include "sql/handler.h"
#include "sql/lock.h"       // lock_schema_name
#include "sql/log.h"        // log_*()
#include "sql/log_event.h"  // Query_log_event
#include "sql/mdl.h"
#include "sql/mysqld.h"  // key_file_misc
#include "sql/mysqld_thd_manager.h"
#include "sql/psi_memory_key.h"  // key_memory_THD_db
#include "sql/rpl_gtid.h"
#include "sql/rpl_slave_commit_order_manager.h"  // Commit_order_manager
#include "sql/session_tracker.h"
#include "sql/sp.h"  // lock_db_routines
#include "sql/sql_admission_control.h"
#include "sql/sql_base.h"   // lock_table_names
#include "sql/sql_class.h"  // THD
#include "sql/sql_const.h"
#include "sql/sql_error.h"
#include "sql/sql_handler.h"  // mysql_ha_rm_tables
#include "sql/sql_table.h"    // build_table_filename
#include "sql/system_variables.h"
#include "sql/table.h"  // TABLE_LIST
#include "sql/thd_raii.h"
#include "sql/transaction.h"  // trans_rollback_stmt
#include "sql_string.h"
#include "typelib.h"

/*
  .frm is left in this list so that any orphan files can be removed on upgrade.
  .SDI needs to be there for now... need to investigate why...
*/
const char *del_exts[] = {".frm", ".BAK", ".TMD", ".opt",
                          ".OLD", ".cfg", ".SDI", NullS};
static TYPELIB deletable_extentions = {array_elements(del_exts) - 1, "del_exts",
                                       del_exts, nullptr};
static constexpr auto read_only_options_key = "read_only";

static Change_db_callback change_db_callback{nullptr};

static bool find_unknown_and_remove_deletable_files(THD *thd, MY_DIR *dirp,
                                                    const char *path);

static bool find_db_tables(THD *thd, const dd::Schema &schema, const char *db,
                           TABLE_LIST **tables);

static long mysql_rm_arc_files(THD *thd, MY_DIR *dirp, const char *org_path);
static bool rm_dir_w_symlink(const char *org_path, bool send_error);
static void mysql_change_db_impl(THD *thd, const LEX_CSTRING &new_db_name,
                                 ulong new_db_access,
                                 const CHARSET_INFO *new_db_charset);

bool get_default_db_collation(const dd::Schema &schema,
                              const CHARSET_INFO **collation) {
  *collation = get_charset(schema.default_collation_id(), MYF(0));
  if (*collation == nullptr) {
    char buff[STRING_BUFFER_USUAL_SIZE];
    my_error(ER_UNKNOWN_COLLATION, MYF(0),
             llstr(schema.default_collation_id(), buff));
    return true;
  }
  return false;
}

/**
  Return default database collation.

  @param       thd          Thread context.
  @param       db_name      Database name.
  @param [out] collation    Charset object pointer if object exists else NULL.

  @return      false No error.
               true  Error (thd->is_error is assumed to be set.)
*/

bool get_default_db_collation(THD *thd, const char *db_name,
                              const CHARSET_INFO **collation) {
  // Check cached info in THD first.
  mysql_mutex_lock(&thd->LOCK_thd_db_default_collation_hash);
  auto it = thd->m_db_default_collation.find(db_name);
  if (it != thd->m_db_default_collation.end()) {
    *collation = it->second;
    mysql_mutex_unlock(&thd->LOCK_thd_db_default_collation_hash);
    return false;
  }
  mysql_mutex_unlock(&thd->LOCK_thd_db_default_collation_hash);

  // We must make sure the schema is released and unlocked in the right order.
  dd::Schema_MDL_locker mdl_handler(thd);
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  const dd::Schema *sch_obj = nullptr;

  if (mdl_handler.ensure_locked(db_name) ||
      thd->dd_client()->acquire(db_name, &sch_obj))
    return true;

  if (sch_obj) {
    bool result = get_default_db_collation(*sch_obj, collation);
    if (!result) {
      // Cache value in THD.
      mysql_mutex_lock(&thd->LOCK_thd_db_default_collation_hash);
      thd->m_db_default_collation.emplace(db_name, *collation);
      mysql_mutex_unlock(&thd->LOCK_thd_db_default_collation_hash);
    }
    return result;
  }
  return false;
}

/*
 * Reads from the data dictionary to determine read only setting, and cache
 * this on the THD, and optionally return in out parameter.
 *
 * Returns false on error, true on success.
 */
static bool fill_db_read_only_from_dd(THD *thd, const char *db,
                                      enum enum_db_read_only *out) {
  DBUG_ENTER("fill_db_read_only_from_dd");
  dd::Schema_MDL_locker mdl_handler(thd);
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  const dd::Schema *sch_obj = NULL;
  enum enum_db_read_only flag = DB_READ_ONLY_NULL;

  if (mdl_handler.ensure_locked(db) ||
      thd->dd_client()->acquire(db, &sch_obj)) {
    DBUG_ASSERT(thd->is_error() || thd->killed);
    DBUG_RETURN(true);
  }

  if (sch_obj) {
    flag = get_db_read_only(*sch_obj);
  } else {
    // Database not found in DD. This happens during CREATE DATABASE, so
    // assume not read only.
    flag = DB_READ_ONLY_NO;
    goto end;
  }

  // Cache value in THD.
  mysql_mutex_lock(&thd->LOCK_thd_db_read_only_hash);
  thd->m_db_read_only_hash[db] = flag;
  mysql_mutex_unlock(&thd->LOCK_thd_db_read_only_hash);

end:
  if (out) {
    *out = flag;
  }
  DBUG_RETURN(false);
}

/*
 * Check if the database is read-only from thread's local map.
 */
bool is_thd_db_read_only_by_name(THD *thd, const char *db) {
  DBUG_ENTER("is_thd_db_read_only_by_name");
  bool super = thd->m_main_security_ctx.check_access(SUPER_ACL);
  bool binlog_admin =
      thd->m_main_security_ctx.has_global_grant(STRING_WITH_LEN("BINLOG_ADMIN"))
          .first;
  enum enum_db_read_only flag = DB_READ_ONLY_NULL;

  // Check cached info in THD first.
  mysql_mutex_lock(&thd->LOCK_thd_db_read_only_hash);
  auto it = thd->m_db_read_only_hash.find(std::string(db));
  if (it != thd->m_db_read_only_hash.end()) {
    flag = it->second;
  }
  mysql_mutex_unlock(&thd->LOCK_thd_db_read_only_hash);

  // Info was not found in THD. Check data dictionary.
  if (flag == DB_READ_ONLY_NULL) {
    if (fill_db_read_only_from_dd(thd, db, &flag)) {
      // Assume read only if we fail to read from DD.
      DBUG_RETURN(true);
    }
  }

  DBUG_ASSERT(flag >= DB_READ_ONLY_NO && flag <= DB_READ_ONLY_SUPER);

  if (flag == DB_READ_ONLY_SUPER ||
      (flag == DB_READ_ONLY_YES && !super && !binlog_admin)) {
    DBUG_RETURN(true);
  }

  DBUG_RETURN(false);
}

enum_db_read_only get_db_read_only(const dd::Schema &schema) {
  DBUG_ENTER("get_db_read_only");
  const dd::Properties &options = schema.options();
  uint32 val = 0;

  if (options.exists(read_only_options_key)) {
    if (options.get(read_only_options_key, &val) || val < DB_READ_ONLY_NO ||
        val > DB_READ_ONLY_SUPER) {
      my_error(ER_UNKNOWN_DB_READ_ONLY, MYF(0), std::to_string(val).c_str());
      DBUG_RETURN(DB_READ_ONLY_NO);
    }
  } else {
    // No option set. Assume not read only.
    DBUG_RETURN(DB_READ_ONLY_NO);
  }

  DBUG_RETURN(static_cast<enum_db_read_only>(val));
}

struct Update_thd_db_metadata : public Do_THD_Impl {
  const char *db_name;
  std::string metadata;

  Update_thd_db_metadata(const char *db_name, const char *metadata)
      : db_name(db_name), metadata(metadata) {}

  virtual void operator()(THD *thd) {
    thd->update_db_metadata(db_name, metadata);
  }
};

/* Update db_metadata in all threads using the specified database */
static void update_thd_db_metadata(const char *db_name,
                                   HA_CREATE_INFO *create) {
  DBUG_ENTER("update_db_metadata");

  Update_thd_db_metadata updater{db_name, create->db_metadata.ptr()};

  Global_THD_manager *thd_manager = Global_THD_manager::get_instance();
  thd_manager->do_for_all_thd(&updater);

  DBUG_VOID_RETURN;
}

struct Remove_thd_db_metadata : public Do_THD_Impl {
  const char *db_name;

  Remove_thd_db_metadata(const char *db_name) : db_name(db_name) {}

  virtual void operator()(THD *thd) { thd->remove_db_metadata(db_name); }
};

/* Remove db_metadata in all threads using the specified database */
static void remove_thd_db_metadata(const char *db_name) {
  DBUG_ENTER("remove_db_metadata");

  Remove_thd_db_metadata remover{db_name};

  Global_THD_manager *thd_manager = Global_THD_manager::get_instance();
  thd_manager->do_for_all_thd(&remover);

  DBUG_VOID_RETURN;
}

/* Update db read only flag in all threads' local hash map */
static void update_thd_db_read_only(const char *db,
                                    enum enum_db_read_only db_read_only) {
  DBUG_ENTER("update_thd_db_read_only");

  struct Update_THD_Read_Only : public Do_THD_Impl {
    const char *m_db;
    enum enum_db_read_only m_db_read_only;
    Update_THD_Read_Only(const char *db, enum enum_db_read_only db_read_only)
        : m_db(db), m_db_read_only(db_read_only) {}
    virtual void operator()(THD *thd) {
      mysql_mutex_lock(&thd->LOCK_thd_db_read_only_hash);
      thd->m_db_read_only_hash[m_db] = m_db_read_only;
      mysql_mutex_unlock(&thd->LOCK_thd_db_read_only_hash);
    }
  } update_thd(db, db_read_only);

  Global_THD_manager *thd_manager = Global_THD_manager::get_instance();
  thd_manager->do_for_all_thd(&update_thd);

  DBUG_VOID_RETURN;
}

/* Delete db read only flag in all threads' local hash map */
static void del_thd_db_read_only(const char *db) {
  DBUG_ENTER("del_thd_db_read_only");

  struct Delete_THD_Read_Only : public Do_THD_Impl {
    const char *m_db;
    Delete_THD_Read_Only(const char *db) : m_db(db) {}
    virtual void operator()(THD *thd) {
      mysql_mutex_lock(&thd->LOCK_thd_db_read_only_hash);
      thd->m_db_read_only_hash.erase(m_db);
      mysql_mutex_unlock(&thd->LOCK_thd_db_read_only_hash);
    }
  } update_thd(db);

  Global_THD_manager *thd_manager = Global_THD_manager::get_instance();
  thd_manager->do_for_all_thd(&update_thd);

  DBUG_VOID_RETURN;
}

/* Update db read only flag in all threads' local hash map */
static void update_thd_db_default_collation(const char *db,
                                            const CHARSET_INFO *collation) {
  DBUG_ENTER("update_thd_db_default_collation");

  struct Update_THD_DB_Default_Collation : public Do_THD_Impl {
    const char *m_db;
    const CHARSET_INFO *m_collation;
    Update_THD_DB_Default_Collation(const char *db,
                                    const CHARSET_INFO *collation)
        : m_db(db), m_collation(collation) {}
    virtual void operator()(THD *thd) {
      mysql_mutex_lock(&thd->LOCK_thd_db_default_collation_hash);
      thd->m_db_default_collation[m_db] = m_collation;
      mysql_mutex_unlock(&thd->LOCK_thd_db_default_collation_hash);
    }
  } update_thd(db, collation);

  Global_THD_manager *thd_manager = Global_THD_manager::get_instance();
  thd_manager->do_for_all_thd(&update_thd);

  DBUG_VOID_RETURN;
}

/* Delete db read only flag in all threads' local hash map */
static void del_thd_db_default_collation(const char *db) {
  DBUG_ENTER("del_thd_db_default_collation");

  struct Delete_THD_DB_Default_Collation : public Do_THD_Impl {
    const char *m_db;
    Delete_THD_DB_Default_Collation(const char *db) : m_db(db) {}
    virtual void operator()(THD *thd) {
      mysql_mutex_lock(&thd->LOCK_thd_db_default_collation_hash);
      thd->m_db_default_collation.erase(m_db);
      mysql_mutex_unlock(&thd->LOCK_thd_db_default_collation_hash);
    }
  } update_thd(db);

  Global_THD_manager *thd_manager = Global_THD_manager::get_instance();
  thd_manager->do_for_all_thd(&update_thd);

  DBUG_VOID_RETURN;
}

/**
  Auxiliary function which writes CREATE/ALTER or DROP DATABASE statement
  to the binary log overriding connection's current database with one
  being dropped.
*/

static bool write_db_cmd_to_binlog(THD *thd, const char *db, bool trx_cache) {
  if (mysql_bin_log.is_open()) {
    int errcode = query_error_code(thd, true);
    Query_log_event qinfo(thd, thd->query().str, thd->query().length, trx_cache,
                          false,
                          /* suppress_use */ true, errcode);
    /*
      Write should use the database being created/altered or dropped
      as the "current database" and not the threads current database,
      which is the default. If we do not change the "current database"
      to the database being created/dropped, the CREATE/DROP statement
      will not be replicated when using --binlog-do-db to select
      databases to be replicated.

      An example (--binlog-do-db=sisyfos):

      CREATE DATABASE bob;        # Not replicated
      USE bob;                    # 'bob' is the current database
      CREATE DATABASE sisyfos;    # Not replicated since 'bob' is
                                  # current database.
      USE sisyfos;                # Will give error on slave since
                                  # database does not exist.
    */
    qinfo.db = db;
    qinfo.db_len = strlen(db);

    thd->add_to_binlog_accessed_dbs(db);

    return mysql_bin_log.write_event(&qinfo);
  }

  return false;
}

static void set_db_default_charset(const THD *thd,
                                   HA_CREATE_INFO *create_info) {
  if (create_info->default_table_charset == nullptr) {
    create_info->default_table_charset = thd->variables.collation_server;
  } else {
    if (!(create_info->used_fields & HA_CREATE_USED_DEFAULT_COLLATE) &&
        create_info->default_table_charset == &my_charset_utf8mb4_0900_ai_ci)
      create_info->default_table_charset =
          thd->variables.default_collation_for_utf8mb4;
  }
}

/**
  Create a database

  @param thd		Thread handler
  @param db		Name of database to create
                        Function assumes that this is already validated.
  @param create_info	Database create options (like character set)

  SIDE-EFFECTS
   1. Report back to client that command succeeded (my_ok)
   2. Report errors to client
   3. Log event to binary log

  @retval false ok
  @retval true  Error
*/

bool mysql_create_db(THD *thd, const char *db, HA_CREATE_INFO *create_info) {
  DBUG_TRACE;

  /*
    Use Auto_releaser to keep uncommitted object for database until
    trans_commit() call.
  */
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());

  // Reject creation of the system schema except for system threads.
  if (!thd->is_dd_system_thread() &&
      dd::get_dictionary()->is_dd_schema_name(db) &&
      !(create_info->options & HA_LEX_CREATE_IF_NOT_EXISTS)) {
    my_error(ER_NO_SYSTEM_SCHEMA_ACCESS, MYF(0), db);
    return true;
  }

  if (ha_check_reserved_db_name(db)) {
    my_error(ER_WRONG_DB_NAME, MYF(0), db);
    return true;
  }

  /*
    Check if user has permission to alter database, if encryption type
    provided differ from global 'default_table_encryption' setting.
    We use 'default_table_encryption' value if encryption is not supplied
    by user.
  */
  bool encrypt_schema = false;
  if (create_info->encrypt_type.str) {
    encrypt_schema = dd::is_encrypted(create_info->encrypt_type);
  } else {
    encrypt_schema = thd->variables.default_table_encryption;
  }
  if (opt_table_encryption_privilege_check &&
      encrypt_schema != thd->variables.default_table_encryption &&
      check_table_encryption_admin_access(thd)) {
    my_error(ER_CANNOT_SET_DATABASE_ENCRYPTION, MYF(0));
    return true;
  }

  /*
    When creating the schema, we must lock the schema name without case (for
    correct MDL locking) when l_c_t_n == 2.
  */
  char name_buf[NAME_LEN + 1];
  const char *lock_db_name = db;
  if (lower_case_table_names == 2) {
    my_stpcpy(name_buf, db);
    my_casedn_str(&my_charset_utf8_tolower_ci, name_buf);
    lock_db_name = name_buf;
  }
  if (lock_schema_name(thd, lock_db_name)) return true;

  dd::cache::Dictionary_client &dc = *thd->dd_client();
  dd::String_type schema_name{db};
  const dd::Schema *existing_schema = nullptr;
  if (dc.acquire(schema_name, &existing_schema)) {
    return true;
  }

  bool store_in_dd = true;
  bool if_not_exists = (create_info->options & HA_LEX_CREATE_IF_NOT_EXISTS);
  if (existing_schema != nullptr) {
    if (if_not_exists == false) {
      my_error(ER_DB_CREATE_EXISTS, MYF(0), db);
      return true;
    }
    push_warning_printf(thd, Sql_condition::SL_NOTE, ER_DB_CREATE_EXISTS,
                        ER_THD(thd, ER_DB_CREATE_EXISTS), db);

    store_in_dd = false;
  }

  /* Check directory */
  char path[FN_REFLEN + 16];
  bool was_truncated;
  size_t path_len = build_table_filename(path, sizeof(path) - 1, db, "", "", 0,
                                         &was_truncated);
  if (was_truncated) {
    my_error(ER_IDENT_CAUSES_TOO_LONG_PATH, MYF(0), sizeof(path) - 1, path);
    return true;
  }
  path[path_len - 1] = 0;  // Remove last '/' from path

  // If we are creating the system schema, then we create it physically
  // only during first time server initialization. During ordinary restart,
  // we still execute the CREATE statement to initialize the meta data, but
  // the physical representation of the schema is not re-created since it
  // already exists.
  MY_STAT stat_info;
  bool schema_dir_exists =
      (mysql_file_stat(key_file_misc, path, &stat_info, MYF(0)) != nullptr);
  if (thd->is_dd_system_thread() &&
      (!opt_initialize || dd::upgrade_57::in_progress()) &&
      dd::get_dictionary()->is_dd_schema_name(db)) {
    /*
      CREATE SCHEMA statement is being executed from bootstrap thread.
      Server should either be in restart mode or upgrade mode to create only
      dd::Schema object for the dictionary cache.
    */
    if (!schema_dir_exists) {
      my_printf_error(ER_BAD_DB_ERROR,
                      "System schema directory does not exist.", MYF(0));
      return true;
    }
  } else if (store_in_dd) {
    if (schema_dir_exists) {
      my_error(ER_SCHEMA_DIR_EXISTS, MYF(0), path);
      return true;
    }

    // Don't create folder inside data directory in case we are upgrading.
    if (my_errno() != ENOENT) {
      char errbuf[MYSYS_STRERROR_SIZE];
      my_error(EE_STAT, MYF(0), path, my_errno(),
               my_strerror(errbuf, sizeof(errbuf), my_errno()));
      return true;
    }
    if (my_mkdir(path, 0777, MYF(0)) < 0) {
      char errbuf[MYSQL_ERRMSG_SIZE];
      my_error(ER_SCHEMA_DIR_CREATE_FAILED, MYF(0), db, my_errno(),
               my_strerror(errbuf, MYSQL_ERRMSG_SIZE, my_errno()));
      return true;
    }
  }

  /*
    Create schema in DD. This is done even when initializing the server
    and creating the system schema. In that case, the shared cache will
    store the object without storing it to disk. When the DD tables have
    been created, the cached objects will be stored persistently.
  */

  if (store_in_dd) {
    set_db_default_charset(thd, create_info);

    if (dd::create_schema(thd, db, create_info->default_table_charset,
                          encrypt_schema, create_info->db_metadata.ptr())) {
      /*
        We could be here due an deadlock or some error reported
        by DD API framework. We remove the database directory
        which we just created above.

        It is expected that rm_dir_w_symlink() would not fail as
        we already old MDL lock on database and no parallel
        thread can remove the table before the current create
        database operation. Even if the call fails due to some
        other error we ignore the error as we anyway return
        failure (true) here.
      */
      if (!schema_dir_exists) rm_dir_w_symlink(path, true);
      return true;
    }
  }

  // Log the query in the handler's binlog
  ha_binlog_log_query(thd, nullptr, LOGCOM_CREATE_DB, thd->query().str,
                      thd->query().length, db, "");

  /*
    If we have not added database to the data-dictionary we don't have
    active transaction at this point. In this case we can't use
    binlog's trx cache, which requires transaction with valid XID.
  */
  if (write_db_cmd_to_binlog(thd, db, store_in_dd)) {
    if (!schema_dir_exists) rm_dir_w_symlink(path, true);
    return true;
  }

  /*
    Do commit locally instead of relying on caller in order to be
    able to remove directory in case of failure.
  */
  if (trans_commit_stmt(thd) || trans_commit(thd)) {
    if (!schema_dir_exists) rm_dir_w_symlink(path, true);
    return true;
  }

  my_ok(thd, 1);
  return false;
}

/* db-name is already validated when we come here */

bool mysql_alter_db(THD *thd, const char *db, HA_CREATE_INFO *create_info) {
  DBUG_TRACE;
  TABLE_LIST *tables = NULL;

  // Reject altering the system schema except for system threads.
  if (!thd->is_dd_system_thread() &&
      dd::get_dictionary()->is_dd_schema_name(db)) {
    my_error(ER_NO_SYSTEM_SCHEMA_ACCESS, MYF(0), db);
    return true;
  }

  /*
    Check if user has permission to alter database, if encryption type
    provided differ from global 'default_table_encryption' setting.
  */
  if (create_info->encrypt_type.str && opt_table_encryption_privilege_check &&
      dd::is_encrypted(create_info->encrypt_type) !=
          thd->variables.default_table_encryption &&
      check_table_encryption_admin_access(thd)) {
    my_error(ER_CANNOT_SET_DATABASE_ENCRYPTION, MYF(0));
    return true;
  }

  if (lock_schema_name(thd, db)) return true;

  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  dd::Schema *schema = nullptr;
  if (thd->dd_client()->acquire_for_modification(db, &schema)) return true;

  if (schema == nullptr) {
    my_error(ER_NO_SUCH_DB, MYF(0), db);
    return true;
  }

  // Lock all tables while under schema lock so that we block all transactions
  // from touching these tables until the ALTER is done. Because the DB read
  // only check and the COMMIT are not done atomically, we use the table MDL
  // to serialize ALTER DB and any write statements.
  if (find_db_tables(thd, *schema, db, &tables)) {
    return true;
  }

  if (lock_table_names_nsec(thd, tables, NULL,
                            thd->variables.lock_wait_timeout_nsec, 0)) {
    return true;
  }

  // Set new collation ID if submitted in the statement.
  if (create_info->used_fields & HA_CREATE_USED_DEFAULT_CHARSET) {
    set_db_default_charset(thd, create_info);
    schema->set_default_collation_id(
        create_info->default_table_charset->number);
  }

  if (create_info->db_metadata.ptr() != nullptr) {
    schema->set_db_metadata(create_info->db_metadata.ptr());
  }

  if (create_info->db_read_only != DB_READ_ONLY_NULL) {
    dd::Properties &properties = schema->options();
    properties.set(read_only_options_key,
                   static_cast<uint32>(create_info->db_read_only));

    // We need to update the value on the current thd early, so that the
    // statement commit does not error with read only.
    if (create_info->db_read_only != DB_READ_ONLY_SUPER) {
      mysql_mutex_lock(&thd->LOCK_thd_db_read_only_hash);
      thd->m_db_read_only_hash[db] = create_info->db_read_only;
      mysql_mutex_unlock(&thd->LOCK_thd_db_read_only_hash);
    } else {
      // If the current ALTER statement is for super read only, we also need
      // to populate the THD cache before the data dictionary is updated, so
      // that statement commit does not error with read only.
      fill_db_read_only_from_dd(thd, db, nullptr);
    }
  }

  // Set encryption type.
  if (create_info->encrypt_type.length > 0)
    schema->set_default_encryption(dd::is_encrypted(create_info->encrypt_type));

  // Update schema.
  if (thd->dd_client()->update(schema)) return true;

  ha_binlog_log_query(thd, nullptr, LOGCOM_ALTER_DB, thd->query().str,
                      thd->query().length, db, "");

  if (write_db_cmd_to_binlog(thd, db, true)) return true;

  /*
    Commit the statement locally instead of relying on caller,
    in order to be sure that it is  successfull, before changing
    options of current database.
  */
  if (trans_commit_stmt(thd) || trans_commit(thd)) return true;

  /* Log the alter to the error log */
  {
    std::string str(thd->query().str, thd->query().length);

    // NO_LINT_DEBUG
    LogErr(INFORMATION_LEVEL, ER_LOG_ALTER_DB, str.c_str(),
           thd->security_context()->user().str,
           thd->security_context()->host().str);
  }

  /*
    Change collation options if the current database is being
    altered and the clause is explicitly submitted in the ALTER
    statement.
  */
  if (create_info->used_fields & HA_CREATE_USED_DEFAULT_CHARSET &&
      thd->db().str && !my_strcasecmp(table_alias_charset, thd->db().str, db)) {
    thd->db_charset = create_info->default_table_charset
                          ? create_info->default_table_charset
                          : thd->variables.collation_server;
    thd->variables.collation_database = thd->db_charset;
  }

  if (create_info->db_read_only != DB_READ_ONLY_NULL) {
    update_thd_db_read_only(db, create_info->db_read_only);
  }

  /* If db_metadata is changed, update it in all threads using this database */
  if (create_info->db_metadata.ptr()) update_thd_db_metadata(db, create_info);

  /* If db_metadata is changed, update it in all threads using this database */
  if (create_info->used_fields & HA_CREATE_USED_DEFAULT_CHARSET) {
    update_thd_db_default_collation(db, create_info->default_table_charset);
  }

  my_ok(thd, 1);
  return false;
}

/**
  Error handler which converts errors during database directory removal
  to warnings/messages to error log.
*/

class Rmdir_error_handler : public Internal_error_handler {
 public:
  Rmdir_error_handler() : m_is_active(false) {}

  virtual bool handle_condition(THD *thd, uint, const char *,
                                Sql_condition::enum_severity_level *,
                                const char *msg) {
    if (!m_is_active) {
      /* Disable the handler to avoid infinite recursion. */
      m_is_active = true;
      push_warning_printf(thd, Sql_condition::SL_WARNING, ER_DB_DROP_RMDIR2,
                          ER_THD(thd, ER_DB_DROP_RMDIR2), msg);
      LogErr(WARNING_LEVEL, ER_DROP_DATABASE_FAILED_RMDIR_MANUALLY, msg);
      m_is_active = false;
      return true;
    }
    return false;
  }

 private:
  /**
    Indicates that we are already in the process of handling
    some error. Allows to re-emit error/warning from the error
    handler without falling into infinite recursion.
  */
  bool m_is_active;
};

/**
  Drop all tables, routines and events in a database and the database itself.

  @param  thd        Thread handle
  @param  db         Database name in the case given by user
                     It's already validated and set to lower case
                     (if needed) when we come here
  @param  if_exists  Don't give error if database doesn't exists

  @note   We do a "best effort" - try to drop as much as possible.
          If dropping the database itself fails, we try to binlog
          the drop of the tables we managed to do.

  @retval  false  OK (Database dropped)
  @retval  true   Error
*/

bool mysql_rm_db(THD *thd, const LEX_CSTRING &db, bool if_exists) {
  ulong deleted_tables = 0;
  bool error = false;
  char path[2 * FN_REFLEN + 16];
  TABLE_LIST *tables = nullptr;
  TABLE_LIST *table;
  Drop_table_error_handler err_handler;
  bool dropped_non_atomic = false;
  std::set<handlerton *> post_ddl_htons;
  Foreign_key_parents_invalidator fk_invalidator;

  DBUG_TRACE;

  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());

  // Reject dropping the system schema except for system threads.
  if (!thd->is_dd_system_thread() &&
      dd::get_dictionary()->is_dd_schema_name(dd::String_type(db.str))) {
    my_error(ER_NO_SYSTEM_SCHEMA_ACCESS, MYF(0), db.str);
    return true;
  }

  if (lock_schema_name(thd, db.str)) return true;

  if (is_thd_db_read_only_by_name(thd, db.str)) {
    my_error(ER_DB_READ_ONLY, MYF(0), db.str, "Cannot drop read only DB.");
    return true;
  }

  build_table_filename(path, sizeof(path) - 1, db.str, "", "", 0);

  DEBUG_SYNC(thd, "before_acquire_in_drop_schema");
  const dd::Schema *schema = nullptr;
  if (thd->dd_client()->acquire(db.str, &schema)) return true;

  DBUG_EXECUTE_IF("pretend_no_schema_in_drop_schema", { schema = nullptr; });

  /* See if the directory exists */
  MY_DIR *schema_dirp = my_dir(path, MYF(MY_DONT_SORT));

  auto dirender = [](MY_DIR *dirp) { my_dirend(dirp); };
  std::unique_ptr<MY_DIR, decltype(dirender)> grd{schema_dirp, dirender};

  if (schema == nullptr)  // Schema not found in DD
  {
    if (schema_dirp != nullptr)  // Schema directory exists
    {
      // This is always an error, even when if_exists is true
      my_error(ER_SCHEMA_DIR_UNKNOWN, MYF(0), db.str, path);
      return true;
    }

    if (!if_exists)  // IF EXISTS not given
    {
      my_error(ER_DB_DROP_EXISTS, MYF(0), db.str);
      return true;
    }
    push_warning_printf(thd, Sql_condition::SL_NOTE, ER_DB_DROP_EXISTS,
                        ER_THD(thd, ER_DB_DROP_EXISTS), db.str);

    /*
      We don't have active transaction at this point so we can't use
      binlog's trx cache, which requires transaction with valid XID.
    */
    if (write_db_cmd_to_binlog(thd, db.str, false)) return true;

    if (trans_commit_stmt(thd) || trans_commit_implicit(thd)) return true;

    /* Fall-through to resetting current database in connection. */
  } else  // Schema found in DD
  {
    /* Database directory does not exist. */
    if (schema_dirp == nullptr) {
      if (!if_exists) {
        my_error(ER_SCHEMA_DIR_MISSING, MYF(0), path);
        return true;
      }
      push_warning_printf(thd, Sql_condition::SL_NOTE, ER_SCHEMA_DIR_MISSING,
                          ER_THD(thd, ER_SCHEMA_DIR_MISSING), path);
    } else {
      if (find_unknown_and_remove_deletable_files(thd, schema_dirp, path)) {
        return true;
      }
    }

    if (find_db_tables(thd, *schema, db.str, &tables)) {
      return true;
    }

    /* Lock all tables and stored routines about to be dropped. */
    if (lock_table_names_nsec(thd, tables, nullptr,
                              thd->variables.lock_wait_timeout_nsec, 0) ||
        rm_table_do_discovery_and_lock_fk_tables(thd, tables) ||
        lock_check_constraint_names(thd, tables) ||
        Events::lock_schema_events(thd, *schema) ||
        lock_db_routines(thd, *schema) || lock_trigger_names(thd, tables))
      return true;

    /* mysql_ha_rm_tables() requires a non-null TABLE_LIST. */
    if (tables) mysql_ha_rm_tables(thd, tables);

    for (table = tables; table; table = table->next_local) {
      deleted_tables++;
    }

    if (thd->killed) return true;

    thd->push_internal_handler(&err_handler);
    if (tables)
      error = mysql_rm_table_no_locks(thd, tables, true, false, true,
                                      &dropped_non_atomic, &post_ddl_htons,
                                      &fk_invalidator, nullptr);

    DBUG_EXECUTE_IF("rm_db_fail_after_dropping_tables", {
      my_error(ER_UNKNOWN_ERROR, MYF(0));
      error = true;
    });

    if (!error) {
      /*
        We temporarily disable the binary log while dropping SPs
        in the database. Since the DROP DATABASE statement is always
        replicated as a statement, execution of it will drop all objects
        in the database on the slave as well, so there is no need to
        replicate the removal of the individual objects in the database
        as well.

        This is more of a safety precaution, since normally no objects
        should be dropped while the database is being cleaned, but in
        the event that a change in the code to remove other objects is
        made, these drops should still not be logged.

        Notice that the binary log have to be enabled over the call to
        ha_drop_database(), since NDB otherwise detects the binary log
        as disabled and will not log the drop database statement on any
        other connected server.
      */

      ha_drop_database(path);
      thd->clear_error(); /* @todo Do not ignore errors */
      Disable_binlog_guard binlog_guard(thd);
      error = Events::drop_schema_events(thd, *schema);
      error = (error || (sp_drop_db_routines(thd, *schema) != SP_OK));
    }
    thd->pop_internal_handler();

    if (!error) error = thd->dd_client()->drop(schema);

    // Remove db_metadata entry from all the THD's
    remove_thd_db_metadata(db.str);

    /*
      If database exists and there was no error we should
      write statement to binary log and remove DD entry.
    */
    if (!error) error = write_db_cmd_to_binlog(thd, db.str, true);

    if (!error) error = trans_commit_stmt(thd) || trans_commit(thd);

    /*
      In case of error rollback the transaction in order to revert
      changes which are possible to rollback (e.g. removal of tables
      in SEs supporting atomic DDL, events and routines).
    */
    if (error) {
      trans_rollback_stmt(thd);
      /*
        Play safe to be sure that THD::transaction_rollback_request is
        cleared before work-around code below is run. This also necessary
        to synchronize state of data-dicitionary on disk and in cache (to
        clear cache of uncommitted objects).
      */
      trans_rollback_implicit(thd);
    }

    /*
      Call post-DDL handlerton hook. For engines supporting atomic DDL
      tables' files are removed from disk on this step.
    */
    for (handlerton *hton : post_ddl_htons) hton->post_ddl(thd);

    fk_invalidator.invalidate(thd);

    /*
      Now we can try removing database directory.

      If the directory is a symbolic link, remove the link first, then
      remove the directory the symbolic link pointed at.

      This can happen only after post-DDL handlerton hook removes files
      from the directory.

      Since the statement is committed already, we do not report unlikely
      failure to remove the directory as an error. Instead we report it
      as a warning, which is sent to user and written to server error log.
    */
    if (!error && schema_dirp != nullptr) {
      Rmdir_error_handler rmdir_handler;
      thd->push_internal_handler(&rmdir_handler);
      (void)rm_dir_w_symlink(path, true);
      thd->pop_internal_handler();
    }

    if (error) {
      if (mysql_bin_log.is_open()) {
        /*
          If GTID_NEXT=='UUID:NUMBER', we must not log an incomplete
          statement.  However, the incomplete DROP has already 'committed'
          (some tables were removed).  So we generate an error and let
          user fix the situation.
        */
        if (thd->variables.gtid_next.type == ASSIGNED_GTID &&
            dropped_non_atomic) {
          char gtid_buf[Gtid::MAX_TEXT_LENGTH + 1];
          thd->variables.gtid_next.gtid.to_string(global_sid_map, gtid_buf,
                                                  true);
          my_error(ER_CANNOT_LOG_PARTIAL_DROP_DATABASE_WITH_GTID, MYF(0), path,
                   gtid_buf, db.str);
          return true;
        }
      }
      return true;
    }

    del_thd_db_read_only(db.str);
    del_thd_db_default_collation(db.str);
  }

  /*
    If this database was the client's selected database, we silently
    change the client's selected database to nothing (to have an empty
    SELECT DATABASE() in the future). For this we free() thd->db and set
    it to 0.
  */
  if (thd->db().str && !strcmp(thd->db().str, db.str)) {
    mysql_change_db_impl(thd, NULL_CSTR, 0, thd->variables.collation_server);
    /*
      Check if current database tracker is enabled. If so, set the 'changed'
      flag.
    */
    if (thd->session_tracker.get_tracker(CURRENT_SCHEMA_TRACKER)
            ->is_enabled()) {
      LEX_CSTRING dummy = {STRING_WITH_LEN("")};
      dummy.length = dummy.length * 1;
      thd->session_tracker.get_tracker(CURRENT_SCHEMA_TRACKER)
          ->mark_as_changed(thd, &dummy);
    }
  }

  if (!error) {
    db_ac->remove(db.str);

    Change_db_callback callback = change_db_callback;
    if (callback) {
      callback(thd, db, true /*drop*/);
    }
  }

  thd->server_status |= SERVER_STATUS_DB_DROPPED;
  my_ok(thd, deleted_tables);
  return false;
}

/**
  Auxiliary function which checks if database directory has any
  files which won't be deleted automatically - either because
  we know that these are temporary/backup files which are safe
  for delete or by dropping the tables in the database.
  Also deletes various temporary/backup files which are known to
  be safe to delete.
*/

static bool find_unknown_and_remove_deletable_files(THD *thd, MY_DIR *dirp,
                                                    const char *path) {
  char filePath[FN_REFLEN];
  DBUG_TRACE;
  DBUG_PRINT("enter", ("path: %s", path));
  TYPELIB *known_extensions = ha_known_exts();

  for (uint idx = 0; idx < dirp->number_off_files && !thd->killed; idx++) {
    FILEINFO *file = dirp->dir_entry + idx;
    char *extension;
    DBUG_PRINT("info", ("Examining: %s", file->name));

    /* skiping . and .. */
    if (file->name[0] == '.' &&
        (!file->name[1] || (file->name[1] == '.' && !file->name[2])))
      continue;

    if (file->name[0] == 'a' && file->name[1] == 'r' && file->name[2] == 'c' &&
        file->name[3] == '\0') {
      /* .frm archive:
        Those archives are obsolete, but following code should
        exist to remove existent "arc" directories.
      */
      char newpath[FN_REFLEN];
      MY_DIR *new_dirp;
      strxmov(newpath, path, "/", "arc", NullS);
      (void)unpack_filename(newpath, newpath);
      if ((new_dirp = my_dir(newpath, MYF(MY_DONT_SORT)))) {
        DBUG_PRINT("my", ("Archive subdir found: %s", newpath));
        if ((mysql_rm_arc_files(thd, new_dirp, newpath)) < 0) return true;
        continue;
      }
      goto found_other_files;
    }
    if (!(extension = strrchr(file->name, '.'))) extension = strend(file->name);
    if (find_type(extension, &deletable_extentions, FIND_TYPE_NO_PREFIX) <= 0) {
      if (find_type(extension, known_extensions, FIND_TYPE_NO_PREFIX) <= 0)
        goto found_other_files;
      continue;
    }
    strxmov(filePath, path, "/", file->name, NullS);
    /*
      We ignore ENOENT error in order to skip files that was deleted
      by concurrently running statement like REAPIR TABLE ...
    */
    if (my_delete_with_symlink(filePath, MYF(0)) && my_errno() != ENOENT) {
      char errbuf[MYSYS_STRERROR_SIZE];
      my_error(EE_DELETE, MYF(0), filePath, my_errno(),
               my_strerror(errbuf, sizeof(errbuf), my_errno()));
      return true;
    }
  }

  return false;

found_other_files:
  char errbuf[MYSQL_ERRMSG_SIZE];
  my_error(ER_DB_DROP_RMDIR, MYF(0), path, EEXIST,
           my_strerror(errbuf, MYSQL_ERRMSG_SIZE, EEXIST));
  return true;
}

/**
  Auxiliary function which retrieves list of all tables in the database
  from the data-dictionary.
*/

static bool find_db_tables(THD *thd, const dd::Schema &schema, const char *db,
                           TABLE_LIST **tables) {
  TABLE_LIST *tot_list = nullptr, **tot_list_next_local, **tot_list_next_global;
  DBUG_TRACE;

  tot_list_next_local = tot_list_next_global = &tot_list;

  std::vector<dd::String_type> sch_tables;
  /*
    Skip tables which are implicitly created and dropped by SE (e.g.
    InnoDB's auxiliary tables for FTS). Other hidden tables (e.g.
    left-over #sql... tables from crashed non-atomic ALTER TABLEs)
    should be dropped by DROP DATABASE.
  */
  if (thd->dd_client()->fetch_schema_table_names_not_hidden_by_se(&schema,
                                                                  &sch_tables))
    return true;

  for (const dd::String_type &table_name : sch_tables) {
    TABLE_LIST *table_list = new (thd->mem_root) TABLE_LIST;
    if (table_list == nullptr) return true; /* purecov: inspected */

    table_list->db = thd->mem_strdup(db);
    table_list->db_length = strlen(db);
    table_list->table_name = thd->mem_strdup(table_name.c_str());
    table_list->table_name_length = table_name.length();

    table_list->open_type = OT_BASE_ONLY;

    /* To be able to correctly look up the table in the table cache. */
    if (lower_case_table_names)
      my_casedn_str(files_charset_info,
                    const_cast<char *>(table_list->table_name));

    table_list->alias = table_list->table_name;  // If lower_case_table_names=2
    table_list->internal_tmp_table =
        is_prefix(table_name.c_str(), tmp_file_prefix);
    MDL_REQUEST_INIT(&table_list->mdl_request, MDL_key::TABLE, table_list->db,
                     table_list->table_name, MDL_EXCLUSIVE, MDL_TRANSACTION);
    /* Link into list */
    (*tot_list_next_local) = table_list;
    (*tot_list_next_global) = table_list;
    tot_list_next_local = &table_list->next_local;
    tot_list_next_global = &table_list->next_global;
  }

  *tables = tot_list;
  return false;
}

/*
  Remove directory with symlink

  SYNOPSIS
    rm_dir_w_symlink()
    org_path    path of derictory
    send_error  send errors
  RETURN
    0 OK
    1 ERROR
*/

static bool rm_dir_w_symlink(const char *org_path, bool send_error) {
  char tmp_path[FN_REFLEN], *pos;
  char *path = tmp_path;
  DBUG_TRACE;
  unpack_filename(tmp_path, org_path);
#ifndef _WIN32
  int error;
  char tmp2_path[FN_REFLEN];

  /* Remove end FN_LIBCHAR as this causes problem on Linux in readlink */
  pos = strend(path);
  if (pos > path && pos[-1] == FN_LIBCHAR) *--pos = 0;

  if ((error = my_readlink(tmp2_path, path, MYF(MY_WME))) < 0) return true;
  if (!error) {
    if (mysql_file_delete(key_file_misc, path, MYF(send_error ? MY_WME : 0))) {
      return send_error;
    }
    /* Delete directory symbolic link pointed at */
    path = tmp2_path;
  }
#endif
  /* Remove last FN_LIBCHAR to not cause a problem on OS/2 */
  pos = strend(path);

  if (pos > path && pos[-1] == FN_LIBCHAR) *--pos = 0;
  if (rmdir(path) < 0 && send_error) {
    char errbuf[MYSQL_ERRMSG_SIZE];
    my_error(ER_DB_DROP_RMDIR, MYF(0), path, errno,
             my_strerror(errbuf, MYSQL_ERRMSG_SIZE, errno));
    return true;
  }
  return false;
}

/*
  Remove .frm archives from directory

  SYNOPSIS
    thd       thread handler
    dirp      list of files in archive directory
    db        data base name
    org_path  path of archive directory

  RETURN
    > 0 number of removed files
    -1  error

  NOTE
    A support of "arc" directories is obsolete, however this
    function should exist to remove existent "arc" directories.
*/
long mysql_rm_arc_files(THD *thd, MY_DIR *dirp, const char *org_path) {
  long deleted = 0;
  ulong found_other_files = 0;
  char filePath[FN_REFLEN];
  DBUG_TRACE;
  DBUG_PRINT("enter", ("path: %s", org_path));

  for (uint idx = 0; idx < dirp->number_off_files && !thd->killed; idx++) {
    FILEINFO *file = dirp->dir_entry + idx;
    char *extension, *revision;
    DBUG_PRINT("info", ("Examining: %s", file->name));

    /* skiping . and .. */
    if (file->name[0] == '.' &&
        (!file->name[1] || (file->name[1] == '.' && !file->name[2])))
      continue;

    extension = fn_ext(file->name);
    if (extension[0] != '.' || extension[1] != 'f' || extension[2] != 'r' ||
        extension[3] != 'm' || extension[4] != '-') {
      found_other_files++;
      continue;
    }
    revision = extension + 5;
    while (*revision && my_isdigit(system_charset_info, *revision)) revision++;
    if (*revision) {
      found_other_files++;
      continue;
    }
    strxmov(filePath, org_path, "/", file->name, NullS);
    if (mysql_file_delete_with_symlink(key_file_misc, filePath, MYF(MY_WME))) {
      goto err;
    }
    deleted++;
  }
  if (thd->killed) goto err;

  my_dirend(dirp);

  /*
    If the directory is a symbolic link, remove the link first, then
    remove the directory the symbolic link pointed at
  */
  if (!found_other_files && rm_dir_w_symlink(org_path, false)) return -1;
  return deleted;

err:
  my_dirend(dirp);
  return -1;
}

/**
  @brief Internal implementation: switch current database to a valid one.

  @param thd            Thread context.
  @param new_db_name    Name of the database to switch to. The function will
                        take ownership of the name (the caller must not free
                        the allocated memory). If the name is NULL, we're
                        going to switch to NULL db.
  @param new_db_access  Privileges of the new database. (with roles)
  @param new_db_charset Character set of the new database.
*/

static void mysql_change_db_impl(THD *thd, const LEX_CSTRING &new_db_name,
                                 ulong new_db_access,
                                 const CHARSET_INFO *new_db_charset) {
  /* 1. Change current database in THD. */

  if (new_db_name.str == nullptr) {
    /*
      THD::set_db() does all the job -- it frees previous database name and
      sets the new one.
    */

    thd->set_db(NULL_CSTR);
  } else if (!strcmp(new_db_name.str, INFORMATION_SCHEMA_NAME.str)) {
    /*
      Here we must use THD::set_db(), because we want to copy
      INFORMATION_SCHEMA_NAME constant.
    */

    thd->set_db(INFORMATION_SCHEMA_NAME);
  } else {
    /*
      Here we already have a copy of database name to be used in THD. So,
      we just call THD::reset_db(). Since THD::reset_db() does not releases
      the previous database name, we should do it explicitly.
    */
    mysql_mutex_lock(&thd->LOCK_thd_data);
    if (thd->db().str) my_free(const_cast<char *>(thd->db().str));
    DEBUG_SYNC(thd, "after_freeing_thd_db");
    thd->reset_db(new_db_name, /* lock_held_skip_metadata */ true);
    mysql_mutex_unlock(&thd->LOCK_thd_data);
    /* Explicitly call set_db_metadata after lock is released */
    thd->set_db_metadata();
  }

  /* 2. Update security context. */

  /* Cache the effective schema level privilege with roles applied */
  thd->security_context()->cache_current_db_access(new_db_access);

  /* 3. Update db-charset environment variables. */

  thd->db_charset = new_db_charset;
  thd->variables.collation_database = new_db_charset;
}

/**
  Backup the current database name before switch.

  @param[in]      thd             thread handle
  @param[in, out] saved_db_name   IN: "str" points to a buffer where to store
                                  the old database name, "length" contains the
                                  buffer size
                                  OUT: if the current (default) database is
                                  not NULL, its name is copied to the
                                  buffer pointed at by "str"
                                  and "length" is updated accordingly.
                                  Otherwise "str" is set to NULL and
                                  "length" is set to 0.
*/

static void backup_current_db_name(THD *thd, LEX_STRING *saved_db_name) {
  if (!thd->db().str) {
    /* No current (default) database selected. */

    saved_db_name->str = nullptr;
    saved_db_name->length = 0;
  } else {
    strmake(saved_db_name->str, thd->db().str, saved_db_name->length - 1);
    saved_db_name->length = thd->db().length;
  }
}

/**
  Return true if db1_name is equal to db2_name, false otherwise.

  The function allows to compare database names according to the MySQL
  rules. The database names db1 and db2 are equal if:
     - db1 is NULL and db2 is NULL;
     or
     - db1 is not-NULL, db2 is not-NULL, db1 is equal (ignoring case) to
       db2 in system character set (UTF8).
*/

static inline bool cmp_db_names(const char *db1_name, const char *db2_name) {
  return
      /* db1 is NULL and db2 is NULL */
      (!db1_name && !db2_name) ||

      /* db1 is not-NULL, db2 is not-NULL, db1 == db2. */
      (db1_name && db2_name &&
       my_strcasecmp(system_charset_info, db1_name, db2_name) == 0);
}

/**
  @brief Change the current database and its attributes unconditionally.

  @param thd          thread handle
  @param new_db_name  database name
  @param force_switch if force_switch is false, then the operation will fail if

                        - new_db_name is NULL or empty;

                        - OR new database name is invalid
                          (check_db_name() failed);

                        - OR user has no privilege on the new database;

                        - OR new database does not exist;

                      if force_switch is true, then

                        - if new_db_name is NULL or empty, the current
                          database will be NULL, @@collation_database will
                          be set to @@collation_server, the operation will
                          succeed.

                        - if new database name is invalid
                          (check_db_name() failed), the current database
                          will be NULL, @@collation_database will be set to
                          @@collation_server, but the operation will fail;

                        - user privileges will not be checked
                          (THD::db_access however is updated);

                          TODO: is this really the intention?
                                (see sp-security.test).

                        - if new database does not exist,the current database
                          will be NULL, @@collation_database will be set to
                          @@collation_server, a warning will be thrown, the
                          operation will succeed.

  @details The function checks that the database name corresponds to a
  valid and existent database, checks access rights and changes the current
  database with database attributes (@@collation_database session variable,
  THD::db_access).

  This function is not the only way to switch the database that is
  currently employed. When the replication slave thread switches the
  database before executing a query, it calls thd->set_db directly.
  However, if the query, in turn, uses a stored routine, the stored routine
  will use this function, even if it's run on the slave.

  This function allocates the name of the database on the system heap: this
  is necessary to be able to uniformly change the database from any module
  of the server. Up to 5.0 different modules were using different memory to
  store the name of the database, and this led to memory corruption:
  a stack pointer set by Stored Procedures was used by replication after
  the stack address was long gone.

  @return Operation status
    @retval false Success
    @retval true  Error
*/

bool mysql_change_db(THD *thd, const LEX_CSTRING &new_db_name,
                     bool force_switch) {
  LEX_STRING new_db_file_name;
  LEX_CSTRING new_db_file_name_cstr;

  Security_context *sctx = thd->security_context();
  ulong db_access = sctx->current_db_access();
  const CHARSET_INFO *db_default_cl = nullptr;

  // We must make sure the schema is released and unlocked in the right order.
  dd::Schema_MDL_locker mdl_handler(thd);
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  const dd::Schema *schema = nullptr;

  DBUG_TRACE;
  DBUG_PRINT("enter", ("name: '%s'", new_db_name.str));

  if (new_db_name.str == nullptr || new_db_name.length == 0) {
    if (force_switch) {
      /*
        This can happen only if we're switching the current database back
        after loading stored program. The thing is that loading of stored
        program can happen when there is no current database.

        TODO: actually, new_db_name and new_db_name->str seem to be always
        non-NULL. In case of stored program, new_db_name->str == "" and
        new_db_name->length == 0.
      */

      mysql_change_db_impl(thd, NULL_CSTR, 0, thd->variables.collation_server);

      goto done;
    } else {
      my_error(ER_NO_DB_ERROR, MYF(0));

      return true;
    }
  }

  if (is_infoschema_db(new_db_name.str, new_db_name.length)) {
    /* Switch the current database to INFORMATION_SCHEMA. */

    mysql_change_db_impl(thd, INFORMATION_SCHEMA_NAME, SELECT_ACL,
                         system_charset_info);
    goto done;
  }

  /*
    Now we need to make a copy because check_db_name requires a
    non-constant argument. Actually, it takes database file name.

    TODO: fix check_db_name().
  */

  new_db_file_name.str = my_strndup(key_memory_THD_db, new_db_name.str,
                                    new_db_name.length, MYF(MY_WME));
  new_db_file_name.length = new_db_name.length;

  if (new_db_file_name.str == nullptr) return true; /* the error is set */

  /*
    NOTE: if check_db_name() fails, we should throw an error in any case,
    even if we are called from sp_head::execute().

    It's next to impossible however to get this error when we are called
    from sp_head::execute(). But let's switch the current database to NULL
    in this case to be sure.
  */

  if (check_and_convert_db_name(&new_db_file_name, false) !=
      Ident_name_check::OK) {
    my_free(new_db_file_name.str);

    if (force_switch)
      mysql_change_db_impl(thd, NULL_CSTR, 0, thd->variables.collation_server);
    return true;
  }
  new_db_file_name_cstr.str = new_db_file_name.str;
  new_db_file_name_cstr.length = new_db_file_name.length;
  DBUG_PRINT("info", ("Use database: %s", new_db_file_name.str));

  if (sctx->get_active_roles()->size() == 0) {
    db_access =
        sctx->check_access(DB_OP_ACLS, new_db_file_name.str)
            ? DB_OP_ACLS
            : acl_get(thd, sctx->host().str, sctx->ip().str,
                      sctx->priv_user().str, new_db_file_name.str, false) |
                  sctx->master_access(new_db_file_name.str);
  } else {
    db_access = sctx->db_acl(new_db_file_name_cstr) |
                sctx->master_access(new_db_file_name.str);
  }

  if (!force_switch && !(db_access & DB_OP_ACLS) &&
      check_grant_db(thd, new_db_file_name.str)) {
    my_error(ER_DBACCESS_DENIED_ERROR, MYF(0), sctx->priv_user().str,
             sctx->priv_host().str, new_db_file_name.str);
    query_logger.general_log_print(
        thd, COM_INIT_DB, ER_DEFAULT(ER_DBACCESS_DENIED_ERROR),
        sctx->priv_user().str, sctx->priv_host().str, new_db_file_name.str);
    my_free(new_db_file_name.str);
    return true;
  }

  if (mdl_handler.ensure_locked(new_db_file_name.str) ||
      thd->dd_client()->acquire(new_db_file_name.str, &schema)) {
    my_free(new_db_file_name.str);
    return true;
  }

  DEBUG_SYNC(thd, "acquired_schema_while_getting_collation");

  if (schema == nullptr) {
    if (force_switch) {
      /* Throw a warning and free new_db_file_name. */

      push_warning_printf(thd, Sql_condition::SL_NOTE, ER_BAD_DB_ERROR,
                          ER_THD(thd, ER_BAD_DB_ERROR), new_db_file_name.str);

      my_free(new_db_file_name.str);

      /* Change db to NULL. */
      mysql_change_db_impl(thd, NULL_CSTR, 0, thd->variables.collation_server);

      /* The operation succeed. */
      goto done;
    } else {
      /* Report an error and free new_db_file_name. */

      my_error(ER_BAD_DB_ERROR, MYF(0), new_db_file_name.str);
      my_free(new_db_file_name.str);

      /* The operation failed. */

      return true;
    }
  }

  if (get_default_db_collation(*schema, &db_default_cl)) {
    my_free(new_db_file_name.str);
    DBUG_ASSERT(thd->is_error() || thd->killed);
    return true;
  }

  db_default_cl = db_default_cl ? db_default_cl : thd->collation();
  /*
    NOTE: in mysql_change_db_impl() new_db_file_name is assigned to THD
    attributes and will be freed in THD::~THD().
  */
  mysql_change_db_impl(thd, new_db_file_name_cstr, db_access, db_default_cl);

done:
  /*
    Check if current database tracker is enabled. If so, set the 'changed' flag.
  */
  if (thd->session_tracker.get_tracker(CURRENT_SCHEMA_TRACKER)->is_enabled()) {
    LEX_CSTRING dummy = {STRING_WITH_LEN("")};
    dummy.length = dummy.length * 1;
    thd->session_tracker.get_tracker(CURRENT_SCHEMA_TRACKER)
        ->mark_as_changed(thd, &dummy);
  }
  if (thd->session_tracker.get_tracker(SESSION_STATE_CHANGE_TRACKER)
          ->is_enabled())
    thd->session_tracker.get_tracker(SESSION_STATE_CHANGE_TRACKER)
        ->mark_as_changed(thd, nullptr);
  if (thd->session_tracker.get_tracker(SESSION_RESP_ATTR_TRACKER)
          ->is_enabled() &&
      thd->db().length > 0) {
    static LEX_CSTRING key = {STRING_WITH_LEN("change_db")};
    thd->session_tracker.get_tracker(SESSION_RESP_ATTR_TRACKER)
        ->mark_as_changed(thd, &key, &thd->db());
  }
  return false;
}

/**
  Change the current database and its attributes if needed.

  @param          thd             thread handle
  @param          new_db_name     database name
  @param[in, out] saved_db_name   IN: "str" points to a buffer where to store
                                  the old database name, "length" contains the
                                  buffer size
                                  OUT: if the current (default) database is
                                  not NULL, its name is copied to the
                                  buffer pointed at by "str"
                                  and "length" is updated accordingly.
                                  Otherwise "str" is set to NULL and
                                  "length" is set to 0.
  @param          force_switch    @see mysql_change_db()
  @param[out]     cur_db_changed  out-flag to indicate whether the current
                                  database has been changed (valid only if
                                  the function suceeded)
*/

bool mysql_opt_change_db(THD *thd, const LEX_CSTRING &new_db_name,
                         LEX_STRING *saved_db_name, bool force_switch,
                         bool *cur_db_changed) {
  *cur_db_changed = !cmp_db_names(thd->db().str, new_db_name.str);

  if (!*cur_db_changed) return false;

  backup_current_db_name(thd, saved_db_name);

  return mysql_change_db(thd, new_db_name, force_switch);
}

/**
  Set change_db_callback notified when db of connection is set or changed.

  @param new_callback New callback to set.
*/
void set_change_db_callback(Change_db_callback new_callback) {
  change_db_callback = new_callback;
}

/**
 * This function is called for the following commands to set
 * the session's default database: COM_INIT_DB, SQLCOM_CHANGE_DB,
 * COM_CONNECT, COM_CHANGE_USER. In this function, we will check the
 * admission control and connection limit for the new database.
 *
 * @retval
 *   0   ok
 * @retval
 *   1   failed to set session db
 */
bool set_session_db_helper(THD *thd, const LEX_CSTRING &new_db) {
  bool error = false;
  bool is_max_db_conn_enabled = opt_max_db_connections != 0;
  Change_db_callback callback = change_db_callback;

  // Order callback and server db changes: if max_db_connections is enabled on
  // server then server goes first; otherwise callback goes first. This is
  // needed to avoid the case when the callback switches to the new db but
  // later the server fails the call due to MT limit but the switch is already
  // done. If the callback always goes last then the same scenario is possible
  // with the roles reversed. So whoever can fail should go first, and the
  // other would see the error and avoid the switch.
  if (callback && !is_max_db_conn_enabled) {
    error = callback(thd, new_db, false /*drop*/);
  }

  if (!error) {
    Ac_switch_guard switch_guard(thd);
    error = switch_guard.add_connection(new_db.str);

    if (error) {
      // Connection rejected/throttled. Push error message.
      std::string entity(new_db.str);
      if (thd->security_context()->host_or_ip().str) {
        if (!entity.empty()) entity += " on ";
        entity += thd->security_context()->host_or_ip().str;
      }

      my_error(ER_MULTI_TENANCY_MAX_CONNECTION, MYF(0), entity.c_str());
    } else {
      // Switch to new db if it is specified.
      if (new_db.length)
        error = mysql_change_db(thd, new_db, false);  // Do not force switch.

      if (!error)
        // No db, or successful switch, so mark guard to keep the changes.
        switch_guard.commit();
    }
  }

  if (!error && callback && is_max_db_conn_enabled) {
    error = callback(thd, new_db, false /*drop*/);
  }

  // Switch guard either commits or rolls back the switch operation.
  return error;
}
