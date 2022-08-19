/*
   Copyright (c) 2004, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/sql_trigger.h"

#include <stddef.h>
#include <string.h>
#include <string>
#include <utility>

#include "m_ctype.h"
#include "m_string.h"
#include "my_base.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_psi_config.h"
#include "my_sys.h"
#include "mysql/psi/mysql_sp.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "sql/auth/auth_acls.h"
#include "sql/auth/auth_common.h"  // check_table_access
#include "sql/auth/sql_security_ctx.h"
#include "sql/binlog.h"
#include "sql/dd/cache/dictionary_client.h"
#include "sql/dd/dd_schema.h"
#include "sql/dd/string_type.h"
#include "sql/dd/types/abstract_table.h"  // dd::enum_table_type
#include "sql/dd/types/table.h"
#include "sql/dd/types/trigger.h"
#include "sql/debug_sync.h"       // DEBUG_SYNC
#include "sql/derror.h"           // ER_THD
#include "sql/mysqld.h"           // trust_function_creators
#include "sql/sp_cache.h"         // sp_invalidate_cache()
#include "sql/sp_head.h"          // sp_name
#include "sql/sql_backup_lock.h"  // acquire_shared_backup_lock
#include "sql/sql_base.h"         // find_temporary_table()
#include "sql/sql_class.h"
#include "sql/sql_error.h"
#include "sql/sql_handler.h"  // mysql_ha_rm_tables()
#include "sql/sql_lex.h"
#include "sql/sql_table.h"  // build_table_filename()
#include "sql/system_variables.h"
#include "sql/table.h"
#include "sql/table_trigger_dispatcher.h"  // Table_trigger_dispatcher
#include "sql/transaction.h"               // trans_commit_stmt, trans_commit
#include "sql_string.h"
#include "thr_lock.h"

namespace dd {
class Schema;
}  // namespace dd
///////////////////////////////////////////////////////////////////////////

bool get_table_for_trigger(THD *thd, const LEX_CSTRING &db_name,
                           const LEX_STRING &trigger_name,
                           bool continue_if_not_exist, TABLE_LIST **table) {
  DBUG_TRACE;
  LEX *lex = thd->lex;
  *table = nullptr;

  // We must lock the schema when this function is called directly from
  // mysql_execute_command.
  dd::Schema_MDL_locker mdl_locker(thd);
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());

  const dd::Schema *sch_obj = nullptr;
  if (mdl_locker.ensure_locked(db_name.str) ||
      thd->dd_client()->acquire(db_name.str, &sch_obj))
    return true;

  if (sch_obj == nullptr) {
    if (continue_if_not_exist) {
      push_warning(thd, Sql_condition::SL_NOTE, ER_BAD_DB_ERROR,
                   ER_THD(thd, ER_BAD_DB_ERROR));
      return false;
    }

    my_error(ER_BAD_DB_ERROR, MYF(0), db_name.str);
    return true;
  }

  dd::String_type table_name;
  if (thd->dd_client()->get_table_name_by_trigger_name(
          *sch_obj, trigger_name.str, &table_name))
    return true;

  if (table_name == "") {
    if (continue_if_not_exist) {
      push_warning(thd, Sql_condition::SL_NOTE, ER_TRG_DOES_NOT_EXIST,
                   ER_THD(thd, ER_TRG_DOES_NOT_EXIST));
      return false;
    }

    my_error(ER_TRG_DOES_NOT_EXIST, MYF(0));
    return true;
  }

  char lc_table_name[NAME_LEN + 1];
  const char *table_name_ptr = table_name.c_str();
  if (lower_case_table_names == 2) {
    my_stpncpy(lc_table_name, table_name.c_str(), NAME_LEN);
    my_casedn_str(files_charset_info, lc_table_name);
    lc_table_name[NAME_LEN] = '\0';
    table_name_ptr = lc_table_name;
  }

  size_t table_name_length = strlen(table_name_ptr);

  *table = new (thd->mem_root) TABLE_LIST(
      thd->strmake(db_name.str, db_name.length), db_name.length,
      thd->strmake(table_name_ptr, table_name_length), table_name_length,
      thd->mem_strdup(table_name_ptr), TL_IGNORE, MDL_SHARED_NO_WRITE);

  if (*table == nullptr) return true;

  (*table)->select_lex = lex->current_select();
  (*table)->cacheable_table = true;

  return false;
}

#ifdef HAVE_PSI_SP_INTERFACE
void remove_all_triggers_from_perfschema(const char *schema_name,
                                         const dd::Table &table) {
  for (const dd::Trigger *trigger : table.triggers()) {
    MYSQL_DROP_SP(to_uint(enum_sp_type::TRIGGER), schema_name,
                  strlen(schema_name), trigger->name().c_str(),
                  trigger->name().length());
  }
}
#endif

bool check_table_triggers_are_not_in_the_same_schema(const char *db_name,
                                                     const dd::Table &table,
                                                     const char *new_db_name) {
  /*
    Since triggers should be in the same schema as their subject tables
    moving table with them between two schemas raises too many questions.
    (E.g. what should happen if in new schema we already have trigger
     with same name ?).
  */
  if (table.has_trigger() &&
      my_strcasecmp(table_alias_charset, db_name, new_db_name)) {
    my_error(ER_TRG_IN_WRONG_SCHEMA, MYF(0));
    return true;
  }

  return false;
}

