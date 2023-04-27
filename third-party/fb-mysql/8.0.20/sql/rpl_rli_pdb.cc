/* Copyright (c) 2011, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/rpl_rli_pdb.h"

#include "my_config.h"

#include <string.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <stdio.h>
#include <algorithm>
#include <atomic>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include <../include/mysql/service_thd_engine_lock.h>
#include "lex_string.h"
#include "m_string.h"
#include "map_helpers.h"
#include "my_bitmap.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_sys.h"
#include "my_systime.h"
#include "my_thread.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/psi_stage_bits.h"
#include "mysql/plugin.h"
#include "mysql/psi/mysql_cond.h"
#include "mysql/psi/mysql_file.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/thread_type.h"
#include "mysqld_error.h"
#include "sql/binlog.h"
#include "sql/binlog_reader.h"
#include "sql/current_thd.h"
#include "sql/debug_sync.h"
#include "sql/log.h"
#include "sql/mdl.h"
#include "sql/mysqld.h"  // key_mutex_slave_parallel_worker
#include "sql/psi_memory_key.h"
#include "sql/rpl_info_handler.h"
#include "sql/rpl_msr.h"  // For channel_map
#include "sql/rpl_reporting.h"
#include "sql/rpl_slave_commit_order_manager.h"  // Commit_order_manager
#include "sql/sql_error.h"
#include "sql/sql_lex.h"
#include "sql/table.h"
#include "sql/transaction_info.h"
#include "thr_mutex.h"

#ifndef DBUG_OFF
ulong w_rr = 0;
uint mts_debug_concurrent_access = 0;
#endif

#define HASH_DYNAMIC_INIT 4
static const int LINES_IN_WORKER_INFO = 12;

using std::max;
using std::min;

/**
   This function is called by both coordinator and workers.

   Upon receiving the STOP command, the workers will identify a
   maximum group index already executed (or under execution).

   All groups whose index are below or equal to the maximum
   group index will be applied by the workers before stopping.

   The workers with groups above the maximum group index will
   exit without applying these groups by setting their running
   status to "STOP_ACCEPTED".

   @param worker    a pointer to the waiting Worker struct
   @param job_item  a pointer to struct carrying a reference to an event

   @return true if STOP command gets accepted otherwise false is returned.
*/
bool handle_slave_worker_stop(Slave_worker *worker, Slave_job_item *job_item) {
  ulonglong group_index = 0;
  Relay_log_info *rli = worker->c_rli;
  mysql_mutex_lock(&rli->exit_count_lock);
  /*
    First, W calculates a group-"at-hands" index which is
    either the currently read ev group index, or the last executed
    group's one when the  queue is empty.
  */
  group_index =
      (job_item->data)
          ? rli->gaq->get_job_group(job_item->data->mts_group_idx)->total_seqno
          : worker->last_groups_assigned_index;

  /*
    The max updated index is being updated as long as
    exit_counter permits. That's stopped with the final W's
    increment of it.
  */
  if (!worker->exit_incremented) {
    if (rli->exit_counter < rli->slave_parallel_workers)
      rli->max_updated_index = max(rli->max_updated_index, group_index);

    ++rli->exit_counter;
    worker->exit_incremented = true;
    DBUG_ASSERT(!is_mts_worker(current_thd));
  }
#ifndef DBUG_OFF
  else
    DBUG_ASSERT(is_mts_worker(current_thd));
#endif

  /*
    Now let's decide about the deferred exit to consider
    the empty queue and the counter value reached
    slave_parallel_workers.
  */
  if (!job_item->data) {
    worker->running_status = Slave_worker::STOP_ACCEPTED;
    mysql_cond_signal(&worker->jobs_cond);
    mysql_mutex_unlock(&rli->exit_count_lock);
    return (true);
  } else if (rli->exit_counter == rli->slave_parallel_workers) {
    // over steppers should exit with accepting STOP
    if (group_index > rli->max_updated_index) {
      worker->running_status = Slave_worker::STOP_ACCEPTED;
      mysql_cond_signal(&worker->jobs_cond);
      mysql_mutex_unlock(&rli->exit_count_lock);
      return (true);
    }
  }
  mysql_mutex_unlock(&rli->exit_count_lock);
  return (false);
}

/**
   This function is called by both coordinator and workers.
   Both coordinator and workers contribute to max_updated_index.

   @param worker    a pointer to the waiting Worker struct
   @param job_item  a pointer to struct carrying a reference to an event

   @return true if STOP command gets accepted otherwise false is returned.
*/
bool set_max_updated_index_on_stop(Slave_worker *worker,
                                   Slave_job_item *job_item) {
  head_queue(&worker->jobs, job_item);
  if (worker->running_status == Slave_worker::STOP) {
    if (handle_slave_worker_stop(worker, job_item)) return true;
  }
  return false;
}

/*
  Please every time you add a new field to the worker slave info, update
  what follows. For now, this is just used to get the number of fields.
*/
const char *info_slave_worker_fields[] = {
    "id",
    /*
      These positions identify what has been executed. Notice that they are
      redudant and only the group_master_log_name and group_master_log_pos
      are really necessary. However, the additional information is kept to
      ease debugging.
    */
    "group_relay_log_name", "group_relay_log_pos", "group_master_log_name",
    "group_master_log_pos",

    /*
      These positions identify what a worker knew about the coordinator at
      the time a job was assigned. Notice that they are redudant and are
      kept to ease debugging.
    */
    "checkpoint_relay_log_name", "checkpoint_relay_log_pos",
    "checkpoint_master_log_name", "checkpoint_master_log_pos",

    /*
      Identify the greatest job, i.e. group, processed by a worker.
    */
    "checkpoint_seqno",
    /*
      Maximum number of jobs that can be assigned to a worker. This
      information is necessary to read the next entry.
    */
    "checkpoint_group_size",
    /*
      Bitmap used to identify what jobs were processed by a worker.
    */
    "checkpoint_group_bitmap",
    /*
      Channel on which this workers are acting
    */
    "channel_name"};

/*
  Number of records in the mts partition hash below which
  entries with zero usage are tolerated so could be quickly
  recycled.
*/
const ulong mts_partition_hash_soft_max = 16;

/*
  index value of some outstanding slots of info_slave_worker_fields
*/
enum {
  LINE_FOR_CHANNEL = 12,
};

const uint info_slave_worker_table_pk_field_indexes[] = {
    LINE_FOR_CHANNEL,
    0,
};

Slave_worker::Slave_worker(Relay_log_info *rli,
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
                           uint param_id, const char *param_channel)
    : Relay_log_info(false,
#ifdef HAVE_PSI_INTERFACE
                     param_key_info_run_lock, param_key_info_data_lock,
                     param_key_info_sleep_lock, param_key_info_thd_lock,
                     param_key_info_data_cond, param_key_info_start_cond,
                     param_key_info_stop_cond, param_key_info_sleep_cond,
#endif
                     param_id + 1, param_channel, true),
      c_rli(rli),
      curr_group_exec_parts(key_memory_db_worker_hash_entry),
      id(param_id),
      checkpoint_relay_log_pos(0),
      checkpoint_master_log_pos(0),
      worker_checkpoint_seqno(0),
      running_status(NOT_RUNNING),
      exit_incremented(false) {
  /*
    In the future, it would be great if we use only one identifier.
    So when factoring out this code, please, consider this.
  */
  DBUG_ASSERT(internal_id == id + 1);
  worker_last_gtid[0] = 0;
  checkpoint_relay_log_name[0] = 0;
  checkpoint_master_log_name[0] = 0;

  mysql_mutex_init(key_mutex_slave_parallel_worker, &jobs_lock,
                   MY_MUTEX_INIT_FAST);
  mysql_cond_init(key_cond_slave_parallel_worker, &jobs_cond);
  mysql_cond_init(key_cond_mts_gaq, &logical_clock_cond);
}

Slave_worker::~Slave_worker() {
  end_info();
  if (jobs.inited_queue) {
    DBUG_ASSERT(jobs.m_Q.size() == jobs.size);
    jobs.m_Q.clear();
  }
  mysql_mutex_destroy(&jobs_lock);
  mysql_cond_destroy(&jobs_cond);
  mysql_cond_destroy(&logical_clock_cond);
  mysql_mutex_lock(&info_thd_lock);
  info_thd = nullptr;
  mysql_mutex_unlock(&info_thd_lock);
  if (set_rli_description_event(nullptr)) {
#ifndef DBUG_OFF
    bool set_rli_description_event_failed = false;
#endif
    DBUG_ASSERT(set_rli_description_event_failed);
  }
}

/**
   Method is executed by Coordinator at Worker startup time to initialize
   members parly with values supplied by Coordinator through rli.

   @param  rli  Coordinator's Relay_log_info pointer
   @param  i    identifier of the Worker

   @return 0          success
           non-zero   failure
*/
int Slave_worker::init_worker(Relay_log_info *rli, ulong i) {
  DBUG_TRACE;
  DBUG_ASSERT(!rli->info_thd->is_error());

  Slave_job_item empty = Slave_job_item();

  c_rli = rli;
  this->set_require_row_format(rli->is_row_format_required());

  set_commit_order_manager(c_rli->get_commit_order_manager());

  if (rli_init_info(false) ||
      DBUG_EVALUATE_IF("inject_init_worker_init_info_fault", true, false))
    return 1;

  if (!rli->is_privilege_checks_user_null()) {
    this->set_privilege_checks_user(
        rli->get_privilege_checks_username().c_str(),
        rli->get_privilege_checks_hostname().c_str());
  }

  id = i;
  curr_group_exec_parts.clear();
  relay_log_change_notified = false;  // the 1st group to contain relaylog name
  checkpoint_notified = false;        // the same as above
  master_log_change_notified =
      false;                   // W learns master log during 1st group exec
  fd_change_notified = false;  // W is to learn master FD version same as above
  server_version = version_product(rli->slave_version_split);
  bitmap_shifted = 0;
  workers = c_rli->workers;  // shallow copying is sufficient
  wq_empty_waits = wq_size_waits_cnt = groups_done = events_done = curr_jobs =
      0;
  usage_partition = 0;
  end_group_sets_max_dbs = false;
  gaq_index = last_group_done_index = c_rli->gaq->size;  // out of range
  last_groups_assigned_index = 0;
  DBUG_ASSERT(!jobs.inited_queue);
  jobs.avail = 0;
  jobs.len = 0;
  jobs.overfill = false;  //  todo: move into Slave_jobs_queue constructor
  jobs.waited_overfill = 0;
  jobs.entry = jobs.size = c_rli->mts_slave_worker_queue_len_max;
  jobs.inited_queue = true;
  curr_group_seen_gtid = false;
  curr_group_seen_metadata = false;
  DBUG_EXECUTE_IF("slave_worker_queue_size", { jobs.entry = jobs.size = 5; });
#ifndef DBUG_OFF
  curr_group_seen_sequence_number = false;
#endif
  jobs.m_Q.resize(jobs.size, empty);
  DBUG_ASSERT(jobs.m_Q.size() == jobs.size);

  wq_overrun_cnt = excess_cnt = 0;
  underrun_level =
      (ulong)((rli->mts_worker_underrun_level * jobs.size) / 100.0);
  // overrun level is symmetric to underrun (as underrun to the full queue)
  overrun_level = jobs.size - underrun_level;

  /* create mts submode for each of the the workers. */
  switch (rli->channel_mts_submode) {
    case MTS_PARALLEL_TYPE_DB_NAME:
      current_mts_submode = (Mts_submode *)new Mts_submode_database();
      break;
    case MTS_PARALLEL_TYPE_LOGICAL_CLOCK:
      current_mts_submode = (Mts_submode *)new Mts_submode_logical_clock();
      break;
    case MTS_PARALLEL_TYPE_DEPENDENCY:
      current_mts_submode = (Mts_submode *)new Mts_submode_dependency();
      break;
    default:
      return 1;
  }

  // workers and coordinator must be of the same type
  DBUG_ASSERT(rli->current_mts_submode->get_type() ==
              current_mts_submode->get_type());
  // copy these over for easy access
  mts_dependency_replication = c_rli->mts_dependency_replication;
  mts_dependency_size = c_rli->mts_dependency_size;
  mts_dependency_refill_threshold = c_rli->mts_dependency_refill_threshold;
  mts_dependency_max_keys = c_rli->mts_dependency_max_keys;
  slave_preserve_commit_order = c_rli->slave_preserve_commit_order;

  reset_commit_order_deadlock();
  return 0;
}

