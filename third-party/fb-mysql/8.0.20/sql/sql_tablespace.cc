/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/sql_tablespace.h"

#include <string.h>
#include <memory>
#include <string>
#include <utility>

#include "m_ctype.h"
#include "my_base.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_sys.h"
#include "mysql_com.h"
#include "mysqld.h"  // opt_table_encryption_privilege_check
#include "mysqld_error.h"
#include "sql/auth/auth_acls.h"  // CREATE_TABLESPACE_ACL
#include "sql/auth/auth_common.h"
#include "sql/dd/cache/dictionary_client.h"  // dd::Dictionary_client
#include "sql/dd/dd.h"                       // dd::create_object
#include "sql/dd/dd_kill_immunizer.h"        // dd::DD_kill_immunizer
#include "sql/dd/dd_table.h"                 // dd::is_encrypted
#include "sql/dd/impl/sdi_utils.h"           // dd::sdi_utils::make_guard
#include "sql/dd/properties.h"
#include "sql/dd/string_type.h"
#include "sql/dd/types/table.h"            // dd::fetch_tablespace_table_refs
#include "sql/dd/types/tablespace.h"       // dd::fetch_tablespace_table_refs
#include "sql/dd/types/tablespace_file.h"  // dd::Tablespace_file
#include "sql/debug_sync.h"                // DBUG_SYNC
#include "sql/derror.h"                    // ER_THD
#include "sql/handler.h"                   // st_alter_tablespace
#include "sql/item_strfunc.h"              // mysql_generate_uuid
#include "sql/mdl.h"
#include "sql/parse_tree_helpers.h"  // resolve_engine
#include "sql/sql_base.h"            // TDC_RT_REMOVE_ALL
#include "sql/sql_class.h"           // THD
#include "sql/sql_const.h"
#include "sql/sql_error.h"
#include "sql/sql_plugin_ref.h"
#include "sql/sql_table.h"  // write_bin_log
#include "sql/strfunc.h"    // lex_cstring_handle
#include "sql/system_variables.h"
#include "sql/thd_raii.h"
#include "sql/transaction.h"  // trans_commit_stmt

namespace {

template <typename LEXSTR>
bool validate_tspnamelen(const LEXSTR &name) {
  if (name.length == 0) {
    // Empty name not allowed
    my_error(ER_WRONG_TABLESPACE_NAME, MYF(0), name.str);
    return true;
  }

  if (name.length > NAME_LEN ||
      my_numchars_mb(system_charset_info, name.str, name.str + name.length) >
          NAME_CHAR_LEN) {
    // Byte length exceeding NAME_LEN, and character lenght exceeding
    // NAME_CHAR_LEN not allowed
    my_error(ER_TOO_LONG_IDENT, MYF(0), name.str);
    return true;
  }

  return false;
}
}  // namespace

st_alter_tablespace::st_alter_tablespace(
    const char *tablespace, const char *logfile_group, ts_command_type cmd,
    enum ts_alter_tablespace_type alter_tablespace_cmd, const char *datafile,
    const char *undofile, const Tablespace_options &opts)
    : tablespace_name{tablespace},
      logfile_group_name{logfile_group},
      ts_cmd_type{cmd},
      ts_alter_tablespace_type{alter_tablespace_cmd},
      data_file_name{datafile},
      undo_file_name{undofile},
      // Propagate Tablespace options from parser
      extent_size{opts.extent_size},
      undo_buffer_size{opts.undo_buffer_size},
      redo_buffer_size{opts.redo_buffer_size},
      initial_size{opts.initial_size},
      autoextend_size{opts.autoextend_size},
      max_size{opts.max_size},
      file_block_size{opts.file_block_size},
      nodegroup_id{opts.nodegroup_id},
      wait_until_completed{opts.wait_until_completed},
      ts_comment{opts.ts_comment.str} {}

bool validate_tablespace_name_length(const char *tablespace_name) {
  DBUG_ASSERT(tablespace_name != nullptr);
  LEX_CSTRING tspname = {tablespace_name, strlen(tablespace_name)};
  return validate_tspnamelen(tspname);
}

bool validate_tablespace_name(ts_command_type ts_cmd,
                              const char *tablespace_name,
                              const handlerton *engine) {
  DBUG_ASSERT(tablespace_name != nullptr);
  DBUG_ASSERT(engine != nullptr);

  // Length must be valid.
  if (validate_tablespace_name_length(tablespace_name)) {
    my_error(ER_WRONG_TABLESPACE_NAME, MYF(0), tablespace_name);
    return true;
  }

  // Invoke SE specific validation of the name.
  if (engine->is_valid_tablespace_name != nullptr &&
      !engine->is_valid_tablespace_name(ts_cmd, tablespace_name)) {
    my_error(ER_WRONG_TABLESPACE_NAME, MYF(0), tablespace_name);
    return true;
  }

  return false;
}

namespace {
class Rollback_guard {
  THD *m_thd;
  bool m_disabled = false;

 public:
  handlerton *m_hton = nullptr;

