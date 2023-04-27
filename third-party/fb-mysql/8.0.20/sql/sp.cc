/*
   Copyright (c) 2002, 2020, Oracle and/or its affiliates. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "sql/sp.h"

#include <string.h>
#include <algorithm>
#include <atomic>
#include <memory>
#include <new>
#include <utility>
#include <vector>

#include "lex_string.h"
#include "m_ctype.h"
#include "m_string.h"
#include "my_alloc.h"
#include "my_base.h"
#include "my_dbug.h"
#include "my_loglevel.h"
#include "my_psi_config.h"
#include "my_sqlcommand.h"
#include "my_sys.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/log_shared.h"
#include "mysql/components/services/psi_statement_bits.h"
#include "mysql/psi/mysql_sp.h"
#include "mysql/psi/psi_base.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "sql/auth/auth_acls.h"
#include "sql/auth/auth_common.h"  // check_some_routine_access
#include "sql/auth/sql_security_ctx.h"
#include "sql/binlog.h"                      // mysql_bin_log
#include "sql/dd/cache/dictionary_client.h"  // dd::cache::Dictionary_client
#include "sql/dd/dd_routine.h"               // dd routine methods.
#include "sql/dd/dd_utility.h"               // dd::normalize_string
#include "sql/dd/string_type.h"
#include "sql/dd/types/function.h"
#include "sql/dd/types/procedure.h"
#include "sql/dd/types/routine.h"
#include "sql/dd/types/schema.h"
#include "sql/dd/types/trigger.h"  //name_collation
#include "sql/dd_sp.h"             // prepare_sp_chistics_from_dd_routine
#include "sql/dd_sql_view.h"       // update_referencing_views_metadata
#include "sql/dd_table_share.h"    // dd_get_mysql_charset
#include "sql/debug_sync.h"        // DEBUG_SYNC
#include "sql/error_handler.h"     // Internal_error_handler
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/lock.h"       // lock_object_name
#include "sql/log_event.h"  // append_query_string
#include "sql/mdl.h"
#include "sql/mysqld.h"  // trust_function_creators
#include "sql/protocol.h"
#include "sql/psi_memory_key.h"  // key_memory_sp_head_main_root
#include "sql/set_var.h"
#include "sql/sp_cache.h"     // sp_cache_invalidate
#include "sql/sp_head.h"      // Stored_program_creation_ctx
#include "sql/sp_pcontext.h"  // sp_pcontext
#include "sql/sql_class.h"
#include "sql/sql_const.h"
#include "sql/sql_db.h"  // get_default_db_collation
#include "sql/sql_digest_stream.h"
#include "sql/sql_error.h"
#include "sql/sql_list.h"
#include "sql/sql_parse.h"  // parse_sql
#include "sql/sql_show.h"   // append_identifier
#include "sql/sql_table.h"  // write_bin_log
#include "sql/strfunc.h"    // lex_string_strmake
#include "sql/system_variables.h"
#include "sql/table.h"
#include "sql/thd_raii.h"
#include "sql/thr_malloc.h"
#include "sql/transaction.h"
#include "sql/transaction_info.h"
#include "sql_string.h"
#include "template_utils.h"

class sp_rcontext;

/* Used in error handling only */
#define SP_TYPE_STRING(type) \
  (type == enum_sp_type::FUNCTION ? "FUNCTION" : "PROCEDURE")
static bool create_string(THD *thd, String *buf, enum_sp_type sp_type,
                          const char *db, size_t dblen, const char *name,
                          size_t namelen, const char *params, size_t paramslen,
                          const char *returns, size_t returnslen,
                          const char *body, size_t bodylen,
                          st_sp_chistics *chistics,
                          const LEX_CSTRING &definer_user,
                          const LEX_CSTRING &definer_host, sql_mode_t sql_mode);

/**************************************************************************
  Fetch stored routines and events creation_ctx for upgrade.
**************************************************************************/

bool load_charset(MEM_ROOT *mem_root, Field *field, const CHARSET_INFO *dflt_cs,
                  const CHARSET_INFO **cs) {
  String cs_name;

  if (get_field(mem_root, field, &cs_name)) {
    *cs = dflt_cs;
    return true;
  }

  *cs = get_charset_by_csname(cs_name.c_ptr(), MY_CS_PRIMARY, MYF(0));

  if (*cs == nullptr) {
    *cs = dflt_cs;
    return true;
  }

  return false;
}

/*************************************************************************/

bool load_collation(MEM_ROOT *mem_root, Field *field,
                    const CHARSET_INFO *dflt_cl, const CHARSET_INFO **cl) {
  String cl_name;

  if (get_field(mem_root, field, &cl_name)) {
    *cl = dflt_cl;
    return true;
  }

  *cl = get_charset_by_name(cl_name.c_ptr(), MYF(0));

  if (*cl == nullptr) {
    *cl = dflt_cl;
    return true;
  }

  return false;
}

/**************************************************************************
  Stored_routine_creation_ctx implementation.
**************************************************************************/

Stored_routine_creation_ctx *
Stored_routine_creation_ctx::create_routine_creation_ctx(
    const dd::Routine *routine) {
  /* Load character set/collation attributes. */
  const CHARSET_INFO *client_cs =
      dd_get_mysql_charset(routine->client_collation_id());
  const CHARSET_INFO *connection_cl =
      dd_get_mysql_charset(routine->connection_collation_id());
  const CHARSET_INFO *db_cl =
      dd_get_mysql_charset(routine->schema_collation_id());

  DBUG_ASSERT(client_cs != nullptr);
  DBUG_ASSERT(connection_cl != nullptr);
  DBUG_ASSERT(db_cl != nullptr);

  // Create the context.
  return new (*THR_MALLOC)
      Stored_routine_creation_ctx(client_cs, connection_cl, db_cl);
}

/*************************************************************************/

Stored_routine_creation_ctx *Stored_routine_creation_ctx::load_from_db(
    THD *thd, const sp_name *name, TABLE *proc_tbl) {
  // Load character set/collation attributes.

  const CHARSET_INFO *client_cs;
  const CHARSET_INFO *connection_cl;
  const CHARSET_INFO *db_cl;

  const char *db_name = thd->strmake(name->m_db.str, name->m_db.length);
  const char *sr_name = thd->strmake(name->m_name.str, name->m_name.length);

  bool invalid_creation_ctx = false;

  if (load_charset(thd->mem_root,
                   proc_tbl->field[MYSQL_PROC_FIELD_CHARACTER_SET_CLIENT],
                   thd->variables.character_set_client, &client_cs)) {
    LogErr(WARNING_LEVEL, ER_SR_BOGUS_VALUE, db_name, sr_name,
           "mysql.proc.character_set_client");

    invalid_creation_ctx = true;
  }

  if (load_collation(thd->mem_root,
                     proc_tbl->field[MYSQL_PROC_FIELD_COLLATION_CONNECTION],
                     thd->variables.collation_connection, &connection_cl)) {
    LogErr(WARNING_LEVEL, ER_SR_BOGUS_VALUE, db_name, sr_name,
           "mysql.proc.collation_connection.");

    invalid_creation_ctx = true;
  }

  if (load_collation(thd->mem_root,
                     proc_tbl->field[MYSQL_PROC_FIELD_DB_COLLATION], nullptr,
                     &db_cl)) {
    LogErr(WARNING_LEVEL, ER_SR_BOGUS_VALUE, db_name, sr_name,
           "mysql.proc.db_collation.");

    invalid_creation_ctx = true;
  }

  if (invalid_creation_ctx) {
    LogErr(WARNING_LEVEL, ER_SR_INVALID_CONTEXT, db_name, sr_name);
  }

  /*
    If we failed to retrieve the database collation, load the default one
    from the disk.
  */

  if (!db_cl) get_default_db_collation(thd, name->m_db.str, &db_cl);

  /* Create the context. */

  return new (thd->mem_root)
      Stored_routine_creation_ctx(client_cs, connection_cl, db_cl);
}

Stored_program_creation_ctx *Stored_routine_creation_ctx::clone(
    MEM_ROOT *mem_root) {
  return new (mem_root)
      Stored_routine_creation_ctx(m_client_cs, m_connection_cl, m_db_cl);
}

Object_creation_ctx *Stored_routine_creation_ctx::create_backup_ctx(
    THD *thd) const {
  DBUG_TRACE;
  return new (thd->mem_root) Stored_routine_creation_ctx(thd);
}

void Stored_routine_creation_ctx::delete_backup_ctx() { destroy(this); }

/**
  Acquire Shared MDL lock on the routine object.

  @param thd           Thread context
  @param type          Type of the routine (enum_sp_type::PROCEDURE/...)
  @param name          Name of the routine
  @param mdl_lock_type Type of MDL lock be acquired on the routine.

  @retval false               Success
  @retval true                Error
*/

static bool lock_routine_name(THD *thd, enum_sp_type type, sp_name *name,
                              enum_mdl_type mdl_lock_type) {
  DBUG_TRACE;

  DBUG_ASSERT(mdl_lock_type == MDL_SHARED_HIGH_PRIO ||
              mdl_lock_type == MDL_SHARED);

  MDL_key mdl_key;
  if (type == enum_sp_type::FUNCTION)
    dd::Function::create_mdl_key(name->m_db.str, name->m_name.str, &mdl_key);
  else
    dd::Procedure::create_mdl_key(name->m_db.str, name->m_name.str, &mdl_key);

  // MDL Lock request on the routine.
  MDL_request routine_request;
  MDL_REQUEST_INIT_BY_KEY(&routine_request, &mdl_key, mdl_lock_type,
                          MDL_TRANSACTION);
  // Acquire MDL locks
  if (thd->mdl_context.acquire_lock_nsec(&routine_request,
                                         thd->variables.lock_wait_timeout_nsec))
    return true;

  return false;
}

/**
  Return appropriate error about recursion limit reaching

  @param thd  Thread handle
  @param sp   The stored procedure executed

  @remark For functions and triggers we return error about
          prohibited recursion. For stored procedures we
          return about reaching recursion limit.
*/

static void recursion_level_error(THD *thd, sp_head *sp) {
  if (sp->m_type == enum_sp_type::PROCEDURE) {
    my_error(ER_SP_RECURSION_LIMIT, MYF(0),
             static_cast<int>(thd->variables.max_sp_recursion_depth),
             sp->m_name.str);
  } else
    my_error(ER_SP_NO_RECURSION, MYF(0));
}

/**
  Find routine definition in data dictionary table and create corresponding
  sp_head object for it.

  @param thd   Thread context
  @param type  Type of routine (PROCEDURE/...)
  @param name  Name of routine
  @param sphp  Out parameter in which pointer to created sp_head
               object is returned (0 in case of error).

  @note
    This function may damage current LEX during execution, so it is good
    idea to create temporary LEX and make it active before calling it.

  @retval
    SP_OK       Success
  @retval
    non-SP_OK   Error (one of special codes like SP_DOES_NOT_EXISTS)
*/

