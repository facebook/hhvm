/*
   Copyright (c) 2000, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/sql_class.h"

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <algorithm>
#include <list>
#include <memory>
#include <string>
#include <utility>

#include <sstream>

#include <rapidjson/document.h>

#include "field_types.h"
#include "m_ctype.h"
#include "m_string.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_loglevel.h"
#include "my_md5.h"
#include "my_md5_size.h"
#include "my_systime.h"
#include "my_thread.h"
#include "my_thread_os_id.h"
#include "my_time.h"
#include "mysql/components/services/log_builtins.h"  // LogErr
#include "mysql/components/services/log_shared.h"
#include "mysql/components/services/psi_error_bits.h"
#include "mysql/plugin_audit.h"
#include "mysql/psi/mysql_cond.h"
#include "mysql/psi/mysql_error.h"
#include "mysql/psi/mysql_ps.h"
#include "mysql/psi/mysql_stage.h"
#include "mysql/psi/mysql_statement.h"
#include "mysql/service_mysql_alloc.h"
#include "mysys_err.h"  // EE_OUTOFMEMORY
#include "pfs_statement_provider.h"
#include "rpl_master.h"  // unregister_slave
#include "sql/auth/auth_acls.h"
#include "sql/auth/sql_security_ctx.h"
#include "sql/binlog.h"
#include "sql/check_stack.h"
#include "sql/conn_handler/connection_handler_manager.h"  // Connection_handler_manager
#include "sql/current_thd.h"
#include "sql/dd/cache/dictionary_client.h"  // Dictionary_client
#include "sql/dd/dd_kill_immunizer.h"        // dd:DD_kill_immunizer
#include "sql/dd/dd_schema.h"                // dd::Schema_MDL_locker
#include "sql/debug_sync.h"                  // DEBUG_SYNC
#include "sql/derror.h"                      // ER_THD
#include "sql/enum_query_type.h"
#include "sql/error_handler.h"  // Internal_error_handler
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/item.h"
#include "sql/item_func.h"        // user_var_entry
#include "sql/lock.h"             // mysql_lock_abort_for_thread
#include "sql/locking_service.h"  // release_all_locking_service_locks
#include "sql/log_event.h"
#include "sql/mdl_context_backup.h"  // MDL context backup for XA
#include "sql/mysqld.h"              // global_system_variables ...
#include "sql/mysqld_thd_manager.h"  // Global_THD_manager
#include "sql/parse_location.h"
#include "sql/protocol.h"
#include "sql/protocol_classic.h"
#include "sql/psi_memory_key.h"
#include "sql/query_result.h"
#include "sql/rpl_rli.h"    // Relay_log_info
#include "sql/rpl_slave.h"  // rpl_master_erroneous_autoinc
#include "sql/rpl_transaction_write_set_ctx.h"
#include "sql/sp_cache.h"         // sp_cache_clear
#include "sql/sp_head.h"          // sp_head
#include "sql/sql_audit.h"        // mysql_audit_free_thd
#include "sql/sql_backup_lock.h"  // release_backup_lock
#include "sql/sql_base.h"         // close_temporary_tables
#include "sql/sql_callback.h"     // MYSQL_CALLBACK
#include "sql/sql_cmd.h"
#include "sql/sql_handler.h"  // mysql_ha_cleanup
#include "sql/sql_lex.h"
#include "sql/sql_parse.h"    // is_update_query
#include "sql/sql_plugin.h"   // plugin_thdvar_init
#include "sql/sql_prepare.h"  // Prepared_statement
#include "sql/sql_profile.h"
#include "sql/sql_thd_internal_api.h"  // thd_yield_cond
#include "sql/sql_timer.h"             // thd_timer_destroy
#include "sql/table.h"
#include "sql/tc_log.h"
#include "sql/thr_malloc.h"
#include "sql/transaction.h"  // trans_rollback
#include "sql/transaction_info.h"
#include "unsafe_string_append.h"

#include "sql/sql_admission_control.h"
#include "sql/xa.h"
#include "template_utils.h"
#include "thr_mutex.h"

class Parse_tree_root;

using std::max;
using std::min;
using std::unique_ptr;

/*
  The following is used to initialise Table_ident with a internal
  table name
*/
char empty_c_string[1] = {0}; /* used for not defined db */

LEX_STRING NULL_STR = {nullptr, 0};
LEX_CSTRING EMPTY_CSTR = {"", 0};
LEX_CSTRING NULL_CSTR = {nullptr, 0};

/* empty string */
static const std::string emptyStr = "";

const char *const THD::DEFAULT_WHERE = "field list";
extern PSI_stage_info stage_waiting_for_disk_space;

void THD::Transaction_state::backup(THD *thd) {
  this->m_sql_command = thd->lex->sql_command;
  this->m_trx = thd->get_transaction();

  thd->backup_ha_data(&this->m_ha_data);

  this->m_tx_isolation = thd->tx_isolation;
  this->m_tx_read_only = thd->tx_read_only;
  this->m_thd_option_bits = thd->variables.option_bits;
  this->m_sql_mode = thd->variables.sql_mode;
  this->m_transaction_psi = thd->m_transaction_psi;
  this->m_server_status = thd->server_status;
  this->m_in_lock_tables = thd->in_lock_tables;
  this->m_time_zone_used = thd->time_zone_used;
  this->m_transaction_rollback_request = thd->transaction_rollback_request;
}

void THD::Transaction_state::restore(THD *thd) {
  thd->set_transaction(this->m_trx);

  thd->restore_ha_data(this->m_ha_data);

  thd->tx_isolation = this->m_tx_isolation;
  thd->variables.sql_mode = this->m_sql_mode;
  thd->tx_read_only = this->m_tx_read_only;
  thd->variables.option_bits = this->m_thd_option_bits;

  thd->m_transaction_psi = this->m_transaction_psi;
  thd->server_status = this->m_server_status;
  thd->lex->sql_command = this->m_sql_command;
  thd->in_lock_tables = this->m_in_lock_tables;
  thd->time_zone_used = this->m_time_zone_used;
  thd->transaction_rollback_request = this->m_transaction_rollback_request;
}

THD::Attachable_trx::Attachable_trx(THD *thd, Attachable_trx *prev_trx)
    : m_thd(thd),
      m_reset_lex(RESET_LEX),
      m_prev_attachable_trx(prev_trx),
      m_trx_state() {
  // Save the transaction state.

  m_trx_state.backup(m_thd);

  // Save and reset query-tables-list and reset the sql-command.
  //
  // NOTE: ha_innobase::store_lock() takes the current sql-command into account.
  // It must be SQLCOM_SELECT.
  //
  // Do NOT reset LEX if we're running tests. LEX is used by SELECT statements.

  bool reset = (m_reset_lex == RESET_LEX ? true : false);
  if (DBUG_EVALUATE_IF("use_attachable_trx", false, reset)) {
    m_thd->lex->reset_n_backup_query_tables_list(
        m_trx_state.m_query_tables_list);
    m_thd->lex->sql_command = SQLCOM_SELECT;
  }

  // Save and reset open-tables.

  m_thd->reset_n_backup_open_tables_state(&m_trx_state.m_open_tables_state,
                                          Open_tables_state::SYSTEM_TABLES);

  // Reset transaction state.

  m_thd->m_transaction.release();  // it's been backed up.
  DBUG_EXECUTE_IF("after_delete_wait", {
    const char act[] = "now SIGNAL leader_reached WAIT_FOR leader_proceed";
    DBUG_ASSERT(!debug_sync_set_action(m_thd, STRING_WITH_LEN(act)));
    DBUG_SET("-d,after_delete_wait");
    DBUG_SET("-d,block_leader_after_delete");
  };);

  m_thd->m_transaction.reset(new Transaction_ctx());

  // Prepare for a new attachable transaction for read-only DD-transaction.

  // LOCK_thd_data must be locked to prevent e.g. KILL CONNECTION from
  // reading ha_data after clear() but before resize().
  mysql_mutex_lock(&m_thd->LOCK_thd_data);
  m_thd->ha_data.clear();
  m_thd->ha_data.resize(m_thd->ha_data.capacity());
  mysql_mutex_unlock(&m_thd->LOCK_thd_data);

  // The attachable transaction must used READ COMMITTED isolation level.

  m_thd->tx_isolation = ISO_READ_COMMITTED;

  // The attachable transaction must be read-only.

  m_thd->tx_read_only = true;

  // The attachable transaction must be AUTOCOMMIT.

  m_thd->variables.option_bits |= OPTION_AUTOCOMMIT;
  m_thd->variables.option_bits &= ~OPTION_NOT_AUTOCOMMIT;
  m_thd->variables.option_bits &= ~OPTION_BEGIN;

  // Nothing should be binlogged from attachable transactions and disabling
  // the binary log allows skipping some code related to figuring out what
  // log format should be used.
  m_thd->variables.option_bits &= ~OPTION_BIN_LOG;

  // Possible parent's involvement to multi-statement transaction is masked

  m_thd->server_status &= ~SERVER_STATUS_IN_TRANS;
  m_thd->server_status &= ~SERVER_STATUS_IN_TRANS_READONLY;

  // Reset SQL_MODE during system operations.

  m_thd->variables.sql_mode = 0;

  // Reset transaction instrumentation.

  m_thd->m_transaction_psi = nullptr;

  // Reset THD::in_lock_tables so InnoDB won't start acquiring table locks.
  m_thd->in_lock_tables = false;

  // Reset @@session.time_zone usage indicator for consistency.
  m_thd->time_zone_used = false;

  /*
    InnoDB can ask to start attachable transaction while rolling back
    the regular transaction. Reset rollback request flag to avoid it
    influencing attachable transaction we are initiating.
  */
  m_thd->transaction_rollback_request = false;
}

THD::Attachable_trx::~Attachable_trx() {
  // Ensure that the SE didn't request rollback in the attachable transaction.
  // Having THD::transaction_rollback_request set most likely means that we've
  // experienced some sort of deadlock/timeout while processing the attachable
  // transaction. That is not possible by the definition of an attachable
  // transaction.
  DBUG_ASSERT(!m_thd->transaction_rollback_request);

  // Commit the attachable transaction before discarding transaction state.
  // This is mostly needed to properly reset transaction state in SE.
  // Note: We can't rely on InnoDB hack which auto-magically commits InnoDB
  // transaction when the last table for a statement in auto-commit mode is
  // unlocked. Apparently it doesn't work correctly in some corner cases
  // (for example, when statement is killed just after tables are locked but
  // before any other operations on the table happes). We try not to rely on
  // it in other places on SQL-layer as well.
  trans_commit_attachable(m_thd);

  // Close all the tables that are open till now.

  close_thread_tables(m_thd);

  // Cleanup connection specific state which was created for attachable
  // transaction (for InnoDB removes cached transaction object).
  //
  // Note that we need to call handlerton::close_connection for all SEs
  // and not only SEs which participated in attachable transaction since
  // connection specific state can be created when TABLE object is simply
  // expelled from the Table Cache (e.g. this happens for MyISAM).
  ha_close_connection(m_thd);

  // Restore the transaction state.

  m_trx_state.restore(m_thd);

  m_thd->restore_backup_open_tables_state(&m_trx_state.m_open_tables_state);

  bool reset = (m_reset_lex == RESET_LEX ? true : false);
  if (DBUG_EVALUATE_IF("use_attachable_trx", false, reset)) {
    m_thd->lex->restore_backup_query_tables_list(
        m_trx_state.m_query_tables_list);
  }
}

THD::Attachable_trx_rw::Attachable_trx_rw(THD *thd)
    : Attachable_trx(thd, nullptr) {
  m_thd->tx_read_only = false;
  m_thd->lex->sql_command = SQLCOM_END;
  thd->get_transaction()->xid_state()->set_state(XID_STATE::XA_NOTR);
}

void THD::enter_stage(const PSI_stage_info *new_stage,
                      PSI_stage_info *old_stage,
                      const char *calling_func MY_ATTRIBUTE((unused)),
                      const char *calling_file,
                      const unsigned int calling_line) {
  DBUG_PRINT("THD::enter_stage",
             ("'%s' %s:%d", new_stage ? new_stage->m_name : "", calling_file,
              calling_line));

  if (old_stage != nullptr) {
    old_stage->m_key = m_current_stage_key;
    old_stage->m_name = proc_info;
  }

  if (new_stage != nullptr) {
    const char *msg = new_stage->m_name;

#if defined(ENABLED_PROFILING)
    profiling->status_change(msg, calling_func, calling_file, calling_line);
#endif

    m_current_stage_key = new_stage->m_key;
    proc_info = msg;

    m_stage_progress_psi =
        MYSQL_SET_STAGE(m_current_stage_key, calling_file, calling_line);
  } else {
    m_stage_progress_psi = nullptr;
  }

  return;
}

void Open_tables_state::set_open_tables_state(Open_tables_state *state) {
  this->open_tables = state->open_tables;

  this->temporary_tables = state->temporary_tables;

  this->lock = state->lock;
  this->extra_lock = state->extra_lock;

  this->locked_tables_mode = state->locked_tables_mode;

  this->state_flags = state->state_flags;

  this->m_reprepare_observers = state->m_reprepare_observers;
}

void Open_tables_state::reset_open_tables_state() {
  open_tables = nullptr;
  temporary_tables = nullptr;
  lock = nullptr;
  extra_lock = nullptr;
  locked_tables_mode = LTM_NONE;
  state_flags = 0U;
  reset_reprepare_observers();
}

THD::THD(bool enable_plugins, bool is_slave)
    : Query_arena(&main_mem_root, STMT_REGULAR_EXECUTION),
      mark_used_columns(MARK_COLUMNS_READ),
      want_privilege(0),
      main_lex(new LEX),
      lex(main_lex.get()),
      m_dd_client(new dd::cache::Dictionary_client(this)),
      m_query_string(NULL_CSTR),
      m_db(NULL_CSTR),
      rli_fake(nullptr),
      rli_slave(nullptr),
      initial_status_var(nullptr),
      status_var_aggregated(false),
      m_connection_attributes(),
      m_current_query_cost(0),
      m_current_query_partial_plans(0),
      m_main_security_ctx(this),
      m_security_ctx(&m_main_security_ctx),
      protocol_text(new Protocol_text),
      protocol_binary(new Protocol_binary),
      query_plan(this),
      m_current_stage_key(0),
      current_mutex(nullptr),
      current_cond(nullptr),
      m_is_admin_conn(false),
      in_sub_stmt(0),
      fill_status_recursion_level(0),
      fill_variables_recursion_level(0),
      ha_data(PSI_NOT_INSTRUMENTED, ha_data.initial_capacity),
      binlog_row_event_extra_data(nullptr),
      skip_readonly_check(false),
      binlog_unsafe_warning_flags(0),
      binlog_table_maps(0),
      binlog_accessed_db_names(nullptr),
      m_trans_log_file(nullptr),
      m_trans_fixed_log_file(nullptr),
      m_trans_fixed_log_path(nullptr),
      m_trans_end_pos(0),
      m_trans_gtid(NULL),
      m_trans_max_gtid(NULL),
      m_transaction(new Transaction_ctx()),
      m_attachable_trx(nullptr),
      table_map_for_update(0),
      m_examined_row_count(0),
      m_current_db_context(NULL),
#if defined(ENABLED_PROFILING)
      profiling(new PROFILING),
#endif
      m_stage_progress_psi(nullptr),
      m_digest(nullptr),
      m_statement_psi(nullptr),
      m_transaction_psi(nullptr),
      m_idle_psi(nullptr),
      m_server_idle(false),
      user_var_events(key_memory_user_var_entry),
      next_to_commit(nullptr),
      binlog_need_explicit_defaults_ts(false),
      kill_immunizer(nullptr),
      m_is_fatal_error(false),
      transaction_rollback_request(false),
      is_fatal_sub_stmt_error(false),
      rand_used(false),
      time_zone_used(false),
      in_lock_tables(false),
      derived_tables_processing(false),
      parsing_system_view(false),
      really_error_partial_strict(false),
      really_audit_instrumented_event(0),
      audited_event_for_command(false),
      sp_runtime_ctx(nullptr),
      m_parser_state(nullptr),
      work_part_info(nullptr),
      // No need to instrument, highly unlikely to have that many plugins.
      audit_class_plugins(PSI_NOT_INSTRUMENTED),
      audit_class_mask(PSI_NOT_INSTRUMENTED),
#if defined(ENABLED_DEBUG_SYNC)
      debug_sync_control(nullptr),
#endif /* defined(ENABLED_DEBUG_SYNC) */
      m_enable_plugins(enable_plugins),
#ifdef HAVE_GTID_NEXT_LIST
      owned_gtid_set(global_sid_map),
#endif
      skip_gtid_rollback(false),
      is_commit_in_middle_of_statement(false),
      has_gtid_consistency_violation(false),
      main_da(false),
      m_parser_da(false),
      m_query_rewrite_plugin_da(false),
      m_query_rewrite_plugin_da_ptr(&m_query_rewrite_plugin_da),
      m_stmt_da(&main_da),
      conn_timeout_err_msg(nullptr),
      duplicate_slave_id(false),
      is_a_srv_session_thd(false),
      m_is_plugin_fake_ddl(false) {
  main_lex->reset();
  set_psi(nullptr);
  mdl_context.init(this);
  init_sql_alloc(key_memory_thd_main_mem_root, &main_mem_root,
                 global_system_variables.query_alloc_block_size,
                 global_system_variables.query_prealloc_size);
  stmt_arena = this;
  thread_stack = nullptr;
  m_catalog.str = "std";
  m_catalog.length = 3;
  password = 0;
  query_start_usec_used = false;
  check_for_truncated_fields = CHECK_FIELD_IGNORE;
  killed = NOT_KILLED;
  killed_reason = NULL;
  col_access = 0;
  is_slave_error = thread_specific_used = false;
  tmp_table = 0;
  num_truncated_fields = 0L;
  m_sent_row_count = 0L;
  current_found_rows = 0;
  previous_found_rows = 0;
  is_operating_gtid_table_implicitly = false;
  is_operating_substatement_implicitly = false;
  m_row_count_func = -1;
  statement_id_counter = 0UL;
  // Must be reset to handle error with THD's created for init of mysqld
  lex->thd = nullptr;
  lex->set_current_select(nullptr);
  utime_after_lock = 0L;
  set_timespec(&start_cputime, 0);
  current_linfo = nullptr;
  slave_thread = false;
  memset(&variables, 0, sizeof(variables));
  m_thread_id = Global_THD_manager::reserved_thread_id;
  file_id = 0;
  query_id = 0;
  query_name_consts = 0;
  db_charset = global_system_variables.collation_database;
  is_killable = false;
  binlog_evt_union.do_union = false;
  enable_slow_log = false;
  commit_error = CE_NONE;
  tx_commit_pending = false;
  durability_property = HA_REGULAR_DURABILITY;
#ifndef DBUG_OFF
  dbug_sentry = THD_SENTRY_MAGIC;
#endif
  mysql_mutex_init(key_LOCK_thd_audit_data, &LOCK_thd_audit_data,
                   MY_MUTEX_INIT_FAST);
  mysql_audit_init_thd(this);
  net.vio = nullptr;
  system_thread = NON_SYSTEM_THREAD;
  cleanup_done = false;
  m_release_resources_done = false;
  peer_port = 0;  // For SHOW PROCESSLIST
  get_transaction()->m_flags.enabled = true;
  m_resource_group_ctx.m_cur_resource_group = nullptr;
  m_resource_group_ctx.m_switch_resource_group_str[0] = '\0';
  m_resource_group_ctx.m_warn = 0;

  m_db_read_only_hash.clear();
  m_db_default_collation.clear();

  mysql_mutex_init(key_LOCK_thd_data, &LOCK_thd_data, MY_MUTEX_INIT_FAST);
  mysql_mutex_init(key_LOCK_thd_query, &LOCK_thd_query, MY_MUTEX_INIT_FAST);
  mysql_mutex_init(key_LOCK_thd_sysvar, &LOCK_thd_sysvar, MY_MUTEX_INIT_FAST);
  mysql_mutex_init(key_LOCK_thd_protocol, &LOCK_thd_protocol,
                   MY_MUTEX_INIT_FAST);
  mysql_mutex_init(key_LOCK_query_plan, &LOCK_query_plan, MY_MUTEX_INIT_FAST);
  mysql_mutex_init(key_LOCK_current_cond, &LOCK_current_cond,
                   MY_MUTEX_INIT_FAST);
  mysql_mutex_init(key_LOCK_thd_db_read_only_hash, &LOCK_thd_db_read_only_hash,
                   MY_MUTEX_INIT_FAST);
  mysql_mutex_init(key_LOCK_thd_db_context, &LOCK_thd_db_context,
                   MY_MUTEX_INIT_FAST);
  mysql_mutex_init(key_LOCK_thd_db_default_collation_hash,
                   &LOCK_thd_db_default_collation_hash, MY_MUTEX_INIT_FAST);
  mysql_cond_init(key_COND_thr_lock, &COND_thr_lock);

  /* Variables with default values */
  proc_info = "login";
  where = THD::DEFAULT_WHERE;
  server_id = ::server_id;
  unmasked_server_id = server_id;
  set_command(COM_CONNECT);
  *scramble = '\0';

  /* Call to init() below requires fully initialized Open_tables_state. */
  reset_open_tables_state();

#if defined(ENABLED_PROFILING)
  profiling->set_thd(this);
#endif
  m_user_connect = nullptr;
  user_vars.clear();

  sp_proc_cache = nullptr;
  sp_func_cache = nullptr;

  /* Protocol */
  m_protocol = protocol_text.get();  // Default protocol
  protocol_text->init(this);
  protocol_binary->init(this);
  protocol_text->set_client_capabilities(0);  // minimalistic client

  init(is_slave);

  /*
    Make sure thr_lock_info_init() is called for threads which do not get
    assigned a proper thread_id value but keep using reserved_thread_id.
  */
  thr_lock_info_init(&lock_info, m_thread_id, &COND_thr_lock);

  m_internal_handler = nullptr;
  m_binlog_invoker = false;
  memset(&m_invoker_user, 0, sizeof(m_invoker_user));
  memset(&m_invoker_host, 0, sizeof(m_invoker_host));

  binlog_next_event_pos.file_name = nullptr;
  binlog_next_event_pos.pos = 0;
  trans_gtid[0] = 0;
  trans_max_gtid[0] = 0;

  timer = nullptr;
  timer_cache = nullptr;

  m_token_array = nullptr;
  if (max_digest_length > 0) {
    m_token_array = (unsigned char *)my_malloc(PSI_INSTRUMENT_ME,
                                               max_digest_length, MYF(MY_WME));
  }
#ifndef DBUG_OFF
  debug_binlog_xid_last.reset();
#endif
  set_system_user(false);
}

