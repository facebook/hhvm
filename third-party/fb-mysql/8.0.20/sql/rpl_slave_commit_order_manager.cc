/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/rpl_slave_commit_order_manager.h"

#include "sql/mysqld.h"       // key_commit_order_manager_mutex ..
#include "sql/rpl_rli_pdb.h"  // Slave_worker
#include "sql/sql_lex.h"

Commit_order_manager::Commit_order_manager(uint32 worker_numbers)
    : m_workers(worker_numbers) {
  mysql_mutex_init(key_commit_order_manager_mutex, &m_mutex, nullptr);
  unset_rollback_status();

  for (uint32 i = 0; i < worker_numbers; i++) {
    mysql_cond_init(key_commit_order_manager_cond, &m_workers[i].cond);
    m_workers[i].stage = enum_transaction_stage::FINISHED;
  }
}

Commit_order_manager::~Commit_order_manager() {
  mysql_mutex_destroy(&m_mutex);

  for (uint32 i = 0; i < m_workers.size(); i++) {
    mysql_cond_destroy(&m_workers[i].cond);
  }
}

void Commit_order_manager::register_trx(Slave_worker *worker) {
  DBUG_TRACE;

  mysql_mutex_lock(&m_mutex);
  const auto db = worker->get_current_db();

  DBUG_PRINT("info", ("Worker %d added to the commit order queue",
                      (int)worker->info_thd->thread_id()));

  /* only transition allowed: FINISHED -> REGISTERED */
  DBUG_ASSERT(m_workers[worker->id].stage == enum_transaction_stage::FINISHED);

  m_workers[worker->id].stage = enum_transaction_stage::REGISTERED;
  queue_push(db, worker->id);

  mysql_mutex_unlock(&m_mutex);
}

bool Commit_order_manager::wait(Slave_worker *worker) {
  DBUG_TRACE;

  /*
    When prior transaction fail, current trx should stop and wait for signal
    to rollback itself
  */
  if (m_workers[worker->id].stage == enum_transaction_stage::REGISTERED) {
    PSI_stage_info old_stage;
    mysql_cond_t *cond = &m_workers[worker->id].cond;
    THD *thd = worker->info_thd;
    const auto db = worker->get_current_db();

    DBUG_PRINT("info", ("Worker %lu is waiting for commit signal", worker->id));
    CONDITIONAL_SYNC_POINT_FOR_TIMESTAMP("commit_order_manager_before_wait");
    mysql_mutex_lock(&m_mutex);
    thd->ENTER_COND(cond, &m_mutex,
                    &stage_worker_waiting_for_its_turn_to_commit, &old_stage);

    while (queue_front(db) != worker->id) {
      if (unlikely(worker->found_commit_order_deadlock())) {
        mysql_mutex_unlock(&m_mutex);
        thd->EXIT_COND(&old_stage);
        return true;
      }
      mysql_cond_wait(cond, &m_mutex);
    }

    bool rollback_status = m_rollback_trx.load();
    mysql_mutex_unlock(&m_mutex);
    thd->EXIT_COND(&old_stage);

    DBUG_EXECUTE_IF("rpl_fake_commit_order_deadlock_for_timestamp_100", {
      if (thd->start_time.tv_sec == 100) {
        my_error(ER_UNKNOWN_ERROR, MYF(0));
        return true;
      }

      if (thd->start_time.tv_sec == 200 && !rollback_status) {
        my_error(ER_UNKNOWN_ERROR, MYF(0));
        return true;
      }
    });

    CONDITIONAL_SYNC_POINT_FOR_TIMESTAMP("commit_order_manager_after_wait");
    m_workers[worker->id].stage = enum_transaction_stage::WAITED;

    if (rollback_status) {
      finish_one(worker);

      DBUG_PRINT("info", ("thd has seen an error signal from old thread"));
      thd->get_stmt_da()->set_overwrite_status(true);
      my_error(ER_SLAVE_WORKER_STOPPED_PREVIOUS_THD_ERROR, MYF(0));
    }
    /*
      Set HA_IGNORE_DURABILITY so that transaction is not flushed to the
      storage engine immediately, instead we hold all the applier worker
      threads and flush them together in group.
      The tx_commit_pending variable determines if transaction commit is
      pending, and next_to_commit is used to maintain the commit queue for
      group commit. The tx_commit_pending and next_to_commit variables are
      reset before thread enters group commit later.
    */
    else if (thd->is_current_stmt_binlog_disabled()) {
      thd->durability_property = HA_IGNORE_DURABILITY;
      thd->tx_commit_pending = true;
      thd->next_to_commit = nullptr;
    }

    return rollback_status;
  }

  return false;
}