static enum_sp_return_code db_find_routine(THD *thd, enum_sp_type type,
                                           sp_name *name, sp_head **sphp) {
  DBUG_TRACE;
  DBUG_PRINT("enter",
             ("type: %d name: %.*s", static_cast<int>(type),
              static_cast<int>(name->m_name.length), name->m_name.str));

  *sphp = nullptr;  // In case of errors

  // Grab shared MDL lock on routine object.
  if (lock_routine_name(thd, type, name, MDL_SHARED)) return SP_INTERNAL_ERROR;

  // Find routine in the data dictionary.
  enum_sp_return_code ret;
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  const dd::Routine *routine = nullptr;

  bool error;
  if (type == enum_sp_type::FUNCTION)
    error = thd->dd_client()->acquire<dd::Function>(name->m_db.str,
                                                    name->m_name.str, &routine);
  else
    error = thd->dd_client()->acquire<dd::Procedure>(
        name->m_db.str, name->m_name.str, &routine);

  if (error) return SP_INTERNAL_ERROR;

  if (routine == nullptr) return SP_DOES_NOT_EXISTS;

  // prepare sp_chistics from the dd::routine object.
  st_sp_chistics sp_chistics;
  prepare_sp_chistics_from_dd_routine(routine, &sp_chistics);

  // prepare stored routine's return type string.
  dd::String_type return_type_str;
  prepare_return_type_string_from_dd_routine(thd, routine, &return_type_str);

  // prepare stored routine's parameters string.
  dd::String_type params_str;
  prepare_params_string_from_dd_routine(thd, routine, &params_str);

  // Create stored routine creation context from the dd::Routine object.
  Stored_program_creation_ctx *creation_ctx =
      Stored_routine_creation_ctx::create_routine_creation_ctx(routine);
  if (creation_ctx == nullptr) return SP_INTERNAL_ERROR;

  DBUG_EXECUTE_IF("fail_stored_routine_load", return SP_PARSE_ERROR;);

  /*
    Create sp_head object for the stored routine from the information obtained
    from the dd::Routine object.
  */
  ret = db_load_routine(
      thd, type, name->m_db.str, name->m_db.length, routine->name().c_str(),
      routine->name().length(), sphp, routine->sql_mode(), params_str.c_str(),
      return_type_str.c_str(), routine->definition().c_str(), &sp_chistics,
      routine->definer_user().c_str(), routine->definer_host().c_str(),
      routine->created(true), routine->last_altered(true), creation_ctx);
  return ret;
}

namespace {

/**
  Silence DEPRECATED SYNTAX warnings when loading a stored procedure
  into the cache.
*/
class Silence_deprecated_warning final : public Internal_error_handler {
 public:
  bool handle_condition(THD *, uint sql_errno, const char *,
                        Sql_condition::enum_severity_level *level,
                        const char *) override {
    if ((sql_errno == ER_WARN_DEPRECATED_SYNTAX ||
         sql_errno == ER_WARN_DEPRECATED_SYNTAX_NO_REPLACEMENT) &&
        (*level) == Sql_condition::SL_WARNING)
      return true;

    return false;
  }
};

}  // namespace

/**
  The function parses input strings and returns SP structure.

  @param[in]      thd               Thread handler
  @param[in]      defstr            CREATE... string
  @param[in]      sql_mode          SQL mode
  @param[in]      creation_ctx      Creation context of stored routines

  @retval         Pointer on sp_head struct   Success
  @retval         NULL                        error
*/

static sp_head *sp_compile(THD *thd, String *defstr, sql_mode_t sql_mode,
                           Stored_program_creation_ctx *creation_ctx) {
  sp_head *sp;
  sql_mode_t old_sql_mode = thd->variables.sql_mode;
  ha_rows old_select_limit = thd->variables.select_limit;
  sp_rcontext *sp_runtime_ctx_saved = thd->sp_runtime_ctx;
  Silence_deprecated_warning warning_handler;
  Parser_state parser_state;
  sql_digest_state *parent_digest = thd->m_digest;
  PSI_statement_locker *parent_locker = thd->m_statement_psi;

  thd->variables.sql_mode = sql_mode;
  thd->variables.select_limit = HA_POS_ERROR;

  if (parser_state.init(thd, defstr->c_ptr(), defstr->length())) {
    thd->variables.sql_mode = old_sql_mode;
    thd->variables.select_limit = old_select_limit;
    return nullptr;
  }

  lex_start(thd);
  thd->push_internal_handler(&warning_handler);
  thd->sp_runtime_ctx = nullptr;

  thd->m_digest = nullptr;
  thd->m_statement_psi = nullptr;
  if (parse_sql(thd, &parser_state, creation_ctx) || thd->lex == nullptr) {
    sp = thd->lex->sphead;
    sp_head::destroy(sp);
    sp = nullptr;
  } else {
    sp = thd->lex->sphead;
  }
  thd->m_digest = parent_digest;
  thd->m_statement_psi = parent_locker;

  thd->pop_internal_handler();
  thd->sp_runtime_ctx = sp_runtime_ctx_saved;
  thd->variables.sql_mode = old_sql_mode;
  thd->variables.select_limit = old_select_limit;
#ifdef HAVE_PSI_SP_INTERFACE
  if (sp != nullptr)
    sp->m_sp_share =
        MYSQL_GET_SP_SHARE(static_cast<uint>(sp->m_type), sp->m_db.str,
                           sp->m_db.length, sp->m_name.str, sp->m_name.length);
#endif
  return sp;
}

class Bad_db_error_handler : public Internal_error_handler {
 public:
  Bad_db_error_handler() : m_error_caught(false) {}

  virtual bool handle_condition(THD *, uint sql_errno, const char *,
                                Sql_condition::enum_severity_level *,
                                const char *) {
    if (sql_errno == ER_BAD_DB_ERROR) {
      m_error_caught = true;
      return true;
    }
    return false;
  }

  bool error_caught() const { return m_error_caught; }

 private:
  bool m_error_caught;
};

enum_sp_return_code db_load_routine(
    THD *thd, enum_sp_type type, const char *sp_db, size_t sp_db_len,
    const char *sp_name, size_t sp_name_len, sp_head **sphp,
    sql_mode_t sql_mode, const char *params, const char *returns,
    const char *body, st_sp_chistics *sp_chistics, const char *definer_user,
    const char *definer_host, longlong created, longlong modified,
    Stored_program_creation_ctx *creation_ctx) {
  LEX *old_lex = thd->lex, newlex;
  char saved_cur_db_name_buf[NAME_LEN + 1];
  LEX_STRING saved_cur_db_name = {saved_cur_db_name_buf,
                                  sizeof(saved_cur_db_name_buf)};
  bool cur_db_changed;
  Bad_db_error_handler db_not_exists_handler;
  enum_sp_return_code ret = SP_OK;

  thd->lex = &newlex;
  newlex.thd = thd;
  newlex.set_current_select(nullptr);

  String defstr;
  defstr.set_charset(creation_ctx->get_client_cs());

  LEX_CSTRING user = {definer_user, strlen(definer_user)};
  LEX_CSTRING host = {definer_host, strlen(definer_host)};

  if (!create_string(thd, &defstr, type, nullptr, 0, sp_name, sp_name_len,
                     params, strlen(params), returns, strlen(returns), body,
                     strlen(body), sp_chistics, user, host, sql_mode)) {
    ret = SP_INTERNAL_ERROR;
    goto end;
  }

  thd->push_internal_handler(&db_not_exists_handler);
  /*
    Change the current database (if needed).

    TODO: why do we force switch here?
  */
  if (mysql_opt_change_db(thd, {sp_db, sp_db_len}, &saved_cur_db_name, true,
                          &cur_db_changed)) {
    ret = SP_INTERNAL_ERROR;
    thd->pop_internal_handler();
    goto end;
  }
  thd->pop_internal_handler();
  if (db_not_exists_handler.error_caught()) {
    ret = SP_INTERNAL_ERROR;
    my_error(ER_BAD_DB_ERROR, MYF(0), sp_db);

    goto end;
  }

  {
    *sphp = sp_compile(thd, &defstr, sql_mode, creation_ctx);
    /*
      Force switching back to the saved current database (if changed),
      because it may be NULL. In this case, mysql_change_db() would
      generate an error.
    */

    if (cur_db_changed &&
        mysql_change_db(thd, to_lex_cstring(saved_cur_db_name), true)) {
      sp_head::destroy(*sphp);
      *sphp = nullptr;
      ret = SP_INTERNAL_ERROR;
      goto end;
    }

    if (!*sphp) {
      ret = SP_PARSE_ERROR;
      goto end;
    }

    (*sphp)->set_definer(user, host);
    (*sphp)->set_info(created, modified, sp_chistics, sql_mode);
    (*sphp)->set_creation_ctx(creation_ctx);
    (*sphp)->optimize();
    /*
      Not strictly necessary to invoke this method here, since we know
      that we've parsed CREATE PROCEDURE/FUNCTION and not an
      UPDATE/DELETE/INSERT/REPLACE/LOAD/CREATE TABLE, but we try to
      maintain the invariant that this method is called for each
      distinct statement, in case its logic is extended with other
      types of analyses in future.
    */
    newlex.set_trg_event_type_for_tables();
  }

end:
  thd->lex->sphead = nullptr;
  lex_end(thd->lex);
  thd->lex = old_lex;
  return ret;
}

/**
  Precheck for create routine statement.

  @param  thd      Thread context.
  @param  sp       Stored routine object to store.

  @retval  false   Success.
  @retval  true    Error.
*/

static bool create_routine_precheck(THD *thd, sp_head *sp) {
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());

  // Check if routine with same name exists.
  bool error;
  const dd::Routine *sr;
  if (sp->m_type == enum_sp_type::FUNCTION)
    error = thd->dd_client()->acquire<dd::Function>(sp->m_db.str,
                                                    sp->m_name.str, &sr);
  else
    error = thd->dd_client()->acquire<dd::Procedure>(sp->m_db.str,
                                                     sp->m_name.str, &sr);
  if (error) {
    // Error is reported by DD API framework.
    return true;
  }
  if (sr != nullptr) {
    my_error(ER_SP_ALREADY_EXISTS, MYF(0), SP_TYPE_STRING(sp->m_type),
             sp->m_name.str);
    return true;
  }

  /*
    Check if stored function creation is allowed only to the users having SUPER
    privileges.
  */
  if (mysql_bin_log.is_open() && (sp->m_type == enum_sp_type::FUNCTION) &&
      !trust_function_creators) {
    if (!sp->m_chistics->detistic) {
      /*
        Note that this test is not perfect; one could use
        a non-deterministic read-only function in an update statement.
      */
      enum enum_sp_data_access access =
          (sp->m_chistics->daccess == SP_DEFAULT_ACCESS)
              ? static_cast<enum_sp_data_access>(SP_DEFAULT_ACCESS_MAPPING)
              : sp->m_chistics->daccess;
      if (access == SP_CONTAINS_SQL || access == SP_MODIFIES_SQL_DATA) {
        my_error(ER_BINLOG_UNSAFE_ROUTINE, MYF(0));
        return true;
      }
    }
    if (!(thd->security_context()->check_access(SUPER_ACL))) {
      my_error(ER_BINLOG_CREATE_ROUTINE_NEED_SUPER, MYF(0));
      return true;
    }
  }

  /*
    Check routine body length.
    Note: Length of routine name and parameters name is already verified in
    parsing phase.
  */
  if (sp->m_body.length > MYSQL_STORED_ROUTINE_BODY_LENGTH ||
      DBUG_EVALUATE_IF("simulate_routine_length_error", 1, 0)) {
    my_error(ER_TOO_LONG_BODY, MYF(0), sp->m_name.str);
    return true;
  }

  // Validate body definition to avoid invalid UTF8 characters.
  if (is_invalid_string(sp->m_body_utf8, system_charset_info)) return true;

  // Validate routine comment.
  if (sp->m_chistics->comment.length) {
    // validate comment string to avoid invalid utf8 characters.
    if (is_invalid_string(LEX_CSTRING{sp->m_chistics->comment.str,
                                      sp->m_chistics->comment.length},
                          system_charset_info))
      return true;

    // Check comment string length.
    if (check_string_char_length(
            {sp->m_chistics->comment.str, sp->m_chistics->comment.length}, "",
            MYSQL_STORED_ROUTINE_COMMENT_LENGTH, system_charset_info, true)) {
      my_error(ER_TOO_LONG_ROUTINE_COMMENT, MYF(0), sp->m_chistics->comment.str,
               MYSQL_STORED_ROUTINE_COMMENT_LENGTH);
      return true;
    }
  }

  return false;
}