  explicit Rollback_guard(THD *thd) : m_thd{thd} {}
  ~Rollback_guard() {
    if (m_disabled) {
      return;
    }
    trans_rollback_stmt(m_thd);
    // Full rollback in case we have THD::transaction_rollback_request.
    trans_rollback(m_thd);

    if (m_hton != nullptr && ddl_is_atomic(m_hton) &&
        m_hton->post_ddl != nullptr) {
      m_hton->post_ddl(m_thd);
    }
  }
  void disable() { m_disabled = true; }
};

template <typename DISABLE_ROLLBACK>
bool complete_stmt(THD *thd, handlerton *hton, DISABLE_ROLLBACK &&dr,
                   bool using_trans = true, bool dont_write_to_binlog = false) {
  if (!dont_write_to_binlog)
    if (write_bin_log(thd, false, thd->query().str, thd->query().length,
                      using_trans && ddl_is_atomic(hton))) {
      return true;
    }

  /* Commit the statement and call storage engine's post-DDL hook. */
  if (trans_commit_stmt(thd) || trans_commit(thd)) {
    return true;
  }

  dr();

  if (hton && ddl_is_atomic(hton) && hton->post_ddl) {
    hton->post_ddl(thd);
  }

  my_ok(thd);
  return false;
}

bool lock_rec(THD *thd, MDL_request_list *rlst, const LEX_STRING &tsp) {
  if (validate_tspnamelen(tsp)) {
    return true;
  }
  MDL_request tail_request;
  MDL_REQUEST_INIT(&tail_request, MDL_key::TABLESPACE, "", tsp.str,
                   MDL_EXCLUSIVE, MDL_TRANSACTION);
  rlst->push_front(&tail_request);

  MDL_request global_request;
  MDL_REQUEST_INIT(&global_request, MDL_key::GLOBAL, "", "",
                   MDL_INTENTION_EXCLUSIVE, MDL_STATEMENT);
  rlst->push_front(&global_request);

  MDL_request backup_lock_request;

  MDL_REQUEST_INIT(&backup_lock_request, MDL_key::BACKUP_LOCK, "", "",
                   MDL_INTENTION_EXCLUSIVE, MDL_TRANSACTION);
  rlst->push_front(&backup_lock_request);

  if (thd->mdl_context.acquire_locks_nsec(
          rlst, thd->variables.lock_wait_timeout_nsec))
    return true;

  /*
    Now when we have protection against concurrent change of read_only
    option we can safely re-check its value.
  */
  if (check_readonly(thd, true)) return true;

  return false;
}

template <typename... Names>
bool lock_rec(THD *thd, MDL_request_list *rlst, const LEX_STRING &tsp,
              Names... names) {
  if (validate_tspnamelen(tsp)) {
    return true;
  }
  MDL_request request;
  MDL_REQUEST_INIT(&request, MDL_key::TABLESPACE, "", tsp.str, MDL_EXCLUSIVE,
                   MDL_TRANSACTION);
  rlst->push_front(&request);
  return lock_rec(thd, rlst, names...);
}

template <typename... Names>
bool lock_tablespace_names(THD *thd, Names... names) {
  MDL_request_list mdl_requests;
  if (thd->locked_tables_mode) {
    my_error(ER_LOCK_OR_ACTIVE_TRANSACTION, MYF(0));
    return true;
  }

  if (thd->global_read_lock.can_acquire_protection()) {
    return true;
  }

  if (lock_rec(thd, &mdl_requests, names...)) {
    return true;
  }

  size_t nts = sizeof...(names);
  if (nts == 1) {
    DEBUG_SYNC(thd, "after_wait_locked_tablespace_name");
  } else {
    DEBUG_SYNC(thd, "after_wait_locked_tablespace_name_for_table");
  }
  return false;
}

template <typename T>
using Mod_pair = std::pair<const T *, T *>;
typedef std::vector<Mod_pair<dd::Table>> Table_pair_list;

template <typename T>
Mod_pair<T> get_mod_pair(dd::cache::Dictionary_client *dcp,
                         const dd::String_type &name) {
  Mod_pair<T> ret{nullptr, nullptr};
  if (dcp->acquire(name, &ret.first)) {
    return {nullptr, nullptr};
  }
  if (ret.first == nullptr) {
    return {nullptr, nullptr};
  }
  if (dcp->acquire_for_modification(name, &ret.second)) {
    return {nullptr, nullptr};
  }
  DBUG_ASSERT(ret.second != nullptr);
  return ret;
}

template <typename T>
Mod_pair<T> get_mod_pair(dd::cache::Dictionary_client *dcp,
                         const dd::String_type &sch_name,
                         const dd::String_type &name) {
  Mod_pair<T> ret{nullptr, nullptr};
  if (dcp->acquire(sch_name, name, &ret.first)) {
    return {nullptr, nullptr};
  }
  if (ret.first == nullptr) {
    return {nullptr, nullptr};
  }
  if (dcp->acquire_for_modification(sch_name, name, &ret.second)) {
    return {nullptr, nullptr};
  }
  DBUG_ASSERT(ret.second != nullptr);
  return ret;
}

const char *real_engine_name(THD *thd, const LEX_CSTRING &alias) {
  plugin_ref pr = ha_resolve_by_name(thd, &alias, false);
  handlerton *hton = (pr != nullptr ? plugin_data<handlerton *>(pr) : nullptr);
  return hton != nullptr ? ha_resolve_storage_engine_name(hton) : "";
}

bool get_stmt_hton(THD *thd, const LEX_CSTRING &engine, const char *object_name,
                   const char *statement, handlerton **htonp) {
  handlerton *hton = nullptr;
  if (engine.str != nullptr &&
      resolve_engine(thd, engine, false, false, &hton)) {
    return true;
  }
  if (hton == nullptr || hton->state != SHOW_OPTION_YES) {
    hton = ha_default_handlerton(thd);
    if (engine.str != nullptr) {
      push_warning_printf(thd, Sql_condition::SL_WARNING,
                          ER_WARN_USING_OTHER_HANDLER,
                          ER_THD(thd, ER_WARN_USING_OTHER_HANDLER),
                          ha_resolve_storage_engine_name(hton), object_name);
    }
  }

  // Check if tablespace operation is disallowed by the storage engine.
  if (ha_is_storage_engine_disabled(hton)) {
    my_error(ER_DISABLED_STORAGE_ENGINE, MYF(0),
             ha_resolve_storage_engine_name(hton));
    return true;
  }
  if (hton->alter_tablespace == nullptr) {
    my_error(ER_ILLEGAL_HA_CREATE_OPTION, MYF(0),
             ha_resolve_storage_engine_name(hton), statement);
    return true;
  }
  *htonp = hton;
  return false;
}

bool get_dd_hton(THD *thd, const dd::String_type &dd_engine,
                 const LEX_CSTRING &stmt_engine, const char *tblspc,
                 const char *stmt, handlerton **htonp) {
  if (stmt_engine.str && dd_engine != real_engine_name(thd, stmt_engine)) {
    my_error(ER_TABLESPACE_ENGINE_MISMATCH, MYF(0), stmt_engine.str,
             dd_engine.c_str(), tblspc);
    return true;
  }
  if (stmt_engine.str != nullptr) {
    push_warning_printf(thd, Sql_condition::SL_WARNING,
                        ER_WARN_DEPRECATED_SYNTAX_NO_REPLACEMENT,
                        ER_THD(thd, ER_WARN_DEPRECATED_SYNTAX_NO_REPLACEMENT),
                        "ENGINE tablespace option");
  }

  plugin_ref pr = ha_resolve_by_name_raw(thd, lex_cstring_handle(dd_engine));
  handlerton *hton = (pr != nullptr ? plugin_data<handlerton *>(pr) : nullptr);
  if (hton == nullptr) {
    my_error(ER_UNKNOWN_STORAGE_ENGINE, MYF(0), dd_engine.c_str());
    return true;
  }

  DBUG_ASSERT(hton->alter_tablespace);
  if (hton->alter_tablespace == nullptr) {
    my_error(ER_ILLEGAL_HA_CREATE_OPTION, MYF(0), dd_engine.c_str(), stmt);
    return true;
  }

  *htonp = hton;
  return false;
}

bool intermediate_commit_unless_atomic_ddl(THD *thd, handlerton *hton) {
  if (ddl_is_atomic(hton)) {
    return false;
  }
  /* purecov: begin inspected */
  Implicit_substatement_state_guard substatement_guard{thd};
  return (trans_commit_stmt(thd) || trans_commit(thd));
  /* purecov: end */
}

bool map_errors(int se_error, const char *statement_txt,
                const st_alter_tablespace *ts_info) {
  switch (se_error) {
    case 0:
      return false;
    case 1:
      return true;
    case HA_ADMIN_NOT_IMPLEMENTED:
      my_error(ER_CHECK_NOT_IMPLEMENTED, MYF(0), statement_txt);
      break;
    case HA_ERR_TABLESPACE_MISSING:
      my_error(ER_TABLESPACE_MISSING, MYF(0), ts_info->tablespace_name);
      break;
    case HA_ERR_TABLESPACE_IS_NOT_EMPTY:
      my_error(ER_TABLESPACE_IS_NOT_EMPTY, MYF(0), ts_info->tablespace_name);
      break;
    case HA_ERR_WRONG_FILE_NAME:
      my_error(ER_WRONG_FILE_NAME, MYF(0), ts_info->data_file_name);
      break;
    case HA_ADMIN_FAILED:
      my_error(ER_CANT_CREATE_FILE, MYF(0), ts_info->data_file_name);
      break;
    case HA_ERR_INNODB_READ_ONLY:
      my_error(ER_INNODB_READ_ONLY, MYF(0));
      break;
    case HA_ERR_RECORD_FILE_FULL:
      my_error(ER_RECORD_FILE_FULL, MYF(0), ts_info->tablespace_name);
      break;
    case HA_WRONG_CREATE_OPTION:
      my_error(ER_ILLEGAL_HA, MYF(0), ts_info->tablespace_name);
      break;
    case HA_ERR_TABLESPACE_EXISTS:
      my_error(ER_TABLESPACE_EXISTS, MYF(0), ts_info->tablespace_name);
      break;
    case HA_ERR_NOT_ALLOWED_COMMAND:
      my_error(ER_DISALLOWED_OPERATION, MYF(0), statement_txt,
               ts_info->tablespace_name);
      break;
    default:
      char errbuf[MYSQL_ERRMSG_SIZE];
      my_error(ER_GET_ERRNO, MYF(0), se_error,
               my_strerror(errbuf, MYSQL_ERRMSG_SIZE, se_error));
  }
  return true;
}

}  // namespace