/**
   A part of Slave worker iitializer that provides a
   minimum context for MTS recovery.

   @param is_gaps_collecting_phase

          clarifies what state the caller
          executes this method from. When it's @c true
          that is @c mts_recovery_groups() and Worker should
          restore the last session time info which is processed
          to collect gaps that is not executed transactions (groups).
          Such recovery Slave_worker intance is destroyed at the end of
          @c mts_recovery_groups().
          Whet it's @c false Slave_worker is initialized for the run time
          nad should not read the last session time stale info.
          Its info will be ultimately reset once all gaps are executed
          to finish off recovery.

   @return 0 on success, non-zero for a failure
*/
int Slave_worker::rli_init_info(bool is_gaps_collecting_phase) {
  enum_return_check return_check = ERROR_CHECKING_REPOSITORY;

  DBUG_TRACE;

  if (inited) return 0;

  /*
    Worker bitmap size depends on recovery mode.
    If it is gaps collecting the bitmaps must be capable to accept
    up to MTS_MAX_BITS_IN_GROUP of bits.
  */
  size_t num_bits = is_gaps_collecting_phase ? MTS_MAX_BITS_IN_GROUP
                                             : c_rli->checkpoint_group;
  /*
    This checks if the repository was created before and thus there
    will be values to be read. Please, do not move this call after
    the handler->init_info().
  */
  return_check = check_info();
  if (return_check == ERROR_CHECKING_REPOSITORY ||
      (return_check == REPOSITORY_DOES_NOT_EXIST && is_gaps_collecting_phase))
    goto err;

  if (handler->init_info()) goto err;

  bitmap_init(&group_executed, nullptr, num_bits);
  bitmap_init(&group_shifted, nullptr, num_bits);

  if (is_gaps_collecting_phase &&
      (DBUG_EVALUATE_IF("mts_slave_worker_init_at_gaps_fails", true, false) ||
       read_info(handler))) {
    bitmap_free(&group_executed);
    bitmap_free(&group_shifted);
    goto err;
  }
  inited = true;

  return 0;

err:
  // todo: handler->end_info(uidx, nidx);
  inited = false;
  LogErr(ERROR_LEVEL, ER_RPL_ERROR_READING_SLAVE_WORKER_CONFIGURATION);
  return 1;
}

void Slave_worker::end_info() {
  DBUG_TRACE;

  if (!inited) return;

  if (handler) handler->end_info();

  if (inited) {
    bitmap_free(&group_executed);
    bitmap_free(&group_shifted);
  }
  inited = false;
}

int Slave_worker::flush_info(const bool force) {
  DBUG_TRACE;

  if (!inited) return 0;

  /*
    We update the sync_period at this point because only here we
    now that we are handling a Slave_worker. This needs to be
    update every time we call flush because the option may be
    dinamically set.
  */
  handler->set_sync_period(sync_relayloginfo_period);

  handler->inc_sync_counter();

  bool do_flush = sync_relayloginfo_period &&
                  handler->get_sync_counter() >= sync_relayloginfo_period;
  /*
    Check whether a write is actually necessary. If not checked,
    write_info() causes unnecessary code path which copies (sprintf),
    writes to file cache and flush_info() causes unnecessary flush of the
    file cache which are anyway completely useless in recovery since
    they are not transactional if we are using FILE based repository.
  */
  if (skip_flush_relay_worker_info && !(force || do_flush)) return 0;

  /*
    This only fails on out-of-memory errors, which are reported (using
    the MY_WME flag to my_malloc).
  */
  if (write_info(handler)) return 1;

  /*
    This fails on errors committing the info, or when
    slave_preserve_commit_order is enabled and a previous transaction
    has failed.  In both cases, the error is reported already.
  */
  if (handler->flush_info(force)) return 1;

  return 0;
}

bool Slave_worker::read_info(Rpl_info_handler *from) {
  DBUG_TRACE;

  ulong temp_group_relay_log_pos = 0;
  ulong temp_group_master_log_pos = 0;
  ulong temp_checkpoint_relay_log_pos = 0;
  ulong temp_checkpoint_master_log_pos = 0;
  ulong temp_checkpoint_seqno = 0;
  ulong nbytes = 0;
  uchar *buffer = (uchar *)group_executed.bitmap;
  int temp_internal_id = 0;

  if (from->prepare_info_for_read()) return true;

  if (!!from->get_info(&temp_internal_id, 0) ||
      !!from->get_info(group_relay_log_name, sizeof(group_relay_log_name),
                       "") ||
      !!from->get_info(&temp_group_relay_log_pos, 0UL) ||
      !!from->get_info(group_master_log_name, sizeof(group_master_log_name),
                       "") ||
      !!from->get_info(&temp_group_master_log_pos, 0UL) ||
      !!from->get_info(checkpoint_relay_log_name,
                       sizeof(checkpoint_relay_log_name), "") ||
      !!from->get_info(&temp_checkpoint_relay_log_pos, 0UL) ||
      !!from->get_info(checkpoint_master_log_name,
                       sizeof(checkpoint_master_log_name), "") ||
      !!from->get_info(&temp_checkpoint_master_log_pos, 0UL) ||
      !!from->get_info(&temp_checkpoint_seqno, 0UL) ||
      !!from->get_info(&nbytes, 0UL) ||
      !!from->get_info(buffer, (size_t)nbytes, (uchar *)nullptr) ||
      /* default is empty string */
      !!from->get_info(channel, sizeof(channel), ""))
    return true;

  DBUG_ASSERT(nbytes <= no_bytes_in_map(&group_executed));

  internal_id = (uint)temp_internal_id;
  group_relay_log_pos = temp_group_relay_log_pos;
  group_master_log_pos = temp_group_master_log_pos;
  checkpoint_relay_log_pos = temp_checkpoint_relay_log_pos;
  checkpoint_master_log_pos = temp_checkpoint_master_log_pos;
  worker_checkpoint_seqno = temp_checkpoint_seqno;

  return false;
}

/*
  This function is used to make a copy of the worker object before we
  destroy it while STOP SLAVE. This new object is then used to report the
  worker status until next START SLAVE following which the new worker objetcs
  will be used.
*/
void Slave_worker::copy_values_for_PFS(ulong worker_id,
                                       en_running_state thd_running_status,
                                       THD *worker_thd, const Error &last_error,
                                       Gtid_monitoring_info *monitoring_info) {
  id = worker_id;
  running_status = thd_running_status;
  info_thd = worker_thd;
  m_last_error = last_error;
  monitoring_info->copy_info_to(get_gtid_monitoring_info());
}

bool Slave_worker::set_info_search_keys(Rpl_info_handler *to) {
  DBUG_TRACE;

  /* primary keys are Id and channel_name */
  if (to->set_info(0, (int)internal_id) ||
      to->set_info(LINE_FOR_CHANNEL, channel))
    return true;

  return false;
}

bool Slave_worker::write_info(Rpl_info_handler *to) {
  DBUG_TRACE;

  ulong nbytes = (ulong)no_bytes_in_map(&group_executed);
  uchar *buffer = (uchar *)group_executed.bitmap;
  DBUG_ASSERT(nbytes <= (c_rli->checkpoint_group + 7) / 8);

  if (to->prepare_info_for_write()) return true;
  if (to->get_rpl_info_type() != INFO_REPOSITORY_FILE) {
    if (to->set_info((int)internal_id) || to->set_info(group_relay_log_name) ||
        to->set_info((ulong)group_relay_log_pos) ||
        to->set_info(group_master_log_name) ||
        to->set_info((ulong)group_master_log_pos) ||
        to->set_info(checkpoint_relay_log_name) ||
        to->set_info((ulong)checkpoint_relay_log_pos) ||
        to->set_info(checkpoint_master_log_name) ||
        to->set_info((ulong)checkpoint_master_log_pos) ||
        to->set_info(worker_checkpoint_seqno) || to->set_info(nbytes) ||
        to->set_info(buffer, (size_t)nbytes) || to->set_info(channel))
      return true;
  } else {
    if (to->set_info(
            LINES_IN_WORKER_INFO - 1,
            "%d\n%s\n%lu\n%s\n%lu\n%s\n%lu\n%s\n%lu\n%lu\n%lu\n",
            (int)internal_id, group_relay_log_name, (ulong)group_relay_log_pos,
            group_master_log_name, (ulong)group_master_log_pos,
            checkpoint_relay_log_name, (ulong)checkpoint_relay_log_pos,
            checkpoint_master_log_name, (ulong)checkpoint_master_log_pos,
            worker_checkpoint_seqno, nbytes) ||
        to->set_info(buffer, (size_t)nbytes) || to->set_info(channel))
      return true;
  }

  return false;
}

/**
   Clean up a part of Worker info table that is regarded in
   in gaps collecting at recovery.
   This worker won't contribute to recovery bitmap at future
   slave restart (see @c mts_recovery_groups).

   @return false as success true as failure
*/
bool Slave_worker::reset_recovery_info() {
  DBUG_TRACE;

  set_group_master_log_name("");
  set_group_master_log_pos(0);

  return flush_info(true);
}

size_t Slave_worker::get_number_worker_fields() {
  return sizeof(info_slave_worker_fields) / sizeof(info_slave_worker_fields[0]);
}

void Slave_worker::set_nullable_fields(MY_BITMAP *nullable_fields) {
  bitmap_init(nullable_fields, nullptr,
              Slave_worker::get_number_worker_fields());
  bitmap_clear_all(nullable_fields);
}

const char *Slave_worker::get_master_log_name() {
  Slave_job_group *ptr_g = c_rli->gaq->get_job_group(gaq_index);

  return (ptr_g->checkpoint_log_name != nullptr) ? ptr_g->checkpoint_log_name
                                                 : checkpoint_master_log_name;
}

bool Slave_worker::commit_positions(Log_event *ev, Slave_job_group *ptr_g,
                                    bool force) {
  DBUG_TRACE;

  /*
    Initial value of checkpoint_master_log_name is learned from
    group_master_log_name. The latter can be passed to Worker
    at rare event of master binlog rotation.
    This initialization is needed to provide to Worker info
    on physical coordiates during execution of the very first group
    after a rotation.
  */
  if (ptr_g->group_master_log_name != nullptr) {
    strmake(group_master_log_name, ptr_g->group_master_log_name,
            sizeof(group_master_log_name) - 1);
    my_free(ptr_g->group_master_log_name);
    ptr_g->group_master_log_name = nullptr;
    strmake(checkpoint_master_log_name, group_master_log_name,
            sizeof(checkpoint_master_log_name) - 1);
  }
  if (ptr_g->checkpoint_log_name != nullptr) {
    strmake(checkpoint_relay_log_name, ptr_g->checkpoint_relay_log_name,
            sizeof(checkpoint_relay_log_name) - 1);
    checkpoint_relay_log_pos = ptr_g->checkpoint_relay_log_pos;
    strmake(checkpoint_master_log_name, ptr_g->checkpoint_log_name,
            sizeof(checkpoint_master_log_name) - 1);
    checkpoint_master_log_pos = ptr_g->checkpoint_log_pos;

    my_free(ptr_g->checkpoint_log_name);
    ptr_g->checkpoint_log_name = nullptr;
    my_free(ptr_g->checkpoint_relay_log_name);
    ptr_g->checkpoint_relay_log_name = nullptr;

    bitmap_copy(&group_shifted, &group_executed);
    bitmap_clear_all(&group_executed);
    for (uint pos = ptr_g->shifted; pos < c_rli->checkpoint_group; pos++) {
      if (bitmap_is_set(&group_shifted, pos))
        bitmap_set_bit(&group_executed, pos - ptr_g->shifted);
    }
  }
  /*
    Extracts an updated relay-log name to store in Worker's rli.
  */
  if (ptr_g->group_relay_log_name) {
    DBUG_ASSERT(strlen(ptr_g->group_relay_log_name) + 1 <=
                sizeof(group_relay_log_name));
    strmake(group_relay_log_name, ptr_g->group_relay_log_name,
            sizeof(group_relay_log_name) - 1);
  }

  DBUG_ASSERT(ptr_g->checkpoint_seqno <= (c_rli->checkpoint_group - 1));

  bitmap_set_bit(&group_executed, ptr_g->checkpoint_seqno);
  worker_checkpoint_seqno = ptr_g->checkpoint_seqno;
  group_relay_log_pos = ev->future_event_relay_log_pos;
  group_master_log_pos = ev->common_header->log_pos;

  /*
    Directly accessing c_rli->get_group_master_log_name() does not
    represent a concurrency issue because the current code places
    a synchronization point when master rotates.
  */
  strmake(group_master_log_name, c_rli->get_group_master_log_name(),
          sizeof(group_master_log_name) - 1);

  DBUG_PRINT("mts", ("Committing worker-id %lu group master log pos %llu "
                     "group master log name %s checkpoint sequence number %lu.",
                     id, group_master_log_pos, group_master_log_name,
                     worker_checkpoint_seqno));

  DBUG_EXECUTE_IF("mts_debug_concurrent_access",
                  { mts_debug_concurrent_access++; };);

  return flush_info(force);
}

void Slave_worker::rollback_positions(Slave_job_group *ptr_g) {
  if (!is_transactional()) {
    bitmap_clear_bit(&group_executed, ptr_g->checkpoint_seqno);
    flush_info(false);
  }
}

static void free_entry(db_worker_hash_entry *entry) {
  THD *c_thd = current_thd;

  DBUG_TRACE;

  DBUG_PRINT("info", ("free_entry %s, %zu", entry->db, strlen(entry->db)));

  DBUG_ASSERT(c_thd->system_thread == SYSTEM_THREAD_SLAVE_SQL);

  /*
    Although assert is correct valgrind senses entry->worker can be freed.

    DBUG_ASSERT(entry->usage == 0 ||
                !entry->worker    ||  // last entry owner could have errored out
                entry->worker->running_status != Slave_worker::RUNNING);
  */

  mts_move_temp_tables_to_thd(c_thd, entry->temporary_tables);
  entry->temporary_tables = nullptr;

  my_free(const_cast<char *>(entry->db));
  my_free(entry);
}

