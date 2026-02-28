/* Copyright (c) 2013, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/rpl_mts_submode.h"

#include <limits.h>
#include <string.h>
#include <time.h>
#include <memory>

#include "lex_string.h"
#include "libbinlogevents/include/compression/iterator.h"
#include "m_string.h"
#include "my_byteorder.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_loglevel.h"
#include "my_systime.h"
#include "my_thread.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/psi_stage_bits.h"
#include "mysql/plugin.h"  // thd_proc_info
#include "mysql/psi/mysql_cond.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysqld_error.h"
#include "sql/binlog_reader.h"
#include "sql/debug_sync.h"
#include "sql/log.h"
#include "sql/log_event.h"  // Query_log_event
#include "sql/mdl.h"
#include "sql/mysqld.h"  // stage_worker_....
#include "sql/query_options.h"
#include "sql/rpl_filter.h"
#include "sql/rpl_rli.h"      // Relay_log_info
#include "sql/rpl_rli_pdb.h"  // db_worker_hash_entry
#include "sql/rpl_slave.h"
#include "sql/rpl_slave_commit_order_manager.h"  // Commit_order_manager
#include "sql/sql_class.h"                       // THD
#include "sql/sql_plugin.h"
#include "sql/system_variables.h"
#include "sql/table.h"

/**
 Does necessary arrangement before scheduling next event.
 @return 1  if  error
          0 no error
*/
int Mts_submode_database::schedule_next_event(Relay_log_info *, Log_event *) {
  /*nothing to do here*/
  return 0;
}

/**
  Logic to attach temporary tables.
*/
void Mts_submode_database::attach_temp_tables(THD *thd, const Relay_log_info *,
                                              Query_log_event *ev) {
  int i, parts;
  DBUG_TRACE;
  if (!is_mts_worker(thd) || (ev->ends_group() || ev->starts_group())) return;
  DBUG_ASSERT(!thd->temporary_tables);
  // in over max-db:s case just one special partition is locked
  parts = ((ev->mts_accessed_dbs == OVER_MAX_DBS_IN_EVENT_MTS)
               ? 1
               : ev->mts_accessed_dbs);
  for (i = 0; i < parts; i++) {
    mts_move_temp_tables_to_thd(
        thd, ev->mts_assigned_partitions[i]->temporary_tables);
    ev->mts_assigned_partitions[i]->temporary_tables = nullptr;
  }
}

/**
   Function is called by Coordinator when it identified an event
   requiring sequential execution.
   Creating sequential context for the event includes waiting
   for the assigned to Workers tasks to be completed and their
   resources such as temporary tables be returned to Coordinator's
   repository.
   In case all workers are waited Coordinator changes its group status.

   @param  rli     Relay_log_info instance of Coordinator
   @param  ignore  Optional Worker instance pointer if the sequential context
                   is established due for the ignore Worker. Its resources
                   are to be retained.

   @note   Resources that are not occupied by Workers such as
           a list of temporary tables held in unused (zero-usage) records
           of APH are relocated to the Coordinator placeholder.

   @return non-negative number of released by Workers partitions
           (one partition by one Worker can count multiple times)

           or -1 to indicate there has been a failure on a not-ignored Worker
           as indicated by its running_status so synchronization can't succeed.
*/

int Mts_submode_database::wait_for_workers_to_finish(Relay_log_info *rli,
                                                     Slave_worker *ignore) {
  uint ret = 0;
  THD *thd = rli->info_thd;
  bool cant_sync = false;
  char llbuf[22];

  DBUG_TRACE;

  llstr(rli->get_event_relay_log_pos(), llbuf);
  DBUG_PRINT("info", ("Coordinator and workers enter synchronization "
                      "procedure when scheduling event relay-log: %s "
                      "pos: %s",
                      rli->get_event_relay_log_name(), llbuf));

  mysql_mutex_lock(&rli->slave_worker_hash_lock);

  for (const auto &key_and_value : rli->mapping_db_to_worker) {
    db_worker_hash_entry *entry = key_and_value.second.get();
    DBUG_ASSERT(entry);

    // the ignore Worker retains its active resources
    if (ignore && entry->worker == ignore && entry->usage > 0) {
      continue;
    }

    if (entry->usage > 0 && !thd->killed) {
      PSI_stage_info old_stage;
      Slave_worker *w_entry = entry->worker;

      entry->worker = nullptr;  // mark Worker to signal when  usage drops to 0
      thd->ENTER_COND(
          &rli->slave_worker_hash_cond, &rli->slave_worker_hash_lock,
          &stage_slave_waiting_worker_to_release_partition, &old_stage);
      do {
        mysql_cond_wait(&rli->slave_worker_hash_cond,
                        &rli->slave_worker_hash_lock);
        DBUG_PRINT("info", ("Either got awakened of notified: "
                            "entry %p, usage %lu, worker %lu",
                            entry, entry->usage, w_entry->id));
      } while (entry->usage != 0 && !thd->killed);
      entry->worker =
          w_entry;  // restoring last association, needed only for assert
      mysql_mutex_unlock(&rli->slave_worker_hash_lock);
      thd->EXIT_COND(&old_stage);
      ret++;
    } else {
      mysql_mutex_unlock(&rli->slave_worker_hash_lock);
    }
    // resources relocation
    mts_move_temp_tables_to_thd(thd, entry->temporary_tables);
    entry->temporary_tables = nullptr;
    if (entry->worker->running_status != Slave_worker::RUNNING)
      cant_sync = true;
    mysql_mutex_lock(&rli->slave_worker_hash_lock);
  }

  mysql_mutex_unlock(&rli->slave_worker_hash_lock);

  if (!ignore) {
    DBUG_PRINT("info", ("Coordinator synchronized with workers, "
                        "waited entries: %d, cant_sync: %d",
                        ret, cant_sync));

    rli->mts_group_status = Relay_log_info::MTS_NOT_IN_GROUP;
  }

  return !cant_sync ? ret : -1;
}

bool Mts_submode_database::unfold_transaction_payload_event(
    Format_description_event &fde, Transaction_payload_log_event &tple,
    std::vector<Log_event *> &events) {
  bool error = false;
  /*
    disable checksums - there are no checksums for events inside the tple
    otherwise, the last 4 bytes would be truncated.

    We do this by copying the fdle from the rli. Then we disable the checksum
    in the copy. Then we use it to decode the events in the payload instead
    of the original fdle.

    We allocate the fdle copy in the stack.

    TODO: simplify this by breaking the binlog_event_deserialize API
    and make it take a single boolean instead that states whether the
    event has a checksum in it or not.
  */
  Format_description_log_event fdle(fde.reader().buffer(), &fde);
  fdle.footer()->checksum_alg = binary_log::BINLOG_CHECKSUM_ALG_OFF;
  fdle.register_temp_buf(const_cast<char *>(fde.reader().buffer()), false);

  // unpack the event
  binary_log::transaction::compression::Iterable_buffer it(
      tple.get_payload(), tple.get_payload_size(), tple.get_uncompressed_size(),
      tple.get_compression_type());

  for (auto ptr : it) {
    Log_event *next = nullptr;
    size_t event_len = uint4korr(ptr + EVENT_LEN_OFFSET);
    if (binlog_event_deserialize(reinterpret_cast<const unsigned char *>(ptr),
                                 event_len, &fdle, true, &next)) {
      error = true; /* purecov: inspected */
      break;        /* purecov: inspected */
    } else {
      DBUG_ASSERT(next != nullptr);
      events.push_back(next);
    }
  }

  return error;
}