Sql_cmd_tablespace::Sql_cmd_tablespace(const LEX_STRING &name,
                                       const Tablespace_options *options)
    : m_tablespace_name(name), m_options(options) {}

/* purecov: begin inspected */
enum_sql_command Sql_cmd_tablespace::sql_command_code() const {
  DBUG_ASSERT(false);
  return SQLCOM_ALTER_TABLESPACE;
}
/* purecov: end */

Sql_cmd_create_tablespace::Sql_cmd_create_tablespace(
    const LEX_STRING &tsname, const LEX_STRING &dfname,
    const LEX_STRING &lfgname, const Tablespace_options *options)
    : Sql_cmd_tablespace{tsname, options},
      m_datafile_name(dfname),
      m_logfile_group_name(lfgname),
      m_auto_generate_datafile_name(dfname.str == nullptr) {}

bool Sql_cmd_create_tablespace::execute(THD *thd) {
  Rollback_guard rollback_on_return{thd};

  if (check_global_access(thd, CREATE_TABLESPACE_ACL)) {
    return true;
  }

  handlerton *hton = nullptr;
  if (get_stmt_hton(thd, m_options->engine_name, m_tablespace_name.str,
                    "CREATE TABLESPACE", &hton)) {
    return true;
  }
  rollback_on_return.m_hton = hton;  // Allow rollback to call hton->post_ddl

  // Check the tablespace name and acquire an MDL X lock.
  if (validate_tablespace_name(CREATE_TABLESPACE, m_tablespace_name.str,
                               hton) ||
      lock_tablespace_names(thd, m_tablespace_name)) {
    return true;
  }

  /*
    Check if user has permission to create tablespace, if encryption type
    provided differ from global 'default_table_encryption' setting.
    We use 'default_table_encryption' value if encryption is not supplied
    by user.
  */
  bool encrypt_tablespace = false;
  dd::String_type encrypt_type;
  if (m_options->encryption.str) {
    encrypt_tablespace = dd::is_encrypted(m_options->encryption);
    encrypt_type = dd::make_string_type(m_options->encryption);
  } else {
    encrypt_tablespace = thd->variables.default_table_encryption;
    encrypt_type = encrypt_tablespace ? "Y" : "N";
  }

  if (opt_table_encryption_privilege_check &&
      encrypt_tablespace != thd->variables.default_table_encryption &&
      check_table_encryption_admin_access(thd)) {
    my_error(ER_CANNOT_SET_TABLESPACE_ENCRYPTION, MYF(0));
    return true;
  }

  auto &dc = *thd->dd_client();
  dd::cache::Dictionary_client::Auto_releaser releaser{&dc};

  // Check if same tablespace already exists.
  auto tsn = dd::make_string_type(m_tablespace_name);
  const dd::Tablespace *ts = nullptr;
  if (dc.acquire(tsn, &ts)) {
    return true;
  }

  if (ts != nullptr) {
    my_error(ER_TABLESPACE_EXISTS, MYF(0), tsn.c_str());
    return true;
  }

  // Create new tablespace.
  std::unique_ptr<dd::Tablespace> tablespace(
      dd::create_object<dd::Tablespace>());

  // Set tablespace name
  tablespace->set_name(tsn);

  // Engine type
  tablespace->set_engine(ha_resolve_storage_engine_name(hton));

  // - Store ENCRYPTION if SE supports it.
  // - Disallow encryption='y', if SE does not support it.
  if (hton->flags & HTON_SUPPORTS_TABLE_ENCRYPTION) {
    tablespace->options().set("encryption", encrypt_type);
  } else if (encrypt_tablespace) {
    my_error(ER_CHECK_NOT_IMPLEMENTED, MYF(0), "ENCRYPTION");
    return true;
  }

  size_t cl = m_options->ts_comment.length;
  if (validate_comment_length(
          thd, m_options->ts_comment.str, &cl, TABLESPACE_COMMENT_MAXLEN,
          ER_TOO_LONG_TABLESPACE_COMMENT, m_tablespace_name.str)) {
    return true;
  }

  tablespace->set_comment(dd::String_type{m_options->ts_comment.str, cl});

  LEX_STRING tblspc_datafile_name = {m_datafile_name.str,
                                     m_datafile_name.length};
  if (m_auto_generate_datafile_name) {
    char tmp_buf[40];
    String str(tmp_buf, sizeof(tmp_buf), &my_charset_bin);
    String *uuid = mysql_generate_uuid(&str);
    if (hton->get_tablespace_filename_ext)
      uuid->append(hton->get_tablespace_filename_ext());

    tblspc_datafile_name.str =
        thd->strmake(uuid->c_ptr_quick(), uuid->length());
    tblspc_datafile_name.length = uuid->length();
  }

  if (tblspc_datafile_name.length > FN_REFLEN) {
    my_error(ER_PATH_LENGTH, MYF(0), "DATAFILE");
    return true;
  }

  // Add datafile
  tablespace->add_file()->set_filename(
      dd::make_string_type(tblspc_datafile_name));

  // Write changes to dictionary.
  if (dc.store(tablespace.get())) {
    return true;
  }

  const bool atomic_ddl = ddl_is_atomic(hton);
  /*
    Commit after creation of tablespace in the data-dictionary for
    storage engines which don't support atomic DDL. We do this to
    avoid being left with tablespace in SE but not in data-dictionary
    in case of crash. Indeed, in this case, we can end-up with tablespace
    present in the data-dictionary and not present in SE. But this can be
    easily fixed by doing DROP TABLESPACE.
  */
  if (intermediate_commit_unless_atomic_ddl(thd, hton)) {
    return true; /* purecov: inspected */
  }

  auto tsmp = get_mod_pair<dd::Tablespace>(&dc, m_tablespace_name.str);
  if (tsmp.first == nullptr) {
    my_error(ER_TABLESPACE_MISSING_WITH_NAME, MYF(0), m_tablespace_name.str);
    return true;
  }
  st_alter_tablespace ts_info{m_tablespace_name.str,
                              m_logfile_group_name.str,
                              CREATE_TABLESPACE,
                              TS_ALTER_TABLESPACE_TYPE_NOT_DEFINED,
                              tblspc_datafile_name.str,
                              nullptr,
                              *m_options};

  if (map_errors(
          hton->alter_tablespace(hton, thd, &ts_info, tsmp.first, tsmp.second),
          "CREATE TABLESPACE", &ts_info)) {
    if (!atomic_ddl) {
      /*
        For engines which don't support atomic DDL addition of tablespace to
        data-dictionary has been committed already so we need to revert it.
      */
      /* purecov: begin inspected */
      if (dc.drop(tsmp.second)) {
        return true;
      }

      Implicit_substatement_state_guard substatement_guard{thd};
      (void)trans_commit_stmt(thd);
      (void)trans_commit(thd);
      /* purecov: end */
    }
    return true;
  }  // if (map_errors

  /*
    Per convention only engines supporting atomic DDL are allowed to
    modify data-dictionary objects in handler::create() and other
    similar calls.
  */
  if (atomic_ddl && dc.update(tsmp.second)) {
    return true; /* purecov: inspected */
  }

  if (complete_stmt(thd, hton, [&]() { rollback_on_return.disable(); })) {
    return true;
  }
  return false;
}

Sql_cmd_drop_tablespace::Sql_cmd_drop_tablespace(
    const LEX_STRING &tsname, const Tablespace_options *options)
    : Sql_cmd_tablespace{tsname, options} {}