/**
  Creates a stored routine.

  Atomicity:
    The operation to create a stored routine is atomic/crash-safe.
    Changes to the Data-dictionary and writing event to binlog are
    part of the same transaction. All the changes are done as part
    of the same transaction or do not have any side effects on the
    operation failure. Data-dictionary, stored routines and table
    definition caches are in sync with operation state. Cache do
    not contain any stale/incorrect data in case of failure.
    In case of crash, there won't be any discrepancy between
    the data-dictionary table and the binary log.

  @param thd     Thread context.
  @param sp      Stored routine object to store.
  @param definer Definer of the SP.

  @retval false success
  @retval true  error
*/

bool sp_create_routine(THD *thd, sp_head *sp, const LEX_USER *definer) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("type: %d  name: %.*s", static_cast<int>(sp->m_type),
                       static_cast<int>(sp->m_name.length), sp->m_name.str));

  DBUG_ASSERT(sp->m_type == enum_sp_type::PROCEDURE ||
              sp->m_type == enum_sp_type::FUNCTION);

  /* Grab an exclusive MDL lock. */
  MDL_key::enum_mdl_namespace mdl_type = (sp->m_type == enum_sp_type::FUNCTION)
                                             ? MDL_key::FUNCTION
                                             : MDL_key::PROCEDURE;
  if (lock_object_name(thd, mdl_type, sp->m_db.str, sp->m_name.str)) {
    my_error(ER_SP_STORE_FAILED, MYF(0), SP_TYPE_STRING(sp->m_type),
             sp->m_name.str);
    return true;
  }
  DEBUG_SYNC(thd, "after_acquiring_mdl_lock_on_routine");

  if (create_routine_precheck(thd, sp)) {
    /* If this happens, an error should have been reported. */
    return true;
  }

  DBUG_EXECUTE_IF("fail_while_acquiring_routine_schema_obj",
                  DBUG_SET("+d,fail_while_acquiring_dd_object"););

  // Check that a database with this name exists.
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  const dd::Schema *schema = nullptr;
  if (thd->dd_client()->acquire(sp->m_db.str, &schema)) {
    DBUG_EXECUTE_IF("fail_while_acquiring_routine_schema_obj",
                    DBUG_SET("-d,fail_while_acquiring_dd_object"););
    // Error is reported by DD API framework.
    return true;
  }
  if (schema == nullptr) {
    my_error(ER_BAD_DB_ERROR, MYF(0), sp->m_db.str);
    return true;
  }

  // Create a stored routine.
  if (dd::create_routine(thd, *schema, sp, definer))
    goto err_report_with_rollback;

  // Update referencing views metadata.
  {
    sp_name spname({sp->m_db.str, sp->m_db.length}, sp->m_name, false);
    if (sp->m_type == enum_sp_type::FUNCTION &&
        update_referencing_views_metadata(thd, &spname)) {
      /* If this happens, an error should have been reported. */
      goto err_with_rollback;
    }
  }

  // Log stored routine create event.
  if (mysql_bin_log.is_open()) {
    String log_query;
    log_query.set_charset(system_charset_info);

    String retstr(64);
    retstr.set_charset(system_charset_info);
    if (sp->m_type == enum_sp_type::FUNCTION) sp->returns_type(thd, &retstr);

    if (!create_string(thd, &log_query, sp->m_type,
                       (sp->m_explicit_name ? sp->m_db.str : nullptr),
                       (sp->m_explicit_name ? sp->m_db.length : 0),
                       sp->m_name.str, sp->m_name.length, sp->m_params.str,
                       sp->m_params.length, retstr.c_ptr(), retstr.length(),
                       sp->m_body.str, sp->m_body.length, sp->m_chistics,
                       definer->user, definer->host, thd->variables.sql_mode))
      goto err_report_with_rollback;

    thd->add_to_binlog_accessed_dbs(sp->m_db.str);

    /*
      This statement will be replicated as a statement, even when using
      row-based replication.
    */
    Save_and_Restore_binlog_format_state binlog_format_state(thd);

    if (write_bin_log(thd, true, log_query.c_ptr(), log_query.length(), true))
      goto err_report_with_rollback;
  }

  // Commit changes to the data-dictionary and binary log.
  if (DBUG_EVALUATE_IF("simulate_create_routine_failure", true, false) ||
      trans_commit_stmt(thd) || trans_commit(thd))
    goto err_report_with_rollback;

  // Invalidate stored routine cache.
  sp_cache_invalidate();

  return false;

err_report_with_rollback:
  my_error(ER_SP_STORE_FAILED, MYF(0), SP_TYPE_STRING(sp->m_type),
           sp->m_name.str);

err_with_rollback:
  trans_rollback_stmt(thd);
  /*
    Full rollback in case we have THD::transaction_rollback_request
    and to synchronize DD state in cache and on disk (as statement
    rollback doesn't clear DD cache of modified uncommitted objects).
  */
  trans_rollback(thd);

  return true;
}

/**
  Drops a stored routine.

  Atomicity:
    The operation to drop a stored routine is atomic/crash-safe.
    Changes to the Data-dictionary and writing event to binlog are
    part of the same transaction. All the changes are done as part
    of the same transaction or do not have any side effects on the
    operation failure. Data-dictionary, stored routines and table
    definition caches are in sync with operation state. Cache do
    not contain any stale/incorrect data in case of failure.
    In case of crash, there won't be any discrepancy between
    the data-dictionary table and the binary log.

  @param thd  Thread context.
  @param type Stored routine type
              (PROCEDURE or FUNCTION)
  @param name Stored routine name.

  @return Error code. SP_OK is returned on success. Other SP_ constants are
  used to indicate about errors.
*/

enum_sp_return_code sp_drop_routine(THD *thd, enum_sp_type type,
                                    sp_name *name) {
  DBUG_TRACE;
  DBUG_PRINT("enter",
             ("type: %d  name: %.*s", static_cast<int>(type),
              static_cast<int>(name->m_name.length), name->m_name.str));

  DBUG_ASSERT(type == enum_sp_type::PROCEDURE ||
              type == enum_sp_type::FUNCTION);

  /* Grab an exclusive MDL lock. */
  MDL_key::enum_mdl_namespace mdl_type =
      (type == enum_sp_type::FUNCTION) ? MDL_key::FUNCTION : MDL_key::PROCEDURE;
  if (lock_object_name(thd, mdl_type, name->m_db.str, name->m_name.str))
    return SP_DROP_FAILED;

  DEBUG_SYNC(thd, "after_acquiring_mdl_lock_on_routine");

  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  const dd::Routine *routine = nullptr;

  bool error;
  if (type == enum_sp_type::FUNCTION)
    error = thd->dd_client()->acquire<dd::Function>(name->m_db.str,
                                                    name->m_name.str, &routine);
  else
    error = thd->dd_client()->acquire<dd::Procedure>(
        name->m_db.str, name->m_name.str, &routine);
  if (error) return SP_INTERNAL_ERROR;

  if (routine == nullptr) return SP_DOES_NOT_EXISTS;
  /*
    If definer has the SYSTEM_USER privilege then invoker can drop procedure
    only if latter also has same privilege.
  */
  Auth_id definer(routine->definer_user().c_str(),
                  routine->definer_host().c_str());
  Security_context *sctx = thd->security_context();
  if (sctx->can_operate_with(definer, consts::system_user, true))
    return SP_INTERNAL_ERROR;

  // Drop routine.
  if (thd->dd_client()->drop(routine)) goto err_with_rollback;

  // Update referencing views metadata.
  if (mdl_type == MDL_key::FUNCTION &&
      update_referencing_views_metadata(thd, name)) {
    /* If this happens, an error should have been reported. */
    goto err_with_rollback;
  }

  // Log drop routine event.
  if (mysql_bin_log.is_open()) {
    thd->add_to_binlog_accessed_dbs(name->m_db.str);
    /*
      This statement will be replicated as a statement, even when using
      row-based replication.
    */
    Save_and_Restore_binlog_format_state binlog_format_state(thd);

    if (write_bin_log(thd, true, thd->query().str, thd->query().length, true))
      goto err_with_rollback;
  }

  // Commit changes to the data-dictionary and binary log.
  if (DBUG_EVALUATE_IF("simulate_drop_routine_failure", true, false) ||
      trans_commit_stmt(thd) || trans_commit(thd))
    goto err_with_rollback;

#ifdef HAVE_PSI_SP_INTERFACE
  /* Drop statistics for this stored program from performance schema. */
  MYSQL_DROP_SP(static_cast<uint>(type), name->m_db.str, name->m_db.length,
                name->m_name.str, name->m_name.length);
#endif

  // Invalidate routine cache.
  {
    sp_cache_invalidate();

    /*
      A lame workaround for lack of cache flush:
      make sure the routine is at least gone from the
      local cache.
    */
    {
      sp_head *sp;
      sp_cache **spc = (type == enum_sp_type::FUNCTION ? &thd->sp_func_cache
                                                       : &thd->sp_proc_cache);
      sp = sp_cache_lookup(spc, name);
      if (sp) sp_cache_flush_obsolete(spc, &sp);
    }
  }

  return SP_OK;

err_with_rollback:
  trans_rollback_stmt(thd);
  /*
    Full rollback in case we have THD::transaction_rollback_request
    and to synchronize DD state in cache and on disk (as statement
    rollback doesn't clear DD cache of modified uncommitted objects).
  */
  trans_rollback(thd);

  return SP_DROP_FAILED;
}

/**
  Updates(Alter) a stored routine.

  Atomicity:
    The operation to Update(Alter) a stored routine is atomic/crash-safe.
    Changes to the Data-dictionary and writing event to binlog are
    part of the same transaction. All the changes are done as part
    of the same transaction or do not have any side effects on the
    operation failure. Data-dictionary and stored routines caches
    caches are in sync with operation state. Cache do not contain any
    stale/incorrect data in case of failure.
    In case of crash, there won't be any discrepancy between
    the data-dictionary table and the binary log.

  @param thd      Thread context.
  @param type     Stored routine type
                  (PROCEDURE or FUNCTION)
  @param name     Stored routine name.
  @param chistics New values of stored routine attributes to write.

  @retval    false    Success.
  @retval    true     Error.
*/