void THD::set_transaction(Transaction_ctx *transaction_ctx) {
  DBUG_ASSERT(is_attachable_ro_transaction_active());

  delete m_transaction.release();
  m_transaction.reset(transaction_ctx);
}

static std::string get_shard_id(const std::string &db_metadata) {
  try {
    rapidjson::Document db_metadata_root;
    // The local_db_metadata format should be:
    // {"shard":"<shard_name>", "replicaset":"<replicaset_id>"}
    if (db_metadata_root.Parse(db_metadata.c_str()).HasParseError() ||
        !db_metadata_root.IsObject()) {
      return {};
    }
    const auto iter = db_metadata_root.FindMember("shard");
    std::string shard_id;
    if (iter != db_metadata_root.MemberEnd()) {
      shard_id = iter->value.GetString();
    }
    return shard_id;
  } catch (const std::exception &) {
    return {};
  }
}

void THD::set_db_metadata() {
  if (m_db.length) {
    mysql_mutex_lock(&LOCK_thd_db_context);
    auto it = m_db_context_hash.find(m_db.str);
    if (it != m_db_context_hash.end()) {
      m_current_db_context = &it->second;
      mysql_mutex_unlock(&LOCK_thd_db_context);
      return;
    }
    mysql_mutex_unlock(&LOCK_thd_db_context);

    dd::Schema_MDL_locker mdl_handler(this);
    dd::cache::Dictionary_client::Auto_releaser releaser(dd_client());
    const dd::Schema *schema = nullptr;

    /*
      Wait forever for the lock to be acquired if needed. Otherwise
      it is possible for change database to succeed, but for the metadata
      to not be loaded. Most code does not appear to be checking
      for error return from setting the database, so either always ensure
      we have the lock before we exit, or the failure is fatal such that
      it does not matter what the metadata is.
     */
    while (mdl_handler.ensure_locked(m_db.str)) {
      /* Something is wrong/unexpected if both conditions are false */
      DBUG_ASSERT(is_error() || killed);
      if (killed || !is_error()) return;

      /*
        Should db_metadata be reset for other errors? Since the lock was not
        acquired, metadata could not be retrieved, so return immediately.
       */
      uint errcode = get_stmt_da()->mysql_errno();
      if (errcode != ER_LOCK_WAIT_TIMEOUT && errcode != ER_LOCK_DEADLOCK)
        return;

      clear_error();
    }

    /*
      acquire returns true on error, but schema could be null if database is not
      found
    */
    if (!dd_client()->acquire(m_db.str, &schema) && schema != nullptr) {
      mysql_mutex_lock(&LOCK_thd_db_context);
      db_context_t &db_context_ref = m_db_context_hash[m_db.str];
      db_context_ref.db_metadata = schema->get_db_metadata().c_str();
      db_context_ref.db_shard_id = get_shard_id(db_context_ref.db_metadata);
      m_current_db_context = &db_context_ref;
      mysql_mutex_unlock(&LOCK_thd_db_context);
      return;
    }
  }
  mysql_mutex_lock(&LOCK_thd_db_context);
  m_current_db_context = nullptr;
  mysql_mutex_unlock(&LOCK_thd_db_context);
}

bool THD::set_db(const LEX_CSTRING &new_db) {
  bool result;
  /*
    Acquiring mutex LOCK_thd_data as we either free the memory allocated
    for the database and reallocating the memory for the new db or memcpy
    the new_db to the db.
  */
  mysql_mutex_lock(&LOCK_thd_data);
  /* Do not reallocate memory if current chunk is big enough. */
  if (m_db.str && new_db.str && m_db.length >= new_db.length)
    memcpy(const_cast<char *>(m_db.str), new_db.str, new_db.length + 1);
  else {
    my_free(const_cast<char *>(m_db.str));
    m_db = NULL_CSTR;
    if (new_db.str)
      m_db.str = my_strndup(key_memory_THD_db, new_db.str, new_db.length,
                            MYF(MY_WME | ME_FATALERROR));
  }
  m_db.length = m_db.str ? new_db.length : 0;
  mysql_mutex_unlock(&LOCK_thd_data);
  set_db_metadata();
  result = new_db.str && !m_db.str;
#ifdef HAVE_PSI_THREAD_INTERFACE
  if (!result)
    PSI_THREAD_CALL(set_thread_db)(new_db.str, static_cast<int>(new_db.length));
#endif
  return result;
}

void THD::push_internal_handler(Internal_error_handler *handler) {
  if (m_internal_handler) {
    handler->m_prev_internal_handler = m_internal_handler;
    m_internal_handler = handler;
  } else
    m_internal_handler = handler;
}

bool THD::handle_condition(uint sql_errno, const char *sqlstate,
                           Sql_condition::enum_severity_level *level,
                           const char *msg) {
  if (!m_internal_handler) return false;

  for (Internal_error_handler *error_handler = m_internal_handler;
       error_handler; error_handler = error_handler->m_prev_internal_handler) {
    if (error_handler->handle_condition(this, sql_errno, sqlstate, level, msg))
      return true;
  }
  return false;
}

Internal_error_handler *THD::pop_internal_handler() {
  DBUG_ASSERT(m_internal_handler != nullptr);
  Internal_error_handler *popped_handler = m_internal_handler;
  m_internal_handler = m_internal_handler->m_prev_internal_handler;
  return popped_handler;
}

void THD::raise_error(uint sql_errno) {
  const char *msg = ER_THD_NONCONST(this, sql_errno);
  (void)raise_condition(sql_errno, nullptr, Sql_condition::SL_ERROR, msg);
}

void THD::raise_error_printf(uint sql_errno, ...) {
  va_list args;
  char ebuff[MYSQL_ERRMSG_SIZE];
  DBUG_TRACE;
  DBUG_PRINT("my", ("nr: %d  errno: %d", sql_errno, errno));
  const char *format = ER_THD_NONCONST(this, sql_errno);
  va_start(args, sql_errno);
  vsnprintf(ebuff, sizeof(ebuff), format, args);
  va_end(args);
  (void)raise_condition(sql_errno, nullptr, Sql_condition::SL_ERROR, ebuff);
}

void THD::raise_warning(uint sql_errno) {
  const char *msg = ER_THD_NONCONST(this, sql_errno);
  (void)raise_condition(sql_errno, nullptr, Sql_condition::SL_WARNING, msg);
}

void THD::raise_warning_printf(uint sql_errno, ...) {
  va_list args;
  char ebuff[MYSQL_ERRMSG_SIZE];
  DBUG_TRACE;
  DBUG_PRINT("enter", ("warning: %u", sql_errno));
  const char *format = ER_THD_NONCONST(this, sql_errno);
  va_start(args, sql_errno);
  vsnprintf(ebuff, sizeof(ebuff), format, args);
  va_end(args);
  (void)raise_condition(sql_errno, nullptr, Sql_condition::SL_WARNING, ebuff);
}

void THD::raise_note(uint sql_errno) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("code: %d", sql_errno));
  if (!(variables.option_bits & OPTION_SQL_NOTES)) return;
  const char *msg = ER_THD_NONCONST(this, sql_errno);
  (void)raise_condition(sql_errno, nullptr, Sql_condition::SL_NOTE, msg);
}

void THD::raise_note_printf(uint sql_errno, ...) {
  va_list args;
  char ebuff[MYSQL_ERRMSG_SIZE];
  DBUG_TRACE;
  DBUG_PRINT("enter", ("code: %u", sql_errno));
  if (!(variables.option_bits & OPTION_SQL_NOTES)) return;
  const char *format = ER_THD_NONCONST(this, sql_errno);
  va_start(args, sql_errno);
  vsnprintf(ebuff, sizeof(ebuff), format, args);
  va_end(args);
  (void)raise_condition(sql_errno, nullptr, Sql_condition::SL_NOTE, ebuff);
}

struct timeval THD::query_start_timeval_trunc(uint decimals) {
  struct timeval tv;
  tv.tv_sec = start_time.tv_sec;
  if (decimals) {
    tv.tv_usec = start_time.tv_usec;
    my_timeval_trunc(&tv, decimals);
    query_start_usec_used = true;
  } else {
    tv.tv_usec = 0;
  }
  return tv;
}

Sql_condition *THD::raise_condition(uint sql_errno, const char *sqlstate,
                                    Sql_condition::enum_severity_level level,
                                    const char *msg, bool fatal_error) {
  DBUG_TRACE;

  if (!(variables.option_bits & OPTION_SQL_NOTES) &&
      (level == Sql_condition::SL_NOTE))
    return nullptr;

  DBUG_ASSERT(sql_errno != 0);
  if (sql_errno == 0) /* Safety in release build */
    sql_errno = ER_UNKNOWN_ERROR;
  if (msg == nullptr) msg = ER_THD_NONCONST(this, sql_errno);
  if (sqlstate == nullptr) sqlstate = mysql_errno_to_sqlstate(sql_errno);

  if (fatal_error) {
    // Only makes sense for errors
    DBUG_ASSERT(level == Sql_condition::SL_ERROR);
    this->fatal_error();
  }

  MYSQL_LOG_ERROR(sql_errno, PSI_ERROR_OPERATION_RAISED);
  if (handle_condition(sql_errno, sqlstate, &level, msg)) return nullptr;

  Diagnostics_area *da = get_stmt_da();
  if (level == Sql_condition::SL_ERROR) {
    /*
      Reporting an error invokes audit API call that notifies the error
      to the plugin. Audit API that generate the error adds a protection
      (condition handler) that prevents entering infinite recursion, when
      a plugin signals error, when already handling the error.

      mysql_audit_notify() must therefore be called after handle_condition().
    */
    mysql_audit_notify(this, AUDIT_EVENT(MYSQL_AUDIT_GENERAL_ERROR), sql_errno,
                       msg, strlen(msg));

    is_slave_error = true;  // needed to catch query errors during replication

    if (!da->is_error()) {
      set_row_count_func(-1);
      da->set_error_status(sql_errno, msg, sqlstate);
    }
  }

  /*
    Avoid pushing a condition for fatal out of memory errors as this will
    require memory allocation and therefore might fail. Non fatal out of
    memory errors can occur if raised by SIGNAL/RESIGNAL statement.
  */
  Sql_condition *cond = nullptr;
  if (!(is_fatal_error() &&
        (sql_errno == EE_OUTOFMEMORY || sql_errno == ER_OUTOFMEMORY ||
         sql_errno == ER_STD_BAD_ALLOC_ERROR))) {
    cond = da->push_warning(this, sql_errno, sqlstate, level, msg);
  }
  return cond;
}

void THD::fix_capability_based_variables() {
  if (m_protocol->has_client_capability(CLIENT_INTERACTIVE))
    variables.net_wait_timeout = variables.net_interactive_timeout;
  if (m_protocol->has_client_capability(CLIENT_IGNORE_SPACE))
    variables.sql_mode |= MODE_IGNORE_SPACE;
}

/*
  Init common variables that has to be reset on start and on cleanup_connection
*/

void THD::init(bool is_slave) {
  plugin_thdvar_init(this, m_enable_plugins);
  /*
    variables= global_system_variables above has reset
    variables.pseudo_thread_id to 0. We need to correct it here to
    avoid temporary tables replication failure.
  */
  variables.pseudo_thread_id = m_thread_id;
  /*
   variables= global_system_variables also clobbers several variables set
   based on per-connection capabilities
   */
  fix_capability_based_variables();

  /*
    NOTE: reset_connection command will reset the THD to its default state.
    All system variables whose scope is SESSION ONLY should be set to their
    default values here.
  */
  reset_first_successful_insert_id();
  user_time.tv_sec = user_time.tv_usec = 0;
  start_time.tv_sec = start_time.tv_usec = 0;
  set_time();
  auto_inc_intervals_forced.empty();
  {
    ulong tmp;
    tmp = sql_rnd_with_mutex();
    randominit(&rand, tmp + (ulong)&rand,
               tmp + (ulong)::atomic_global_query_id);
  }

  server_status = SERVER_STATUS_AUTOCOMMIT;
  if (variables.sql_mode & MODE_NO_BACKSLASH_ESCAPES)
    server_status |= SERVER_STATUS_NO_BACKSLASH_ESCAPES;

  get_transaction()->reset_unsafe_rollback_flags(Transaction_ctx::SESSION);
  get_transaction()->reset_unsafe_rollback_flags(Transaction_ctx::STMT);
  open_options = ha_open_options;
  update_lock_default =
      (variables.low_priority_updates ? TL_WRITE_LOW_PRIORITY : TL_WRITE);
  insert_lock_default =
      (variables.low_priority_updates ? TL_WRITE_LOW_PRIORITY
                                      : TL_WRITE_CONCURRENT_INSERT);

  if (is_slave) {
    variables.transaction_isolation =
        static_cast<enum_tx_isolation>(slave_tx_isolation);
  }

  tx_isolation = (enum_tx_isolation)variables.transaction_isolation;
  tx_read_only = variables.transaction_read_only;
  tx_priority = 0;
  thd_tx_priority = 0;
  update_charset();
  reset_current_stmt_binlog_format_row();
  reset_binlog_local_stmt_filter();
  reset_system_status_vars(&status_var);
  binlog_row_event_extra_data = nullptr;

  if (variables.sql_log_bin)
    variables.option_bits |= OPTION_BIN_LOG;
  else
    variables.option_bits &= ~OPTION_BIN_LOG;

#if defined(ENABLED_DEBUG_SYNC)
  /* Initialize the Debug Sync Facility. See debug_sync.cc. */
  debug_sync_init_thread(this);
#endif /* defined(ENABLED_DEBUG_SYNC) */

  /* Initialize session_tracker and create all tracker objects */
  session_tracker.init(this->charset());
  session_tracker.enable(this);

  owned_gtid.clear();
  owned_sid.clear();
  m_se_gtid_flags.reset();
  owned_gtid.dbug_print(nullptr, "set owned_gtid (clear) in THD::init");

  // This will clear the writeset session history.
  rpl_thd_ctx.dependency_tracker_ctx().set_last_session_sequence_number(0);

  /*
    This variable is used to temporarily disable the password validation plugin
    when a RANDOM PASSWORD is generated during SET PASSWORD,CREATE USER or
    ALTER USER statements.
  */
  m_disable_password_validation = false;

  mt_key_clear(THD::SQL_ID);
  mt_key_clear(THD::SQL_HASH);
  mt_key_clear(THD::CLIENT_ID);
  reset_stmt_stats();
  set_trx_dml_row_count(0);
  set_trx_dml_cpu_time_limit_warning(false);
  m_trx_dml_start_time_is_set = false;
}

/**
  Reset statement stats counters before next statement.
*/
void THD::reset_stmt_stats() {
  m_binlog_bytes_written = 0; /* binlog bytes written */
  m_stmt_total_write_time = 0;
  m_stmt_start_write_time_is_set = false;

  /* The disk usage of a single statement is the difference between the peak
     session usage during the statement execution and the session usage at
     the start of the statement. So remember disk usage at the start and
     use it to offset the peak. */
  m_stmt_filesort_disk_usage_offset = m_filesort_disk_usage;
  m_stmt_tmp_table_disk_usage_offset = m_tmp_table_disk_usage;
}

void THD::init_query_mem_roots() {
  mem_root->set_block_size(variables.query_alloc_block_size);
  get_transaction()->init_mem_root_defaults(variables.trans_alloc_block_size,
                                            variables.trans_prealloc_size);
}

void THD::set_new_thread_id() {
  m_thread_id = Global_THD_manager::get_instance()->get_new_thread_id();
  variables.pseudo_thread_id = m_thread_id;
  thr_lock_info_init(&lock_info, m_thread_id, &COND_thr_lock);
}

/*
  Do what's needed when one invokes change user

  SYNOPSIS
    cleanup_connection()

  IMPLEMENTATION
    Reset all resources that are connection specific
*/

void THD::cleanup_connection(void) {
  cleanup();
  killed = NOT_KILLED;
  cleanup_done = false;

  /* Aggregate to global status now that cleanup is done. */
  mysql_mutex_lock(&LOCK_status);
  add_to_status(&global_status_var, &status_var);
  reset_system_status_vars(&status_var);
  mysql_mutex_unlock(&LOCK_status);

  propagate_pending_global_disk_usage();

#if defined(ENABLED_DEBUG_SYNC)
  /* End the Debug Sync Facility. See debug_sync.cc. */
  debug_sync_end_thread(this);
#endif /* defined(ENABLED_DEBUG_SYNC) */
  running_explain_analyze = false;
  init(/* is_slave = */ false);
  stmt_map.reset();
  user_vars.clear();
  sp_cache_clear(&sp_proc_cache);
  sp_cache_clear(&sp_func_cache);

  reset_connection_certificate();

  clear_error();
  // clear the warnings
  get_stmt_da()->reset_condition_info(this);
  // clear profiling information
#if defined(ENABLED_PROFILING)
  profiling->cleanup();
#endif

#ifndef DBUG_OFF
  /* DEBUG code only (begin) */
  bool check_cleanup = false;
  DBUG_EXECUTE_IF("debug_test_cleanup_connection", check_cleanup = true;);
  if (check_cleanup) {
    /* isolation level should be default */
    DBUG_ASSERT(variables.transaction_isolation == ISO_REPEATABLE_READ);
    /* check autocommit is ON by default */
    DBUG_ASSERT(server_status == SERVER_STATUS_AUTOCOMMIT);
    /* check prepared stmts are cleaned up */
    DBUG_ASSERT(prepared_stmt_count == 0);
    /* check diagnostic area is cleaned up */
    DBUG_ASSERT(get_stmt_da()->status() == Diagnostics_area::DA_EMPTY);
    /* check if temp tables are deleted */
    DBUG_ASSERT(temporary_tables == nullptr);
    /* check if tables are unlocked */
    DBUG_ASSERT(locked_tables_list.locked_tables() == nullptr);
  }
  /* DEBUG code only (end) */
#endif
}