bool Sql_cmd_drop_tablespace::execute(THD *thd) {
  Rollback_guard rollback_on_return{thd};

  if (check_global_access(thd, CREATE_TABLESPACE_ACL)) {
    return true;
  }

  if (lock_tablespace_names(thd, m_tablespace_name)) {
    return true;
  }

  auto &dc = *thd->dd_client();
  dd::cache::Dictionary_client::Auto_releaser releaser{&dc};

  const dd::Tablespace *old_ts_def = nullptr;
  if (dc.acquire(m_tablespace_name.str, &old_ts_def)) {
    return true;
  }
  if (old_ts_def == nullptr) {
    my_error(ER_TABLESPACE_MISSING_WITH_NAME, MYF(0), m_tablespace_name.str);
    return true;
  }

  handlerton *hton = nullptr;
  if (get_dd_hton(thd, old_ts_def->engine(), m_options->engine_name,
                  m_tablespace_name.str, "DROP TABLESPACE", &hton)) {
    return true;
  }
  rollback_on_return.m_hton = hton;

  /*
    Even if the tablespace already exists in the DD we still need to
    validate the name, since we are not allowed to modify
    tablespaces created by the system.
  */
  if (validate_tablespace_name(DROP_TABLESPACE, m_tablespace_name.str, hton)) {
    return true;
  }

  bool is_empty;
  if (old_ts_def->is_empty(thd, &is_empty)) {
    return true;
  }
  if (!is_empty) {
    my_error(ER_TABLESPACE_IS_NOT_EMPTY, MYF(0), m_tablespace_name.str);
    return true;
  }

  st_alter_tablespace ts_info{m_tablespace_name.str,
                              nullptr,
                              DROP_TABLESPACE,
                              TS_ALTER_TABLESPACE_TYPE_NOT_DEFINED,
                              nullptr,
                              nullptr,
                              *m_options};

  int ha_error =
      hton->alter_tablespace(hton, thd, &ts_info, old_ts_def, nullptr);
  if (map_errors(ha_error, "DROP TABLEPSPACE", &ts_info)) {
    if (ha_error == HA_ERR_TABLESPACE_MISSING && !ddl_is_atomic(hton)) {
      /*
        For engines which don't support atomic DDL we might have
        orphan tablespace entries in the data-dictionary which do not
        correspond to tablespaces in SEs.  To allow user to do manual
        clean-up we drop tablespace from the dictionary even if SE
        says it is missing (but still report error).
      */
      /* purecov: begin inspected */
      if (dc.drop(old_ts_def)) {
        return true;
      }

      Implicit_substatement_state_guard substatement_guard{thd};
      (void)trans_commit_stmt(thd);
      (void)trans_commit(thd);
      /* purecov: end */
    }
    return true;
  }  // if (map_errors

  if (dc.drop(old_ts_def)) {
    return true;
  }

  /*
    DROP for engines which don't support atomic DDL still needs to be
    handled by doing commit right after updating data-dictionary.
  */
  if (intermediate_commit_unless_atomic_ddl(thd, hton)) {
    return true; /* purecov: inspected */
  }

  if (complete_stmt(thd, hton, [&]() { rollback_on_return.disable(); })) {
    return true;
  }

  return false;
}

/*
  Mark the tables in the tablespace with ENCRYPTION='y/n'. We only update
  DD objects in memory and the real update of the data-dictionary happens
  later.

  We also check if the table encryption type conflicts with the schema
  default encryption, and throw error if table_encryption_privilege_check
  is enabled and the user does not own TABLE_ENCRYPTION_ADMIN privilege.

  @param thd    Thread.
  @param ts     Reference to tablespace object being altered.
  @param trefs  Reference to list of tables in tablespace.
  @param tpl    Reference to dd::Table objects to be updated.
  @param requested_encryption Type of encryption requested in ALTER.
  @param table_mdl_reqs[out] Pointer to MDL_request_list containing all MDL
                             lock requests.

  @returns true on error, false on success
*/
static bool set_table_encryption_type(THD *thd, const dd::Tablespace &ts,
                                      dd::Tablespace_table_ref_vec *trefs,
                                      Table_pair_list *tpl,
                                      const LEX_STRING &requested_encryption,
                                      MDL_request_list *table_mdl_reqs) {
  bool is_request_to_encrypt = dd::is_encrypted(requested_encryption);

  // If the source tablespace encryption type is same as request type.
  dd::String_type source_tablespace_encryption;
  if (ts.options().exists("encryption"))
    (void)ts.options().get("encryption", &source_tablespace_encryption);
  else
    source_tablespace_encryption = "N";
  if (dd::is_encrypted(source_tablespace_encryption) == is_request_to_encrypt)
    return false;

  // Retrieve table references that use tablespaces.
  if (dd::fetch_tablespace_table_refs(thd, ts, trefs)) {
    return true;
  }

  // Nothing to update, if there are no tables in it.
  if (trefs->empty()) return false;

  // Acquire MDL lock on objects.
  std::vector<dd::String_type> schemaset;
  for (auto &tref : *trefs) {
    table_mdl_reqs->push_front(dd::mdl_req(thd, tref, MDL_SHARED_UPGRADABLE));
    schemaset.push_back(tref.m_schema_name);
  }
  for (auto &sname : schemaset) {
    table_mdl_reqs->push_front(dd::mdl_schema_req(thd, sname));
  }
  if (thd->mdl_context.acquire_locks_nsec(
          table_mdl_reqs, thd->variables.lock_wait_timeout_nsec)) {
    return true;
  }

  // Acquire DD objects to update.
  auto &dc = *thd->dd_client();
  for (auto &tref : *trefs) {
    // Acquire table object pair
    auto tblp = get_mod_pair<dd::Table>(&dc, tref.m_schema_name, tref.m_name);
    if (tblp.first == nullptr) {
      my_error(ER_UNKNOWN_TABLE, MYF(0), tref.m_name.c_str());
      tpl->clear();
      return true;
    }
    tpl->push_back(tblp);
  }

  /*
    Check if any table's schema has default encryption is conflicting with
    the requested encryption type.
  */
  bool is_schema_encryption_conflicting = false;
  for (auto &tref : *trefs) {
    if (tref.m_schema_encryption != is_request_to_encrypt) {
      is_schema_encryption_conflicting = true;
      break;
    }
  }
  if (is_schema_encryption_conflicting) {
    if (opt_table_encryption_privilege_check &&
        check_table_encryption_admin_access(thd)) {
      my_error(is_request_to_encrypt ? ER_TABLESPACE_CANNOT_BE_ENCRYPTED
                                     : ER_TABLESPACE_CANNOT_BE_DECRYPTED,
               MYF(0));
      return true;
    }
    // We throw warning only when creating a unencrypted table in a schema
    // which has default encryption enabled.
    else if (is_request_to_encrypt == false)
      push_warning(thd, Sql_condition::SL_WARNING,
                   WARN_UNENCRYPTED_TABLE_IN_ENCRYPTED_DB,
                   ER_THD(thd, WARN_UNENCRYPTED_TABLE_IN_ENCRYPTED_DB));
  }

  // Update encryption as 'Y/N' for all tables in this tablespace.
  for (auto &tp : *tpl) {
    dd::Properties *table_options = &tp.second->options();
    table_options->set("encrypt_type", (is_request_to_encrypt ? "Y" : "N"));
  }

  return false;
}

/*
  We upgrade the table lock to X so that we can update the dictionary
  table metadata with proper ENCRYPTION clause.

  @param thd    Thread.
  @param table_mdl_reqs Pointer to MDL_request_list containing all MDL
                        lock requests.

  @note hton->alter_tablespace() in InnoDB SE doesn't support rollback yet.
        To workaround this issue, which caused assertions in ntest runs
        we make lock upgrade immune to KILL QUERY and low timeout
        values.

  @returns true on error,
           false on success or when there are no tables in tablespace.
 */