bool init_hash_workers(Relay_log_info *rli) {
  DBUG_TRACE;

  rli->inited_hash_workers = true;
  mysql_mutex_init(key_mutex_slave_worker_hash, &rli->slave_worker_hash_lock,
                   MY_MUTEX_INIT_FAST);
  mysql_cond_init(key_cond_slave_worker_hash, &rli->slave_worker_hash_cond);

  return false;
}

void destroy_hash_workers(Relay_log_info *rli) {
  DBUG_TRACE;
  if (rli->inited_hash_workers) {
    rli->mapping_db_to_worker.clear();
    mysql_mutex_destroy(&rli->slave_worker_hash_lock);
    mysql_cond_destroy(&rli->slave_worker_hash_cond);
    rli->inited_hash_workers = false;
  }
}

/**
   Relocating temporary table reference into @c entry's table list head.
   Sources can be the coordinator's and the Worker's thd->temporary_tables.

   @param table   TABLE instance pointer
   @param thd     THD instance pointer of the source of relocation
   @param entry   db_worker_hash_entry instance pointer

   @note  thd->temporary_tables can become NULL

   @return the pointer to a table following the unlinked
*/
TABLE *mts_move_temp_table_to_entry(TABLE *table, THD *thd,
                                    db_worker_hash_entry *entry) {
  TABLE *ret = table->next;

  if (table->prev) {
    table->prev->next = table->next;
    if (table->prev->next) table->next->prev = table->prev;
  } else {
    /* removing the first item from the list */
    DBUG_ASSERT(table == thd->temporary_tables);

    thd->temporary_tables = table->next;
    if (thd->temporary_tables) table->next->prev = nullptr;
  }
  table->next = entry->temporary_tables;
  table->prev = nullptr;
  if (table->next) table->next->prev = table;
  entry->temporary_tables = table;

  return ret;
}

/**
   Relocation of the list of temporary tables to thd->temporary_tables.

   @param thd     THD instance pointer of the destination
   @param temporary_tables
                  the source temporary_tables list

   @note     destroying references to the source list, if necessary,
             is left to the caller.

   @return   the post-merge value of thd->temporary_tables.
*/
TABLE *mts_move_temp_tables_to_thd(THD *thd, TABLE *temporary_tables) {
  DBUG_TRACE;
  TABLE *table = temporary_tables;
  if (!table) return nullptr;

  // accept only if this is the start of the list.
  DBUG_ASSERT(!table->prev);

  // walk along the source list and associate the tables with thd
  do {
    table->in_use = thd;
  } while (table->next && (table = table->next));

  // link the former list against the tail of the source list
  if (thd->temporary_tables) thd->temporary_tables->prev = table;
  table->next = thd->temporary_tables;
  thd->temporary_tables = temporary_tables;
  return thd->temporary_tables;
}

/**
   Relocating references of temporary tables of a database
   of the entry argument from THD into the entry.

   @param thd    THD pointer of the source temporary_tables list
   @param entry  a pointer to db_worker_hash_entry record
                 containing database descriptor and temporary_tables list.

*/
static void move_temp_tables_to_entry(THD *thd, db_worker_hash_entry *entry) {
  for (TABLE *table = thd->temporary_tables; table;) {
    if (strcmp(table->s->db.str, entry->db) == 0) {
      // table pointer is shifted inside the function
      table = mts_move_temp_table_to_entry(table, thd, entry);
    } else {
      table = table->next;
    }
  }
}

static double calculate_imbalance(Relay_log_info *rli,
                                  const Slave_worker_array *ws) {
  DBUG_ENTER("calculate_imbalance");

  mysql_mutex_assert_owner(&rli->slave_worker_hash_lock);

  // case: if the number of workers == number of db's
  // nothing can be done to rebalance the workers
  if (ws->size() == rli->mapping_db_to_worker.size()) DBUG_RETURN(0);

  double max_usage = 0;
  double total_usage = 0;
  std::vector<ulong> load_on_workers(ws->size());

  // calculate the total load per worker
  for (auto const &entry : rli->mapping_db_to_worker) {
    load_on_workers[entry.second->worker->id] += entry.second->load;
  }

  for (ulong load : load_on_workers) {
    total_usage += load;
    if (load > max_usage) max_usage = load;
  }

  if (total_usage == 0) DBUG_RETURN(0);

  DBUG_RETURN(max_usage / total_usage * 100);
}

void rebalance_workers(Relay_log_info *rli) {
  DBUG_ENTER("rebalance_workers");
  DBUG_ASSERT(!rli->curr_group_seen_begin && !rli->curr_group_seen_gtid);

  const Slave_worker_array *ws = &rli->workers;
  std::priority_queue<worker_load, std::vector<worker_load>,
                      std::greater<worker_load>>
      heap;

  mysql_mutex_lock(&rli->slave_worker_hash_lock);

  double imbalance = calculate_imbalance(rli, ws);
  if (imbalance < opt_mts_imbalance_threshold) {
    mysql_mutex_unlock(&rli->slave_worker_hash_lock);
    DBUG_VOID_RETURN;
  }

  // initializing the heap
  for (Slave_worker *current_worker : *ws) {
    heap.emplace(worker_load(current_worker));
  }

  double new_max_wrk_load = 0;
  double total_wrk_load = 0;
  // build the heap according to the current load
  for (auto const &entry : rli->mapping_db_to_worker) {
    // assign least occupied worker
    worker_load wrk_load = heap.top();
    wrk_load.db_entries.push_back(entry.second.get());
    heap.pop();

    wrk_load.load += entry.second->load;

    if (new_max_wrk_load < wrk_load.load) new_max_wrk_load = wrk_load.load;
    total_wrk_load += entry.second->load;

    heap.emplace(std::move(wrk_load));
  }

  // before we remap, check if the rebalancing will make the imbalance
  // less than the threshold if no we should leave the map alone and
  // there is no need to sync the workers
  // NOTE: the comparision method here should be equivalent to @
  // calculate_imbalance
  if (total_wrk_load &&
      new_max_wrk_load / total_wrk_load * 100 > opt_mts_imbalance_threshold) {
    mysql_mutex_unlock(&rli->slave_worker_hash_lock);
    DBUG_VOID_RETURN;
  }

  // remap the db->worker map
  while (!heap.empty()) {
    worker_load wrk_load = heap.top();
    for (db_worker_hash_entry *entry : wrk_load.db_entries) {
      entry->worker = wrk_load.worker;
    }
    heap.pop();
  }

  mysql_mutex_unlock(&rli->slave_worker_hash_lock);

  // wait for all workers to finish
  (void)rli->current_mts_submode->wait_for_workers_to_finish(rli);

  DBUG_VOID_RETURN;
}

/**
   The function produces a reference to the struct of a Worker
   that has been or will be engaged to process the @c dbname -keyed  partition
   (D). It checks a local to Coordinator CGAP list first and returns
   @c last_assigned_worker when found (todo: assert).

   Otherwise, the partition is appended to the current group list:

        CGAP .= D

   here .= is concatenate operation,
   and a possible D's Worker id is searched in Assigned Partition Hash
   (APH) that collects tuples (P, W_id, U, mutex, cond).
   In case not found,

        W_d := W_c unless W_c is NULL.

   When W_c is NULL it is assigned to a least occupied as defined by
   @c get_least_occupied_worker().

        W_d := W_c := W_{least_occupied}

        APH .=  a new (D, W_d, 1)

   In a case APH contains W_d == W_c, (assert U >= 1)

        update APH set  U++ where  APH.P = D

   The case APH contains a W_d != W_c != NULL assigned to D-partition represents
   the hashing conflict and is handled as the following:

     a. marks the record of APH with a flag requesting to signal in the
        cond var when `U' the usage counter drops to zero by the other Worker;
     b. waits for the other Worker to finish tasks on that partition and
        gets the signal;
     c. updates the APH record to point to the first Worker (naturally, U := 1),
        scheduled the event, and goes back into the parallel mode

   @param  dbname      pointer to c-string containing database name
                       It can be empty string to indicate specific locking
                       to faciliate sequential applying.
   @param  rli         pointer to Coordinators relay-log-info instance
   @param  ptr_entry   reference to a pointer to the resulted entry in
                       the Assigne Partition Hash where
                       the entry's pointer is stored at return.
   @param  need_temp_tables
                       if false migration of temporary tables not needed
   @param  last_worker caller opts for this Worker, it must be
                       rli->last_assigned_worker if one is determined.

   @note modifies  CGAP, APH and unlinks @c dbname -keyd temporary tables
         from C's thd->temporary_tables to move them into the entry record.

   @return the pointer to a Worker struct
*/
Slave_worker *map_db_to_worker(const char *dbname, Relay_log_info *rli,
                               db_worker_hash_entry **ptr_entry,
                               bool need_temp_tables,
                               Slave_worker *last_worker) {
  Slave_worker_array *workers = &rli->workers;

  THD *thd = rli->info_thd;

  DBUG_TRACE;

  DBUG_ASSERT(!rli->last_assigned_worker ||
              rli->last_assigned_worker == last_worker);
  DBUG_ASSERT(is_mts_db_partitioned(rli));

  if (!rli->inited_hash_workers) return nullptr;

  db_worker_hash_entry *entry = nullptr;
  size_t dblength = strlen(dbname);

  // Search in CGAP
  for (db_worker_hash_entry **it = rli->curr_group_assigned_parts.begin();
       it != rli->curr_group_assigned_parts.end(); ++it) {
    entry = *it;
    if ((uchar)entry->db_len != dblength)
      continue;
    else if (strncmp(entry->db, const_cast<char *>(dbname), dblength) == 0) {
      *ptr_entry = entry;
      return last_worker;
    }
  }

  DBUG_PRINT("info", ("Searching for %s, %zu", dbname, dblength));

  mysql_mutex_lock(&rli->slave_worker_hash_lock);

  std::string key(dbname, dblength);
  entry = find_or_nullptr(rli->mapping_db_to_worker, key);
  if (!entry) {
    DBUG_PRINT("debug", ("NO ENTRY found for: %s!", dbname));
    /*
      The database name was not found which means that a worker never
      processed events from that database. In such case, we need to
      map the database to a worker my inserting an entry into the
      hash map.
    */
    bool ret;
    char *db = nullptr;

    mysql_mutex_unlock(&rli->slave_worker_hash_lock);

    DBUG_PRINT("info", ("Inserting %s, %zu", dbname, dblength));
    /*
      Allocate an entry to be inserted and if the operation fails
      an error is returned.
    */
    if (!(db = (char *)my_malloc(key_memory_db_worker_hash_entry, dblength + 1,
                                 MYF(0))))
      goto err;
    if (!(entry = (db_worker_hash_entry *)my_malloc(
              key_memory_db_worker_hash_entry, sizeof(db_worker_hash_entry),
              MYF(0)))) {
      my_free(db);
      goto err;
    }
    my_stpcpy(db, dbname);
    entry->db = db;
    entry->db_len = strlen(db);
    entry->usage = 1;
    entry->load = 1;
    entry->temporary_tables = nullptr;
    /*
      Unless \exists the last assigned Worker, get a free worker based
      on a policy described in the function get_least_occupied_worker().
    */
    mysql_mutex_lock(&rli->slave_worker_hash_lock);

    entry->worker = (!last_worker)
                        ? get_least_occupied_worker(rli, workers, nullptr)
                        : last_worker;
    entry->worker->usage_partition++;
    if (rli->mapping_db_to_worker.size() > mts_partition_hash_soft_max) {
      /*
        remove zero-usage (todo: rare or long ago scheduled) records.
        Free the element if the usage of the hash entry is 0 or not.
      */
      for (auto it = rli->mapping_db_to_worker.begin();
           it != rli->mapping_db_to_worker.end();) {
        DBUG_ASSERT(!entry->temporary_tables || !entry->temporary_tables->prev);
        DBUG_ASSERT(!thd->temporary_tables || !thd->temporary_tables->prev);

        db_worker_hash_entry *zero_entry = it->second.get();
        if (zero_entry->usage == 0) {
          mts_move_temp_tables_to_thd(thd, zero_entry->temporary_tables);
          zero_entry->temporary_tables = nullptr;
          it = rli->mapping_db_to_worker.erase(it);
        } else
          ++it;
      }
    }

    ret =
        !rli->mapping_db_to_worker
             .emplace(entry->db, unique_ptr_with_deleter<db_worker_hash_entry>(
                                     entry, free_entry))
             .second;

    if (ret) {
      my_free(db);
      entry = nullptr;
      goto err;
    }
    DBUG_PRINT("info", ("Inserted %s, %zu", entry->db, strlen(entry->db)));
    DBUG_PRINT("debug", ("worker=%lu, partition=%s, usage=%ld, wait=false!",
                         entry->worker->id, entry->db,
                         entry->worker->usage_partition++));
  } else {
    DBUG_PRINT("debug", ("ENTRY found for: %s!", entry->db));
    /* There is a record. Either  */
    if (entry->usage == 0) {
      entry->worker = (!last_worker)
                          ? get_least_occupied_worker(rli, workers, nullptr)
                          : last_worker;
      entry->worker->usage_partition++;
      entry->usage++;
      entry->load++;
      DBUG_PRINT(
          "debug",
          ("worker=%lu, partition=%s, usage=%ld (was 0), wait=false!",
           entry->worker->id, entry->db, entry->worker->usage_partition++));
    } else if (entry->worker == last_worker || !last_worker) {
      DBUG_ASSERT(entry->worker);

      entry->usage++;
      entry->load++;
      DBUG_PRINT("debug", ("worker=%lu, partition=%s, usage=%ld, wait=false!",
                           entry->worker->id, entry->db,
                           entry->worker->usage_partition++));
    } else {
      // The case APH contains a W_d != W_c != NULL assigned to
      // D-partition represents
      // the hashing conflict and is handled as the following:
      PSI_stage_info old_stage;

      DBUG_ASSERT(last_worker != nullptr &&
                  rli->curr_group_assigned_parts.size() > 0);

      // future assignenment and marking at the same time
      entry->worker = last_worker;
      DBUG_PRINT("debug", ("worker=%lu, partition=%s, usage=%ld, wait=true!",
                           entry->worker->id, entry->db,
                           entry->worker->usage_partition++));
      // loop while a user thread is stopping Coordinator gracefully
      do {
        thd->ENTER_COND(
            &rli->slave_worker_hash_cond, &rli->slave_worker_hash_lock,
            &stage_slave_waiting_worker_to_release_partition, &old_stage);
        mysql_cond_wait(&rli->slave_worker_hash_cond,
                        &rli->slave_worker_hash_lock);
      } while (entry->usage != 0 && !thd->killed);

      mysql_mutex_unlock(&rli->slave_worker_hash_lock);
      thd->EXIT_COND(&old_stage);
      if (thd->killed) {
        entry = nullptr;
        goto err;
      }
      mysql_mutex_lock(&rli->slave_worker_hash_lock);
      entry->usage = 1;
      entry->load = 1;
      entry->worker->usage_partition++;
    }
  }

  /*
     relocation belonging to db temporary tables from C to W via entry
  */
  if (entry->usage == 1 && need_temp_tables) {
    if (!entry->temporary_tables) {
      if (entry->db_len != 0) {
        move_temp_tables_to_entry(thd, entry);
      } else {
        entry->temporary_tables = thd->temporary_tables;
        thd->temporary_tables = nullptr;
      }
    }
#ifndef DBUG_OFF
    else {
      // all entries must have been emptied from temps by the caller

      for (TABLE *table = thd->temporary_tables; table; table = table->next) {
        DBUG_ASSERT(0 != strcmp(table->s->db.str, entry->db));
      }
    }
#endif
  }
  mysql_mutex_unlock(&rli->slave_worker_hash_lock);

  DBUG_ASSERT(entry);

err:
  if (entry) {
    DBUG_PRINT("info",
               ("Updating %s with worker %lu", entry->db, entry->worker->id));
    rli->curr_group_assigned_parts.push_back(entry);
    *ptr_entry = entry;
  }
  return entry ? entry->worker : nullptr;
}

