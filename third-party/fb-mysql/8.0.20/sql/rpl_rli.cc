/* Copyright (c) 2006, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/rpl_rli.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <algorithm>
#include <chrono>
#include <regex>
#include <vector>

#include "libbinlogevents/include/binlog_event.h"
#include "m_ctype.h"
#include "mutex_lock.h"  // Mutex_lock
#include "my_bitmap.h"
#include "my_dbug.h"
#include "my_dir.h"  // MY_STAT
#include "my_sqlcommand.h"
#include "my_systime.h"
#include "my_thread.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/psi_stage_bits.h"
#include "mysql/plugin.h"
#include "mysql/psi/mysql_cond.h"
#include "mysql/psi/mysql_file.h"
#include "mysql/psi/psi_base.h"
#include "mysql/service_mysql_alloc.h"
#include "mysql/service_thd_wait.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "sql/auth/auth_acls.h"  // SUPER_ACL
#include "sql/auth/roles.h"      // Roles::Role_activation
#include "sql/auth/sql_auth_cache.h"
#include "sql/debug_sync.h"
#include "sql/derror.h"
#include "sql/log_event.h"  // Log_event
#include "sql/mdl.h"
#include "sql/mysqld.h"  // sync_relaylog_period ...
#include "sql/protocol.h"
#include "sql/rpl_info_factory.h"  // Rpl_info_factory
#include "sql/rpl_info_handler.h"
#include "sql/rpl_mi.h"   // Master_info
#include "sql/rpl_msr.h"  // channel_map
#include "sql/rpl_reporting.h"
#include "sql/rpl_rli_pdb.h"  // Slave_worker
#include "sql/rpl_slave.h"
#include "sql/rpl_trx_boundary_parser.h"
#include "sql/sql_base.h"  // close_thread_tables
#include "sql/sql_error.h"
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/sql_plugin.h"
#include "sql/strfunc.h"      // strconvert
#include "sql/transaction.h"  // trans_commit_stmt
#include "sql/transaction_info.h"
#include "sql/xa.h"
#include "sql_string.h"
#include "thr_mutex.h"

class Item;

using std::max;
using std::min;

/*
  Please every time you add a new field to the relay log info, update
  what follows. For now, this is just used to get the number of
  fields.
*/
const char *info_rli_fields[] = {"number_of_lines",
                                 "group_relay_log_name",
                                 "group_relay_log_pos",
                                 "group_master_log_name",
                                 "group_master_log_pos",
                                 "sql_delay",
                                 "number_of_workers",
                                 "id",
                                 "channel_name",
                                 "privilege_checks_user",
                                 "privilege_checks_hostname",
                                 "require_row_format",
                                 "require_table_primary_key_check"};

Relay_log_info::Relay_log_info(bool is_slave_recovery,
#ifdef HAVE_PSI_INTERFACE
                               PSI_mutex_key *param_key_info_run_lock,
                               PSI_mutex_key *param_key_info_data_lock,
                               PSI_mutex_key *param_key_info_sleep_lock,
                               PSI_mutex_key *param_key_info_thd_lock,
                               PSI_mutex_key *param_key_info_data_cond,
                               PSI_mutex_key *param_key_info_start_cond,
                               PSI_mutex_key *param_key_info_stop_cond,
                               PSI_mutex_key *param_key_info_sleep_cond,
#endif
                               uint param_id, const char *param_channel,
                               bool is_rli_fake)
    : Rpl_info("SQL",
#ifdef HAVE_PSI_INTERFACE
               param_key_info_run_lock, param_key_info_data_lock,
               param_key_info_sleep_lock, param_key_info_thd_lock,
               param_key_info_data_cond, param_key_info_start_cond,
               param_key_info_stop_cond, param_key_info_sleep_cond,
#endif
               param_id, param_channel),
      replicate_same_server_id(::replicate_same_server_id),
      relay_log(&sync_relaylog_period, true),
      is_relay_log_recovery(is_slave_recovery),
      recovery_sid_lock(),
      recovery_sid_map(&recovery_sid_lock),
      save_temporary_tables(nullptr),
      mi(nullptr),
      error_on_rli_init_info(false),
      gtid_timestamps_warning_logged(false),
      transaction_parser(
          Transaction_boundary_parser::TRX_BOUNDARY_PARSER_APPLIER),
      group_relay_log_pos(0),
      event_relay_log_number(0),
      event_relay_log_pos(0),
      event_start_pos(0),
      group_master_log_pos(0),
      gtid_set(nullptr),
      rli_fake(is_rli_fake),
      gtid_retrieved_initialized(false),
      m_privilege_checks_username{""},
      m_privilege_checks_hostname{""},
      m_privilege_checks_user_corrupted{false},
      m_require_row_format(false),
      m_require_table_primary_key_check(PK_CHECK_STREAM),
      is_group_master_log_pos_invalid(false),
      log_space_total(0),
      ignore_log_space_limit(false),
      sql_force_rotate_relay(false),
      last_master_timestamp(0),
      slave_skip_counter(0),
      abort_pos_wait(0),
      until_condition(UNTIL_NONE),
      trans_retries(0),
      retried_trans(0),
      tables_to_lock(nullptr),
      tables_to_lock_count(0),
      rows_query_ev(nullptr),
      last_event_start_time(0),
      deferred_events(nullptr),
      workers(PSI_NOT_INSTRUMENTED),
      workers_array_initialized(false),
      curr_group_assigned_parts(PSI_NOT_INSTRUMENTED),
      curr_group_da(PSI_NOT_INSTRUMENTED),
      curr_group_seen_gtid(0),
      curr_group_seen_begin(false),
      curr_group_seen_metadata(false),
      mts_end_group_sets_max_dbs(false),
      slave_parallel_workers(0),
      exit_counter(0),
      max_updated_index(0),
      recovery_parallel_workers(0),
      rli_checkpoint_seqno(0),
      checkpoint_group(opt_mts_checkpoint_group),
      recovery_groups_inited(false),
      mts_recovery_group_cnt(0),
      mts_recovery_index(0),
      mts_recovery_group_seen_begin(false),
      mts_group_status(MTS_NOT_IN_GROUP),
      stats_exec_time(0),
      stats_read_time(0),
      least_occupied_workers(PSI_NOT_INSTRUMENTED),
      current_mts_submode(nullptr),
      reported_unsafe_warning(false),
      rli_description_event(nullptr),
      commit_order_mngr(nullptr),
      sql_delay(0),
      sql_delay_end(0),
      m_flags(0),
      row_stmt_start_timestamp(0),
      long_find_row_note_printed(false),
      thd_tx_priority(0),
      is_engine_ha_data_detached(false),
      current_event(nullptr),
      ddl_not_atomic(false) {
  DBUG_TRACE;

#ifdef HAVE_PSI_INTERFACE
  relay_log.set_psi_keys(
      key_RELAYLOG_LOCK_index, key_RELAYLOG_LOCK_commit,
      key_RELAYLOG_LOCK_commit_queue, key_RELAYLOG_LOCK_done,
      key_RELAYLOG_LOCK_flush_queue, key_RELAYLOG_LOCK_log,
      key_RELAYLOG_LOCK_log_end_pos, key_RELAYLOG_LOCK_sync,
      key_RELAYLOG_LOCK_sync_queue, key_RELAYLOG_LOCK_xids,
      key_RELAYLOG_LOCK_non_xid_trxs, key_RELAYLOG_LOCK_lost_gtids_for_tailing,
      key_RELAYLOG_COND_done, key_RELAYLOG_update_cond,
      key_RELAYLOG_prep_xids_cond, key_RELAYLOG_non_xid_trxs_cond,
      key_file_relaylog, key_file_relaylog_index, key_file_relaylog_cache,
      key_file_relaylog_index_cache);
#endif

  group_relay_log_name[0] = event_relay_log_name[0] = group_master_log_name[0] =
      last_gtid[0] = 0;
  ign_master_log_name_end[0] = 0;
  set_timespec_nsec(&last_clock, 0);
  cached_charset_invalidate();
  inited_hash_workers = false;
  commit_timestamps_status = COMMIT_TS_UNKNOWN;

  if (!rli_fake) {
    mysql_mutex_init(key_relay_log_info_log_space_lock, &log_space_lock,
                     MY_MUTEX_INIT_FAST);
    mysql_cond_init(key_relay_log_info_log_space_cond, &log_space_cond);
    mysql_mutex_init(key_mutex_slave_parallel_pend_jobs, &pending_jobs_lock,
                     MY_MUTEX_INIT_FAST);
    mysql_cond_init(key_cond_slave_parallel_pend_jobs, &pending_jobs_cond);
    mysql_mutex_init(key_mutex_slave_parallel_worker_count, &exit_count_lock,
                     MY_MUTEX_INIT_FAST);
    mysql_mutex_init(key_mts_temp_table_LOCK, &mts_temp_table_LOCK,
                     MY_MUTEX_INIT_FAST);
    mysql_mutex_init(key_mts_gaq_LOCK, &mts_gaq_LOCK, MY_MUTEX_INIT_FAST);
    mysql_cond_init(key_cond_mts_gaq, &logical_clock_cond);

    relay_log.init_pthread_objects();
    force_flush_postponed_due_to_split_trans = false;

    Checkable_rwlock *sid_lock = new Checkable_rwlock(
#if defined(HAVE_PSI_INTERFACE)
        key_rwlock_receiver_sid_lock
#endif
    );
    Sid_map *sid_map = new Sid_map(sid_lock);
    gtid_set = new Gtid_set(sid_map, sid_lock);
  }
  gtid_monitoring_info = new Gtid_monitoring_info();
  do_server_version_split(::server_version, slave_version_split);
  until_option = nullptr;
  rpl_filter = nullptr;

  recovery_max_engine_gtid.clear();
}

/**
   The method to invoke at slave threads start
*/
void Relay_log_info::init_workers(ulong n_workers) {
  /*
    Parallel slave parameters initialization is done regardless
    whether the feature is or going to be active or not.
  */
  mts_groups_assigned = mts_events_assigned = pending_jobs = wq_size_waits_cnt =
      0;
  mts_wq_excess_cnt = mts_wq_no_underrun_cnt = mts_wq_overfill_cnt = 0;
  mts_total_wait_overlap = 0;
  mts_total_wait_worker_avail = 0;
  mts_last_online_stat = 0;

  workers.reserve(n_workers);
  workers_array_initialized = true;  // set after init
}

/**
   The method to invoke at slave threads stop
*/
void Relay_log_info::deinit_workers() { workers.clear(); }

Relay_log_info::~Relay_log_info() {
  DBUG_TRACE;

  if (!rli_fake) {
    if (recovery_groups_inited) bitmap_free(&recovery_groups);
    delete current_mts_submode;

    if (rpl_filter != nullptr) {
      /* Remove the channel's replication filter from rpl_channel_filters. */
      rpl_channel_filters.delete_filter(rpl_filter);
      rpl_filter = nullptr;
    }

    if (workers_copy_pfs.size()) {
      for (int i = static_cast<int>(workers_copy_pfs.size()) - 1; i >= 0; i--)
        delete workers_copy_pfs[i];
      workers_copy_pfs.clear();
    }

    mysql_mutex_destroy(&log_space_lock);
    mysql_cond_destroy(&log_space_cond);
    mysql_mutex_destroy(&pending_jobs_lock);
    mysql_cond_destroy(&pending_jobs_cond);
    mysql_mutex_destroy(&exit_count_lock);
    mysql_mutex_destroy(&mts_temp_table_LOCK);
    mysql_mutex_destroy(&mts_gaq_LOCK);
    mysql_cond_destroy(&logical_clock_cond);
    relay_log.cleanup();

    Sid_map *sid_map = gtid_set->get_sid_map();
    Checkable_rwlock *sid_lock = sid_map->get_sid_lock();

    sid_lock->wrlock();
    gtid_set->clear();
    sid_map->clear();
    delete gtid_set;
    delete sid_map;
    sid_lock->unlock();
    delete sid_lock;
  }

  if (set_rli_description_event(nullptr)) {
#ifndef DBUG_OFF
    bool set_rli_description_event_failed{false};
#endif
    DBUG_ASSERT(set_rli_description_event_failed);
  }
  delete until_option;
  delete gtid_monitoring_info;
}

/**
   Method is called when MTS coordinator senses the relay-log name
   has been changed.
   It marks each Worker member with this fact to make an action
   at time it will distribute a terminal event of a group to the Worker.

   Worker receives the new name at the group commiting phase
   @c Slave_worker::slave_worker_ends_group().
*/
void Relay_log_info::reset_notified_relay_log_change() {
  if (!is_parallel_exec()) return;
  for (Slave_worker **it = workers.begin(); it != workers.end(); ++it) {
    Slave_worker *w = *it;
    w->relay_log_change_notified = false;
  }
}

/**
   This method is called in mts_checkpoint_routine() to mark that each
   worker is required to adapt to a new checkpoint data whose coordinates
   are passed to it through GAQ index.

   Worker notices the new checkpoint value at the group commit to reset
   the current bitmap and starts using the clean bitmap indexed from zero
   of being reset rli_checkpoint_seqno.

    New seconds_behind_master timestamp is installed.

   @param shift            number of bits to shift by Worker due to the
                           current checkpoint change.
   @param new_ts           new seconds_behind_master timestamp value
                           unless zero. Zero could be due to FD event
                           or fake rotate event.

                              @param new_ts_millis  new
   milli_seconds_behind_master timestamp value
   @param update_timestamp if true, this function will update the
                           rli->last_master_timestamp.
*/
void Relay_log_info::reset_notified_checkpoint(ulong shift, time_t new_ts,
                                               ulonglong new_ts_millis,
                                               bool update_timestamp) {
  /*
    If this is not a parallel execution we return immediately.
  */
  if (!is_parallel_exec()) return;

  for (Slave_worker **it = workers.begin(); it != workers.end(); ++it) {
    Slave_worker *w = *it;
    /*
      Reseting the notification information in order to force workers to
      assign jobs with the new updated information.
      Notice that the bitmap_shifted is accumulated to indicate how many
      consecutive jobs were successfully processed.

      The worker when assigning a new job will set the value back to
      zero.
    */
    w->checkpoint_notified = false;
    w->bitmap_shifted = w->bitmap_shifted + shift;
    /*
      Zero shift indicates the caller rotates the master binlog.
      The new name will be passed to W through the group descriptor
      during the first post-rotation time scheduling.
    */
    if (shift == 0) w->master_log_change_notified = false;

    DBUG_PRINT("mts", ("reset_notified_checkpoint shift --> %lu, "
                       "worker->bitmap_shifted --> %lu, worker --> %u.",
                       shift, w->bitmap_shifted,
                       static_cast<unsigned>(it - workers.begin())));
  }
  /*
    There should not be a call where (shift == 0 && rli_checkpoint_seqno != 0).
    Then the new checkpoint sequence is updated by subtracting the number
    of consecutive jobs that were successfully processed.
  */
  DBUG_ASSERT(current_mts_submode->get_type() != MTS_PARALLEL_TYPE_DB_NAME ||
              !(shift == 0 && rli_checkpoint_seqno != 0));
  rli_checkpoint_seqno = rli_checkpoint_seqno - shift;
  DBUG_PRINT("mts", ("reset_notified_checkpoint shift --> %lu, "
                     "rli_checkpoint_seqno --> %u.",
                     shift, rli_checkpoint_seqno));

  if (update_timestamp) {
    mysql_mutex_lock(&data_lock);
    set_last_master_timestamp(new_ts, new_ts_millis);
    mysql_mutex_unlock(&data_lock);
  }
}

/**
   Reset recovery info from Worker info table and
   mark MTS recovery is completed.

   @return false on success true when @c reset_notified_checkpoint failed.
*/
bool Relay_log_info::mts_finalize_recovery() {
  bool ret = false;
  uint i;
  uint repo_type = get_rpl_info_handler()->get_rpl_info_type();

  DBUG_TRACE;

  for (Slave_worker **it = workers.begin(); !ret && it != workers.end(); ++it) {
    Slave_worker *w = *it;
    ret = w->reset_recovery_info();
    DBUG_EXECUTE_IF("mts_debug_recovery_reset_fails", ret = true;);
  }
  /*
    The loop is traversed in the worker index descending order due
    to specifics of the Worker table repository that does not like
    even temporary holes. Therefore stale records are deleted
    from the tail.
  */
  DBUG_EXECUTE_IF("enable_mts_wokrer_failure_in_recovery_finalize",
                  { DBUG_SET("+d,mts_worker_thread_init_fails"); });
  for (i = recovery_parallel_workers; i > workers.size() && !ret; i--) {
    Slave_worker *w =
        Rpl_info_factory::create_worker(repo_type, i - 1, this, true);
    /*
      If an error occurs during the above create_worker call, the newly created
      worker object gets deleted within the above function call itself and only
      NULL is returned. Hence the following check has been added to verify
      that a valid worker object exists.
    */
    if (w) {
      ret = w->remove_info();
      delete w;
    } else {
      ret = true;
      goto err;
    }
  }
  recovery_parallel_workers = slave_parallel_workers;

err:
  return ret;
}