static bool upgrade_lock_for_tables_in_tablespace(
    THD *thd, MDL_request_list *table_mdl_reqs) {
  /*
    The following code gets executed only when there is request to change
    the tablespace ENCRYPTION "and" if there are some tables in the
    tablespace.
  */
  if (table_mdl_reqs->elements() == 0) return false;

  // Install KILL QUERY immunizer.
  dd::DD_kill_immunizer m_kill_immunizer(thd);

  DEBUG_SYNC(thd, "upgrade_lock_for_tables_in_tablespace_kill_point");

  MDL_request_list::Iterator it(*table_mdl_reqs);
  const size_t req_count = table_mdl_reqs->elements();
  for (size_t i = 0; i < req_count; ++i) {
    MDL_request *r = it++;
    if (r->key.mdl_namespace() == MDL_key::TABLE &&
        thd->mdl_context.upgrade_shared_lock_nsec(r->ticket, MDL_EXCLUSIVE,
                                                  LONG_TIMEOUT_NSEC))
      return true;
  }

  return false;
}

Sql_cmd_alter_tablespace::Sql_cmd_alter_tablespace(
    const LEX_STRING &ts_name, const Tablespace_options *options)
    : Sql_cmd_tablespace{ts_name, options} {}

bool Sql_cmd_alter_tablespace::execute(THD *thd) {
  Rollback_guard rollback_on_return{thd};

  if (check_global_access(thd, CREATE_TABLESPACE_ACL)) {
    return true;
  }

  /*
    Check if user has permission to alter tablespace, if encryption type
    provided differ from global 'default_table_encryption' setting.
  */
  if (m_options->encryption.str && opt_table_encryption_privilege_check &&
      dd::is_encrypted(m_options->encryption) !=
          thd->variables.default_table_encryption &&
      check_table_encryption_admin_access(thd)) {
    my_error(ER_CANNOT_SET_TABLESPACE_ENCRYPTION, MYF(0));
    return true;
  }

  if (lock_tablespace_names(thd, m_tablespace_name)) {
    return true;
  }

  auto &dc = *thd->dd_client();
  dd::cache::Dictionary_client::Auto_releaser releaser(&dc);

  auto tsmp = get_mod_pair<dd::Tablespace>(&dc, m_tablespace_name.str);
  if (tsmp.first == nullptr) {
    my_error(ER_TABLESPACE_MISSING_WITH_NAME, MYF(0), m_tablespace_name.str);
    return true;
  }

  handlerton *hton = nullptr;
  if (get_dd_hton(thd, tsmp.first->engine(), m_options->engine_name,
                  m_tablespace_name.str,
                  "ALTER TABLESPACE ... <tablespace_options>", &hton)) {
    return true;
  }
  rollback_on_return.m_hton = hton;
  if (ha_is_storage_engine_disabled(hton)) {
    my_error(ER_DISABLED_STORAGE_ENGINE, MYF(0),
             ha_resolve_storage_engine_name(hton));
    return true;
  }

  Table_pair_list table_object_pairs;
  dd::Tablespace_table_ref_vec trefs;
  MDL_request_list table_mdl_reqs;
  if (m_options->encryption.str) {
    // The storage engine must support encryption.
    if (dd::is_encrypted(m_options->encryption) &&
        !((hton)->flags & HTON_SUPPORTS_TABLE_ENCRYPTION)) {
      my_error(ER_CHECK_NOT_IMPLEMENTED, MYF(0), "ENCRYPTION");
      return true;
    }

    /*
      If we are on SE supporting ENCRYPTION, then store the option
      and update ENCRYPTION clause of tables in this tablespace.
    */
    if (hton->flags & HTON_SUPPORTS_TABLE_ENCRYPTION) {
      tsmp.second->options().set("encryption",
                                 dd::make_string_type(m_options->encryption));
      if (set_table_encryption_type(thd, *tsmp.first, &trefs,
                                    &table_object_pairs, m_options->encryption,
                                    &table_mdl_reqs))
        return true;
    }
  }

  /*
    Even if the tablespace already exists in the DD we still need to
    validate the name, since we are not allowed to modify
    tablepspaces created by the system.
    FUTURE: Would be better if this was made into a
    property/attribute of dd::Tablespace
  */
  if (validate_tablespace_name(ALTER_TABLESPACE, m_tablespace_name.str, hton)) {
    return true;
  }

  st_alter_tablespace ts_info{m_tablespace_name.str,
                              nullptr,
                              ALTER_TABLESPACE,
                              ALTER_TABLESPACE_OPTIONS,
                              nullptr,
                              nullptr,
                              *m_options};

  if (map_errors(
          hton->alter_tablespace(hton, thd, &ts_info, tsmp.first, tsmp.second),
          "ALTER TABLESPACE ... <tablespace_options>", &ts_info)) {
    return true;
  }

  // Upgrade SU to X on table names.
  if (upgrade_lock_for_tables_in_tablespace(thd, &table_mdl_reqs)) return true;

  // Wait and remove TABLE object from TDC.
  for (auto &tref : trefs) {
    // Lock and release the mutex each time to allow others to access the tdc.
    // Alter tablespace can afford to wait for mutex repeatedly.
    tdc_remove_table(thd, TDC_RT_REMOVE_ALL, tref.m_schema_name.c_str(),
                     tref.m_name.c_str(), false /*has_lock*/);
  }

  if (dc.update(tsmp.second)) {
    return true;
  }

  // Persist the modified object changes in DD.
  if (!table_object_pairs.empty()) {
    for (auto &tp : table_object_pairs) {
      if (dc.update(tp.second)) {
        return true;
      }
    }
  }

  /*
    Per convention only engines supporting atomic DDL are allowed to
    modify data-dictionary objects in handler::create() and other
    similar calls. However, DROP and ALTER TABLESPACE for engines which
    don't support atomic DDL still needs to be handled by doing commit
    right after updating data-dictionary.
  */
  if (intermediate_commit_unless_atomic_ddl(thd, hton)) {
    return true;
  }

  if (complete_stmt(thd, hton, [&]() { rollback_on_return.disable(); })) {
    return true;
  }

  return false;
}

Sql_cmd_alter_tablespace_add_datafile::Sql_cmd_alter_tablespace_add_datafile(
    const LEX_STRING &tsname, const LEX_STRING &dfname,
    const Tablespace_options *options)
    : Sql_cmd_tablespace{tsname, options}, m_datafile_name(dfname) {}

bool Sql_cmd_alter_tablespace_add_datafile::execute(THD *thd) {
  Rollback_guard rollback_on_return{thd};

  if (check_global_access(thd, CREATE_TABLESPACE_ACL)) {
    return true;
  }

  if (lock_tablespace_names(thd, m_tablespace_name)) {
    return true;
  }

  auto &dc = *thd->dd_client();
  dd::cache::Dictionary_client::Auto_releaser releaser{&dc};

  auto tsmp = get_mod_pair<dd::Tablespace>(&dc, m_tablespace_name.str);
  if (tsmp.first == nullptr) {
    my_error(ER_TABLESPACE_MISSING_WITH_NAME, MYF(0), m_tablespace_name.str);
    return true;
  }

  if (m_datafile_name.length > FN_REFLEN) {
    my_error(ER_PATH_LENGTH, MYF(0), "DATAFILE");
    return true;
  }

  dd::Tablespace_file *tsf_obj = tsmp.second->add_file();
  tsf_obj->set_filename(
      dd::String_type{m_datafile_name.str, m_datafile_name.length});

  handlerton *hton = nullptr;
  if (get_dd_hton(thd, tsmp.first->engine(), m_options->engine_name,
                  m_tablespace_name.str, "ALTER TABLESPACE ... ADD DATAFILE",
                  &hton)) {
    return true;
  }
  rollback_on_return.m_hton = hton;
  if (ha_is_storage_engine_disabled(hton)) {
    my_error(ER_DISABLED_STORAGE_ENGINE, MYF(0),
             ha_resolve_storage_engine_name(hton));
    return true;
  }
  /*
    Even if the tablespace already exists in the DD we still need to
    validate the name, since we are not allowed to modify
    tablespaces created by the system.

    FUTURE: Would be better if this was made into a
    property/attribute of dd::Tablespace
  */
  if (validate_tablespace_name(ALTER_TABLESPACE, m_tablespace_name.str, hton)) {
    return true;
  }

  st_alter_tablespace ts_info{m_tablespace_name.str,
                              nullptr,
                              ALTER_TABLESPACE,
                              ALTER_TABLESPACE_ADD_FILE,
                              m_datafile_name.str,
                              nullptr,
                              *m_options};

  if (map_errors(
          hton->alter_tablespace(hton, thd, &ts_info, tsmp.first, tsmp.second),
          "ALTER TABLESPACE ... ADD DATAFILE", &ts_info)) {
    return true;
  }

  if (dc.update(tsmp.second)) {
    return true;
  }

  /*
    ALTER TABLESPACE for engines which don't support atomic DDL still
    needs to be handled by doing commit right after updating
    data-dictionary.
  */
  if (intermediate_commit_unless_atomic_ddl(thd, hton)) {
    return true;
  }

  if (complete_stmt(thd, hton, [&]() { rollback_on_return.disable(); })) {
    return true;
  }
  return false;
}