bool Mts_submode_database::set_multi_threaded_applier_context(
    const Relay_log_info &rli, Log_event &ev) {
  bool error = false;

  // if this is a transaction payload event, we need to set the proper
  // databases that its internal events update
  if (ev.get_type_code() == binary_log::TRANSACTION_PAYLOAD_EVENT) {
    Mts_db_names toset;
    bool max_mts_dbs_in_event = false;
    std::set<std::string> dbs;
    auto &tple = static_cast<Transaction_payload_log_event &>(ev);
    std::vector<Log_event *> events;
    unfold_transaction_payload_event(*rli.get_rli_description_event(), tple,
                                     events);

    for (auto inner : events) {
      Mts_db_names mts_dbs;

      // This transaction payload event is already marked to run in
      // isolation or the event being handled does not contain partition
      // information
      if (max_mts_dbs_in_event || !inner->contains_partition_info(true)) {
        delete inner;
        continue;
      }

      // The following queries should run in isolation, thence setting
      // OVER_MAX_DBS_IN_EVENT_MTS
      if ((inner->get_type_code() == binary_log::QUERY_EVENT)) {
        auto qev = static_cast<Query_log_event *>(inner);
        if (qev->is_query_prefix_match(STRING_WITH_LEN("XA COMMIT")) ||
            qev->is_query_prefix_match(STRING_WITH_LEN("XA ROLLBACK"))) {
          max_mts_dbs_in_event = true;
          delete inner;
          continue;
        }
      }

      // OK, now that we have ruled the exceptions, lets handle the databases
      // in the inner event.
      inner->get_mts_dbs(&mts_dbs, rli.rpl_filter);

      // inner event has mark to run in isolation
      if (mts_dbs.num == OVER_MAX_DBS_IN_EVENT_MTS) {
        max_mts_dbs_in_event = true;
        delete inner;
        continue;
      }

      // iterate over the databases and add them to the set
      for (int i = 0; i < mts_dbs.num; i++) {
        dbs.insert(mts_dbs.name[i]);
        if (dbs.size() == MAX_DBS_IN_EVENT_MTS) {
          max_mts_dbs_in_event = true;
          break;
        }
      }

      // inner event not needed anymore. Delete.
      delete inner;
    }

    // now set the database information in the event
    if (max_mts_dbs_in_event) {
      toset.name[0] = "\0";
      toset.num = OVER_MAX_DBS_IN_EVENT_MTS;
    } else {
      int i = 0;
      // set the databases
      for (auto &db : dbs) toset.name[i++] = db.c_str();

      // set the number of databases
      toset.num = dbs.size();
    }

    // save the mts_dbs to the payload event
    tple.set_mts_dbs(toset);
  }

  return error;
}

/**
 Logic to detach the temporary tables from the worker threads upon
 event execution.
 @param thd THD instance
 @param rli Relay_log_info pointer
 @param ev  Query_log_event that is being applied
*/
void Mts_submode_database::detach_temp_tables(THD *thd,
                                              const Relay_log_info *rli,
                                              Query_log_event *ev) {
  DBUG_TRACE;
  if (!is_mts_worker(thd)) return;
  int parts = ((ev->mts_accessed_dbs == OVER_MAX_DBS_IN_EVENT_MTS)
                   ? 1
                   : ev->mts_accessed_dbs);
  /*
    todo: optimize for a case of

    a. one db
       Only detaching temporary_tables from thd to entry would require
       instead of the double-loop below.

    b. unchanged thd->temporary_tables.
       In such case the involved entries would continue to hold the
       unmodified lists provided that the attach_ method does not
       destroy references to them.
  */
  for (int i = 0; i < parts; i++) {
    ev->mts_assigned_partitions[i]->temporary_tables = nullptr;
  }

  Rpl_filter *rpl_filter = rli->rpl_filter;
  for (TABLE *table = thd->temporary_tables; table;) {
    int i;
    const char *db_name = nullptr;

    // find which entry to go
    for (i = 0; i < parts; i++) {
      db_name = ev->mts_accessed_db_names[i];
      if (!strlen(db_name)) break;
      // Only default database is rewritten.
      if (!rpl_filter->is_rewrite_empty() && !strcmp(ev->get_db(), db_name)) {
        size_t dummy_len;
        const char *db_filtered =
            rpl_filter->get_rewrite_db(db_name, &dummy_len);
        // db_name != db_filtered means that db_name is rewritten.
        if (strcmp(db_name, db_filtered)) db_name = db_filtered;
      }
      if (strcmp(table->s->db.str, db_name) < 0)
        continue;
      else {
        // When rewrite db rules are used we can not rely on
        // mts_accessed_db_names elements order.
        if (!rpl_filter->is_rewrite_empty() &&
            strcmp(table->s->db.str, db_name))
          continue;
        else
          break;
      }
    }
    DBUG_ASSERT(db_name &&
                (!strcmp(table->s->db.str, db_name) || !strlen(db_name)));
    DBUG_ASSERT(i < ev->mts_accessed_dbs);
    // table pointer is shifted inside the function
    table = mts_move_temp_table_to_entry(table, thd,
                                         ev->mts_assigned_partitions[i]);
  }

  DBUG_ASSERT(!thd->temporary_tables);
#ifndef DBUG_OFF
  for (int i = 0; i < parts; i++) {
    DBUG_ASSERT(!ev->mts_assigned_partitions[i]->temporary_tables ||
                !ev->mts_assigned_partitions[i]->temporary_tables->prev);
  }
#endif
}

/**
  Logic to get least occupied worker when the sql mts_submode= database
  @param ws  array of worker threads
  @return slave worker thread
 */
Slave_worker *Mts_submode_database::get_least_occupied_worker(
    Relay_log_info *, Slave_worker_array *ws, Log_event *) {
  long usage = LONG_MAX;
  Slave_worker **ptr_current_worker = nullptr, *worker = nullptr;

  DBUG_TRACE;

#ifndef DBUG_OFF

  if (DBUG_EVALUATE_IF("mts_distribute_round_robin", 1, 0)) {
    worker = ws->at(w_rr % ws->size());
    LogErr(INFORMATION_LEVEL, ER_RPL_WORKER_ID_IS, worker->id,
           static_cast<ulong>(w_rr % ws->size()));
    DBUG_ASSERT(worker != nullptr);
    return worker;
  }
#endif

  for (Slave_worker **it = ws->begin(); it != ws->end(); ++it) {
    ptr_current_worker = it;
    if ((*ptr_current_worker)->usage_partition <= usage) {
      worker = *ptr_current_worker;
      usage = (*ptr_current_worker)->usage_partition;
    }
  }
  DBUG_ASSERT(worker != nullptr);
  return worker;
}

/* MTS submode master Default constructor */
Mts_submode_logical_clock::Mts_submode_logical_clock() {
  type = MTS_PARALLEL_TYPE_LOGICAL_CLOCK;
  first_event = true;
  force_new_group = false;
  is_new_group = true;
  delegated_jobs = 0;
  jobs_done = 0;
  last_lwm_timestamp = SEQ_UNINIT;
  last_lwm_index = INDEX_UNDEF;
  is_error = false;
  min_waited_timestamp = SEQ_UNINIT;
  last_committed = SEQ_UNINIT;
  sequence_number = SEQ_UNINIT;
}