bool sp_update_routine(THD *thd, enum_sp_type type, sp_name *name,
                       st_sp_chistics *chistics) {
  DBUG_TRACE;
  DBUG_PRINT("enter",
             ("type: %d  name: %.*s", static_cast<int>(type),
              static_cast<int>(name->m_name.length), name->m_name.str));

  DBUG_ASSERT(type == enum_sp_type::PROCEDURE ||
              type == enum_sp_type::FUNCTION);

  /* Grab an exclusive MDL lock. */
  MDL_key::enum_mdl_namespace mdl_type =
      (type == enum_sp_type::FUNCTION) ? MDL_key::FUNCTION : MDL_key::PROCEDURE;
  if (lock_object_name(thd, mdl_type, name->m_db.str, name->m_name.str)) {
    my_error(ER_SP_CANT_ALTER, MYF(0), SP_TYPE_STRING(type), name->m_name.str);
    return true;
  }

  // Check if routine exists.
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  dd::Routine *routine = nullptr;
  bool error;
  if (type == enum_sp_type::FUNCTION)
    error = thd->dd_client()->acquire_for_modification<dd::Function>(
        name->m_db.str, name->m_name.str, &routine);
  else
    error = thd->dd_client()->acquire_for_modification<dd::Procedure>(
        name->m_db.str, name->m_name.str, &routine);
  if (error) {
    // Error is reported by DD API framework.
    return true;
  }

  if (routine == nullptr) {
    my_error(ER_SP_DOES_NOT_EXIST, MYF(0), SP_TYPE_STRING(type),
             thd->lex->spname->m_qname.str);
    return true;
  }
  /*
    If definer has the SYSTEM_USER privilege then invoker can alter procedure
    only if latter also has same privilege.
  */
  Auth_id definer(routine->definer_user().c_str(),
                  routine->definer_host().c_str());
  Security_context *sctx = thd->security_context();
  if (sctx->can_operate_with(definer, consts::system_user, true)) return true;

  if (mysql_bin_log.is_open() && type == enum_sp_type::FUNCTION &&
      !trust_function_creators &&
      (chistics->daccess == SP_CONTAINS_SQL ||
       chistics->daccess == SP_MODIFIES_SQL_DATA)) {
    if (!routine->is_deterministic()) {
      my_error(ER_BINLOG_UNSAFE_ROUTINE, MYF(0));
      return true;
    }
  }

  // Validate routine comment.
  if (chistics->comment.str) {
    // validate comment string to invalid utf8 characters.
    if (is_invalid_string(chistics->comment, system_charset_info)) return true;

    // Check comment string length.
    if (check_string_char_length(
            {chistics->comment.str, chistics->comment.length}, "",
            MYSQL_STORED_ROUTINE_COMMENT_LENGTH, system_charset_info, true)) {
      my_error(ER_TOO_LONG_ROUTINE_COMMENT, MYF(0), chistics->comment.str,
               MYSQL_STORED_ROUTINE_COMMENT_LENGTH);
      return true;
    }
  }

  // Alter stored routine.
  if (DBUG_EVALUATE_IF("simulate_alter_routine_failure", true, false) ||
      dd::alter_routine(thd, routine, chistics))
    goto err_report_with_rollback;

  // Log update statement.
  if (mysql_bin_log.is_open()) {
    /*
      This statement will be replicated as a statement, even when using
      row-based replication.
    */
    Save_and_Restore_binlog_format_state binlog_format_state(thd);

    if (write_bin_log(thd, true, thd->query().str, thd->query().length, true))
      goto err_report_with_rollback;
  }

  // Commit changes to the data-dictionary and binary log.
  if (DBUG_EVALUATE_IF("simulate_alter_routine_xcommit_failure", true, false) ||
      trans_commit_stmt(thd) || trans_commit(thd))
    goto err_report_with_rollback;

  sp_cache_invalidate();

  return false;

err_report_with_rollback:
  my_error(ER_SP_CANT_ALTER, MYF(0), SP_TYPE_STRING(type),
           thd->lex->spname->m_qname.str);

  trans_rollback_stmt(thd);
  /*
    Full rollback in case we have THD::transaction_rollback_request
    and to synchronize DD state in cache and on disk (as statement
    rollback doesn't clear DD cache of modified uncommitted objects).
  */
  trans_rollback(thd);

  return true;
}

bool lock_db_routines(THD *thd, const dd::Schema &schema) {
  DBUG_TRACE;

  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());

  // Vector for the stored routines of the schema.
  std::vector<const dd::Routine *> routines;

  // Fetch stored routines of the schema.
  if (thd->dd_client()->fetch_schema_components(&schema, &routines))
    return true;

  /*
    If lower_case_table_names == 2 then schema names should be lower cased for
    proper hash key comparisons.
  */
  const char *schema_name = schema.name().c_str();
  char schema_name_buf[NAME_LEN + 1];
  if (lower_case_table_names == 2) {
    my_stpcpy(schema_name_buf, schema_name);
    my_casedn_str(system_charset_info, schema_name_buf);
    schema_name = schema_name_buf;
  }

  MDL_request_list mdl_requests;
  for (const dd::Routine *routine : routines) {
    MDL_key mdl_key;

    if (is_dd_routine_type_function(routine))
      dd::Function::create_mdl_key(dd::String_type(schema_name),
                                   routine->name(), &mdl_key);
    else
      dd::Procedure::create_mdl_key(dd::String_type(schema_name),
                                    routine->name(), &mdl_key);

    // Add MDL_request for routine to mdl_requests list.
    MDL_request *mdl_request = new (thd->mem_root) MDL_request;
    MDL_REQUEST_INIT_BY_KEY(mdl_request, &mdl_key, MDL_EXCLUSIVE,
                            MDL_TRANSACTION);
    mdl_requests.push_front(mdl_request);
  }

  return thd->mdl_context.acquire_locks_nsec(
      &mdl_requests, thd->variables.lock_wait_timeout_nsec);
}

/**
  Drop all routines in database 'db'

  @param   thd         Thread context.
  @param   schema      Schema object.

  @retval  SP_OK       Success
  @retval  non-SP_OK   Error (Other constants are used to indicate errors)
*/

enum_sp_return_code sp_drop_db_routines(THD *thd, const dd::Schema &schema) {
  DBUG_TRACE;

  bool is_routine_dropped = false;

  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());

  // Vector for the stored routines of the schema.
  std::vector<const dd::Routine *> routines;

  // Fetch stored routines of the schema.
  if (thd->dd_client()->fetch_schema_components(&schema, &routines))
    return SP_INTERNAL_ERROR;

  enum_sp_return_code ret_code = SP_OK;
  for (const dd::Routine *routine : routines) {
    sp_name name(
        {schema.name().c_str(), schema.name().length()},
        {const_cast<char *>(routine->name().c_str()), routine->name().length()},
        false);
    enum_sp_type type = is_dd_routine_type_function(routine)
                            ? enum_sp_type::FUNCTION
                            : enum_sp_type::PROCEDURE;

    DBUG_EXECUTE_IF("fail_drop_db_routines", {
      my_error(ER_SP_DROP_FAILED, MYF(0), "ROUTINE", "");
      return SP_DROP_FAILED;
    });

    if (thd->dd_client()->drop(routine)) {
      ret_code = SP_DROP_FAILED;
      my_error(ER_SP_DROP_FAILED, MYF(0),
               is_dd_routine_type_function(routine) ? "FUNCTION" : "PROCEDURE",
               routine->name().c_str());
      break;
    }

    if (type == enum_sp_type::FUNCTION &&
        update_referencing_views_metadata(thd, &name))
      return SP_INTERNAL_ERROR;

    is_routine_dropped = true;

#ifdef HAVE_PSI_SP_INTERFACE
    /* Drop statistics for this stored routine from performance schema. */
    MYSQL_DROP_SP(to_uint(type), schema.name().c_str(), schema.name().length(),
                  routine->name().c_str(), routine->name().length());
#endif
  }

  // Invalidate the sp cache.
  if (is_routine_dropped) sp_cache_invalidate();

  return ret_code;
}

/**
  Prepare show create routine output from the DD routine object.

  @param[in] thd       Thread handle.
  @param[in] type      Stored routine type.
  @param[in] sp        Stored routine name.
  @param[in] routine   Routine object read for the routine.

  @retval    false     on success
  @retval    true      on error
*/
static bool show_create_routine_from_dd_routine(THD *thd, enum_sp_type type,
                                                sp_name *sp,
                                                const dd::Routine *routine) {
  DBUG_TRACE;

  /*
    Check if user has full access to the routine properties (i.e including
    stored routine code), or partial access (i.e to view its other properties).
  */

  bool full_access = has_full_view_routine_access(
      thd, sp->m_db.str, routine->definer_user().c_str(),
      routine->definer_host().c_str());

  if (!full_access &&
      !has_partial_view_routine_access(thd, sp->m_db.str, sp->m_name.str,
                                       type == enum_sp_type::PROCEDURE))
    return true;

  // prepare st_sp_chistics object from the dd::Routine.
  st_sp_chistics sp_chistics;
  prepare_sp_chistics_from_dd_routine(routine, &sp_chistics);

  // prepare stored routine return type string.
  dd::String_type return_type_str;
  prepare_return_type_string_from_dd_routine(thd, routine, &return_type_str);

  // Prepare stored routine definition string.
  String defstr;
  defstr.set_charset(system_charset_info);
  if (!create_string(
          thd, &defstr, type, nullptr, 0, routine->name().c_str(),
          routine->name().length(), routine->parameter_str().c_str(),
          routine->parameter_str().length(), return_type_str.c_str(),
          return_type_str.length(), routine->definition().c_str(),
          routine->definition().length(), &sp_chistics,
          {routine->definer_user().c_str(), routine->definer_user().length()},
          {routine->definer_host().c_str(), routine->definer_host().length()},
          routine->sql_mode()))
    return true;

  // Prepare sql_mode string representation.
  LEX_STRING sql_mode;
  sql_mode_string_representation(thd, routine->sql_mode(), &sql_mode);

  /* Send header. */
  List<Item> fields;
  // Column type
  const char *col1_caption =
      (type == enum_sp_type::PROCEDURE) ? "Procedure" : "Function";
  fields.push_back(new Item_empty_string(col1_caption, NAME_CHAR_LEN));

  // Column sql_mode
  fields.push_back(new Item_empty_string("sql_mode", sql_mode.length));

  // Column Create Procedure/Function.
  {
    const char *col3_caption = (type == enum_sp_type::PROCEDURE)
                                   ? "Create Procedure"
                                   : "Create Function";
    /*
      NOTE: SQL statement field must be not less than 1024 in order not to
      confuse old clients.
    */
    Item_empty_string *stmt_fld = new Item_empty_string(
        col3_caption, std::max<size_t>(defstr.length(), 1024U));

    stmt_fld->maybe_null = true;

    fields.push_back(stmt_fld);
  }

  // Column character_set_client.
  fields.push_back(
      new Item_empty_string("character_set_client", MY_CS_NAME_SIZE));

  // Column collation collation.
  fields.push_back(
      new Item_empty_string("collation_connection", MY_CS_NAME_SIZE));

  // Column database collection.
  fields.push_back(
      new Item_empty_string("Database Collation", MY_CS_NAME_SIZE));

  if (thd->send_result_metadata(&fields,
                                Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF))
    return true;

  /* Send data. */
  Protocol *protocol = thd->get_protocol();
  protocol->start_row();

  // Routine Name
  protocol->store_string(routine->name().c_str(), routine->name().length(),
                         system_charset_info);

  // sql mode.
  protocol->store_string(sql_mode.str, sql_mode.length, system_charset_info);

  // Routine definition.
  const CHARSET_INFO *cs_info =
      dd_get_mysql_charset(routine->client_collation_id());
  if (full_access)
    protocol->store_string(defstr.c_ptr(), defstr.length(), cs_info);
  else
    protocol->store_null();

  // character_set_client
  protocol->store(cs_info->csname, system_charset_info);
  // connection_collation
  cs_info = dd_get_mysql_charset(routine->connection_collation_id());
  protocol->store(cs_info->name, system_charset_info);
  // database_collation
  cs_info = dd_get_mysql_charset(routine->schema_collation_id());
  protocol->store(cs_info->name, system_charset_info);

  bool err_status = protocol->end_row();

  if (!err_status) my_eof(thd);

  return err_status;
}