Sql_cmd_alter_tablespace_drop_datafile::Sql_cmd_alter_tablespace_drop_datafile(
    const LEX_STRING &tsname, const LEX_STRING &dfname,
    const Tablespace_options *options)
    : Sql_cmd_tablespace{tsname, options}, m_datafile_name(dfname) {}

bool Sql_cmd_alter_tablespace_drop_datafile::execute(THD *thd) {
  Rollback_guard rollback_on_return{thd};

  if (check_global_access(thd, CREATE_TABLESPACE_ACL)) {
    return true;
  }

  if (lock_tablespace_names(thd, m_tablespace_name)) {
    return true;
  }

  auto &dc = *thd->dd_client();
  dd::cache::Dictionary_client::Auto_releaser releaser{&dc};

  auto tsmp = get_mod_pair<dd::Tablespace>(&dc, m_tablespace_name.str);
  if (tsmp.first == nullptr) {
    my_error(ER_TABLESPACE_MISSING_WITH_NAME, MYF(0), m_tablespace_name.str);
    return true;
  }

  if (tsmp.second->remove_file(
          dd::String_type{m_datafile_name.str, m_datafile_name.length})) {
    my_error(ER_MISSING_TABLESPACE_FILE, MYF(0), m_tablespace_name.str,
             m_datafile_name.str);
    return true;
  }
  handlerton *hton = nullptr;
  if (get_dd_hton(thd, tsmp.first->engine(), m_options->engine_name,
                  m_tablespace_name.str, "ALTER TABLESPACE ... DROP DATAFILE",
                  &hton)) {
    return true;
  }
  rollback_on_return.m_hton = hton;
  if (ha_is_storage_engine_disabled(hton)) {
    my_error(ER_DISABLED_STORAGE_ENGINE, MYF(0),
             ha_resolve_storage_engine_name(hton));
    return true;
  }

  /*
    Even if the tablespace already exists in the DD we still need to
    validate the name, since we are not allowed to modify tablespaces
    created by the system.
  */
  if (validate_tablespace_name(ALTER_TABLESPACE, m_tablespace_name.str, hton)) {
    return true;
  }
  st_alter_tablespace ts_info{m_tablespace_name.str,
                              nullptr,
                              ALTER_TABLESPACE,
                              ALTER_TABLESPACE_DROP_FILE,
                              m_datafile_name.str,
                              nullptr,
                              *m_options};
  if (map_errors(
          hton->alter_tablespace(hton, thd, &ts_info, tsmp.first, tsmp.second),
          "ALTER TABLESPACE ... DROP DATAFILE", &ts_info)) {
    return true;
  }

  if (dc.update(tsmp.second)) {
    return true;
  }

  /*
    ALTER TABLESPACE for engines which don't support atomic
    DDL still needs to be handled by doing commit right after updating
    data-dictionary.
  */
  if (intermediate_commit_unless_atomic_ddl(thd, hton)) {
    return true; /* purecov: inspected */
  }

  if (complete_stmt(thd, hton, [&]() { rollback_on_return.disable(); })) {
    return true;
  }
  return false;
}

Sql_cmd_alter_tablespace_rename::Sql_cmd_alter_tablespace_rename(
    const LEX_STRING &old_name, const LEX_STRING &new_name)
    : Sql_cmd_tablespace{old_name, nullptr}, m_new_name(new_name) {}

bool Sql_cmd_alter_tablespace_rename::execute(THD *thd) {
  Rollback_guard rollback_on_return{thd};

  if (check_global_access(thd, CREATE_TABLESPACE_ACL)) {
    return true;
  }

  // Can't check the name in SE, yet. Need to acquire Tablespace
  // object first, so that we can get the engine name.

  // Lock both tablespace names in one go
  if (lock_tablespace_names(thd, m_tablespace_name, m_new_name)) {
    return true;
  }
  dd::cache::Dictionary_client *dc = thd->dd_client();
  dd::cache::Dictionary_client::Auto_releaser releaser(dc);

  dd::String_type old_name = dd::make_string_type(m_tablespace_name);
  dd::String_type new_name = dd::make_string_type(m_new_name);

  auto tsmp = get_mod_pair<dd::Tablespace>(dc, m_tablespace_name.str);
  if (tsmp.first == nullptr) {
    my_error(ER_TABLESPACE_MISSING_WITH_NAME, MYF(0), m_tablespace_name.str);
    return true;
  }
  tsmp.second->set_name(new_name);

  const dd::Tablespace *existing_new_ts_def = nullptr;
  if (dc->acquire(new_name, &existing_new_ts_def)) {
    return true;
  }
  if (existing_new_ts_def != nullptr) {
    my_error(ER_TABLESPACE_EXISTS, MYF(0), new_name.c_str());
    return true;
  }

  handlerton *hton = nullptr;
  if (get_dd_hton(thd, tsmp.first->engine(), {nullptr, 0},
                  m_tablespace_name.str, "ALTER TABLESPACE ... RENAME TO",
                  &hton)) {
    return true;
  }
  if (ha_is_storage_engine_disabled(hton)) {
    my_error(ER_DISABLED_STORAGE_ENGINE, MYF(0),
             ha_resolve_storage_engine_name(hton));
    return true;
  }
  rollback_on_return.m_hton = hton;

  /*
    Now with the hton, we need to validate BOTH the old and the new
    name - since we are not allowed to rename reserved names
    FUTURE - Could be a property/attribute of dd::Tablespace
  */
  if (validate_tablespace_name(DROP_TABLESPACE, m_tablespace_name.str, hton)) {
    return true;
  }

  // Also validate the new tablespace name in the SE
  if (validate_tablespace_name(CREATE_TABLESPACE, m_new_name.str, hton)) {
    return true;
  }

  dd::Tablespace_table_ref_vec trefs;
  if (dd::fetch_tablespace_table_refs(thd, *tsmp.first, &trefs)) {
    return true;
  }
  MDL_request_list table_reqs;
  for (auto &tref : trefs) {
    table_reqs.push_front(dd::mdl_req(thd, tref, MDL_EXCLUSIVE));
  }

  if (thd->mdl_context.acquire_locks_nsec(
          &table_reqs, thd->variables.lock_wait_timeout_nsec)) {
    return true;
  }

  for (auto &tref : trefs) {
    // Lock and release the mutex each time to allow others to access the tdc.
    // Rename tablespace can afford to wait for mutex repeatedly.
    tdc_remove_table(thd, TDC_RT_REMOVE_ALL, tref.m_schema_name.c_str(),
                     tref.m_name.c_str(), false /*has_lock*/);
  }
  st_alter_tablespace ts_info{
      m_tablespace_name.str,   nullptr, ALTER_TABLESPACE,
      ALTER_TABLESPACE_RENAME, nullptr, nullptr,
      Tablespace_options{}};
  if (map_errors(
          hton->alter_tablespace(hton, thd, &ts_info, tsmp.first, tsmp.second),
          "ALTER TABLESPACE ... RENAME TO", &ts_info)) {
    return true;
  }

  // TODO WL#9536: Until crash-safe ddl is implemented we need to do
  // manual compensation in case of rollback
  auto compensate_grd = dd::sdi_utils::make_guard(hton, [&](handlerton *ht) {
    std::unique_ptr<dd::Tablespace> comp{tsmp.first->clone()};
    (void)ht->alter_tablespace(ht, thd, &ts_info, tsmp.second, comp.get());
  });

  DBUG_EXECUTE_IF("tspr_post_se", {
    my_error(ER_UNKNOWN_ERROR, MYF(0));
    return true;
  });

  if (dc->update(tsmp.second)) {
    return true;
  }

  DBUG_EXECUTE_IF("tspr_post_update", {
    my_error(ER_UNKNOWN_ERROR, MYF(0));
    return true;
  });

  /*
    ALTER TABLESPACE for engines which don't support atomic
    DDL still needs to be handled by doing commit right after updating
    data-dictionary.
  */
  if (intermediate_commit_unless_atomic_ddl(thd, hton)) {
    return true;
  }

  if (!ddl_is_atomic(hton)) {
    compensate_grd.release();
  }

  DBUG_EXECUTE_IF("tspr_post_intcmt", {
    my_error(ER_UNKNOWN_ERROR, MYF(0));
    return true;
  });

  if (complete_stmt(thd, hton, [&]() {
        rollback_on_return.disable();
        compensate_grd.release();
      })) {
    return true;
  }
  return false;
}