/**
   Get the least occupied worker.

   @param rli pointer to Relay_log_info of Coordinator
   @param ws  dynarray of pointers to Slave_worker
   @param ev event for which we are searching for a worker

   @return a pointer to chosen Slave_worker instance

*/
Slave_worker *get_least_occupied_worker(Relay_log_info *rli,
                                        Slave_worker_array *ws, Log_event *ev) {
  return rli->current_mts_submode->get_least_occupied_worker(rli, ws, ev);
}

/**
   Deallocation routine to cancel out few effects of
   @c map_db_to_worker().
   Involved into processing of the group APH tuples are updated.
   @c last_group_done_index member is set to the GAQ index of
   the current group.
   CGEP the Worker partition cache is cleaned up.

   @param ev     a pointer to Log_event
   @param error  error code after processing the event by caller.
*/
void Slave_worker::slave_worker_ends_group(Log_event *ev, int error) {
  DBUG_TRACE;
  Slave_job_group *ptr_g = nullptr;

  if (!error) {
    ptr_g = c_rli->gaq->get_job_group(gaq_index);

    DBUG_ASSERT(gaq_index == ev->mts_group_idx);
    /*
      It guarantees that the worker is removed from order commit queue when
      its transaction doesn't binlog anything. It will break innodb group
      commit, but it should rarely happen.
    */
    Commit_order_manager::wait_and_finish(info_thd, false);

    // first ever group must have relay log name
    DBUG_ASSERT(last_group_done_index != c_rli->gaq->size ||
                ptr_g->group_relay_log_name != nullptr);
    DBUG_ASSERT(ptr_g->worker_id == id);

    /*
      DDL that has not yet updated the slave info repository does it now.
    */
    if (ev->get_type_code() != binary_log::XID_EVENT &&
        ev->get_type_code() != binary_log::TRANSACTION_PAYLOAD_EVENT &&
        !is_committed_ddl(ev)) {
      commit_positions(ev, ptr_g, true);
      DBUG_EXECUTE_IF(
          "crash_after_commit_and_update_pos",
          sql_print_information("Crashing crash_after_commit_and_update_pos.");
          flush_info(true); DBUG_SUICIDE(););
    }

    ptr_g->group_master_log_pos = group_master_log_pos;
    ptr_g->group_relay_log_pos = group_relay_log_pos;
    ptr_g->done.store(1);
    last_group_done_index = gaq_index;
    last_groups_assigned_index = ptr_g->total_seqno;
    reset_gaq_index();
    groups_done++;

  } else {
    if (running_status != STOP_ACCEPTED) {
      // tagging as exiting so Coordinator won't be able synchronize with it
      mysql_mutex_lock(&jobs_lock);
      running_status = ERROR_LEAVING;
      mysql_mutex_unlock(&jobs_lock);

      // Fatal error happens, it notifies the following transaction to rollback
      Commit_order_manager::wait_and_finish(info_thd, true);

      // Killing Coordinator to indicate eventual consistency error
      mysql_mutex_lock(&c_rli->info_thd->LOCK_thd_data);
      c_rli->info_thd->awake(THD::KILL_QUERY);
      mysql_mutex_unlock(&c_rli->info_thd->LOCK_thd_data);
    }
  }

  /*
    Cleanup relating to the last executed group regardless of error.
  */
  if (current_mts_submode->get_type() == MTS_PARALLEL_TYPE_DB_NAME) {
#ifndef DBUG_OFF
    {
      std::stringstream ss;
      for (size_t i = 0; i < curr_group_exec_parts.size(); i++) {
        if (curr_group_exec_parts[i]->db_len) {
          ss << curr_group_exec_parts[i]->db << ", ";
        }
      }
      DBUG_PRINT("debug", ("UNASSIGN %p %s", current_thd, ss.str().c_str()));
    }
#endif
    for (size_t i = 0; i < curr_group_exec_parts.size(); i++) {
      db_worker_hash_entry *entry = curr_group_exec_parts[i];

      mysql_mutex_lock(&c_rli->slave_worker_hash_lock);

      DBUG_ASSERT(entry);

      entry->usage--;

      DBUG_ASSERT(entry->usage >= 0);

      if (entry->usage == 0) {
        usage_partition--;
        /*
          The detached entry's temp table list, possibly updated, remains
          with the entry at least until time Coordinator will deallocate it
          from the hash, that is either due to stop or extra size of the hash.
        */
        DBUG_ASSERT(usage_partition >= 0);
        DBUG_ASSERT(this->info_thd->temporary_tables == nullptr);
        DBUG_ASSERT(!entry->temporary_tables || !entry->temporary_tables->prev);

        if (entry->worker != this)  // Coordinator is waiting
        {
          DBUG_PRINT("info", ("Notifying entry %p release by worker %lu", entry,
                              this->id));

          mysql_cond_signal(&c_rli->slave_worker_hash_cond);
        }
      } else
        DBUG_ASSERT(usage_partition != 0);

      mysql_mutex_unlock(&c_rli->slave_worker_hash_lock);
    }

    curr_group_exec_parts.clear();
    curr_group_exec_parts.shrink_to_fit();

    if (error) {
      // Awakening Coordinator that could be waiting for entry release
      mysql_mutex_lock(&c_rli->slave_worker_hash_lock);
      mysql_cond_signal(&c_rli->slave_worker_hash_cond);
      mysql_mutex_unlock(&c_rli->slave_worker_hash_lock);
    }
  } else if (current_mts_submode->get_type() ==
             MTS_PARALLEL_TYPE_LOGICAL_CLOCK) {
    DBUG_ASSERT(current_mts_submode->get_type() ==
                MTS_PARALLEL_TYPE_LOGICAL_CLOCK);
    /*
      Check if there're any waiter. If there're try incrementing lwm and
      signal to those who've got sasfied with the waiting condition.

      In a "good" "likely" execution branch the waiter set is expected
      to be empty. LWM is advanced by Coordinator asynchronously.
      Also lwm is advanced by a dependent Worker when it inserts its waiting
      request into the waiting list.
    */
    Mts_submode_logical_clock *mts_submode =
        static_cast<Mts_submode_logical_clock *>(c_rli->current_mts_submode);
    int64 min_child_waited_logical_ts =
        mts_submode->min_waited_timestamp.load();

    DBUG_EXECUTE_IF("slave_worker_ends_group_before_signal_lwm", {
      const char act[] = "now WAIT_FOR worker_continue";
      DBUG_ASSERT(!debug_sync_set_action(current_thd, STRING_WITH_LEN(act)));
    });

    if (unlikely(error)) {
      mysql_mutex_lock(&c_rli->mts_gaq_LOCK);
      mts_submode->is_error = true;
      if (mts_submode->min_waited_timestamp != SEQ_UNINIT)
        mysql_cond_signal(&c_rli->logical_clock_cond);
      mysql_mutex_unlock(&c_rli->mts_gaq_LOCK);
    } else if (min_child_waited_logical_ts != SEQ_UNINIT) {
      mysql_mutex_lock(&c_rli->mts_gaq_LOCK);

      /*
        min_child_waited_logical_ts may include an old value, so we need to
        check it again after getting the lock.
      */
      if (mts_submode->min_waited_timestamp != SEQ_UNINIT) {
        longlong curr_lwm = mts_submode->get_lwm_timestamp(c_rli, true);

        if (mts_submode->clock_leq(mts_submode->min_waited_timestamp,
                                   curr_lwm)) {
          /*
            There's a transaction that depends on the current.
          */
          mysql_cond_signal(&c_rli->logical_clock_cond);
        }
      }
      mysql_mutex_unlock(&c_rli->mts_gaq_LOCK);
    }

#ifndef DBUG_OFF
    curr_group_seen_sequence_number = false;
#endif
  }
  curr_group_seen_gtid = false;
  curr_group_seen_metadata = false;
}

/**
   two index comparision to determine which of the two
   is ordered first.

   @note   The caller makes sure the args are within the valid
           range, incl cases the queue is empty or full.

   @return true  if the first arg identifies a queue entity ordered
                 after one defined by the 2nd arg,
           false otherwise.
*/
template <typename Element_type>
bool circular_buffer_queue<Element_type>::gt(ulong i, ulong k) {
  DBUG_ASSERT(i < size && k < size);
  DBUG_ASSERT(avail != entry);

  if (i >= entry)
    if (k >= entry)
      return i > k;
    else
      return false;
  else if (k >= entry)
    return true;
  else
    return i > k;
}

Slave_committed_queue::Slave_committed_queue(ulong max, uint n)
    : circular_buffer_queue<Slave_job_group>(max),
      inited(false),
      last_done(key_memory_Slave_job_group_group_relay_log_name) {
  if (max >= (ulong)-1 || !inited_queue)
    return;
  else
    inited = true;

  last_done.resize(n);

  lwm.group_relay_log_name = (char *)my_malloc(
      key_memory_Slave_job_group_group_relay_log_name, FN_REFLEN + 1, MYF(0));
  lwm.group_relay_log_name[0] = 0;
  lwm.sequence_number = SEQ_UNINIT;
}

#ifndef DBUG_OFF
bool Slave_committed_queue::count_done(Relay_log_info *rli) {
  ulong i, k, cnt = 0;

  for (i = entry, k = 0; k < len; i = (i + 1) % size, k++) {
    Slave_job_group *ptr_g;

    ptr_g = &m_Q[i];

    if (ptr_g->worker_id != MTS_WORKER_UNDEF && ptr_g->done) cnt++;
  }

  DBUG_ASSERT(cnt <= size);

  DBUG_PRINT("mts",
             ("Checking if it can simulate a crash:"
              " mts_checkpoint_group %u counter %lu parallel slaves %lu\n",
              opt_mts_checkpoint_group, cnt, rli->slave_parallel_workers));

  return (cnt == (rli->slave_parallel_workers * opt_mts_checkpoint_group));
}
#endif