/**
   The method finds the minimum logical timestamp (low-water-mark) of
   committed transactions.
   The successful search results in a pair of a logical timestamp value and a
   GAQ index that contains it. last_lwm_timestamp may still be raised though the
   search does not find any satisfying running index. Search is implemented as
   headway scanning of GAQ from a point of a previous search's stop position
   (last_lwm_index). Whether the cached (memorized) index value is considered to
   be stale when its timestamp gets less than the current "stable" LWM:

        last_lwm_timestamp <= GAQ.lwm.sequence_number           (*)

   Staleness is caused by GAQ garbage collection that increments the rhs of (*),
   see @c move_queue_head(). When that's diagnosed, the search in GAQ needs
   restarting from the queue tail.

   Formally, the undefined cached value of last_lwm_timestamp is also stale.

   @verbatim
              the last time index containg lwm
                  +------+
                  | LWM  |
                  |  |   |
                  V  V   V
   GAQ:   xoooooxxxxxXXXXX...X
                ^   ^
                |   | LWM+1
                |
                +- tne new current_lwm

         <---- logical (commit) time ----
   @endverbatim

   here `x' stands for committed, `X' for committed and discarded from
   the running range of the queue, `o' for not committed.

   @param  rli         Relay_log_info pointer
   @param  need_lock   Either the caller or the function must hold a mutex
                       to avoid race with concurrent GAQ update.

   @return possibly updated current_lwm
*/
longlong Mts_submode_logical_clock::get_lwm_timestamp(Relay_log_info *rli,
                                                      bool need_lock) {
  longlong lwm_estim;
  Slave_job_group *ptr_g = nullptr;
  bool is_stale = false;

  if (!need_lock) mysql_mutex_lock(&rli->mts_gaq_LOCK);

  /*
    Make the "stable" LWM-based estimate which will be compared
    against the cached "instant" value.
  */
  lwm_estim = rli->gaq->lwm.sequence_number;
  /*
    timestamp continuity invariant: if the queue has any item
    its timestamp is greater on one than the estimate.
  */
  DBUG_ASSERT(lwm_estim == SEQ_UNINIT || rli->gaq->empty() ||
              lwm_estim + 1 ==
                  rli->gaq->get_job_group(rli->gaq->entry)->sequence_number);

  last_lwm_index = rli->gaq->find_lwm(
      &ptr_g,
      /*
        The underfined "stable" forces the scan's restart
        as the stale value does.
      */
      lwm_estim == SEQ_UNINIT ||
              (is_stale = clock_leq(last_lwm_timestamp, lwm_estim))
          ? rli->gaq->entry
          : last_lwm_index);
  /*
    if the returned index is sane update the timestamp.
  */
  if (last_lwm_index != rli->gaq->size) {
    // non-decreasing lwm invariant
    DBUG_ASSERT(clock_leq(last_lwm_timestamp, ptr_g->sequence_number));

    last_lwm_timestamp = ptr_g->sequence_number;
  } else if (is_stale) {
    last_lwm_timestamp.store(lwm_estim);
  }

  if (!need_lock) mysql_mutex_unlock(&rli->mts_gaq_LOCK);

  return last_lwm_timestamp;
}

/**
   The method implements logical timestamp conflict detection
   and resolution through waiting by the calling thread.
   The conflict or waiting condition is like the following

           lwm < last_committed,

   where lwm is a minimum logical timestamp of committed transactions.
   Since the lwm's exact value is not always available its pessimistic
   estimate (an old version) is improved (get_lwm_timestamp()) as the
   first step before to the actual waiting commitment.

   Special cases include:

   When @c last_committed_arg is uninitialized the calling thread must
   proceed without waiting for anyone. Any possible dependency with unknown
   commit parent transaction shall be sorted out by the parent;

   When the gaq index is subsequent to the last lwm index
   there's no dependency of the current transaction with any regardless of
   lwm timestamp should it be SEQ_UNINIT.
   Consequently when GAQ consists of just one item there's none to wait.
   Such latter case is left to the caller to handle.

   @note The caller must make sure the current transaction won't be waiting
         for itself. That is the method should not be caller by a Worker
         whose group assignment is in the GAQ front item.

   @param rli relay log info of coordinator
   @param last_committed_arg  logical timestamp of a parent transaction
   @return false as success,
           true  when the error flag is raised or
                 the caller thread is found killed.
*/
bool Mts_submode_logical_clock::wait_for_last_committed_trx(
    Relay_log_info *rli, longlong last_committed_arg) {
  THD *thd = rli->info_thd;

  DBUG_TRACE;

  if (last_committed_arg == SEQ_UNINIT) return false;

  mysql_mutex_lock(&rli->mts_gaq_LOCK);

  DBUG_ASSERT(min_waited_timestamp == SEQ_UNINIT);

  min_waited_timestamp.store(last_committed_arg);
  /*
    This transaction is a candidate for insertion into the waiting list.
    That fact is descibed by incrementing waited_timestamp_cnt.
    When the candidate won't make it the counter is decremented at once
    while the mutex is hold.
  */
  if ((!rli->info_thd->killed && !is_error) &&
      !clock_leq(last_committed_arg, get_lwm_timestamp(rli, true))) {
    PSI_stage_info old_stage;
    struct timespec ts[2];
    set_timespec_nsec(&ts[0], 0);

    DBUG_ASSERT(rli->gaq->len >= 2);  // there's someone to wait

    thd->ENTER_COND(&rli->logical_clock_cond, &rli->mts_gaq_LOCK,
                    &stage_worker_waiting_for_commit_parent, &old_stage);
    do {
      mysql_cond_wait(&rli->logical_clock_cond, &rli->mts_gaq_LOCK);
    } while ((!rli->info_thd->killed && !is_error) &&
             !clock_leq(last_committed_arg, estimate_lwm_timestamp()));
    min_waited_timestamp.store(SEQ_UNINIT);  // reset waiting flag
    mysql_mutex_unlock(&rli->mts_gaq_LOCK);
    thd->EXIT_COND(&old_stage);
    set_timespec_nsec(&ts[1], 0);
    rli->mts_total_wait_overlap += diff_timespec(&ts[1], &ts[0]);
  } else {
    min_waited_timestamp.store(SEQ_UNINIT);
    mysql_mutex_unlock(&rli->mts_gaq_LOCK);
  }

  return rli->info_thd->killed || is_error;
}

/**
 Does necessary arrangement before scheduling next event.
 The method computes the meta-group status of the being scheduled
 transaction represented by the event argument. When the status
 is found OUT (of the current meta-group) as encoded as is_new_group == true
 the global Scheduler (Coordinator thread) requests full synchronization
 with all Workers.
 The current being assigned group descriptor gets associated with
 the group's logical timestamp aka sequence_number.

 @return ER_MTS_CANT_PARALLEL, ER_MTS_INCONSISTENT_DATA
          0 if no error or slave has been killed gracefully
 */