void Commit_order_manager::flush_engine_and_signal_threads(
    Slave_worker *worker) {
  DBUG_TRACE;

  /*
    Enroll the session in FLUSH stage queue so that transactions can be flushed
    together in group with binlog queued sessions. enroll_for will return false
    in case this thread became a follower; in that case the leader has completed
    the flush and signal. If this thread is leader, enroll_for returns true and
    holds the queue lock for COMMIT_ORDER_FLUSH_STAGE as well as the stage lock
    for the the flush stage. In the rest of this function, this thread is a
    leader and will flush to engine and update gtid state, on behalf of all the
    followers.
  */
  if (!Commit_stage_manager::get_instance().enroll_for(
          Commit_stage_manager::COMMIT_ORDER_FLUSH_STAGE, worker->info_thd,
          nullptr, mysql_bin_log.get_log_lock())) {
    m_workers[worker->id].stage = enum_transaction_stage::FINISHED;
    return;
  }

  /* Fetch leader thread from commit order flush queue */
  THD *first =
      Commit_stage_manager::get_instance().fetch_queue_skip_acquire_lock(
          Commit_stage_manager::COMMIT_ORDER_FLUSH_STAGE);

  Commit_stage_manager::get_instance().unlock_queue(
      Commit_stage_manager::COMMIT_ORDER_FLUSH_STAGE);

  mysql_mutex_lock(mysql_bin_log.get_commit_lock());
  mysql_mutex_unlock(mysql_bin_log.get_log_lock());

  CONDITIONAL_SYNC_POINT_FOR_TIMESTAMP(
      "commit_order_leader_before_ha_flush_logs");

  /* flush transactions to the storage engine in a group */
  ha_flush_logs(true);

  reset_server_status(first);

  /* add to @@global.gtid_executed */
  gtid_state->update_commit_group(first);

  mysql_mutex_unlock(mysql_bin_log.get_commit_lock());

  /*
    awake all waiting threads for leader to flush transactions
    to the storage engine
  */
  Commit_stage_manager::get_instance().signal_done(
      first, Commit_stage_manager::COMMIT_ORDER_FLUSH_STAGE);
}

void Commit_order_manager::reset_server_status(THD *first_thd) {
  DBUG_TRACE;

  for (THD *thd = first_thd; thd != nullptr; thd = thd->next_to_commit) {
    thd->server_status &= ~SERVER_STATUS_IN_TRANS;
  }
}

void Commit_order_manager::finish_one(Slave_worker *worker) {
  DBUG_TRACE;

  const auto db = worker->get_current_db();

  mysql_mutex_lock(&m_mutex);

  if (m_workers[worker->id].stage == enum_transaction_stage::WAITED) {
    DBUG_ASSERT(queue_front(db) == worker->id);
    DBUG_ASSERT(!queue_empty(db));

    /* Set next worker in the queue as the head and signal the trx to commit. */
    queue_pop(db);

    if (!queue_empty(db)) mysql_cond_signal(&m_workers[queue_front(db)].cond);

    m_workers[worker->id].stage = enum_transaction_stage::FINISHED;
  }

  mysql_mutex_unlock(&m_mutex);
}

void Commit_order_manager::finish(Slave_worker *worker) {
  DBUG_TRACE;

  if (m_workers[worker->id].stage == enum_transaction_stage::WAITED) {
    DBUG_PRINT("info",
               ("Worker %lu is signalling next transaction", worker->id));

    if (!get_rollback_status() &&
        worker->info_thd->is_current_stmt_binlog_disabled()) {
      /*
        If flush queue contains only threads executing slave preserve commit
        order, then flush committed transactions of all those waiting threads
        to the storage engine and awake them from wait. But if flush queue
        also contains threads writing to binlog, then it changes leader, so
        that the first BGC thread becomes leader. It waits until the new leader
        has committed and signalled all waiting commit order threads.
      */
      flush_engine_and_signal_threads(worker);

    } else {
      /*
        signal top worker of commit order queue to come out of wait
        and continue processing.
      */
      finish_one(worker);
    }
  }
}

void Commit_order_manager::check_and_report_deadlock(THD *thd_self,
                                                     THD *thd_wait_for) {
  DBUG_TRACE;

  Slave_worker *self_w = get_thd_worker(thd_self);
  Slave_worker *wait_for_w = get_thd_worker(thd_wait_for);
  Commit_order_manager *mngr = self_w->get_commit_order_manager();

  /* Check if both workers are working for the same channel */
  if (mngr != nullptr && self_w->c_rli == wait_for_w->c_rli &&
      wait_for_w->sequence_number() > self_w->sequence_number()) {
    DBUG_PRINT("info", ("Found slave order commit deadlock"));
    mngr->report_deadlock(wait_for_w);
  }
}