/**
   The queue is processed from the head item by item
   to purge items representing committed groups.
   Progress in GAQ is assessed through comparision of GAQ index value
   with Worker's @c last_group_done_index.
   Purging breaks at a first discovered gap, that is an item
   that the assinged item->w_id'th Worker has not yet completed.

   The caller is supposed to be the checkpoint handler.

   A copy of the last discarded item containing
   the refreshed value of the committed low-water-mark is stored
   into @c lwm container member for further caller's processing.
   @c last_done is updated with the latest total_seqno for each Worker
   that was met during GAQ parse.

   @note dyn-allocated members of Slave_job_group such as
         group_relay_log_name as freed here.

   @return number of discarded items
*/
ulong Slave_committed_queue::move_queue_head(Slave_worker_array *ws) {
  DBUG_TRACE;
  ulong i, cnt = 0;

  for (i = entry; i != avail && !empty(); cnt++, i = (i + 1) % size) {
    Slave_worker *w_i;
    Slave_job_group *ptr_g;
    char grl_name[FN_REFLEN];

#ifndef DBUG_OFF
    if (DBUG_EVALUATE_IF("check_slave_debug_group", 1, 0) &&
        cnt == opt_mts_checkpoint_period)
      return cnt;
#endif

    grl_name[0] = 0;
    ptr_g = &m_Q[i];

    /*
      The current job has not been processed or it was not
      even assigned, this means there is a gap.
    */
    if (ptr_g->worker_id == MTS_WORKER_UNDEF || ptr_g->done.load() == 0)
      break; /* gap at i'th */

    /* Worker-id domain guard */
    static_assert(MTS_WORKER_UNDEF > MTS_MAX_WORKERS, "");

    w_i = ws->at(ptr_g->worker_id);

    /*
      Memorizes the latest valid group_relay_log_name.
    */
    if (ptr_g->group_relay_log_name) {
      strcpy(grl_name, ptr_g->group_relay_log_name);
      my_free(ptr_g->group_relay_log_name);
      /*
        It is important to mark the field as freed.
      */
      ptr_g->group_relay_log_name = nullptr;
    }

    /*
      Removes the job from the (G)lobal (A)ssigned (Q)ueue.
    */
    Slave_job_group g = Slave_job_group();
#ifndef DBUG_OFF
    ulong ind =
#endif
        de_queue(&g);

    /*
      Stores the memorized name into the result struct. Note that we
      take care of the pointer first and then copy the other elements
      by assigning the structures.
    */
    if (grl_name[0] != 0) {
      strcpy(lwm.group_relay_log_name, grl_name);
    }
    g.group_relay_log_name = lwm.group_relay_log_name;
    lwm = g;

    DBUG_ASSERT(ind == i);
    DBUG_ASSERT(!ptr_g->group_relay_log_name);
    DBUG_ASSERT(ptr_g->total_seqno == lwm.total_seqno);
#ifndef DBUG_OFF
    {
      ulonglong l = last_done[w_i->id];
      /*
        There must be some progress otherwise we should have
        exit the loop earlier.
      */
      DBUG_ASSERT(l < ptr_g->total_seqno);
    }
#endif
    /*
      This is used to calculate the last time each worker has
      processed events.
    */
    last_done[w_i->id] = ptr_g->total_seqno;
  }

  DBUG_ASSERT(cnt <= size);

  return cnt;
}

/**
   Finds low-water mark of committed jobs in GAQ.
   That is an index below which all jobs are marked as done.

   Notice the first available index is returned when the queue
   does not have any incomplete jobs. That includes cases of
   the empty and the full of complete jobs queue.
   A mutex protecting from concurrent LWM change by
   move_queue_head() (by Coordinator) should be taken by the caller.

   @param [out] arg_g  a double pointer to Slave job descriptor item
                       last marked with done-as-true boolean.
   @param start_index  a GAQ index to start/resume searching.
                       Caller is to make sure the index points into
                       assigned (occupied) range of circular buffer of GAQ.
   @return             GAQ index of the last consecutive done job, or the GAQ
                       size when none is found.
*/
ulong Slave_committed_queue::find_lwm(Slave_job_group **arg_g,
                                      ulong start_index) {
  Slave_job_group *ptr_g = nullptr;
  ulong i, k, cnt;

  DBUG_ASSERT(start_index <= size);

  if (empty()) return size;

  /*
    Loop continuation condition relies on
    (TODO: assert it)
    the start_index being in the running range:

       start_index \in [entry, avail - 1].

    It satisfies any queue size including 1.
    It does not satisfy the empty queue case which is bailed out earlier above.
  */
  for (i = start_index, cnt = 0;
       cnt < len - (start_index + size - entry) % size;
       i = (i + 1) % size, cnt++) {
    ptr_g = &m_Q[i];
    if (ptr_g->done.load() == 0) {
      if (cnt == 0) return size;  // the first node of the queue is not done
      break;
    }
  }
  ptr_g = &m_Q[k = (i + size - 1) % size];
  *arg_g = ptr_g;

  return k;
}

/**
   Method should be executed at slave system stop to
   cleanup dynamically allocated items that remained as unprocessed
   by Coordinator and Workers in their regular execution course.
*/
void Slave_committed_queue::free_dynamic_items() {
  ulong i, k;
  for (i = entry, k = 0; k < len; i = (i + 1) % size, k++) {
    Slave_job_group *ptr_g = &m_Q[i];
    if (ptr_g->group_relay_log_name) {
      my_free(ptr_g->group_relay_log_name);
    }
    if (ptr_g->checkpoint_log_name) {
      my_free(ptr_g->checkpoint_log_name);
    }
    if (ptr_g->checkpoint_relay_log_name) {
      my_free(ptr_g->checkpoint_relay_log_name);
    }
    if (ptr_g->group_master_log_name) {
      my_free(ptr_g->group_master_log_name);
    }
  }
  DBUG_ASSERT((avail == size /* full */ || entry == size /* empty */) ||
              i == avail /* all occupied are processed */);
}

void Slave_worker::do_report(loglevel level, int err_code, const char *msg,
                             va_list args) const {
  char buff_coord[MAX_SLAVE_ERRMSG];
  char buff_gtid[Gtid::MAX_TEXT_LENGTH + 1];
  const char *log_name =
      const_cast<Slave_worker *>(this)->get_master_log_name();
  ulonglong log_pos = const_cast<Slave_worker *>(this)->get_master_log_pos();
  bool is_group_replication_applier_channel =
      channel_map.is_group_replication_channel_name(c_rli->get_channel(), true);
  const Gtid_specification *gtid_next = &info_thd->variables.gtid_next;
  THD *thd = info_thd;

  gtid_next->to_string(global_sid_map, buff_gtid, true);

  if (is_group_replication_applier_channel) {
    snprintf(buff_coord, sizeof(buff_coord),
             "Worker %u failed executing transaction '%s'", internal_id,
             buff_gtid);
  } else {
    snprintf(buff_coord, sizeof(buff_coord),
             "Worker %u failed executing transaction '%s' at "
             "master log %s, end_log_pos %llu",
             internal_id, buff_gtid, log_name, log_pos);
  }

  /*
    Error reporting by the worker. The worker updates its error fields as well
    as reports the error in the log.
  */
  this->va_report(level, err_code, buff_coord, msg, args);

  if (level == ERROR_LEVEL && (!has_temporary_error(thd, err_code) ||
                               thd->get_transaction()->cannot_safely_rollback(
                                   Transaction_ctx::SESSION))) {
    char coordinator_errmsg[MAX_SLAVE_ERRMSG];
    const char *err_msg = nullptr;

    mysql_mutex_lock(&err_lock);
    if (m_last_error.number) {
      /* We know for sure there is an valid error */
      err_msg = m_last_error.message;
    } else {
      /* Fallback to generic error message just in case */
      err_msg = buff_coord;
    }

    if (is_group_replication_applier_channel) {
      snprintf(coordinator_errmsg, MAX_SLAVE_ERRMSG,
               "Coordinator stopped because there were error(s) in the "
               "worker(s). "
               "The most recent failure being: %s; "
               "See error log and/or "
               "performance_schema.replication_applier_status_by_worker "
               "table for "
               "more details about this failure or others, if any.",
               err_msg);
    } else {
      snprintf(coordinator_errmsg, MAX_SLAVE_ERRMSG,
               "Coordinator stopped because there were error(s) in the "
               "worker(s). "
               "The most recent failure being: %s; "
               "See error log and/or "
               "performance_schema.replication_applier_status_by_worker "
               "table for "
               "more details about this failure or others, if any.",
               err_msg);
    }
    mysql_mutex_unlock(&err_lock);

    /*
      We want to update the errors in coordinator as well as worker.
      The fill_coord_err_buf() function update the error number, message and
      timestamp fields. This function is different from va_report() as
      va_report() also logs the error message in the log apart from updating the
      error fields. So, the worker does the job of reporting the error in the
      log. We just make coordinator aware of the error.
    */
    c_rli->fill_coord_err_buf(level, err_code, coordinator_errmsg);
  }
}

#ifndef DBUG_OFF
static bool may_have_timestamp(Log_event *ev) {
  bool res = false;

  switch (ev->get_type_code()) {
    case binary_log::QUERY_EVENT:
      res = true;
      break;

    case binary_log::GTID_LOG_EVENT:
      res = true;
      break;

    default:
      break;
  }

  return res;
}

static int64 get_last_committed(Log_event *ev) {
  int64 res = SEQ_UNINIT;

  switch (ev->get_type_code()) {
    case binary_log::GTID_LOG_EVENT:
      res = static_cast<Gtid_log_event *>(ev)->last_committed;
      break;

    default:
      break;
  }

  return res;
}

static int64 get_sequence_number(Log_event *ev) {
  int64 res = SEQ_UNINIT;

  switch (ev->get_type_code()) {
    case binary_log::GTID_LOG_EVENT:
      res = static_cast<Gtid_log_event *>(ev)->sequence_number;
      break;

    default:
      break;
  }

  return res;
}
#endif

/**
  MTS worker main routine.
  The worker thread loops in waiting for an event, executing it and
  fixing statistics counters.

  @return 0 success
         -1 got killed or an error happened during applying
*/
int Slave_worker::slave_worker_exec_event(Log_event *ev) {
  Relay_log_info *rli = c_rli;
  THD *thd = info_thd;
  int ret = 0;

  DBUG_TRACE;

  if (found_commit_order_deadlock()) {
    return ER_LOCK_DEADLOCK;
  }

  if (is_mts_parallel_type_dependency(rli)) {
    Slave_job_group *ptr_g = rli->gaq->get_job_group(ev->mts_group_idx);
    ptr_g->worker = this;
    ptr_g->worker_id = id;
    ptr_g->shifted = bitmap_shifted;
    bitmap_shifted = 0;
  }

  thd->server_id = ev->server_id;
  thd->set_time();
  thd->lex->set_current_select(nullptr);
  if (!ev->common_header->when.tv_sec)
    ev->common_header->when.tv_sec = static_cast<long>(my_time(0));
  ev->thd = thd;  // todo: assert because up to this point, ev->thd == 0
  ev->worker = this;

#ifndef DBUG_OFF
  if (rli->current_mts_submode->get_type() == MTS_PARALLEL_TYPE_LOGICAL_CLOCK &&
      may_have_timestamp(ev) && !curr_group_seen_sequence_number) {
    curr_group_seen_sequence_number = true;

    longlong lwm_estimate =
        static_cast<Mts_submode_logical_clock *>(rli->current_mts_submode)
            ->estimate_lwm_timestamp();
    int64 last_committed = get_last_committed(ev);
    int64 sequence_number = get_sequence_number(ev);
    /*
      The commit timestamp waiting condition:

        lwm_estimate < last_committed  <=>  last_committed  \not <= lwm_estimate

      must have been satisfied by Coordinator.
      The first scheduled transaction does not have to wait for anybody.
    */
    DBUG_ASSERT(
        rli->gaq->entry == ev->mts_group_idx ||
        Mts_submode_logical_clock::clock_leq(last_committed, lwm_estimate));
    DBUG_ASSERT(lwm_estimate != SEQ_UNINIT ||
                rli->gaq->entry == ev->mts_group_idx);
    /*
      The current transaction's timestamp can't be less that lwm.
    */
    DBUG_ASSERT(sequence_number == SEQ_UNINIT ||
                !Mts_submode_logical_clock::clock_leq(
                    sequence_number, static_cast<Mts_submode_logical_clock *>(
                                         rli->current_mts_submode)
                                         ->estimate_lwm_timestamp()));
  }
#endif

  // Address partioning only in database mode
  if (!is_gtid_event(ev) && is_mts_db_partitioned(rli)) {
    if (ev->contains_partition_info(end_group_sets_max_dbs)) {
      uint num_dbs = ev->mts_number_dbs();

      if (num_dbs == OVER_MAX_DBS_IN_EVENT_MTS) num_dbs = 1;

      DBUG_ASSERT(num_dbs > 0);

      for (uint k = 0; k < num_dbs; k++) {
        bool found = false;

        for (size_t i = 0; i < curr_group_exec_parts.size() && !found; i++) {
          found = curr_group_exec_parts[i] == ev->mts_assigned_partitions[k];
        }
        if (!found) {
          /*
            notice, can't assert
            DBUG_ASSERT(ev->mts_assigned_partitions[k]->worker == worker);
            since entry could be marked as wanted by other worker.
          */
          curr_group_exec_parts.push_back(ev->mts_assigned_partitions[k]);
        }
      }
      end_group_sets_max_dbs = false;
    }
  }

  set_future_event_relay_log_pos(ev->future_event_relay_log_pos);
  set_master_log_pos(static_cast<ulong>(ev->common_header->log_pos));
  set_gaq_index(ev->mts_group_idx);

  ev->check_and_set_idempotent_recovery(rli, worker_last_gtid);

  ret = ev->do_apply_event_worker(this);

  if (is_gtid_event(ev)) {
    auto gtid_ev = static_cast<Gtid_log_event *>(ev);
    gtid_ev->extract_last_gtid_to(worker_last_gtid);
  }

  DBUG_EXECUTE_IF("after_executed_write_rows_event", {
    if (ev->get_type_code() == binary_log::WRITE_ROWS_EVENT) {
      static constexpr char act[] = "now signal executed";
      DBUG_ASSERT(opt_debug_sync_timeout > 0);
      DBUG_ASSERT(!debug_sync_set_action(thd, STRING_WITH_LEN(act)));
    }
  };);

  return ret;
}

