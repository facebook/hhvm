/* Copyright (c) 2010, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/sql_truncate.h"

#include <stddef.h>
#include <sys/types.h>

#include "lex_string.h"
#include "m_ctype.h"
#include "my_base.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_sys.h"
#include "mysql/service_mysql_alloc.h"
#include "mysqld.h"  // table_alias_charset
#include "mysqld_error.h"
#include "scope_guard.h"  // create_scope_guard
#include "sql/auth/auth_acls.h"
#include "sql/auth/auth_common.h"            // DROP_ACL
#include "sql/dd/cache/dictionary_client.h"  // dd::cache::Dictionary_client
#include "sql/dd/dd_schema.h"                // dd::Schema_MDL_locker
#include "sql/dd/dd_table.h"                 // dd::table_storage_engine
#include "sql/dd/properties.h"               // dd::Properties
#include "sql/dd/types/abstract_table.h"     // dd::enum_table_type
#include "sql/dd/types/table.h"              // dd::Table
#include "sql/debug_sync.h"                  // DEBUG_SYNC
#include "sql/handler.h"
#include "sql/lock.h"  // MYSQL_OPEN_* flags
#include "sql/mdl.h"
#include "sql/query_options.h"
#include "sql/sql_audit.h"        // mysql_audit_table_access_notify
#include "sql/sql_backup_lock.h"  // acquire_shared_backup_lock
#include "sql/sql_base.h"         // open_and_lock_tables
#include "sql/sql_class.h"        // THD
#include "sql/sql_const.h"
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/sql_show.h"   // append_identifier()
#include "sql/sql_table.h"  // write_bin_log
#include "sql/system_variables.h"
#include "sql/table.h"        // TABLE
#include "sql/transaction.h"  // trans_commit_stmt()
#include "sql/transaction_info.h"
#include "sql_string.h"
#include "thr_lock.h"

namespace dd {
class Table;
}  // namespace dd

/**
  Generate a foreign key description suitable for a error message.

  @param thd    Thread context.
  @param fk_p   Object describing foreign key in parent table.

  @return A human-readable string describing the foreign key.
*/

static const char *fk_info_str(const THD *thd,
                               const dd::Foreign_key_parent *fk_p) {
  bool res = false;
  char buffer[STRING_BUFFER_USUAL_SIZE * 2];
  String str(buffer, sizeof(buffer), system_charset_info);

  str.length(0);

  /*
    `db`.`tbl`, CONSTRAINT `id`
  */

  append_identifier(&str, fk_p->child_schema_name().c_str(),
                    fk_p->child_schema_name().length());
  res |= str.append(".");
  append_identifier(&str, fk_p->child_table_name().c_str(),
                    fk_p->child_table_name().length());
  res |= str.append(", CONSTRAINT ");
  append_identifier(&str, fk_p->fk_name().c_str(), fk_p->fk_name().length());

  return res ? nullptr : thd->strmake(str.ptr(), str.length());
}

/**
  Check and emit a fatal error if the table which is going to be
  affected by TRUNCATE TABLE is a parent table in some non-self-
  referencing foreign key.

  @remark The intention is to allow truncate only for tables that
          are not dependent on other tables.

  @param  thd         Thread context.
  @param  table_list  Table list element for the table.
  @param  table_def   dd::Table describing the table.

  @retval false  This table is not parent in a non-self-referencing foreign
                 key. Statement can proceed.
  @retval true   This table is parent in a non-self-referencing foreign key,
                 error was emitted.
*/

static bool fk_truncate_illegal_if_parent(THD *thd, TABLE_LIST *table_list,
                                          dd::Table *table_def) {
  for (const dd::Foreign_key_parent *fk_p : table_def->foreign_key_parents()) {
    if (my_strcasecmp(table_alias_charset, fk_p->child_schema_name().c_str(),
                      table_list->db) == 0 &&
        my_strcasecmp(table_alias_charset, fk_p->child_table_name().c_str(),
                      table_list->table_name) == 0)
      continue;

    /* Table is parent in a non-self-referencing foreign key. */
    my_error(ER_TRUNCATE_ILLEGAL_FK, MYF(0), fk_info_str(thd, fk_p));
    return true;
  }
  return false;
}

enum class Truncate_result {
  OK = 0,
  FAILED_BUT_BINLOG,
  FAILED_SKIP_BINLOG,
  FAILED_OPEN
};