bool acquire_mdl_for_trigger(THD *thd, const char *db, const char *trg_name,
                             enum_mdl_type trigger_name_mdl_type) {
  DBUG_ASSERT(trg_name != nullptr);
  DBUG_ASSERT(trigger_name_mdl_type == MDL_EXCLUSIVE ||
              trigger_name_mdl_type == MDL_SHARED_HIGH_PRIO);

  if (thd->global_read_lock.can_acquire_protection()) return true;

  MDL_key mdl_key;
  dd::Trigger::create_mdl_key(dd::String_type(db), dd::String_type(trg_name),
                              &mdl_key);

  MDL_request mdl_request;
  MDL_REQUEST_INIT_BY_KEY(&mdl_request, &mdl_key, trigger_name_mdl_type,
                          MDL_TRANSACTION);
  /*
    It isn't required to create MDL request for MDL_key::GLOBAL,
    MDL_key::SCHEMA since it was already done before while
    calling the method open_and_lock_subj_table().
  */
  if (thd->mdl_context.acquire_lock_nsec(&mdl_request,
                                         thd->variables.lock_wait_timeout_nsec))
    return true;

  DEBUG_SYNC(thd, "after_acquiring_mdl_lock_on_trigger");

  return false;
}

/**
  Check that the user has TRIGGER privilege on the subject table.

  @param thd  current thread context
  @param table  table to check

  @return Operation status.
    @retval false Success
    @retval true  Failure
*/

bool Sql_cmd_ddl_trigger_common::check_trg_priv_on_subj_table(
    THD *thd, TABLE_LIST *table) const {
  TABLE_LIST **save_query_tables_own_last = thd->lex->query_tables_own_last;
  thd->lex->query_tables_own_last = nullptr;

  bool err_status =
      check_table_access(thd, TRIGGER_ACL, table, false, 1, false);

  thd->lex->query_tables_own_last = save_query_tables_own_last;

  return err_status;
}

/**
  Open and lock a table associated with a trigger.

  @param[in] thd  current thread context
  @param[in] tables  trigger's table
  @param[out] mdl_ticket  granted metadata lock

  @return Opened TABLE object on success, nullptr on failure.
*/