bool Relay_log_info::mts_workers_queue_empty() const {
  ulong ret = 0;

  for (const auto &worker : workers) {
    mysql_mutex_lock(&worker->jobs_lock);
    ret += worker->curr_jobs;
    mysql_mutex_unlock(&worker->jobs_lock);
  }

  return ret == 0;
}

/* Checks if all in-flight stmts/trx can be safely rollbacked */
bool Relay_log_info::cannot_safely_rollback() const {
  if (!is_parallel_exec())
    return info_thd->get_transaction()->cannot_safely_rollback(
        Transaction_ctx::SESSION);

  bool ret = false;

  for (const auto &worker : workers) {
    mysql_mutex_lock(&worker->jobs_lock);
    if (worker->running_status != Slave_worker::NOT_RUNNING) {
      ret = worker->info_thd->get_transaction()->cannot_safely_rollback(
          Transaction_ctx::SESSION);
    }
    mysql_mutex_unlock(&worker->jobs_lock);
    if (ret) break;
  }

  return ret;
}

static inline int add_relay_log(Relay_log_info *rli, LOG_INFO *linfo) {
  MY_STAT s;
  DBUG_TRACE;
  mysql_mutex_assert_owner(&rli->log_space_lock);
  if (!mysql_file_stat(key_file_relaylog, linfo->log_file_name, &s, MYF(0))) {
    LogErr(ERROR_LEVEL, ER_RPL_FAILED_TO_STAT_LOG_IN_INDEX,
           linfo->log_file_name);
    return 1;
  }
  rli->log_space_total += s.st_size;
#ifndef DBUG_OFF
  char buf[22];
  DBUG_PRINT("info", ("log_space_total: %s", llstr(rli->log_space_total, buf)));
#endif
  return 0;
}

int Relay_log_info::count_relay_log_space() {
  LOG_INFO flinfo;
  DBUG_TRACE;
  MUTEX_LOCK(lock, &log_space_lock);
  log_space_total = 0;
  if (relay_log.find_log_pos(&flinfo, NullS, true)) {
    LogErr(ERROR_LEVEL, ER_RPL_LOG_NOT_FOUND_WHILE_COUNTING_RELAY_LOG_SPACE);
    return 1;
  }
  do {
    if (add_relay_log(this, &flinfo)) return 1;
  } while (!relay_log.find_next_log(&flinfo, true));
  /*
     As we have counted everything, including what may have written in a
     preceding write, we must reset bytes_written, or we may count some space
     twice.
  */
  relay_log.reset_bytes_written();
  return 0;
}

bool Relay_log_info::reset_group_relay_log_pos(const char **errmsg) {
  LOG_INFO linfo;

  mysql_mutex_assert_owner(&data_lock);

  if (relay_log.find_log_pos(&linfo, NullS, true)) {
    *errmsg = "Could not find first log during relay log initialization";
    return true;
  }
  set_group_relay_log_name(linfo.log_file_name);
  group_relay_log_pos = BIN_LOG_HEADER_SIZE;
  return false;
}

bool Relay_log_info::is_group_relay_log_name_invalid(const char **errmsg) {
  DBUG_TRACE;
  const char *errmsg_fmt = nullptr;
  static char errmsg_buff[MYSQL_ERRMSG_SIZE + FN_REFLEN];
  LOG_INFO linfo;

  *errmsg = nullptr;
  if (relay_log.find_log_pos(&linfo, group_relay_log_name, true)) {
    errmsg_fmt =
        "Could not find target log file mentioned in "
        "relay log info in the index file '%s' during "
        "relay log initialization";
    sprintf(errmsg_buff, errmsg_fmt, relay_log.get_index_fname());
    *errmsg = errmsg_buff;
    return true;
  }
  return false;
}

/**
  Update the error number, message and timestamp fields. This function is
  different from va_report() as va_report() also logs the error message in the
  log apart from updating the error fields.

  SYNOPSIS
  @param[in]  level          specifies the level- error, warning or information,
  @param[in]  err_code       error number,
  @param[in]  buff_coord     error message to be used.

*/
void Relay_log_info::fill_coord_err_buf(loglevel level, int err_code,
                                        const char *buff_coord) const {
  mysql_mutex_lock(&err_lock);

  if (level == ERROR_LEVEL) {
    m_last_error.number = err_code;
    snprintf(m_last_error.message, sizeof(m_last_error.message), "%.*s",
             MAX_SLAVE_ERRMSG - 1, buff_coord);
    m_last_error.update_timestamp();
  }

  mysql_mutex_unlock(&err_lock);
}

/**
  Waits until the SQL thread reaches (has executed up to) the
  log/position or timed out.

  SYNOPSIS
  @param[in]  thd             client thread that sent @c SELECT @c
  MASTER_POS_WAIT,
  @param[in]  log_name        log name to wait for,
  @param[in]  log_pos         position to wait for,
  @param[in]  timeout         @c timeout in seconds before giving up waiting.
                              @c timeout is double whereas it should be ulong;
  but this is to catch if the user submitted a negative timeout.

  @retval  -2   improper arguments (log_pos<0)
                or slave not running, or master info changed
                during the function's execution,
                or client thread killed. -2 is translated to NULL by caller,
  @retval  -1   timed out
  @retval  >=0  number of log events the function had to wait
                before reaching the desired log/position
 */

int Relay_log_info::wait_for_pos(THD *thd, String *log_name, longlong log_pos,
                                 double timeout) {
  int event_count = 0;
  ulong init_abort_pos_wait;
  int error = 0;
  struct timespec abstime;  // for timeout checking
  PSI_stage_info old_stage;
  DBUG_TRACE;

  if (!inited) return -2;

  DBUG_PRINT("enter", ("log_name: '%s'  log_pos: %lu  timeout: %lu",
                       log_name->c_ptr_safe(), (ulong)log_pos, (ulong)timeout));

  DEBUG_SYNC(thd, "begin_master_pos_wait");

  set_timespec_nsec(&abstime, static_cast<ulonglong>(timeout * 1000000000ULL));
  mysql_mutex_lock(&data_lock);
  thd->ENTER_COND(&data_cond, &data_lock,
                  &stage_waiting_for_the_slave_thread_to_advance_position,
                  &old_stage);
  /*
     This function will abort when it notices that some CHANGE MASTER or
     RESET MASTER has changed the master info.
     To catch this, these commands modify abort_pos_wait ; We just monitor
     abort_pos_wait and see if it has changed.
     Why do we have this mechanism instead of simply monitoring slave_running
     in the loop (we do this too), as CHANGE MASTER/RESET SLAVE require that
     the SQL thread be stopped?
     This is becasue if someones does:
     STOP SLAVE;CHANGE MASTER/RESET SLAVE; START SLAVE;
     the change may happen very quickly and we may not notice that
     slave_running briefly switches between 1/0/1.
  */
  init_abort_pos_wait = abort_pos_wait;

  /*
    We'll need to
    handle all possible log names comparisons (e.g. 999 vs 1000).
    We use ulong for string->number conversion ; this is no
    stronger limitation than in find_uniq_filename in sql/log.cc
  */
  ulong log_name_extension;
  char log_name_tmp[FN_REFLEN];  // make a char[] from String

  strmake(log_name_tmp, log_name->ptr(),
          min<uint32>(log_name->length(), FN_REFLEN - 1));

  const char *p = fn_ext(log_name_tmp);
  char *p_end;
  if (!*p || log_pos < 0) {
    error = -2;  // means improper arguments
    goto err;
  }
  // Convert 0-3 to 4
  log_pos = max(log_pos, static_cast<longlong>(BIN_LOG_HEADER_SIZE));
  /* p points to '.' */
  log_name_extension = strtoul(++p, &p_end, 10);
  /*
    p_end points to the first invalid character.
    If it equals to p, no digits were found, error.
    If it contains '\0' it means conversion went ok.
  */
  if (p_end == p || *p_end) {
    error = -2;
    goto err;
  }

  /* The "compare and wait" main loop */
  while (!thd->killed && init_abort_pos_wait == abort_pos_wait &&
         slave_running) {
    bool pos_reached;
    int cmp_result = 0;

    DBUG_PRINT("info", ("init_abort_pos_wait: %ld  abort_pos_wait: %ld",
                        init_abort_pos_wait, abort_pos_wait));
    DBUG_PRINT("info", ("group_master_log_name: '%s'  pos: %lu",
                        group_master_log_name, (ulong)group_master_log_pos));

    /*
      group_master_log_name can be "", if we are just after a fresh
      replication start or after a CHANGE MASTER TO MASTER_HOST/PORT
      (before we have executed one Rotate event from the master) or
      (rare) if the user is doing a weird slave setup (see next
      paragraph).  If group_master_log_name is "", we assume we don't
      have enough info to do the comparison yet, so we just wait until
      more data. In this case master_log_pos is always 0 except if
      somebody (wrongly) sets this slave to be a slave of itself
      without using --replicate-same-server-id (an unsupported
      configuration which does nothing), then group_master_log_pos
      will grow and group_master_log_name will stay "".
      Also in case the group master log position is invalid (e.g. after
      CHANGE MASTER TO RELAY_LOG_POS ), we will wait till the first event
      is read and the log position is valid again.
    */
    if (*group_master_log_name && !is_group_master_log_pos_invalid) {
      char *basename =
          (group_master_log_name + dirname_length(group_master_log_name));
      /*
        First compare the parts before the extension.
        Find the dot in the master's log basename,
        and protect against user's input error :
        if the names do not match up to '.' included, return error
      */
      const char *q = fn_ext(basename) + 1;
      if (strncmp(basename, log_name_tmp, (int)(q - basename))) {
        error = -2;
        break;
      }
      // Now compare extensions.
      char *q_end;
      ulong group_master_log_name_extension = strtoul(q, &q_end, 10);
      if (group_master_log_name_extension < log_name_extension)
        cmp_result = -1;
      else
        cmp_result =
            (group_master_log_name_extension > log_name_extension) ? 1 : 0;

      pos_reached =
          ((!cmp_result && group_master_log_pos >= (ulonglong)log_pos) ||
           cmp_result > 0);
      if (pos_reached || thd->killed) break;
    }

    // wait for master update, with optional timeout.

    DBUG_PRINT("info", ("Waiting for master update"));
    /*
      We are going to mysql_cond_(timed)wait(); if the SQL thread stops it
      will wake us up.
    */
    thd_wait_begin(thd, THD_WAIT_BINLOG);
    if (timeout > 0) {
      /*
        Note that mysql_cond_timedwait checks for the timeout
        before for the condition ; i.e. it returns ETIMEDOUT
        if the system time equals or exceeds the time specified by abstime
        before the condition variable is signaled or broadcast, _or_ if
        the absolute time specified by abstime has already passed at the time
        of the call.
        For that reason, mysql_cond_timedwait will do the "timeoutting" job
        even if its condition is always immediately signaled (case of a loaded
        master).
      */
      error = mysql_cond_timedwait(&data_cond, &data_lock, &abstime);
    } else
      mysql_cond_wait(&data_cond, &data_lock);
    DBUG_PRINT("info", ("Got signal of master update or timed out"));
    if (is_timeout(error)) {
#ifndef DBUG_OFF
      /*
        Doing this to generate a stack trace and make debugging
        easier.
      */
      if (DBUG_EVALUATE_IF("debug_crash_slave_time_out", 1, 0)) DBUG_ASSERT(0);
#endif
      error = -1;
      break;
    }
    error = 0;
    event_count++;
    DBUG_PRINT("info", ("Testing if killed or SQL thread not running"));
  }

err:
  mysql_mutex_unlock(&data_lock);
  thd->EXIT_COND(&old_stage);
  thd_wait_end(thd);
  DBUG_PRINT("exit",
             ("killed: %d  abort: %d  slave_running: %d "
              "improper_arguments: %d  timed_out: %d",
              thd->killed.load(), (int)(init_abort_pos_wait != abort_pos_wait),
              (int)slave_running, (int)(error == -2), (int)(error == -1)));
  if (thd->killed || init_abort_pos_wait != abort_pos_wait || !slave_running) {
    error = -2;
  }
  return error ? error : event_count;
}

int Relay_log_info::wait_for_gtid_set(THD *thd, const char *gtid,
                                      double timeout, bool update_THD_status) {
  DBUG_TRACE;

  DBUG_PRINT("info", ("Waiting for %s timeout %lf", gtid, timeout));

  Gtid_set wait_gtid_set(global_sid_map);
  global_sid_lock->rdlock();
  enum_return_status ret = wait_gtid_set.add_gtid_text(gtid);
  global_sid_lock->unlock();

  if (ret != RETURN_STATUS_OK) {
    DBUG_PRINT("exit", ("improper gtid argument"));
    return -2;
  }

  return wait_for_gtid_set(thd, &wait_gtid_set, timeout, update_THD_status);
}

int Relay_log_info::wait_for_gtid_set(THD *thd, String *gtid, double timeout,
                                      bool update_THD_status) {
  DBUG_TRACE;
  return wait_for_gtid_set(thd, gtid->c_ptr_safe(), timeout, update_THD_status);
}

/*
  TODO: This is a duplicated code that needs to be simplified.
  This will be done while developing all possible sync options.
  See WL#3584's specification.

  /Alfranio
*/
int Relay_log_info::wait_for_gtid_set(THD *thd, const Gtid_set *wait_gtid_set,
                                      double timeout, bool update_THD_status) {
  int event_count = 0;
  ulong init_abort_pos_wait;
  int error = 0;
  struct timespec abstime;  // for timeout checking
  PSI_stage_info old_stage;
  DBUG_TRACE;

  if (!inited) return -2;

  DEBUG_SYNC(thd, "begin_wait_for_gtid_set");

  set_timespec_nsec(&abstime, static_cast<ulonglong>(timeout * 1000000000ULL));

  mysql_mutex_lock(&data_lock);
  if (update_THD_status)
    thd->ENTER_COND(&data_cond, &data_lock,
                    &stage_waiting_for_the_slave_thread_to_advance_position,
                    &old_stage);
  /*
     This function will abort when it notices that some CHANGE MASTER or
     RESET MASTER has changed the master info.
     To catch this, these commands modify abort_pos_wait ; We just monitor
     abort_pos_wait and see if it has changed.
     Why do we have this mechanism instead of simply monitoring slave_running
     in the loop (we do this too), as CHANGE MASTER/RESET SLAVE require that
     the SQL thread be stopped?
     This is becasue if someones does:
     STOP SLAVE;CHANGE MASTER/RESET SLAVE; START SLAVE;
     the change may happen very quickly and we may not notice that
     slave_running briefly switches between 1/0/1.
  */
  init_abort_pos_wait = abort_pos_wait;

  /* The "compare and wait" main loop */
  while (!thd->killed && init_abort_pos_wait == abort_pos_wait &&
         slave_running) {
    DBUG_PRINT("info", ("init_abort_pos_wait: %ld  abort_pos_wait: %ld",
                        init_abort_pos_wait, abort_pos_wait));

    // wait for master update, with optional timeout.

    DBUG_ASSERT(wait_gtid_set->get_sid_map() == nullptr ||
                wait_gtid_set->get_sid_map() == global_sid_map);

    global_sid_lock->wrlock();
    const Gtid_set *executed_gtids = gtid_state->get_executed_gtids();
    const Owned_gtids *owned_gtids = gtid_state->get_owned_gtids();

#ifndef DBUG_OFF
    char *wait_gtid_set_buf;
    wait_gtid_set->to_string(&wait_gtid_set_buf);
    DBUG_PRINT("info",
               ("Waiting for '%s'. is_subset: %d and "
                "!is_intersection_nonempty: %d",
                wait_gtid_set_buf, wait_gtid_set->is_subset(executed_gtids),
                !owned_gtids->is_intersection_nonempty(wait_gtid_set)));
    my_free(wait_gtid_set_buf);
    executed_gtids->dbug_print("gtid_executed:");
    owned_gtids->dbug_print("owned_gtids:");
#endif

    /*
      Since commit is performed after log to binary log, we must also
      check if any GTID of wait_gtid_set is not yet committed.
    */
    if (wait_gtid_set->is_subset(executed_gtids) &&
        !owned_gtids->is_intersection_nonempty(wait_gtid_set)) {
      global_sid_lock->unlock();
      break;
    }
    global_sid_lock->unlock();

    DBUG_PRINT("info", ("Waiting for master update"));

    /*
      We are going to mysql_cond_(timed)wait(); if the SQL thread stops it
      will wake us up.
    */
    thd_wait_begin(thd, THD_WAIT_BINLOG);
    if (timeout > 0) {
      /*
        Note that mysql_cond_timedwait checks for the timeout
        before for the condition ; i.e. it returns ETIMEDOUT
        if the system time equals or exceeds the time specified by abstime
        before the condition variable is signaled or broadcast, _or_ if
        the absolute time specified by abstime has already passed at the time
        of the call.
        For that reason, mysql_cond_timedwait will do the "timeoutting" job
        even if its condition is always immediately signaled (case of a loaded
        master).
      */
      error = mysql_cond_timedwait(&data_cond, &data_lock, &abstime);
    } else
      mysql_cond_wait(&data_cond, &data_lock);
    DBUG_PRINT("info", ("Got signal of master update or timed out"));
    if (is_timeout(error)) {
#ifndef DBUG_OFF
      /*
        Doing this to generate a stack trace and make debugging
        easier.
      */
      if (DBUG_EVALUATE_IF("debug_crash_slave_time_out", 1, 0)) DBUG_ASSERT(0);
#endif
      error = -1;
      break;
    }
    error = 0;
    event_count++;
    DBUG_PRINT("info", ("Testing if killed or SQL thread not running"));
  }

  mysql_mutex_unlock(&data_lock);

  if (update_THD_status) thd->EXIT_COND(&old_stage);
  thd_wait_end(thd);
  DBUG_PRINT("exit",
             ("killed: %d  abort: %d  slave_running: %d "
              "improper_arguments: %d  timed_out: %d",
              thd->killed.load(), (int)(init_abort_pos_wait != abort_pos_wait),
              (int)slave_running, (int)(error == -2), (int)(error == -1)));
  if (thd->killed || init_abort_pos_wait != abort_pos_wait || !slave_running) {
    error = -2;
  }
  return error ? error : event_count;
}