/**
  Open and truncate a locked base table.

  @param  thd           Thread context.
  @param  table_ref     Table list element for the table to be truncated.
  @param  table_def     Dictionary table object.

  @retval Truncate_result::OK   Truncate was successful and statement can be
                        safely binlogged.

  @retval Truncate_result::FAILED_BUT_BINLOG Truncate failed but still
                        go ahead with binlogging as in case of non
                        transactional tables partial truncation is
                        possible.

  @retval Truncate_result::FAILED_SKIP_BINLOG Truncate was not
                        successful hence do not binlog the statement.

  @retval Truncate_result::FAILED_OPEN Truncate failed to open table,
                        do not binlog the statement.
*/

static Truncate_result handler_truncate_base(THD *thd, TABLE_LIST *table_ref,
                                             dd::Table *table_def) {
  DBUG_TRACE;
  DBUG_ASSERT(table_def != nullptr);

  /*
    Can't recreate, the engine must mechanically delete all rows
    in the table. Use open_and_lock_tables() to open a write cursor.
  */

  /* We don't need to load triggers. */
  DBUG_ASSERT(table_ref->trg_event_map == 0);
  /*
    Our metadata lock guarantees that no transaction is reading
    or writing into the table. Yet, to open a write cursor we need
    a thr_lock lock. Allow to open base tables only.
  */
  table_ref->required_type = dd::enum_table_type::BASE_TABLE;
  /*
    Ignore pending FLUSH TABLES since we don't want to release
    the MDL lock taken above and otherwise there is no way to
    wait for FLUSH TABLES in deadlock-free fashion.
  */
  uint flags = MYSQL_OPEN_IGNORE_FLUSH;
  /*
    Even though we have an MDL lock on the table here, we don't
    pass MYSQL_OPEN_HAS_MDL_LOCK to open_and_lock_tables
    since to truncate a MERGE table, we must open and lock
    merge children, and on those we don't have an MDL lock.
    Thus clear the ticket to satisfy MDL asserts.
  */
  table_ref->mdl_request.ticket = nullptr;

  /* Open the table as it will handle some required preparations. */
  if (open_and_lock_tables(thd, table_ref, flags))
    return Truncate_result::FAILED_OPEN;

  /*
    Remove all TABLE/handler instances except the one to be used for
    handler::ha_truncate() call. This is necessary for InnoDB to be
    able to correctly handle truncate as atomic drop and re-create
    internally. If we under LOCK TABLES the caller will re-open tables
    as necessary later.
  */
  close_all_tables_for_name(thd, table_ref->table->s, false, table_ref->table);

  int error = table_ref->table->file->ha_truncate(table_def);

  if (error) {
    table_ref->table->file->print_error(error, MYF(0));
    /*
      If truncate method is not implemented then we don't binlog the
      statement. If truncation has failed in a transactional engine then also we
      donot binlog the statment. Only in non transactional engine we binlog
      inspite of errors.
     */
    if (error == HA_ERR_WRONG_COMMAND ||
        table_ref->table->file->has_transactions())
      return Truncate_result::FAILED_SKIP_BINLOG;
    else
      return Truncate_result::FAILED_BUT_BINLOG;
  } else if ((table_ref->table->file->ht->flags & HTON_SUPPORTS_ATOMIC_DDL)) {
    if (thd->dd_client()->update(table_def)) {
      /* Statement rollback will revert effect of handler::truncate() as well.
       */
      return Truncate_result::FAILED_SKIP_BINLOG;
    }
  }
  return Truncate_result::OK;
}

/**
  Open and truncate a locked temporary table.

  @param  thd           Thread context.
  @param  table_ref     Table list element for the table to be truncated.

  @retval Truncate_result::OK   Truncate was successful and statement can be
                        safely binlogged.

  @retval Truncate_result::FAILED_BUT_BINLOG Truncate failed but still
                        go ahead with binlogging as in case of non
                        transactional tables partial truncation is
                        possible.

  @retval Truncate_result::FAILED_SKIP_BINLOG Truncate was not successful hence
                        do not binlog the statement.

  @retval Truncate_result::FAILED_OPEN Truncate failed to open table, do not
                        binlog the statement.
*/