std::string THD::extract_peer_certificate_info(const THD *thd, bool printable) {
  // Extracting user certificate from the thread
  if (!thd->has_net_vio_ssl_arg()) return {};

  auto ssl = static_cast<const SSL *>(thd->get_net_vio_ssl_arg());

  // Creating new X509 abstraction
  auto cert_deleter = [](X509 *cert) {
    if (cert != nullptr) X509_free(cert);
  };
  using x509_ptr = std::unique_ptr<X509, decltype(cert_deleter)>;

  x509_ptr cert{SSL_get_peer_certificate(ssl), cert_deleter};
  if (!cert) return {};

  // Creating new memory-based BIO object
  auto bio_deleter = [](BIO *bio) {
    if (bio != nullptr) BIO_free(bio);
  };
  using bio_ptr = std::unique_ptr<BIO, decltype(bio_deleter)>;

  bio_ptr bio{BIO_new(BIO_s_mem()), bio_deleter};
  if (!bio) return {};

  // Printing the certificate to the bio object
  int print_result = 0;
  if (printable)
    print_result = X509_print(bio.get(), cert.get());
  else
    print_result = PEM_write_bio_X509(bio.get(), cert.get());
  if (print_result != 1) return {};

  // Extracting data from the bio object
  BUF_MEM *buf_mem;
  BIO_get_mem_ptr(bio.get(), &buf_mem);
  assert(buf_mem->length <= buf_mem->max);
  if (buf_mem->data == nullptr) return {};

  return std::string{buf_mem->data, buf_mem->length};
}

/*
  Do what's needed when one invokes change user.
  Also used during THD::release_resources, i.e. prior to THD destruction.
*/
void THD::cleanup(void) {
  Transaction_ctx *trn_ctx = get_transaction();
  XID_STATE *xs = trn_ctx->xid_state();

  DBUG_TRACE;
  DBUG_ASSERT(cleanup_done == 0);
  DEBUG_SYNC(this, "thd_cleanup_start");

  killed = KILL_CONNECTION;
  if (trn_ctx->xid_state()->has_state(XID_STATE::XA_PREPARED)) {
    /*
      Return error is not an option as XA is in prepared state and
      connection is gone. Log the error and continue.
    */
    if (MDL_context_backup_manager::instance().create_backup(
            &mdl_context, xs->get_xid()->key(), xs->get_xid()->key_length())) {
      LogErr(ERROR_LEVEL, ER_XA_CANT_CREATE_MDL_BACKUP);
    }
    transaction_cache_detach(trn_ctx);
  } else {
    xs->set_state(XID_STATE::XA_NOTR);
    trans_rollback(this);
    transaction_cache_delete(trn_ctx);
  }

  locked_tables_list.unlock_locked_tables(this);
  mysql_ha_cleanup(this);

  DBUG_ASSERT(open_tables == nullptr);
  /*
    If the thread was in the middle of an ongoing transaction (rolled
    back a few lines above) or under LOCK TABLES (unlocked the tables
    and left the mode a few lines above), there will be outstanding
    metadata locks. Release them.
  */
  mdl_context.release_transactional_locks();

  /* Release the global read lock, if acquired. */
  if (global_read_lock.is_acquired())
    global_read_lock.unlock_global_read_lock(this);

  mysql_ull_cleanup(this);
  /*
    All locking service locks must be released on disconnect.
  */
  release_all_locking_service_locks(this);

  /*
    If Backup Lock was acquired it must be released on disconnect.
  */
  release_backup_lock(this);

  /* All metadata locks must have been released by now. */
  DBUG_ASSERT(!mdl_context.has_locks());

  /* Protects user_vars. */
  mysql_mutex_lock(&LOCK_thd_data);
  user_vars.clear();
  mysql_mutex_unlock(&LOCK_thd_data);

  /*
    When we call drop table for temporary tables, the
    user_var_events container is not cleared this might
    cause error if the container was filled before the
    drop table command is called.
    So call this before calling close_temporary_tables.
  */
  user_var_events.clear();
  close_temporary_tables(this);
  sp_cache_clear(&sp_proc_cache);
  sp_cache_clear(&sp_func_cache);

  /*
    Actions above might generate events for the binary log, so we
    commit the current transaction coordinator after executing cleanup
    actions.
   */
  if (tc_log && !trn_ctx->xid_state()->has_state(XID_STATE::XA_PREPARED))
    tc_log->commit(this, true);

  /*
    Destroy trackers only after finishing manipulations with transaction
    state to avoid issues with Transaction_state_tracker.
  */
  session_tracker.deinit();

  /*
    If we have a Security_context, make sure it is "logged out"
  */

  cleanup_done = true;
}

/**
  Release most resources, prior to THD destruction.
 */
void THD::release_resources() {
  DBUG_ASSERT(m_release_resources_done == false);

  Global_THD_manager::get_instance()->release_thread_id(m_thread_id);

  /* Ensure that no one is using THD */
  mysql_mutex_lock(&LOCK_thd_data);
  mysql_mutex_lock(&LOCK_query_plan);

  if (m_explicit_snapshot) {
    set_explicit_snapshot(nullptr);
  }

  /* if we are still in admission control, release it */
  if (is_in_ac) {
    multi_tenancy_exit_query(this);
  }

  /* Close connection */
  if (is_classic_protocol() && get_protocol_classic()->get_vio()) {
    vio_delete(get_protocol_classic()->get_vio());
    get_protocol_classic()->end_net();
  }

  /* Free nonsuper connection reference if needed */
  remove_nonsuper_connections_ref(true);

  /* modification plan for UPDATE/DELETE should be freed. */
  DBUG_ASSERT(query_plan.get_modification_plan() == nullptr);
  mysql_mutex_unlock(&LOCK_query_plan);
  mysql_mutex_unlock(&LOCK_thd_data);
  mysql_mutex_lock(&LOCK_thd_query);
  mysql_mutex_unlock(&LOCK_thd_query);

  stmt_map.reset(); /* close all prepared statements */
  if (!cleanup_done) cleanup();

  mdl_context.destroy();
  ha_close_connection(this);

  /*
    Debug sync system must be closed after ha_close_connection, because
    DEBUG_SYNC is used in InnoDB connection handlerton close.
  */
#if defined(ENABLED_DEBUG_SYNC)
  /* End the Debug Sync Facility. See debug_sync.cc. */
  debug_sync_end_thread(this);
#endif /* defined(ENABLED_DEBUG_SYNC) */

  plugin_thdvar_cleanup(this, m_enable_plugins);

  DBUG_ASSERT(timer == nullptr);

  if (timer_cache) thd_timer_destroy(timer_cache);

  if (rli_fake) {
    rli_fake->end_info();
    delete rli_fake;
    rli_fake = nullptr;
  }
  mysql_audit_free_thd(this);

  if (current_thd == this) restore_globals();

  reset_connection_certificate();

  mysql_mutex_lock(&LOCK_status);
  /* Add thread status to the global totals. */
  add_to_status(&global_status_var, &status_var);
#ifdef HAVE_PSI_THREAD_INTERFACE
  /* Aggregate thread status into the Performance Schema. */
  if (m_psi != nullptr) {
    PSI_THREAD_CALL(aggregate_thread_status)(m_psi);
  }
#endif /* HAVE_PSI_THREAD_INTERFACE */
  /* Ensure that the thread status is not re-aggregated to the global totals. */
  status_var_aggregated = true;

  mysql_mutex_unlock(&LOCK_status);

  propagate_pending_global_disk_usage();

  m_release_resources_done = true;
}

THD::~THD() {
  THD_CHECK_SENTRY(this);
  DBUG_TRACE;
  DBUG_PRINT("info", ("THD dtor, this %p", this));

  if (!m_release_resources_done) release_resources();

  DBUG_ASSERT(nonsuper_ref == 0);

  clear_next_event_pos();

  /* Ensure that no one is using THD */
  mysql_mutex_lock(&LOCK_thd_data);
  mysql_mutex_unlock(&LOCK_thd_data);
  mysql_mutex_lock(&LOCK_thd_query);
  mysql_mutex_unlock(&LOCK_thd_query);
  mysql_mutex_lock(&LOCK_thd_db_read_only_hash);
  m_db_read_only_hash.clear();
  mysql_mutex_unlock(&LOCK_thd_db_read_only_hash);

  mysql_mutex_lock(&LOCK_thd_db_default_collation_hash);
  m_db_default_collation.clear();
  mysql_mutex_unlock(&LOCK_thd_db_default_collation_hash);

  DBUG_ASSERT(!m_attachable_trx);

  my_free(const_cast<char *>(m_db.str));
  m_db = NULL_CSTR;
  get_transaction()->free_memory(MYF(0));
  mysql_mutex_destroy(&LOCK_query_plan);
  mysql_mutex_destroy(&LOCK_thd_data);
  mysql_mutex_destroy(&LOCK_thd_query);
  mysql_mutex_destroy(&LOCK_thd_sysvar);
  mysql_mutex_destroy(&LOCK_thd_protocol);
  mysql_mutex_destroy(&LOCK_current_cond);
  mysql_mutex_destroy(&LOCK_thd_db_read_only_hash);
  mysql_mutex_destroy(&LOCK_thd_db_context);
  mysql_mutex_destroy(&LOCK_thd_audit_data);
  mysql_mutex_destroy(&LOCK_thd_db_default_collation_hash);
  mysql_cond_destroy(&COND_thr_lock);
#ifndef DBUG_OFF
  dbug_sentry = THD_SENTRY_GONE;
#endif

  if (variables.gtid_next_list.gtid_set != nullptr) {
#ifdef HAVE_GTID_NEXT_LIST
    delete variables.gtid_next_list.gtid_set;
    variables.gtid_next_list.gtid_set = NULL;
    variables.gtid_next_list.is_non_null = false;
#else
    DBUG_ASSERT(0);
#endif
  }
  if (rli_slave) rli_slave->cleanup_after_session();

  /*
    As slaves can be added in one mysql command like COM_REGISTER_SLAVE
    but then need to be removed on error scenarios, we call this method
    here
  */
  unregister_slave(this, true, true);

  free_root(&main_mem_root, MYF(0));

  if (m_token_array != nullptr) {
    my_free(m_token_array);
  }
  if (killed_reason != NULL) {
    my_free(killed_reason);
  }
}

/**
  Awake a thread.

  @param[in]  state_to_set    value for THD::killed

  This is normally called from another thread's THD object.

  @note Do always call this while holding LOCK_thd_data.
*/

void THD::awake(THD::killed_state state_to_set, const char *reason) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("this: %p current_thd: %p", this, current_thd));
  THD_CHECK_SENTRY(this);
  mysql_mutex_assert_owner(&LOCK_thd_data);

  /* Shutdown clone vio always, to wake up clone waiting for remote. */
  shutdown_clone_vio();

  /*
    If THD is in kill immune mode (i.e. operation on new DD tables is in
    progress) then just save state_to_set with THD::kill_immunizer object.

    While exiting kill immune mode, awake() is called again with the killed
    state saved in THD::kill_immunizer object.
  */
  if (kill_immunizer && kill_immunizer->is_active()) {
    kill_immunizer->save_killed_state(state_to_set);
    return;
  }

  /*
    Set killed flag if the connection is being killed (state_to_set
    is KILL_CONNECTION) or the connection is processing a query
    (state_to_set is KILL_QUERY and m_server_idle flag is not set).
    If the connection is idle and state_to_set is KILL QUERY, the
    the killed flag is not set so that it doesn't affect the next
    command incorrectly.
  */
  if (this->m_server_idle && state_to_set == KILL_QUERY) { /* nothing */
  } else {
    killed = state_to_set;
    if (reason) {
      static constexpr int len = KILLED_REASON_MAX_LEN;
      if (!killed_reason) {
        killed_reason = (char *)my_malloc(PSI_NOT_INSTRUMENTED, len, MYF(0));
      }
      if (killed_reason) {
        strncpy(killed_reason, reason, len - 1);
        killed_reason[len - 1] = '\0';
      }
    } else {
      if (killed_reason && killed_reason[0] != '\0') {
        // no reason is given, let's clean up previous killed_reason
        killed_reason[0] = '\0';
      }
    }
  }

  if (state_to_set != THD::KILL_QUERY && state_to_set != THD::KILL_TIMEOUT) {
    if (this != current_thd || kill_immunizer) {
      DBUG_ASSERT(!kill_immunizer || !kill_immunizer->is_active());

      /*
        Before sending a signal, let's close the socket of the thread
        that is being killed ("this", which is not the current thread).
        This is to make sure it does not block if the signal is lost.
        This needs to be done only on platforms where signals are not
        a reliable interruption mechanism.

        Note that the downside of this mechanism is that we could close
        the connection while "this" target thread is in the middle of
        sending a result to the application, thus violating the client-
        server protocol.

        On the other hand, without closing the socket we have a race
        condition. If "this" target thread passes the check of
        thd->killed, and then the current thread runs through
        THD::awake(), sets the 'killed' flag and completes the
        signaling, and then the target thread runs into read(), it will
        block on the socket. As a result of the discussions around
        Bug#37780, it has been decided that we accept the race
        condition. A second KILL awakes the target from read().

        If we are killing ourselves, we know that we are not blocked.
        We also know that we will check thd->killed before we go for
        reading the next statement.
      */

      shutdown_active_vio();
    }

    /* Send an event to the scheduler that a thread should be killed. */
    if (!slave_thread)
      MYSQL_CALLBACK(Connection_handler_manager::event_functions,
                     post_kill_notification, (this));
  }

  /* Interrupt target waiting inside a storage engine. */
  if (state_to_set != THD::NOT_KILLED) ha_kill_connection(this);

  if (state_to_set == THD::KILL_TIMEOUT) {
    DBUG_ASSERT(!status_var_aggregated);
    status_var.max_execution_time_exceeded++;
  }

  /* Broadcast a condition to kick the target if it is waiting on it. */
  if (is_killable) {
    mysql_mutex_lock(&LOCK_current_cond);
    /*
      This broadcast could be up in the air if the victim thread
      exits the cond in the time between read and broadcast, but that is
      ok since all we want to do is to make the victim thread get out
      of waiting on current_cond.
      If we see a non-zero current_cond: it cannot be an old value (because
      then exit_cond() should have run and it can't because we have mutex); so
      it is the true value but maybe current_mutex is not yet non-zero (we're
      in the middle of enter_cond() and there is a "memory order
      inversion"). So we test the mutex too to not lock 0.

      Note that there is a small chance we fail to kill. If victim has locked
      current_mutex, but hasn't yet entered enter_cond() (which means that
      current_cond and current_mutex are 0), then the victim will not get
      a signal and it may wait "forever" on the cond (until
      we issue a second KILL or the status it's waiting for happens).
      It's true that we have set its thd->killed but it may not
      see it immediately and so may have time to reach the cond_wait().

      However, where possible, we test for killed once again after
      enter_cond(). This should make the signaling as safe as possible.
      However, there is still a small chance of failure on platforms with
      instruction or memory write reordering.
    */
    if (current_cond.load() && current_mutex.load()) {
      DBUG_EXECUTE_IF("before_dump_thread_acquires_current_mutex", {
        const char act[] =
            "now signal dump_thread_signal wait_for go_dump_thread";
        DBUG_ASSERT(!debug_sync_set_action(current_thd, STRING_WITH_LEN(act)));
      };);
      mysql_mutex_lock(current_mutex);
      mysql_cond_broadcast(current_cond);
      mysql_mutex_unlock(current_mutex);
    }
    mysql_mutex_unlock(&LOCK_current_cond);
  }
}

/**
  Close the Vio associated this session.

  @remark LOCK_thd_data is taken due to the fact that
          the Vio might be disassociated concurrently.
*/

void THD::disconnect(bool server_shutdown) {
  Vio *vio = nullptr;

  mysql_mutex_lock(&LOCK_thd_data);

  /* Shutdown clone vio always, to wake up clone waiting for remote. */
  shutdown_clone_vio();

  /*
    If thread is in kill immune mode (i.e. operation on new DD tables
    is in progress) then just save state_to_set with THD::kill_immunizer
    object.

    While exiting kill immune mode, awake() is called again with the killed
    state saved in THD::kill_immunizer object.

    active_vio is aleady associated to the thread when it is in the kill
    immune mode. THD::awake() closes the active_vio.
   */
  if (kill_immunizer != nullptr)
    kill_immunizer->save_killed_state(THD::KILL_CONNECTION);
  else {
    killed = THD::KILL_CONNECTION;

    /*
      Since a active vio might might have not been set yet, in
      any case save a reference to avoid closing a inexistent
      one or closing the vio twice if there is a active one.
    */
    vio = active_vio;
    shutdown_active_vio();

    /* Disconnect even if a active vio is not associated. */
    if (is_classic_protocol() && get_protocol_classic()->get_vio() != vio &&
        get_protocol_classic()->connection_alive()) {
      m_protocol->shutdown(server_shutdown);
    }
  }

  mysql_mutex_unlock(&LOCK_thd_data);
}

void THD::notify_shared_lock(MDL_context_owner *ctx_in_use,
                             bool needs_thr_lock_abort) {
  THD *in_use = ctx_in_use->get_thd();

  if (needs_thr_lock_abort) {
    mysql_mutex_lock(&in_use->LOCK_thd_data);
    for (TABLE *thd_table = in_use->open_tables; thd_table;
         thd_table = thd_table->next) {
      /*
        Check for TABLE::needs_reopen() is needed since in some places we call
        handler::close() for table instance (and set TABLE::db_stat to 0)
        and do not remove such instances from the THD::open_tables
        for some time, during which other thread can see those instances
        (e.g. see partitioning code).
      */
      if (!thd_table->needs_reopen())
        mysql_lock_abort_for_thread(this, thd_table);
    }
    mysql_mutex_unlock(&in_use->LOCK_thd_data);
  }
}

bool THD::kill_shared_lock(MDL_context_owner *ctx_in_use) {
  bool result = false;
  THD *in_use = ctx_in_use->get_thd();

  // Only allow super user with ddl command to kill blocking threads
  if (m_security_ctx->master_access() & SUPER_ACL) {
    bool is_high_priority_ddl = (variables.high_priority_ddl ||
                                 (slave_thread && slave_high_priority_ddl)) &&
                                support_high_priority(lex->sql_command);

    if (is_high_priority_ddl || variables.kill_conflicting_connections) {
      mysql_mutex_lock(&in_use->LOCK_thd_data);
      /*
        process the kill only if thread is not already undergoing any kill
        connection.
      */
      if (in_use->killed != THD::KILL_CONNECTION) {
        in_use->awake(THD::KILL_CONNECTION);
      }
      mysql_mutex_unlock(&in_use->LOCK_thd_data);
      result = true;
    }
  }
  return result;
}

/*
  Remember the location of thread info, the structure needed for
  (*THR_MALLOC)->Alloc() and the structure for the net buffer
*/

void THD::store_globals() {
  /*
    Assert that thread_stack is initialized: it's necessary to be able
    to track stack overrun.
  */
  DBUG_ASSERT(thread_stack);

  current_thd = this;
  THR_MALLOC = &mem_root;
  /*
    is_killable is concurrently readable by a killer thread.
    It is protected by LOCK_thd_data, it is not needed to lock while the
    value is changing from false not true. If the kill thread reads
    true we need to ensure that the thread doesn't proceed to assign
    another thread to the same TLS reference.
  */
  is_killable = true;
#ifndef DBUG_OFF
  /*
    Let mysqld define the thread id (not mysys)
    This allows us to move THD to different threads if needed.
  */
  set_my_thread_var_id(m_thread_id);
#endif
  real_id = my_thread_self();  // For debugging

  capture_system_thread_id();
}

