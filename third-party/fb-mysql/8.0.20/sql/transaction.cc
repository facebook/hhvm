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

#include "sql/transaction.h"

#include <stddef.h>

#include "lex_string.h"
#include "m_ctype.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_loglevel.h"
#include "my_psi_config.h"
#include "my_sys.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/psi/mysql_transaction.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "sql/auth/auth_common.h"
#include "sql/dd/cache/dictionary_client.h"
#include "sql/debug_sync.h"  // DEBUG_SYNC
#include "sql/handler.h"
#include "sql/log.h"
#include "sql/mdl.h"
#include "sql/mysqld.h"  // opt_readonly
#include "sql/query_options.h"
#include "sql/rpl_context.h"
#include "sql/rpl_gtid.h"
#include "sql/rpl_master.h"
#include "sql/rpl_rli.h"
#include "sql/rpl_transaction_write_set_ctx.h"
#include "sql/session_tracker.h"
#include "sql/sql_class.h"  // THD
#include "sql/sql_lex.h"
#include "sql/system_variables.h"
#include "sql/tc_log.h"
#include "sql/transaction_info.h"
#include "sql/xa.h"

/**
  Helper: Tell tracker (if any) that transaction ended.
*/
void trans_track_end_trx(THD *thd) {
  TX_TRACKER_GET(tst);
  tst->end_trx(thd);
}

/**
  Helper: transaction ended, SET TRANSACTION one-shot variables
  revert to session values. Let the transaction state tracker know.
*/
void trans_reset_one_shot_chistics(THD *thd) {
  if (thd->variables.session_track_transaction_info > TX_TRACK_NONE) {
    TX_TRACKER_GET(tst);
    tst->set_read_flags(thd, TX_READ_INHERIT);
    tst->set_isol_level(thd, TX_ISOL_INHERIT);
  }

  thd->tx_isolation = (enum_tx_isolation)thd->variables.transaction_isolation;
  thd->tx_read_only = thd->variables.transaction_read_only;
}

/**
  Check if we have a condition where the transaction state must
  not be changed (committed or rolled back). Currently we check
  that we are not executing a stored program and that we don't
  have an active XA transaction.

  @return true if the commit/rollback cannot be executed,
          false otherwise.
*/

bool trans_check_state(THD *thd) {
  DBUG_TRACE;

  /*
    Always commit statement transaction before manipulating with
    the normal one.
  */
  DBUG_ASSERT(thd->get_transaction()->is_empty(Transaction_ctx::STMT));

  if (unlikely(thd->in_sub_stmt)) {
    my_error(ER_COMMIT_NOT_ALLOWED_IN_SF_OR_TRG, MYF(0));
    return true;
  }

  if (thd->get_transaction()->xid_state()->check_in_xa(true)) return true;

  return false;
}

/**
  Begin a new transaction.

  @note Beginning a transaction implicitly commits any current
        transaction and releases existing locks.

  @param thd     Current thread
  @param flags   Transaction flags

  @retval false  Success
  @retval true   Failure
*/