static Truncate_result handler_truncate_temporary(THD *thd,
                                                  TABLE_LIST *table_ref) {
  DBUG_TRACE;

  /*
    Can't recreate, the engine must mechanically delete all rows
    in the table. Use open_and_lock_tables() to open a write cursor.
  */

  /* Open the table as it will handle some required preparations. */
  if (open_and_lock_tables(thd, table_ref, 0))
    return Truncate_result::FAILED_OPEN;

  int error =
      table_ref->table->file->ha_truncate(table_ref->table->s->tmp_table_def);

  if (error) {
    table_ref->table->file->print_error(error, MYF(0));
    /*
      If truncate method is not implemented then we don't binlog the
      statement. If truncation has failed in a transactional engine then also we
      donot binlog the statment. Only in non transactional engine we binlog
      inspite of errors.
     */
    if (error == HA_ERR_WRONG_COMMAND ||
        table_ref->table->file->has_transactions())
      return Truncate_result::FAILED_SKIP_BINLOG;
    else
      return Truncate_result::FAILED_BUT_BINLOG;
  }
  return Truncate_result::OK;
}

/*
  Handle locking a base table for truncate.

  @param[in]  thd               Thread context.
  @param[in]  table_ref         Table list element for the table to
                                be truncated.

  @retval  false  Success.
  @retval  true   Error.
*/

bool Sql_cmd_truncate_table::lock_table(THD *thd, TABLE_LIST *table_ref) {
  TABLE *table = nullptr;
  DBUG_TRACE;

  /* Lock types are set in the parser. */
  DBUG_ASSERT(table_ref->lock_descriptor().type == TL_WRITE);
  /* The handler truncate protocol dictates a exclusive lock. */
  DBUG_ASSERT(table_ref->mdl_request.type == MDL_EXCLUSIVE);

  /*
    Before doing anything else, acquire a metadata lock on the table,
    or ensure we have one.  We don't use open_and_lock_tables()
    right away because we want to be able to truncate (and recreate)
    corrupted tables, those that we can't fully open.

    MySQL manual documents that TRUNCATE can be used to repair a
    damaged table, i.e. a table that can not be fully "opened".
    In particular MySQL manual says: As long as the table format
    file tbl_name.frm is valid, the table can be re-created as
    an empty table with TRUNCATE TABLE, even if the data or index
    files have become corrupted.
  */
  if (thd->locked_tables_mode) {
    if (!(table = find_table_for_mdl_upgrade(thd, table_ref->db,
                                             table_ref->table_name, false)))
      return true;

    if (acquire_shared_backup_lock_nsec(thd,
                                        thd->variables.lock_wait_timeout_nsec))
      return true;

    table_ref->mdl_request.ticket = table->mdl_ticket;

    /*
      A storage engine can recreate or truncate the table only if there
      are no references to it from anywhere, i.e. no cached TABLE in the
      table cache.
    */
    DEBUG_SYNC(thd, "upgrade_lock_for_truncate");
    /* To remove the table from the cache we need an exclusive lock. */
    if (wait_while_table_is_used(thd, table, HA_EXTRA_FORCE_REOPEN))
      return true;
    m_ticket_downgrade = table->mdl_ticket;
    /* Close if table is going to be recreated. */
    if (table->s->db_type()->flags & HTON_CAN_RECREATE)
      close_all_tables_for_name(thd, table->s, false, nullptr);

    return false;
  }  //  if (thd->locked_tables_mode)
  DBUG_ASSERT(!thd->locked_tables_mode);

  /* Acquire an exclusive lock. */
  DBUG_ASSERT(table_ref->next_global == nullptr);
  if (lock_table_names_nsec(thd, table_ref, nullptr,
                            thd->variables.lock_wait_timeout_nsec, 0))
    return true;

  /* Table is already locked exclusively. Remove cached instances. */
  tdc_remove_table(thd, TDC_RT_REMOVE_ALL, table_ref->db, table_ref->table_name,
                   false);

  return false;
}

/**
   Completes transaction by attempting to binlog and commit if
   truncate has been successful so far. Rolls back if truncate has
   already failed, or if binlogging or commit fails.
 */
void Sql_cmd_truncate_table::end_transaction(THD *thd, bool binlog_stmt,
                                             bool binlog_is_trans) {
  if (binlog_stmt) {
    int ble = write_bin_log(thd, !m_error, thd->query().str,
                            thd->query().length, binlog_is_trans);
    m_error |= (ble != 0);
  }

  if (!m_error)
    m_error = (trans_commit_stmt(thd) || trans_commit_implicit(thd));

  if (m_error) {
    trans_rollback_stmt(thd);
    /*
      Full rollback in case we have THD::transaction_rollback_request
      and to synchronize DD state in cache and on disk (as statement
      rollback doesn't clear DD cache of modified uncommitted objects).
    */
    trans_rollback(thd);
  }
}