/*
  Remove the thread specific info (THD and mem_root pointer) stored during
  store_global call for this thread.
*/
void THD::restore_globals() {
  /*
    Assert that thread_stack is initialized: it's necessary to be able
    to track stack overrun.
  */
  DBUG_ASSERT(thread_stack);

  /* Undocking the thread specific data. */
  current_thd = nullptr;
  THR_MALLOC = nullptr;
}

/*
  Cleanup after query.

  SYNOPSIS
    THD::cleanup_after_query()

  DESCRIPTION
    This function is used to reset thread data to its default state.

  NOTE
    This function is not suitable for setting thread data to some
    non-default values, as there is only one replication thread, so
    different master threads may overwrite data of each other on
    slave.
*/

void THD::cleanup_after_query() {
  /*
    Reset rand_used so that detection of calls to rand() will save random
    seeds if needed by the slave.

    Do not reset rand_used if inside a stored function or trigger because
    only the call to these operations is logged. Thus only the calling
    statement needs to detect rand() calls made by its substatements. These
    substatements must not set rand_used to 0 because it would remove the
    detection of rand() by the calling statement.
  */
  if (!in_sub_stmt) /* stored functions and triggers are a special case */
  {
    /* Forget those values, for next binlogger: */
    stmt_depends_on_first_successful_insert_id_in_prev_stmt = false;
    auto_inc_intervals_in_cur_stmt_for_binlog.empty();
    rand_used = false;
    binlog_accessed_db_names = nullptr;

    /*
      Clean possible unused INSERT_ID events by current statement.
      is_update_query() is needed to ignore SET statements:
        Statements that don't update anything directly and don't
        used stored functions. This is mostly necessary to ignore
        statements in binlog between SET INSERT_ID and DML statement
        which is intended to consume its event (there can be other
        SET statements between them).
    */
    if ((rli_slave || rli_fake) && is_update_query(lex->sql_command))
      auto_inc_intervals_forced.empty();
  }

  /*
    In case of stored procedures, stored functions, triggers and events
    m_trans_fixed_log_file will not be set to NULL. The memory will be reused.
  */
  if (!sp_runtime_ctx)
    m_trans_fixed_log_file = m_trans_fixed_log_path = nullptr;

  /*
    Forget the binlog stmt filter for the next query.
    There are some code paths that:
    - do not call THD::decide_logging_format()
    - do call THD::binlog_query(),
    making this reset necessary.
  */
  reset_binlog_local_stmt_filter();
  if (first_successful_insert_id_in_cur_stmt > 0) {
    /* set what LAST_INSERT_ID() will return */
    first_successful_insert_id_in_prev_stmt =
        first_successful_insert_id_in_cur_stmt;
    first_successful_insert_id_in_cur_stmt = 0;
  }
  arg_of_last_insert_id_function = false;
  /* Hack for cleaning up view security contexts */
  List_iterator<Security_context> it(m_view_ctx_list);
  while (Security_context *ctx = it++) {
    ctx->logout();
  }
  m_view_ctx_list.empty();
  /* Free Items that were created during this execution */
  free_items();
  /* Reset where. */
  where = THD::DEFAULT_WHERE;
  /* reset table map for multi-table update */
  table_map_for_update = 0;
  m_binlog_invoker = false;
  /* reset replication info structure */
  if (lex) {
    lex->mi.repl_ignore_server_ids.clear();
  }
  if (rli_slave) rli_slave->cleanup_after_query();
  // Set the default "cute" mode for the execution environment:
  check_for_truncated_fields = CHECK_FIELD_IGNORE;
}

/*
  Convert a string to another character set

  @param to             Store new allocated string here
  @param to_cs          New character set for allocated string
  @param from           String to convert
  @param from_length    Length of string to convert
  @param from_cs        Original character set
  @param report_error   Raise error (when true) or warning (when false) if
                        there is problem when doing conversion

  @note to will be 0-terminated to make it easy to pass to system funcs

  @retval false ok
  @retval true  End of memory.
                In this case to->str will point to 0 and to->length will be 0.
*/

bool THD::convert_string(LEX_STRING *to, const CHARSET_INFO *to_cs,
                         const char *from, size_t from_length,
                         const CHARSET_INFO *from_cs, bool report_error) {
  DBUG_TRACE;
  size_t new_length = to_cs->mbmaxlen * from_length;
  if (!(to->str = (char *)alloc(new_length + 1))) {
    to->length = 0;  // Safety fix
    return true;     // EOM
  }
  uint errors = 0;
  to->length = copy_and_convert(to->str, new_length, to_cs, from, from_length,
                                from_cs, &errors);
  to->str[to->length] = 0;  // Safety
  if (errors != 0) {
    char printable_buff[32];
    convert_to_printable(printable_buff, sizeof(printable_buff), from,
                         from_length, from_cs, 6);
    if (report_error) {
      my_error(ER_CANNOT_CONVERT_STRING, MYF(0), printable_buff,
               from_cs->csname, to_cs->csname);
      return true;
    } else {
      push_warning_printf(this, Sql_condition::SL_WARNING,
                          ER_INVALID_CHARACTER_STRING,
                          ER_THD(this, ER_CANNOT_CONVERT_STRING),
                          printable_buff, from_cs->csname, to_cs->csname);
    }
  }

  return false;
}

/*
  Update some cache variables when character set changes
*/

void THD::update_charset() {
  size_t not_used;
  charset_is_system_charset = !String::needs_conversion(
      0, variables.character_set_client, system_charset_info, &not_used);
  charset_is_collation_connection =
      !String::needs_conversion(0, variables.character_set_client,
                                variables.collation_connection, &not_used);
  charset_is_character_set_filesystem =
      !String::needs_conversion(0, variables.character_set_client,
                                variables.character_set_filesystem, &not_used);
}

int THD::send_explain_fields(Query_result *result) {
  List<Item> field_list;
  Item *item;
  CHARSET_INFO *cs = system_charset_info;
  field_list.push_back(new Item_return_int("id", 3, MYSQL_TYPE_LONGLONG));
  field_list.push_back(new Item_empty_string("select_type", 19, cs));
  field_list.push_back(item =
                           new Item_empty_string("table", NAME_CHAR_LEN, cs));
  item->maybe_null = true;
  /* Maximum length of string that make_used_partitions_str() can produce */
  item = new Item_empty_string("partitions", MAX_PARTITIONS * (1 + FN_LEN), cs);
  field_list.push_back(item);
  item->maybe_null = true;
  field_list.push_back(item = new Item_empty_string("type", 10, cs));
  item->maybe_null = true;
  field_list.push_back(item = new Item_empty_string(
                           "possible_keys", NAME_CHAR_LEN * MAX_KEY, cs));
  item->maybe_null = true;
  field_list.push_back(item = new Item_empty_string("key", NAME_CHAR_LEN, cs));
  item->maybe_null = true;
  field_list.push_back(
      item = new Item_empty_string("key_len", NAME_CHAR_LEN * MAX_KEY));
  item->maybe_null = true;
  field_list.push_back(
      item = new Item_empty_string("ref", NAME_CHAR_LEN * MAX_REF_PARTS, cs));
  item->maybe_null = true;
  field_list.push_back(
      item = new Item_return_int("rows", 10, MYSQL_TYPE_LONGLONG));
  item->maybe_null = true;
  field_list.push_back(
      item = new Item_float(NAME_STRING("filtered"), 0.1234, 2, 4));
  item->maybe_null = true;
  field_list.push_back(new Item_empty_string("Extra", 255, cs));
  item->maybe_null = true;
  return (result->send_result_set_metadata(
      this, field_list, Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF));
}

enum_vio_type THD::get_vio_type() const {
  DBUG_TRACE;
  return get_protocol()->connection_type();
}

void THD::shutdown_active_vio() {
  DBUG_TRACE;
  mysql_mutex_assert_owner(&LOCK_thd_data);
  if (active_vio) {
    vio_shutdown(active_vio);
    active_vio = nullptr;
    m_SSL = nullptr;
  }
}

void THD::shutdown_clone_vio() {
  DBUG_TRACE;
  mysql_mutex_assert_owner(&LOCK_thd_data);
  if (clone_vio != nullptr) {
    vio_shutdown(clone_vio);
    clone_vio = nullptr;
  }
}

/*
  Register an item tree tree transformation, performed by the query
  optimizer.
*/

void THD::nocheck_register_item_tree_change(Item **place, Item *new_value) {
  Item_change_record *change;
  /*
    Now we use one node per change, which adds some memory overhead,
    but still is rather fast as we use alloc_root for allocations.
    A list of item tree changes of an average query should be short.
  */
  void *change_mem = mem_root->Alloc(sizeof(*change));
  if (change_mem == nullptr) {
    /*
      OOM, thd->fatal_error() is called by the error handler of the
      memroot. Just return.
    */
    return;
  }
  change = new (change_mem) Item_change_record(place, new_value);
  change_list.push_front(change);
}

Item *THD::replace_rollback_place(Item **new_place) {
  I_List_iterator<Item_change_record> it(change_list);
  Item_change_record *change;
  while ((change = it++)) {
    if (change->new_value == *new_place) {
      DBUG_PRINT("info", ("replace_rollback_place new_value %p place %p",
                          *new_place, new_place));
      change->place = new_place;
      return change->old_value;
    }
  }
  return nullptr;
}

void THD::alias_rollback(Item **place) {
  if (stmt_arena->is_regular()) return;

  Item *new_value = *place;
  Item *old_value = nullptr;
  I_List_iterator<Item_change_record> it(change_list);
  Item_change_record *change;
  while ((change = it++)) {
    if (change->new_value == new_value) {
      old_value = change->old_value;
      break;
    }
  }
  DBUG_ASSERT(old_value != nullptr);
  *place = old_value;
  nocheck_register_item_tree_change(place, new_value);
  *place = new_value;
}

/**
  After a permant transformation by SELECT_LEX::transform_grouped_to_derived,
  a block's from list is moved to a new derived table.
  We may have fields that originally resolved to tables in orig_block
  but that now belong to tables in new_block.
  Such field may be rolled back and thus need their name resolution contexts
  updated, so they will resolve correctly on EXECUTE a being rolled back at the
  end of a PREPARE. Can be removed after WL#6570.

  @param orig_block  The query block to which these fields originally belonged
  @param new_block   The query block they now belong to and whose name
                     resolution context they need.
*/
void THD::update_ident_context(SELECT_LEX *orig_block, SELECT_LEX *new_block) {
  I_List_iterator<Item_change_record> it(change_list);
  Item_change_record *change;
  while ((change = it++)) {
    Item *old_item = change->old_value;
    if (old_item->type() == Item::FIELD_ITEM) {
      Item_field *orig_field = down_cast<Item_field *>(old_item);
      if (orig_field->context->select_lex == orig_block)
        orig_field->context = &new_block->context;
    }
  }
}

void THD::cancel_rollback(Item *i) {
  I_List_iterator<Item_change_record> it(change_list);
  Item_change_record *change;
  while ((change = it++)) {
    if (change->new_value == i) {
      change->m_cancel = true;
      // Find any other change records for this location also
      cancel_rollback_at(change->place);
    }
  }
}

void THD::cancel_rollback_at(Item **place) {
  I_List_iterator<Item_change_record> it(change_list);
  Item_change_record *change;
  while ((change = it++)) {
    if (change->place == place) {
      change->m_cancel = true;
    }
  }
}
void THD::rollback_item_tree_changes() {
  I_List_iterator<Item_change_record> it(change_list);
  Item_change_record *change;
  DBUG_TRACE;

  while ((change = it++)) {
    if (change->m_cancel) continue;

    DBUG_PRINT("info", ("rollback_item_tree_changes "
                        "place %p curr_value %p old_value %p",
                        change->place, *change->place, change->old_value));
    *change->place = change->old_value;
  }
  /* We can forget about changes memory: it's allocated in runtime memroot */
  change_list.empty();
}

void Query_arena::add_item(Item *item) {
  item->next_free = m_item_list;
  m_item_list = item;
}

void Query_arena::free_items() {
  Item *next;
  DBUG_TRACE;
  /* This works because items are allocated with (*THR_MALLOC)->Alloc() */
  for (; m_item_list; m_item_list = next) {
    next = m_item_list->next_free;
    m_item_list->delete_self();
  }
  /* Postcondition: free_list is 0 */
}

void Query_arena::set_query_arena(const Query_arena &set) {
  mem_root = set.mem_root;
  set_item_list(set.item_list());
  state = set.state;
}

void Query_arena::swap_query_arena(const Query_arena &source,
                                   Query_arena *backup) {
  backup->set_query_arena(*this);
  set_query_arena(source);
}

void THD::end_statement() {
  DBUG_TRACE;
  /* Cleanup SQL processing state to reuse this statement in next query. */
  lex_end(lex);
  //@todo Check lifetime of Query_result objects.
  // delete lex->result;
  lex->result = nullptr;  // Prepare for next statement
  /* Note that item list is freed in cleanup_after_query() */

  /*
    Don't free mem_root, as mem_root is freed in the end of dispatch_command
    (once for any command).
  */
}

Prepared_statement_map::Prepared_statement_map()
    : st_hash(key_memory_prepared_statement_map),
      names_hash(system_charset_info, key_memory_prepared_statement_map),
      m_last_found_statement(nullptr) {}

int Prepared_statement_map::insert(Prepared_statement *statement) {
  st_hash.emplace(statement->id, unique_ptr<Prepared_statement>(statement));
  if (statement->name().str) {
    names_hash.emplace(to_string(statement->name()), statement);
  }
  mysql_mutex_lock(&LOCK_prepared_stmt_count);
  /*
    We don't check that prepared_stmt_count is <= max_prepared_stmt_count
    because we would like to allow to lower the total limit
    of prepared statements below the current count. In that case
    no new statements can be added until prepared_stmt_count drops below
    the limit.
  */
  if (prepared_stmt_count >= max_prepared_stmt_count) {
    mysql_mutex_unlock(&LOCK_prepared_stmt_count);
    my_error(ER_MAX_PREPARED_STMT_COUNT_REACHED, MYF(0),
             max_prepared_stmt_count);
    goto err_max;
  }
  prepared_stmt_count++;
  mysql_mutex_unlock(&LOCK_prepared_stmt_count);

  m_last_found_statement = statement;
  return 0;

err_max:
  if (statement->name().str) names_hash.erase(to_string(statement->name()));
  st_hash.erase(statement->id);
  return 1;
}

Prepared_statement *Prepared_statement_map::find_by_name(
    const LEX_CSTRING &name) {
  return find_or_nullptr(names_hash, to_string(name));
}

Prepared_statement *Prepared_statement_map::find(ulong id) {
  if (m_last_found_statement == nullptr || id != m_last_found_statement->id) {
    Prepared_statement *stmt = find_or_nullptr(st_hash, id);
    if (stmt && stmt->name().str) return nullptr;
    m_last_found_statement = stmt;
  }
  return m_last_found_statement;
}

void Prepared_statement_map::erase(Prepared_statement *statement) {
  if (statement == m_last_found_statement) m_last_found_statement = nullptr;
  if (statement->name().str) names_hash.erase(to_string(statement->name()));

  st_hash.erase(statement->id);
  mysql_mutex_lock(&LOCK_prepared_stmt_count);
  DBUG_ASSERT(prepared_stmt_count > 0);
  prepared_stmt_count--;
  mysql_mutex_unlock(&LOCK_prepared_stmt_count);
}

void Prepared_statement_map::claim_memory_ownership() {
  for (const auto &key_and_value : st_hash) {
    my_claim(key_and_value.second.get());
  }
}

void Prepared_statement_map::reset() {
  if (!st_hash.empty()) {
#ifdef HAVE_PSI_PS_INTERFACE
    for (auto &key_and_value : st_hash) {
      Prepared_statement *stmt = key_and_value.second.get();
      MYSQL_DESTROY_PS(stmt->get_PS_prepared_stmt());
    }
#endif
    mysql_mutex_lock(&LOCK_prepared_stmt_count);
    DBUG_ASSERT(prepared_stmt_count >= st_hash.size());
    prepared_stmt_count -= st_hash.size();
    mysql_mutex_unlock(&LOCK_prepared_stmt_count);
  }
  names_hash.clear();
  st_hash.clear();
  m_last_found_statement = nullptr;
}

Prepared_statement_map::~Prepared_statement_map() {
  /*
    We do not want to grab the global LOCK_prepared_stmt_count mutex here.
    reset() should already have been called to maintain prepared_stmt_count.
   */
  DBUG_ASSERT(st_hash.empty());
}

void THD::send_kill_message() const {
  int err = killed;
  if (err && !get_stmt_da()->is_set()) {
    if ((err == KILL_CONNECTION) && !connection_events_loop_aborted())
      err = KILL_QUERY;
    /*
      KILL is fatal because:
      - if a condition handler was allowed to trap and ignore a KILL, one
      could create routines which the DBA could not kill
      - INSERT/UPDATE IGNORE should fail: if KILL arrives during
      JOIN::optimize(), statement cannot possibly run as its caller expected
      => "OK" would be misleading the caller.

      EXPLAIN ANALYZE still succeeds (but with a warning in the output),
      assuming it's come as far as the execution stage, so that the user
      can look at the execution plan and statistics so far.
    */
    if (!running_explain_analyze) {
      if (err == ER_QUERY_INTERRUPTED) {
        std::string reason;
        if (killed_reason && killed_reason[0] != '\0') {
          reason.append(", reason: ");
          reason.append(killed_reason);
        }
        my_printf_error(err, "%s%s", MYF(ME_FATALERROR), ER_THD(this, err),
                        reason.c_str());
      } else {
        my_error(err, MYF(ME_FATALERROR));
      }
    }
  }
}

/****************************************************************************
  Handling of open and locked tables states.

  This is used when we want to open/lock (and then close) some tables when
  we already have a set of tables open and locked. We use these methods for
  access to mysql.proc table to find definitions of stored routines.
****************************************************************************/

void THD::reset_n_backup_open_tables_state(Open_tables_backup *backup,
                                           uint add_state_flags) {
  DBUG_TRACE;
  backup->set_open_tables_state(this);
  backup->mdl_system_tables_svp = mdl_context.mdl_savepoint();
  reset_open_tables_state();
  state_flags |= (Open_tables_state::BACKUPS_AVAIL | add_state_flags);
}

void THD::restore_backup_open_tables_state(Open_tables_backup *backup) {
  DBUG_TRACE;
  mdl_context.rollback_to_savepoint(backup->mdl_system_tables_svp);
  /*
    Before we will throw away current open tables state we want
    to be sure that it was properly cleaned up.
  */
  DBUG_ASSERT(open_tables == nullptr && temporary_tables == nullptr &&
              lock == nullptr && locked_tables_mode == LTM_NONE &&
              get_reprepare_observer() == nullptr);

  set_open_tables_state(backup);
}

void THD::begin_attachable_ro_transaction() {
  m_attachable_trx = new Attachable_trx(this, m_attachable_trx);
}

void THD::end_attachable_transaction() {
  Attachable_trx *prev_trx = m_attachable_trx->get_prev_attachable_trx();
  delete m_attachable_trx;
  // Restore attachable transaction which was active before we started
  // the one which just has ended. NULL in most cases.
  m_attachable_trx = prev_trx;
}

void THD::begin_attachable_rw_transaction() {
  DBUG_ASSERT(!m_attachable_trx);

  m_attachable_trx = new Attachable_trx_rw(this);
}

void THD::get_trans_marker(int64_t *term, int64_t *index) const {
  /* The get and set (below) is used and called today serially during different
   * stages of ordered commit. Hence the get and set are mutually exlusive. If
   * this changes later, then we may need to protect these by locks */
  *term = term_;
  *index = index_;
}

void THD::set_trans_marker(int64_t term, int64_t index) {
  term_ = term;
  index_ = index;
}

void THD::clear_safe_purge_file() { safe_purge_file.clear(); }

void THD::set_safe_purge_file(std::string purge_file) {
  safe_purge_file = std::move(purge_file);
}

std::string THD::get_safe_purge_file() const { return safe_purge_file; }