int Mts_submode_logical_clock::schedule_next_event(Relay_log_info *rli,
                                                   Log_event *ev) {
  longlong last_sequence_number = sequence_number;
  bool gap_successor = false;

  DBUG_TRACE;
  // We should check if the SQL thread was already killed before we schedule
  // the next transaction
  if (sql_slave_killed(rli->info_thd, rli)) return 0;

  Slave_job_group *ptr_group =
      rli->gaq->get_job_group(rli->gaq->assigned_group_index);
  /*
    A group id updater must satisfy the following:
    - A query log event ("BEGIN" ) or a GTID EVENT
    - A DDL or an implicit DML commit.
  */
  switch (ev->get_type_code()) {
    case binary_log::GTID_LOG_EVENT:
    case binary_log::ANONYMOUS_GTID_LOG_EVENT:
      // TODO: control continuity
      ptr_group->sequence_number = sequence_number =
          static_cast<Gtid_log_event *>(ev)->sequence_number;
      ptr_group->last_committed = last_committed =
          static_cast<Gtid_log_event *>(ev)->last_committed;
      break;

    default:

      sequence_number = last_committed = SEQ_UNINIT;

      break;
  }

  DBUG_PRINT("info", ("sequence_number %lld, last_committed %lld",
                      sequence_number, last_committed));

  if (first_event) {
    first_event = false;
  } else {
    if (unlikely(clock_leq(sequence_number, last_committed) &&
                 last_committed != SEQ_UNINIT)) {
      /* inconsistent (buggy) timestamps */
      LogErr(ERROR_LEVEL, ER_RPL_INCONSISTENT_TIMESTAMPS_IN_TRX,
             sequence_number, last_committed);
      return ER_MTS_CANT_PARALLEL;
    }
    if (unlikely(clock_leq(sequence_number, last_sequence_number) &&
                 sequence_number != SEQ_UNINIT)) {
      /* inconsistent (buggy) timestamps */
      LogErr(ERROR_LEVEL, ER_RPL_INCONSISTENT_SEQUENCE_NO_IN_TRX,
             sequence_number, last_sequence_number);
      return ER_MTS_CANT_PARALLEL;
    }
    /*
      Being scheduled transaction sequence may have gaps, even in
      relay log. In such case a transaction that succeeds a gap will
      wait for all ealier that were scheduled to finish. It's marked
      as gap successor now.
    */
    static_assert(SEQ_UNINIT == 0, "");
    if (unlikely(sequence_number > last_sequence_number + 1)) {
      /*
        TODO: account autopositioning
        DBUG_ASSERT(rli->replicate_same_server_id);
      */
      DBUG_PRINT("info", ("sequence_number gap found, "
                          "last_sequence_number %lld, sequence_number %lld",
                          last_sequence_number, sequence_number));
      gap_successor = true;
    }
  }

  /*
    The new group flag is practically the same as the force flag
    when up to indicate syncronization with Workers.
  */
  is_new_group =
      (/* First event after a submode switch; */
       first_event ||
       /* Require a fresh group to be started; */
       // todo: turn `force_new_group' into sequence_number == SEQ_UNINIT
       // condition
       force_new_group ||
       /* Rewritten event without commit point timestamp (todo: find use case)
        */
       sequence_number == SEQ_UNINIT ||
       /*
         undefined parent (e.g the very first trans from the master),
         or old master.
       */
       last_committed == SEQ_UNINIT ||
       /*
         When gap successor depends on a gap before it the scheduler has
         to serialize this transaction execution with previously
         scheduled ones. Below for simplicity it's assumed that such
         gap-dependency is always the case.
       */
       gap_successor ||
       /*
         previous group did not have sequence number assigned.
         It's execution must be finished until the current group
         can be assigned.
         Dependency of the current group on the previous
         can't be tracked. So let's wait till the former is over.
       */
       last_sequence_number == SEQ_UNINIT);
  /*
    The coordinator waits till all transactions on which the current one
    depends on are applied.
  */
  if (!is_new_group) {
    longlong lwm_estimate = estimate_lwm_timestamp();

    if (!clock_leq(last_committed, lwm_estimate) &&
        rli->gaq->assigned_group_index != rli->gaq->entry) {
      /*
        "Unlikely" branch.

        The following block improves possibly stale lwm and when the
        waiting condition stays, recompute min_waited_timestamp and go
        waiting.
        At awakening set min_waited_timestamp to commit_parent in the
        subsequent GAQ index (could be NIL).
      */
      if (wait_for_last_committed_trx(rli, last_committed)) {
        /*
          MTS was waiting for a dependent transaction to finish but either it
          has failed or the applier was requested to stop. In any case, this
          transaction wasn't started yet and should not warn about the
          coordinator stopping in a middle of a transaction to avoid polluting
          the server error log.
        */
        rli->reported_unsafe_warning = true;
        return -1;
      }
      /*
        Making the slave's max last committed (lwm) to satisfy this
        transaction's scheduling condition.
      */
      if (gap_successor) last_lwm_timestamp = sequence_number - 1;
      DBUG_ASSERT(!clock_leq(sequence_number, estimate_lwm_timestamp()));
    }

    delegated_jobs++;

    DBUG_ASSERT(!force_new_group);
  } else {
    DBUG_ASSERT(delegated_jobs >= jobs_done);
    DBUG_ASSERT(is_error || (rli->gaq->len + jobs_done == 1 + delegated_jobs));
    DBUG_ASSERT(rli->mts_group_status == Relay_log_info::MTS_IN_GROUP);

    /*
      Under the new group fall the following use cases:
      - events from an OLD (sequence_number unaware) master;
      - malformed (missed BEGIN or GTID_NEXT) group incl. its
        particular form of CREATE..SELECT..from..@user_var (or rand- and
        int- var in place of @user- var).
        The malformed group is handled exceptionally each event is executed
        as a solitary group yet by the same (zero id) worker.
    */
    if (-1 == wait_for_workers_to_finish(rli)) return ER_MTS_INCONSISTENT_DATA;

    rli->mts_group_status = Relay_log_info::MTS_IN_GROUP;  // wait set it to NOT
    DBUG_ASSERT(min_waited_timestamp == SEQ_UNINIT);
    /*
      the instant last lwm timestamp must reset when force flag is up.
    */
    rli->gaq->lwm.sequence_number = last_lwm_timestamp = SEQ_UNINIT;
    delegated_jobs = 1;
    jobs_done = 0;
    force_new_group = false;
    /*
      Not sequenced event can be followed with a logically relating
      e.g User var to be followed by CREATE table.
      It's supported to be executed in one-by-one fashion.
      Todo: remove with the event group parser worklog.
    */
    if (sequence_number == SEQ_UNINIT && last_committed == SEQ_UNINIT)
      rli->last_assigned_worker = *rli->workers.begin();
  }

#ifndef DBUG_OFF
  mysql_mutex_lock(&rli->mts_gaq_LOCK);
  DBUG_ASSERT(is_error || (rli->gaq->len + jobs_done == delegated_jobs));
  mysql_mutex_unlock(&rli->mts_gaq_LOCK);
#endif
  return 0;
}