bool trans_begin(THD *thd, uint flags, bool *need_ok, handlerton *hton) {
  bool res = false;
  DBUG_TRACE;

  if (trans_check_state(thd)) return true;

  TX_TRACKER_GET(tst);

  thd->locked_tables_list.unlock_locked_tables(thd);

  DBUG_ASSERT(!thd->locked_tables_mode);

  if (thd->in_multi_stmt_transaction_mode() ||
      (thd->variables.option_bits & OPTION_TABLE_LOCK)) {
    thd->variables.option_bits &= ~OPTION_TABLE_LOCK;
    thd->server_status &=
        ~(SERVER_STATUS_IN_TRANS | SERVER_STATUS_IN_TRANS_READONLY);
    DBUG_PRINT("info", ("clearing SERVER_STATUS_IN_TRANS"));
    res = ha_commit_trans(thd, true);
  }

  thd->variables.option_bits &= ~OPTION_BEGIN;
  thd->get_transaction()->reset_unsafe_rollback_flags(Transaction_ctx::SESSION);

  if (res) return true;

  /*
    Release transactional metadata locks only after the
    transaction has been committed.
  */
  thd->mdl_context.release_transactional_locks();

  // The RO/RW options are mutually exclusive.
  DBUG_ASSERT(!((flags & MYSQL_START_TRANS_OPT_READ_ONLY) &&
                (flags & MYSQL_START_TRANS_OPT_READ_WRITE)));
  if (flags & MYSQL_START_TRANS_OPT_READ_ONLY) {
    thd->tx_read_only = true;
    if (tst) tst->set_read_flags(thd, TX_READ_ONLY);
  } else if (flags & MYSQL_START_TRANS_OPT_READ_WRITE) {
    /*
      Explicitly starting a RW transaction when the server is in
      read-only mode, is not allowed unless the user has SUPER priv.
      Implicitly starting a RW transaction is allowed for backward
      compatibility.
    */
    if (check_readonly(thd, true)) return true;
    thd->tx_read_only = false;
    /*
      This flags that tx_read_only was set explicitly, rather than
      just from the session's default.
    */
    if (tst) tst->set_read_flags(thd, TX_READ_WRITE);
  }

  DBUG_EXECUTE_IF("dbug_set_high_prio_trx", {
    DBUG_ASSERT(thd->tx_priority == 0);
    thd->tx_priority = 1;
  });

  thd->variables.option_bits |= OPTION_BEGIN;
  thd->server_status |= SERVER_STATUS_IN_TRANS;
  if (thd->tx_read_only) thd->server_status |= SERVER_STATUS_IN_TRANS_READONLY;
  DBUG_PRINT("info", ("setting SERVER_STATUS_IN_TRANS"));

  if (tst) tst->add_trx_state(thd, TX_EXPLICIT);

  snapshot_info_st ss_info;
  ss_info.snapshot_id = thd->lex->snapshot_id;

  /* ha_start_consistent_snapshot() relies on OPTION_BEGIN flag set. */
  if (flags & MYSQL_START_TRANS_OPT_WITH_CONS_SNAPSHOT) {
    if (tst) tst->add_trx_state(thd, TX_WITH_SNAPSHOT);
    res = ha_start_consistent_snapshot(thd, nullptr, nullptr);
  } else if (flags & MYSQL_START_TRANS_OPT_WITH_CONS_ENGINE_SNAPSHOT) {
    DBUG_ASSERT(need_ok != nullptr);
    /*
      Even though a single engine is specified, for safety, all
      engines should start a consistent snapshot.
     */
    res = ha_start_consistent_snapshot(thd, &ss_info, nullptr) ||
          show_master_offset(thd, ss_info, need_ok);
  } else if (flags & MYSQL_START_TRANS_OPT_WITH_SHAR_ENGINE_SNAPSHOT) {
    DBUG_ASSERT(need_ok != nullptr);
    ss_info.op = snapshot_operation::SNAPSHOT_CREATE;
    res = ha_start_shared_snapshot(thd, &ss_info, hton) ||
          show_master_offset(thd, ss_info, need_ok);
  } else if (flags & MYSQL_START_TRANS_OPT_WITH_EXIS_ENGINE_SNAPSHOT) {
    DBUG_ASSERT(need_ok != nullptr);
    ss_info.op = snapshot_operation::SNAPSHOT_ATTACH;
    res = ha_start_shared_snapshot(thd, &ss_info, hton) ||
          show_master_offset(thd, ss_info, need_ok);
  }

  /*
    Register transaction start in performance schema if not done already.
    We handle explicitly started transactions here, implicitly started
    transactions (and single-statement transactions in autocommit=1 mode)
    are handled in trans_register_ha().
    We can't handle explicit transactions in the same way as implicit
    because we want to correctly attribute statements which follow
    BEGIN but do not touch any transactional tables.
  */
#ifdef HAVE_PSI_TRANSACTION_INTERFACE
  if (thd->m_transaction_psi == nullptr) {
    thd->m_transaction_psi =
        MYSQL_START_TRANSACTION(&thd->m_transaction_state, nullptr, nullptr,
                                thd->tx_isolation, thd->tx_read_only, false);
    DEBUG_SYNC(thd, "after_set_transaction_psi_before_set_transaction_gtid");
    gtid_set_performance_schema_values(thd);
  }
#endif

  return res;
}

/**
  Commit the current transaction, making its changes permanent.

  @param[in] thd                       Current thread
  @param[in] ignore_global_read_lock   Allow commit to complete even if a
                                       global read lock is active. This can be
                                       used to allow changes to internal tables
                                       (e.g. slave status tables, analyze
  table).

  @retval false  Success
  @retval true   Failure
*/