/**
  Implement SHOW CREATE statement for stored routines.

  The operation finds the stored routine object specified by name and
  then calls show_create_routine_from_dd_routine().

  @param thd  Thread context.
  @param type Stored routine type
              (PROCEDURE or FUNCTION)
  @param name Stored routine name.

  @retval false on success
  @retval true on error
*/

bool sp_show_create_routine(THD *thd, enum_sp_type type, sp_name *name) {
  DBUG_TRACE;
  DBUG_PRINT("enter",
             ("name: %.*s", (int)name->m_name.length, name->m_name.str));

  DBUG_ASSERT(type == enum_sp_type::PROCEDURE ||
              type == enum_sp_type::FUNCTION);

  // Lock routine for read.
  if (lock_routine_name(thd, type, name, MDL_SHARED_HIGH_PRIO)) {
    my_error(ER_SP_LOAD_FAILED, MYF(0), name->m_name.str);
    return true;
  }

  // Find routine in data dictionary.
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  const dd::Routine *routine = nullptr;

  bool error;
  if (type == enum_sp_type::FUNCTION)
    error = thd->dd_client()->acquire<dd::Function>(name->m_db.str,
                                                    name->m_name.str, &routine);
  else
    error = thd->dd_client()->acquire<dd::Procedure>(
        name->m_db.str, name->m_name.str, &routine);

  if (error) {
    my_error(ER_SP_LOAD_FAILED, MYF(0), name->m_name.str);
    return true;
  }

  // show create routine.
  if (routine == nullptr ||
      show_create_routine_from_dd_routine(thd, type, name, routine)) {
    /*
      If we have insufficient privileges, pretend the routine
      does not exist.
    */
    my_error(ER_SP_DOES_NOT_EXIST, MYF(0),
             type == enum_sp_type::FUNCTION ? "FUNCTION" : "PROCEDURE",
             name->m_name.str);
    return true;
  }

  return false;
}

/**
  Obtain object representing stored procedure/function by its name from
  stored procedures cache and looking into data dictionary if needed.

  @param thd          thread context
  @param type         type of object (FUNCTION or PROCEDURE)
  @param name         name of procedure
  @param cp           hash to look routine in
  @param cache_only   if true perform cache-only lookup
                      (Don't look in data dictionary)

  @retval
    NonNULL pointer to sp_head object for the procedure
  @retval
    NULL    in case of error.
*/

sp_head *sp_find_routine(THD *thd, enum_sp_type type, sp_name *name,
                         sp_cache **cp, bool cache_only) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("name:  %.*s.%.*s  type: %d  cache only %d",
                       static_cast<int>(name->m_db.length), name->m_db.str,
                       static_cast<int>(name->m_name.length), name->m_name.str,
                       static_cast<int>(type), cache_only));

  sp_head *sp = sp_cache_lookup(cp, name);
  if (sp != nullptr) return sp;

  if (!cache_only) {
    if (db_find_routine(thd, type, name, &sp) == SP_OK) {
      sp_cache_insert(cp, sp);
      DBUG_PRINT("info", ("added new: 0x%lx, level: %lu, flags %x", (ulong)sp,
                          sp->m_recursion_level, sp->m_flags));
    }
  }
  return sp;
}

/**
  Setup a cached routine for execution

  @param thd          thread context
  @param type         type of object (FUNCTION or PROCEDURE)
  @param name         name of procedure
  @param cp           hash to look routine in

  @retval
    NonNULL pointer to sp_head object for the procedure
  @retval
    NULL    in case of error.
*/

sp_head *sp_setup_routine(THD *thd, enum_sp_type type, sp_name *name,
                          sp_cache **cp) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("name:  %.*s.%.*s  type: %d ",
                       static_cast<int>(name->m_db.length), name->m_db.str,
                       static_cast<int>(name->m_name.length), name->m_name.str,
                       static_cast<int>(type)));

  sp_head *sp = sp_cache_lookup(cp, name);
  if (sp == nullptr) return nullptr;

  DBUG_PRINT("info", ("found: 0x%lx", (ulong)sp));

  const ulong depth = type == enum_sp_type::PROCEDURE
                          ? thd->variables.max_sp_recursion_depth
                          : 0;

  if (sp->m_first_free_instance) {
    DBUG_PRINT("info", ("first free: 0x%lx  level: %lu  flags %x",
                        (ulong)sp->m_first_free_instance,
                        sp->m_first_free_instance->m_recursion_level,
                        sp->m_first_free_instance->m_flags));
    DBUG_ASSERT(!(sp->m_first_free_instance->m_flags & sp_head::IS_INVOKED));
    if (sp->m_first_free_instance->m_recursion_level > depth) {
      recursion_level_error(thd, sp);
      return nullptr;
    }
    return sp->m_first_free_instance;
  }

  /*
    Actually depth could be +1 than the actual value in case a SP calls
    SHOW CREATE PROCEDURE. Hence, the linked list could hold up to one more
    instance.
  */

  ulong level = sp->m_last_cached_sp->m_recursion_level + 1;
  if (level > depth) {
    recursion_level_error(thd, sp);
    return nullptr;
  }

  const char *returns = "";
  String retstr(64);
  retstr.set_charset(sp->get_creation_ctx()->get_client_cs());
  if (type == enum_sp_type::FUNCTION) {
    sp->returns_type(thd, &retstr);
    returns = retstr.ptr();
  }

  sp_head *new_sp;
  if (db_load_routine(thd, type, name->m_db.str, name->m_db.length,
                      name->m_name.str, name->m_name.length, &new_sp,
                      sp->m_sql_mode, sp->m_params.str, returns, sp->m_body.str,
                      sp->m_chistics, sp->m_definer_user.str,
                      sp->m_definer_host.str, sp->m_created, sp->m_modified,
                      sp->get_creation_ctx()) != SP_OK)
    return nullptr;

  sp->m_last_cached_sp->m_next_cached_sp = new_sp;
  new_sp->m_recursion_level = level;
  new_sp->m_first_instance = sp;
  sp->m_last_cached_sp = sp->m_first_free_instance = new_sp;
  DBUG_PRINT("info", ("added level: 0x%lx, level: %lu, flags %x", (ulong)new_sp,
                      new_sp->m_recursion_level, new_sp->m_flags));
  return new_sp;
}

/**
  This is used by sql_acl.cc:mysql_routine_grant() and is used to find
  the routines in 'routines'.

  @param thd Thread handler
  @param routines List of needles in the hay stack
  @param is_proc  Indicates whether routines in the list are procedures
                  or functions.

  @retval false Found.
  @retval true  Not found
*/

bool sp_exist_routines(THD *thd, TABLE_LIST *routines, bool is_proc) {
  TABLE_LIST *routine;
  bool sp_object_found;
  DBUG_TRACE;
  for (routine = routines; routine; routine = routine->next_global) {
    sp_name *name;
    LEX_CSTRING lex_db;
    LEX_STRING lex_name;
    lex_db.length = strlen(routine->db);
    lex_name.length = strlen(routine->table_name);
    lex_db.str = thd->strmake(routine->db, lex_db.length);
    lex_name.str = thd->strmake(routine->table_name, lex_name.length);
    name = new (thd->mem_root) sp_name(lex_db, lex_name, true);
    name->init_qname(thd);
    sp_object_found =
        is_proc ? sp_find_routine(thd, enum_sp_type::PROCEDURE, name,
                                  &thd->sp_proc_cache, false) != nullptr
                : sp_find_routine(thd, enum_sp_type::FUNCTION, name,
                                  &thd->sp_func_cache, false) != nullptr;
    thd->get_stmt_da()->reset_condition_info(thd);
    if (!sp_object_found) {
      my_error(ER_SP_DOES_NOT_EXIST, MYF(0), is_proc ? "PROCEDURE" : "FUNCTION",
               routine->table_name);
      return true;
    }
  }
  return false;
}

/**
  Auxilary function that adds new element to the set of stored routines
  used by statement.

  The elements of Query_tables_list::sroutines set are accessed on prepared
  statement re-execution. Because of this we have to allocate memory for both
  hash element and copy of its key in persistent arena.

  @param prelocking_ctx  Prelocking context of the statement
  @param arena           Arena in which memory for new element will be
                         allocated
  @param key             Key for the hash representing set
  @param key_length      Key length.
  @param db_length       Length of db name component in the key.
  @param name            Name of the routine.
  @param name_length     Length of the routine name.
  @param belong_to_view  Uppermost view which uses this routine
                         (0 if routine is not used by view)

  @note
    Will also add element to end of 'Query_tables_list::sroutines_list' list.

  @todo
    When we will got rid of these accesses on re-executions we will be
    able to allocate memory for hash elements in non-persitent arena
    and directly use key values from sp_head::m_sroutines sets instead
    of making their copies.

  @retval
    true   new element was added.
  @retval
    false  element was not added (because it is already present in
    the set).
*/

static bool sp_add_used_routine(Query_tables_list *prelocking_ctx,
                                Query_arena *arena, const uchar *key,
                                size_t key_length, size_t db_length,
                                const char *name, size_t name_length,
                                TABLE_LIST *belong_to_view) {
  if (prelocking_ctx->sroutines == nullptr) {
    prelocking_ctx->sroutines.reset(
        new malloc_unordered_map<std::string, Sroutine_hash_entry *>(
            PSI_INSTRUMENT_ME));
  }

  std::string key_str(pointer_cast<const char *>(key), key_length);
  if (prelocking_ctx->sroutines->count(key_str) == 0) {
    Sroutine_hash_entry *rn =
        (Sroutine_hash_entry *)arena->alloc(sizeof(Sroutine_hash_entry));
    if (!rn)  // OOM. Error will be reported using fatal_error().
      return false;
    rn->m_key = (char *)arena->alloc(key_length);
    if (!rn->m_key)  // Ditto.
      return false;
    rn->m_key_length = key_length;
    rn->m_db_length = db_length;
    memcpy(rn->m_key, key, key_length);

    rn->m_object_name = {nullptr, 0};
    if (rn->use_normalized_key() &&
        lex_string_strmake(arena->mem_root, &(rn->m_object_name), name,
                           name_length))
      return false;  // OOM, Error will be reported using fatal_error().

    prelocking_ctx->sroutines->emplace(key_str, rn);
    prelocking_ctx->sroutines_list.link_in_list(rn, &rn->next);
    rn->belong_to_view = belong_to_view;
    rn->m_cache_version = 0;
    return true;
  }
  return false;
}