/**
  Sleep for a given amount of seconds or until killed.

  @param seconds    The number of seconds to sleep.

  @retval True if the thread has been killed, false otherwise.
*/

bool Slave_worker::worker_sleep(ulong seconds) {
  bool ret = false;
  struct timespec abstime;
  mysql_mutex_t *lock = &jobs_lock;
  mysql_cond_t *cond = &jobs_cond;

  /* Absolute system time at which the sleep time expires. */
  set_timespec(&abstime, seconds);

  mysql_mutex_lock(lock);
  info_thd->ENTER_COND(cond, lock, nullptr, nullptr);

  while (!(ret = info_thd->killed || running_status != RUNNING)) {
    int error = mysql_cond_timedwait(cond, lock, &abstime);
    if (is_timeout(error)) break;
  }

  mysql_mutex_unlock(lock);
  info_thd->EXIT_COND(nullptr);
  return ret;
}

void Slave_worker::reset_commit_order_deadlock() {
  m_commit_order_deadlock.store(false);
}

bool Slave_worker::found_commit_order_deadlock() {
  return m_commit_order_deadlock.load();
}

void Slave_worker::report_commit_order_deadlock() {
  DBUG_TRACE;
  DBUG_ASSERT(get_commit_order_manager() != nullptr);
  m_commit_order_deadlock.store(true);
}

std::tuple<bool, bool, uint> Slave_worker::check_and_report_end_of_retries(
    THD *thd) {
  DBUG_TRACE;

  bool silent = false;
  uint error = 0;

  if (found_commit_order_deadlock()) {
    /*
      This transaction was allowed to be executed in parallel with other that
      happened earlier according to binary log order. It was asked to be
      rolled back by the other transaction as it was holding a lock that is
      needed by the other transaction to progress, according to binary log
      order this configure a deadlock.

      At this point, this transaction *should* have no non-temporary errors.

      Having a non-temporary error may be a sign of:

      a) Slave has diverged from the master;
      b) There is an issue in the logical clock allowing a transaction to be
         applied in parallel with its dependencies (the two transactions are
         trying to change the same record in parallel).

      For (a), a retry of this transaction will produce the same error. For
      (b), this transaction might succeed upon retry, allowing the slave to
      progress without manual intervention, but it is a sign of problems in LC
      generation at the master.

      So, we will make the worker to retry this transaction only if there is
      no error or the error is a temporary error.
    */
    Diagnostics_area *da = thd->get_stmt_da();
    if (!da->is_error() ||
        has_temporary_error(thd, da->is_error() ? da->mysql_errno() : 0,
                            &silent)) {
      error = ER_LOCK_DEADLOCK;
    }
#ifndef DBUG_OFF
    else {
      /*
        The non-debug binary will not retry this transactions, stopping the
        SQL thread because of the non-temporary error. But, as this situation
        is not supposed to happen as described in the comment above, we will
        fail an assert to ease the issue investigation when it happens.
      */
      if (DBUG_EVALUATE_IF("rpl_fake_cod_deadlock", 0, 1)) DBUG_ASSERT(false);
    }
#endif
  }

  if (!has_temporary_error(thd, error, &silent) ||
      thd->get_transaction()->cannot_safely_rollback(Transaction_ctx::SESSION))
    return std::make_tuple(true, silent, error);

  if (trans_retries >= slave_trans_retries) {
    thd->fatal_error();
    c_rli->report(ERROR_LEVEL, thd->get_stmt_da()->mysql_errno(),
                  "worker thread retried transaction %lu time(s) "
                  "in vain, giving up. Consider raising the value of "
                  "the slave_transaction_retries variable.",
                  trans_retries);
    return std::make_tuple(true, silent, error);
  }

  return std::make_tuple(false, silent, error);
}

bool Slave_worker::retry_transaction(uint start_relay_number,
                                     my_off_t start_relay_pos,
                                     uint end_relay_number,
                                     my_off_t end_relay_pos) {
  THD *thd = info_thd;
  bool silent = false;

  DBUG_TRACE;

  if (slave_trans_retries == 0) return true;

  do {
    /* Simulate a lock deadlock error */
    uint error = 0;
    bool ret;

    std::tie(ret, silent, error) = check_and_report_end_of_retries(thd);
    if (ret) return true;

    if (!silent) {
      trans_retries++;
      if (current_thd->rli_slave->is_processing_trx()) {
        // if the error code is zero, we get the top of the error stack
        uint transient_error =
            (error == 0) ? thd->get_stmt_da()->mysql_errno() : error;
        current_thd->rli_slave->retried_processing(
            transient_error, ER_THD_NONCONST(thd, transient_error),
            trans_retries);
#ifndef DBUG_OFF
        if (trans_retries == 2 || trans_retries == 6)
          DBUG_EXECUTE_IF("rpl_ps_tables_worker_retry", {
            char const act[] =
                "now SIGNAL signal.rpl_ps_tables_worker_retry_pause "
                "WAIT_FOR signal.rpl_ps_tables_worker_retry_continue";
            DBUG_ASSERT(opt_debug_sync_timeout > 0);
            // we can't add the usual DBUG_ASSERT here because thd->is_error()
            // is true (and that's OK)
            debug_sync_set_action(thd, STRING_WITH_LEN(act));
          });
#endif
      }
    }

    mysql_mutex_lock(&c_rli->data_lock);
    c_rli->retried_trans++;
    mysql_mutex_unlock(&c_rli->data_lock);

    cleanup_context(thd, true);
    reset_commit_order_deadlock();
    worker_sleep(min<ulong>(trans_retries, MAX_SLAVE_RETRY_PAUSE));

  } while (read_and_apply_events(start_relay_number, start_relay_pos,
                                 end_relay_number, end_relay_pos));
  return false;
}

/**
  Read events from relay logs and apply them.

  @param[in] start_relay_number The extension number of the relay log which
               includes the first event of the transaction.
  @param[in] start_relay_pos The offset of the transaction's first event.

  @param[in] end_relay_number The extension number of the relay log which
               includes the last event it should retry.
  @param[in] end_relay_pos The offset of the last event it should retry.

  @return false if succeeds, otherwise returns true.
*/
bool Slave_worker::read_and_apply_events(uint start_relay_number,
                                         my_off_t start_relay_pos,
                                         uint end_relay_number,
                                         my_off_t end_relay_pos) {
  DBUG_TRACE;

  Relay_log_info *rli = c_rli;
  char file_name[FN_REFLEN + 1];
  uint file_number = start_relay_number;
  bool error = true;
  bool arrive_end = false;
  Relaylog_file_reader relaylog_file_reader(opt_slave_sql_verify_checksum);

  relay_log_number_to_name(start_relay_number, file_name);

  while (!arrive_end) {
    Log_event *ev = nullptr;

    if (!relaylog_file_reader.is_open()) {
      DBUG_PRINT("info", ("Open relay log %s", file_name));

      if (relaylog_file_reader.open(file_name, start_relay_pos)) {
        LogErr(ERROR_LEVEL, ER_RPL_FAILED_TO_OPEN_RELAY_LOG, file_name,
               relaylog_file_reader.get_error_str());
        goto end;
      }
    }

    /* If it is the last event, then set arrive_end as true */
    arrive_end = (relaylog_file_reader.position() == end_relay_pos &&
                  file_number == end_relay_number);

    ev = relaylog_file_reader.read_event_object();
    if (ev != nullptr) {
      /* It is a event belongs to the transaction */
      if (!ev->is_mts_sequential_exec()) {
        int ret = 0;

        ev->future_event_relay_log_pos = relaylog_file_reader.position();
        ev->mts_group_idx = gaq_index;

        // event was re-read again, thence context was lost, attach
        // additional context needed, before re-executing (just like in
        // the main loop before exec_relay_log_event)
        rli->current_mts_submode->set_multi_threaded_applier_context(*rli, *ev);

        // we re-assign partitions only on retries
        if (is_mts_db_partitioned(rli) && ev->contains_partition_info(true))
          assign_partition_db(ev);

        ret = slave_worker_exec_event(ev);
        if (ev->worker != nullptr) {
          delete ev;
          ev = nullptr;
        }

        if (ret != 0) goto end;
      } else {
        /*
          It is a Rotate_log_event, Format_description_log_event event or other
          type event doesn't belong to the transaction.
        */
        delete ev;
        ev = nullptr;
      }
    } else {
      /*
        IO error happens if error_type is not READ_EOF, otherwise it arrives the
        end of the relay log
      */
      if (relaylog_file_reader.get_error_type() !=
          Binlog_read_error::READ_EOF) {
        LogErr(ERROR_LEVEL, ER_RPL_WORKER_CANT_READ_RELAY_LOG,
               rli->get_event_relay_log_name(),
               relaylog_file_reader.position());
        goto end;
      }

      if (rli->relay_log.find_next_relay_log(file_name)) {
        LogErr(ERROR_LEVEL, ER_RPL_WORKER_CANT_FIND_NEXT_RELAY_LOG, file_name);
        goto end;
      }

      file_number = relay_log_name_to_number(file_name);

      relaylog_file_reader.close();
      start_relay_pos = BIN_LOG_HEADER_SIZE;
    }
  }

  error = false;
end:
  return error;
}

/*
  Find database entry from map_db_to_worker hash table.
 */
static db_worker_hash_entry *find_entry_from_db_map(const char *dbname,
                                                    Relay_log_info *rli) {
  db_worker_hash_entry *entry = nullptr;

  mysql_mutex_lock(&rli->slave_worker_hash_lock);
  entry = find_or_nullptr(rli->mapping_db_to_worker, dbname);
  mysql_mutex_unlock(&rli->slave_worker_hash_lock);
  return entry;
}

/*
  Initialize Log_event::mts_assigned_partitions array. It is for transaction
  retry and is only called when retrying a transaction by workers.
*/
void Slave_worker::assign_partition_db(Log_event *ev) {
  Mts_db_names mts_dbs;
  int i;

  ev->get_mts_dbs(&mts_dbs, c_rli->rpl_filter);

  if (mts_dbs.num == OVER_MAX_DBS_IN_EVENT_MTS)
    ev->mts_assigned_partitions[0] = find_entry_from_db_map("", c_rli);
  else
    for (i = 0; i < mts_dbs.num; i++)
      ev->mts_assigned_partitions[i] =
          find_entry_from_db_map(mts_dbs.name[i], c_rli);
}

// returns the next available! (TODO: incompatible to circurla_buff method!!!)
static int en_queue(Slave_jobs_queue *jobs, Slave_job_item *item) {
  if (jobs->avail == jobs->size) {
    DBUG_ASSERT(jobs->avail == jobs->m_Q.size());
    return -1;
  }

  // store

  jobs->m_Q[jobs->avail] = *item;

  // pre-boundary cond
  if (jobs->entry == jobs->size) jobs->entry = jobs->avail;

  jobs->avail = (jobs->avail + 1) % jobs->size;
  jobs->len++;

  // post-boundary cond
  if (jobs->avail == jobs->entry) jobs->avail = jobs->size;
  DBUG_ASSERT(jobs->avail == jobs->entry ||
                      jobs->len == (jobs->avail >= jobs->entry)
                  ? (jobs->avail - jobs->entry)
                  : (jobs->size + jobs->avail - jobs->entry));
  return jobs->avail;
}

/**
   return the value of @c data member of the head of the queue.
*/
void *head_queue(Slave_jobs_queue *jobs, Slave_job_item *ret) {
  if (jobs->entry == jobs->size) {
    DBUG_ASSERT(jobs->len == 0);
    ret->data = nullptr;  // todo: move to caller
    return nullptr;
  }
  *ret = jobs->m_Q[jobs->entry];

  DBUG_ASSERT(ret->data);  // todo: move to caller

  return ret;
}

/**
   return a job item through a struct which point is supplied via argument.
*/
Slave_job_item *de_queue(Slave_jobs_queue *jobs, Slave_job_item *ret) {
  if (jobs->entry == jobs->size) {
    DBUG_ASSERT(jobs->len == 0);
    return nullptr;
  }
  *ret = jobs->m_Q[jobs->entry];
  jobs->len--;

  // pre boundary cond
  if (jobs->avail == jobs->size) jobs->avail = jobs->entry;
  jobs->entry = (jobs->entry + 1) % jobs->size;

  // post boundary cond
  if (jobs->avail == jobs->entry) jobs->entry = jobs->size;

  DBUG_ASSERT(jobs->entry == jobs->size ||
              (jobs->len == (jobs->avail >= jobs->entry)
                   ? (jobs->avail - jobs->entry)
                   : (jobs->size + jobs->avail - jobs->entry)));

  return ret;
}