void Commit_order_manager::report_deadlock(Slave_worker *worker) {
  DBUG_TRACE;
  THD *thd = worker->info_thd;
  mysql_mutex_lock(&thd->LOCK_thd_data);
  ++slave_commit_order_deadlocks;
  worker->report_commit_order_deadlock();
  DBUG_EXECUTE_IF("rpl_fake_cod_deadlock", {
    const char act[] = "now signal reported_deadlock";
    DBUG_ASSERT(!debug_sync_set_action(current_thd, STRING_WITH_LEN(act)));
  });
  /* Let's signal to any wait loop this worker is executing, this will also
   * cover the wait loop in Commit_order_manager::wait().
   * NOTE: we just want to send a signal without changing the killed flag
   */
  thd->awake(thd->killed);
  mysql_mutex_unlock(&thd->LOCK_thd_data);
}

bool Commit_order_manager::wait(THD *thd) {
  DBUG_TRACE;
  DBUG_ASSERT(thd);

  if (has_commit_order_manager(thd)) {
    /*
      We only care about read/write transactions and those that
      have been registered in the commit order manager.
     */
    Slave_worker *worker = dynamic_cast<Slave_worker *>(thd->rli_slave);
    Commit_order_manager *mngr = worker->get_commit_order_manager();

    if (mngr->wait(worker)) {
      thd->commit_error = THD::CE_COMMIT_ERROR;
      return true;
    }
  }
  return false;
}

void Commit_order_manager::wait_and_finish(THD *thd, bool error) {
  DBUG_TRACE;
  DBUG_ASSERT(thd);

  if (has_commit_order_manager(thd)) {
    /*
      We only care about read/write transactions and those that
      have been registered in the commit order manager.
     */
    Slave_worker *worker = dynamic_cast<Slave_worker *>(thd->rli_slave);
    Commit_order_manager *mngr = worker->get_commit_order_manager();

    if (error || worker->found_commit_order_deadlock()) {
      // Error or deadlock: if not retryable, release next worker
      bool ret;
      std::tie(ret, std::ignore, std::ignore) =
          worker->check_and_report_end_of_retries(thd);
      if (ret) {
        worker->reset_commit_order_deadlock();
        /*
          worker can set m_rollback_trx when it is its turn to commit,
          so need to call wait() before updating m_rollback_trx.
        */
        mngr->wait(worker);
        mngr->set_rollback_status();
        mngr->finish(worker);
      }
    } else {
      // No error or deadlock: release next worker.
      mngr->wait(worker);
      mngr->finish(worker);
    }
  }
}

bool Commit_order_manager::get_rollback_status() {
  return m_rollback_trx.load();
}

void Commit_order_manager::set_rollback_status() { m_rollback_trx.store(true); }

void Commit_order_manager::unset_rollback_status() {
  m_rollback_trx.store(false);
}

bool Commit_order_manager::get_rollback_status(THD *thd) {
  DBUG_TRACE;
  DBUG_ASSERT(thd);
  if (has_commit_order_manager(thd)) {
    /*
      We only care about read/write transactions and those that
      have been registered in the commit order manager.
     */
    Slave_worker *worker = dynamic_cast<Slave_worker *>(thd->rli_slave);
    Commit_order_manager *mngr = worker->get_commit_order_manager();

    return mngr->get_rollback_status();
  }
  return false;
}

void Commit_order_manager::finish_one(THD *thd) {
  DBUG_TRACE;
  DBUG_ASSERT(thd);
  if (has_commit_order_manager(thd)) {
    /*
      We only care about read/write transactions and those that
      have been registered in the commit order manager.
     */
    Slave_worker *worker = dynamic_cast<Slave_worker *>(thd->rli_slave);
    Commit_order_manager *mngr = worker->get_commit_order_manager();

    mngr->finish_one(worker);
  }
}

bool has_commit_order_manager(THD *thd) {
  return is_mts_worker(thd) &&
         thd->rli_slave->get_commit_order_manager() != nullptr;
}

bool Commit_order_manager::wait_for_its_turn_before_flush_stage(THD *thd) {
  switch (thd->lex->sql_command) {
    case SQLCOM_ALTER_TABLE:
    case SQLCOM_ANALYZE:
    case SQLCOM_DROP_DB:
    case SQLCOM_DROP_EVENT:
    case SQLCOM_DROP_FUNCTION:
    case SQLCOM_DROP_PROCEDURE:
    case SQLCOM_DROP_TRIGGER:
    case SQLCOM_DROP_TABLE:
    case SQLCOM_DROP_VIEW:
    case SQLCOM_OPTIMIZE:
    case SQLCOM_REPAIR:
      return has_commit_order_manager(thd);
    default:
      break;
  }
  return false;
}