TABLE *Sql_cmd_ddl_trigger_common::open_and_lock_subj_table(
    THD *thd, TABLE_LIST *tables, MDL_ticket **mdl_ticket) const {
  /* We should have only one table in table list. */
  DBUG_ASSERT(tables->next_global == nullptr);

  /* We also don't allow creation of triggers on views. */
  tables->required_type = dd::enum_table_type::BASE_TABLE;
  /*
    Also prevent DROP TRIGGER from opening temporary table which might
    shadow the subject table on which trigger to be dropped is defined.
  */
  tables->open_type = OT_BASE_ONLY;

  /* Keep consistent with respect to other DDL statements */
  mysql_ha_rm_tables(thd, tables);

  if (thd->locked_tables_mode) {
    /* Under LOCK TABLES we must only accept write locked tables. */
    tables->table =
        find_table_for_mdl_upgrade(thd, tables->db, tables->table_name, false);
    if (tables->table == nullptr) return nullptr;

    if (acquire_shared_backup_lock_nsec(thd,
                                        thd->variables.lock_wait_timeout_nsec))
      return nullptr;
  } else {
    tables->table = open_n_lock_single_table(thd, tables, TL_READ_NO_INSERT, 0);
    if (tables->table == nullptr) return nullptr;
    tables->table->use_all_columns();
  }

  TABLE *table = tables->table;
  table->set_pos_in_table_list(tables);

  /* Later on we will need it to downgrade the lock */
  *mdl_ticket = table->mdl_ticket;

  if (wait_while_table_is_used(thd, table, HA_EXTRA_FORCE_REOPEN))
    return nullptr;

  return table;
}

/**
  Close all open instances of a trigger's table, reopen it if needed,
  invalidate SP-cache and possibly write a statement to binlog.

  @param[in] thd         Current thread context
  @param[in] db_name     Database name where trigger's table defined
  @param[in] table       Table associated with a trigger
  @param[in] stmt_query  Query string to write to binlog
  @param[in] binlog_stmt Should the statement be binlogged?

  @return Operation status.
    @retval false Success
    @retval true  Failure
*/

static bool finalize_trigger_ddl(THD *thd, const char *db_name, TABLE *table,
                                 const String &stmt_query, bool binlog_stmt) {
  close_all_tables_for_name(thd, table->s, false, nullptr);
  /*
    Reopen the table if we were under LOCK TABLES.
    Ignore the return value for now. It's better to
    keep master/slave in consistent state.
  */
  thd->locked_tables_list.reopen_tables(thd);
  /*
    Invalidate SP-cache. That's needed because triggers may change list of
    pre-locking tables.
  */
  sp_cache_invalidate();

  if (!binlog_stmt) return false;

  thd->add_to_binlog_accessed_dbs(db_name);

  DEBUG_SYNC(thd, "trigger_ddl_stmt_before_write_to_binlog");
  return write_bin_log(thd, true, stmt_query.ptr(), stmt_query.length(), true);
}

void Sql_cmd_ddl_trigger_common::restore_original_mdl_state(
    THD *thd, MDL_ticket *mdl_ticket) const {
  /*
    If we are under LOCK TABLES we should restore original state of
    meta-data locks. Otherwise all locks will be released along
    with the implicit commit.
  */
  if (thd->locked_tables_mode)
    mdl_ticket->downgrade_lock(MDL_SHARED_NO_READ_WRITE);
}

/**
  Execute CREATE TRIGGER statement.

  @param thd     current thread context (including trigger definition in LEX)

  @note
    This method is mainly responsible for opening and locking of table and
    invalidation of all its instances in table cache after trigger creation.
    Real work on trigger creation is done inside Table_trigger_dispatcher
    methods.

  @todo
    TODO: We should check if user has TRIGGER privilege for table here.
    Now we just require SUPER privilege for creating/dropping because
    we don't have proper privilege checking for triggers in place yet.

  @retval false on success.
  @retval true on error
*/