/**
   Performs cleanup actions after truncate of a normal (non-temporary)
   table. Calls post_ddl hook, reopens locked tables in locked_tables
   mode and possibly downgrades a temporarily upgraded MDL.
*/
void Sql_cmd_truncate_table::cleanup_base(THD *thd, const handlerton *hton) {
  if (hton != nullptr && (hton->flags & HTON_SUPPORTS_ATOMIC_DDL) &&
      hton->post_ddl)
    hton->post_ddl(thd);

  if (thd->locked_tables_mode && thd->locked_tables_list.reopen_tables(thd))
    thd->locked_tables_list.unlink_all_closed_tables(thd, nullptr, 0);

  /*
    A locked table ticket was upgraded to a exclusive lock. After the
    the query has been written to the binary log, downgrade the lock
    to a shared one.
  */
  if (m_ticket_downgrade) {
    m_ticket_downgrade->downgrade_lock(MDL_SHARED_NO_READ_WRITE);
    m_ticket_downgrade = nullptr;  // For possible re-execution
  }
}

/**
   Reopens a temporary table after truncate if supported.
   Deletes table and flags error if unuccessful.
*/
void Sql_cmd_truncate_table::cleanup_temporary(THD *thd, handlerton *hton,
                                               const TABLE_LIST &table_ref,
                                               Up_table *tdef_holder_ptr,
                                               const std::string &saved_path) {
  DBUG_ASSERT(m_ticket_downgrade == nullptr);

  if ((hton->flags & HTON_CAN_RECREATE) == 0 || !(*tdef_holder_ptr)) {
    // For the non-recreate case, or if we bailed before closing the table
    // (e.g. if thd->decide_logging_format() returns true)
    return;
  }

  /* Temporary table was closed and needs to be reopened. */
  DBUG_ASSERT(*tdef_holder_ptr);
  DBUG_ASSERT(saved_path.length() > 0);
  TABLE *new_table =
      open_table_uncached(thd, saved_path.c_str(), table_ref.db,
                          table_ref.table_name, true, true, **tdef_holder_ptr);
  if (new_table == nullptr) {
    // Not checking return value since we are cleaning up after a
    // failed open
    rm_temporary_table(thd, hton, saved_path.c_str(), tdef_holder_ptr->get());
    m_error = true;
    return;
  }
  DBUG_ASSERT(new_table != nullptr);

  // Transfer ownership of dd::Table object to the new
  // TABLE_SHARE. tdef_holder_ptr could either be the old or the new
  // table definition at this point.
  new_table->s->tmp_table_def = tdef_holder_ptr->release();
  thd->thread_specific_used = true;
}

/**
  Optimized delete of all rows by doing a full generate of the base
  (non-temporary) table.

  @remark Will work even if the .MYI and .MYD files are destroyed.
          In other words, it works as long as the .FRM is intact and
          the engine supports re-create.

  @param  thd         Thread context.
  @param  table_ref   Table list element for the table to be truncated.

  @retval  false  Success.
  @retval  true   Error.
*/