/**
 Logic to attach the temporary tables from the worker threads upon
 event execution.
 @param thd THD instance
 @param rli Relay_log_info instance
 @param ev  Query_log_event that is being applied
*/
void Mts_submode_logical_clock::attach_temp_tables(THD *thd,
                                                   const Relay_log_info *rli,
                                                   Query_log_event *ev) {
  bool shifted = false;
  TABLE *table, *cur_table;
  DBUG_TRACE;
  if (!is_mts_worker(thd) || (ev->ends_group() || ev->starts_group())) return;
  /* fetch coordinator's rli */
  Relay_log_info *c_rli = static_cast<const Slave_worker *>(rli)->c_rli;
  DBUG_ASSERT(!thd->temporary_tables);
  mysql_mutex_lock(&c_rli->mts_temp_table_LOCK);
  if (!(table = c_rli->info_thd->temporary_tables)) {
    mysql_mutex_unlock(&c_rli->mts_temp_table_LOCK);
    return;
  }
  c_rli->info_thd->temporary_tables = nullptr;
  do {
    /* store the current table */
    cur_table = table;
    /* move the table pointer to next in list, so that we can isolate the
    current table */
    table = table->next;
    std::pair<uint, my_thread_id> st_id_pair =
        get_server_and_thread_id(cur_table);
    if (thd->server_id == st_id_pair.first &&
        thd->variables.pseudo_thread_id == st_id_pair.second) {
      /* short the list singling out the current table */
      if (cur_table->prev)  // not the first node
        cur_table->prev->next = cur_table->next;
      if (cur_table->next)  // not the last node
        cur_table->next->prev = cur_table->prev;
      /* isolate the table */
      cur_table->prev = nullptr;
      cur_table->next = nullptr;
      mts_move_temp_tables_to_thd(thd, cur_table);
    } else
        /* We must shift the C->temp_table pointer to the fist table unused in
           this iteration. If all the tables have ben used C->temp_tables will
           point to NULL */
        if (!shifted) {
      c_rli->info_thd->temporary_tables = cur_table;
      shifted = true;
    }
  } while (table);
  mysql_mutex_unlock(&c_rli->mts_temp_table_LOCK);
}

/**
 Logic to detach the temporary tables from the worker threads upon
 event execution.
 @param thd THD instance
 @param rli Relay_log_info instance
*/
void Mts_submode_logical_clock::detach_temp_tables(THD *thd,
                                                   const Relay_log_info *rli,
                                                   Query_log_event *) {
  DBUG_TRACE;
  if (!is_mts_worker(thd)) return;
  /*
    Here in detach section we will move the tables from the worker to the
    coordinaor thread. Since coordinator is shared we need to make sure that
    there are no race conditions which may lead to assert failures and
    non-deterministic results.
  */
  Relay_log_info *c_rli = static_cast<const Slave_worker *>(rli)->c_rli;
  mysql_mutex_lock(&c_rli->mts_temp_table_LOCK);
  mts_move_temp_tables_to_thd(c_rli->info_thd, thd->temporary_tables);
  mysql_mutex_unlock(&c_rli->mts_temp_table_LOCK);
  thd->temporary_tables = nullptr;
}

/**
  Logic to get least occupied worker when the sql mts_submode= master_parallel
  @param rli relay log info of coordinator
  @param ws  array of worker threads
  @param ev  event for which we are searching for a worker.
  @return slave worker thread or NULL when coordinator is killed by any worker.
 */

Slave_worker *Mts_submode_logical_clock::get_least_occupied_worker(
    Relay_log_info *rli, Slave_worker_array *ws MY_ATTRIBUTE((unused)),
    Log_event *ev) {
  Slave_worker *worker = nullptr;
  PSI_stage_info *old_stage = nullptr;
  THD *thd = rli->info_thd;
  DBUG_TRACE;
#ifndef DBUG_OFF

  if (DBUG_EVALUATE_IF("mts_distribute_round_robin", 1, 0)) {
    worker = ws->at(w_rr % ws->size());
    LogErr(INFORMATION_LEVEL, ER_RPL_WORKER_ID_IS, worker->id,
           static_cast<ulong>(w_rr % ws->size()));
    DBUG_ASSERT(worker != nullptr);
    return worker;
  }
  Slave_committed_queue *gaq = rli->gaq;
  Slave_job_group *ptr_group;
  ptr_group = gaq->get_job_group(rli->gaq->assigned_group_index);
#endif
  /*
    The scheduling works as follows, in this sequence
      -If this is an internal event of a transaction  use the last assigned
        worker
      -If the i-th transaction is being scheduled in this group where "i" <=
       number of available workers then schedule the events to the consecutive
       workers
      -If the i-th transaction is being scheduled in this group where "i" >
       number of available workers then schedule this to the forst worker that
       becomes free.
   */
  if (rli->last_assigned_worker) {
    worker = rli->last_assigned_worker;
    DBUG_ASSERT(ev->get_type_code() != binary_log::USER_VAR_EVENT ||
                worker->id == 0 || rli->curr_group_seen_begin ||
                rli->curr_group_seen_gtid);
  } else {
    worker = get_free_worker(rli);

    DBUG_ASSERT(ev->get_type_code() != binary_log::USER_VAR_EVENT ||
                rli->curr_group_seen_begin || rli->curr_group_seen_gtid);

    if (worker == nullptr) {
      struct timespec ts[2];

      set_timespec_nsec(&ts[0], 0);
      // Update thd info as waiting for workers to finish.
      thd->enter_stage(&stage_slave_waiting_for_workers_to_process_queue,
                       old_stage, __func__, __FILE__, __LINE__);
      while (!worker && !thd->killed) {
        /*
          Busy wait with yielding thread control before to next attempt
          to find a free worker. As of current, a worker
          can't have more than one assigned group of events in its
          queue.

          todo: replace this At-Most-One assignment policy with
                First Available Worker as
                this method clearly can't be considered as optimal.
        */
#if !defined(_WIN32)
        sched_yield();
#else
        my_sleep(rli->mts_coordinator_basic_nap);
#endif
        worker = get_free_worker(rli);
      }
      THD_STAGE_INFO(thd, *old_stage);
      set_timespec_nsec(&ts[1], 0);
      rli->mts_total_wait_worker_avail += diff_timespec(&ts[1], &ts[0]);
      rli->mts_wq_no_underrun_cnt++;
      /*
        Even OPTION_BEGIN is set, the 'BEGIN' event is not dispatched to
        any worker thread. So The flag is removed and Coordinator thread
        will not try to finish the group before abort.
      */
      if (worker == nullptr)
        rli->info_thd->variables.option_bits &= ~OPTION_BEGIN;
    }
    if (rli->get_commit_order_manager() != nullptr && worker != nullptr)
      rli->get_commit_order_manager()->register_trx(worker);
  }

  DBUG_ASSERT(ptr_group);
  // assert that we have a worker thread for this event or the slave has
  // stopped.
  DBUG_ASSERT(worker != nullptr || thd->killed);
  /* The master my have send  db partition info. make sure we never use them*/
  if (ev->get_type_code() == binary_log::QUERY_EVENT)
    static_cast<Query_log_event *>(ev)->mts_accessed_dbs = 0;

  return worker;
}

/**
  Protected method to fetch a worker having no events assigned.
  The method is supposed to be called by Coordinator, therefore
  comparison like w_i->jobs.len == 0 must (eventually) succeed.

  todo: consider to optimize scan that is getting more expensive with
  more # of Workers.

  @return  a pointer to Worker or NULL if none is free.
*/
Slave_worker *Mts_submode_logical_clock::get_free_worker(Relay_log_info *rli) {
  for (Slave_worker **it = rli->workers.begin(); it != rli->workers.end();
       ++it) {
    Slave_worker *w_i = *it;
    if (w_i->jobs.len == 0) return w_i;
  }
  return nullptr;
}