/****************************************************************************
  Handling of statement states in functions and triggers.

  This is used to ensure that the function/trigger gets a clean state
  to work with and does not cause any side effects of the calling statement.

  It also allows most stored functions and triggers to replicate even
  if they are used items that would normally be stored in the binary
  replication (like last_insert_id() etc...)

  The following things is done
  - Disable binary logging for the duration of the statement
  - Disable multi-result-sets for the duration of the statement
  - Value of last_insert_id() is saved and restored
  - Value set by 'SET INSERT_ID=#' is reset and restored
  - Value for found_rows() is reset and restored
  - examined_row_count is added to the total
  - num_truncated_fields is added to the total
  - new savepoint level is created and destroyed

  NOTES:
    Seed for random() is saved for the first! usage of RAND()
    We reset examined_row_count and num_truncated_fields and add these to the
    result to ensure that if we have a bug that would reset these within
    a function, we are not loosing any rows from the main statement.

    We do not reset value of last_insert_id().
****************************************************************************/

void THD::reset_sub_statement_state(Sub_statement_state *backup,
                                    uint new_state) {
  /* BUG#33029, if we are replicating from a buggy master, reset
     auto_inc_intervals_forced to prevent substatement
     (triggers/functions) from using erroneous INSERT_ID value
   */
  if (rpl_master_erroneous_autoinc(this)) {
    DBUG_ASSERT(backup->auto_inc_intervals_forced.nb_elements() == 0);
    auto_inc_intervals_forced.swap(&backup->auto_inc_intervals_forced);
  }

  backup->option_bits = variables.option_bits;
  backup->check_for_truncated_fields = check_for_truncated_fields;
  backup->in_sub_stmt = in_sub_stmt;
  backup->enable_slow_log = enable_slow_log;
  backup->current_found_rows = current_found_rows;
  backup->previous_found_rows = previous_found_rows;
  backup->examined_row_count = m_examined_row_count;
  backup->sent_row_count = m_sent_row_count;
  backup->num_truncated_fields = num_truncated_fields;
  backup->client_capabilities = m_protocol->get_client_capabilities();
  backup->savepoints = get_transaction()->m_savepoints;
  backup->first_successful_insert_id_in_prev_stmt =
      first_successful_insert_id_in_prev_stmt;
  backup->first_successful_insert_id_in_cur_stmt =
      first_successful_insert_id_in_cur_stmt;

  if ((!lex->requires_prelocking() || is_update_query(lex->sql_command)) &&
      !is_current_stmt_binlog_format_row()) {
    variables.option_bits &= ~OPTION_BIN_LOG;
  }

  if ((backup->option_bits & OPTION_BIN_LOG) &&
      is_update_query(lex->sql_command) && !is_current_stmt_binlog_format_row())
    mysql_bin_log.start_union_events(this, this->query_id);

  /* Disable result sets */
  if (is_classic_protocol())
    get_protocol_classic()->remove_client_capability(CLIENT_MULTI_RESULTS);
  in_sub_stmt |= new_state;
  m_examined_row_count = 0;
  m_sent_row_count = 0;
  num_truncated_fields = 0;
  get_transaction()->m_savepoints = nullptr;
  first_successful_insert_id_in_cur_stmt = 0;

  /* Reset savepoint on transaction write set */
  if (is_current_stmt_binlog_row_enabled_with_write_set_extraction()) {
    get_transaction()->get_transaction_write_set_ctx()->reset_savepoint_list();
  }
}

void THD::restore_sub_statement_state(Sub_statement_state *backup) {
  DBUG_TRACE;
  /* BUG#33029, if we are replicating from a buggy master, restore
     auto_inc_intervals_forced so that the top statement can use the
     INSERT_ID value set before this statement.
   */
  if (rpl_master_erroneous_autoinc(this)) {
    backup->auto_inc_intervals_forced.swap(&auto_inc_intervals_forced);
    DBUG_ASSERT(backup->auto_inc_intervals_forced.nb_elements() == 0);
  }

  /*
    To save resources we want to release savepoints which were created
    during execution of function or trigger before leaving their savepoint
    level. It is enough to release first savepoint set on this level since
    all later savepoints will be released automatically.
  */
  if (get_transaction()->m_savepoints) {
    SAVEPOINT *sv;
    for (sv = get_transaction()->m_savepoints; sv->prev; sv = sv->prev) {
    }
    /* ha_release_savepoint() never returns error. */
    (void)ha_release_savepoint(this, sv);
  }
  check_for_truncated_fields = backup->check_for_truncated_fields;
  get_transaction()->m_savepoints = backup->savepoints;
  variables.option_bits = backup->option_bits;
  in_sub_stmt = backup->in_sub_stmt;
  enable_slow_log = backup->enable_slow_log;
  first_successful_insert_id_in_prev_stmt =
      backup->first_successful_insert_id_in_prev_stmt;
  first_successful_insert_id_in_cur_stmt =
      backup->first_successful_insert_id_in_cur_stmt;
  current_found_rows = backup->current_found_rows;
  previous_found_rows = backup->previous_found_rows;
  set_sent_row_count(backup->sent_row_count);
  if (is_classic_protocol())
    get_protocol_classic()->set_client_capabilities(
        backup->client_capabilities);

  /*
    If we've left sub-statement mode, reset the fatal error flag.
    Otherwise keep the current value, to propagate it up the sub-statement
    stack.

    NOTE: is_fatal_sub_stmt_error can be set only if we've been in the
    sub-statement mode.
  */

  if (!in_sub_stmt) is_fatal_sub_stmt_error = false;

  if ((variables.option_bits & OPTION_BIN_LOG) &&
      is_update_query(lex->sql_command) && !is_current_stmt_binlog_format_row())
    mysql_bin_log.stop_union_events(this);

  /*
    The below assert mostly serves as reminder that optimization in
    DML_prelocking_strategy::handle_table() relies on the fact
    that stored function/trigger can't change FOREIGN_KEY_CHECKS
    value for the top-level statement which invokes them.
  */
  DBUG_ASSERT((variables.option_bits & OPTION_NO_FOREIGN_KEY_CHECKS) ==
              (backup->option_bits & OPTION_NO_FOREIGN_KEY_CHECKS));

  /*
    The following is added to the old values as we are interested in the
    total complexity of the query
  */
  inc_examined_row_count(backup->examined_row_count);
  num_truncated_fields += backup->num_truncated_fields;

  /* Restore savepoint on transaction write set */
  if (is_current_stmt_binlog_row_enabled_with_write_set_extraction()) {
    get_transaction()
        ->get_transaction_write_set_ctx()
        ->restore_savepoint_list();
  }
}

void THD::check_yield(std::function<bool()> cond) {
  yield_cond = cond;
  thd_wait_begin(this, THD_WAIT_YIELD);
  thd_wait_end(this);
  yield_cond = nullptr;
}

/**
  Concurrency control for query.

  @return 0 if the query is admitted, 1 otherwise
*/
int THD::admit_query() {
  // Begin records that admission is required, end performs admission.
  thd_wait_begin(this, THD_WAIT_ADMIT);
  thd_wait_end(this);

  return is_error();
}

/**
  Check if wait type should release AC slot.

  @param wait_type Wait type to check.
  @param new_mode New AC request mode if AC slot is released.
  @return true if should release, false otherwise.
*/
bool THD::filter_wait_type(int wait_type,
                           enum_admission_control_request_mode &new_mode) {
  new_mode = AC_REQUEST_QUERY_READMIT_HIPRI;
  switch (wait_type) {
    case THD_WAIT_SLEEP:
      return admission_control_wait_events & ADMISSION_CONTROL_THD_WAIT_SLEEP;
    case THD_WAIT_ROW_LOCK:
      return admission_control_wait_events &
             ADMISSION_CONTROL_THD_WAIT_ROW_LOCK;
    case THD_WAIT_META_DATA_LOCK:
      return admission_control_wait_events &
             ADMISSION_CONTROL_THD_WAIT_META_DATA_LOCK;
    case THD_WAIT_INNODB_CONC:
      return admission_control_wait_events &
             ADMISSION_CONTROL_THD_WAIT_INNODB_CONC;
    case THD_WAIT_NET_IO:
      return admission_control_wait_events & ADMISSION_CONTROL_THD_WAIT_NET_IO;
    case THD_WAIT_YIELD:
      if (admission_control_wait_events & ADMISSION_CONTROL_THD_WAIT_YIELD) {
        DBUG_ASSERT(last_yield_counter <= yield_counter);
        yield_counter++;

        // yield_cond() could be expensive, so it should only be called after
        // we've determined that we are eligible for yielding.
        // Hence, we call yield_cond() after checking yield counters here.
        if (last_yield_counter + admission_control_yield_freq < yield_counter &&
            thd_yield_cond(this)) {
          // For explicit yields, we want to send the query to the back of the
          // queue to allow for other queries to run. For other yields, it's
          // likely we want to finish the query as soon as possible.
          new_mode = AC_REQUEST_QUERY_READMIT_LOPRI;
          return true;
        }
      }
      return false;
    case THD_WAIT_COMMIT:
      // For now do not readmit after commit because of possibility of AC
      // timeout error returned to client after successful commit.
      new_mode = AC_REQUEST_NONE;
      return admission_control_wait_events & ADMISSION_CONTROL_THD_WAIT_COMMIT;
    case THD_WAIT_ADMIT:
      // If filter is enabled, and command is filtered, then release AC and do
      // not readmit.
      new_mode = AC_REQUEST_NONE;
      return admission_control_multiquery_filter &&
             filter_command(lex->sql_command);
    default:
      return false;
  }
}

/**
  Callback for thd_wait_begin.

  @param wait_type Wait type.
*/
void THD::wait_begin(int wait_type) {
  if (is_in_ac) {
    // Confirm that thd_wait_end() has been called.
    DBUG_ASSERT(readmission_mode == AC_REQUEST_NONE);

    enum_admission_control_request_mode new_mode;
    if (filter_wait_type(wait_type, new_mode)) {
      multi_tenancy_exit_query(this);

      readmission_mode = new_mode;

      // Assert that thd_wait_begin/thd_wait_end calls should match.
      // In case they do not, reset the nesting level in release.
      DBUG_ASSERT(readmission_nest_level == 0);
      readmission_nest_level = 0;
    }
  } else if (wait_type == THD_WAIT_ADMIT) {
    // Confirm that thd_wait_end() has been called.
    DBUG_ASSERT(readmission_mode == AC_REQUEST_NONE);
    DBUG_ASSERT(readmission_nest_level == 0);
    readmission_nest_level = 0;

    // This is a signal to admit query. Multi-query batch only gets here for
    // the first query, subsequently THD_WAIT_ADMIT is filtered out.
    readmission_mode = AC_REQUEST_QUERY;
  } else if (readmission_mode > AC_REQUEST_NONE) {
    // Nested thd_wait_begin so need to skip thd_wait_end.
    ++readmission_nest_level;
  }
}

/**
  Callback for thd_wait_end.
*/
void THD::wait_end() {
  if (readmission_mode > AC_REQUEST_NONE) {
    if (readmission_nest_level > 0) {
      // Skip this nested thd_wait_end call.
      --readmission_nest_level;
    } else {
      if (readmission_mode == AC_REQUEST_QUERY_READMIT_HIPRI &&
          ++readmission_count % 1000 == 0) {
        readmission_mode = AC_REQUEST_QUERY_READMIT_LOPRI;
      }

      multi_tenancy_admit_query(this, readmission_mode);
      readmission_mode = AC_REQUEST_NONE;
    }
  }
}

void THD::set_sent_row_count(ha_rows count) {
  m_sent_row_count = count;
  MYSQL_SET_STATEMENT_ROWS_SENT(m_statement_psi, m_sent_row_count);
}

void THD::inc_sent_row_count(ha_rows count) {
  m_sent_row_count += count;
  status_var.rows_sent += count;
  MYSQL_SET_STATEMENT_ROWS_SENT(m_statement_psi, m_sent_row_count);
}

void THD::inc_examined_row_count(ha_rows count) {
  m_examined_row_count += count;
  status_var.rows_examined += count;
  MYSQL_SET_STATEMENT_ROWS_EXAMINED(m_statement_psi, m_examined_row_count);
}

void THD::inc_deleted_row_count(ha_rows count) {
  MYSQL_INC_STATEMENT_ROWS_DELETED(m_statement_psi, count);
  m_trx_dml_row_count++;
}

void THD::inc_inserted_row_count(ha_rows count) {
  MYSQL_INC_STATEMENT_ROWS_INSERTED(m_statement_psi, count);
  m_trx_dml_row_count++;
}

void THD::inc_updated_row_count(ha_rows count) {
  MYSQL_INC_STATEMENT_ROWS_UPDATED(m_statement_psi, count);
  m_trx_dml_row_count++;
}

void THD::inc_status_created_tmp_disk_tables() {
  DBUG_ASSERT(!status_var_aggregated);
  status_var.created_tmp_disk_tables++;
#ifdef HAVE_PSI_STATEMENT_INTERFACE
  PSI_STATEMENT_CALL(inc_statement_created_tmp_disk_tables)(m_statement_psi, 1);
#endif
}

void THD::inc_status_created_tmp_tables() {
  DBUG_ASSERT(!status_var_aggregated);
  status_var.created_tmp_tables++;
#ifdef HAVE_PSI_STATEMENT_INTERFACE
  PSI_STATEMENT_CALL(inc_statement_created_tmp_tables)(m_statement_psi, 1);
#endif
}

void THD::inc_status_select_full_join() {
  DBUG_ASSERT(!status_var_aggregated);
  status_var.select_full_join_count++;
#ifdef HAVE_PSI_STATEMENT_INTERFACE
  PSI_STATEMENT_CALL(inc_statement_select_full_join)(m_statement_psi, 1);
#endif
}

void THD::inc_status_select_full_range_join() {
  DBUG_ASSERT(!status_var_aggregated);
  status_var.select_full_range_join_count++;
#ifdef HAVE_PSI_STATEMENT_INTERFACE
  PSI_STATEMENT_CALL(inc_statement_select_full_range_join)(m_statement_psi, 1);
#endif
}

void THD::inc_status_select_range() {
  DBUG_ASSERT(!status_var_aggregated);
  status_var.select_range_count++;
#ifdef HAVE_PSI_STATEMENT_INTERFACE
  PSI_STATEMENT_CALL(inc_statement_select_range)(m_statement_psi, 1);
#endif
}

void THD::inc_status_select_range_check() {
  DBUG_ASSERT(!status_var_aggregated);
  status_var.select_range_check_count++;
#ifdef HAVE_PSI_STATEMENT_INTERFACE
  PSI_STATEMENT_CALL(inc_statement_select_range_check)(m_statement_psi, 1);
#endif
}

void THD::inc_status_select_scan() {
  DBUG_ASSERT(!status_var_aggregated);
  status_var.select_scan_count++;
#ifdef HAVE_PSI_STATEMENT_INTERFACE
  PSI_STATEMENT_CALL(inc_statement_select_scan)(m_statement_psi, 1);
#endif
}

void THD::inc_status_sort_merge_passes() {
  DBUG_ASSERT(!status_var_aggregated);
  status_var.filesort_merge_passes++;
#ifdef HAVE_PSI_STATEMENT_INTERFACE
  PSI_STATEMENT_CALL(inc_statement_sort_merge_passes)(m_statement_psi, 1);
#endif
}

void THD::inc_status_sort_range() {
  DBUG_ASSERT(!status_var_aggregated);
  status_var.filesort_range_count++;
#ifdef HAVE_PSI_STATEMENT_INTERFACE
  PSI_STATEMENT_CALL(inc_statement_sort_range)(m_statement_psi, 1);
#endif
}

void THD::inc_status_sort_rows(ha_rows count) {
  DBUG_ASSERT(!status_var_aggregated);
  status_var.filesort_rows += count;
#ifdef HAVE_PSI_STATEMENT_INTERFACE
  PSI_STATEMENT_CALL(inc_statement_sort_rows)
  (m_statement_psi, static_cast<ulong>(count));
#endif
}

void THD::inc_status_sort_scan() {
  DBUG_ASSERT(!status_var_aggregated);
  status_var.filesort_scan_count++;
#ifdef HAVE_PSI_STATEMENT_INTERFACE
  PSI_STATEMENT_CALL(inc_statement_sort_scan)(m_statement_psi, 1);
#endif
}

void THD::set_status_no_index_used() {
  server_status |= SERVER_QUERY_NO_INDEX_USED;
#ifdef HAVE_PSI_STATEMENT_INTERFACE
  PSI_STATEMENT_CALL(set_statement_no_index_used)(m_statement_psi);
#endif
}

void THD::set_status_no_good_index_used() {
  server_status |= SERVER_QUERY_NO_GOOD_INDEX_USED;
#ifdef HAVE_PSI_STATEMENT_INTERFACE
  PSI_STATEMENT_CALL(set_statement_no_good_index_used)(m_statement_psi);
#endif
}

void THD::capture_system_thread_id() {
#ifdef _WIN32
  m_system_thread_id = 0;
#else
  m_system_thread_id = syscall(SYS_gettid);
#endif  // _WIN32
}

void THD::set_command(enum enum_server_command command) {
  m_command = command;
#ifdef HAVE_PSI_THREAD_INTERFACE
  PSI_THREAD_CALL(set_thread_command)(m_command);
#endif
}

void THD::debug_assert_query_locked() const {
  if (current_thd != this) mysql_mutex_assert_owner(&LOCK_thd_query);
}

void THD::set_query(LEX_CSTRING query_arg) {
  DBUG_ASSERT(this == current_thd);
  mysql_mutex_lock(&LOCK_thd_query);
  m_query_string = query_arg;
  mysql_mutex_unlock(&LOCK_thd_query);
}

/**
  Leave explicit LOCK TABLES or prelocked mode and restore value of
  transaction sentinel in MDL subsystem.
*/

void THD::leave_locked_tables_mode() {
  if (locked_tables_mode == LTM_LOCK_TABLES) {
    /*
      When leaving LOCK TABLES mode we have to change the duration of most
      of the metadata locks being held, except for HANDLER and GRL locks,
      to transactional for them to be properly released at UNLOCK TABLES.
    */
    mdl_context.set_transaction_duration_for_all_locks();
    /*
      Make sure we don't release the global read lock and commit blocker
      when leaving LTM.
    */
    global_read_lock.set_explicit_lock_duration(this);
    /*
      Also ensure that we don't release metadata locks for open HANDLERs
      and user-level locks.
    */
    if (!handler_tables_hash.empty()) mysql_ha_set_explicit_lock_duration(this);
    if (!ull_hash.empty()) mysql_ull_set_explicit_lock_duration(this);
  }
  locked_tables_mode = LTM_NONE;
}

void THD::get_definer(LEX_USER *definer) {
  binlog_invoker();
  if (slave_thread && has_invoker()) {
    definer->user = m_invoker_user;
    definer->host = m_invoker_host;
    definer->plugin.str = "";
    definer->plugin.length = 0;
    definer->auth.str = nullptr;
    definer->auth.length = 0;
  } else
    get_default_definer(this, definer);
}

/**
  Mark transaction to rollback and mark error as fatal to a sub-statement.

  @param  all   true <=> rollback main transaction.
*/

void THD::mark_transaction_to_rollback(bool all) {
  /*
    There is no point in setting is_fatal_sub_stmt_error unless
    we are actually in_sub_stmt.
  */
  if (in_sub_stmt) is_fatal_sub_stmt_error = true;

  transaction_rollback_request = all;
}

void THD::set_next_event_pos(const char *_filename, ulonglong _pos) {
  char *&filename = binlog_next_event_pos.file_name;
  if (filename == nullptr) {
    /* First time, allocate maximal buffer */
    filename =
        (char *)my_malloc(key_memory_LOG_POS_COORD, FN_REFLEN + 1, MYF(MY_WME));
    if (filename == nullptr) return;
  }

  assert(strlen(_filename) <= FN_REFLEN);
  strcpy(filename, _filename);
  filename[FN_REFLEN] = 0;

  binlog_next_event_pos.pos = _pos;
}

void THD::clear_next_event_pos() {
  if (binlog_next_event_pos.file_name != nullptr) {
    my_free(binlog_next_event_pos.file_name);
  }
  binlog_next_event_pos.file_name = nullptr;
  binlog_next_event_pos.pos = 0;
}