Sql_cmd_create_undo_tablespace::Sql_cmd_create_undo_tablespace(
    ts_command_type cmd_type, const LEX_STRING &utsname,
    const LEX_STRING &dfname, const Tablespace_options *options)
    : m_cmd(cmd_type),
      m_undo_tablespace_name(utsname),
      m_datafile_name(dfname),
      m_options(options) {}

bool Sql_cmd_create_undo_tablespace::execute(THD *thd) {
  Rollback_guard rollback_on_return{thd};

  if (check_global_access(thd, CREATE_TABLESPACE_ACL)) {
    return true;
  }

  handlerton *hton = nullptr;
  if (get_stmt_hton(thd, m_options->engine_name, m_undo_tablespace_name.str,
                    "CREATE UNDO TABLESPACE", &hton)) {
    return true;
  }

  rollback_on_return.m_hton = hton;  // Allow rollback to call hton->post_ddl

  // Check the tablespace name and acquire an MDL X lock.
  if (validate_tablespace_name(CREATE_UNDO_TABLESPACE,
                               m_undo_tablespace_name.str, hton) ||
      lock_tablespace_names(thd, m_undo_tablespace_name)) {
    return true;
  }

  auto &dc = *thd->dd_client();
  dd::cache::Dictionary_client::Auto_releaser releaser{&dc};

  // Check if same tablespace already exists.
  auto tsn = dd::make_string_type(m_undo_tablespace_name);
  const dd::Tablespace *ts = nullptr;

  if (dc.acquire(tsn, &ts)) {
    return true;
  }

  if (ts != nullptr) {
    my_error(ER_TABLESPACE_EXISTS, MYF(0), tsn.c_str());
    return true;
  }

  // Create new tablespace.
  std::unique_ptr<dd::Tablespace> tablespace(
      dd::create_object<dd::Tablespace>());

  // Set tablespace name
  tablespace->set_name(tsn);

  // Engine type
  tablespace->set_engine(ha_resolve_storage_engine_name(hton));

  if (m_datafile_name.length > FN_REFLEN) {
    my_error(ER_PATH_LENGTH, MYF(0), "DATAFILE");
    return true;
  }

  // ENCRYPTION clause is not supported for undo tablespace.
  if (hton->flags & HTON_SUPPORTS_TABLE_ENCRYPTION) {
    tablespace->options().set("encryption", "N");
  }

  // Add datafile
  tablespace->add_file()->set_filename(dd::make_string_type(m_datafile_name));

  // Write changes to dictionary.
  if (dc.store(tablespace.get())) {
    return true;
  }

  /*
    Commit after creation of tablespace in the data-dictionary for
    storage engines which don't support atomic DDL. We do this to
    avoid being left with tablespace in SE but not in data-dictionary
    in case of crash. Indeed, in this case, we can end-up with tablespace
    present in the data-dictionary and not present in SE. But this can be
    easily fixed by doing DROP TABLESPACE.
  */
  if (intermediate_commit_unless_atomic_ddl(thd, hton)) {
    return true;
  }

  auto tsmp = get_mod_pair<dd::Tablespace>(&dc, m_undo_tablespace_name.str);
  if (tsmp.first == nullptr) {
    my_error(ER_TABLESPACE_MISSING_WITH_NAME, MYF(0),
             m_undo_tablespace_name.str);
    return true;
  }
  st_alter_tablespace ts_info{m_undo_tablespace_name.str,
                              nullptr,
                              CREATE_UNDO_TABLESPACE,
                              TS_ALTER_TABLESPACE_TYPE_NOT_DEFINED,
                              m_datafile_name.str,
                              nullptr,
                              *m_options};

  int ha_error =
      hton->alter_tablespace(hton, thd, &ts_info, tsmp.first, tsmp.second);
  if (map_errors(ha_error, "CREATE UNDO TABLEPSPACE", &ts_info)) {
    if (!ddl_is_atomic(hton)) {
      /*
        For engines which don't support atomic DDL addition of tablespace to
        data-dictionary has been committed already so we need to revert it.
      */
      if (dc.drop(tsmp.second)) {
        return true;
      }

      Implicit_substatement_state_guard substatement_guard{thd};
      (void)trans_commit_stmt(thd);
      (void)trans_commit(thd);
    }
    return true;
  }  // if (map_errors

  /*
    Per convention only engines supporting atomic DDL are allowed to
    modify data-dictionary objects in handler::create() and other
    similar calls.
  */
  if (ddl_is_atomic(hton) && dc.update(tsmp.second)) {
    return true;
  }

  if (complete_stmt(
          thd, hton, [&]() { rollback_on_return.disable(); }, true, true)) {
    return true;
  }

  return false;
}

enum_sql_command Sql_cmd_create_undo_tablespace::sql_command_code() const {
  return SQLCOM_ALTER_TABLESPACE;
}

Sql_cmd_alter_undo_tablespace::Sql_cmd_alter_undo_tablespace(
    ts_command_type cmd_type, const LEX_STRING &utsname,
    const LEX_STRING &dfname, const Tablespace_options *options,
    ts_alter_tablespace_type at_type)
    : m_cmd(cmd_type),
      m_undo_tablespace_name(utsname),
      m_datafile_name(dfname),
      m_at_type(at_type),
      m_options(options) {
  // These only at_type values that the syntax currently accepts
  DBUG_ASSERT(at_type == TS_ALTER_TABLESPACE_TYPE_NOT_DEFINED ||
              at_type == ALTER_UNDO_TABLESPACE_SET_ACTIVE ||
              at_type == ALTER_UNDO_TABLESPACE_SET_INACTIVE);
}