int Relay_log_info::inc_group_relay_log_pos(ulonglong log_pos,
                                            bool need_data_lock, bool force) {
  int error = 0;
  DBUG_TRACE;

  if (need_data_lock)
    mysql_mutex_lock(&data_lock);
  else
    mysql_mutex_assert_owner(&data_lock);

  inc_event_relay_log_pos();
  group_relay_log_pos = event_relay_log_pos;
  strmake(group_relay_log_name, event_relay_log_name,
          sizeof(group_relay_log_name) - 1);

  /*
    In 4.x we used the event's len to compute the positions here. This is
    wrong if the event was 3.23/4.0 and has been converted to 5.0, because
    then the event's len is not what is was in the master's binlog, so this
    will make a wrong group_master_log_pos (yes it's a bug in 3.23->4.0
    replication: Exec_master_log_pos is wrong). Only way to solve this is to
    have the original offset of the end of the event the relay log. This is
    what we do in 5.0: log_pos has become "end_log_pos" (because the real use
    of log_pos in 4.0 was to compute the end_log_pos; so better to store
    end_log_pos instead of begin_log_pos.
    If we had not done this fix here, the problem would also have appeared
    when the slave and master are 5.0 but with different event length (for
    example the slave is more recent than the master and features the event
    UID). It would give false MASTER_POS_WAIT, false Exec_master_log_pos in
    SHOW SLAVE STATUS, and so the user would do some CHANGE MASTER using this
    value which would lead to badly broken replication.
    Even the relay_log_pos will be corrupted in this case, because the len is
    the relay log is not "val".
    With the end_log_pos solution, we avoid computations involving lengthes.
  */
  DBUG_PRINT("info", ("log_pos: %lu  group_master_log_pos: %lu", (long)log_pos,
                      (long)group_master_log_pos));

  if (log_pos > 0)  // 3.23 binlogs don't have log_posx
    group_master_log_pos = log_pos;
  /*
    If the master log position was invalidiated by say, "CHANGE MASTER TO
    RELAY_LOG_POS=N", it is now valid,
   */
  if (is_group_master_log_pos_invalid) is_group_master_log_pos_invalid = false;

  /*
    In MTS mode FD or Rotate event commit their solitary group to
    Coordinator's info table. Callers make sure that Workers have been
    executed all assignements.
    Broadcast to master_pos_wait() waiters should be done after
    the table is updated.
  */
  DBUG_ASSERT(!is_parallel_exec() ||
              mts_group_status != Relay_log_info::MTS_IN_GROUP);
  /*
    We do not force synchronization at this point, except for Rotate event
    (see Rotate_log_event::do_update_pos), note @c force is false by default,
    because a non-transactional change is being committed.

    For that reason, the synchronization here is subjected to
    the option sync_relay_log_info.

    See sql/rpl_rli.h for further information on this behavior.
  */
  error = flush_info(force);

  mysql_cond_broadcast(&data_cond);
  if (need_data_lock) mysql_mutex_unlock(&data_lock);
  return error;
}

void Relay_log_info::close_temporary_tables() {
  TABLE *table, *next;
  int num_closed_temp_tables = 0;
  DBUG_TRACE;

  for (table = save_temporary_tables; table; table = next) {
    next = table->next;
    /*
      Don't ask for disk deletion. For now, anyway they will be deleted when
      slave restarts, but it is a better intention to not delete them.
    */
    DBUG_PRINT("info", ("table: %p", table));
    close_temporary(nullptr, table, true, false);
    num_closed_temp_tables++;
  }
  save_temporary_tables = nullptr;
  atomic_slave_open_temp_tables -= num_closed_temp_tables;
  atomic_channel_open_temp_tables -= num_closed_temp_tables;
}

/**
  Purges relay logs. It assumes to have a run lock on rli and that no
  slave thread are running.

  @param[in]   thd         connection,
  @param[out]  errmsg      store pointer to an error message.
  @param[in]   delete_only If true, do not start writing to a new log file.

  @retval 0 successfully executed,
  @retval 1 otherwise error, where errmsg is set to point to the error message.
*/

int Relay_log_info::purge_relay_logs(THD *thd, const char **errmsg,
                                     bool delete_only) {
  int error = 0;
  const char *ln;
  /* name of the index file if opt_relaylog_index_name is set*/
  const char *log_index_name;
  /*
    Buffer to add channel name suffix when relay-log-index option is
    provided
   */
  char relay_bin_index_channel[FN_REFLEN];

  const char *ln_without_channel_name;
  /*
    Buffer to add channel name suffix when relay-log option is provided.
   */
  char relay_bin_channel[FN_REFLEN];

  char buffer[FN_REFLEN];

  mysql_mutex_t *log_lock = relay_log.get_log_lock();

  DBUG_TRACE;

  /*
    Even if inited==0, we still try to empty master_log_* variables. Indeed,
    inited==0 does not imply that they already are empty.

    It could be that slave's info initialization partly succeeded: for example
    if relay-log.info existed but all relay logs have been manually removed,
    init_info reads the old relay-log.info and fills rli->master_log_*, then
    init_info checks for the existence of the relay log, this fails and
    init_info leaves inited to 0.
    In that pathological case, master_log_pos* will be properly reinited at
    the next START SLAVE (as RESET SLAVE or CHANGE MASTER, the callers of
    purge_relay_logs, will delete bogus *.info files or replace them with
    correct files), however if the user does SHOW SLAVE STATUS before START
    SLAVE, he will see old, confusing master_log_*. In other words, we reinit
    master_log_* for SHOW SLAVE STATUS to display fine in any case.
  */
  group_master_log_name[0] = 0;
  group_master_log_pos = 0;

  /*
    Following the the relay log purge, the master_log_pos will be in sync
    with relay_log_pos, so the flag should be cleared. Refer bug#11766010.
  */

  is_group_master_log_pos_invalid = false;

  if (!inited) {
    DBUG_PRINT("info", ("inited == 0"));
    if (error_on_rli_init_info ||
        /*
          mi->reset means that the channel was reset but still exists. Channel
          shall have the index and the first relay log file.

          Those files shall be remove in a following RESET SLAVE ALL (even when
          channel was not inited again).
        */
        (mi->reset && delete_only)) {
      DBUG_ASSERT(relay_log.is_relay_log);
      ln_without_channel_name =
          relay_log.generate_name(opt_relay_logname, "-relay-bin", buffer);

      ln = add_channel_to_relay_log_name(relay_bin_channel, FN_REFLEN,
                                         ln_without_channel_name);
      if (opt_relaylog_index_name_supplied) {
        char index_file_withoutext[FN_REFLEN];
        relay_log.generate_name(opt_relaylog_index_name, "",
                                index_file_withoutext);

        log_index_name = add_channel_to_relay_log_name(
            relay_bin_index_channel, FN_REFLEN, index_file_withoutext);
      } else
        log_index_name = nullptr;

      if (relay_log.open_index_file(log_index_name, ln, true)) {
        LogErr(ERROR_LEVEL, ER_SLAVE_RELAY_LOG_PURGE_FAILED,
               "Failed to open relay log index file:",
               relay_log.get_index_fname());
        return 1;
      }
      mysql_mutex_lock(&mi->data_lock);
      mysql_mutex_lock(log_lock);
      if (relay_log.open_binlog(
              ln, nullptr,
              (max_relay_log_size ? max_relay_log_size : max_binlog_size), true,
              true /*need_lock_index=true*/, true /*need_sid_lock=true*/,
              mi->get_mi_description_event())) {
        mysql_mutex_unlock(log_lock);
        mysql_mutex_unlock(&mi->data_lock);
        LogErr(ERROR_LEVEL, ER_SLAVE_RELAY_LOG_PURGE_FAILED,
               "Failed to open relay log file:", relay_log.get_log_fname());
        return 1;
      }
      mysql_mutex_unlock(log_lock);
      mysql_mutex_unlock(&mi->data_lock);
    } else
      return 0;
  } else {
    DBUG_ASSERT(slave_running == 0);
    DBUG_ASSERT(mi->slave_running == 0);
  }
  /* Reset the transaction boundary parser and clear the last GTID queued */
  mi->transaction_parser.reset();
  mysql_mutex_lock(&mi->data_lock);
  mi->clear_gtid_monitoring_info();
  mysql_mutex_unlock(&mi->data_lock);

  slave_skip_counter = 0;
  mysql_mutex_lock(&data_lock);

  /**
    Clear the retrieved gtid set for this channel.
  */
  get_sid_lock()->wrlock();
  (const_cast<Gtid_set *>(get_gtid_set()))->clear_set_and_sid_map();
  get_sid_lock()->unlock();

  if (relay_log.reset_logs(thd, delete_only)) {
    *errmsg = "Failed during log reset";
    error = 1;
    goto err;
  }

  /* Save name of used relay log file */
  set_group_relay_log_name(relay_log.get_log_fname());
  group_relay_log_pos = BIN_LOG_HEADER_SIZE;
  if (!delete_only && count_relay_log_space()) {
    *errmsg = "Error counting relay log space";
    error = 1;
    goto err;
  }

  if (!inited && error_on_rli_init_info)
    relay_log.close(LOG_CLOSE_INDEX | LOG_CLOSE_STOP_EVENT,
                    true /*need_lock_log=true*/, true /*need_lock_index=true*/);
err:
#ifndef DBUG_OFF
  char buf[22];
#endif
  DBUG_PRINT("info", ("log_space_total: %s", llstr(log_space_total, buf)));
  mysql_mutex_unlock(&data_lock);
  return error;
}

const char *Relay_log_info::add_channel_to_relay_log_name(
    char *buff, uint buff_size, const char *base_name) {
  char *ptr;
  char channel_to_file[FN_REFLEN];
  uint errors, length;
  uint base_name_len;
  uint suffix_buff_size;

  DBUG_ASSERT(base_name != nullptr);

  base_name_len = strlen(base_name);
  suffix_buff_size = buff_size - base_name_len;

  ptr = strmake(buff, base_name, buff_size - 1);

  if (channel[0]) {
    /* adding a "-" */
    ptr = strmake(ptr, "-", suffix_buff_size - 1);

    /*
      Convert the channel name to the file names charset.
      Channel name is in system_charset which is UTF8_general_ci
      as it was defined as utf8 in the mysql.slaveinfo tables.
    */
    length = strconvert(system_charset_info, channel, &my_charset_filename,
                        channel_to_file, NAME_LEN, &errors);
    ptr = strmake(ptr, channel_to_file, suffix_buff_size - length - 1);
  }

  return (const char *)buff;
}

void Relay_log_info::cached_charset_invalidate() {
  DBUG_TRACE;

  /* Full of zeroes means uninitialized. */
  memset(cached_charset, 0, sizeof(cached_charset));
}

bool Relay_log_info::cached_charset_compare(char *charset) const {
  DBUG_TRACE;

  if (memcmp(cached_charset, charset, sizeof(cached_charset))) {
    memcpy(const_cast<char *>(cached_charset), charset, sizeof(cached_charset));
    return true;
  }
  return false;
}

int Relay_log_info::stmt_done(my_off_t event_master_log_pos) {
  clear_flag(IN_STMT);

  DBUG_ASSERT(!belongs_to_client());
  /* Worker does not execute binlog update position logics */
  DBUG_ASSERT(!is_mts_worker(info_thd));

  /*
    Replication keeps event and group positions to specify the
    set of events that were executed.
    Event positions are incremented after processing each event
    whereas group positions are incremented when an event or a
    set of events is processed such as in a transaction and are
    committed or rolled back.

    A transaction can be ended with a Query Event, i.e. either
    commit or rollback, or by a Xid Log Event. Query Event is
    used to terminate pseudo-transactions that are executed
    against non-transactional engines such as MyIsam. Xid Log
    Event denotes though that a set of changes executed
    against a transactional engine is about to commit.

    Events' positions are incremented at stmt_done(). However,
    transactions that are ended with Xid Log Event have their
    group position incremented in the do_apply_event() and in
    the do_apply_event_work().

    Notice that the type of the engine, i.e. where data and
    positions are stored, against what events are being applied
    are not considered in this logic.

    Regarding the code that follows, notice that the executed
    group coordinates don't change if the current event is internal
    to the group. The same applies to MTS Coordinator when it
    handles a Format Descriptor event that appears in the middle
    of a group that is about to be assigned.
  */
  if ((!is_parallel_exec() && is_in_group()) ||
      mts_group_status != MTS_NOT_IN_GROUP) {
    inc_event_relay_log_pos();
  } else {
    return inc_group_relay_log_pos(event_master_log_pos,
                                   true /*need_data_lock*/);
  }
  return 0;
}