void THD::set_original_commit_timestamp_for_slave_thread() {
  /*
    This function may be called in four cases:

    - From SQL thread while executing Gtid_log_event::do_apply_event

    - From an mts worker thread that executes a Gtid_log_event::do_apply_event.

    - From an mts worker thread that is processing an old binlog that
      is missing Gtid events completely, from gtid_pre_statement_checks().

    - From a normal client thread that is executing output from
      mysqlbinlog when mysqlbinlog is processing an old binlog file
      that is missing Gtid events completely, from
      gtid_pre_statement_checks() for a statement that appears after a
      BINLOG statement containing a Format_description_log_event
      originating from the master.

    Because of the last case, we need to add the following conditions to set
    original_commit_timestamp.
  */
  if (system_thread == SYSTEM_THREAD_SLAVE_SQL ||
      system_thread == SYSTEM_THREAD_SLAVE_WORKER) {
    rli_slave->original_commit_timestamp = variables.original_commit_timestamp;
  }
}

bool THD::create_explicit_snapshot() {
  auto hton = lex->create_info->db_type;
  snapshot_info_st ss_info;
  ss_info.op = snapshot_operation::SNAPSHOT_CREATE;
  bool error = ha_explicit_snapshot(this, hton, &ss_info);
  bool need_ok = true;
  error = error || show_master_offset(this, ss_info, &need_ok);
  return error;
}

bool THD::attach_explicit_snapshot(const ulonglong snapshot_id) {
  auto hton = lex->create_info->db_type;
  snapshot_info_st ss_info;
  ss_info.snapshot_id = snapshot_id;
  ss_info.op = snapshot_operation::SNAPSHOT_ATTACH;
  bool error = ha_explicit_snapshot(this, hton, &ss_info);
  bool need_ok = true;
  error = error || show_master_offset(this, ss_info, &need_ok);
  return error;
}

bool THD::release_explicit_snapshot() {
  auto hton = lex->create_info->db_type;
  snapshot_info_st ss_info;
  ss_info.op = snapshot_operation::SNAPSHOT_RELEASE;
  bool error = ha_explicit_snapshot(this, hton, &ss_info);
  bool need_ok = true;
  error = error || show_master_offset(this, ss_info, &need_ok);
  return error;
}

void THD::update_global_binlog_max_gtid(void) {
  DBUG_ENTER("THD::update_global_binlog_max_gtid");

  if (owned_gtid.greater_than(mysql_bin_log.engine_binlog_max_gtid) ||
      mysql_bin_log.engine_binlog_max_gtid.sidno != owned_gtid.sidno) {
    mysql_bin_log.engine_binlog_max_gtid = owned_gtid;
  }

  mysql_bin_log.engine_binlog_max_gtid.to_string(global_sid_map, trans_max_gtid,
                                                 true);

  DBUG_VOID_RETURN;
}

void THD::set_user_connect(USER_CONN *uc) {
  DBUG_TRACE;

  m_user_connect = uc;
}

void THD::increment_user_connections_counter() {
  DBUG_TRACE;

  m_user_connect->connections++;
}

void THD::decrement_user_connections_counter() {
  DBUG_TRACE;

  DBUG_ASSERT(m_user_connect->connections > 0);
  m_user_connect->connections--;
}

void THD::increment_con_per_hour_counter() {
  DBUG_TRACE;

  m_user_connect->conn_per_hour++;
}

void THD::increment_updates_counter() {
  DBUG_TRACE;

  m_user_connect->updates++;
}

void THD::increment_questions_counter() {
  DBUG_TRACE;

  m_user_connect->questions++;
}

ulong THD::add_nonsuper_connections_ref() {
  DBUG_ENTER("THD::add_nonsuper_connections_ref");

  // Changing user temporarily is considered two references
  // If max_nonsuper_connections is 1, then this could be a problem
  nonsuper_ref++;
  ulong count = ++nonsuper_connections;

  DBUG_RETURN(count);
}

void THD::remove_nonsuper_connections_ref(bool clear) {
  DBUG_ENTER("THD::remove_nonsuper_connections_ref");

  if (nonsuper_ref == 0) {
    DBUG_VOID_RETURN;
  }

  bool underflow;
  if (clear) {
    underflow = nonsuper_connections < nonsuper_ref;
    nonsuper_connections -= nonsuper_ref;
    nonsuper_ref = 0;
  } else {
    underflow = ((nonsuper_connections--) == 0);
    nonsuper_ref--;
  }

  DBUG_ASSERT(!underflow);
  (void)underflow;  // prevent compiler warning about unused on non-debug builds

  DBUG_VOID_RETURN;
}

/*
  Reset per-hour user resource limits when it has been more than
  an hour since they were last checked

  SYNOPSIS:
    time_out_user_resource_limits()

  NOTE:
    This assumes that the LOCK_user_conn mutex has been acquired, so it is
    safe to test and modify members of the USER_CONN structure.
*/
void THD::time_out_user_resource_limits() {
  mysql_mutex_assert_owner(&LOCK_user_conn);
  ulonglong check_time = start_utime;
  DBUG_TRACE;

  /* If more than a hour since last check, reset resource checking */
  if (check_time - m_user_connect->reset_utime >= 3600000000LL) {
    m_user_connect->questions = 1;
    m_user_connect->updates = 0;
    m_user_connect->conn_per_hour = 0;
    m_user_connect->reset_utime = check_time;
  }
}

#ifndef DBUG_OFF
void THD::Query_plan::assert_plan_is_locked_if_other() const {
  if (current_thd != thd) mysql_mutex_assert_owner(&thd->LOCK_query_plan);
}
#endif

void THD::Query_plan::set_query_plan(enum_sql_command sql_cmd, LEX *lex_arg,
                                     bool ps) {
  DBUG_ASSERT(current_thd == thd);

  // No need to grab mutex for repeated (SQLCOM_END, NULL, false).
  if (sql_command == sql_cmd && lex == lex_arg && is_ps == ps) {
    return;
  }

  thd->lock_query_plan();
  sql_command = sql_cmd;
  lex = lex_arg;
  is_ps = ps;
  thd->unlock_query_plan();
}

void THD::Query_plan::set_modification_plan(Modification_plan *plan_arg) {
  DBUG_ASSERT(current_thd == thd);
  mysql_mutex_assert_owner(&thd->LOCK_query_plan);
  modification_plan = plan_arg;
}

/**
  Push an error message into MySQL diagnostic area with line number and position

  This function provides semantic action implementers with a way
  to push the famous "You have a syntax error near..." error
  message into the diagnostic area, which is normally produced only if
  a syntax error is discovered according to the Bison grammar.
  Unlike the syntax_error_at() function, the error position points to the last
  parsed token.

  @note Parse-time only function!

  @param format         Error format message. NULL means ER(ER_SYNTAX_ERROR).
*/
void THD::syntax_error(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vsyntax_error_at(m_parser_state->m_lip.get_tok_start(), format, args);
  va_end(args);
}

/**
  Push an error message into MySQL diagnostic area with line number and position

  This function provides semantic action implementers with a way
  to push the famous "You have a syntax error near..." error
  message into the diagnostic area, which is normally produced only if
  a syntax error is discovered according to the Bison grammar.
  Unlike the syntax_error_at() function, the error position points to the last
  parsed token.

  @note Parse-time only function!

  @param mysql_errno    Error number to get a format string with ER_THD().
*/
void THD::syntax_error(int mysql_errno, ...) {
  va_list args;
  va_start(args, mysql_errno);
  vsyntax_error_at(m_parser_state->m_lip.get_tok_start(),
                   ER_THD_NONCONST(this, mysql_errno), args);
  va_end(args);
}

/**
  Push a syntax error message into MySQL diagnostic area with line
  and position information.

  This function provides semantic action implementers with a way
  to push the famous "You have a syntax error near..." error
  message into the diagnostic area, which is normally produced only if
  a parse error is discovered internally by the Bison generated
  parser.

  @note Parse-time only function!

  @param location       YYSTYPE object: error position.
  @param format         Error format message. NULL means ER(ER_SYNTAX_ERROR).
*/

void THD::syntax_error_at(const YYLTYPE &location, const char *format, ...) {
  va_list args;
  va_start(args, format);
  vsyntax_error_at(location, format, args);
  va_end(args);
}

/**
  Push a syntax error message into MySQL diagnostic area with line
  and position information.

  This function provides semantic action implementers with a way
  to push the famous "You have a syntax error near..." error
  message into the diagnostic area, which is normally produced only if
  a parse error is discovered internally by the Bison generated
  parser.

  @note Parse-time only function!

  @param location       YYSTYPE object: error position
  @param mysql_errno    Error number to get a format string with ER_THD()
*/
void THD::syntax_error_at(const YYLTYPE &location, int mysql_errno, ...) {
  va_list args;
  va_start(args, mysql_errno);
  vsyntax_error_at(location, ER_THD_NONCONST(this, mysql_errno), args);
  va_end(args);
}

void THD::vsyntax_error_at(const YYLTYPE &location, const char *format,
                           va_list args) {
  vsyntax_error_at(location.raw.start, format, args);
}

/**
  Push a syntax error message into MySQL diagnostic area with line number and
  position

  This function provides semantic action implementers with a way
  to push the famous "You have a syntax error near..." error
  message into the error stack, which is normally produced only if
  a parse error is discovered internally by the Bison generated
  parser.

  @param pos_in_lexer_raw_buffer        Pointer into LEX::m_buf or NULL.
  @param format                         An error message format string.
  @param args                           Arguments to the format string.
*/

void THD::vsyntax_error_at(const char *pos_in_lexer_raw_buffer,
                           const char *format, va_list args) {
  DBUG_ASSERT(
      pos_in_lexer_raw_buffer == nullptr ||
      (pos_in_lexer_raw_buffer >= m_parser_state->m_lip.get_buf() &&
       pos_in_lexer_raw_buffer <= m_parser_state->m_lip.get_end_of_query()));

  char buff[MYSQL_ERRMSG_SIZE];
  if (check_stack_overrun(this, STACK_MIN_SIZE, (uchar *)buff)) return;

  const uint lineno =
      pos_in_lexer_raw_buffer
          ? m_parser_state->m_lip.get_lineno(pos_in_lexer_raw_buffer)
          : 1;
  const char *pos = pos_in_lexer_raw_buffer ? pos_in_lexer_raw_buffer : "";
  ErrConvString err(pos, variables.character_set_client);
  (void)vsnprintf(buff, sizeof(buff), format, args);
  my_printf_error(ER_PARSE_ERROR, ER_THD(this, ER_PARSE_ERROR), MYF(0), buff,
                  err.ptr(), lineno);
}

bool THD::send_result_metadata(List<Item> *list, uint flags) {
  DBUG_TRACE;
  List_iterator_fast<Item> it(*list);
  Item *item;
  uchar buff[MAX_FIELD_WIDTH];
  String tmp((char *)buff, sizeof(buff), &my_charset_bin);

  if (m_protocol->start_result_metadata(list->elements, flags,
                                        variables.character_set_results))
    goto err;
  switch (variables.resultset_metadata) {
    case RESULTSET_METADATA_FULL:
      /* Sent metadata. */
      while ((item = it++)) {
        Send_field field;
        item->make_field(&field);
        m_protocol->start_row();
        if (m_protocol->send_field_metadata(&field,
                                            item->charset_for_protocol()))
          goto err;
        if (flags & Protocol::SEND_DEFAULTS) item->send(m_protocol, &tmp);
        if (m_protocol->end_row()) return true;
      }
      break;

    case RESULTSET_METADATA_NONE:
      /* Skip metadata. */
      break;

    default:
      /* Unknown @@resultset_metadata value. */
      return true;
  }

  return m_protocol->end_result_metadata();

err:
  my_error(ER_OUT_OF_RESOURCES, MYF(0)); /* purecov: inspected */
  return true;                           /* purecov: inspected */
}

bool THD::send_result_set_row(List<Item> *row_items) {
  char buffer[MAX_FIELD_WIDTH];
  String str_buffer(buffer, sizeof(buffer), &my_charset_bin);
  List_iterator_fast<Item> it(*row_items);

  DBUG_TRACE;

  for (Item *item = it++; item; item = it++) {
    if (item->send(m_protocol, &str_buffer) || is_error()) return true;
    /*
      Reset str_buffer to its original state, as it may have been altered in
      Item::send().
    */
    str_buffer.set(buffer, sizeof(buffer), &my_charset_bin);
  }
  return false;
}

void THD::send_statement_status() {
  DBUG_TRACE;
  DBUG_ASSERT(!get_stmt_da()->is_sent());
  bool error = false;
  Diagnostics_area *da = get_stmt_da();

  /* Can not be true, but do not take chances in production. */
  if (da->is_sent()) return;

  switch (da->status()) {
    case Diagnostics_area::DA_ERROR:
      /* The query failed, send error to log and abort bootstrap. */
      error = m_protocol->send_error(da->mysql_errno(), da->message_text(),
                                     da->returned_sqlstate());
      break;
    case Diagnostics_area::DA_EOF:
      error =
          m_protocol->send_eof(server_status, da->last_statement_cond_count());
      break;
    case Diagnostics_area::DA_OK:
      error = m_protocol->send_ok(
          server_status, da->last_statement_cond_count(), da->affected_rows(),
          da->last_insert_id(), da->message_text());
      break;
    case Diagnostics_area::DA_DISABLED:
      break;
    case Diagnostics_area::DA_EMPTY:
    default:
      DBUG_ASSERT(0);
      error = m_protocol->send_ok(server_status, 0, 0, 0, nullptr);
      break;
  }
  if (!error) da->set_is_sent(true);
}

void THD::claim_memory_ownership() {
/*
  Ownership of the THD object is transfered to this thread.
  This happens typically:
  - in the event scheduler,
    when the scheduler thread creates a work item and
    starts a worker thread to run it
  - in the main thread, when the code that accepts a new
    network connection creates a work item and starts a
    connection thread to run it.
  Accounting for memory statistics needs to be told
  that memory allocated by thread X now belongs to thread Y,
  so that statistics by thread/account/user/host are accurate.
  Inspect every piece of memory allocated in THD,
  and call PSI_MEMORY_CALL(memory_claim)().
*/
#ifdef HAVE_PSI_MEMORY_INTERFACE
  main_mem_root.Claim();
  my_claim(m_token_array);
  Protocol_classic *p = get_protocol_classic();
  if (p != nullptr) p->claim_memory_ownership();
  session_tracker.claim_memory_ownership();
  session_sysvar_res_mgr.claim_memory_ownership();
  for (const auto &key_and_value : user_vars) {
    my_claim(key_and_value.second.get());
  }
#if defined(ENABLED_DEBUG_SYNC)
  debug_sync_claim_memory_ownership(this);
#endif /* defined(ENABLED_DEBUG_SYNC) */
  get_transaction()->claim_memory_ownership();
  stmt_map.claim_memory_ownership();
#endif /* HAVE_PSI_MEMORY_INTERFACE */
}

void THD::rpl_detach_engine_ha_data() {
  Relay_log_info *rli =
      is_binlog_applier() ? rli_fake : (slave_thread ? rli_slave : nullptr);

  DBUG_ASSERT(!rli_fake || !rli_fake->is_engine_ha_data_detached);
  DBUG_ASSERT(!rli_slave || !rli_slave->is_engine_ha_data_detached);

  if (rli) rli->detach_engine_ha_data(this);
}

void THD::rpl_reattach_engine_ha_data() {
  Relay_log_info *rli =
      is_binlog_applier() ? rli_fake : (slave_thread ? rli_slave : nullptr);

  DBUG_ASSERT(!rli_fake || !rli_fake->is_engine_ha_data_detached);
  DBUG_ASSERT(!rli_slave || !rli_slave->is_engine_ha_data_detached);

  if (rli) rli->reattach_engine_ha_data(this);
}

bool THD::rpl_unflag_detached_engine_ha_data() const {
  Relay_log_info *rli =
      is_binlog_applier() ? rli_fake : (slave_thread ? rli_slave : nullptr);
  return rli ? rli->unflag_detached_engine_ha_data() : false;
}

bool THD::is_current_stmt_binlog_disabled() const {
  return (!(variables.option_bits & OPTION_BIN_LOG) ||
          !mysql_bin_log.is_open());
}

bool THD::is_current_stmt_binlog_log_slave_updates_disabled() const {
  return ((!opt_bin_log || (slave_thread && !opt_log_slave_updates)) ||
          !mysql_bin_log.is_open());
}

bool THD::is_current_stmt_binlog_row_enabled_with_write_set_extraction() const {
  return ((variables.transaction_write_set_extraction != HASH_ALGORITHM_OFF) &&
          is_current_stmt_binlog_format_row() &&
          !is_current_stmt_binlog_disabled());
}

bool THD::is_enabled_idempotent_recovery() const noexcept {
  return get_gtid_mode(GTID_MODE_LOCK_NONE) != GTID_MODE_OFF &&
         variables.binlog_format == BINLOG_FORMAT_ROW &&
         slave_use_idempotent_for_recovery_options ==
             SLAVE_USE_IDEMPOTENT_FOR_RECOVERY_YES;
}

bool THD::Query_plan::is_single_table_plan() const {
  assert_plan_is_locked_if_other();
  return lex->m_sql_cmd->is_single_table_plan();
}

const String THD::normalized_query() {
  m_normalized_query.mem_free();
  lex->unit->print(this, &m_normalized_query, QT_NORMALIZED_FORMAT);
  return m_normalized_query;
}

bool add_item_to_list(THD *thd, Item *item) {
  return thd->lex->select_lex->add_item_to_list(item);
}

THD::Transaction_state::Transaction_state()
    : m_query_tables_list(new Query_tables_list()),
      m_ha_data(PSI_NOT_INSTRUMENTED, m_ha_data.initial_capacity) {}

THD::Transaction_state::~Transaction_state() { delete m_query_tables_list; }

void THD::change_item_tree(Item **place, Item *new_value) {
  if (m_permanent_transform) {
    // Delete any change records for this location
    I_List_iterator<Item_change_record> it(change_list);
    Item_change_record *change;
    while ((change = it++))
      if (change->place == place) change->m_cancel = true;

    *place = new_value;
  } else {
    /* TODO: check for OOM condition here */
    if (!stmt_arena->is_regular()) {
      DBUG_PRINT("info", ("change_item_tree place %p old_value %p new_value %p",
                          place, *place, new_value));
      if (new_value)
        new_value->set_runtime_created(); /* Note the change of item tree */
      nocheck_register_item_tree_change(place, new_value);
    }
    *place = new_value;
  }
}

bool THD::notify_hton_pre_acquire_exclusive(const MDL_key *mdl_key,
                                            bool *victimized) {
  return ha_notify_exclusive_mdl(this, mdl_key, HA_NOTIFY_PRE_EVENT,
                                 victimized);
}

void THD::notify_hton_post_release_exclusive(const MDL_key *mdl_key) {
  bool unused_arg;
  ha_notify_exclusive_mdl(this, mdl_key, HA_NOTIFY_POST_EVENT, &unused_arg);
}

void reattach_engine_ha_data_to_thd(THD *thd, const struct handlerton *hton) {
  DBUG_TRACE;
  if (hton->replace_native_transaction_in_thd) {
    /* restore the saved original engine transaction's link with thd */
    void **trx_backup = &thd->get_ha_data(hton->slot)->ha_ptr_backup;

    hton->replace_native_transaction_in_thd(thd, *trx_backup, nullptr);
    *trx_backup = nullptr;
  }
}

/**
  Call parser to transform statement into a parse tree.
  Then, transform the parse tree further into an AST, ready for resolving.
*/
bool THD::sql_parser() {
  /*
    SQL parser function generated by YACC from sql_yacc.yy.

    In the case of success returns 0, and THD::is_error() is false.
    Otherwise returns 1, or THD::>is_error() is true.

    The second (output) parameter "root" returns the new parse tree.
    It is undefined (unchanged) on error. If "root" is NULL on success,
    then the parser has already called lex->make_sql_cmd() internally.
  */
  extern int MYSQLparse(class THD * thd, class Parse_tree_root * *root);

  Parse_tree_root *root = nullptr;
  if (MYSQLparse(this, &root) || is_error()) {
    /*
      Restore the original LEX if it was replaced when parsing
      a stored procedure. We must ensure that a parsing error
      does not leave any side effects in the THD.
    */
    cleanup_after_parse_error();
    return true;
  }
  if (root != nullptr && lex->make_sql_cmd(root)) {
    return true;
  }
  return false;
}