/**
  Add routine or trigger which is used by statement to the set of
  stored routines used by this statement.

  To be friendly towards prepared statements one should pass
  persistent arena as second argument.

  @param prelocking_ctx                Prelocking context of the statement
  @param arena                         Arena in which memory for new element of
                                       the set will be allocated
  @param type                          Routine type (one of FUNCTION/PROCEDURE/
                                                     TRIGGER ...)
  @param db                            Database name
  @param db_length                     Database name length
  @param name                          Routine name
  @param name_length                   Routine name length
  @param lowercase_db                  Indicates whether db needs to be
                                       lowercased when constructing key.
  @param name_normalize_type           Indicates if names needs to be
                                       normalized (lowercased / accent needs to
                                       be removed).
  @param own_routine                   Indicates whether routine is explicitly
                                       or implicitly used.
  @param belong_to_view                Uppermost view which uses this routine
                                       (nullptr if routine is not used by view)

  @note
    Will also add element to end of 'Query_tables_list::sroutines_list' list
    (and will take into account if this is an explicitly used routine).

  @retval True  - new element was added.
  @retval False - element was not added (because it is already present
          in the set).
*/

bool sp_add_used_routine(Query_tables_list *prelocking_ctx, Query_arena *arena,
                         Sroutine_hash_entry::entry_type type, const char *db,
                         size_t db_length, const char *name, size_t name_length,
                         bool lowercase_db,
                         Sp_name_normalize_type name_normalize_type,
                         bool own_routine, TABLE_LIST *belong_to_view) {
  // Length of routine name components needs to be checked earlier.
  DBUG_ASSERT(db_length <= NAME_LEN && name_length <= NAME_LEN);

  uchar key[1 + NAME_LEN + 1 + NAME_LEN + 1];
  size_t key_length = 0;

  key[key_length++] = static_cast<uchar>(type);
  memcpy(key + key_length, db, db_length + 1);
  if (lowercase_db) {
    /*
      In lower-case-table-names > 0 modes db name will be already in
      lower case here in most cases. However db names associated with
      FKs come here in original form in lower-case-table-names == 2
      mode. So for the proper hash key comparison db name needs to be
      converted to lower case while preparing the key.
    */
    key_length +=
        my_casedn_str(system_charset_info, (char *)(key) + key_length) + 1;
  } else
    key_length += db_length + 1;

  switch (name_normalize_type) {
    case Sp_name_normalize_type::LEAVE_AS_IS:
      memcpy(key + key_length, name, name_length + 1);
      key_length += name_length + 1;
      break;
    case Sp_name_normalize_type::LOWERCASE_NAME:
      memcpy(key + key_length, name, name_length + 1);
      /*
        Object names are case-insensitive. So for the proper hash key
        comparison, object name is converted to the lower case while preparing
        the Sroutine_hash_entry key.
      */
      key_length +=
          my_casedn_str(system_charset_info, (char *)(key) + key_length) + 1;
      break;
    case Sp_name_normalize_type::UNACCENT_AND_LOWERCASE_NAME: {
      const CHARSET_INFO *cs = nullptr;
      switch (type) {
        case Sroutine_hash_entry::FUNCTION:
        case Sroutine_hash_entry::PROCEDURE:
          cs = dd::Routine::name_collation();
          break;
        case Sroutine_hash_entry::TRIGGER:
          cs = dd::Trigger::name_collation();
          break;
        default:
          DBUG_ASSERT(false);
          break;
      }

      /*
        Stored routine names are case and accent insensitive. So for the proper
        hash key comparision, case and accent is stripped off by replacing the
        characters with their sort weight when preparing the Sroutine_hash_entry
        key.
      */
      key_length += dd::normalize_string(cs, name, (char *)(key) + key_length,
                                         NAME_CHAR_LEN * 2);
      *(key + key_length++) = 0;
      break;
    }
    default:
      DBUG_ASSERT(false);
      break;
  }

  if (sp_add_used_routine(prelocking_ctx, arena, key, key_length, db_length,
                          name, name_length, belong_to_view)) {
    if (own_routine) {
      prelocking_ctx->sroutines_list_own_last =
          prelocking_ctx->sroutines_list.next;
      prelocking_ctx->sroutines_list_own_elements =
          prelocking_ctx->sroutines_list.elements;
    }
    return true;
  }

  return false;
}

/**
  Remove routines which are only indirectly used by statement from
  the set of routines used by this statement.

  @param prelocking_ctx  Prelocking context of the statement
*/

void sp_remove_not_own_routines(Query_tables_list *prelocking_ctx) {
  Sroutine_hash_entry *not_own_rt, *next_rt;
  for (not_own_rt = *prelocking_ctx->sroutines_list_own_last; not_own_rt;
       not_own_rt = next_rt) {
    /*
      It is safe to obtain not_own_rt->next after calling hash_delete() now
      but we want to be more future-proof.
    */
    next_rt = not_own_rt->next;
    prelocking_ctx->sroutines->erase(
        std::string(not_own_rt->m_key, not_own_rt->m_key_length));
  }

  *prelocking_ctx->sroutines_list_own_last = nullptr;
  prelocking_ctx->sroutines_list.next = prelocking_ctx->sroutines_list_own_last;
  prelocking_ctx->sroutines_list.elements =
      prelocking_ctx->sroutines_list_own_elements;
}

/**
  Add contents of hash representing set of routines to the set of
  routines used by statement.

  @param thd             Thread context
  @param prelocking_ctx  Prelocking context of the statement
  @param src             Hash representing set from which routines will
                         be added
  @param belong_to_view  Uppermost view which uses these routines, 0 if none

  @note It will also add elements to end of
        'Query_tables_list::sroutines_list' list.
*/

void sp_update_stmt_used_routines(
    THD *thd, Query_tables_list *prelocking_ctx,
    malloc_unordered_map<std::string, Sroutine_hash_entry *> *src,
    TABLE_LIST *belong_to_view) {
  for (const auto &key_and_value : *src) {
    Sroutine_hash_entry *rt = key_and_value.second;
    (void)sp_add_used_routine(prelocking_ctx, thd->stmt_arena,
                              pointer_cast<const uchar *>(rt->m_key),
                              rt->m_key_length, rt->m_db_length, rt->name(),
                              rt->name_length(), belong_to_view);
  }
}

/**
  Add contents of list representing set of routines to the set of
  routines used by statement.

  @param thd             Thread context
  @param prelocking_ctx  Prelocking context of the statement
  @param src             List representing set from which routines will
                         be added
  @param belong_to_view  Uppermost view which uses these routines, 0 if none

  @note It will also add elements to end of
        'Query_tables_list::sroutines_list' list.
*/

void sp_update_stmt_used_routines(THD *thd, Query_tables_list *prelocking_ctx,
                                  SQL_I_List<Sroutine_hash_entry> *src,
                                  TABLE_LIST *belong_to_view) {
  for (Sroutine_hash_entry *rt = src->first; rt; rt = rt->next)
    (void)sp_add_used_routine(prelocking_ctx, thd->stmt_arena,
                              pointer_cast<const uchar *>(rt->m_key),
                              rt->m_key_length, rt->m_db_length, rt->name(),
                              rt->name_length(), belong_to_view);
}

/**
  A helper wrapper around sp_cache_routine() to use from
  prelocking until 'sp_name' is eradicated as a class.
*/

enum_sp_return_code sp_cache_routine(THD *thd, Sroutine_hash_entry *rt,
                                     bool lookup_only, sp_head **sp) {
  char qname_buff[NAME_LEN * 2 + 1 + 1];
  sp_name name(rt, qname_buff);
  enum_sp_type type = (rt->type() == Sroutine_hash_entry::FUNCTION)
                          ? enum_sp_type::FUNCTION
                          : enum_sp_type::PROCEDURE;

#ifndef DBUG_OFF
  MDL_key mdl_key;
  if (rt->type() == Sroutine_hash_entry::FUNCTION)
    dd::Function::create_mdl_key(rt->db(), rt->name(), &mdl_key);
  else
    dd::Procedure::create_mdl_key(rt->db(), rt->name(), &mdl_key);

  /*
    Check that we have an MDL lock on this routine, unless it's a top-level
    CALL. The assert below should be unambiguous: the first element
    in sroutines_list has an MDL lock unless it's a top-level call, or a
    trigger, but triggers can't occur here (see the preceding assert).
  */
  DBUG_ASSERT(
      thd->mdl_context.owns_equal_or_stronger_lock(&mdl_key, MDL_SHARED) ||
      rt == thd->lex->sroutines_list.first);
#endif

  return sp_cache_routine(thd, type, &name, lookup_only, sp);
}

/**
  Ensure that routine is present in cache by loading it from the data
  dictionary if needed. If the routine is present but old, reload it.
  Emit an appropriate error if there was a problem during
  loading.

  @param[in]  thd   Thread context.
  @param[in]  type  Type of object (FUNCTION or PROCEDURE).
  @param[in]  name  Name of routine.
  @param[in]  lookup_only Only check that the routine is in the cache.
                    If it's not, don't try to load. If it is present,
                    but old, don't try to reload.
  @param[out] sp    Pointer to sp_head object for routine, NULL if routine was
                    not found.

  @retval SP_OK      Either routine is found and was successfully loaded into
                     cache or it does not exist.
  @retval non-SP_OK  Error while loading routine from DD table.
*/

enum_sp_return_code sp_cache_routine(THD *thd, enum_sp_type type, sp_name *name,
                                     bool lookup_only, sp_head **sp) {
  enum_sp_return_code ret = SP_OK;
  sp_cache **spc = (type == enum_sp_type::FUNCTION) ? &thd->sp_func_cache
                                                    : &thd->sp_proc_cache;

  DBUG_TRACE;

  DBUG_ASSERT(type == enum_sp_type::FUNCTION ||
              type == enum_sp_type::PROCEDURE);

  *sp = sp_cache_lookup(spc, name);

  if (lookup_only) return SP_OK;

  if (*sp) {
    sp_cache_flush_obsolete(spc, sp);
    if (*sp) return SP_OK;
  }

  switch ((ret = db_find_routine(thd, type, name, sp))) {
    case SP_OK:
      sp_cache_insert(spc, *sp);
      break;
    case SP_DOES_NOT_EXISTS:
      ret = SP_OK;
      break;
    default:
      /* Query might have been killed, don't set error. */
      if (thd->killed) break;
      /*
        Any error when loading an existing routine is either some problem
        with the DD table, or a parse error because the contents
        has been tampered with (in which case we clear that error).
      */
      if (ret == SP_PARSE_ERROR) thd->clear_error();
      /*
        If we cleared the parse error, or when db_find_routine() flagged
        an error with it's return value without calling my_error(), we
        set the generic "mysql.proc table corrupt" error here.
      */
      if (!thd->is_error()) {
        /*
          SP allows full NAME_LEN chars thus he have to allocate enough
          size in bytes. Otherwise there is stack overrun could happen
          if multibyte sequence is `name`. `db` is still safe because the
          rest of the server checks agains NAME_LEN bytes and not chars.
          Hence, the overrun happens only if the name is in length > 32 and
          uses multibyte (cyrillic, greek, etc.)
        */
        char n[NAME_LEN * 2 + 2];

        /* m_qname.str is not always \0 terminated */
        memcpy(n, name->m_qname.str, name->m_qname.length);
        n[name->m_qname.length] = '\0';
        my_error(ER_SP_LOAD_FAILED, MYF(0), n);
      }
      break;
  }
  return ret;
}