bool trans_commit(THD *thd, bool ignore_global_read_lock) {
  int res;
  DBUG_TRACE;

  if (trans_check_state(thd)) return true;

  thd->server_status &=
      ~(SERVER_STATUS_IN_TRANS | SERVER_STATUS_IN_TRANS_READONLY);
  DBUG_PRINT("info", ("clearing SERVER_STATUS_IN_TRANS"));
  res = ha_commit_trans(thd, true, ignore_global_read_lock);
  if (res == false)
    if (thd->rpl_thd_ctx.session_gtids_ctx().notify_after_transaction_commit(
            thd))
      LogErr(WARNING_LEVEL, ER_TRX_GTID_COLLECT_REJECT);
  /*
    When gtid mode is enabled, a transaction may cause binlog
    rotation, which inserts a record into the gtid system table
    (which is probably a transactional table). Thence, the flag
    SERVER_STATUS_IN_TRANS may be set again while calling
    ha_commit_trans(...) Consequently, we need to reset it back,
    much like we are doing before calling ha_commit_trans(...).

    We would really only need to do this when gtid_mode=on.  However,
    checking gtid_mode requires holding a lock, which is costly.  So
    we clear the bit unconditionally.  This has no side effects since
    if gtid_mode=off the bit is already cleared.
  */
  thd->server_status &= ~SERVER_STATUS_IN_TRANS;
  thd->variables.option_bits &= ~OPTION_BEGIN;
  thd->get_transaction()->reset_unsafe_rollback_flags(Transaction_ctx::SESSION);
  thd->lex->start_transaction_opt = 0;

  /* The transaction should be marked as complete in P_S. */
  DBUG_ASSERT(thd->m_transaction_psi == nullptr);

  thd->tx_priority = 0;

  trans_track_end_trx(thd);

  /*
    Avoid updating modified uncommitted objects when committing attachable
    read-write transaction. This is required to allow I_S queries to update
    table statistics during CREATE TABLE ... SELECT, otherwise the
    uncommitted object added by DDL would be removed by I_S query.
  */
  if (!thd->is_attachable_rw_transaction_active()) {
    /*
      If the SE failed to commit the transaction, we must rollback the
      modified dictionary objects to make sure the DD cache, the DD
      tables and the state in the SE stay in sync.
    */
    if (res)
      thd->dd_client()->rollback_modified_objects();
    else
      thd->dd_client()->commit_modified_objects();
  }

  thd->locked_tables_list.adjust_renamed_tablespace_mdls(&thd->mdl_context);

  return res;
}

/**
  Implicitly commit the current transaction.

  @note A implicit commit does not releases existing table locks.

  @param[in] thd                       Current thread
  @param[in] ignore_global_read_lock   Allow commit to complete even if a
                                       global read lock is active. This can be
                                       used to allow changes to internal tables
                                       (e.g. slave status tables, analyze
  table).


  @retval false  Success
  @retval true   Failure
*/