bool Sql_cmd_create_trigger::execute(THD *thd) {
  DBUG_TRACE;

  if (!thd->lex->spname->m_db.length || !m_trigger_table->db_length) {
    my_error(ER_NO_DB_ERROR, MYF(0));
    return true;
  }

  // Normally, the schema is locked in open_and_lock...() below,
  // but not when in LOCK TABLE mode.
  dd::Schema_MDL_locker schema_mdl_locker(thd);

  if (thd->locked_tables_mode &&
      schema_mdl_locker.ensure_locked(m_trigger_table->db))
    return true;

  // This auto releaser will own the DD objects that we commit
  // at the bottom of this function.
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());

  /*
    We don't allow creating triggers on tables in the 'mysql' schema
  */
  if (!my_strcasecmp(system_charset_info, "mysql", m_trigger_table->db)) {
    my_error(ER_NO_TRIGGERS_ON_SYSTEM_SCHEMA, MYF(0));
    return true;
  }

  /*
    There is no DETERMINISTIC clause for triggers, so can't check it.
    But a trigger can in theory be used to do nasty things (if it supported
    DROP for example) so we do the check for privileges. For now there is
    already a stronger test right above; but when this stronger test will
    be removed, the test below will hold. Because triggers have the same
    nature as functions regarding binlogging: their body is implicitly
    binlogged, so they share the same danger, so trust_function_creators
    applies to them too.
  */
  Security_context *sctx = thd->security_context();
  if (!trust_function_creators && mysql_bin_log.is_open() &&
      !(sctx->check_access(SUPER_ACL) ||
        sctx->has_global_grant(STRING_WITH_LEN("SET_USER_ID")).first)) {
    my_error(ER_BINLOG_CREATE_ROUTINE_NEED_SUPER, MYF(0));
    return true;
  }

  if (check_trg_priv_on_subj_table(thd, m_trigger_table)) return true;

  /* We do not allow creation of triggers on temporary tables. */
  if (find_temporary_table(thd, m_trigger_table) != nullptr) {
    my_error(ER_TRG_ON_VIEW_OR_TEMP_TABLE, MYF(0), m_trigger_table->alias);
    return true;
  }

  MDL_ticket *mdl_ticket = nullptr;
  TABLE *table = open_and_lock_subj_table(thd, m_trigger_table, &mdl_ticket);
  if (table == nullptr) return true;

  if (acquire_exclusive_mdl_for_trigger(thd, thd->lex->spname->m_db.str,
                                        thd->lex->spname->m_name.str)) {
    restore_original_mdl_state(thd, mdl_ticket);
    return true;
  }

  DEBUG_SYNC(thd, "create_trigger_has_acquired_mdl");

  if (table->triggers == nullptr &&
      (table->triggers = Table_trigger_dispatcher::create(table)) == nullptr) {
    restore_original_mdl_state(thd, mdl_ticket);
    return true;
  }

  /* Charset of the buffer for statement must be system one. */
  String stmt_query;
  stmt_query.set_charset(system_charset_info);

  bool result = table->triggers->create_trigger(thd, &stmt_query);

  result |= finalize_trigger_ddl(thd, m_trigger_table->db, table, stmt_query,
                                 !result);

  DBUG_EXECUTE_IF("simulate_create_trigger_failure", {
    result = true;
    my_error(ER_UNKNOWN_ERROR, MYF(0));
  });

  if (result) {
    trans_rollback_stmt(thd);
    // Full rollback in case we have THD::transaction_rollback_request.
    trans_rollback(thd);
  } else {
    if (!(result = trans_commit_stmt(thd)) && !(result = trans_commit(thd))) {
      my_ok(thd);
    }
  }

  restore_original_mdl_state(thd, mdl_ticket);

  return result;
}

/**
  Execute DROP TRIGGER statement.

  @param thd     current thread context

  @todo
    TODO: We should check if user has TRIGGER privilege for table here.
    Now we just require SUPER privilege for creating/dropping because
    we don't have proper privilege checking for triggers in place yet.

  @retval
    false Success
  @retval
    true  error
*/