/**
  Generates the CREATE... string from the table information.

  @return
    Returns true on success, false on (alloc) failure.
*/
static bool create_string(
    THD *thd, String *buf, enum_sp_type type, const char *db, size_t dblen,
    const char *name, size_t namelen, const char *params, size_t paramslen,
    const char *returns, size_t returnslen, const char *body, size_t bodylen,
    st_sp_chistics *chistics, const LEX_CSTRING &definer_user,
    const LEX_CSTRING &definer_host, sql_mode_t sql_mode) {
  sql_mode_t old_sql_mode = thd->variables.sql_mode;
  /* Make some room to begin with */
  if (buf->alloc(100 + dblen + 1 + namelen + paramslen + returnslen + bodylen +
                 chistics->comment.length + 10 /* length of " DEFINER= "*/ +
                 USER_HOST_BUFF_SIZE))
    return false;

  thd->variables.sql_mode = sql_mode;
  buf->append(STRING_WITH_LEN("CREATE "));
  append_definer(thd, buf, definer_user, definer_host);
  if (type == enum_sp_type::FUNCTION)
    buf->append(STRING_WITH_LEN("FUNCTION "));
  else
    buf->append(STRING_WITH_LEN("PROCEDURE "));
  if (dblen > 0) {
    append_identifier(thd, buf, db, dblen);
    buf->append('.');
  }
  append_identifier(thd, buf, name, namelen);
  buf->append('(');
  buf->append(params, paramslen, system_charset_info);
  buf->append(')');
  if (type == enum_sp_type::FUNCTION) {
    buf->append(STRING_WITH_LEN(" RETURNS "));
    buf->append(returns, returnslen);
  }
  buf->append('\n');
  switch (chistics->daccess) {
    case SP_NO_SQL:
      buf->append(STRING_WITH_LEN("    NO SQL\n"));
      break;
    case SP_READS_SQL_DATA:
      buf->append(STRING_WITH_LEN("    READS SQL DATA\n"));
      break;
    case SP_MODIFIES_SQL_DATA:
      buf->append(STRING_WITH_LEN("    MODIFIES SQL DATA\n"));
      break;
    case SP_DEFAULT_ACCESS:
    case SP_CONTAINS_SQL:
      /* Do nothing */
      break;
  }
  if (chistics->detistic) buf->append(STRING_WITH_LEN("    DETERMINISTIC\n"));
  if (chistics->suid == SP_IS_NOT_SUID)
    buf->append(STRING_WITH_LEN("    SQL SECURITY INVOKER\n"));
  if (chistics->comment.length) {
    buf->append(STRING_WITH_LEN("    COMMENT "));
    append_unescaped(buf, chistics->comment.str, chistics->comment.length);
    buf->append('\n');
  }
  buf->append(body, bodylen);
  thd->variables.sql_mode = old_sql_mode;
  return true;
}

/**
  The function loads sp_head struct for information schema purposes
  (used for I_S ROUTINES & PARAMETERS tables).

  @param[in]      thd               thread handler
  @param[in]      db_name           DB name.
  @param[in]      routine           dd::Routine object.
  @param[out]     free_sp_head      returns 1 if we need to free sp_head struct
                                    otherwise returns 0

  @return     Pointer on sp_head struct
    @retval   NULL                  error
*/

sp_head *sp_load_for_information_schema(THD *thd, LEX_CSTRING db_name,
                                        const dd::Routine *routine,
                                        bool *free_sp_head) {
  sp_head *sp;
  enum_sp_type type = is_dd_routine_type_function(routine)
                          ? enum_sp_type::FUNCTION
                          : enum_sp_type::PROCEDURE;
  *free_sp_head = false;
  sp_cache **spc = (type == enum_sp_type::FUNCTION) ? &thd->sp_func_cache
                                                    : &thd->sp_proc_cache;
  sp_name sp_name_obj(
      db_name,
      {const_cast<char *>(routine->name().c_str()), routine->name().length()},
      true);
  sp_name_obj.init_qname(thd);

  if ((sp = sp_cache_lookup(spc, &sp_name_obj))) {
    return sp;
  }

  // Create stored program creation context from routine object.
  Stored_program_creation_ctx *creation_ctx =
      Stored_routine_creation_ctx::create_routine_creation_ctx(routine);
  if (creation_ctx == nullptr) return nullptr;

  // Prepare stored routine return type string.
  dd::String_type return_type_str;
  prepare_return_type_string_from_dd_routine(thd, routine, &return_type_str);

  // Prepare stored routine parameter's string.
  dd::String_type params_str;
  prepare_params_string_from_dd_routine(thd, routine, &params_str);

  // Dummy Routine body.
  LEX_CSTRING sr_body;
  if (type == enum_sp_type::FUNCTION)
    sr_body = {STRING_WITH_LEN("RETURN NULL")};
  else
    sr_body = {STRING_WITH_LEN("BEGIN END")};

  // Dummy stored routine definer.
  const LEX_CSTRING definer_user = EMPTY_CSTR;
  const LEX_CSTRING definer_host = EMPTY_CSTR;

  // Dummy st_sp_chistics object.
  struct st_sp_chistics sp_chistics;
  memset(&sp_chistics, 0, sizeof(st_sp_chistics));

  String defstr;
  defstr.set_charset(creation_ctx->get_client_cs());
  if (!create_string(thd, &defstr, type, db_name.str, db_name.length,
                     routine->name().c_str(), routine->name().length(),
                     params_str.c_str(), params_str.length(),
                     return_type_str.c_str(), return_type_str.length(),
                     sr_body.str, sr_body.length, &sp_chistics, definer_user,
                     definer_host, routine->sql_mode()))
    return nullptr;

  LEX *old_lex = thd->lex, newlex;
  thd->lex = &newlex;
  newlex.thd = thd;
  newlex.set_current_select(nullptr);
  sp = sp_compile(thd, &defstr, routine->sql_mode(), creation_ctx);
  *free_sp_head = true;
  thd->lex->sphead = nullptr;
  lex_end(thd->lex);
  thd->lex = old_lex;
  return sp;
}

/**
  Start parsing of a stored program.

  This function encapsulates all the steps necessary to initialize sp_head to
  start parsing SP.

  Every successful call of sp_start_parsing() must finish with
  sp_finish_parsing().

  @param thd      Thread context.
  @param sp_type  The stored program type
  @param sp_name  The stored progam name

  @return properly initialized sp_head-instance in case of success, or NULL is
  case of out-of-memory error.
*/
sp_head *sp_start_parsing(THD *thd, enum_sp_type sp_type, sp_name *sp_name) {
  // The order is important:
  // 1. new sp_head()
  MEM_ROOT own_root;

  init_sql_alloc(key_memory_sp_head_main_root, &own_root, MEM_ROOT_BLOCK_SIZE,
                 MEM_ROOT_PREALLOC);

  void *rawmem = own_root.Alloc(sizeof(sp_head));
  if (!rawmem) return nullptr;

  sp_head *sp = new (rawmem) sp_head(std::move(own_root), sp_type);

  // 2. start_parsing_sp_body()

  sp->m_parser_data.start_parsing_sp_body(thd, sp);

  // 3. finish initialization.

  sp->m_root_parsing_ctx = new (thd->mem_root) sp_pcontext(thd);

  if (!sp->m_root_parsing_ctx) return nullptr;

  thd->lex->set_sp_current_parsing_ctx(sp->m_root_parsing_ctx);

  // 4. set name.

  sp->init_sp_name(thd, sp_name);

  return sp;
}

/**
  Finish parsing of a stored program.

  This is a counterpart of sp_start_parsing().

  @param thd  Thread context.
*/
void sp_finish_parsing(THD *thd) {
  sp_head *sp = thd->lex->sphead;

  DBUG_ASSERT(sp);

  sp->set_body_end(thd);

  sp->m_parser_data.finish_parsing_sp_body(thd);
}

/// @return Item_result code corresponding to the RETURN-field type code.
Item_result sp_map_result_type(enum enum_field_types type) {
  switch (type) {
    case MYSQL_TYPE_BIT:
    case MYSQL_TYPE_TINY:
    case MYSQL_TYPE_SHORT:
    case MYSQL_TYPE_LONG:
    case MYSQL_TYPE_LONGLONG:
    case MYSQL_TYPE_INT24:
      return INT_RESULT;
    case MYSQL_TYPE_DECIMAL:
    case MYSQL_TYPE_NEWDECIMAL:
      return DECIMAL_RESULT;
    case MYSQL_TYPE_FLOAT:
    case MYSQL_TYPE_DOUBLE:
      return REAL_RESULT;
    default:
      return STRING_RESULT;
  }
}

/// @return Item::Type code corresponding to the RETURN-field type code.
Item::Type sp_map_item_type(enum enum_field_types type) {
  switch (type) {
    case MYSQL_TYPE_BIT:
    case MYSQL_TYPE_TINY:
    case MYSQL_TYPE_SHORT:
    case MYSQL_TYPE_LONG:
    case MYSQL_TYPE_LONGLONG:
    case MYSQL_TYPE_INT24:
      return Item::INT_ITEM;
    case MYSQL_TYPE_DECIMAL:
    case MYSQL_TYPE_NEWDECIMAL:
      return Item::DECIMAL_ITEM;
    case MYSQL_TYPE_FLOAT:
    case MYSQL_TYPE_DOUBLE:
      return Item::REAL_ITEM;
    default:
      return Item::STRING_ITEM;
  }
}