void Relay_log_info::cleanup_context(THD *thd, bool error) {
  DBUG_TRACE;

  DBUG_ASSERT(info_thd == thd);
  /*
    1) Instances of Table_map_log_event, if ::do_apply_event() was called on
    them, may have opened tables, which we cannot be sure have been closed
    (because maybe the Rows_log_event have not been found or will not be,
    because slave SQL thread is stopping, or relay log has a missing tail etc).
    So we close all thread's tables. And so the table mappings have to be
    cancelled. 2) Rows_log_event::do_apply_event() may even have started
    statements or transactions on them, which we need to rollback in case of
    error. 3) If finding a Format_description_log_event after a BEGIN, we also
    need to rollback before continuing with the next events. 4) so we need this
    "context cleanup" function.
  */
  if (error) {
    trans_rollback_stmt(thd);  // if a "statement transaction"
    trans_rollback(thd);       // if a "real transaction"
    thd->variables.original_commit_timestamp = UNDEFINED_COMMIT_TIMESTAMP;
  }
  if (rows_query_ev) {
    /*
      In order to avoid invalid memory access, THD::reset_query() should be
      called before deleting the rows_query event.
    */
    info_thd->reset_query();
    info_thd->reset_query_for_display();
    delete rows_query_ev;
    rows_query_ev = nullptr;
    DBUG_EXECUTE_IF("after_deleting_the_rows_query_ev", {
      const char action[] =
          "now SIGNAL deleted_rows_query_ev WAIT_FOR go_ahead";
      DBUG_ASSERT(!debug_sync_set_action(info_thd, STRING_WITH_LEN(action)));
    };);
  }
  m_table_map.clear_tables();
  slave_close_thread_tables(thd);
  if (error) {
    /*
      trans_rollback above does not rollback XA transactions.
      It could be done only after necessarily closing tables which dictates
      the following placement.
    */
    XID_STATE *xid_state = thd->get_transaction()->xid_state();
    if (!xid_state->has_state(XID_STATE::XA_NOTR)) {
      DBUG_ASSERT(
          DBUG_EVALUATE_IF("simulate_commit_failure", 1,
                           xid_state->has_state(XID_STATE::XA_ACTIVE) ||
                               xid_state->has_state(XID_STATE::XA_IDLE)));

      xa_trans_force_rollback(thd);
      xid_state->reset();
      cleanup_trans_state(thd);
      thd->rpl_unflag_detached_engine_ha_data();
    }
    thd->mdl_context.release_transactional_locks();
  }
  clear_flag(IN_STMT);
  /*
    Cleanup for the flags that have been set at do_apply_event.
  */
  thd->variables.option_bits &= ~OPTION_NO_FOREIGN_KEY_CHECKS;
  thd->variables.option_bits &= ~OPTION_RELAXED_UNIQUE_CHECKS;

  /*
    Reset state related to long_find_row notes in the error log:
    - timestamp
    - flag that decides whether the slave prints or not
  */
  reset_row_stmt_start_timestamp();
  unset_long_find_row_note_printed();

  /*
    If the slave applier changed the current transaction isolation level,
    it need to be restored to the session default value once having the
    current transaction cleared.

    We should call "trans_reset_one_shot_chistics()" only if the "error"
    flag is "true", because "cleanup_context()" is called at the end of each
    set of Table_maps/Rows representing a statement (when the rows event
    is tagged with the STMT_END_F) with the "error" flag as "false".

    So, without the "if (error)" below, the isolation level might be reset
    in the middle of a pure row based transaction.
  */
  if (error) trans_reset_one_shot_chistics(thd);
}

void Relay_log_info::clear_tables_to_lock() {
  DBUG_TRACE;
#ifndef DBUG_OFF
  /**
    When replicating in RBR and MyISAM Merge tables are involved
    open_and_lock_tables (called in do_apply_event) appends the
    base tables to the list of tables_to_lock. Then these are
    removed from the list in close_thread_tables (which is called
    before we reach this point).

    This assertion just confirms that we get no surprises at this
    point.
   */
  uint i = 0;
  for (TABLE_LIST *ptr = tables_to_lock; ptr; ptr = ptr->next_global, i++)
    ;
  DBUG_ASSERT(i == tables_to_lock_count);
#endif

  while (tables_to_lock) {
    uchar *to_free = reinterpret_cast<uchar *>(tables_to_lock);
    if (tables_to_lock->m_tabledef_valid) {
      tables_to_lock->m_tabledef.table_def::~table_def();
      tables_to_lock->m_tabledef_valid = false;
    }

    /*
      If blob fields were used during conversion of field values
      from the master table into the slave table, then we need to
      free the memory used temporarily to store their values before
      copying into the slave's table.
    */
    if (tables_to_lock->m_conv_table) free_blobs(tables_to_lock->m_conv_table);

    tables_to_lock = static_cast<RPL_TABLE_LIST *>(tables_to_lock->next_global);
    tables_to_lock_count--;
    my_free(to_free);
  }
  DBUG_ASSERT(tables_to_lock == nullptr && tables_to_lock_count == 0);
}

void Relay_log_info::slave_close_thread_tables(THD *thd) {
  thd->get_stmt_da()->set_overwrite_status(true);
  DBUG_TRACE;
  thd->is_error() ? trans_rollback_stmt(thd) : trans_commit_stmt(thd);
  thd->get_stmt_da()->set_overwrite_status(false);

  close_thread_tables(thd);
  /*
    - If transaction rollback was requested due to deadlock
    perform it and release metadata locks.
    - If inside a multi-statement transaction,
    defer the release of metadata locks until the current
    transaction is either committed or rolled back. This prevents
    other statements from modifying the table for the entire
    duration of this transaction.  This provides commit ordering
    and guarantees serializability across multiple transactions.
    - If in autocommit mode, or outside a transactional context,
    automatically release metadata locks of the current statement.
  */
  if (thd->transaction_rollback_request) {
    trans_rollback_implicit(thd);
    thd->mdl_context.release_transactional_locks();
  } else if (!thd->in_multi_stmt_transaction_mode())
    thd->mdl_context.release_transactional_locks();
  else
    thd->mdl_context.release_statement_locks();

  clear_tables_to_lock();
}

/**
  Execute a SHOW RELAYLOG EVENTS statement.

  When multiple replication channels exist on this slave
  and no channel name is specified through FOR CHANNEL clause
  this function errors out and exits.

  @param thd Pointer to THD object for the client thread executing the
  statement.

  @retval false success
  @retval true failure
*/
bool mysql_show_relaylog_events(THD *thd) {
  Master_info *mi = nullptr;
  List<Item> field_list;
  bool res;
  DBUG_TRACE;

  DBUG_ASSERT(thd->lex->sql_command == SQLCOM_SHOW_RELAYLOG_EVENTS);

  channel_map.wrlock();

  if (!thd->lex->mi.for_channel && channel_map.get_num_instances() > 1) {
    my_error(ER_SLAVE_MULTIPLE_CHANNELS_CMD, MYF(0));
    res = true;
    goto err;
  }

  Log_event::init_show_field_list(&field_list);
  if (thd->send_result_metadata(&field_list,
                                Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF)) {
    res = true;
    goto err;
  }

  mi = channel_map.get_mi(thd->lex->mi.channel);

  if (!mi && strcmp(thd->lex->mi.channel, channel_map.get_default_channel())) {
    my_error(ER_SLAVE_CHANNEL_DOES_NOT_EXIST, MYF(0), thd->lex->mi.channel);
    res = true;
    goto err;
  }

  if (mi == nullptr) {
    my_error(ER_SLAVE_CONFIGURATION, MYF(0));
    res = true;
    goto err;
  }

  res = show_binlog_events(thd, &mi->rli->relay_log);

err:
  channel_map.unlock();

  return res;
}

int Relay_log_info::rli_init_info(bool skip_received_gtid_set_recovery) {
  int error = 0;
  enum_return_check check_return = ERROR_CHECKING_REPOSITORY;
  const char *msg = nullptr;
  DBUG_TRACE;

  mysql_mutex_assert_owner(&data_lock);

  /*
    If Relay_log_info is issued again after a failed init_info(), for
    instance because of missing relay log files, it will generate new
    files and ignore the previous failure, to avoid that we set
    error_on_rli_init_info as true.
    This a consequence of the behaviour change, in the past server was
    stopped when there were replication initialization errors, now it is
    not and so init_info() must be aware of previous failures.
  */
  if (error_on_rli_init_info) {
    // In raft  mode, these error codes are critical. Hence we should
    //     not chew them.
    if (enable_raft_plugin) error = 1;
    goto err;
  }

  if (inited) {
    return recovery_parallel_workers ? mts_recovery_groups(this) : 0;
  }

  slave_skip_counter = 0;
  abort_pos_wait = 0;
  log_space_limit = relay_log_space_limit;
  log_space_total = 0;
  tables_to_lock = nullptr;
  tables_to_lock_count = 0;

  char pattern[FN_REFLEN];
  (void)my_realpath(pattern, slave_load_tmpdir, 0);
  /*
   @TODO:
    In MSR, sometimes slave fail with the following error:
    Unable to use slave's temporary directory /tmp -
    Can't create/write to file
   '/tmp/SQL_LOAD-92d1eee0-9de4-11e3-8874-68730ad50fcb'    (Errcode: 17 - File
   exists), Error_code: 1

   */
  if (fn_format(pattern, PREFIX_SQL_LOAD, pattern, "",
                MY_SAFE_PATH | MY_RETURN_REAL_PATH) == NullS) {
    LogErr(ERROR_LEVEL, ER_SLAVE_CANT_USE_TEMPDIR, slave_load_tmpdir);
    return 1;
  }
  unpack_filename(slave_patternload_file, pattern);
  slave_patternload_file_size = strlen(slave_patternload_file);

  /*
    The relay log will now be opened, as a WRITE_CACHE IO_CACHE.
    Note that the I/O thread flushes it to disk after writing every
    event, in flush_info within the master info.
  */
  /*
    For the maximum log size, we choose max_relay_log_size if it is
    non-zero, max_binlog_size otherwise. If later the user does SET
    GLOBAL on one of these variables, fix_max_binlog_size and
    fix_max_relay_log_size will reconsider the choice (for example
    if the user changes max_relay_log_size to zero, we have to
    switch to using max_binlog_size for the relay log) and update
    relay_log.max_size (and mysql_bin_log.max_size).
  */
  {
    /* Reports an error and returns, if the --relay-log's path
       is a directory.*/
    if (opt_relay_logname &&
        opt_relay_logname[strlen(opt_relay_logname) - 1] == FN_LIBCHAR) {
      LogErr(ERROR_LEVEL, ER_RPL_RELAY_LOG_NEEDS_FILE_NOT_DIRECTORY,
             opt_relay_logname);
      return 1;
    }

    /* Reports an error and returns, if the --relay-log-index's path
       is a directory.*/
    if (opt_relaylog_index_name &&
        opt_relaylog_index_name[strlen(opt_relaylog_index_name) - 1] ==
            FN_LIBCHAR) {
      LogErr(ERROR_LEVEL, ER_RPL_RELAY_LOG_INDEX_NEEDS_FILE_NOT_DIRECTORY,
             opt_relaylog_index_name);
      return 1;
    }

    char buf[FN_REFLEN];
    /* The base name of the relay log file considering multisource rep */
    const char *ln;
    /*
      relay log name without channel prefix taking into account
      --relay-log option.
    */
    const char *ln_without_channel_name;
    static bool name_warning_sent = false;

    /*
      Buffer to add channel name suffix when relay-log option is provided.
    */
    char relay_bin_channel[FN_REFLEN];
    /*
      Buffer to add channel name suffix when relay-log-index option is provided
    */
    char relay_bin_index_channel[FN_REFLEN];

    /* name of the index file if opt_relaylog_index_name is set*/
    const char *log_index_name;

    // In raft mode, relay log always points to the raft log (which is the
    // binlog)
    if (enable_raft_plugin) {
      ln_without_channel_name =
          relay_log.generate_name(opt_bin_logname, "-bin", buf);
      ln = add_channel_to_relay_log_name(relay_bin_channel, FN_REFLEN,
                                         ln_without_channel_name);
    } else {
      ln_without_channel_name =
          relay_log.generate_name(opt_relay_logname, "-relay-bin", buf);

      ln = add_channel_to_relay_log_name(relay_bin_channel, FN_REFLEN,
                                         ln_without_channel_name);
    }

    /* We send the warning only at startup, not after every RESET SLAVE */
    if (!opt_relay_logname_supplied && !opt_relaylog_index_name_supplied &&
        !name_warning_sent) {
      /*
        User didn't give us info to name the relay log index file.
        Picking `hostname`-relay-bin.index like we do, causes replication to
        fail if this slave's hostname is changed later. So, we would like to
        instead require a name. But as we don't want to break many existing
        setups, we only give warning, not error.
      */
      LogErr(WARNING_LEVEL, ER_RPL_PLEASE_USE_OPTION_RELAY_LOG,
             ln_without_channel_name);
      name_warning_sent = true;
    }

    /*
       If relay log index option is set, convert into channel specific
       index file. If the opt_relaylog_index has an extension, we strip
       it too. This is inconsistent to relay log names.
    */
    if (opt_relaylog_index_name_supplied) {
      char index_file_withoutext[FN_REFLEN];
      if (enable_raft_plugin)
        relay_log.generate_name(opt_binlog_index_name, "",
                                index_file_withoutext);
      else
        relay_log.generate_name(opt_relaylog_index_name, "",
                                index_file_withoutext);

      log_index_name = add_channel_to_relay_log_name(
          relay_bin_index_channel, FN_REFLEN, index_file_withoutext);
    } else
      log_index_name = nullptr;

    if (relay_log.open_index_file(log_index_name, ln, true)) {
      LogErr(ERROR_LEVEL, ER_RPL_OPEN_INDEX_FILE_FAILED);
      return 1;
    }
    /*
      Remove entries of logs from the index that were deleted from
      the file system but not from the index due to a crash.
    */
    if (relay_log.remove_deleted_logs_from_index(true, false) == LOG_INFO_IO) {
      LogErr(ERROR_LEVEL, ER_RPL_FAILED_IN_RLI_INIT_INFO,
             "remove_deleted_logs_from_index()");
      return 1;
    }

    if (!gtid_retrieved_initialized) {
      /* Store the GTID of a transaction spanned in multiple relay log files */
      Gtid_monitoring_info *partial_trx = mi->get_gtid_monitoring_info();
      partial_trx->clear();
#ifndef DBUG_OFF
      get_sid_lock()->wrlock();
      gtid_set->dbug_print("set of GTIDs in relay log before initialization");
      get_sid_lock()->unlock();
#endif
      /*
        In the init_gtid_set below we pass the mi->transaction_parser.
        This will be useful to ensure that we only add a GTID to
        the Retrieved_Gtid_Set for fully retrieved transactions. Also, it will
        be useful to ensure the Retrieved_Gtid_Set behavior when auto
        positioning is disabled (we could have transactions spanning multiple
        relay log files in this case).
        We will skip this initialization if relay_log_recovery is set in order
        to save time, as neither the GTIDs nor the transaction_parser state
        would be useful when the relay log will be cleaned up later when calling
        init_recovery.
      */
      if (!is_relay_log_recovery && !gtid_retrieved_initialized &&
          !skip_received_gtid_set_recovery &&
          relay_log.init_gtid_sets(
              gtid_set, nullptr, opt_slave_sql_verify_checksum,
              true /*true=need lock*/, &mi->transaction_parser, partial_trx)) {
        LogErr(ERROR_LEVEL, ER_RPL_CANT_INITIALIZE_GTID_SETS_IN_RLI_INIT_INFO);
        return 1;
      }
      gtid_retrieved_initialized = true;
#ifndef DBUG_OFF
      get_sid_lock()->wrlock();
      gtid_set->dbug_print("set of GTIDs in relay log after initialization");
      get_sid_lock()->unlock();
#endif
    }
    /*
      Configures what object is used by the current log to store processed
      gtid(s). This is necessary in the MYSQL_BIN_LOG::MYSQL_BIN_LOG to
      correctly compute the set of previous gtids.
    */
    relay_log.set_previous_gtid_set_relaylog(gtid_set);
    /*
      note, that if open() fails, we'll still have index file open
      but a destructor will take care of that
    */

    mysql_mutex_t *log_lock = relay_log.get_log_lock();
    mysql_mutex_lock(log_lock);

    if (enable_raft_plugin) {
      if (relay_log.open_existing_binlog(opt_bin_logname, max_binlog_size)) {
        // NO_LINT_DEBUG
        sql_print_error(
            "Failed to open existing binlog called from "
            "Relay_log_info::rli_init_info().");
        LogErr(ERROR_LEVEL, ER_RPL_CANT_OPEN_LOG_IN_RLI_INIT_INFO);
        return 1;
      }
    } else {
      if (relay_log.open_binlog(
              ln, nullptr,
              (max_relay_log_size ? max_relay_log_size : max_binlog_size), true,
              true /*need_lock_index=true*/, true /*need_sid_lock=true*/,
              mi->get_mi_description_event())) {
        mysql_mutex_unlock(log_lock);
        LogErr(ERROR_LEVEL, ER_RPL_CANT_OPEN_LOG_IN_RLI_INIT_INFO);
        return 1;
      }
    }
    mysql_mutex_unlock(log_lock);
  }

  /*
   This checks if the repository was created before and thus there
   will be values to be read. Please, do not move this call after
   the handler->init_info().
 */
  if ((check_return = check_info()) == ERROR_CHECKING_REPOSITORY) {
    msg = "Error checking relay log repository";
    error = 1;
    goto err;
  }

  if (handler->init_info()) {
    msg = "Error reading relay log configuration";
    error = 1;
    goto err;
  }

  check_return = check_if_info_was_cleared(check_return);

  if (check_return & REPOSITORY_EXISTS) {
    if (read_info(handler)) {
      msg = "Error reading relay log configuration";
      error = 1;
      goto err;
    }

    if (clone_startup) {
      char *channel_name =
          (const_cast<Relay_log_info *>(mi->rli))->get_channel();
      bool is_group_replication_applier_channel =
          channel_map.is_group_replication_channel_name(channel_name);
      if (is_group_replication_applier_channel) {
        if (clear_info()) {
          msg =
              "Error cleaning relay log configuration for group replication "
              "after clone";
          error = 1;
          goto err;
        }
        if (Rpl_info_factory::reset_workers(this)) {
          msg =
              "Error cleaning relay log worker configuration for group "
              "replication after clone";
          error = 1;
          goto err;
        }
        check_return = REPOSITORY_CLEARED;
      } else {
        if (!is_relay_log_recovery) {
          LogErr(WARNING_LEVEL, ER_RPL_RELAY_LOG_RECOVERY_INFO_AFTER_CLONE,
                 channel_name);
          // After a clone if we detect information is present we always invoke
          // relay log recovery. Not doing so would probably mean failure at
          // initialization due to missing relay log files.
          if (init_recovery(mi)) {
            msg = "Error on the relay log recovery after a clone operation";
            error = 1;
            goto err;
          }
        }
      }
    }
  }

  if (check_return == REPOSITORY_DOES_NOT_EXIST ||  // Hasn't been initialized
      check_return == REPOSITORY_CLEARED  // Was initialized but was RESET
  ) {
    /* Init relay log with first entry in the relay index file */
    if (reset_group_relay_log_pos(&msg)) {
      error = 1;
      goto err;
    }
    group_master_log_name[0] = 0;
    group_master_log_pos = 0;
  } else {
    if (is_relay_log_recovery && init_recovery(mi)) {
      error = 1;
      goto err;
    }

    if (is_group_relay_log_name_invalid(&msg)) {
      LogErr(ERROR_LEVEL, ER_RPL_MTS_RECOVERY_CANT_OPEN_RELAY_LOG,
             group_relay_log_name, std::to_string(group_relay_log_pos).c_str());
      error = 1;
      goto err;
    }
  }

  inited = true;
  error_on_rli_init_info = false;
  if (flush_info(true)) {
    msg = "Error reading relay log configuration";
    error = 1;
    goto err;
  }

  if (count_relay_log_space()) {
    msg = "Error counting relay log space";
    error = 1;
    goto err;
  }

  /*
    In case of MTS the recovery is deferred until the end of
    load_mi_and_rli_from_repositories.
  */
  if (!mi->rli->mts_recovery_group_cnt) is_relay_log_recovery = false;
  return error;

err:
  handler->end_info();
  inited = false;
  error_on_rli_init_info = true;
  if (msg) LogErr(ERROR_LEVEL, ER_RPL_RLI_INIT_INFO_MSG, msg);
  relay_log.close(LOG_CLOSE_INDEX | LOG_CLOSE_STOP_EVENT,
                  true /*need_lock_log=true*/, true /*need_lock_index=true*/);
  return error;
}