/**
  Waits for slave workers to finish off the pending tasks before returning.
  Used in this submode to make sure that all assigned jobs have been done.

  @param rli  coordinator rli.
  @param ignore worker to ignore.
  @return -1 for error.
           0 no error.
 */
int Mts_submode_logical_clock::wait_for_workers_to_finish(
    Relay_log_info *rli, MY_ATTRIBUTE((unused)) Slave_worker *ignore) {
  PSI_stage_info *old_stage = nullptr;
  THD *thd = rli->info_thd;
  DBUG_TRACE;
  DBUG_PRINT("info", ("delegated %d, jobs_done %d", delegated_jobs, jobs_done));
  // Update thd info as waiting for workers to finish.
  thd->enter_stage(&stage_slave_waiting_for_workers_to_process_queue, old_stage,
                   __func__, __FILE__, __LINE__);
  while (delegated_jobs > jobs_done && !thd->killed && !is_error) {
    // Todo: consider to replace with a. GAQ::get_lwm_timestamp() or
    // b. (better) pthread wait+signal similarly to DB type.
    if (mts_checkpoint_routine(rli, true)) return -1;
  }

  // Check if there is a failure on a not-ignored Worker
  for (Slave_worker **it = rli->workers.begin(); it != rli->workers.end();
       ++it) {
    Slave_worker *w_i = *it;
    if (w_i->running_status != Slave_worker::RUNNING) return -1;
  }

  DBUG_EXECUTE_IF("wait_for_workers_to_finish_after_wait", {
    const char act[] = "now WAIT_FOR coordinator_continue";
    DBUG_ASSERT(!debug_sync_set_action(rli->info_thd, STRING_WITH_LEN(act)));
  });

  // The current commit point sequence may end here (e.g Rotate to new log)
  rli->gaq->lwm.sequence_number = SEQ_UNINIT;
  // Restore previous info.
  THD_STAGE_INFO(thd, *old_stage);
  DBUG_PRINT("info", ("delegated %d, jobs_done %d, Workers have finished their"
                      " jobs",
                      delegated_jobs, jobs_done));
  rli->mts_group_status = Relay_log_info::MTS_NOT_IN_GROUP;
  return !thd->killed && !is_error ? 0 : -1;
}

/**
  Protected method to fetch the server_id and pseudo_thread_id from a
  temporary table
  @param  table instance pointer of TABLE structure.
  @return std:pair<uint, my_thread_id>
  @note   It is the caller's responsibility to make sure we call this
          function only for temp tables.
 */
std::pair<uint, my_thread_id>
Mts_submode_logical_clock::get_server_and_thread_id(TABLE *table) {
  DBUG_TRACE;
  const char *extra_string = table->s->table_cache_key.str;
  size_t extra_string_len = table->s->table_cache_key.length;
  // assert will fail when called with non temporary tables.
  DBUG_ASSERT(table->s->table_cache_key.length > 0);
  std::pair<uint, my_thread_id> ret_pair = std::make_pair(
      /* last 8  bytes contains the server_id + pseudo_thread_id */
      // fetch first 4 bytes to get the server id.
      uint4korr(extra_string + extra_string_len - 8),
      /* next  4 bytes contains the pseudo_thread_id */
      uint4korr(extra_string + extra_string_len - 4));
  return ret_pair;
}

/**
 * Wait for all dependency slave workers to finish working on all enqueued trxs
 *
 * @param rli           Relay log info of the coordinator thread
 * @param partial_trx   Have we queued a partial transaction?
 *                      If true, we can't blindly wait for the trx counter to be
 *                      zero (since the worker will not be able to complete
 *                      that transaction)
 * @return bool         true if all is well, false otherwise
 */
bool Mts_submode_dependency::wait_for_dep_workers_to_finish(
    Relay_log_info *rli, const bool partial_trx) {
  DBUG_ASSERT(is_mts_parallel_type_dependency(rli));
  PSI_stage_info old_stage;

  mysql_mutex_lock(&dep_lock);

  const ulonglong num = partial_trx ? 1 : 0;
  rli->info_thd->ENTER_COND(&dep_trx_all_done_cond, &dep_lock,
                            &stage_slave_waiting_for_dependency_workers,
                            &old_stage);
  while (num_in_flight_trx > num && !rli->info_thd->killed) {
    mysql_cond_wait(&dep_trx_all_done_cond, &dep_lock);
  }

  mysql_mutex_unlock(&dep_lock);
  rli->info_thd->EXIT_COND(&old_stage);

  return !rli->info_thd->killed;
}

void Mts_submode_dependency::add_row_event(
    Relay_log_info *rli, Rows_log_event *rle,
    std::shared_ptr<Log_event_wrapper> ev,
    std::deque<Dependency_key> &keylist) {
  // case: something went wrong while finding keys for this event, switch to
  // sync mode!
  if (unlikely(!rle->get_keys(rli, keylist) ||
               keylist.size() + keys_accessed_by_group.size() >
                   rli->mts_dependency_max_keys)) {
    set_dep_sync_group(true);
    keylist.clear();
    keys_accessed_by_group.clear();
    return;
  }

  keys_accessed_by_group.insert(keylist.begin(), keylist.end());

  mysql_mutex_lock(&dep_key_lookup_mutex);
  /* Handle dependencies. */
  for (const auto &k : keylist) {
    auto last_key_event = dep_key_lookup.find(k);
    if (last_key_event != dep_key_lookup.end()) {
      last_key_event->second->add_dependent(ev);
    }
  }
  mysql_mutex_unlock(&dep_key_lookup_mutex);
}

void Mts_submode_dependency::register_keys(
    std::shared_ptr<Log_event_wrapper> &ev) {
  mysql_mutex_lock(&dep_key_lookup_mutex);
  if (!ev->finalized()) {
    for (const auto &key : keys_accessed_by_group) {
      dep_key_lookup[key] = ev;
      ev->keys.insert(key);
    }
  }
  mysql_mutex_unlock(&dep_key_lookup_mutex);
}

void Mts_submode_dependency::unregister_keys(
    std::shared_ptr<Log_event_wrapper> &ev) {
  /* Attempt to clean up entries from the key lookup
   *
   * There are two cases:
   * 1) The "value" of the key-value pair is equal to this event. In this case,
   *    remove the key-value pair from the map.
   * 2) The "value" of the key-value pair is _not_ equal to this event. In this
   *    case, leave it be; the event corresponds to a later transaction.
   */
  mysql_mutex_lock(&dep_key_lookup_mutex);
  if (likely(!dep_key_lookup.empty())) {
    for (const auto &key : ev->keys) {
      const auto it = dep_key_lookup.find(key);
      DBUG_ASSERT(it != dep_key_lookup.end());

      /* Case 1. (Case 2 is implicitly handled by doing nothing.) */
      if (it->second == ev) {
        dep_key_lookup.erase(it);
      }
    }
  }
  ev->finalize();
  mysql_mutex_unlock(&dep_key_lookup_mutex);
}