bool THD::is_one_phase_commit() {
  /* Check if XA Commit. */
  if (lex->sql_command != SQLCOM_XA_COMMIT) {
    return (false);
  }
  auto xa_commit_cmd = static_cast<Sql_cmd_xa_commit *>(lex->m_sql_cmd);
  auto xa_op = xa_commit_cmd->get_xa_opt();
  return (xa_op == XA_ONE_PHASE);
}

bool THD::secondary_storage_engine_eligible() const {
  return secondary_engine_optimization() !=
             Secondary_engine_optimization::PRIMARY_ONLY &&
         variables.use_secondary_engine != SECONDARY_ENGINE_OFF &&
         locked_tables_mode == LTM_NONE && !in_multi_stmt_transaction_mode() &&
         sp_runtime_ctx == nullptr;
}

/**
  Set the rewritten query (with passwords obfuscated etc.) on the THD.
  Wraps this in the LOCK_thd_query mutex to protect against race conditions
  with SHOW PROCESSLIST inspecting that string.

  This uses swap() and therefore changes the argument in the caller.
  That behavior is expected by (save|restore)_rlb() in sql_prepare.cc,
  and harmless in sql_rewrite.cc. Using it elsewhere is almost certainly
  wrong and must be reviewed with extreme prejudice. If in doubt, please
  check with the runtime team.

  @param query_arg  The rewritten query to use for slow/bin/general logging.
                    The value will be changed in the caller.
*/
void THD::swap_rewritten_query(String &query_arg) {
  DBUG_ASSERT(this == current_thd);

  mysql_mutex_lock(&LOCK_thd_query);
  m_rewritten_query.swap(query_arg);
  // The rewritten query should always be a valid C string, just in case.
  (void)m_rewritten_query.c_ptr_safe();
  mysql_mutex_unlock(&LOCK_thd_query);
}

/**
  Restore session state in case of parse error.

  This is a clean up function that is invoked after the Bison generated
  parser before returning an error from THD::sql_parser(). If your
  semantic actions manipulate with the session state (which
  is a very bad practice and should not normally be employed) and
  need a clean-up in case of error, and you can not use %destructor
  rule in the grammar file itself, this function should be used
  to implement the clean up.
*/

void THD::cleanup_after_parse_error() {
  sp_head *sp = lex->sphead;

  if (sp) {
    sp->m_parser_data.finish_parsing_sp_body(this);
    //  Do not delete sp_head if is invoked in the context of sp execution.
    if (sp_runtime_ctx == nullptr) {
      sp_head::destroy(sp);
      lex->sphead = nullptr;
    }
  }
}

bool THD::is_classic_protocol() const {
  return get_protocol()->type() == Protocol::PROTOCOL_BINARY ||
         get_protocol()->type() == Protocol::PROTOCOL_TEXT;
}

bool THD::is_connected() {
  /*
    All system threads (e.g., the slave IO thread) are connected but
    not using vio. So this function always returns true for all
    system threads.
  */
  if (system_thread) return true;

  if (is_classic_protocol())
    return get_protocol()->connection_alive() &&
           vio_is_connected(get_protocol_classic()->get_vio());

  return get_protocol()->connection_alive();
}

void THD::push_protocol(Protocol *protocol) {
  DBUG_ASSERT(m_protocol != nullptr);
  DBUG_ASSERT(protocol != nullptr);
  m_protocol->push_protocol(protocol);
  m_protocol = protocol;
}

void THD::pop_protocol() {
  DBUG_ASSERT(m_protocol != nullptr);
  m_protocol = m_protocol->pop_protocol();
  DBUG_ASSERT(m_protocol != nullptr);
}

void THD::set_time() {
  start_utime = utime_after_lock = my_micro_time();
  if (user_time.tv_sec || user_time.tv_usec)
    start_time = user_time;
  else
    my_micro_time_to_timeval(start_utime, &start_time);

#ifdef HAVE_PSI_THREAD_INTERFACE
  PSI_THREAD_CALL(set_thread_start_time)(query_start_in_secs());
#endif
}

/*
  Records the CPU time spent since start_cputime, and records this into
  perfschema via MYSQL_SET_STATEMENT_CPU_TIME.
*/
ulonglong THD::get_cpu_time() {
  struct timespec end_cputime;
  ulonglong cpu_time = 0;
  set_timespec(&end_cputime, 0);
  if (diff_timespec(&end_cputime, &start_cputime) != 0) {
    if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end_cputime) == 0)
      cpu_time = (ulonglong)diff_timespec(&end_cputime, &start_cputime) / 1000;
    MYSQL_SET_STATEMENT_CPU_TIME(m_statement_psi, cpu_time);
  }
  return cpu_time;
}

void THD::set_start_cputime() {
  if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start_cputime) != 0) {
    set_timespec(&start_cputime, 0);
  }
}

void THD::set_time_after_lock() {
  /*
    If mysql_lock_tables() is called multiple times,
    we stick with the first timestamp. This prevents
    anomalities with things like CREATE INDEX, where
    otherwise, we'll get the lock timestamp for the
    data dictionary update.
  */
  if (utime_after_lock != start_utime) return;
  utime_after_lock = my_micro_time();
  MYSQL_SET_STATEMENT_LOCK_TIME(m_statement_psi,
                                (utime_after_lock - start_utime));
}

void THD::update_slow_query_status() {
  if (my_micro_time() > start_utime + variables.long_query_time)
    server_status |= SERVER_QUERY_WAS_SLOW;
}

void my_ok(THD *thd, ulonglong affected_rows, ulonglong id,
           const char *message) {
  thd->set_row_count_func(affected_rows);
  thd->get_stmt_da()->set_ok_status(affected_rows, id, message);
}

void my_eof(THD *thd) {
  thd->set_row_count_func(-1);
  thd->get_stmt_da()->set_eof_status(thd);
  if (thd->variables.session_track_transaction_info > TX_TRACK_NONE) {
    TX_TRACKER_GET(tst);
    tst->add_trx_state(thd, TX_RESULT_SET);
  }
}

static std::string net_read_str(const char **ptr) {
  size_t len =
      net_field_length(const_cast<uchar **>(pointer_cast<const uchar **>(ptr)));
  const char *str = *ptr;
  *ptr += len;
  return std::string(str, len);
}

static void set_attrs_map(
    const char *ptr, size_t length,
    std::unordered_map<std::string, std::string> &attrs_map) {
  const char *end = ptr + length;

  attrs_map.clear();
  while (ptr < end) {
    std::string key = net_read_str(&ptr);
    std::string value = net_read_str(&ptr);
    attrs_map[key] = value;
  }
}

void THD::set_connection_attrs(const char *attrs, size_t length) {
  mysql_mutex_lock(&LOCK_thd_data);
  set_attrs_map(attrs, length, connection_attrs_map);
  mysql_mutex_unlock(&LOCK_thd_data);
}

static void set_attrs(
    const char *ptr, size_t length,
    std::vector<std::pair<std::string, std::string>> &query_attrs_list) {
  const char *end = ptr + length;

  query_attrs_list.clear();
  while (ptr < end) {
    std::string key = net_read_str(&ptr);
    std::string value = net_read_str(&ptr);
    auto kvp = std::make_pair<std::string, std::string>(std::move(key),
                                                        std::move(value));
    query_attrs_list.emplace_back(std::move(kvp));
  }
}

void THD::set_query_attrs(const char *attrs, size_t length) {
  query_attrs_string = std::string(attrs, length);

  mysql_mutex_lock(&LOCK_thd_data);
  set_attrs(query_attrs_string.c_str(), query_attrs_string.length(),
            query_attrs_list);
  mysql_mutex_unlock(&LOCK_thd_data);
}

static const std::string query_info_key{"query_info"};
static const std::string schema_info_key{"schema_info"};
static const std::string traceid_key{"traceid"};
static const std::string query_type_key{"query_type"};
static const std::string num_queries_key{"num_queries"};

int THD::parse_query_info_attr() {
  for (const auto &kvp : query_attrs_list) {
    if (kvp.first == traceid_key) {
      this->trace_id = kvp.second;
    } else if (kvp.first == query_info_key) {
      rapidjson::Document root;
      if (root.Parse(kvp.second.c_str()).HasParseError()) {
        my_error(ER_MALFORMED_QUERY_ATTRS, MYF(0), kvp.second.c_str());
        return -1;
      }

      {
        const auto iter = root.FindMember(traceid_key.c_str());
        if (iter != root.MemberEnd()) {
          this->trace_id = iter->value.GetString();
        }
      }

      {
        const auto iter = root.FindMember(query_type_key.c_str());
        if (iter != root.MemberEnd()) {
          this->query_type = iter->value.GetString();
        } else {
          my_error(ER_MALFORMED_QUERY_ATTRS, MYF(0), kvp.second.c_str());
          return -1;
        }
      }

      {
        const auto iter = root.FindMember(num_queries_key.c_str());
        if (iter != root.MemberEnd()) {
          this->num_queries = iter->value.GetUint64();
        } else {
          my_error(ER_MALFORMED_QUERY_ATTRS, MYF(0), kvp.second.c_str());
          return -1;
        }
      }
      return 0;
    } else if (kvp.first == schema_info_key) {
      if (!variables.validate_schema_from_attributes) {
        continue;
      }

      rapidjson::Document root;
      if (root.Parse(kvp.second.c_str()).HasParseError()) {
        my_error(ER_MALFORMED_QUERY_ATTRS, MYF(0), kvp.second.c_str());
        return -1;
      }

      if (!root.IsArray()) {
        my_error(ER_MALFORMED_QUERY_ATTRS, MYF(0), kvp.second.c_str());
        return -1;
      }
      for (auto &obj : root.GetArray()) {
        if (!obj.IsArray() || obj.Size() != 3) {
          my_error(ER_MALFORMED_QUERY_ATTRS, MYF(0), kvp.second.c_str());
          return -1;
        }
        const auto &db = obj[0].GetString();
        const auto &table = obj[1].GetString();
        const auto &cols = obj[2];

        Column_type_info column_info;
        for (auto &c : cols.GetObject()) {
          const auto &col_name = c.name.GetString();
          if (!c.value.IsArray() || c.value.Size() != 2) {
            my_error(ER_MALFORMED_QUERY_ATTRS, MYF(0), kvp.second.c_str());
            return -1;
          }
          const auto &type_name = c.value[0].GetUint64();
          if (MYSQL_TYPE_TYPED_ARRAY < type_name &&
              type_name < MYSQL_TYPE_JSON) {
            my_error(ER_MALFORMED_QUERY_ATTRS, MYF(0), kvp.second.c_str());
            return -1;
          }
          const auto &type_len = c.value[1].GetUint64();

          column_info[col_name] =
              std::make_pair((enum_field_types)type_name, type_len);
        }

        schema_info_attrs.emplace(
            std::make_pair(std::string(db), std::string(table)), column_info);
      }
    }
  }
  return 0;
}

/**
  Validates the tables passed in to see if the actual schema matches what the
  client expects. At the moment, this only checks to see if the types match
  exactly.

  @param tl The list of tables to check.
*/
int THD::validate_schema_info(TABLE_LIST *tl) {
  if (!variables.validate_schema_from_attributes) {
    return 0;
  }

  for (const TABLE_LIST *table = tl; table != nullptr;
       table = table->next_global) {
    if (table->is_view_or_derived()) {
      continue;
    }

    const TABLE_SHARE *share = table->table->s;

    const char *db_name = share->db.str;
    const char *tbl_name = share->table_name.str;

    const auto &attrs_it = schema_info_attrs.find(
        std::make_pair(std::string(db_name), std::string(tbl_name)));
    if (attrs_it != schema_info_attrs.end()) {
      for (const auto &col_info : attrs_it->second) {
        const auto &col_name = col_info.first;
        const auto &col_type = col_info.second.first;
        const auto &col_len = col_info.second.second;
        const Field *field = nullptr;

        // A hash structure is only created if we have more than
        // MAX_FIELDS_BEFORE_HASH fields.
        if (share->name_hash) {
          const auto &col_it = share->name_hash->find(col_name);
          if (col_it != share->name_hash->end()) {
            // Found a matching column.
            field = *col_it->second;
          }
        } else {
          if (share->field) {
            for (uint i = 0; i < share->fields; ++i) {
              if (share->field[i] && !my_strcasecmp(system_charset_info,
                                                    share->field[i]->field_name,
                                                    col_name.c_str())) {
                // Found a matching column.
                field = share->field[i];
              }
            }
          }
        }

        if (field == nullptr) {
          my_error(ER_SCHEMA_COLUMN_CHECK_FAILED, MYF(0), "missing column",
                   col_name.c_str());
          return -1;
        }

        if (col_type != field->type()) {
          my_error(ER_SCHEMA_COLUMN_CHECK_FAILED, MYF(0), "type mismatch",
                   col_name.c_str());
          return -1;
        }

        if ((col_type == MYSQL_TYPE_VARCHAR || col_type == MYSQL_TYPE_STRING ||
             col_type == MYSQL_TYPE_BLOB) &&
            col_len < field->char_length()) {
          my_error(ER_SCHEMA_COLUMN_CHECK_FAILED, MYF(0), "type too narrow",
                   col_name.c_str());
          return -1;
        }
      }
    }
  }

  return 0;
}

std::string THD::shard_id() {
  mysql_mutex_lock(&LOCK_thd_db_context);
  if (!m_current_db_context) {
    mysql_mutex_unlock(&LOCK_thd_db_context);
    return "";
  }
  std::string shard_id_copy = m_current_db_context->db_shard_id;
  mysql_mutex_unlock(&LOCK_thd_db_context);
  return shard_id_copy;
}

void THD::update_db_metadata(const char *db_name, const std::string &metadata) {
  mysql_mutex_lock(&LOCK_thd_data);
  mysql_mutex_lock(&LOCK_thd_db_context);
  auto it = m_db_context_hash.find(db_name);
  if (it != m_db_context_hash.end()) {
    it->second.db_metadata = metadata;
  }
  mysql_mutex_unlock(&LOCK_thd_db_context);
  mysql_mutex_unlock(&LOCK_thd_data);
}

void THD::remove_db_metadata(const char *db_name) {
  const auto name_len = strlen(db_name);
  mysql_mutex_lock(&LOCK_thd_data);
  bool is_current_db =
      (name_len == db().length) && (strncmp(db().str, db_name, name_len) == 0);
  mysql_mutex_lock(&LOCK_thd_db_context);
  m_db_context_hash.erase(db_name);
  if (is_current_db) m_current_db_context = nullptr;
  mysql_mutex_unlock(&LOCK_thd_db_context);
  mysql_mutex_unlock(&LOCK_thd_data);
}

/*
  serialize_client_attrs
    Extracts and serializes client attributes into the buffer
    THD::client_attrs_string.

    This is only calculated once per command (as opposed to per statement),
    and cleared at the end of the command. This is because attributes are
    attached commands, not statements.
*/
void THD::serialize_client_attrs(const char *query, size_t query_length) {
  StringBuffer<256> client_attrs_string;
  std::array<unsigned char, MD5_HASH_SIZE> client_id;

  std::vector<std::pair<String, String>> client_attrs;
  bool found_async_id = false;

  mysql_mutex_lock(&LOCK_global_sql_findings);
  // Populate caller, original_caller, async_id, etc
  for (const std::string &name_iter : client_attribute_names) {
    bool found = false;
    for (auto it = query_attrs_list.begin(); it != query_attrs_list.end();
         ++it) {
      if (it->first == name_iter) {
        client_attrs.emplace_back(
            String(it->first.data(), it->first.size(), &my_charset_bin),
            String(it->second.data(), it->second.size(), &my_charset_bin));
        found = true;
      }
    }

    if (!found) {
      auto it = connection_attrs_map.find(name_iter);
      if (it != connection_attrs_map.end()) {
        client_attrs.emplace_back(
            String(it->first.data(), it->first.size(), &my_charset_bin),
            String(it->second.data(), it->second.size(), &my_charset_bin));
        found = true;
      }
    }

    if (found) {
      if (name_iter == "async_id") {
        found_async_id = true;
      }
    }
  }
  mysql_mutex_unlock(&LOCK_global_sql_findings);

  // Populate async id (inspired from find_async_tag)
  //
  // Search only in first 100 characters to avoid scanning the whole query.
  // The async id is usually near the beginning.
  //
  // Only look if async_id was not passed down.
  if (!found_async_id) {
    String query100(query, std::min<size_t>(100, query_length),
                    &my_charset_bin);
    String async_word(STRING_WITH_LEN("async-"), &my_charset_bin);

    int pos = query100.strstr(async_word);

    if (pos != -1) {
      pos += async_word.length();
      int epos = pos;

      while (epos < (int)query100.length() && std::isdigit(query100[epos])) {
        epos++;
      }
      client_attrs.emplace_back(
          String("async_id", &my_charset_bin),
          String(&query100[pos], epos - pos, &my_charset_bin));
    }
  }

  // Serialize into JSON
  auto &buf = client_attrs_string;
  q_append('{', &buf);

  for (size_t i = 0; i < client_attrs.size(); i++) {
    const auto &p = client_attrs[i];

    if (i > 0) {
      q_append(STRING_WITH_LEN(", "), &buf);
    }
    q_append('\'', &buf);
    q_append(p.first.ptr(), std::min<size_t>(100, p.first.length()), &buf);
    q_append(STRING_WITH_LEN("' : '"), &buf);
    q_append(p.second.ptr(), std::min<size_t>(100, p.second.length()), &buf);
    q_append('\'', &buf);
  }
  q_append('}', &buf);

  compute_md5_hash((char *)client_id.data(), client_attrs_string.ptr(),
                   client_attrs_string.length());

  int bytes_lost MY_ATTRIBUTE((unused)) = PSI_THREAD_CALL(
      set_thread_client_attrs)(client_id.data(), client_attrs_string.ptr(),
                               client_attrs_string.length());

  memcpy(mt_key_val[THD::CLIENT_ID].data(), client_id.data(), MD5_HASH_SIZE);
  mt_key_val_set[THD::CLIENT_ID] = true;

  DBUG_ASSERT(bytes_lost == 0);
}

/*
  Start the timer for CPU write time to be collected for write_statistics
*/
void THD::set_stmt_start_write_time() {
  if (m_stmt_start_write_time_is_set) return;

  int result;
  result = clock_gettime(CLOCK_THREAD_CPUTIME_ID, &m_stmt_start_write_time);

  m_stmt_start_write_time_is_set = result == 0;
}

/*
  Capture the total cpu time(ms) spent to write the rows for stmt
 */
void THD::set_stmt_total_write_time() {
  timespec time_end;
  if (m_stmt_start_write_time_is_set &&
      (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &time_end) == 0)) {
    /* diff_timespec returns nanoseconds */
    m_stmt_total_write_time =
        diff_timespec(&time_end, &m_stmt_start_write_time);
    m_stmt_total_write_time /= 1000; /* convert to microseconds */
  }
}

/*
  Returns all MT keys for the current write query
 */
void THD::get_mt_keys_for_write_query(
    std::array<std::string, WRITE_STATISTICS_DIMENSION_COUNT> &keys) {
  // Get keys for all the target dimensions to update write stats for
  // USER
  keys[0] = get_user_name();

  // CLIENT ID
  char client_id[MD5_HASH_TO_STRING_LENGTH + 1];
  mt_hex_value(THD::CLIENT_ID, client_id, MD5_HASH_TO_STRING_LENGTH + 1);
  keys[1] = client_id;

  // SHARD
  keys[2] = get_db_name();

  // SQL ID
  char sql_id[DIGEST_HASH_TO_STRING_LENGTH + 1];
  mt_hex_value(THD::SQL_ID, sql_id, DIGEST_HASH_TO_STRING_LENGTH + 1);
  keys[3] = sql_id;
}