bool Sql_cmd_alter_undo_tablespace::execute(THD *thd) {
  Rollback_guard rollback_on_return{thd};

  if (check_global_access(thd, CREATE_TABLESPACE_ACL)) {
    return true;
  }

  handlerton *hton = nullptr;
  if (get_stmt_hton(thd, m_options->engine_name, m_undo_tablespace_name.str,
                    "ALTER UNDO TABLESPACE", &hton)) {
    return true;
  }

  // Check the tablespace name and acquire an MDL X lock.
  if (validate_tablespace_name(ALTER_UNDO_TABLESPACE,
                               m_undo_tablespace_name.str, hton) ||
      lock_tablespace_names(thd, m_undo_tablespace_name)) {
    return true;
  }

  auto &dc = *thd->dd_client();
  dd::cache::Dictionary_client::Auto_releaser releaser{&dc};

  // Get the existing dd::Tablespace for this tablespace name.
  auto tsn = dd::make_string_type(m_undo_tablespace_name);
  const dd::Tablespace *ts = nullptr;

  if (dc.acquire(tsn, &ts)) {
    return true;
  }

  // Must already exist
  if (ts == nullptr) {
    my_error(ER_TABLESPACE_MISSING_WITH_NAME, MYF(0), tsn.c_str());
    return true;
  }

  auto tsmp = get_mod_pair<dd::Tablespace>(&dc, m_undo_tablespace_name.str);
  if (tsmp.first == nullptr) {
    my_error(ER_TABLESPACE_MISSING_WITH_NAME, MYF(0),
             m_undo_tablespace_name.str);
    return true;
  }
  st_alter_tablespace ts_info{m_undo_tablespace_name.str,
                              nullptr,
                              ALTER_UNDO_TABLESPACE,
                              m_at_type,
                              nullptr,
                              nullptr,
                              *m_options};

  int ha_error =
      hton->alter_tablespace(hton, thd, &ts_info, tsmp.first, tsmp.second);
  if (map_errors(ha_error, "ALTER UNDO TABLEPSPACE", &ts_info)) {
    if (!ddl_is_atomic(hton)) {
      Implicit_substatement_state_guard substatement_guard{thd};
      (void)trans_commit_stmt(thd);
      (void)trans_commit(thd);
    }
    return true;
  }  // if (map_errors

  /*
    Per convention only engines supporting atomic DDL are allowed to
    modify data-dictionary objects in handler::create() and other
    similar calls.
  */
  if (ddl_is_atomic(hton) && dc.update(tsmp.second)) {
    return true;
  }

  if (complete_stmt(
          thd, hton, [&]() { rollback_on_return.disable(); }, true, true)) {
    return true;
  }

  return false;
}

enum_sql_command Sql_cmd_alter_undo_tablespace::sql_command_code() const {
  return SQLCOM_ALTER_TABLESPACE;
}

Sql_cmd_drop_undo_tablespace::Sql_cmd_drop_undo_tablespace(
    ts_command_type cmd_type, const LEX_STRING &utsname,
    const LEX_STRING &dfname, const Tablespace_options *options)
    : m_cmd(cmd_type),
      m_undo_tablespace_name(utsname),
      m_datafile_name(dfname),
      m_options(options) {}

bool Sql_cmd_drop_undo_tablespace::execute(THD *thd) {
  Rollback_guard rollback_on_return{thd};

  if (check_global_access(thd, CREATE_TABLESPACE_ACL)) {
    return true;
  }

  handlerton *hton = nullptr;
  if (get_stmt_hton(thd, m_options->engine_name, m_undo_tablespace_name.str,
                    "DROP UNDO TABLESPACE", &hton)) {
    return true;
  }
  rollback_on_return.m_hton = hton;

  /*
    Check the tablespace name and acquire an MDL X lock.
    Even if the tablespace already exists in the DD we still need to
    validate the name, since we are not allowed to modify
    tablespaces created by the system.
  */
  if (validate_tablespace_name(DROP_UNDO_TABLESPACE, m_undo_tablespace_name.str,
                               hton) ||
      lock_tablespace_names(thd, m_undo_tablespace_name)) {
    return true;
  }

  auto &dc = *thd->dd_client();
  dd::cache::Dictionary_client::Auto_releaser releaser{&dc};

  // Get the existing dd::Tablespace for this tablespace name.
  auto tsn = dd::make_string_type(m_undo_tablespace_name);
  const dd::Tablespace *ts = nullptr;

  if (dc.acquire(tsn, &ts)) {
    return true;
  }

  if (ts == nullptr) {
    my_error(ER_TABLESPACE_MISSING_WITH_NAME, MYF(0), tsn.c_str());
    return true;
  }

  st_alter_tablespace ts_info{m_undo_tablespace_name.str,
                              nullptr,
                              DROP_UNDO_TABLESPACE,
                              TS_ALTER_TABLESPACE_TYPE_NOT_DEFINED,
                              nullptr,
                              nullptr,
                              *m_options};

  int ha_error = hton->alter_tablespace(hton, thd, &ts_info, ts, nullptr);
  if (map_errors(ha_error, "DROP UNDO TABLEPSPACE", &ts_info)) {
    if (!ddl_is_atomic(hton)) {
      /*
        For engines which don't support atomic DDL we might have
        orphan tablespace entries in the data-dictionary which do not
        correspond to tablespaces in SEs.  To allow user to do manual
        clean-up we drop tablespace from the dictionary even if SE
        says it is missing (but still report error).
      */
      if (dc.drop(ts)) {
        return true;
      }

      Implicit_substatement_state_guard substatement_guard{thd};
      (void)trans_commit_stmt(thd);
      (void)trans_commit(thd);
    }
    return true;
  }  // if (map_errors

  if (dc.drop(ts)) {
    return true;
  }

  /*
    DROP for engines which don't support atomic DDL still needs to be
    handled by doing commit right after updating data-dictionary.
  */
  if (intermediate_commit_unless_atomic_ddl(thd, hton)) {
    return true; /* purecov: inspected */
  }

  if (complete_stmt(
          thd, hton, [&]() { rollback_on_return.disable(); }, true, true)) {
    return true;
  }

  return false;
}

enum_sql_command Sql_cmd_drop_undo_tablespace::sql_command_code() const {
  return SQLCOM_ALTER_TABLESPACE;
}

Sql_cmd_logfile_group::Sql_cmd_logfile_group(
    ts_command_type cmd_type, const LEX_STRING &logfile_group_name,
    const Tablespace_options *options, const LEX_STRING &undofile_name)
    : m_cmd(cmd_type),
      m_logfile_group_name(logfile_group_name),
      m_undofile_name(undofile_name),
      m_options(options) {}

bool Sql_cmd_logfile_group::execute(THD *thd) {
  Rollback_guard rollback_on_return{thd};

  if (check_global_access(thd, CREATE_TABLESPACE_ACL)) {
    return true;
  }

  handlerton *hton = nullptr;
  if (get_stmt_hton(thd, m_options->engine_name, m_logfile_group_name.str,
                    "CREATE/ALTER/DROP LOGFILE GROUP", &hton)) {
    return true;
  }

  // Lock tablespace(since the logfile group is stored as such)
  if (lock_tablespace_names(thd, m_logfile_group_name)) {
    return true;
  }

  st_alter_tablespace ts_info{nullptr,   m_logfile_group_name.str,
                              m_cmd,     TS_ALTER_TABLESPACE_TYPE_NOT_DEFINED,
                              nullptr,   m_undofile_name.str,
                              *m_options};

  if (map_errors(hton->alter_tablespace(hton, thd, &ts_info, nullptr, nullptr),
                 "CREATE/ALTER/DROP LOGFILE GROUP", &ts_info)) {
    return true;
  }

  // The CREATE/ALTER/DROP LOGFILE GROUP command is atomic in the SE
  // but does not modify the DD and thus there is no active transaction
  // -> turn off "using_trans"
  const bool using_trans = false;
  if (complete_stmt(
          thd, hton, [&]() { rollback_on_return.disable(); }, using_trans)) {
    return true;
  }
  return false;
}

enum_sql_command Sql_cmd_logfile_group::sql_command_code() const {
  return SQLCOM_ALTER_TABLESPACE; /* purecov: inspected */
}