void Sql_cmd_truncate_table::truncate_base(THD *thd, TABLE_LIST *table_ref) {
  DBUG_TRACE;
  DBUG_ASSERT(is_temporary_table(table_ref) == false);

  m_error = true;
  bool binlog_stmt = false;
  bool binlog_is_trans = false;
  handlerton *hton = nullptr;

  DBUG_ASSERT((!table_ref->table) || (table_ref->table && table_ref->table->s));
  DBUG_ASSERT(m_ticket_downgrade == nullptr);

  dd::Schema_MDL_locker mdl_locker(thd);
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());

  // Actions needed to cleanup before leaving scope.
  auto cleanup_guard = create_scope_guard([&]() {
    end_transaction(thd, binlog_stmt, binlog_is_trans);
    cleanup_base(thd, hton);
  });
  if (mdl_locker.ensure_locked(table_ref->db)) return;

  if (lock_table(thd, table_ref)) return;

  dd::Table *table_def = nullptr;
  if (thd->dd_client()->acquire_for_modification(
          table_ref->db, table_ref->table_name, &table_def)) {
    return;
  }
  if (table_def == nullptr ||
      table_def->hidden() == dd::Abstract_table::HT_HIDDEN_SE) {
    my_error(ER_NO_SUCH_TABLE, MYF(0), table_ref->db, table_ref->table_name);
    return;
  }
  DBUG_ASSERT(table_def != nullptr);

  if (table_def->options().exists("secondary_engine")) {
    /* Truncate operation is not allowed for tables with secondary engine
     * since it's not currently supported by change propagation
     */
    my_error(ER_SECONDARY_ENGINE_DDL, MYF(0));
    return;
  }
  if (dd::table_storage_engine(thd, table_def, &hton)) {
    return;
  }
  DBUG_ASSERT(hton != nullptr);

  /*
    Check if table can't be truncated because there is a foreign key
    on some other table which references it.
  */
  if (!(thd->variables.option_bits & OPTION_NO_FOREIGN_KEY_CHECKS)) {
    if (fk_truncate_illegal_if_parent(thd, table_ref, table_def)) {
      return;
    }
  }

  if (hton->flags & HTON_CAN_RECREATE) {
    // Set this before any potential error returns
    binlog_is_trans = (hton->flags & HTON_SUPPORTS_ATOMIC_DDL);

    if (mysql_audit_table_access_notify(thd, table_ref) != 0) {
      return;
    }

    /*
      The storage engine can truncate the table by creating an
      empty table with the same structure.
    */
    HA_CREATE_INFO create_info;

    // Create a path to the table, but without a extension
    char path[FN_REFLEN + 1];
    build_table_filename(path, sizeof(path) - 1, table_ref->db,
                         table_ref->table_name, "", 0);

    // Attempt to reconstruct the table
    if (ha_create_table(thd, path, table_ref->db, table_ref->table_name,
                        &create_info, true, false, table_def) != 0) {
      return;
    }

    // Binlog only if truncate-by-recreate succeeds.
    m_error = false;
    binlog_stmt = true;
    return;
  }  // hton->flags & HTON_CAN_RECREATE

  DBUG_ASSERT((hton->flags & HTON_CAN_RECREATE) == false);
  /*
    The engine does not support truncate-by-recreate.
    Attempt to use the handler truncate method.
    MYSQL_AUDIT_TABLE_ACCESS_READ audit event is generated when opening
    tables using open_tables function.
  */
  const Truncate_result tr = handler_truncate_base(thd, table_ref, table_def);
  switch (tr) {
      /*
        All effects of a TRUNCATE TABLE operation are committed even if
        truncation fails in the case of non transactional tables. Thus, the
        query must be written to the binary log for such tables.
        The exceptions are failure to open table or unimplemented truncate
        method.
      */
    case Truncate_result::OK:
      m_error = false;
      // fallthrough
    case Truncate_result::FAILED_BUT_BINLOG:
      binlog_stmt = true;
      binlog_is_trans = table_ref->table->file->has_transactions();
      // fallthrough
    case Truncate_result::FAILED_SKIP_BINLOG:
      /*
        Call to handler_truncate() might have updated table definition
        in the data-dictionary, let us remove TABLE_SHARE from the TDC.
        This needs to be done even in case of failure so InnoDB SE
        properly invalidates its internal cache.
      */
      close_all_tables_for_name(thd, table_ref->table->s, false, nullptr);
      break;

    case Truncate_result::FAILED_OPEN:
      // Nothing to do here
      break;

    default:
      DBUG_ASSERT(false);
  };

  DBUG_ASSERT(m_error || !thd->get_stmt_da()->is_set());
}

/**
  Optimized delete of all rows by doing a full generate of the temporary table.

  @remark Will work even if the .MYI and .MYD files are destroyed.
          In other words, it works as long as the .FRM is intact and
          the engine supports re-create.

  @param  thd         Thread context.
  @param  table_ref   Table list element for the table to be truncated.

  @retval  false  Success.
  @retval  true   Error.
*/