/**
   Coordinator enqueues a job item into a Worker private queue.

   @param job_item  a pointer to struct carrying a reference to an event
   @param worker    a pointer to the assigned Worker struct
   @param rli       a pointer to Relay_log_info of Coordinator

   @return false Success.
           true  Thread killed or worker stopped while waiting for
                 successful enqueue.
*/
bool append_item_to_jobs(slave_job_item *job_item, Slave_worker *worker,
                         Relay_log_info *rli) {
  THD *thd = rli->info_thd;
  int ret = -1;
  size_t ev_size = job_item->data->common_header->data_written;
  ulonglong new_pend_size;
  PSI_stage_info old_stage;

  DBUG_ASSERT(is_mts_parallel_type_dependency(rli) || thd == current_thd);

  mysql_mutex_lock(&rli->pending_jobs_lock);
  new_pend_size = rli->mts_pending_jobs_size + ev_size;
  bool big_event = (ev_size > rli->mts_pending_jobs_size_max);
  /*
    C waits basing on *data* sizes in the queues.
    If it is a big event (event size is greater than
    slave_pending_jobs_size_max but less than slave_max_allowed_packet),
    it will wait for all the jobs in the workers's queue to be
    completed. If it is normal event (event size is less than
    slave_pending_jobs_size_max), then it will wait for
    enough empty memory to keep the event in one of the workers's
    queue.
    NOTE: Receiver thread (I/O thread) is taking care of restricting
    the event size to slave_max_allowed_packet. If an event from
    the master is bigger than this value, IO thread will be stopped
    with error ER_NET_PACKET_TOO_LARGE.
  */
  while ((!big_event && new_pend_size > rli->mts_pending_jobs_size_max) ||
         (big_event && rli->mts_pending_jobs_size != 0)) {
    rli->mts_wq_oversize = true;
    rli->wq_size_waits_cnt++;  // waiting due to the total size
    thd->ENTER_COND(&rli->pending_jobs_cond, &rli->pending_jobs_lock,
                    &stage_slave_waiting_worker_to_free_events, &old_stage);
    mysql_cond_wait(&rli->pending_jobs_cond, &rli->pending_jobs_lock);
    mysql_mutex_unlock(&rli->pending_jobs_lock);
    thd->EXIT_COND(&old_stage);
    if (thd->killed) return true;
    if (rli->wq_size_waits_cnt % 10 == 1)
      LogErr(INFORMATION_LEVEL, ER_RPL_MTS_SLAVE_COORDINATOR_HAS_WAITED,
             rli->wq_size_waits_cnt, ev_size);
    mysql_mutex_lock(&rli->pending_jobs_lock);

    new_pend_size = rli->mts_pending_jobs_size + ev_size;
  }
  rli->pending_jobs++;
  rli->mts_pending_jobs_size = new_pend_size;
  rli->mts_events_assigned++;

  mysql_mutex_unlock(&rli->pending_jobs_lock);

  /*
    Sleep unless there is an underrunning Worker and the current Worker
    queue is empty or filled lightly (not more than underrun level).
  */
  if (rli->mts_wq_underrun_w_id == MTS_WORKER_UNDEF &&
      worker->jobs.len > worker->underrun_level) {
    /*
      todo: experiment with weight to get a good approximation formula.
      Max possible nap time is choosen 1 ms.
      The bigger the excessive overrun counter the longer the nap.
    */
    ulong nap_weight = rli->mts_wq_excess_cnt + 1;
    /*
       Nap time is a product of a weight factor and the basic nap unit.
       The weight factor is proportional to the worker queues overrun excess
       counter. For example when there were only one overruning Worker
       the max nap_weight as 0.1 * worker->jobs.size would be
       about 1600 so the max nap time is approx 0.008 secs.
       Such value is not reachable because of min().
       Notice, granularity of sleep depends on the resolution of the software
       clock, High-Resolution Timer (HRT) configuration. Without HRT
       the precision of wake-up through @c select() may be greater or
       equal 1 ms. So don't expect the nap last a prescribed fraction of 1 ms
       in such case.
       Sleep only if this flow control is enabled through system variable
       rpl_slave_flow_control
    */
    if (rpl_slave_flow_control) {
      my_sleep(min<ulong>(1000, nap_weight * rli->mts_coordinator_basic_nap));
    }
    rli->mts_wq_no_underrun_cnt++;
  }

  mysql_mutex_lock(&worker->jobs_lock);

  // possible WQ overfill
  while (worker->running_status == Slave_worker::RUNNING && !thd->killed &&
         (ret = en_queue(&worker->jobs, job_item)) == -1) {
    thd->ENTER_COND(&worker->jobs_cond, &worker->jobs_lock,
                    &stage_slave_waiting_worker_queue, &old_stage);
    worker->jobs.overfill = true;
    worker->jobs.waited_overfill++;
    rli->mts_wq_overfill_cnt++;
    mysql_cond_wait(&worker->jobs_cond, &worker->jobs_lock);
    mysql_mutex_unlock(&worker->jobs_lock);
    thd->EXIT_COND(&old_stage);

    mysql_mutex_lock(&worker->jobs_lock);
  }
  if (ret != -1) {
    worker->curr_jobs++;
    if (worker->jobs.len == 1) mysql_cond_signal(&worker->jobs_cond);

    mysql_mutex_unlock(&worker->jobs_lock);
  } else {
    mysql_mutex_unlock(&worker->jobs_lock);

    mysql_mutex_lock(&rli->pending_jobs_lock);
    rli->pending_jobs--;  // roll back of the prev incr
    rli->mts_pending_jobs_size -= ev_size;
    mysql_mutex_unlock(&rli->pending_jobs_lock);
  }

  return (-1 != ret ? false : true);
}

/**
  Remove a job item from the given workers job queue. It also updates related
  status.

  param[in] job_item The job item will be removed
  param[in] worker   The worker which job_item belongs to.
  param[in] rli      slave's relay log info object.
 */
static void remove_item_from_jobs(slave_job_item *job_item,
                                  Slave_worker *worker, Relay_log_info *rli) {
  Log_event *ev = job_item->data;

  mysql_mutex_lock(&worker->jobs_lock);
  de_queue(&worker->jobs, job_item);
  /* possible overfill */
  if (worker->jobs.len == worker->jobs.size - 1 &&
      worker->jobs.overfill == true) {
    worker->jobs.overfill = false;
    // todo: worker->hungry_cnt++;
    mysql_cond_signal(&worker->jobs_cond);
  }
  mysql_mutex_unlock(&worker->jobs_lock);

  /* statistics */

  /* todo: convert to rwlock/atomic write */
  mysql_mutex_lock(&rli->pending_jobs_lock);

  rli->pending_jobs--;
  rli->mts_pending_jobs_size -= ev->common_header->data_written;
  DBUG_ASSERT(rli->mts_pending_jobs_size < rli->mts_pending_jobs_size_max);

  /*
    The positive branch is underrun: number of pending assignments
    is less than underrun level.
    Zero of jobs.len has to reset underrun w_id as the worker may get
    the next piece of assignement in a long time.
  */
  if (worker->underrun_level > worker->jobs.len && worker->jobs.len != 0) {
    rli->mts_wq_underrun_w_id = worker->id;
  } else if (rli->mts_wq_underrun_w_id == worker->id) {
    // reset only own marking
    rli->mts_wq_underrun_w_id = MTS_WORKER_UNDEF;
  }

  /*
    Overrun handling.
    Incrementing the Worker private and the total excess counter corresponding
    to number of events filled above the overrun_level.
    The increment amount to the total counter is a difference between
    the current and the previous private excess (worker->wq_overrun_cnt).
    When the current queue length drops below overrun_level the global
    counter is decremented, the local is reset.
  */
  if (worker->overrun_level < worker->jobs.len) {
    ulong last_overrun = worker->wq_overrun_cnt;
    ulong excess_delta;

    /* current overrun */
    worker->wq_overrun_cnt = worker->jobs.len - worker->overrun_level;
    excess_delta = worker->wq_overrun_cnt - last_overrun;
    worker->excess_cnt += excess_delta;
    rli->mts_wq_excess_cnt += excess_delta;
    rli->mts_wq_overrun_cnt++;  // statistics

    // guarding correctness of incrementing in case of the only one Worker
    DBUG_ASSERT(rli->workers.size() != 1 ||
                rli->mts_wq_excess_cnt == worker->wq_overrun_cnt);
  } else if (worker->excess_cnt > 0) {
    // When level drops below the total excess is decremented by the
    // value of the worker's contribution to the total excess.
    rli->mts_wq_excess_cnt -= worker->excess_cnt;
    worker->excess_cnt = 0;
    worker->wq_overrun_cnt = 0;  // and the local is reset

    DBUG_ASSERT(rli->mts_wq_excess_cnt >= 0);
    DBUG_ASSERT(rli->mts_wq_excess_cnt == 0 || rli->workers.size() > 1);
  }

  /* coordinator can be waiting */
  if (rli->mts_pending_jobs_size < rli->mts_pending_jobs_size_max &&
      rli->mts_wq_oversize)  // TODO: unit/general test wq_oversize
  {
    rli->mts_wq_oversize = false;
    mysql_cond_signal(&rli->pending_jobs_cond);
  }

  mysql_mutex_unlock(&rli->pending_jobs_lock);

  worker->events_done++;
}
/**
   Worker's routine to wait for a new assignement through
   @c append_item_to_jobs()

   @param worker    a pointer to the waiting Worker struct
   @param job_item  a pointer to struct carrying a reference to an event

   @return NULL failure or
           a-pointer to an item.
*/
static struct slave_job_item *pop_jobs_item(Slave_worker *worker,
                                            Slave_job_item *job_item) {
  THD *thd = worker->info_thd;

  mysql_mutex_lock(&worker->jobs_lock);

  job_item->data = nullptr;
  while (!job_item->data && !thd->killed &&
         (worker->running_status == Slave_worker::RUNNING ||
          worker->running_status == Slave_worker::STOP)) {
    PSI_stage_info old_stage;

    if (set_max_updated_index_on_stop(worker, job_item)) break;
    if (job_item->data == nullptr) {
      worker->wq_empty_waits++;
      thd->ENTER_COND(&worker->jobs_cond, &worker->jobs_lock,
                      &stage_slave_waiting_event_from_coordinator, &old_stage);
      mysql_cond_wait(&worker->jobs_cond, &worker->jobs_lock);
      mysql_mutex_unlock(&worker->jobs_lock);
      thd->EXIT_COND(&old_stage);
      mysql_mutex_lock(&worker->jobs_lock);
    }
  }
  if (job_item->data) worker->curr_jobs--;

  mysql_mutex_unlock(&worker->jobs_lock);

  thd_proc_info(worker->info_thd, "Executing event");
  return job_item;
}

/**
  Report a not yet reported error to the coordinator if necessary.

  All issues detected when applying binary log events are reported using
  rli->report(), but when an issue is not reported by the log event being
  applied, there is a workaround at handle_slave_sql() to report the issue
  also using rli->report() for the STS applier (or the MTS coordinator).

  This function implements the workaround for a MTS worker.

  @param worker the worker to be evaluated.
*/
void report_error_to_coordinator(Slave_worker *worker) {
  THD *thd = worker->info_thd;
  /*
    It is possible that the worker had failed to apply the event but
    did not reported about the failure using rli->report(). An example
    of such cases are failures caused by setting GTID_NEXT variable with
    an unsupported GTID mode (GTID_SET when GTID_MODE = OFF, anonymous
    GTID when GTID_MODE = ON).
  */
  if (thd->is_error()) {
    char const *const errmsg = thd->get_stmt_da()->message_text();
    DBUG_PRINT("info", ("thd->get_stmt_da()->get_mysql_errno()=%d; "
                        "worker->last_error.number=%d",
                        thd->get_stmt_da()->mysql_errno(),
                        worker->last_error().number));

    if (worker->last_error().number == 0 &&
        /*
          When another worker that should commit before the current worker
          being evaluated has failed and the commit order should be preserved
          the current worker was asked to roll back and would stop with the
          ER_SLAVE_WORKER_STOPPED_PREVIOUS_THD_ERROR not yet reported to the
          coordinator. Reporting this error to the coordinator would be a
          mistake and would mask the real issue that lead to the MTS stop as
          the coordinator reports only the last error reported to it as the
          cause of the MTS failure.

          So, we should skip reporting the error if it was reported because
          the current transaction had to be rolled back by a failure in a
          previous transaction in the commit order while the current
          transaction was waiting to be committed.
        */
        thd->get_stmt_da()->mysql_errno() !=
            ER_SLAVE_WORKER_STOPPED_PREVIOUS_THD_ERROR) {
      /*
        This function is reporting an error which was not reported
        while executing exec_relay_log_event().
      */
      worker->report(ERROR_LEVEL, thd->get_stmt_da()->mysql_errno(), "%s",
                     errmsg);
    }
  }
}

/**
  apply one job group.

  @note the function maintains worker's CGEP and modifies APH, updates
        the current group item in GAQ via @c slave_worker_ends_group().

  param[in] worker the worker which calls it.
  param[in] rli    slave's relay log info object.

  return returns 0 if the group of jobs are applied successfully, otherwise
         returns an error code.
 */