int Relay_log_info::remove_logged_gtids(
    const std::vector<std::string> &trimmed_gtids) {
  DBUG_TRACE;
  global_sid_lock->assert_some_lock();

  if (trimmed_gtids.empty()) RETURN_OK;

  Gtid gtid;
  for (const auto &trimmed_gtid : trimmed_gtids) {
    if (gtid.parse(global_sid_map, trimmed_gtid.c_str()) != RETURN_STATUS_OK) {
      // NO_LINT_DEBUG
      sql_print_error("Failed to parse gtid %s", trimmed_gtid.c_str());
      RETURN_REPORTED_ERROR;
    }

    if (gtid.sidno > 0) {
      /* Remove Gtid from logged_gtid set. */
      DBUG_PRINT("info",
                 ("Removing gtid(sidno:%d, gno:%lld) from rli logged gtids",
                  gtid.sidno, gtid.gno));
      get_sid_lock()->wrlock();
      gtid_set->_remove_gtid(gtid);
      get_sid_lock()->unlock();
    }
  }

  RETURN_OK;
}

void Relay_log_info::end_info() {
  DBUG_TRACE;

  error_on_rli_init_info = false;
  if (!inited) return;

  handler->end_info();

  inited = false;
  relay_log.close(LOG_CLOSE_INDEX | LOG_CLOSE_STOP_EVENT,
                  true /*need_lock_log=true*/, true /*need_lock_index=true*/);
  relay_log.harvest_bytes_written(this, true /*need_log_space_lock=true*/);
  /*
    Delete the slave's temporary tables from memory.
    In the future there will be other actions than this, to ensure persistance
    of slave's temp tables after shutdown.
  */
  close_temporary_tables();
}

int Relay_log_info::flush_current_log() {
  DBUG_TRACE;

  /* When we come to this place in code, relay log may or not be initialized; */
  if (relay_log.flush()) return 2;

  return 0;
}

void Relay_log_info::set_master_info(Master_info *info) { mi = info; }

/**
  Stores the file and position where the execute-slave thread are in the
  relay log:

    - As this is only called by the slave thread or on STOP SLAVE, with the
      log_lock grabbed and the slave thread stopped, we don't need to have
      a lock here.
    - If there is an active transaction, then we don't update the position
      in the relay log.  This is to ensure that we re-execute statements
      if we die in the middle of an transaction that was rolled back.
    - As a transaction never spans binary logs, we don't have to handle the
      case where we do a relay-log-rotation in the middle of the transaction.
      If this would not be the case, we would have to ensure that we
      don't delete the relay log file where the transaction started when
      we switch to a new relay log file.

  @retval  0   ok,
  @retval  1   write error, otherwise.
*/

/**
  Store the file and position where the slave's SQL thread are in the
  relay log.

  Notes:

  - This function should be called either from the slave SQL thread,
    or when the slave thread is not running.  (It reads the
    group_{relay|master}_log_{pos|name} and delay fields in the rli
    object.  These may only be modified by the slave SQL thread or by
    a client thread when the slave SQL thread is not running.)

  - If there is an active transaction, then we do not update the
    position in the relay log.  This is to ensure that we re-execute
    statements if we die in the middle of an transaction that was
    rolled back.

  - As a transaction never spans binary logs, we don't have to handle
    the case where we do a relay-log-rotation in the middle of the
    transaction.  If transactions could span several binlogs, we would
    have to ensure that we do not delete the relay log file where the
    transaction started before switching to a new relay log file.

  - Error can happen if writing to file fails or if flushing the file
    fails.

  @todo Change the log file information to a binary format to avoid
  calling longlong2str.

  @return 0 on success, 1 on error.
*/
int Relay_log_info::flush_info(const bool force) {
  DBUG_TRACE;

  if (!inited) return 0;

  /*
    We update the sync_period at this point because only here we
    now that we are handling a relay log info. This needs to be
    update every time we call flush because the option maybe
    dinamically set.
  */
  mysql_mutex_lock(&mts_temp_table_LOCK);
  handler->set_sync_period(sync_relayloginfo_period);

  if (write_info(handler)) goto err;

  if (handler->flush_info(force || force_flush_postponed_due_to_split_trans))
    goto err;

  force_flush_postponed_due_to_split_trans = false;
  mysql_mutex_unlock(&mts_temp_table_LOCK);
  return 0;

err:
  LogErr(ERROR_LEVEL, ER_RPL_ERROR_WRITING_RELAY_LOG_CONFIGURATION);
  mysql_mutex_unlock(&mts_temp_table_LOCK);
  return 1;
}

enum_return_check Relay_log_info::check_if_info_was_cleared(
    const enum_return_check &previous_result) const {
  enum_return_check result = previous_result;

  if (result == REPOSITORY_EXISTS) {
    char number_of_lines[FN_REFLEN] = {0};

    if (this->handler->prepare_info_for_read() ||
        !!this->handler->get_info(number_of_lines, sizeof(number_of_lines), ""))
      return ERROR_CHECKING_REPOSITORY;

    char *first_non_digit{nullptr};
    int lines = strtoul(number_of_lines, &first_non_digit, 10);

    if (number_of_lines[0] != '\0' && *first_non_digit == '\0' &&
        lines >= LINES_IN_RELAY_LOG_INFO_WITH_REQUIRE_ROW_FORMAT) {
      char log_name[FN_REFLEN] = {0};

      if (this->handler->get_info(log_name, sizeof(log_name), "") ==
          Rpl_info_handler::enum_field_get_status::FAILURE)
        return ERROR_CHECKING_REPOSITORY;

      if (log_name[0] == '\0') return REPOSITORY_CLEARED;
    }
  }
  return result;
}

bool Relay_log_info::clear_info() {
  this->handler->init_info();

  if (this->handler->prepare_info_for_write() ||
      this->handler->set_info((int)MAXIMUM_LINES_IN_RELAY_LOG_INFO_FILE) ||
      this->handler->set_info(nullptr) || this->handler->set_info(nullptr) ||
      this->handler->set_info(nullptr) || this->handler->set_info(nullptr) ||
      this->handler->set_info(nullptr) || this->handler->set_info(nullptr) ||
      this->handler->set_info(nullptr) ||
      this->handler->set_info(this->channel))
    return true;

  if (this->m_privilege_checks_username.length() != 0) {
    if (this->handler->set_info(this->m_privilege_checks_username.c_str()))
      return true;
  } else {
    if (this->handler->set_info(nullptr)) {
      return true;
    }
  }
  if (this->m_privilege_checks_hostname.length() != 0) {
    if (this->handler->set_info(this->m_privilege_checks_hostname.c_str()))
      return true;
  } else {
    if (this->handler->set_info(nullptr)) return true;
  }

  if (this->handler->set_info(this->m_require_row_format)) return true;

  if (DBUG_EVALUATE_IF("rpl_rli_clear_info_error", true, false) ||
      this->handler->set_info((ulong)this->m_require_table_primary_key_check))
    return true;

  if (this->handler->flush_info(true)) return true;

  this->group_relay_log_name[0] = '\0';
  this->group_relay_log_pos = 0;
  this->group_master_log_name[0] = '\0';
  this->group_master_log_pos = 0;
  this->sql_delay = 0;
  this->recovery_parallel_workers = 0;
  this->internal_id = 1;

  return false;
}

size_t Relay_log_info::get_number_info_rli_fields() {
  return sizeof(info_rli_fields) / sizeof(info_rli_fields[0]);
}

void Relay_log_info::set_nullable_fields(MY_BITMAP *nullable_fields) {
  bitmap_init(nullable_fields, nullptr,
              Relay_log_info::get_number_info_rli_fields());
  bitmap_set_all(nullable_fields);       // All fields may be NULL except for
  bitmap_clear_bit(nullable_fields, 0);  // NUMBER_OF_LINES and
  bitmap_clear_bit(nullable_fields, 8);  // CHANNEL_NAME
}

void Relay_log_info::start_sql_delay(time_t delay_end) {
  mysql_mutex_assert_owner(&data_lock);
  sql_delay_end = delay_end;
  THD_STAGE_INFO(info_thd, stage_sql_thd_waiting_until_delay);
}

bool Relay_log_info::read_info(Rpl_info_handler *from) {
  int lines = 0;
  char *first_non_digit = nullptr;
  ulong temp_group_relay_log_pos = 0;
  ulong temp_group_master_log_pos = 0;
  int temp_sql_delay = 0;
  int temp_internal_id = internal_id;
  int temp_require_row_format = 0;
  ulong temp_require_table_primary_key_check = Relay_log_info::PK_CHECK_STREAM;
  Rpl_info_handler::enum_field_get_status status{
      Rpl_info_handler::enum_field_get_status::FAILURE};

  DBUG_TRACE;

  /*
    Should not read RLI from file in client threads. Client threads
    only use RLI to execute BINLOG statements.

    @todo Uncomment the following assertion. Currently,
    Relay_log_info::init() is called from init_master_info() before
    the THD object Relay_log_info::sql_thd is created. That means we
    cannot call belongs_to_client() since belongs_to_client()
    dereferences Relay_log_info::sql_thd. So we need to refactor
    slightly: the THD object should be created by Relay_log_info
    constructor (or passed to it), so that we are guaranteed that it
    exists at this point. /Sven
  */
  // DBUG_ASSERT(!belongs_to_client());

  /*
    Starting from 5.1.x, relay-log.info has a new format. Now, its
    first line contains the number of lines in the file. By reading
    this number we can determine which version our master.info comes
    from. We can't simply count the lines in the file, since
    versions before 5.1.x could generate files with more lines than
    needed. If first line doesn't contain a number, or if it
    contains a number less than LINES_IN_RELAY_LOG_INFO_WITH_DELAY,
    then the file is treated like a file from pre-5.1.x version.
    There is no ambiguity when reading an old master.info: before
    5.1.x, the first line contained the binlog's name, which is
    either empty or has an extension (contains a '.'), so can't be
    confused with an integer.

    So we're just reading first line and trying to figure which
    version is this.
  */

  /*
    The first row is temporarily stored in mi->master_log_name, if
    it is line count and not binlog name (new format) it will be
    overwritten by the second row later.
  */
  if (from->prepare_info_for_read() ||
      !!from->get_info(group_relay_log_name, sizeof(group_relay_log_name), ""))
    return true;

  lines = strtoul(group_relay_log_name, &first_non_digit, 10);

  if (group_relay_log_name[0] != '\0' && *first_non_digit == '\0' &&
      lines >= LINES_IN_RELAY_LOG_INFO_WITH_DELAY) {
    /* Seems to be new format => read group relay log name */
    status =
        from->get_info(group_relay_log_name, sizeof(group_relay_log_name), "");
    if (status == Rpl_info_handler::enum_field_get_status::FAILURE) return true;
    if (status == Rpl_info_handler::enum_field_get_status::FIELD_VALUE_IS_NULL)
      group_relay_log_name[0] = '\0';
  } else
    DBUG_PRINT("info", ("relay_log_info file is in old format."));

  status =
      from->get_info(&temp_group_relay_log_pos, (ulong)BIN_LOG_HEADER_SIZE);
  if (status == Rpl_info_handler::enum_field_get_status::FAILURE) return true;

  status =
      from->get_info(group_master_log_name, sizeof(group_master_log_name), "");
  if (status == Rpl_info_handler::enum_field_get_status::FAILURE) return true;
  if (status == Rpl_info_handler::enum_field_get_status::FIELD_VALUE_IS_NULL)
    group_master_log_name[0] = '\0';

  status = from->get_info(&temp_group_master_log_pos, 0UL);
  if (status == Rpl_info_handler::enum_field_get_status::FAILURE) return true;

  if (lines >= LINES_IN_RELAY_LOG_INFO_WITH_DELAY) {
    status = from->get_info(&temp_sql_delay, 0);
    if (status == Rpl_info_handler::enum_field_get_status::FAILURE) return true;
  }

  if (lines >= LINES_IN_RELAY_LOG_INFO_WITH_WORKERS) {
    status = from->get_info(&recovery_parallel_workers, 0UL);
    if (status == Rpl_info_handler::enum_field_get_status::FAILURE) return true;
  }

  if (lines >= LINES_IN_RELAY_LOG_INFO_WITH_ID) {
    status = from->get_info(&temp_internal_id, 1);
    if (status == Rpl_info_handler::enum_field_get_status::FAILURE) return true;
  }

  if (lines >= LINES_IN_RELAY_LOG_INFO_WITH_CHANNEL) {
    /* the default value is empty string"" */
    if (!!from->get_info(channel, sizeof(channel), "")) return true;
  }

  /*
   * Here +4 is used to accomodate string if debug point
   * `simulate_priv_check_username_above_limit` is set in test.
   */
  char temp_privilege_checks_username[PRIV_CHECKS_USERNAME_LENGTH + 4] = {0};
  char *username = nullptr;
  if (lines >= LINES_IN_RELAY_LOG_INFO_WITH_PRIV_CHECKS_USERNAME) {
    status = from->get_info(temp_privilege_checks_username,
                            PRIV_CHECKS_USERNAME_LENGTH + 1, nullptr);
    if (status == Rpl_info_handler::enum_field_get_status::FAILURE) return true;
    if (status == Rpl_info_handler::enum_field_get_status::FIELD_VALUE_NOT_NULL)
      username = temp_privilege_checks_username;
  }

  /*
   * Here +4 is used to accomodate string if debug point
   * `simulate_priv_check_hostname_above_limit` is set in test.
   */
  char temp_privilege_checks_hostname[PRIV_CHECKS_HOSTNAME_LENGTH + 4] = {0};
  char *hostname = nullptr;
  if (lines >= LINES_IN_RELAY_LOG_INFO_WITH_PRIV_CHECKS_HOSTNAME) {
    status = from->get_info(temp_privilege_checks_hostname,
                            PRIV_CHECKS_HOSTNAME_LENGTH + 1, nullptr);
    if (status == Rpl_info_handler::enum_field_get_status::FAILURE) return true;
    if (status == Rpl_info_handler::enum_field_get_status::FIELD_VALUE_NOT_NULL)
      hostname = temp_privilege_checks_hostname;
  }

  if (lines >= LINES_IN_RELAY_LOG_INFO_WITH_REQUIRE_ROW_FORMAT) {
    if (!!from->get_info(&temp_require_row_format, 0)) return true;
  } else {
    if (channel_map.is_group_replication_channel_name(channel))
      temp_require_row_format = 1;
  }
  m_require_row_format = temp_require_row_format;

  if (lines >= LINES_IN_RELAY_LOG_INFO_WITH_REQUIRE_TABLE_PRIMARY_KEY_CHECK) {
    if (!!from->get_info(&temp_require_table_primary_key_check, 1)) return true;
  }
  if (temp_require_table_primary_key_check < PK_CHECK_STREAM ||
      temp_require_table_primary_key_check > Relay_log_info::PK_CHECK_OFF)
    return true;
  m_require_table_primary_key_check =
      static_cast<Relay_log_info::enum_require_table_primary_key>(
          temp_require_table_primary_key_check);

  group_relay_log_pos = temp_group_relay_log_pos;
  group_master_log_pos = temp_group_master_log_pos;
  sql_delay = (int32)temp_sql_delay;
  internal_id = (uint)temp_internal_id;

  DBUG_EXECUTE_IF("simulate_priv_check_username_above_limit", {
    strcpy(temp_privilege_checks_username,
           "repli_priv_checks_user_more_than_32");
    username = temp_privilege_checks_username;
  });

  DBUG_EXECUTE_IF("simulate_priv_check_hostname_above_limit", {
    strcpy(
        temp_privilege_checks_hostname,
        "replication_privilege_checks_hostname_more_than_255_replication_"
        "privilege_checks_hostname_more_than_255_replication_privilege_checks_"
        "hostname_more_than_255_replication_privilege_checks_hostname_more_"
        "than_255_replication_privilege_checks_hostname_more_than255");
    hostname = temp_privilege_checks_hostname;
  });
  enum_priv_checks_status error = set_privilege_checks_user(username, hostname);
  if (!!error) {
    set_privilege_checks_user_corrupted(true);
    report_privilege_check_error(WARNING_LEVEL, error, false, channel, username,
                                 hostname);
    return true;
  }

  return false;
}