bool trans_commit_implicit(THD *thd, bool ignore_global_read_lock) {
  bool res = false;
  DBUG_TRACE;

  /*
    Ensure that trans_check_state() was called before trans_commit_implicit()
    by asserting that conditions that are checked in the former function are
    true.
  */
  DBUG_ASSERT(thd->get_transaction()->is_empty(Transaction_ctx::STMT) &&
              !thd->in_sub_stmt &&
              !thd->get_transaction()->xid_state()->check_in_xa(false));

  if (thd->in_multi_stmt_transaction_mode() ||
      (thd->variables.option_bits & OPTION_TABLE_LOCK)) {
    /* Safety if one did "drop table" on locked tables */
    if (!thd->locked_tables_mode)
      thd->variables.option_bits &= ~OPTION_TABLE_LOCK;
    thd->server_status &=
        ~(SERVER_STATUS_IN_TRANS | SERVER_STATUS_IN_TRANS_READONLY);
    DBUG_PRINT("info", ("clearing SERVER_STATUS_IN_TRANS"));
    res = ha_commit_trans(thd, true, ignore_global_read_lock);
  } else if (tc_log)
    res = tc_log->commit(thd, true);

  if (res == false)
    if (thd->rpl_thd_ctx.session_gtids_ctx().notify_after_transaction_commit(
            thd))
      LogErr(WARNING_LEVEL, ER_TRX_GTID_COLLECT_REJECT);
  thd->variables.option_bits &= ~OPTION_BEGIN;
  thd->get_transaction()->reset_unsafe_rollback_flags(Transaction_ctx::SESSION);

  /* The transaction should be marked as complete in P_S. */
  DBUG_ASSERT(thd->m_transaction_psi == nullptr);

  /*
    Upon implicit commit, reset the current transaction
    isolation level and access mode. We do not care about
    @@session.completion_type since it's documented
    to not have any effect on implicit commit.
  */
  trans_reset_one_shot_chistics(thd);

  trans_track_end_trx(thd);

  /*
    Avoid updating modified uncommitted objects when committing attachable
    read-write transaction. This is required to allow I_S queries to update
    table statistics during CREATE TABLE ... SELECT, otherwise the
    uncommitted object added by DDL would be removed by I_S query.
  */
  if (!thd->is_attachable_rw_transaction_active()) {
    /*
      If the SE failed to commit the transaction, we must rollback the
      modified dictionary objects to make sure the DD cache, the DD
      tables and the state in the SE stay in sync.
    */
    if (res)
      thd->dd_client()->rollback_modified_objects();
    else
      thd->dd_client()->commit_modified_objects();
  }

  thd->locked_tables_list.adjust_renamed_tablespace_mdls(&thd->mdl_context);
  return res;
}

/**
  Rollback the current transaction, canceling its changes.

  @param thd     Current thread

  @retval false  Success
  @retval true   Failure
*/

bool trans_rollback(THD *thd) {
  int res;
  DBUG_TRACE;

  if (trans_check_state(thd)) return true;

  thd->server_status &=
      ~(SERVER_STATUS_IN_TRANS | SERVER_STATUS_IN_TRANS_READONLY);
  DBUG_PRINT("info", ("clearing SERVER_STATUS_IN_TRANS"));
  res = ha_rollback_trans(thd, true);
  thd->variables.option_bits &= ~OPTION_BEGIN;
  thd->get_transaction()->reset_unsafe_rollback_flags(Transaction_ctx::SESSION);
  thd->lex->start_transaction_opt = 0;

  /* The transaction should be marked as complete in P_S. */
  DBUG_ASSERT(thd->m_transaction_psi == nullptr);

  thd->tx_priority = 0;

  trans_track_end_trx(thd);

  /*
    Avoid updating modified uncommitted objects when rolling back
    attachable read-write transaction. This is required to allow I_S
    queries to update table statistics during CREATE TABLE ... SELECT,
    otherwise the uncommitted object added by DDL would be removed by I_S
    query.
  */
  if (!thd->is_attachable_rw_transaction_active())
    thd->dd_client()->rollback_modified_objects();

  thd->locked_tables_list.discard_renamed_tablespace_mdls();

  return res;
}

/**
  Implicitly rollback the current transaction, typically
  after deadlock was discovered.

  @param thd     Current thread

  @retval False Success
  @retval True  Failure

  @note ha_rollback_low() which is indirectly called by this
        function will mark XA transaction for rollback by
        setting appropriate RM error status if there was
        transaction rollback request.
*/

bool trans_rollback_implicit(THD *thd) {
  int res;
  DBUG_TRACE;

  /*
    Always commit/rollback statement transaction before manipulating
    with the normal one.
    Don't perform rollback in the middle of sub-statement, wait till
    its end.
  */
  DBUG_ASSERT(thd->get_transaction()->is_empty(Transaction_ctx::STMT) &&
              !thd->in_sub_stmt);

  thd->server_status &=
      ~(SERVER_STATUS_IN_TRANS | SERVER_STATUS_IN_TRANS_READONLY);
  DBUG_PRINT("info", ("clearing SERVER_STATUS_IN_TRANS"));
  res = ha_rollback_trans(thd, true);
  thd->variables.option_bits &= ~OPTION_BEGIN;
  thd->get_transaction()->reset_unsafe_rollback_flags(Transaction_ctx::SESSION);

  /* Rollback should clear transaction_rollback_request flag. */
  DBUG_ASSERT(!thd->transaction_rollback_request);
  /* The transaction should be marked as complete in P_S. */
  DBUG_ASSERT(thd->m_transaction_psi == nullptr);

  trans_track_end_trx(thd);

  /*
    Avoid updating modified uncommitted objects when rolling back
    attachable read-write transaction. This is required to allow I_S
    queries to update table statistics during CREATE TABLE ... SELECT,
    otherwise the uncommitted object added by DDL would be removed by I_S
    query.
  */
  if (!thd->is_attachable_rw_transaction_active())
    thd->dd_client()->rollback_modified_objects();

  thd->locked_tables_list.discard_renamed_tablespace_mdls();

  return res;
}