void Sql_cmd_truncate_table::truncate_temporary(THD *thd,
                                                TABLE_LIST *table_ref) {
  DBUG_TRACE;
  DBUG_ASSERT(is_temporary_table(table_ref));

  /* Initialize, or reinitialize in case of reexecution (SP). */
  m_error = true;
  bool binlog_stmt = false;
  bool binlog_is_trans = false;
  handlerton *hton = nullptr;

  Up_table tdef_holder;
  std::string saved_path;

  m_ticket_downgrade = nullptr;

  DBUG_ASSERT((!table_ref->table) || (table_ref->table && table_ref->table->s));

  // Actions needed to cleanup before leaving scope.
  auto cleanup_guard = create_scope_guard([&]() {
    end_transaction(thd, binlog_stmt, binlog_is_trans);
    cleanup_temporary(thd, hton, *table_ref, &tdef_holder, saved_path);
  });

  TABLE *tmp_table = table_ref->table;
  hton = tmp_table->s->db_type();

  /*
    THD::decide_logging_format has not yet been called and may
    not be called at all depending on the engine, so call it here.
  */
  if (thd->decide_logging_format(table_ref) != 0) return;

  /* Note that a temporary table cannot be partitioned. */
  if (hton->flags & HTON_CAN_RECREATE) {
    tmp_table->file->info(HA_STATUS_AUTO | HA_STATUS_NO_LOCK);
    /*
      If LOCK TABLES list is not empty and contains this table
      then unlock the table and remove it from this list.
    */
    mysql_lock_remove(thd, thd->lock, tmp_table);

    /*
      Transfer ownership of dd::Table object and save path so
      we can reopen table after freeing the TABLE_SHARE.
    */
    tdef_holder.reset(tmp_table->s->tmp_table_def);
    tmp_table->s->tmp_table_def = nullptr;
    saved_path.assign(tmp_table->s->path.str, tmp_table->s->path.length);

    /* Save normalized path so we can free TABLE_SHARE. */
    std::string saved_norm_path{tmp_table->s->normalized_path.str,
                                tmp_table->s->normalized_path.length};

    /* Free TABLE and TABLE_SHARE but don't delete table. */
    close_temporary_table(thd, tmp_table, true, false);

    /*
      We must use normalized_path since for temporary tables it
      differs from what dd_recreate_table() would generate based
      on table and schema names.
    */
    HA_CREATE_INFO create_info;

    // Create a clone of the tdef which can be manipulated by ha_create_table
    Up_table tdef_clone = Up_table{tdef_holder->clone()};
    m_error = ha_create_table(thd, saved_norm_path.c_str(), table_ref->db,
                              table_ref->table_name, &create_info, true, true,
                              tdef_clone.get());

    DBUG_ASSERT(
        !thd->get_transaction()->cannot_safely_rollback(Transaction_ctx::STMT));
    binlog_is_trans = (hton->flags & HTON_SUPPORTS_ATOMIC_DDL);

    if (!m_error || (hton->flags & HTON_SUPPORTS_ATOMIC_DDL) == 0) {
      // If create succeeded, or the SE does not support atomic ddl,
      // we need to replace the table def
      tdef_holder.reset(tdef_clone.release());
    }
    if (m_error) {
      return;
    }

    /* Only binlog if truncate-by-recreate succeeds. */
    /* In RBR, the statement is not binlogged if the table is temporary. */
    binlog_stmt = !thd->is_current_stmt_binlog_format_row();
    return;
  }  // hton->flags & HTON_CAN_RECREATE

  DBUG_ASSERT((hton->flags & HTON_CAN_RECREATE) == false);
  /*
    The engine does not support truncate-by-recreate. Open the
    table and invoke the handler truncate. In such a manner this
    can in fact open several tables if it's a temporary MyISAMMRG
    table.
  */
  const Truncate_result tr = handler_truncate_temporary(thd, table_ref);
  if (tr != Truncate_result::OK) {
    return;
  }
  m_error = false;
  /* Only binlog if truncate succeeds. */
  /* In RBR, the statement is not binlogged if the table is temporary. */
  binlog_stmt = !thd->is_current_stmt_binlog_format_row();
  binlog_is_trans = table_ref->table->file->has_transactions();
}

/**
  Execute a TRUNCATE statement at runtime.

  @param  thd   The current thread.

  @return false on success.
*/
bool Sql_cmd_truncate_table::execute(THD *thd) {
  DBUG_TRACE;

  TABLE_LIST *first_table = thd->lex->select_lex->table_list.first;
  if (check_one_table_access(thd, DROP_ACL, first_table)) return true;

  if (is_temporary_table(first_table))
    truncate_temporary(thd, first_table);
  else
    truncate_base(thd, first_table);

  if (!m_error) my_ok(thd);

  return m_error;
}