std::shared_ptr<Log_event_wrapper> Mts_submode_dependency::dequeue(
    Relay_log_info *rli, Slave_worker *worker, Commit_order_manager *co_mngr) {
  std::shared_ptr<Log_event_wrapper> ret;

  mysql_mutex_lock(&dep_lock);

  PSI_stage_info old_stage;
  worker->info_thd->ENTER_COND(&dep_empty_cond, &dep_lock,
                               &stage_slave_waiting_event_from_coordinator,
                               &old_stage);

  while (!worker->info_thd->killed &&
         worker->running_status == Slave_worker::RUNNING && dep_queue.empty()) {
    ++begin_event_waits;
    ++num_workers_waiting;
    const auto timeout_nsec =
        worker->c_rli->mts_dependency_cond_wait_timeout * 1000000;
    struct timespec abstime;
    set_timespec_nsec(&abstime, timeout_nsec);
    mysql_cond_timedwait(&dep_empty_cond, &dep_lock, &abstime);
    --num_workers_waiting;
  }

  if (!dep_queue.empty()) {
    ret = dep_queue.front();
    dep_queue.pop_front();
  }

  // case: place ourselves in the commit order queue
  if (ret && co_mngr != nullptr) {
    DBUG_ASSERT(rli->slave_preserve_commit_order);
    // case: if we need to order commits by DB we set current DB
    if (rli->mts_dependency_order_commits == DEP_RPL_ORDER_DB)
      worker->set_current_db(ret->get_db());
    co_mngr->register_trx(worker);
  }

  // case: signal if queue is now empty
  if (dep_queue.empty()) mysql_cond_signal(&dep_empty_cond);

  // admission control
  if (unlikely(dep_full)) {
    DBUG_ASSERT(dep_queue.size() > 0);
    // case: signal if dep has space
    if (dep_queue.size() < (rli->mts_dependency_size *
                            rli->mts_dependency_refill_threshold / 100)) {
      dep_full = false;
      mysql_cond_signal(&dep_full_cond);
    }
  }

  mysql_mutex_unlock(&dep_lock);
  worker->info_thd->EXIT_COND(&old_stage);
  return ret;
}

void Mts_submode_dependency::enqueue(
    Relay_log_info *rli,
    const std::shared_ptr<Log_event_wrapper> &begin_event) {
  mysql_mutex_lock(&dep_lock);

  // wait if queue has reached full capacity
  while (unlikely(dep_full)) {
    const auto timeout_nsec = rli->mts_dependency_cond_wait_timeout * 1000000;
    struct timespec abstime;
    set_timespec_nsec(&abstime, timeout_nsec);
    mysql_cond_timedwait(&dep_full_cond, &dep_lock, &abstime);
  }

  dep_queue.push_back(begin_event);

  // case: workers are waiting on empty queue, let's signal
  if (likely(num_workers_waiting > 0)) {
    DBUG_ASSERT(num_workers_waiting <= rli->opt_slave_parallel_workers);
    mysql_cond_signal(&dep_empty_cond);
  }

  // admission control in dep queue
  dep_full = dep_queue.size() >= rli->mts_dependency_size;

  ++num_in_flight_trx;
  trx_queued = true;

  mysql_mutex_unlock(&dep_lock);
}

/*
  Adds an event to the dependency queue
*/
bool Mts_submode_dependency::schedule_dep(Relay_log_info *rli, Log_event *ev) {
  DBUG_ENTER("Log_event::schedule_dep");

  auto evw = std::make_shared<Log_event_wrapper>(
      ev, current_begin_event, rli->get_event_relay_log_number(),
      rli->get_event_start_pos());
  ev->prepare_dep(rli, evw);

  if (ev->starts_group()) {
    rli->curr_group_seen_begin = true;
    rli->mts_end_group_sets_max_dbs = true;
  } else if (is_gtid_event(ev)) {
    rli->curr_group_seen_gtid = true;
  } else if (ev->contains_partition_info(rli->mts_end_group_sets_max_dbs)) {
    if (ev->get_type_code() == binary_log::TABLE_MAP_EVENT) {
      const auto tbe = static_cast<Table_map_log_event *>(ev);
      const auto id = tbe->get_table_id();
      table_map_events[id] = tbe;
    }

    rli->mts_end_group_sets_max_dbs = false;

    // NOTE: we don't update @Relay_log_info::keys_accessed_by_group here
    // because not all partition events contain table info
    Mts_db_names mts_dbs;
    const uint8 num_dbs = ev->get_mts_dbs(&mts_dbs, rli->rpl_filter);

    if (likely(num_dbs != OVER_MAX_DBS_IN_EVENT_MTS)) {
      for (int i = 0; i < mts_dbs.num; ++i)
        dbs_accessed_by_group.insert(mts_dbs.name[i]);
    }

    // we execute the trx in isolation if num_dbs is greater than one and if
    // OVER_MAX_DBS_IN_EVENT_MTS is set
    set_dep_sync_group(dep_sync_group || num_dbs == OVER_MAX_DBS_IN_EVENT_MTS ||
                       (dbs_accessed_by_group.size() > 1));
  }

  if (unlikely(dep_sync_group)) {
    if (!wait_for_dep_workers_to_finish(rli, trx_queued)) DBUG_RETURN(false);
  }

  handle_terminal_event(ev->thd, rli, evw);

#ifndef DBUG_OFF
  if (!evw->is_end_event) {
    auto be = evw->is_begin_event ? evw : evw->begin_event();
    DBUG_ASSERT(be == current_begin_event);
  }
#endif

  ev->mts_group_idx = rli->gaq->assigned_group_index;

  // case: current trx is to be executed in isolation or the DB has been set, so
  // let's queue it for execution
  if (!trx_queued &&
      (dep_sync_group || !current_begin_event->get_db().empty())) {
    enqueue(rli, current_begin_event);
  }

  DBUG_ASSERT(evw->is_begin_event || prev_event);
  if (prev_event) {
    prev_event->put_next(evw);
  }

  if (unlikely(evw->is_end_event)) {
    prev_event = nullptr;
    current_begin_event.reset();
    trx_queued = false;
  } else {
    prev_event = evw;
  }

  // case: this group needs to be executed in isolation
  if (unlikely(dep_sync_group && evw->is_end_event)) {
    if (wait_for_workers_to_finish(rli) == -1) DBUG_RETURN(false);
    set_dep_sync_group(false);
  }

  DBUG_RETURN(true);
}