bool Relay_log_info::set_info_search_keys(Rpl_info_handler *to) {
  DBUG_TRACE;

  if (to->set_info(LINES_IN_RELAY_LOG_INFO_WITH_CHANNEL, channel)) return true;

  return false;
}

bool Relay_log_info::write_info(Rpl_info_handler *to) {
  DBUG_TRACE;

  /*
    @todo Uncomment the following assertion. See todo in
    Relay_log_info::read_info() for details. /Sven
  */
  // DBUG_ASSERT(!belongs_to_client());

  if (to->prepare_info_for_write()) return true;

  if (to->get_rpl_info_type() != INFO_REPOSITORY_FILE) {
    if (to->set_info((int)MAXIMUM_LINES_IN_RELAY_LOG_INFO_FILE) ||
        to->set_info(group_relay_log_name) ||
        to->set_info((ulong)group_relay_log_pos) ||
        to->set_info(group_master_log_name) ||
        to->set_info((ulong)group_master_log_pos) ||
        to->set_info((int)sql_delay) ||
        to->set_info(recovery_parallel_workers) ||
        to->set_info((int)internal_id) || to->set_info(channel))
      return true;
  } else {
    if (to->set_info(LINES_IN_RELAY_LOG_INFO_WITH_CHANNEL +
                         1, /* 9 params after the format string */
                     "%d\n%s\n%lu\n%s\n%lu\n%d\n%lu\n%d\n%s\n",
                     (int)MAXIMUM_LINES_IN_RELAY_LOG_INFO_FILE,
                     group_relay_log_name, (ulong)group_relay_log_pos,
                     group_master_log_name, (ulong)group_master_log_pos,
                     (int)sql_delay, recovery_parallel_workers,
                     (int)internal_id, channel))
      return true;
  }

  if (m_privilege_checks_username.length()) {
    if (to->set_info(m_privilege_checks_username.c_str())) return true;
  } else {
#ifndef DBUG_OFF
    if (DBUG_EVALUATE_IF("simulate_priv_check_user_nullptr_t", true, false)) {
      const char *null_char{nullptr};
      if (to->set_info(null_char)) return true;
    } else
#endif
        if (to->set_info(nullptr)) {
      return true;
    }
  }
  if (m_privilege_checks_hostname.length()) {
    if (to->set_info(m_privilege_checks_hostname.c_str())) return true;
  } else {
#ifndef DBUG_OFF
    if (DBUG_EVALUATE_IF("simulate_priv_check_user_nullptr_t", true, false)) {
      const uchar *null_char{nullptr};
      if (to->set_info(null_char, 0)) return true;
    } else
#endif
        if (to->set_info(nullptr))
      return true;
  }

  if (to->set_info((int)m_require_row_format)) {
    return true; /* purecov: inspected */
  }

  if (to->set_info((ulong)m_require_table_primary_key_check)) {
    return true; /* purecov: inspected */
  }
  return false;
}

/**
   The method is run by SQL thread/MTS Coordinator.
   It replaces the current FD event with a new one.
   A version adaptation routine is invoked for the new FD
   to align the slave applier execution context with the master version.

   Since FD are shared by Coordinator and Workers in the MTS mode,
   deletion of the old FD is done through decrementing its usage counter.
   The destructor runs when the later drops to zero,
   also see @c Slave_worker::set_rli_description_event().
   The usage counter of the new FD is incremented.

   Although notice that MTS worker runs it, inefficiently (see assert),
   once at its destruction time.

   @param  fe Pointer to be installed into execution context
           FormatDescriptor event

   @return 1 if an error was encountered, 0 otherwise.
*/

int Relay_log_info::set_rli_description_event(
    Format_description_log_event *fe) {
  DBUG_TRACE;
  DBUG_ASSERT(!info_thd || !is_mts_worker(info_thd) || !fe);

  if (fe) {
    ulong fe_version = adapt_to_master_version(fe);

    if (info_thd) {
      /* @see rpl_rli_pdb.h:Slave_worker::set_rli_description_event for a
         detailed explanation on the following code block's logic. */
      if (info_thd->variables.gtid_next.type == AUTOMATIC_GTID ||
          info_thd->variables.gtid_next.type == UNDEFINED_GTID) {
        bool in_active_multi_stmt =
            info_thd->in_active_multi_stmt_transaction();

        if (!is_in_group() && !in_active_multi_stmt) {
          DBUG_PRINT("info",
                     ("Setting gtid_next.type to NOT_YET_DETERMINED_GTID"));
          info_thd->variables.gtid_next.set_not_yet_determined();
        } else if (in_active_multi_stmt) {
          my_error(ER_VARIABLE_NOT_SETTABLE_IN_TRANSACTION, MYF(0),
                   "gtid_next");
          return 1;
        }
      }

      if (is_parallel_exec() && fe_version > 0) {
        /*
          Prepare for workers' adaption to a new FD version. Workers
          will see notification through scheduling of a first event of
          a new post-new-FD.
        */
        for (Slave_worker **it = workers.begin(); it != workers.end(); ++it)
          (*it)->fd_change_notified = false;
      }
    }
  }
  if (rli_description_event &&
      --rli_description_event->atomic_usage_counter == 0)
    delete rli_description_event;
#ifndef DBUG_OFF
  else
    /* It must be MTS mode when the usage counter greater than 1. */
    DBUG_ASSERT(!rli_description_event || is_parallel_exec());
#endif
  rli_description_event = fe;
  if (rli_description_event) ++rli_description_event->atomic_usage_counter;

  return 0;
}

struct st_feature_version {
  /*
    The enum must be in the version non-descending top-down order,
    the last item formally corresponds to highest possible server
    version (never reached, thereby no adapting actions here);
    enumeration starts from zero.
  */
  enum {
    WL6292_TIMESTAMP_EXPLICIT_DEFAULT = 0,
    _END_OF_LIST  // always last
  } item;
  /*
    Version where the feature is introduced.
  */
  uchar version_split[3];
  /*
    Action to perform when according to FormatDescriptor event Master
    is found to be feature-aware while previously it has *not* been.
  */
  void (*upgrade)(THD *);
  /*
    Action to perform when according to FormatDescriptor event Master
    is found to be feature-*un*aware while previously it has been.
  */
  void (*downgrade)(THD *);
};

static void wl6292_upgrade_func(THD *thd) {
  thd->variables.explicit_defaults_for_timestamp = false;
  if (global_system_variables.explicit_defaults_for_timestamp)
    thd->variables.explicit_defaults_for_timestamp = true;

  return;
}

static void wl6292_downgrade_func(THD *thd) {
  if (global_system_variables.explicit_defaults_for_timestamp)
    thd->variables.explicit_defaults_for_timestamp = false;

  return;
}

/**
   Sensitive to Master-vs-Slave version difference features
   should be listed in the version non-descending order.
*/
static st_feature_version s_features[] = {
    // order is the same as in the enum
    {st_feature_version::WL6292_TIMESTAMP_EXPLICIT_DEFAULT,
     {5, 6, 6},
     wl6292_upgrade_func,
     wl6292_downgrade_func},
    {st_feature_version::_END_OF_LIST, {255, 255, 255}, nullptr, nullptr}};

/**
   The method computes the incoming "master"'s FD server version and that
   of the currently installed (if ever) rli_description_event, to
   invoke more specific method to compare the two and adapt slave applier
   execution context to the new incoming master's version.

   This method is specifically for STS applier/MTS Coordinator as well as
   for a user thread applying binlog events.

   @param  fdle  a pointer to new Format Description event that is being
                 set up a new execution context.
   @return 0                when the versions are equal,
           master_version   otherwise
*/
ulong Relay_log_info::adapt_to_master_version(
    Format_description_log_event *fdle) {
  ulong master_version, current_version, slave_version;

  slave_version = version_product(slave_version_split);
  /* When rli_description_event is uninitialized yet take the slave's version */
  master_version = !fdle ? slave_version : fdle->get_product_version();
  current_version = !rli_description_event
                        ? slave_version
                        : rli_description_event->get_product_version();
  return adapt_to_master_version_updown(master_version, current_version);
}

/**
  The method compares two supplied versions and carries out down- or
  up- grade customization of execution context of the slave applier
  (thd).

  The method is invoked in the STS case through
  Relay_log_info::adapt_to_master_version() right before a new master
  FD is installed into the applier execution context; in the MTS
  case it's done by the Worker when it's assigned with a first event
  after the latest new FD has been installed.

  Comparison of the current (old, existing) and the master (new,
  incoming) versions yields adaptive actions.
  To explain that, let's denote V_0 as the current, and the master's
  one as V_1.
  In the downgrade case (V_1 < V_0) a server feature that is undefined
  in V_1 but is defined starting from some V_f of [V_1 + 1, V_0] range
  (+1 to mean V_1 excluded) are invalidated ("removed" from execution context)
  by running so called here downgrade action.
  Conversely in the upgrade case a feature defined in [V_0 + 1, V_1] range
  is validated ("added" to execution context) by running its upgrade action.
  A typical use case showing how adaptive actions are necessary for the slave
  applier is when the master version is lesser than the slave's one.
  In such case events generated on the "older" master may need to be applied
  in their native server context. And such context can be provided by downgrade
  actions.
  Conversely, when the old master events are run out and a newer master's events
  show up for applying, the execution context will be upgraded through
  the namesake actions.

  Notice that a relay log may have two FD events, one the slave local
  and the other from the Master. As there's no concern for the FD
  originator this leads to two adapt_to_master_version() calls.
  It's not harmful as can be seen from the following example.
  Say the currently installed FD's version is
  V_m, then at relay-log rotation the following transition takes
  place:

     V_m  -adapt-> V_s -adapt-> V_m.

  here and further `m' subscript stands for the master, `s' for the slave.
  It's clear that in this case an ineffective V_m -> V_m transition occurs.

  At composing downgrade/upgrade actions keep in mind that the slave applier
  version transition goes the following route:
  The initial version is that of the slave server (V_ss).
  It changes to a magic 4.0 at the slave relay log initialization.
  In the following course versions are extracted from each FD read out,
  regardless of what server generated it. Here is a typical version
  transition sequence underscored with annotation:

   V_ss -> 4.0 -> V(FD_s^1) -> V(FD_m^2)   --->   V(FD_s^3) -> V(FD_m^4)  ...

    ----------     -----------------     --------  ------------------     ---
     bootstrap       1st relay log       rotation      2nd log            etc

  The upper (^) subscipt enumerates Format Description events, V(FD^i) stands
  for a function extrating the version data from the i:th FD.

  There won't be any action to execute when info_thd is undefined,
  e.g at bootstrap.

  @param  master_version   an upcoming new version
  @param  current_version  the current version
  @return 0                when the new version is equal to the current one,
          master_version   otherwise
*/
ulong Relay_log_info::adapt_to_master_version_updown(ulong master_version,
                                                     ulong current_version) {
  THD *thd = info_thd;
  /*
    When the SQL thread or MTS Coordinator executes this method
    there's a constraint on current_version argument.
  */
  DBUG_ASSERT(
      !thd || thd->rli_fake != nullptr ||
      thd->system_thread == SYSTEM_THREAD_SLAVE_WORKER ||
      (thd->system_thread == SYSTEM_THREAD_SLAVE_SQL &&
       (!rli_description_event ||
        current_version == rli_description_event->get_product_version())));

  if (master_version == current_version)
    return 0;
  else if (!thd)
    return master_version;

  bool downgrade = master_version < current_version;
  /*
   find item starting from and ending at for which adaptive actions run
   for downgrade or upgrade branches.
   (todo: convert into bsearch when number of features will grow significantly)
 */
  long i, i_first = st_feature_version::_END_OF_LIST, i_last = i_first;

  for (i = 0; i < st_feature_version::_END_OF_LIST; i++) {
    ulong ver_f = version_product(s_features[i].version_split);

    if ((downgrade ? master_version : current_version) < ver_f &&
        i_first == st_feature_version::_END_OF_LIST)
      i_first = i;
    if ((downgrade ? current_version : master_version) < ver_f) {
      i_last = i;
      DBUG_ASSERT(i_last >= i_first);
      break;
    }
  }

  /*
     actions, executed in version non-descending st_feature_version order
  */
  for (i = i_first; i < i_last; i++) {
    /* Run time check of the st_feature_version items ordering */
    DBUG_ASSERT(!i || version_product(s_features[i - 1].version_split) <=
                          version_product(s_features[i].version_split));

    DBUG_ASSERT((downgrade ? master_version : current_version) <
                    version_product(s_features[i].version_split) &&
                (downgrade ? current_version
                           : master_version >=
                                 version_product(s_features[i].version_split)));

    if (downgrade && s_features[i].downgrade) {
      s_features[i].downgrade(thd);
    } else if (s_features[i].upgrade) {
      s_features[i].upgrade(thd);
    }
  }

  return master_version;
}

void Relay_log_info::relay_log_number_to_name(uint number,
                                              char name[FN_REFLEN + 1]) {
  char *str = nullptr;
  char relay_bin_channel[FN_REFLEN + 1];
  const char *relay_log_basename_channel = add_channel_to_relay_log_name(
      relay_bin_channel, FN_REFLEN + 1, relay_log_basename);

  /* str points to closing null of relay log basename channel */
  str = strmake(name, relay_log_basename_channel, FN_REFLEN + 1);
  *str++ = '.';
  sprintf(str, "%06u", number);
}

uint Relay_log_info::relay_log_name_to_number(const char *name) {
  return static_cast<uint>(atoi(fn_ext(name) + 1));
}

bool is_mts_db_partitioned(Relay_log_info *rli) {
  return (rli->current_mts_submode->get_type() == MTS_PARALLEL_TYPE_DB_NAME);
}