bool Sql_cmd_drop_trigger::execute(THD *thd) {
  DBUG_TRACE;

  if (!thd->lex->spname->m_db.length) {
    my_error(ER_NO_DB_ERROR, MYF(0));
    return true;
  }

  if (check_readonly(thd, true)) return true;

  // This auto releaser will own the DD objects that we commit
  // at the bottom of this function.
  dd::Schema_MDL_locker schema_mdl_locker(thd);
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());

  if (schema_mdl_locker.ensure_locked(thd->lex->spname->m_db.str)) return true;

  if (acquire_exclusive_mdl_for_trigger(thd, thd->lex->spname->m_db.str,
                                        thd->lex->spname->m_name.str))
    return true;

  /* Charset of the buffer for statement must be system one. */
  String stmt_query;
  stmt_query.set_charset(system_charset_info);

  TABLE_LIST *tables = nullptr;
  if (get_table_for_trigger(thd, thd->lex->spname->m_db,
                            thd->lex->spname->m_name, thd->lex->drop_if_exists,
                            &tables))
    return true;

  if (tables == nullptr) {
    DBUG_ASSERT(thd->lex->drop_if_exists == true);
    /*
      Since the trigger does not exist, there is no associated table,
      and therefore :
      - no TRIGGER privileges to check,
      - no trigger to drop,
      - no table to lock/modify,
      so the drop statement is successful.
    */

    /* Still, we need to log the query ... */
    stmt_query.append(thd->query().str, thd->query().length);
    bool result =
        write_bin_log(thd, true, stmt_query.ptr(), stmt_query.length(), false);

    if (!result) my_ok(thd);
    return result;
  }

  if (check_trg_priv_on_subj_table(thd, tables)) return true;

  MDL_ticket *mdl_ticket = nullptr;
  TABLE *table = open_and_lock_subj_table(thd, tables, &mdl_ticket);
  if (table == nullptr) return true;

  DEBUG_SYNC(thd, "drop_trigger_has_acquired_mdl");

  dd::Table *dd_table = nullptr;
  if (thd->dd_client()->acquire_for_modification(tables->db, tables->table_name,
                                                 &dd_table)) {
    // Error is reported by the dictionary subsystem.
    restore_original_mdl_state(thd, mdl_ticket);
    return true;
  }
  DBUG_ASSERT(dd_table != nullptr);

  const dd::Trigger *dd_trig_obj =
      dd_table->get_trigger(thd->lex->spname->m_name.str);
  if (dd_trig_obj == nullptr) {
    my_error(ER_TRG_DOES_NOT_EXIST, MYF(0));
    restore_original_mdl_state(thd, mdl_ticket);
    return true;
  }

  dd_table->drop_trigger(dd_trig_obj);
  bool result = thd->dd_client()->update(dd_table);

  if (!result)
    result = stmt_query.append(thd->query().str, thd->query().length);

  result |= finalize_trigger_ddl(thd, tables->db, table, stmt_query, !result);

  DBUG_EXECUTE_IF("simulate_drop_trigger_failure", {
    result = true;
    my_error(ER_UNKNOWN_ERROR, MYF(0));
  });

  if (result) {
    trans_rollback_stmt(thd);
    trans_rollback(thd);
  } else {
    if (!(result = trans_commit_stmt(thd)) && !(result = trans_commit(thd))) {
#ifdef HAVE_PSI_SP_INTERFACE
      /* Drop statistics for this stored program from performance schema. */
      MYSQL_DROP_SP(to_uint(enum_sp_type::TRIGGER), thd->lex->spname->m_db.str,
                    thd->lex->spname->m_db.length, thd->lex->spname->m_name.str,
                    thd->lex->spname->m_name.length);
#endif
      my_ok(thd);
    }
  }

  restore_original_mdl_state(thd, mdl_ticket);

  return result;
}