/**
  Commit the single statement transaction.

  @note Note that if the autocommit is on, then the following call
        inside InnoDB will commit or rollback the whole transaction
        (= the statement). The autocommit mechanism built into InnoDB
        is based on counting locks, but if the user has used LOCK
        TABLES then that mechanism does not know to do the commit.

  @param[in] thd                       Current thread
  @param[in] ignore_global_read_lock   Allow commit to complete even if a
                                       global read lock is active. This can be
                                       used to allow changes to internal tables
                                       (e.g. slave status tables, analyze
  table).


  @retval false  Success
  @retval true   Failure
*/

bool trans_commit_stmt(THD *thd, bool ignore_global_read_lock) {
  DBUG_TRACE;
  int res = false;
  /*
    We currently don't invoke commit/rollback at end of
    a sub-statement.  In future, we perhaps should take
    a savepoint for each nested statement, and release the
    savepoint when statement has succeeded.
  */
  DBUG_ASSERT(!thd->in_sub_stmt);

  /*
    Some code in MYSQL_BIN_LOG::commit and ha_commit_low() is not safe
    for attachable transactions.
  */
  DBUG_ASSERT(!thd->is_attachable_ro_transaction_active());

  thd->get_transaction()->merge_unsafe_rollback_flags();

  if (thd->get_transaction()->is_active(Transaction_ctx::STMT)) {
    res = ha_commit_trans(thd, false, ignore_global_read_lock);
    if (!thd->in_active_multi_stmt_transaction())
      trans_reset_one_shot_chistics(thd);
  } else if (tc_log)
    res = tc_log->commit(thd, false);
  if (res == false && !thd->in_active_multi_stmt_transaction())
    if (thd->rpl_thd_ctx.session_gtids_ctx().notify_after_transaction_commit(
            thd))
      LogErr(WARNING_LEVEL, ER_TRX_GTID_COLLECT_REJECT);
  /* In autocommit=1 mode the transaction should be marked as complete in P_S */
  DBUG_ASSERT(thd->in_active_multi_stmt_transaction() ||
              thd->m_transaction_psi == nullptr);

  thd->get_transaction()->reset(Transaction_ctx::STMT);

  return res;
}

/**
  Rollback the single statement transaction.

  @param thd     Current thread

  @retval false  Success
  @retval true   Failure
*/
bool trans_rollback_stmt(THD *thd) {
  DBUG_TRACE;

  /*
    We currently don't invoke commit/rollback at end of
    a sub-statement.  In future, we perhaps should take
    a savepoint for each nested statement, and release the
    savepoint when statement has succeeded.
  */
  DBUG_ASSERT(!thd->in_sub_stmt);

  /*
    Some code in MYSQL_BIN_LOG::rollback and ha_rollback_low() is not safe
    for attachable transactions.
  */
  DBUG_ASSERT(!thd->is_attachable_ro_transaction_active());

  thd->get_transaction()->merge_unsafe_rollback_flags();

  if (thd->get_transaction()->is_active(Transaction_ctx::STMT)) {
    ha_rollback_trans(thd, false);
    if (!thd->in_active_multi_stmt_transaction())
      trans_reset_one_shot_chistics(thd);
  } else if (tc_log)
    tc_log->rollback(thd, false);

  if (!thd->owned_gtid_is_empty() && !thd->in_active_multi_stmt_transaction()) {
    /*
      To a failed single statement transaction on auto-commit mode,
      we roll back its owned gtid if it does not modify
      non-transational table or commit its owned gtid if it has modified
      non-transactional table when rolling back it if binlog is disabled,
      as we did when binlog is enabled.
      We do not need to check if binlog is enabled here, since we already
      released its owned gtid in MYSQL_BIN_LOG::rollback(...) right before
      this if binlog is enabled.
    */
    if (thd->get_transaction()->has_modified_non_trans_table(
            Transaction_ctx::STMT))
      gtid_state->update_on_commit(thd);
    else
      gtid_state->update_on_rollback(thd);
  }
  /*
    Statement rollback for replicated atomic DDL should call
    post-rollback hook. This ensures the slave info won't be updated
    for failed atomic DDL statements during the eventual implicit
    commit which is done even even in case of DDL failure.

    Unlike the post_commit it has to be invoked even when there's
    no active statement transaction.
    TODO: consider to align the commit case to invoke pre- and post-
    hooks on the same level with the rollback one.
  */
  if (is_atomic_ddl_commit_on_slave(thd)) thd->rli_slave->post_rollback();

  /* In autocommit=1 mode the transaction should be marked as complete in P_S */
  DBUG_ASSERT(thd->in_active_multi_stmt_transaction() ||
              thd->m_transaction_psi == nullptr ||
              /* Todo: BUG#20488921 is in the way. */
              DBUG_EVALUATE_IF("simulate_xa_commit_log_failure", true, false));

  thd->get_transaction()->reset(Transaction_ctx::STMT);

  return false;
}