/**
  @param lex LEX-object, representing an SQL-statement inside SP.

  @return a combination of:
    - sp_head::MULTI_RESULTS: added if the 'cmd' is a command that might
      result in multiple result sets being sent back.
    - sp_head::CONTAINS_DYNAMIC_SQL: added if 'cmd' is one of PREPARE,
      EXECUTE, DEALLOCATE.
*/
uint sp_get_flags_for_command(LEX *lex) {
  uint flags;

  switch (lex->sql_command) {
    case SQLCOM_SELECT:
      if (lex->result) {
        flags = 0; /* This is a SELECT with INTO clause */
        break;
      }
      /* fallthrough */
    case SQLCOM_ANALYZE:
    case SQLCOM_OPTIMIZE:
    case SQLCOM_PRELOAD_KEYS:
    case SQLCOM_ASSIGN_TO_KEYCACHE:
    case SQLCOM_CHECKSUM:
    case SQLCOM_CHECK:
    case SQLCOM_HA_READ:
    case SQLCOM_SHOW_BINLOGS:
    case SQLCOM_SHOW_BINLOG_EVENTS:
    case SQLCOM_SHOW_RELAYLOG_EVENTS:
    case SQLCOM_SHOW_CHARSETS:
    case SQLCOM_SHOW_COLLATIONS:
    case SQLCOM_SHOW_CREATE:
    case SQLCOM_SHOW_CREATE_DB:
    case SQLCOM_SHOW_CREATE_FUNC:
    case SQLCOM_SHOW_CREATE_PROC:
    case SQLCOM_SHOW_CREATE_EVENT:
    case SQLCOM_SHOW_CREATE_TRIGGER:
    case SQLCOM_SHOW_DATABASES:
    case SQLCOM_SHOW_ERRORS:
    case SQLCOM_SHOW_FIELDS:
    case SQLCOM_SHOW_FUNC_CODE:
    case SQLCOM_SHOW_GRANTS:
    case SQLCOM_SHOW_ENGINE_STATUS:
    case SQLCOM_SHOW_ENGINE_LOGS:
    case SQLCOM_SHOW_ENGINE_MUTEX:
    case SQLCOM_SHOW_ENGINE_TRX:
    case SQLCOM_SHOW_EVENTS:
    case SQLCOM_SHOW_KEYS:
    case SQLCOM_SHOW_MASTER_STAT:
    case SQLCOM_SHOW_MEMORY_STATUS:
    case SQLCOM_SHOW_OPEN_TABLES:
    case SQLCOM_SHOW_PRIVILEGES:
    case SQLCOM_SHOW_PROCESSLIST:
    case SQLCOM_SHOW_PROC_CODE:
    case SQLCOM_SHOW_SLAVE_HOSTS:
    case SQLCOM_SHOW_SLAVE_STAT:
    case SQLCOM_SHOW_STATUS:
    case SQLCOM_SHOW_STATUS_FUNC:
    case SQLCOM_SHOW_STATUS_PROC:
    case SQLCOM_SHOW_STORAGE_ENGINES:
    case SQLCOM_SHOW_TABLES:
    case SQLCOM_SHOW_TABLE_STATUS:
    case SQLCOM_SHOW_VARIABLES:
    case SQLCOM_SHOW_WARNS:
    case SQLCOM_REPAIR:
    case SQLCOM_FIND_GTID_POSITION:
      flags = sp_head::MULTI_RESULTS;
      break;
    /*
      EXECUTE statement may return a result set, but doesn't have to.
      We can't, however, know it in advance, and therefore must add
      this statement here. This is ok, as is equivalent to a result-set
      statement within an IF condition.
    */
    case SQLCOM_EXECUTE:
      flags = sp_head::MULTI_RESULTS | sp_head::CONTAINS_DYNAMIC_SQL;
      break;
    case SQLCOM_PREPARE:
    case SQLCOM_DEALLOCATE_PREPARE:
      flags = sp_head::CONTAINS_DYNAMIC_SQL;
      break;
    case SQLCOM_CREATE_TABLE:
      if (lex->create_info->options & HA_LEX_CREATE_TMP_TABLE)
        flags = 0;
      else
        flags = sp_head::HAS_COMMIT_OR_ROLLBACK;
      break;
    case SQLCOM_DROP_TABLE:
      if (lex->drop_temporary)
        flags = 0;
      else
        flags = sp_head::HAS_COMMIT_OR_ROLLBACK;
      break;
    case SQLCOM_FLUSH:
      flags = sp_head::HAS_SQLCOM_FLUSH;
      break;
    case SQLCOM_RESET:
      flags = sp_head::HAS_SQLCOM_RESET;
      break;
    case SQLCOM_CREATE_INDEX:
    case SQLCOM_CREATE_DB:
    case SQLCOM_CREATE_VIEW:
    case SQLCOM_CREATE_TRIGGER:
    case SQLCOM_CREATE_USER:
    case SQLCOM_ALTER_TABLE:
    case SQLCOM_GRANT:
    case SQLCOM_GRANT_ROLE:
    case SQLCOM_REVOKE:
    case SQLCOM_REVOKE_ROLE:
    case SQLCOM_BEGIN:
    case SQLCOM_RENAME_TABLE:
    case SQLCOM_RENAME_USER:
    case SQLCOM_DROP_INDEX:
    case SQLCOM_DROP_DB:
    case SQLCOM_REVOKE_ALL:
    case SQLCOM_DROP_USER:
    case SQLCOM_DROP_VIEW:
    case SQLCOM_DROP_TRIGGER:
    case SQLCOM_TRUNCATE:
    case SQLCOM_COMMIT:
    case SQLCOM_ROLLBACK:
    case SQLCOM_LOAD:
    case SQLCOM_LOCK_TABLES:
    case SQLCOM_CREATE_PROCEDURE:
    case SQLCOM_CREATE_SPFUNCTION:
    case SQLCOM_ALTER_PROCEDURE:
    case SQLCOM_ALTER_FUNCTION:
    case SQLCOM_DROP_PROCEDURE:
    case SQLCOM_DROP_FUNCTION:
    case SQLCOM_CREATE_EVENT:
    case SQLCOM_ALTER_EVENT:
    case SQLCOM_DROP_EVENT:
    case SQLCOM_INSTALL_PLUGIN:
    case SQLCOM_UNINSTALL_PLUGIN:
    case SQLCOM_ALTER_DB:
    case SQLCOM_ALTER_USER:
    case SQLCOM_CREATE_SERVER:
    case SQLCOM_ALTER_SERVER:
    case SQLCOM_DROP_SERVER:
    case SQLCOM_CHANGE_MASTER:
    case SQLCOM_CHANGE_REPLICATION_FILTER:
    case SQLCOM_SLAVE_START:
    case SQLCOM_SLAVE_STOP:
    case SQLCOM_ALTER_INSTANCE:
    case SQLCOM_CREATE_ROLE:
    case SQLCOM_DROP_ROLE:
    case SQLCOM_CREATE_SRS:
    case SQLCOM_DROP_SRS:
    case SQLCOM_CREATE_RESOURCE_GROUP:
    case SQLCOM_ALTER_RESOURCE_GROUP:
    case SQLCOM_DROP_RESOURCE_GROUP:
      flags = sp_head::HAS_COMMIT_OR_ROLLBACK;
      break;
    default:
      flags = lex->is_explain() ? sp_head::MULTI_RESULTS : 0;
      break;
  }
  return flags;
}

/**
  Check that the name 'ident' is ok.  It's assumed to be an 'ident'
  from the parser, so we only have to check length and trailing spaces.
  The former is a standard requirement (and 'show status' assumes a
  non-empty name), the latter is a mysql:ism as trailing spaces are
  removed by get_field().

  @retval true    bad name
  @retval false   name is ok
*/

bool sp_check_name(LEX_STRING *ident) {
  DBUG_ASSERT(ident != nullptr && ident->str != nullptr);

  if (!ident->str[0] || ident->str[ident->length - 1] == ' ') {
    my_error(ER_SP_WRONG_NAME, MYF(0), ident->str);
    return true;
  }

  LEX_CSTRING ident_cstr = {ident->str, ident->length};
  if (check_string_char_length(ident_cstr, "", NAME_CHAR_LEN,
                               system_charset_info, true)) {
    my_error(ER_TOO_LONG_IDENT, MYF(0), ident->str);
    return true;
  }

  return false;
}

/**
  Prepare an Item for evaluation (call of fix_fields).

  @param thd       thread handler
  @param it_addr   pointer on item reference

  @retval
    NULL      error
  @retval
    non-NULL  prepared item
*/
Item *sp_prepare_func_item(THD *thd, Item **it_addr) {
  it_addr = (*it_addr)->this_item_addr(thd, it_addr);

  if (!(*it_addr)->fixed &&
      ((*it_addr)->fix_fields(thd, it_addr) || (*it_addr)->check_cols(1))) {
    DBUG_PRINT("info", ("fix_fields() failed"));
    return nullptr;
  }

  thd->lex->set_exec_started();

  return *it_addr;
}

/**
  Evaluate an expression and store the result in the field.

  @param thd                    current thread object
  @param result_field           the field to store the result
  @param expr_item_ptr          the root item of the expression

  @retval
    false  on success
  @retval
    true   on error
*/
bool sp_eval_expr(THD *thd, Field *result_field, Item **expr_item_ptr) {
  Item *expr_item;
  Strict_error_handler strict_handler(
      Strict_error_handler::ENABLE_SET_SELECT_STRICT_ERROR_HANDLER);
  enum_check_fields save_check_for_truncated_fields =
      thd->check_for_truncated_fields;
  unsigned int stmt_unsafe_rollback_flags =
      thd->get_transaction()->get_unsafe_rollback_flags(Transaction_ctx::STMT);

  if (!*expr_item_ptr) goto error;

  if (!(expr_item = sp_prepare_func_item(thd, expr_item_ptr))) goto error;

  /*
    Set THD flags to emit warnings/errors in case of overflow/type errors
    during saving the item into the field.

    Save original values and restore them after save.
  */

  thd->check_for_truncated_fields = CHECK_FIELD_ERROR_FOR_NULL;
  thd->get_transaction()->reset_unsafe_rollback_flags(Transaction_ctx::STMT);

  /*
    Variables declared within SP/SF with DECLARE keyword like
      DECLARE var INTEGER;
    will follow the rules of assignment corresponding to the data type column
    in a table. So, STRICT mode gives error if an invalid value is assigned
    to the variable here.
  */
  if (thd->install_strict_handler() && !thd->lex->is_ignore())
    thd->push_internal_handler(&strict_handler);
  // Save the value in the field. Convert the value if needed.
  expr_item->save_in_field(result_field, false);

  if (thd->install_strict_handler() && !thd->lex->is_ignore())
    thd->pop_internal_handler();
  thd->check_for_truncated_fields = save_check_for_truncated_fields;
  thd->get_transaction()->set_unsafe_rollback_flags(Transaction_ctx::STMT,
                                                    stmt_unsafe_rollback_flags);

  if (!thd->is_error()) return false;

error:
  /*
    In case of error during evaluation, leave the result field set to NULL.
    Sic: we can't do it in the beginning of the function because the
    result field might be needed for its own re-evaluation, e.g. case of
    set x = x + 1;
  */
  result_field->set_null();
  return true;
}

/**
  Return a string representation of the Item value.

  @param thd  Thread context.
  @param item The item to evaluate
  @param str  String buffer for representation of the value.

  @note
    If the item has a string result type, the string is escaped
    according to its character set.

  @retval NULL      on error
  @retval non-NULL  a pointer to valid a valid string on success
*/
String *sp_get_item_value(THD *thd, Item *item, String *str) {
  switch (item->result_type()) {
    case REAL_RESULT:
    case INT_RESULT:
    case DECIMAL_RESULT:
      if (item->data_type() != MYSQL_TYPE_BIT)
        return item->val_str(str);
      else { /* Bit type is handled as binary string */
      }
      // Fall through
    case STRING_RESULT: {
      String *result = item->val_str(str);

      if (!result) return nullptr;

      {
        char buf_holder[STRING_BUFFER_USUAL_SIZE];
        String buf(buf_holder, sizeof(buf_holder), result->charset());
        const CHARSET_INFO *cs = thd->variables.character_set_client;

        /* We must reset length of the buffer, because of String specificity. */
        buf.length(0);

        buf.append('_');
        buf.append(result->charset()->csname);
        if (cs->escape_with_backslash_is_dangerous) buf.append(' ');
        append_query_string(thd, cs, result, &buf);
        buf.append(" COLLATE '");
        buf.append(item->collation.collation->name);
        buf.append('\'');
        str->copy(buf);

        return str;
      }
    }

    case ROW_RESULT:
    default:
      return nullptr;
  }
}