bool is_mts_parallel_type_logical_clock(const Relay_log_info *rli) {
  return (rli->current_mts_submode->get_type() ==
          MTS_PARALLEL_TYPE_LOGICAL_CLOCK);
}

bool is_mts_parallel_type_dependency(const Relay_log_info *rli) {
  return (rli->current_mts_submode->get_type() == MTS_PARALLEL_TYPE_DEPENDENCY);
}

const char *Relay_log_info::get_for_channel_str(bool upper_case) const {
  if (rli_fake || mi == nullptr)
    return "";
  else
    return mi->get_for_channel_str(upper_case);
}

enum_return_status Relay_log_info::add_gtid_set(const Gtid_set *gtid_set) {
  DBUG_TRACE;

  get_sid_lock()->wrlock();
  enum_return_status return_status = this->gtid_set->add_gtid_set(gtid_set);
  get_sid_lock()->unlock();

  return return_status;
}

const char *Relay_log_info::get_until_log_name() {
  if (until_condition == UNTIL_MASTER_POS ||
      until_condition == UNTIL_RELAY_POS) {
    DBUG_ASSERT(until_option != nullptr);
    return ((Until_position *)until_option)->get_until_log_name();
  }
  return "";
}

my_off_t Relay_log_info::get_until_log_pos() {
  if (until_condition == UNTIL_MASTER_POS ||
      until_condition == UNTIL_RELAY_POS) {
    DBUG_ASSERT(until_option != nullptr);
    return ((Until_position *)until_option)->get_until_log_pos();
  }
  return 0;
}

/**
 * Update the last master timestamp seen by the slave
 * Last master timestamp is used to calculate lag (seconds/milli-seconds behind
 * master).
 */
void Relay_log_info::set_last_master_timestamp(time_t ts, ulonglong ts_millis) {
  auto now = std::chrono::system_clock::now().time_since_epoch();
  auto now_sec = std::chrono::duration_cast<std::chrono::seconds>(now).count();
  auto now_msec =
      std::chrono::duration_cast<std::chrono::milliseconds>(now).count();

  /*
    Note that we only skip assigning ts to last_master_timestamp when
    ts is smaller than last_master_timestamp to avoid a sudden spike on
    second behind master. If ts is very big, say bigger than now(), we
    will assign the current time to last_master_timestamp instead.
    Same for last_master_timestamp_millis
  */
  if (ts > last_master_timestamp) {
    // penultimate_master_timestamp= last_master_timestamp;
    last_master_timestamp = std::min(ts, now_sec);
    mysql_bin_log.last_master_timestamp.store(last_master_timestamp);
  }

  if (ts_millis > last_master_timestamp_millis)
    last_master_timestamp_millis = std::min(ts_millis, (ulonglong)now_msec);
}

int Relay_log_info::init_until_option(THD *thd,
                                      const LEX_MASTER_INFO *master_param) {
  DBUG_TRACE;
  int ret = 0;
  Until_option *option = nullptr;

  until_condition = UNTIL_NONE;
  clear_until_option();

  try {
    if (master_param->pos) {
      Until_master_position *until_mp = nullptr;

      if (master_param->relay_log_pos) return ER_BAD_SLAVE_UNTIL_COND;

      option = until_mp = new Until_master_position(this);
      until_condition = UNTIL_MASTER_POS;
      ret = until_mp->init(master_param->log_file_name, master_param->pos);
    } else if (master_param->relay_log_pos) {
      Until_relay_position *until_rp = nullptr;

      if (master_param->pos) return ER_BAD_SLAVE_UNTIL_COND;

      option = until_rp = new Until_relay_position(this);
      until_condition = UNTIL_RELAY_POS;
      ret = until_rp->init(master_param->relay_log_name,
                           master_param->relay_log_pos);
    } else if (master_param->gtid) {
      Until_gtids *until_g = nullptr;

      if (LEX_MASTER_INFO::UNTIL_SQL_BEFORE_GTIDS ==
          master_param->gtid_until_condition) {
        option = until_g = new Until_before_gtids(this);
        until_condition = UNTIL_SQL_BEFORE_GTIDS;
      } else {
        DBUG_ASSERT(LEX_MASTER_INFO::UNTIL_SQL_AFTER_GTIDS ==
                    master_param->gtid_until_condition);

        option = until_g = new Until_after_gtids(this);
        until_condition = UNTIL_SQL_AFTER_GTIDS;
        if (opt_slave_parallel_workers != 0) {
          opt_slave_parallel_workers = 0;
          push_warning_printf(
              thd, Sql_condition::SL_NOTE, ER_MTS_FEATURE_IS_NOT_SUPPORTED,
              ER_THD(thd, ER_MTS_FEATURE_IS_NOT_SUPPORTED), "UNTIL condtion",
              "Slave is started in the sequential execution mode.");
        }
      }
      ret = until_g->init(master_param->gtid);
    } else if (master_param->until_after_gaps) {
      Until_mts_gap *until_mg = nullptr;

      option = until_mg = new Until_mts_gap(this);
      until_condition = UNTIL_SQL_AFTER_MTS_GAPS;
      until_mg->init();
    } else if (master_param->view_id) {
      Until_view_id *until_vi = nullptr;

      option = until_vi = new Until_view_id(this);
      until_condition = UNTIL_SQL_VIEW_ID;
      ret = until_vi->init(master_param->view_id);
    }
  } catch (...) {
    return ER_OUTOFMEMORY;
  }

  if (until_condition == UNTIL_MASTER_POS ||
      until_condition == UNTIL_RELAY_POS) {
    /* Issuing warning then started without --skip-slave-start */
    if (!opt_skip_slave_start)
      push_warning(thd, Sql_condition::SL_NOTE, ER_MISSING_SKIP_SLAVE,
                   ER_THD(thd, ER_MISSING_SKIP_SLAVE));
  }

  mysql_mutex_lock(&data_lock);
  until_option = option;
  mysql_mutex_unlock(&data_lock);
  return ret;
}

void Relay_log_info::detach_engine_ha_data(THD *thd) {
  is_engine_ha_data_detached = true;
  /*
    In case of slave thread applier or processing binlog by client,
    detach the engine ha_data ("native" engine transaction)
    in favor of dynamically created.
  */
  plugin_foreach(thd, detach_native_trx, MYSQL_STORAGE_ENGINE_PLUGIN, nullptr);
}

void Relay_log_info::reattach_engine_ha_data(THD *thd) {
  is_engine_ha_data_detached = false;
  /*
    In case of slave thread applier or processing binlog by client,
    reattach the engine ha_data ("native" engine transaction)
    in favor of dynamically created.
  */
  plugin_foreach(thd, reattach_native_trx, MYSQL_STORAGE_ENGINE_PLUGIN,
                 nullptr);
}

bool Relay_log_info::commit_positions() {
  int error = 0;
  char saved_group_master_log_name[FN_REFLEN];
  my_off_t saved_group_master_log_pos;
  char saved_group_relay_log_name[FN_REFLEN];
  my_off_t saved_group_relay_log_pos;

  /*
    In FILE case Query_log_event::update_pos(), called after commit,
    updates the info object.
  */
  if (!is_transactional()) return false;

  mysql_mutex_lock(&data_lock);

  /* save the rli positions to restore after flush_info() */
  strmake(saved_group_master_log_name, get_group_master_log_name(),
          FN_REFLEN - 1);
  saved_group_master_log_pos = get_group_master_log_pos();
  strmake(saved_group_relay_log_name, get_group_relay_log_name(),
          FN_REFLEN - 1);
  saved_group_relay_log_pos = get_group_relay_log_pos();

  /* Update to new values just for the sake of flush_info */
  inc_event_relay_log_pos();
  set_group_relay_log_pos(get_event_relay_log_pos());
  set_group_relay_log_name(get_event_relay_log_name());
  set_group_master_log_pos(current_event->common_header->log_pos);
  /* save them too, but now for post_commit() time */
  strmake(new_group_master_log_name, get_group_master_log_name(),
          FN_REFLEN - 1);
  new_group_master_log_pos = get_group_master_log_pos();
  strmake(new_group_relay_log_name, get_group_relay_log_name(), FN_REFLEN - 1);
  new_group_relay_log_pos = get_group_relay_log_pos();

  error = flush_info(true);

  /*
    Restore the saved ones so they remain actual until the replicated
    statement commits.
  */
  set_group_master_log_name(saved_group_master_log_name);
  set_group_master_log_pos(saved_group_master_log_pos);
  set_group_relay_log_name(saved_group_relay_log_name);
  set_group_relay_log_pos(saved_group_relay_log_pos);

  mysql_mutex_unlock(&data_lock);

  return error != 0;
}

void Relay_log_info::post_commit(bool on_rollback) {
  THD *thd = info_thd;

  if (on_rollback) {
    if (thd->owned_gtid_is_empty()) gtid_state->update_on_rollback(thd);
  } else {
    /*
      New executed coordinates prepared in pre_commit() are
      finally installed.
    */
    mysql_mutex_lock(&data_lock);
    set_group_master_log_name(new_group_master_log_name);
    set_group_master_log_pos(new_group_master_log_pos);
    set_group_relay_log_name(new_group_relay_log_name);
    set_group_relay_log_pos(new_group_relay_log_pos);
    mysql_mutex_unlock(&data_lock);

    if (is_transactional()) {
      /* DDL's commit has been completed */
      DBUG_ASSERT(
          !current_event || !is_atomic_ddl_event(current_event) ||
          static_cast<Query_log_event *>(current_event)->has_ddl_committed);
      /*
        Marked as already-committed DDL may have not updated the GTID
        executed state, which is the case when slave filters it out.
        on slave side with binlog filtering out.
        When this is detected now the gtid state will be sorted later,
        and the gtid-executed based signaling will be done in the
        "last-chance-to-commit" branch of Log_event::do_update_pos().
        However in order to enter the branch has_ddl_committed needs false.
      */
      if (!thd->owned_gtid_is_empty())
        static_cast<Query_log_event *>(current_event)->has_ddl_committed =
            false;

      mysql_mutex_lock(&data_lock);
      /* Post-commit cleanup for Relay_log_info::wait_for_pos() */
      if (is_group_master_log_pos_invalid)
        is_group_master_log_pos_invalid = false;

      notify_group_master_log_name_update();
      mysql_cond_broadcast(&data_cond);
      mysql_mutex_unlock(&data_lock);
    } else {
      /*
        In the non-transactional slave info repository case should the
        current event be DDL it would still have to finalize commit.
      */
      DBUG_ASSERT(
          !current_event || !is_atomic_ddl_event(current_event) ||
          !static_cast<Query_log_event *>(current_event)->has_ddl_committed);
    }
  }
}

void Relay_log_info::notify_relay_log_truncated() {
  mysql_mutex_lock(&data_lock);
  m_relay_log_truncated = true;
  mysql_mutex_unlock(&data_lock);
}

void Relay_log_info::clear_relay_log_truncated() {
  mysql_mutex_assert_owner(&data_lock);
  m_relay_log_truncated = false;
}

bool Relay_log_info::is_time_for_mts_checkpoint() {
  bool period_check = opt_mts_checkpoint_period != 0 &&
                      !curr_group_seen_begin && !curr_group_seen_gtid;
  if (is_parallel_exec() && period_check) {
    struct timespec curr_clock;
    set_timespec_nsec(&curr_clock, 0);
    return diff_timespec(&curr_clock, &last_clock) >=
           opt_mts_checkpoint_period * 1000000ULL;
  }
  return false;
}

bool operator!(Relay_log_info::enum_priv_checks_status status) {
  return status == Relay_log_info::enum_priv_checks_status::SUCCESS;
}

bool operator!(Relay_log_info::enum_require_row_status status) {
  return status == Relay_log_info::enum_require_row_status::SUCCESS;
}

std::string Relay_log_info::get_privilege_checks_username() const {
  return this->m_privilege_checks_username;
}

std::string Relay_log_info::get_privilege_checks_hostname() const {
  return this->m_privilege_checks_hostname;
}

bool Relay_log_info::is_privilege_checks_user_null() const {
  DBUG_ASSERT(this->m_privilege_checks_username.length() != 0 ||
              (this->m_privilege_checks_username.length() == 0 &&
               this->m_privilege_checks_hostname.length() == 0));
  return this->m_privilege_checks_username.length() == 0;
}

bool Relay_log_info::is_privilege_checks_user_corrupted() const {
  return this->m_privilege_checks_user_corrupted;
}

void Relay_log_info::clear_privilege_checks_user() {
  DBUG_TRACE;
  this->m_privilege_checks_username.clear();
  this->m_privilege_checks_hostname.clear();
  this->m_privilege_checks_user_corrupted = false;
}

void Relay_log_info::set_privilege_checks_user_corrupted(bool is_corrupted) {
  DBUG_TRACE;
  this->m_privilege_checks_user_corrupted = is_corrupted;
}

Relay_log_info::enum_priv_checks_status
Relay_log_info::set_privilege_checks_user(
    char const *param_privilege_checks_username,
    char const *param_privilege_checks_hostname) {
  DBUG_TRACE;

  enum_priv_checks_status error = this->check_privilege_checks_user(
      param_privilege_checks_username, param_privilege_checks_hostname);
  if (!!error) return error;

  if (param_privilege_checks_username == nullptr) {
    this->clear_privilege_checks_user();
    return enum_priv_checks_status::SUCCESS;
  }

  this->m_privilege_checks_user_corrupted = false;
  this->m_privilege_checks_username =
      static_cast<std::string>(param_privilege_checks_username);

  if (param_privilege_checks_hostname != nullptr) {
    this->m_privilege_checks_hostname =
        static_cast<std::string>(param_privilege_checks_hostname);
    std::transform(this->m_privilege_checks_hostname.begin(),
                   this->m_privilege_checks_hostname.end(),
                   this->m_privilege_checks_hostname.begin(), ::tolower);
  }

  return enum_priv_checks_status::SUCCESS;
}

Relay_log_info::enum_priv_checks_status
Relay_log_info::check_privilege_checks_user() {
  DBUG_TRACE;
  DBUG_ASSERT(this->m_privilege_checks_username.length() != 0 ||
              (this->m_privilege_checks_username.length() == 0 &&
               this->m_privilege_checks_hostname.length() == 0));

  return this->check_privilege_checks_user(
      this->m_privilege_checks_username.length() != 0
          ? this->m_privilege_checks_username.data()
          : nullptr,
      this->m_privilege_checks_hostname.length() != 0
          ? this->m_privilege_checks_hostname.data()
          : nullptr);
}

Relay_log_info::enum_priv_checks_status
Relay_log_info::check_privilege_checks_user(
    char const *param_privilege_checks_username,
    char const *param_privilege_checks_hostname) {
  DBUG_TRACE;

  if (param_privilege_checks_username == nullptr) {
    if (param_privilege_checks_hostname != nullptr)
      return enum_priv_checks_status::USERNAME_NULL_HOSTNAME_NOT_NULL;
    return enum_priv_checks_status::SUCCESS;
  }

  if (strlen(param_privilege_checks_username) == 0)
    return enum_priv_checks_status::USER_ANONYMOUS;

  if (strlen(param_privilege_checks_username) > 32)
    return enum_priv_checks_status::USERNAME_TOO_LONG;

  if (param_privilege_checks_hostname != nullptr &&
      strlen(param_privilege_checks_hostname) > 255)
    return enum_priv_checks_status::HOSTNAME_TOO_LONG;

  if (param_privilege_checks_hostname != nullptr) {
    std::regex static const hostname_regex{"^((?![@])[\\x20-\\x7e])+$",
                                           std::regex_constants::ECMAScript};
    if (!std::regex_match(param_privilege_checks_hostname, hostname_regex))
      return enum_priv_checks_status::HOSTNAME_SYNTAX_ERROR;
  }

  enum_priv_checks_status error = this->check_applier_acl_user(
      param_privilege_checks_username, param_privilege_checks_hostname);
  if (!!error) return error;

  return enum_priv_checks_status::SUCCESS;
}