/**
  Commit the attachable transaction.

  @note This is slimmed down version of trans_commit_stmt() which commits
        attachable transaction but skips code which is unnecessary and
        unsafe for them (like dealing with GTIDs).

  @param thd     Current thread

  @retval False - Success
  @retval True  - Failure
*/
bool trans_commit_attachable(THD *thd) {
  DBUG_TRACE;
  int res = 0;

  /* This function only handles attachable transactions. */
  DBUG_ASSERT(thd->is_attachable_ro_transaction_active());

  /*
    Since the attachable transaction is AUTOCOMMIT we only need to commit
    statement transaction.
  */
  DBUG_ASSERT(!thd->get_transaction()->is_active(Transaction_ctx::SESSION));

  /* Attachable transactions should not do anything unsafe. */
  DBUG_ASSERT(
      !thd->get_transaction()->cannot_safely_rollback(Transaction_ctx::STMT));

  if (thd->get_transaction()->is_active(Transaction_ctx::STMT)) {
    res = ha_commit_attachable(thd);
  }

  DBUG_ASSERT(thd->m_transaction_psi == nullptr);

  thd->get_transaction()->reset(Transaction_ctx::STMT);

  return res;
}

/* Find a named savepoint in the current transaction. */
static SAVEPOINT **find_savepoint(THD *thd, LEX_STRING name) {
  SAVEPOINT **sv = &thd->get_transaction()->m_savepoints;

  while (*sv) {
    if (my_strnncoll(system_charset_info, (uchar *)name.str, name.length,
                     (uchar *)(*sv)->name, (*sv)->length) == 0)
      break;
    sv = &(*sv)->prev;
  }

  return sv;
}

/**
  Set a named transaction savepoint.

  @param thd    Current thread
  @param name   Savepoint name

  @retval false  Success
  @retval true   Failure
*/

bool trans_savepoint(THD *thd, LEX_STRING name) {
  SAVEPOINT **sv, *newsv;
  DBUG_TRACE;

  if (!(thd->in_multi_stmt_transaction_mode() || thd->in_sub_stmt) ||
      !opt_using_transactions)
    return false;

  if (thd->get_transaction()->xid_state()->check_has_uncommitted_xa())
    return true;

  sv = find_savepoint(thd, name);

  if (*sv) /* old savepoint of the same name exists */
  {
    newsv = *sv;
    if (ha_release_savepoint(thd, *sv)) {
      DBUG_ASSERT(thd->is_error() || thd->is_killed());
      return true;
    }
    *sv = (*sv)->prev;
  } else if ((newsv = (SAVEPOINT *)thd->get_transaction()->allocate_memory(
                  savepoint_alloc_size)) == nullptr) {
    my_error(ER_OUT_OF_RESOURCES, MYF(0));
    return true;
  }

  newsv->name = thd->get_transaction()->strmake(name.str, name.length);
  newsv->length = name.length;

  /*
    if we'll get an error here, don't add new savepoint to the list.
    we'll lose a little bit of memory in transaction mem_root, but it'll
    be free'd when transaction ends anyway
  */
  if (ha_savepoint(thd, newsv)) return true;

  newsv->prev = thd->get_transaction()->m_savepoints;
  thd->get_transaction()->m_savepoints = newsv;

  /*
    Remember locks acquired before the savepoint was set.
    They are used as a marker to only release locks acquired after
    the setting of this savepoint.
    Note: this works just fine if we're under LOCK TABLES,
    since mdl_savepoint() is guaranteed to be beyond
    the last locked table. This allows to release some
    locks acquired during LOCK TABLES.
  */
  newsv->mdl_savepoint = thd->mdl_context.mdl_savepoint();

  if (thd->is_current_stmt_binlog_row_enabled_with_write_set_extraction()) {
    thd->get_transaction()->get_transaction_write_set_ctx()->add_savepoint(
        name.str);
  }

  return false;
}