/**
   Should the query be throttled(error/warning) to avoid replication lag based
   on query tags Returns 1 for warning only, 2 for throwing error and 0 if query
   tag isn't present.
*/
enum_control_level THD::get_mt_throttle_tag_level() const {
  std::string query_attr_key = "mt_throttle_okay";
  std::string query_attr_value = "";
  for (const auto &p : query_attrs_list) {
    if (p.first == query_attr_key) {
      query_attr_value = p.second;
      break;
    }
  }
  // For tag_only traffic(TAO), it should be throttled only if query attribute
  // mt_throttle_okay is present
  if (variables.write_throttle_tag_only && query_attr_value != "") {
    if (query_attr_value == "WARN") return CONTROL_LEVEL_WARN;
    if (query_attr_value == "ERROR") return CONTROL_LEVEL_ERROR;
  }
  return CONTROL_LEVEL_OFF;
}

void THD::set_trx_dml_start_time() {
  /* if the dml_start_time is already initialized then do nothing */
  if (m_trx_dml_start_time_is_set) return;

  /* if stmt_start_write_time is set, use that value */
  if (m_stmt_start_write_time_is_set) {
    m_trx_dml_start_time = m_stmt_start_write_time;
    return;
  }

  int result = clock_gettime(CLOCK_THREAD_CPUTIME_ID, &m_trx_dml_start_time);

  /* remember that we have initialized m_trx_dml_start_time */
  m_trx_dml_start_time_is_set = (result == 0);
}

/*
  Capture the total cpu time(ms) spent to write the rows for stmt
 */
ulonglong THD::get_trx_dml_write_time() {
  timespec time_end;
  ulonglong dml_write_time = 0;
  if (m_trx_dml_start_time_is_set &&
      (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &time_end) == 0)) {
    /* diff_timespec returns nanoseconds */
    dml_write_time = diff_timespec(&time_end, &m_trx_dml_start_time);
    dml_write_time /= 1000; /* convert to microseconds */
  }
  return dml_write_time;
}

/**
  check if CPU execution time limit has exceeded

  @return       true if the CPU execution time limit has exceeded and the
                query needs to be errored out. Returns false otherwise.

  Note: The function will register a warning as note if the variable
        'write_control_level' is set to 'NOTE'. If the variable is set
        to 'WARN' then a regular warning is raised.
        The function will return TRUE if the CPU execution time limt
        has exceeded only if the variable 'write_control_level' is set to ERROR
        and if the variables 'write_cpu_limit_milliseconds' and
        'write_time_check_batch' are set to non-zero values.
*/
bool THD::dml_execution_cpu_limit_exceeded() {
  /* enforcing DML execution time limit is disabled if
   * - write_control_level is set to 'OFF' or
   * - write_cpu_limit_milliseconds is set to 0 or
   * - write_time_check_batch is set to 0
   */

  if (write_control_level == CONTROL_LEVEL_OFF ||
      write_cpu_limit_milliseconds == 0 || write_time_check_batch == 0 ||
      variables.sql_log_bin == 0) {
    return false;
  }

  /* if the variable 'write_control_level' is set to 'NOTE' or 'WARN'
   * then stop checking for CPU execution time limit any further
   * because the warning needs to be raised only once per statement/transaction
   */
  if ((write_control_level == CONTROL_LEVEL_NOTE ||  /* NOTE */
       write_control_level == CONTROL_LEVEL_WARN) && /* WARN */
      m_trx_dml_cpu_time_limit_warning) {
    return false;
  }

  ulonglong dml_rows_processed = get_trx_dml_row_count();

  /* bail out if there are no rows processed for DML */
  if (dml_rows_processed == 0) {
    return false;
  }

  /* first row processed */
  if (dml_rows_processed == 1) {
    set_trx_dml_start_time();
  } else if (dml_rows_processed % write_time_check_batch == 0) {
    ulonglong dml_cpu_time = get_trx_dml_write_time();
    DBUG_EXECUTE_IF("dbug.force_long_running_query",
                    dml_cpu_time = write_cpu_limit_milliseconds;);

    if (dml_cpu_time >= (ulonglong)write_cpu_limit_milliseconds) {
      /* raise warning if 'write_control_level' is 'NOTE' or 'WARN' */
      if (write_control_level == CONTROL_LEVEL_NOTE ||
          write_control_level == CONTROL_LEVEL_WARN) {
        /* raise warning */
        push_warning(
            this,
            (write_control_level == CONTROL_LEVEL_NOTE)
                ? Sql_condition::SL_NOTE
                : Sql_condition::SL_WARNING,
            ER_WARN_WRITE_EXCEEDED_CPU_LIMIT_MILLISECONDS,
            ER_THD(this, ER_WARN_WRITE_EXCEEDED_CPU_LIMIT_MILLISECONDS));

        /* remember that the warning has been raised so that further
         * warnings will not be raised for the same statement/transaction
         */
        m_trx_dml_cpu_time_limit_warning = true;
      } else if (write_control_level == CONTROL_LEVEL_ERROR) {
        return true;
      }
    }
  }

  return false;
}

void THD::mt_hex_value(enum_mt_key key_name, char *hex_val, uint len) {
  DBUG_ASSERT(key_name < MT_KEY_MAX);

  if (mt_key_is_set(key_name)) {
    if (key_name == SQL_ID) {
      DBUG_ASSERT(len == DIGEST_HASH_TO_STRING_LENGTH + 1);
      array_to_hex(hex_val, mt_key_value(key_name).data(), DIGEST_HASH_SIZE);
      hex_val[len - 1] = '\0';
    } else {
      DBUG_ASSERT(len == MD5_HASH_TO_STRING_LENGTH + 1);
      array_to_hex(hex_val, mt_key_value(key_name).data(), MD5_HASH_SIZE);
      hex_val[len - 1] = '\0';
    }
  } else {
    hex_val[0] = '\0';
  }
}

/**
  Helper function to adjust global usage/peak counters by accumulated delta.

  @param unreported_delta   pending delta for global usage update
  @param g_value            global usage value
  @param g_peak             global usage peak
  @param g_period_peak      global usage peak for some period
*/
static void adjust_global_by(std::atomic<longlong> *unreported_delta,
                             std::atomic<ulonglong> *g_value,
                             std::atomic<ulonglong> *g_peak,
                             std::atomic<ulonglong> *g_period_peak) {
  /* Counters could be disabled at runtime, re-enable requires restart. */
  if (max_tmp_disk_usage == TMP_DISK_USAGE_DISABLED) return;

  /*
    This is where threads on the secondary could race to find which
    one is doing global update. Only one will grab the whole unreported
    amount.
  */
  longlong delta = unreported_delta->exchange(0);

  /*
    It's possible that delta now is less than the increment but it would
    be very rare so just do the update regardless.
  */
  if (delta != 0) {
    ulonglong old_value = g_value->fetch_add(delta);
    ulonglong new_value = old_value + delta;

    /* Check for over and underflow. */
    DBUG_ASSERT(delta >= 0 ? new_value >= old_value : new_value < old_value);

    /* Now update peaks. */
    update_peak(g_peak, new_value);
    update_peak(g_period_peak, new_value);
  }
}

/**
  Helper function to adjust local session and global usage/peak counters
  by specified delta. The global updates are batched to avoid frequent
  updates.

  @param value              session usage value
  @param peak               session usage peak
  @param delta              signed usage delta in bytes
  @param unreported_delta   pending delta for global usage update
  @param g_value            global usage value
  @param g_peak             global usage peak
  @param g_period_peak      global usage peak for some period
  @param skip_session       do not update session counters
*/
static void adjust_by(ulonglong *value, std::atomic<ulonglong> *peak,
                      longlong delta, std::atomic<longlong> *unreported_delta,
                      std::atomic<ulonglong> *g_value,
                      std::atomic<ulonglong> *g_peak,
                      std::atomic<ulonglong> *g_period_peak,
                      bool skip_session) {
  if (!skip_session) {
    ulonglong old_value = *value;
    ulonglong new_value = old_value + delta;
    *value = new_value;

    /* Check for over and underflow. */
    DBUG_ASSERT(delta >= 0 ? new_value >= old_value : new_value < old_value);

    /* Local peak is maintained as atomic because REFRESH_STATUS resets
       peak from another thead. */
    update_peak(peak, new_value);
  }

  /* Avoid frequent updates of global usage. */
  constexpr ulonglong DISK_USAGE_REPORTING_INCREMENT = 8192;
  longlong new_delta = unreported_delta->fetch_add(delta) + delta;
  ulonglong abs_delta = new_delta >= 0 ? new_delta : -new_delta;

  if (abs_delta >= DISK_USAGE_REPORTING_INCREMENT) {
    adjust_global_by(unreported_delta, g_value, g_peak, g_period_peak);
  }
}

/**
  Adjust filesort disk usage for current session.

  @param delta    signed delta value in bytes
*/
void THD::adjust_filesort_disk_usage(longlong delta) {
  /* No need to track filesort usage per replication thread. */
  bool skip_session = slave_thread;
  adjust_by(&m_filesort_disk_usage, &m_filesort_disk_usage_peak, delta,
            &m_unreported_global_filesort_delta, &filesort_disk_usage,
            &filesort_disk_usage_peak, &filesort_disk_usage_period_peak,
            skip_session);

#ifdef HAVE_PSI_STATEMENT_INTERFACE
  ulonglong stmt_usage =
      m_filesort_disk_usage - m_stmt_filesort_disk_usage_offset;
  PSI_STATEMENT_CALL(update_statement_filesort_disk_usage)
  (m_statement_psi, stmt_usage);
#endif
}

/**
  Adjust tmp table disk usage for current session.

  @param delta    signed delta value in bytes
*/
void THD::adjust_tmp_table_disk_usage(longlong delta) {
  /* Replication threads are combined in one tablespace so tracking per
     session is not feasible, and there's no need for it. */
  bool skip_session = slave_thread;
  adjust_by(&m_tmp_table_disk_usage, &m_tmp_table_disk_usage_peak, delta,
            &m_unreported_global_tmp_table_delta, &tmp_table_disk_usage,
            &tmp_table_disk_usage_peak, &tmp_table_disk_usage_period_peak,
            skip_session);

#ifdef HAVE_PSI_STATEMENT_INTERFACE
  ulonglong stmt_usage =
      m_tmp_table_disk_usage - m_stmt_tmp_table_disk_usage_offset;
  PSI_STATEMENT_CALL(update_statement_tmp_table_disk_usage)
  (m_statement_psi, stmt_usage);
#endif
}

/**
  Propagate pending global disk usage at the end of session.
*/
void THD::propagate_pending_global_disk_usage() {
  adjust_global_by(&m_unreported_global_filesort_delta, &filesort_disk_usage,
                   &filesort_disk_usage_peak, &filesort_disk_usage_period_peak);
  adjust_global_by(&m_unreported_global_tmp_table_delta, &tmp_table_disk_usage,
                   &tmp_table_disk_usage_peak,
                   &tmp_table_disk_usage_period_peak);
}

/**
  Reset session status vars on REFRESH_STATUS.
*/
void THD::reset_status_vars() {
  reset_system_status_vars(&status_var);

  /* Handle special session status vars here. */
  reset_peak(&m_filesort_disk_usage_peak, m_filesort_disk_usage);
  reset_peak(&m_tmp_table_disk_usage_peak, m_tmp_table_disk_usage);
}

/*
  Get the OS thread id associated with this THD.
 */
my_thread_os_id_t THD::get_thread_os_id() const {
  my_thread_os_id_t id = 0;
#ifdef HAVE_PSI_THREAD_INTERFACE
  id = PSI_THREAD_CALL(get_thread_os_id)(get_psi());
#endif
  return id;
}

/*
  Get the priority of the underlying OS thread.
 */
int THD::get_thread_priority() const {
  int pri = 0;
#ifdef HAVE_PSI_THREAD_INTERFACE
  pri = PSI_THREAD_CALL(get_thread_priority)(get_psi());
#endif
  return pri;
}

/*
  Set the priority of the underlying OS thread.

  @param pri    The priority to set the thread to.
  @return       true on success, false otherwise.
*/
bool THD::set_thread_priority(int pri) {
  DBUG_ENTER("THD::set_thread_priority");

  bool ret = true;

  my_thread_os_id_t thread_os_id = get_thread_os_id();
  if (thread_os_id && get_thread_priority() != pri) {
    ret = set_system_thread_priority(thread_os_id, pri);
#ifdef HAVE_PSI_THREAD_INTERFACE
    if (ret) {
      PSI_THREAD_CALL(set_thread_priority)(get_psi(), pri);
    }
#endif
  }

  DBUG_RETURN(ret);
}

/*
  Returns the (integer) value of the specified attribute from the
  query attribute map or connection attribute map, in this order.
  In case the attribute is not found or its value exceeds the max
  value passed in then return the specified default value
 */
ulong THD::get_query_or_connect_attr_value(const char *attr_name,
                                           ulong default_value,
                                           ulong max_value) {
  ulong attr_value = 0;
  for (const auto &it : query_attrs_list) {
    if (it.first == attr_name) {
      if (!stoul_noexcept(it.second.c_str(), &attr_value) &&
          attr_value < max_value)
        return attr_value;
    }
  }

  auto it = connection_attrs_map.find(attr_name);
  if (it != connection_attrs_map.end()) {
    if (!stoul_noexcept(it->second.c_str(), &attr_value) &&
        attr_value < max_value)
      return attr_value;
  }

  return default_value;
}

/**
  Get the value of the query attribute

  @param qattr_key Name of the query attribute

  @return Value of the query attribute 'qattr_key'
*/
const std::string &THD::get_query_attr(const std::string &qattr_key) {
  /* iterate through all the query attributes */
  for (const auto &kvp : query_attrs_list) {
    /* look for qattr_key */
    if (kvp.first == qattr_key) {
      return kvp.second;
    }
  }

  /* return empty result */
  return emptyStr;
}

/**
  Get the value of the connection attribute

  @param cattr_key Name of the connection attribute

  @return Value of the query attribute 'cattr_key'
*/
const std::string &THD::get_connection_attr(const std::string &cattr_key) {
  /*
   * pointers (to db name, table names etc) are only valid until
   * the end of the current query
   */
  DBUG_ASSERT(this == current_thd);

  auto it = connection_attrs_map.find(cattr_key);
  if (it != connection_attrs_map.end()) {
    return it->second;
  }

  /* return empty result */
  return emptyStr;
}

/**
  Check whether the provided DB and table names combination is unique
  It also inserts the combination into the list if it is unique
  @param  unique_full_names - list of full names to check duplicate in
          db_name           - database name
          tab_name          - table name

  @return true if the combination is unique
 */
static bool check_unique(std::list<std::string> &unique_full_names,
                         const char *db_name, const char *tab_name) {
  std::string full_tname = db_name;
  full_tname.append(".");
  full_tname.append(tab_name);

  /* skip duplicate entries */
  if (find(unique_full_names.begin(), unique_full_names.end(), full_tname) !=
      unique_full_names.end())
    return false;

  unique_full_names.emplace_back(full_tname);
  return true;
}

/**
  Get read and write tables in the query. The tables are returned as a list
  of pairs where the first value is the DB name and the second value is the
  table name.
  The read and writes are paired together

  @return List of pairs (dbname, table name) for each of read or write tables
 */
using Table_List = std::list<std::pair<const char *, const char *>>;
std::pair<Table_List, Table_List> THD::get_read_write_tables() {
  Table_List read_tables;
  Table_List write_tables;

  std::list<std::string> unique_read_tables;   // unique read table names
  std::list<std::string> unique_write_tables;  // unique write table names
  for (const TABLE_LIST *table_iter = lex->query_tables; table_iter;
       table_iter = table_iter->next_global) {
    if (table_iter->is_view_or_derived()) continue;
    if (table_iter->updating) {
      if (check_unique(unique_write_tables, table_iter->get_db_name(),
                       table_iter->get_table_name()))
        write_tables.emplace_back(table_iter->get_db_name(),
                                  table_iter->get_table_name());
    } else {
      if (check_unique(unique_read_tables, table_iter->get_db_name(),
                       table_iter->get_table_name()))
        read_tables.emplace_back(table_iter->get_db_name(),
                                 table_iter->get_table_name());
    }
  }

  return std::make_pair(read_tables, write_tables);
}

/**
  Get tables in the query. The tables are returned as a list of pairs
  where the first value is the dbname and the second value is the table name.

  @return List of pairs: dbname, table name
 */
Table_List THD::get_query_tables() {
  Table_List uniq_tables;
  std::list<std::string> uniq_tables_str;
  /*
   * pointers (to db name, table names etc) are only valid until
   * the end of the current query
   */
  DBUG_ASSERT(this == current_thd);

  /* iterate through the list of tables */
  for (const TABLE_LIST *table = lex->query_tables; table != nullptr;
       table = table->next_global) {
    // not a view/derived table && has not been added already
    if (!table->is_view_or_derived() &&
        check_unique(uniq_tables_str, table->get_db_name(),
                     table->get_table_name()))
      uniq_tables.emplace_back(table->get_db_name(), table->get_table_name());
  }
  return uniq_tables;
}

static const char missing_digest_msg[] = "<digest_missing>";

/**
  Get query digest
*/
void THD::get_query_digest(String *digest_buffer, const char **str,
                           size_t *length, const CHARSET_INFO **cs) {
  if (m_digest != NULL) {
    compute_digest_text(&m_digest->m_digest_storage, digest_buffer);
  }

  if (digest_buffer->is_empty() ||
      (digest_buffer->length() == 1 && digest_buffer->ptr()[0] == 0)) {
    /* We couldn't compute digest for whatever reason */
    *str = missing_digest_msg;
    *length = sizeof(missing_digest_msg) / sizeof(char) - 1;
    *cs = &my_charset_utf8_bin;
  } else {
    *str = digest_buffer->c_ptr_safe();
    *length = digest_buffer->length();
    *cs = digest_buffer->charset();
  }
}

/**
  Call thd_wait_begin to mark the wait start.
*/
Thd_wait_scope::Thd_wait_scope(THD *thd, int wait_type) : m_thd(thd) {
  thd_wait_begin(m_thd, wait_type);
}

/**
  Call thd_wait_end to mark the wait end.
*/
Thd_wait_scope::~Thd_wait_scope() { thd_wait_end(m_thd); }

bool THD::set_dscp_on_socket() {
  int dscp_val = variables.dscp_on_socket;

  if (dscp_val < 0 || dscp_val >= 64) {
    // NO_LINT_DEBUG
    sql_print_warning("Invalid DSCP_QOS value in global var: %d", dscp_val);
    return false;
  }

  if (!is_classic_protocol()) {
    return false;
  }

  NET *net = get_protocol_classic()->get_net();

  int tos = dscp_val << 2;

  // figure out what domain is the socket in
  uint16_t test_family;
  socklen_t len = sizeof(test_family);
  int res =
      mysql_socket_getsockopt(net->vio->mysql_socket, SOL_SOCKET, SO_DOMAIN,
                              reinterpret_cast<void *>(&test_family), &len);

  // Lets fail, if we can't determine IPV6 vs IPV4
  if (res != 0) {
    // NO_LINT_DEBUG
    sql_print_warning(
        "Failed to get socket domain "
        "while adjusting DSCP_QOS (error: %s)",
        strerror(errno));
    return false;
  }

  if (test_family == AF_INET6) {
    res = mysql_socket_setsockopt(net->vio->mysql_socket, IPPROTO_IPV6,
                                  IPV6_TCLASS, &tos, sizeof(tos));
  } else if (test_family == AF_INET) {
    res = mysql_socket_setsockopt(net->vio->mysql_socket, IPPROTO_IP, IP_TOS,
                                  &tos, sizeof(tos));
  } else if (test_family == PF_LOCAL) {
    // skip setting socket TOS/TCLASS when access from local to host
    return true;
  } else {
    // NO_LINT_DEBUG
    sql_print_warning("Failed to get socket family %d", test_family);
    return false;
  }

  if (res != 0) {
    // NO_LINT_DEBUG
    sql_print_warning(
        "Failed to set TOS/TCLASS "
        "with (error: %s) DSCP: %d.",
        strerror(errno), tos);
    return false;
  }

  return true;
}