/**
  Encapsulation for things to be done for terminal begin and end events
*/
void Mts_submode_dependency::handle_terminal_event(
    THD *thd, Relay_log_info *rli, std::shared_ptr<Log_event_wrapper> &evw) {
  Log_event *ev = evw->raw_event();

  if (evw->is_begin_event) {
    DBUG_ASSERT(current_begin_event == NULL);
    DBUG_ASSERT(table_map_events.empty());
    DBUG_ASSERT(keys_accessed_by_group.empty());
    DBUG_ASSERT(!trx_queued);

    // update rli state
    current_begin_event = evw;
    rli->mts_groups_assigned++;

    // insert a group representative in the GAQ
    Slave_job_group group;
    group.reset(ev->common_header->log_pos, rli->mts_groups_assigned);
    group.group_master_log_name = my_strdup(
        key_memory_log_event, rli->get_group_master_log_name(), MYF(MY_WME));
    rli->gaq->assigned_group_index = rli->gaq->en_queue(&group);
  }

  DBUG_ASSERT(current_begin_event);

  // case: we have found a DB to set for the current trx
  if (current_begin_event->get_db().empty() && !dbs_accessed_by_group.empty()) {
    // NOTE: we can set any accessed DB to this trx, if more than one DB are
    // accessed then it'll be executed in isolation anyway
    DBUG_ASSERT(dep_sync_group || dbs_accessed_by_group.size() == 1);
    current_begin_event->set_db(*dbs_accessed_by_group.begin());
  }

  if (evw->is_end_event) {
    // Populate key->last trx penultimate event in the key lookup
    //
    // NOTE: We store the end event for a single event trx
    //
    // Why store penultimate event instead of end event?
    // This is to improve perf for TBL mode. When we depend on the end event of
    // trx we basically wait for it to commit. In TBL mode we might end up
    // waiting for a trx that we don't have any row conflits with. That's why we
    // wait on the penultimate event instead, this enables us to start executing
    // before waiting for the dependent trx to commit if there are no conflits.
    // If there are conflicts we expect row locks to kick in and in that case
    // we'll automatically wait for the conflicting trx to commit.
    //
    // Note that this is not required for STMT mode since we'll already be
    // depending on trxs based on actual row conflicts. So we depend on the end
    // event directly.
    //
    // Why penultimate though? Why not just depend on the conflicting row event?
    // This is done to support trx retries. On secondaries we're allowed to
    // retry trx on temprary errors like lock wait timeouts. Depending on
    // penultimate event allows the trx we depend on to retry execution,
    // otherwise we'll end up taking the row lock as soon as the row we depend
    // on is executed which can create deadlock if commit ordering is enabled.
    auto to_add = rli->mts_dependency_replication == DEP_RPL_TABLE && prev_event
                      ? prev_event
                      : evw;
    register_keys(to_add);

    // update rli state
    table_map_events.clear();
    dbs_accessed_by_group.clear();
    keys_accessed_by_group.clear();

    rli->mts_group_status = Relay_log_info::MTS_END_GROUP;
    rli->curr_group_seen_begin = rli->curr_group_seen_gtid = false;

    // update coordinates in GAQ
    Slave_job_group *ptr_group =
        rli->gaq->get_job_group(rli->gaq->assigned_group_index);

    ptr_group->group_relay_log_name = my_strdup(
        key_memory_log_event, rli->get_event_relay_log_name(), MYF(MY_WME));
    ptr_group->checkpoint_log_name = my_strdup(
        key_memory_log_event, rli->get_group_master_log_name(), MYF(MY_WME));
    ptr_group->checkpoint_relay_log_name = my_strdup(
        key_memory_log_event, rli->get_group_relay_log_name(), MYF(MY_WME));
    ptr_group->checkpoint_log_pos = rli->get_group_master_log_pos();
    ptr_group->checkpoint_relay_log_pos = rli->get_group_relay_log_pos();

    ptr_group->checkpoint_seqno = rli->rli_checkpoint_seqno;
    rli->rli_checkpoint_seqno++;

    // seconds_behind_master related
    if (unlikely(ev->is_relay_log_event()))
      ptr_group->ts = 0;
    else {
      ptr_group->ts = ev->common_header->when.tv_sec + (time_t)ev->exec_time;

      ptr_group->ts_millis = (rli->group_timestamp_millis
                                  ? rli->group_timestamp_millis
                                  : ev->common_header->when.tv_sec * 1000) +
                             ev->exec_time * 1000;
      // reset for next group
      rli->group_timestamp_millis = 0;
    }
    DBUG_EXECUTE_IF("dbug.dep_wait_before_sending_end_event", {
      const char act[] = "now signal signal.reached wait_for signal.done";
      DBUG_ASSERT(opt_debug_sync_timeout > 0);
      DBUG_ASSERT(!debug_sync_set_action(rli->info_thd, STRING_WITH_LEN(act)));
    };);
    free_root(thd->mem_root, MYF(MY_KEEP_PREALLOC));
  }
}

void Mts_submode_dependency::signal_trx_done(
    std::shared_ptr<Log_event_wrapper> begin_event) {
  mysql_mutex_lock(&dep_lock);
  if (begin_event) {
    DBUG_ASSERT(num_in_flight_trx > 0);
    --num_in_flight_trx;
  }
  if (num_in_flight_trx <= 1) mysql_cond_broadcast(&dep_trx_all_done_cond);
  mysql_mutex_unlock(&dep_lock);

  cleanup_group(begin_event);
}

bool Mts_submode_dependency::is_partially_enqueued_trx(Relay_log_info *rli) {
  // this will be used to determine if we need to deal with a worker that is
  // handling a partially enqueued trx
  bool partial = false;
  mysql_mutex_lock(&dep_lock);
  // case: until condition is specified, so we have to execute all
  // transactions in the queue
  if (unlikely(rli->until_condition != Relay_log_info::UNTIL_NONE)) {
    // we have a partial trx if the coordinator has set the trx_queued flag,
    // this is because this flag is set to false after coordinator sees an end
    // event
    partial = trx_queued;
  }
  // case: until condition is not specified
  else {
    // the partial check is slightly complicated in this case because we only
    // care about partial trx that has been pulled by a worker, since the
    // queue is going to be emptied next anyway, we make this check by
    // checking of the queue is empty
    partial = dep_queue.empty() && trx_queued;
    // let's cleanup, we can clear the queue in this case
    clear_dep(false);
  }
  mysql_mutex_unlock(&dep_lock);

  return partial;
}

void Mts_submode_dependency::stop_dependency_workers(Relay_log_info *rli) {
  THD *thd = rli->info_thd;

  wait_for_dep_workers_to_finish(rli, is_partially_enqueued_trx(rli));

  // case: if UNTIL is specified let's clean up after waiting for workers
  if (unlikely(rli->until_condition != Relay_log_info::UNTIL_NONE)) {
    clear_dep(true);
  }

  // set all workers as STOP_ACCEPTED, and signal blocked workers
  for (int i = static_cast<int>(rli->workers.size()) - 1; i >= 0; i--) {
    Slave_worker *w = rli->workers[i];
    mysql_mutex_lock(&w->jobs_lock);

    if (w->running_status != Slave_worker::RUNNING) {
      mysql_mutex_unlock(&w->jobs_lock);
      continue;
    }

    w->running_status = Slave_worker::STOP_ACCEPTED;

    // unblock workers waiting for new events or trxs
    mysql_mutex_lock(&w->info_thd->LOCK_thd_data);
    w->info_thd->awake(w->info_thd->killed);
    mysql_mutex_unlock(&w->info_thd->LOCK_thd_data);

    mysql_mutex_unlock(&w->jobs_lock);
  }

  thd_proc_info(thd, "Waiting for workers to exit");

  for (int i = static_cast<int>(rli->workers.size()) - 1; i >= 0; i--) {
    Slave_worker *w = rli->workers[i];
    mysql_mutex_lock(&w->jobs_lock);

    // wait for workers to stop running
    while (w->running_status != Slave_worker::NOT_RUNNING) {
      struct timespec abstime;
      set_timespec(&abstime, 1);
      mysql_cond_timedwait(&w->jobs_cond, &w->jobs_lock, &abstime);
    }

    mysql_mutex_unlock(&w->jobs_lock);
  }
  dependency_worker_error = false;
}