/**
  Rollback a transaction to the named savepoint.

  @note Modifications that the current transaction made to
        rows after the savepoint was set are undone in the
        rollback.

  @note Savepoints that were set at a later time than the
        named savepoint are deleted.

  @param thd    Current thread
  @param name   Savepoint name

  @retval false  Success
  @retval true   Failure
*/

bool trans_rollback_to_savepoint(THD *thd, LEX_STRING name) {
  int res = false;
  SAVEPOINT *sv = *find_savepoint(thd, name);
  DBUG_TRACE;

  if (sv == nullptr) {
    my_error(ER_SP_DOES_NOT_EXIST, MYF(0), "SAVEPOINT", name.str);
    return true;
  }

  if (thd->get_transaction()->xid_state()->check_has_uncommitted_xa())
    return true;

  if (ha_rollback_to_savepoint(thd, sv))
    res = true;
  else if (thd->get_transaction()->cannot_safely_rollback(
               Transaction_ctx::SESSION) &&
           !thd->slave_thread)
    thd->get_transaction()->push_unsafe_rollback_warnings(thd);

  thd->get_transaction()->m_savepoints = sv;

  /**
    Checking whether it is safe to release metadata locks acquired after
    savepoint, if rollback to savepoint is successful.

    Whether it is safe to release MDL after rollback to savepoint depends
    on storage engines participating in transaction:

    - InnoDB doesn't release any row-locks on rollback to savepoint so it
      is probably a bad idea to release MDL as well.
    - Binary log implementation in some cases (e.g when non-transactional
      tables involved) may choose not to remove events added after savepoint
      from transactional cache, but instead will write them to binary
      log accompanied with ROLLBACK TO SAVEPOINT statement. Since the real
      write happens at the end of transaction releasing MDL on tables
      mentioned in these events (i.e. acquired after savepoint and before
      rollback ot it) can break replication, as concurrent DROP TABLES
      statements will be able to drop these tables before events will get
      into binary log,
  */

  if (!res && ha_rollback_to_savepoint_can_release_mdl(thd))
    thd->mdl_context.rollback_to_savepoint(sv->mdl_savepoint);

  if (thd->is_current_stmt_binlog_row_enabled_with_write_set_extraction()) {
    thd->get_transaction()
        ->get_transaction_write_set_ctx()
        ->rollback_to_savepoint(name.str);
  }

  return res;
}

/**
  Remove the named savepoint from the set of savepoints of
  the current transaction.

  @note No commit or rollback occurs. It is an error if the
        savepoint does not exist.

  @param thd    Current thread
  @param name   Savepoint name

  @retval false  Success
  @retval true   Failure
*/

bool trans_release_savepoint(THD *thd, LEX_STRING name) {
  int res = false;
  SAVEPOINT *sv = *find_savepoint(thd, name);
  DBUG_TRACE;

  if (sv == nullptr) {
    my_error(ER_SP_DOES_NOT_EXIST, MYF(0), "SAVEPOINT", name.str);
    return true;
  }

  if (thd->get_transaction()->xid_state()->check_has_uncommitted_xa())
    return true;

  if (ha_release_savepoint(thd, sv)) res = true;

  thd->get_transaction()->m_savepoints = sv->prev;

  if (thd->is_current_stmt_binlog_row_enabled_with_write_set_extraction()) {
    thd->get_transaction()->get_transaction_write_set_ctx()->del_savepoint(
        name.str);
  }

  return res;
}