Relay_log_info::enum_priv_checks_status Relay_log_info::check_applier_acl_user(
    char const *param_privilege_checks_username,
    char const *param_privilege_checks_hostname) {
  DBUG_TRACE;
  DBUG_ASSERT(param_privilege_checks_username != nullptr &&
              strlen(param_privilege_checks_username) != 0);

  THD_instance_guard thd{current_thd != nullptr ? current_thd : this->info_thd};
  Acl_cache_lock_guard acl_cache_lock{thd, Acl_cache_lock_mode::READ_MODE};
  if (!acl_cache_lock.lock()) {  // If we're unable to acquire the lock we're
                                 // unable to check if the user exists
    return enum_priv_checks_status::USER_DOES_NOT_EXIST;  // so, return
                                                          // accordingly.
  }

  ACL_USER *applier_acl_user = find_acl_user(
      param_privilege_checks_hostname, param_privilege_checks_username, false);

  if (applier_acl_user == nullptr) {
    return enum_priv_checks_status::USER_DOES_NOT_EXIST;
  }

  return enum_priv_checks_status::SUCCESS;
}

std::pair<const char *, const char *>
Relay_log_info::print_applier_security_context_user_host() const {
  if (this->m_privilege_checks_username.length() != 0) {
    return {this->m_privilege_checks_username.data(),
            this->m_privilege_checks_hostname.length() == 0
                ? "%"
                : this->m_privilege_checks_hostname.data()};
  } else if (this->info_thd != nullptr &&
             this->info_thd->security_context() != nullptr) {
    return {this->info_thd->security_context()->user().str != nullptr
                ? this->info_thd->security_context()->user().str
                : "",
            this->info_thd->security_context()->host().str != nullptr
                ? this->info_thd->security_context()->host().str
                : ""};
  }
  return {"", ""};
}

void Relay_log_info::report_privilege_check_error(
    enum loglevel level, enum_priv_checks_status status_code, bool to_client,
    char const *channel_name_arg, char const *user_name_arg,
    char const *host_name_arg) const {
  DBUG_TRACE;

  char const *channel_name{channel_name_arg != nullptr ? channel_name_arg
                                                       : get_channel()};
  char const *user_name{user_name_arg};
  char const *host_name{host_name_arg};
  if (user_name == nullptr) {
    std::tie(user_name, host_name) =
        this->print_applier_security_context_user_host();
  }

  switch (status_code) {
    case enum_priv_checks_status::SUCCESS: {
      DBUG_ASSERT(false);
      break;
    }
    case enum_priv_checks_status::USER_ANONYMOUS: {
      if (to_client)
        my_error(ER_CLIENT_PRIVILEGE_CHECKS_USER_CANNOT_BE_ANONYMOUS, MYF(0),
                 channel_name, host_name);
      else {
        THD_instance_guard thd{current_thd != nullptr ? current_thd
                                                      : this->info_thd};
        this->report(level, ER_WARN_LOG_PRIVILEGE_CHECKS_USER_CORRUPT,
                     ER_THD(thd, ER_WARN_LOG_PRIVILEGE_CHECKS_USER_CORRUPT),
                     channel_name);
      }
      break;
    }
    case enum_priv_checks_status::USERNAME_TOO_LONG: {
      if (to_client)
        my_error(ER_WRONG_STRING_LENGTH, MYF(0), user_name, "user name", 32);
      else {
        THD_instance_guard thd{current_thd != nullptr ? current_thd
                                                      : this->info_thd};
        this->report(level, ER_WARN_LOG_PRIVILEGE_CHECKS_USER_CORRUPT,
                     ER_THD(thd, ER_WARN_LOG_PRIVILEGE_CHECKS_USER_CORRUPT),
                     channel_name);
      }
      break;
    }
    case enum_priv_checks_status::HOSTNAME_TOO_LONG: {
      if (to_client)
        my_error(ER_WRONG_STRING_LENGTH, MYF(0), user_name, "host name", 255);
      else {
        THD_instance_guard thd{current_thd != nullptr ? current_thd
                                                      : this->info_thd};
        this->report(level, ER_WARN_LOG_PRIVILEGE_CHECKS_USER_CORRUPT,
                     ER_THD(thd, ER_WARN_LOG_PRIVILEGE_CHECKS_USER_CORRUPT),
                     channel_name);
      }
      break;
    }
    case enum_priv_checks_status::HOSTNAME_SYNTAX_ERROR: {
      if (to_client)
        my_printf_error(ER_UNKNOWN_ERROR, "Malformed hostname (illegal symbol)",
                        MYF(0));
      else {
        THD_instance_guard thd{current_thd != nullptr ? current_thd
                                                      : this->info_thd};
        this->report(level, ER_WARN_LOG_PRIVILEGE_CHECKS_USER_CORRUPT,
                     ER_THD(thd, ER_WARN_LOG_PRIVILEGE_CHECKS_USER_CORRUPT),
                     channel_name);
      }
      break;
    }
    case enum_priv_checks_status::USER_DATA_CORRUPTED:
    case enum_priv_checks_status::USERNAME_NULL_HOSTNAME_NOT_NULL: {
      if (to_client)
        my_error(ER_CLIENT_PRIVILEGE_CHECKS_USER_CORRUPT, MYF(0), channel_name);
      else {
        THD_instance_guard thd{current_thd != nullptr ? current_thd
                                                      : this->info_thd};
        this->report(level, ER_WARN_LOG_PRIVILEGE_CHECKS_USER_CORRUPT,
                     ER_THD(thd, ER_WARN_LOG_PRIVILEGE_CHECKS_USER_CORRUPT),
                     channel_name);
      }
      break;
    }
    case enum_priv_checks_status::USER_DOES_NOT_EXIST: {
      if (to_client)
        my_error(ER_CLIENT_PRIVILEGE_CHECKS_USER_DOES_NOT_EXIST, MYF(0),
                 channel_name, user_name, host_name);
      else {
        THD_instance_guard thd{current_thd != nullptr ? current_thd
                                                      : this->info_thd};
        this->report(
            level, ER_WARN_LOG_PRIVILEGE_CHECKS_USER_DOES_NOT_EXIST,
            ER_THD(thd, ER_WARN_LOG_PRIVILEGE_CHECKS_USER_DOES_NOT_EXIST),
            channel_name, user_name, host_name);
      }
      break;
    }
    case enum_priv_checks_status::USER_DOES_NOT_HAVE_PRIVILEGES: {
      if (!to_client) {
        THD_instance_guard thd{current_thd != nullptr ? current_thd
                                                      : this->info_thd};
        this->report(
            level, ER_WARN_LOG_PRIVILEGE_CHECKS_USER_NEEDS_RPL_APPLIER_PRIV,
            ER_THD(thd,
                   ER_WARN_LOG_PRIVILEGE_CHECKS_USER_NEEDS_RPL_APPLIER_PRIV),
            channel_name, user_name, host_name);
      }
      break;
    }
    case enum_priv_checks_status::LOAD_DATA_EVENT_NOT_ALLOWED: {
      if (!to_client) {
        THD_instance_guard thd{current_thd != nullptr ? current_thd
                                                      : this->info_thd};
        this->report(level, ER_FILE_PRIVILEGE_FOR_REPLICATION_CHECKS,
                     ER_THD(thd, ER_FILE_PRIVILEGE_FOR_REPLICATION_CHECKS),
                     channel_name);
      }
      break;
    }
  }
}

Relay_log_info::enum_priv_checks_status
Relay_log_info::initialize_security_context(THD *thd) {
  DBUG_TRACE;

  if (this->m_privilege_checks_user_corrupted)
    return enum_priv_checks_status::USER_DATA_CORRUPTED;

  if (this->m_privilege_checks_username.length() != 0) {
    if (acl_getroot(thd, &thd->m_main_security_ctx,
                    this->m_privilege_checks_username.data(),
                    this->m_privilege_checks_hostname.data(), nullptr,
                    nullptr)) {
      return enum_priv_checks_status::USER_DOES_NOT_EXIST;
    }

    Roles::Role_activation role_activation{
        thd, thd->security_context(),
        opt_always_activate_granted_roles == 0 ? role_enum::ROLE_DEFAULT
                                               : role_enum::ROLE_ALL,
        nullptr, true};
    if (role_activation.activate())
      return enum_priv_checks_status::USER_DOES_NOT_HAVE_PRIVILEGES;

    bool has_grant{false};
    std::tie(has_grant, std::ignore) =
        thd->m_main_security_ctx.has_global_grant(
            STRING_WITH_LEN("REPLICATION_APPLIER"));
    if (!has_grant) {
      return enum_priv_checks_status::USER_DOES_NOT_HAVE_PRIVILEGES;
    }
  } else
    thd->security_context()->skip_grants();

  return enum_priv_checks_status::SUCCESS;
}

Relay_log_info::enum_priv_checks_status
Relay_log_info::initialize_applier_security_context() {
  DBUG_TRACE;
  return this->initialize_security_context(this->info_thd);
}

bool Relay_log_info::is_row_format_required() const {
  return this->m_require_row_format;
}

void Relay_log_info::set_require_row_format(bool require_row) {
  DBUG_TRACE;
  this->m_require_row_format = require_row;
}

Relay_log_info::enum_require_table_primary_key
Relay_log_info::get_require_table_primary_key_check() const {
  return this->m_require_table_primary_key_check;
}

void Relay_log_info::set_require_table_primary_key_check(
    Relay_log_info::enum_require_table_primary_key require_pk) {
  DBUG_TRACE;
  this->m_require_table_primary_key_check = require_pk;
}

void Relay_log_info::populate_recovery_binlog_max_gtid() {
  const std::string &max_binlog_gtid =
      mysql_bin_log.get_recovery_binlog_max_gtid();
  if (!max_binlog_gtid.empty()) {
    recovery_sid_lock.rdlock();
    recovery_max_engine_gtid.parse(&recovery_sid_map, max_binlog_gtid.c_str());
    recovery_sid_lock.unlock();
  }
}

MDL_lock_guard::MDL_lock_guard(THD *target) : m_target{target} { DBUG_TRACE; }

MDL_lock_guard::MDL_lock_guard(THD *target,
                               MDL_key::enum_mdl_namespace namespace_arg,
                               enum_mdl_type mdl_type_arg, bool blocking)
    : m_target{target} {
  DBUG_TRACE;
  this->lock(namespace_arg, mdl_type_arg, blocking);
}

bool MDL_lock_guard::lock(MDL_key::enum_mdl_namespace namespace_arg,
                          enum_mdl_type mdl_type_arg, bool blocking) {
  DBUG_TRACE;
  if (this->m_target != nullptr &&
      !this->m_target->mdl_context.has_locks(namespace_arg)) {
    MDL_REQUEST_INIT(&this->m_request, namespace_arg, "", "", mdl_type_arg,
                     MDL_EXPLICIT);

    if (blocking)
      this->m_target->mdl_context.acquire_lock_nsec(
          &this->m_request, this->m_target->variables.lock_wait_timeout_nsec);
    else
      this->m_target->mdl_context.try_acquire_lock(&this->m_request);

    return !this->is_locked();
  }
  return true;
}

MDL_lock_guard::~MDL_lock_guard() {
  DBUG_TRACE;
  if (this->m_request.ticket != nullptr) {
    this->m_target->mdl_context.release_lock(this->m_request.ticket);
  }
}

bool MDL_lock_guard::is_locked() { return this->m_request.ticket != nullptr; }

Applier_security_context_guard::Applier_security_context_guard(
    Relay_log_info const *rli, THD const *thd)
    : m_target{rli},
      m_thd{thd},
      m_current{nullptr},
      m_previous{nullptr},
      m_privilege_checks_none{
          this->m_target->get_privilege_checks_username().length() == 0 &&
          (this->m_thd->lex == nullptr ||
           this->m_thd->lex->binlog_stmt_arg.length == 0 ||
           this->m_thd->lex->binlog_stmt_arg.length >= 2048 ||
           this->m_thd->lex->binlog_stmt_arg.str == nullptr)} {
  DBUG_TRACE;

  if (this->m_thd->lex != nullptr &&
      this->m_thd->lex->binlog_stmt_arg.length != 0 &&
      this->m_thd->lex->binlog_stmt_arg.length < 2048 &&
      this->m_thd->lex->binlog_stmt_arg.str != nullptr) {
    Security_context *standing_ctx = this->m_thd->security_context();
    if (standing_ctx != nullptr &&
        (standing_ctx->check_access(SUPER_ACL) ||
         standing_ctx->has_global_grant(STRING_WITH_LEN("BINLOG_ADMIN")).first))
      this->m_privilege_checks_none = true;
  }
  if (this->m_privilege_checks_none) return;

  if (this->m_target->get_privilege_checks_username().length() != 0) {
    std::string user = this->m_target->get_privilege_checks_username();
    std::string host = this->m_target->get_privilege_checks_hostname();
    LEX_CSTRING username{user.c_str(), user.length()};
    LEX_CSTRING hostname{host.c_str(), host.length()};

    if (this->m_applier_security_ctx.change_security_context(
            const_cast<THD *>(this->m_thd), username, hostname, nullptr,
            &this->m_previous)) {
      return;
    }
  }

  if (this->m_previous != nullptr) {
    Roles::Role_activation role_activation{
        const_cast<THD *>(this->m_thd), this->m_thd->security_context(),
        opt_always_activate_granted_roles == 0 ? role_enum::ROLE_DEFAULT
                                               : role_enum::ROLE_ALL,
        nullptr, true};
    if (role_activation.activate()) return;
  }

  this->m_current = this->m_thd->security_context();
}

Applier_security_context_guard::~Applier_security_context_guard() {
  if (this->m_privilege_checks_none) return;

  if (this->m_previous != nullptr && this->m_previous != this->m_current)
    this->m_applier_security_ctx.restore_security_context(
        const_cast<THD *>(this->m_thd), this->m_previous);
}

bool Applier_security_context_guard::skip_priv_checks() const {
  return this->m_privilege_checks_none;
}

bool Applier_security_context_guard::has_access(
    std::initializer_list<ulong> extra_privileges) const {
  if (this->m_privilege_checks_none) return true;
  if (this->m_current == nullptr) return false;

  for (auto privilege : extra_privileges)
    if (!this->m_current->check_access(privilege, "", true)) return false;

  return true;
}

bool Applier_security_context_guard::has_access(
    std::initializer_list<std::string> extra_privileges) const {
  if (this->m_privilege_checks_none) return true;
  if (this->m_current == nullptr) return false;

  for (auto privilege : extra_privileges) {
    if (!this->m_current->has_global_grant(privilege.data(), privilege.length())
             .first)
      return false;
  }
  return true;
}

bool Applier_security_context_guard::has_access(
    std::vector<std::tuple<ulong, TABLE const *, Rows_log_event *>>
        &extra_privileges) const {
  if (this->m_privilege_checks_none) return true;
  if (this->m_current == nullptr) return false;

  ulong priv{0};
  TABLE const *table{nullptr};

  if (this->m_thd->variables.binlog_row_image == BINLOG_ROW_IMAGE_FULL) {
    for (auto tpl : extra_privileges) {
      std::tie(priv, table, std::ignore) = tpl;
      if (this->m_current->is_table_blocked(priv, table)) return false;
    }
  } else {
    Rows_log_event *event{nullptr};

    for (auto tpl : extra_privileges) {
      std::tie(priv, table, event) = tpl;

      if (event->get_general_type_code() == binary_log::DELETE_ROWS_EVENT) {
        if (this->m_current->is_table_blocked(priv, table)) return false;
      } else {
        std::vector<std::string> columns;
        this->extract_columns_to_check(table, event, columns);
        if (!this->m_current->has_column_access(priv, table, columns))
          return false;
      }
    }
  }

  return true;
}

std::string Applier_security_context_guard::get_username() const {
  if (this->m_privilege_checks_none || this->m_current == nullptr) return "";
  return std::string(this->m_current->user().str,
                     this->m_current->user().length);
}

std::string Applier_security_context_guard::get_hostname() const {
  if (this->m_privilege_checks_none || this->m_current == nullptr) return "";
  return std::string(this->m_current->host().str,
                     this->m_current->host().length);
}

void Applier_security_context_guard::extract_columns_to_check(
    TABLE const *table, Rows_log_event *event,
    std::vector<std::string> &columns) const {
  MY_BITMAP const *bitmap{nullptr};

  if (event->get_general_type_code() == binary_log::WRITE_ROWS_EVENT)
    bitmap = event->get_cols();
  else if (event->get_general_type_code() == binary_log::UPDATE_ROWS_EVENT)
    bitmap = event->get_cols_ai();
  else
    return;

  size_t max = std::min(bitmap->n_bits, table->s->fields);
  for (size_t idx = 0; idx != max; ++idx) {
    if (bitmap_is_set(bitmap, idx)) {
      columns.push_back(table->field[idx]->field_name);
    }
  }
}