int slave_worker_exec_job_group(Slave_worker *worker, Relay_log_info *rli) {
  struct slave_job_item item = {nullptr, 0, 0};
  struct slave_job_item *job_item = &item;
  THD *thd = worker->info_thd;
  bool seen_gtid = false;
  bool seen_begin = false;
  int error = 0;
  Log_event *ev = nullptr;
  uint start_relay_number;
  my_off_t start_relay_pos;

  DBUG_TRACE;

  if (unlikely(worker->trans_retries > 0)) worker->trans_retries = 0;

  job_item = pop_jobs_item(worker, job_item);
  start_relay_number = job_item->relay_number;
  start_relay_pos = job_item->relay_pos;

  /* Current event with Worker associator. */
  RLI_current_event_raii worker_curr_ev(worker, ev);

  while (true) {
    Slave_job_group *ptr_g;

    if (unlikely(thd->killed ||
                 worker->running_status == Slave_worker::STOP_ACCEPTED)) {
      DBUG_ASSERT(worker->running_status != Slave_worker::ERROR_LEAVING);
      // de-queueing and decrement counters is in the caller's exit branch
      error = -1;
      goto err;
    }

    ev = job_item->data;
    DBUG_ASSERT(ev != nullptr);
    DBUG_PRINT("info", ("W_%lu <- job item: %p data: %p thd: %p", worker->id,
                        job_item, ev, thd));
    /*
      Associate the freshly read event with worker.
      The binding also remains when the loop breaks at the group end event
      so a DDL Query_log_event as such a breaker would remain pinned to
      the Worker by the slave info table update and commit time,
      see slave_worker_ends_group().
    */
    worker_curr_ev.set_current_event(ev);

    if (is_gtid_event(ev)) seen_gtid = true;
    if (!seen_begin && ev->starts_group()) {
      seen_begin = true;  // The current group is started with B-event
      worker->end_group_sets_max_dbs = true;
    }
    set_timespec_nsec(&worker->ts_exec[0], 0);  // pre-exec
    worker->stats_read_time +=
        diff_timespec(&worker->ts_exec[0], &worker->ts_exec[1]);
    /* Adapting to possible new Format_description_log_event */
    ptr_g = rli->gaq->get_job_group(ev->mts_group_idx);
    if (ptr_g->new_fd_event) {
      error = worker->set_rli_description_event(ptr_g->new_fd_event);
      if (unlikely(error)) goto err;
      ptr_g->new_fd_event = nullptr;
    }

    error = worker->slave_worker_exec_event(ev);

    set_timespec_nsec(&worker->ts_exec[1], 0);  // pre-exec
    worker->stats_exec_time +=
        diff_timespec(&worker->ts_exec[1], &worker->ts_exec[0]);
    if (error || worker->found_commit_order_deadlock()) {
      error = worker->retry_transaction(start_relay_number, start_relay_pos,
                                        job_item->relay_number,
                                        job_item->relay_pos);
      if (error) goto err;
    }
    /*
      p-event or any other event of B-free (malformed) group can
      "commit" with logical clock scheduler. In that case worker id
      points to the only active "exclusive" Worker that processes such
      malformed group events one by one.
      WL#7592 refines the original assert disjunction formula
      with the final disjunct.
    */
    DBUG_ASSERT(seen_begin || is_gtid_event(ev) ||
                ev->get_type_code() == binary_log::QUERY_EVENT ||
                is_mts_db_partitioned(rli) || worker->id == 0 || seen_gtid);

    if (ev->ends_group() ||
        (!seen_begin && !is_gtid_event(ev) &&
         (ev->get_type_code() == binary_log::QUERY_EVENT ||
          /* break through by LC only in GTID off */
          (!seen_gtid && is_mts_parallel_type_logical_clock(rli)))))
      break;

    remove_item_from_jobs(job_item, worker, rli);
    /* The event will be used later if worker is NULL, so it is not freed */
    if (ev->worker != nullptr) delete ev;

    job_item = pop_jobs_item(worker, job_item);
  }

  DBUG_PRINT("info", (" commits GAQ index %lu, last committed  %lu",
                      ev->mts_group_idx, worker->last_group_done_index));
  /* The group is applied successfully, so error should be 0 */
  worker->slave_worker_ends_group(ev, 0);

  /*
    Check if the finished group started with a Gtid_log_event to update the
    monitoring information
  */
  if (current_thd->rli_slave->is_processing_trx()) {
    DBUG_EXECUTE_IF("rpl_ps_tables", {
      const char act[] =
          "now SIGNAL signal.rpl_ps_tables_apply_before "
          "WAIT_FOR signal.rpl_ps_tables_apply_finish";
      DBUG_ASSERT(opt_debug_sync_timeout > 0);
      DBUG_ASSERT(!debug_sync_set_action(current_thd, STRING_WITH_LEN(act)));
    };);
    if (ev->get_type_code() == binary_log::QUERY_EVENT &&
        ((Query_log_event *)ev)->rollback_injected_by_coord) {
      /*
        If this was a rollback event injected by the coordinator because of a
        partial transaction in the relay log, we must not consider this
        transaction completed and, instead, clear the monitoring info.
      */
      current_thd->rli_slave->clear_processing_trx();
    } else {
      current_thd->rli_slave->finished_processing();
    }
    DBUG_EXECUTE_IF("rpl_ps_tables", {
      const char act[] =
          "now SIGNAL signal.rpl_ps_tables_apply_after_finish "
          "WAIT_FOR signal.rpl_ps_tables_apply_continue";
      DBUG_ASSERT(opt_debug_sync_timeout > 0);
      DBUG_ASSERT(!debug_sync_set_action(current_thd, STRING_WITH_LEN(act)));
    };);
  }

#ifndef DBUG_OFF
  DBUG_PRINT("mts", ("Check_slave_debug_group worker %lu mts_checkpoint_group"
                     " %u processed %lu debug %d\n",
                     worker->id, opt_mts_checkpoint_group, worker->groups_done,
                     DBUG_EVALUATE_IF("check_slave_debug_group", 1, 0)));

  if (DBUG_EVALUATE_IF("check_slave_debug_group", 1, 0) &&
      opt_mts_checkpoint_group == worker->groups_done) {
    DBUG_PRINT("mts", ("Putting worker %lu in busy wait.", worker->id));
    while (true) my_sleep(6000000);
  }
#endif

  remove_item_from_jobs(job_item, worker, rli);
  delete ev;

  return 0;
err:
  if (error) {
    report_error_to_coordinator(worker);
    DBUG_PRINT("info", ("Worker %lu is exiting: killed %i, error %i, "
                        "running_status %d",
                        worker->id, thd->killed.load(), thd->is_error(),
                        worker->running_status.load()));
    worker->slave_worker_ends_group(ev, error); /* last done sets post exec */
  }
  return error;
}

const char *Slave_worker::get_for_channel_str(bool upper_case) const {
  return c_rli->get_for_channel_str(upper_case);
}

const uint *Slave_worker::get_table_pk_field_indexes() {
  return info_slave_worker_table_pk_field_indexes;
}

uint Slave_worker::get_channel_field_index() { return LINE_FOR_CHANNEL; }

/**
  apply a single job.

  @note the function maintains worker's CGEP and modifies APH, updates
        the current group item in GAQ via @c slave_worker_ends_group().

  param[in] worker the worker which calls it.
  param[in] rli    slave's relay log info object.

  return returns 0 if a job is applied successfully, otherwise
         returns an error code.
 */
int slave_worker_exec_single_job(Slave_worker *worker, Relay_log_info *rli,
                                 std::shared_ptr<Log_event_wrapper> &ev_wrap,
                                 uint start_relay_number,
                                 my_off_t start_relay_pos) {
  THD *thd = worker->info_thd;
  int error = 0;
  Log_event *ev = nullptr;

  DBUG_ENTER("slave_worker_exec_single_job");

  /* Current event with Worker associator. */
  RLI_current_event_raii worker_curr_ev(worker, ev);

  Slave_job_group *ptr_g;

  if (unlikely(thd->killed ||
               worker->running_status == Slave_worker::STOP_ACCEPTED)) {
    DBUG_ASSERT(worker->running_status != Slave_worker::ERROR_LEAVING);
    // de-queueing and decrement counters is in the caller's exit branch
    error = -1;
    goto err;
  }

  ev = ev_wrap->raw_event();
  DBUG_ASSERT(ev != nullptr);
  DBUG_PRINT("info",
             ("W_%lu <- job item: data: %p thd: %p", worker->id, ev, thd));
  /*
    Associate the freshly read event with worker.
    The binding also remains when the loop breaks at the group end event
    so a DDL Query_log_event as such a breaker would remain pinned to
    the Worker by the slave info table update and commit time,
    see slave_worker_ends_group().
  */
  worker_curr_ev.set_current_event(ev);

  if (ev->starts_group()) {
    if (unlikely(worker->trans_retries > 0)) worker->trans_retries = 0;
    worker->end_group_sets_max_dbs = true;
  }
  set_timespec_nsec(&worker->ts_exec[0], 0);  // pre-exec
  worker->stats_read_time +=
      diff_timespec(&worker->ts_exec[0], &worker->ts_exec[1]);
  /* Adapting to possible new Format_description_log_event */
  ptr_g = rli->gaq->get_job_group(ev->mts_group_idx);
  if (ptr_g->new_fd_event) {
    worker->set_rli_description_event(ptr_g->new_fd_event);
    ptr_g->new_fd_event = nullptr;
  }

  error = worker->slave_worker_exec_event(ev);
  ev_wrap->set_slave_worker(worker);

  DBUG_EXECUTE_IF("dbug.dep_fake_gap_lock_on_insert", {
    auto scheduler =
        static_cast<Mts_submode_dependency *>(rli->current_mts_submode);
    if (ev->get_type_code() == binary_log::WRITE_ROWS_EVENT) {
      if (!scheduler->dep_fake_gap_lock.try_lock()) {
        thd_report_row_lock_wait(thd,
                                 scheduler->dep_fake_gap_lock_worker->info_thd);
        scheduler->dep_fake_gap_lock.lock();
        scheduler->dep_fake_gap_lock_worker = worker;
      } else {
        scheduler->dep_fake_gap_lock_worker = worker;
      }
    } else if (ev->ends_group() &&
               worker == scheduler->dep_fake_gap_lock_worker) {
      scheduler->dep_fake_gap_lock_worker = nullptr;
      scheduler->dep_fake_gap_lock.unlock();
    }
  };);

  set_timespec_nsec(&worker->ts_exec[1], 0);  // pre-exec
  worker->stats_exec_time +=
      diff_timespec(&worker->ts_exec[1], &worker->ts_exec[0]);
  if (error || worker->found_commit_order_deadlock()) {
    DBUG_EXECUTE_IF("dbug.dep_fake_gap_lock_on_insert", {
      auto scheduler =
          static_cast<Mts_submode_dependency *>(rli->current_mts_submode);
      if (worker == scheduler->dep_fake_gap_lock_worker) {
        scheduler->dep_fake_gap_lock_worker = nullptr;
        scheduler->dep_fake_gap_lock.unlock();
      }
    };);
    error = worker->retry_transaction(start_relay_number, start_relay_pos,
                                      ev_wrap->get_event_relay_log_number(),
                                      ev_wrap->get_event_start_pos());
    if (error) goto err;
  }

  if (!ev_wrap->is_end_event) {
    /* The event will be used later if worker is NULL, so it is not freed */
    if (ev->worker != nullptr) {
      /* cache TABLE_MAP_EVENT events for usage by
       * Relay_log_info::table_map_events */
      if (ev->get_type_code() != binary_log::TABLE_MAP_EVENT) {
        ev_wrap->is_appended_to_queue = true;
        delete ev;
      }
    } else {
      ev_wrap->is_appended_to_queue = true;
    }

    DBUG_RETURN(0);
  }

  DBUG_PRINT("info", (" commits GAQ index %lu, last committed  %lu",
                      ev->mts_group_idx, worker->last_group_done_index));
  /* The group is applied successfully, so error should be 0 */
  worker->slave_worker_ends_group(ev, 0);

  /*
    Check if the finished group started with a Gtid_log_event to update the
    monitoring information
  */
  if (current_thd->rli_slave->is_processing_trx()) {
    DBUG_EXECUTE_IF("rpl_ps_tables", {
      const char act[] =
          "now SIGNAL signal.rpl_ps_tables_apply_before "
          "WAIT_FOR signal.rpl_ps_tables_apply_finish";
      DBUG_ASSERT(opt_debug_sync_timeout > 0);
      DBUG_ASSERT(!debug_sync_set_action(current_thd, STRING_WITH_LEN(act)));
    };);
    if (ev->get_type_code() == binary_log::QUERY_EVENT &&
        ((Query_log_event *)ev)->rollback_injected_by_coord) {
      /*
        If this was a rollback event injected by the coordinator because of a
        partial transaction in the relay log, we must not consider this
        transaction completed and, instead, clear the monitoring info.
      */
      current_thd->rli_slave->clear_processing_trx();
    } else {
      current_thd->rli_slave->finished_processing();
    }
    DBUG_EXECUTE_IF("rpl_ps_tables", {
      const char act[] =
          "now SIGNAL signal.rpl_ps_tables_apply_after_finish "
          "WAIT_FOR signal.rpl_ps_tables_apply_continue";
      DBUG_ASSERT(opt_debug_sync_timeout > 0);
      DBUG_ASSERT(!debug_sync_set_action(current_thd, STRING_WITH_LEN(act)));
    };);
  }

  ev_wrap->is_appended_to_queue = true;
  delete ev;

  DBUG_RETURN(0);
err:
  if (error) {
    report_error_to_coordinator(worker);
    DBUG_PRINT("info", ("Worker %lu is exiting: killed %i, error %i, "
                        "running_status %d",
                        worker->id, thd->killed.load(), thd->is_error(),
                        worker->running_status.load()));
    worker->slave_worker_ends_group(ev, error); /* last done sets post exec */
  }
  DBUG_RETURN(error);
}
