/* Copyright (c) 2016, Facebook. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include "sql/sql_admission_control.h"
#include "sql/auth/auth_acls.h"
#include "sql/mysqld.h"
#include "sql/sql_class.h"
#include "sql/sql_lex.h"

#define IS_BIT_SET(val, n) ((val) & (1 << (n)))

bool opt_admission_control_by_trx = false;
ulonglong admission_control_filter;
char *admission_control_weights;
ulonglong admission_control_wait_events;
ulonglong admission_control_yield_freq;
bool admission_control_multiquery_filter;

AC *db_ac;
#ifdef HAVE_PSI_INTERFACE
PSI_stage_info stage_admission_control_enter = {0, "Admission control enter", 0,
                                                PSI_DOCUMENT_ME};
PSI_stage_info stage_admission_control_exit = {0, "Admission control exit", 0,
                                               PSI_DOCUMENT_ME};
PSI_stage_info stage_waiting_for_admission = {0, "waiting for admission", 0,
                                              PSI_DOCUMENT_ME};
PSI_stage_info stage_waiting_for_readmission = {0, "waiting for readmission", 0,
                                                PSI_DOCUMENT_ME};
#endif

/**
  @param sql_command command the thread is currently executing

  @return true Skips the current query in admission control
          false Admission control checks are applied for this query
*/
bool filter_command(enum_sql_command sql_command) {
  switch (sql_command) {
    case SQLCOM_ALTER_TABLE:
    case SQLCOM_ALTER_DB:
    case SQLCOM_ALTER_PROCEDURE:
    case SQLCOM_ALTER_FUNCTION:
    case SQLCOM_ALTER_TABLESPACE:
    case SQLCOM_ALTER_SERVER:
    case SQLCOM_ALTER_EVENT:
    case SQLCOM_ALTER_USER:
    case SQLCOM_RENAME_TABLE:
    case SQLCOM_RENAME_USER:
    case SQLCOM_ALTER_INSTANCE:
    case SQLCOM_ALTER_USER_DEFAULT_ROLE:
    case SQLCOM_ALTER_RESOURCE_GROUP:
      return IS_BIT_SET(admission_control_filter, ADMISSION_CONTROL_ALTER);

    case SQLCOM_BEGIN:
      return IS_BIT_SET(admission_control_filter, ADMISSION_CONTROL_BEGIN);

    case SQLCOM_COMMIT:
      return IS_BIT_SET(admission_control_filter, ADMISSION_CONTROL_COMMIT);

    case SQLCOM_CREATE_TABLE:
    case SQLCOM_CREATE_INDEX:
    case SQLCOM_CREATE_DB:
    case SQLCOM_CREATE_FUNCTION:
    case SQLCOM_CREATE_USER:
    case SQLCOM_CREATE_PROCEDURE:
    case SQLCOM_CREATE_SPFUNCTION:
    case SQLCOM_CREATE_VIEW:
    case SQLCOM_CREATE_TRIGGER:
    case SQLCOM_CREATE_SERVER:
    case SQLCOM_CREATE_EVENT:
    case SQLCOM_CREATE_ROLE:
    case SQLCOM_CREATE_RESOURCE_GROUP:
    case SQLCOM_CREATE_SRS:
      return IS_BIT_SET(admission_control_filter, ADMISSION_CONTROL_CREATE);

    case SQLCOM_DELETE:
    case SQLCOM_DELETE_MULTI:
      return IS_BIT_SET(admission_control_filter, ADMISSION_CONTROL_DELETE);

    case SQLCOM_DROP_TABLE:
    case SQLCOM_DROP_INDEX:
    case SQLCOM_DROP_DB:
    case SQLCOM_DROP_FUNCTION:
    case SQLCOM_DROP_USER:
    case SQLCOM_DROP_PROCEDURE:
    case SQLCOM_DROP_VIEW:
    case SQLCOM_DROP_TRIGGER:
    case SQLCOM_DROP_SERVER:
    case SQLCOM_DROP_EVENT:
    case SQLCOM_DROP_ROLE:
    case SQLCOM_DROP_SRS:
      return IS_BIT_SET(admission_control_filter, ADMISSION_CONTROL_DROP);

    case SQLCOM_INSERT:
    case SQLCOM_INSERT_SELECT:
      return IS_BIT_SET(admission_control_filter, ADMISSION_CONTROL_INSERT);

    case SQLCOM_LOAD:
      return IS_BIT_SET(admission_control_filter, ADMISSION_CONTROL_LOAD);

    case SQLCOM_SELECT:
      return IS_BIT_SET(admission_control_filter, ADMISSION_CONTROL_SELECT);

    case SQLCOM_SET_OPTION:
      return IS_BIT_SET(admission_control_filter, ADMISSION_CONTROL_SET);

    case SQLCOM_REPLACE:
    case SQLCOM_REPLACE_SELECT:
      return IS_BIT_SET(admission_control_filter, ADMISSION_CONTROL_REPLACE);

    case SQLCOM_ROLLBACK:
      return IS_BIT_SET(admission_control_filter, ADMISSION_CONTROL_ROLLBACK);

    case SQLCOM_TRUNCATE:
      return IS_BIT_SET(admission_control_filter, ADMISSION_CONTROL_TRUNCATE);

    case SQLCOM_UPDATE:
    case SQLCOM_UPDATE_MULTI:
      return IS_BIT_SET(admission_control_filter, ADMISSION_CONTROL_UPDATE);

    case SQLCOM_SHOW_DATABASES:
    case SQLCOM_SHOW_TABLES:
    case SQLCOM_SHOW_FIELDS:
    case SQLCOM_SHOW_KEYS:
    case SQLCOM_SHOW_VARIABLES:
    case SQLCOM_SHOW_STATUS:
    case SQLCOM_SHOW_ENGINE_LOGS:
    case SQLCOM_SHOW_ENGINE_STATUS:
    case SQLCOM_SHOW_ENGINE_MUTEX:
    case SQLCOM_SHOW_PROCESSLIST:
    case SQLCOM_SHOW_MASTER_STAT:
    case SQLCOM_SHOW_SLAVE_STAT:
    case SQLCOM_SHOW_GRANTS:
    case SQLCOM_SHOW_CREATE:
    case SQLCOM_SHOW_CHARSETS:
    case SQLCOM_SHOW_COLLATIONS:
    case SQLCOM_SHOW_CREATE_DB:
    case SQLCOM_SHOW_TABLE_STATUS:
    case SQLCOM_SHOW_TRIGGERS:
    case SQLCOM_SHOW_BINLOGS:
    case SQLCOM_SHOW_OPEN_TABLES:
    case SQLCOM_SHOW_SLAVE_HOSTS:
    case SQLCOM_SHOW_BINLOG_EVENTS:
    case SQLCOM_SHOW_WARNS:
    case SQLCOM_SHOW_ERRORS:
    case SQLCOM_SHOW_STORAGE_ENGINES:
    case SQLCOM_SHOW_PRIVILEGES:
    case SQLCOM_SHOW_CREATE_PROC:
    case SQLCOM_SHOW_CREATE_FUNC:
    case SQLCOM_SHOW_STATUS_PROC:
    case SQLCOM_SHOW_STATUS_FUNC:
    case SQLCOM_SHOW_PROC_CODE:
    case SQLCOM_SHOW_FUNC_CODE:
    case SQLCOM_SHOW_PLUGINS:
    case SQLCOM_SHOW_CREATE_EVENT:
    case SQLCOM_SHOW_EVENTS:
    case SQLCOM_SHOW_CREATE_TRIGGER:
    case SQLCOM_SHOW_PROFILE:
    case SQLCOM_SHOW_PROFILES:
    case SQLCOM_SHOW_RELAYLOG_EVENTS:
    case SQLCOM_SHOW_ENGINE_TRX:
    case SQLCOM_SHOW_MEMORY_STATUS:
      return IS_BIT_SET(admission_control_filter, ADMISSION_CONTROL_SHOW);

    case SQLCOM_CHANGE_DB:
      return IS_BIT_SET(admission_control_filter, ADMISSION_CONTROL_USE);

    default:
      return false;
  }
}

/**
 * Admit a query based on database entity
 *
 * @param thd THD structure
 * @param mode how to re-enter admission control
 *
 * @return 0 if the query is admitted, 1 otherwise
 */
int multi_tenancy_admit_query(THD *thd,
                              enum_admission_control_request_mode mode) {
  bool admission_check = false;

  // Waiting for admission while holding a mutex is prone to deadlocks
  // if all threads holding AC slots wait for this mutex.
  DBUG_ASSERT(!thd->current_mutex);

  // Return if THD is already in an admission control
  // (e.g. part of a multi query packet).
  if (thd->is_in_ac) return 0;

  /*
   * Admission control check will be enforced if ALL of the following
   * conditions are satisfied
   *  1. The query is run by regular (non-super) user
   *  2. The THD is not a replication thread
   *  3. The query is not part of a transaction (controlled by global var
   *     opt_admission_control_by_trx), nor the THD is already in an admission
   *     control (e.g. part of a multi query packet)
   *  4. AC entity is set for THD
   *  5. sys var max_running_queries > 0
   *  6. The command is not filtered by admission_control_filter
   */
  bool beginning_of_trx =
      !thd->get_transaction()->is_active(Transaction_ctx::SESSION);

  if (!thd->security_context()->check_access(SUPER_ACL) &&   /* 1 */
      !thd->rli_slave &&                                     /* 2 */
      (!opt_admission_control_by_trx || beginning_of_trx) && /* 3 */
      thd->ac_node && thd->ac_node->ac_info &&               /* 4 */
      db_ac->get_max_running_queries() &&                    /* 5 */
      !filter_command(thd->lex->sql_command)                 /* 6 */
  ) {
    admission_check = true;
  }

  if (admission_check) {
    Ac_result res = db_ac->admission_control_enter(thd, mode);
    if (res == Ac_result::AC_ABORTED) {
      my_error(ER_DB_ADMISSION_CONTROL, MYF(0),
               db_ac->get_max_waiting_queries(),
               thd->ac_node->ac_info->get_entity().c_str());
      return 1;
    } else if (res == Ac_result::AC_TIMEOUT) {
      my_error(ER_DB_ADMISSION_CONTROL_TIMEOUT, MYF(0),
               thd->ac_node->ac_info->get_entity().c_str());
      return 1;
    } else if (res == Ac_result::AC_KILLED) {
      return 1;
    }

    thd->is_in_ac = true;
  }

  return 0;
}

/**
 * Exit a query based on database entity
 *
 * @param thd THD structure
 *
 * @return 0 if successful, 1 if otherwise
 */
int multi_tenancy_exit_query(THD *thd) {
  DBUG_ASSERT(thd->is_in_ac);
  thd->is_in_ac = false;
  thd->last_yield_counter = thd->yield_counter;

  // is_in_ac shouldn't be set without ac_info.
  DBUG_ASSERT(thd->ac_node && thd->ac_node->ac_info);

  db_ac->admission_control_exit(thd);
  return 0;
}

/*
 * A wrapper around std::stoul that catches any exceptions to return an error
 * code instead. On success, val is populated with the output.
 *
 * @return 0 on success, -1 on error
 */
int stoul_noexcept(const std::string str, ulong *val) {
  try {
    size_t pos = 0;
    *val = std::stoul(str, &pos);
    // Check if at least 1 charater was read and that the whole string was
    // parsed completely.
    if (pos > 0 && pos == str.size()) {
      return 0;
    }
  } catch (const std::exception &e) {
    return -1;
  }

  return -1;
}

static ulong get_queue(THD *thd) {
  // To get queue name, we look at query attribute, connection attribute,
  // and then session variable in that order.
  return thd->get_query_or_connect_attr_value(
      "@@admission_control_queue",             // name
      thd->variables.admission_control_queue,  // default
      MAX_AC_QUEUES);                          // maximum
}

/**
 * Add a connection to admission control.
 *
 * @param thd THD structure
 *
 * @return 0 if the connection is added, 1 if rejected
 */
int multi_tenancy_add_connection(THD *thd, const char *new_db) {
  DBUG_ASSERT(!thd->ac_node || !thd->ac_node->ac_info);

  Ac_result result = Ac_result::AC_ADMITTED;

  // TODO: add accounting for connections without database, and for super.
  if (new_db && new_db[0] &&
      !thd->security_context()->check_access(SUPER_ACL)) {
    result = db_ac->add_connection(thd, new_db);
  }

  return result == Ac_result::AC_ADMITTED ? 0 : 1;
}

/**
 * Release a connection from admission control.
 *
 * @param thd THD structure
 *
 * @return 0 if successful, 1 if otherwise
 */
int multi_tenancy_close_connection(THD *thd) {
  if (!thd->ac_node || !thd->ac_node->ac_info) return 0;

  if (thd->is_in_ac) multi_tenancy_exit_query(thd);

  db_ac->close_connection(thd);
  return 0;
}

/**
 * Applies connection control checks for the entity.

 * @param thd THD structure.
 * @param new_db db to add connection for.
 * @return AC_ADMITTED - Admitted
 *         AC_ABORTED  - Rejected because limit is reached
 */
Ac_result AC::add_connection(THD *thd, const char *new_db) {
  Ac_result res = Ac_result::AC_ADMITTED;
  std::string entity = new_db;

  if (!thd->ac_node) {
    // Both THD and the admission control queue will share the object
    // created here.
    thd->ac_node = std::make_shared<st_ac_node>(thd);
  }

  DBUG_ASSERT(!thd->ac_node->ac_info);

  mysql_rwlock_rdlock(&LOCK_ac);
  auto it = ac_map.find(entity);
  if (it == ac_map.end()) {
    // New DB.
    mysql_rwlock_unlock(&LOCK_ac);
    insert(entity);
    mysql_rwlock_rdlock(&LOCK_ac);
    it = ac_map.find(entity);
  }

  // Entity could be removed right after it was inserted so double check.
  if (it != ac_map.end()) {
    auto ac_info = it->second;
    mysql_mutex_lock(&ac_info->lock);

    if (!max_connections || ac_info->connections < max_connections) {
      ++ac_info->connections;
      thd->ac_node->ac_info = ac_info;
    } else {
      ++ac_info->rejected_connections;
      ++total_rejected_connections;
      res = Ac_result::AC_ABORTED;
    }

    mysql_mutex_unlock(&ac_info->lock);
  }

  mysql_rwlock_unlock(&LOCK_ac);
  return res;
}

/**
 * @brief Releases connection for the current entity.

 * @param thd THD structure.
 */
void AC::close_connection(THD *thd) {
  auto ac_info = thd->ac_node->ac_info;
  DBUG_ASSERT(ac_info);

  mysql_mutex_lock(&ac_info->lock);

  DBUG_ASSERT(ac_info->connections > 0);
  --ac_info->connections;
  thd->ac_node->ac_info = nullptr;

  mysql_mutex_unlock(&ac_info->lock);
}

/**
 * @param thd THD structure.
 * @param mode how to re-enter admission control
 *
 * Applies admission control checks for the entity. Outline of
 * the steps in this function:
 * 1. Error out if we crossed the max waiting limit.
 * 2. Put the thd in a queue.
 * 3. If we crossed the max running limit then wait for signal from threads
 *    that completed their query execution.
 *
 * Note current implementation assumes the admission control entity is
 * database. We will lift the assumption and implement the entity logic in
 * multitenancy plugin.
 *
 * @return AC_ADMITTED - Admitted
 *         AC_ABORTED  - Rejected because queue size too large
 *         AC_TIMEOUT  - Rejected because waiting on queue for too long
 *         AC_KILLED   - Killed while waiting for admission
 */
Ac_result AC::admission_control_enter(
    THD *thd, enum_admission_control_request_mode mode) {
  Ac_result res = Ac_result::AC_ADMITTED;
  const char *prev_proc_info = thd->proc_info;
  THD_STAGE_INFO(thd, stage_admission_control_enter);
  // Unlock this before waiting.
  mysql_rwlock_rdlock(&LOCK_ac);
  if (max_running_queries) {
    auto ac_info = thd->ac_node->ac_info;
    thd->ac_node->queue = get_queue(thd);
    mysql_mutex_lock(&ac_info->lock);

    if (ac_info->running_queries < max_running_queries) {
      // We are below the max running limit.
      ++ac_info->running_queries;
      ++ac_info->queues[thd->ac_node->queue].running_queries;
      DBUG_ASSERT(!thd->ac_node->running);
      thd->ac_node->running = true;
    } else if (max_waiting_queries &&
               ac_info->waiting_queries >= max_waiting_queries) {
      ++ac_info->queues[thd->ac_node->queue].aborted_queries;
      ++ac_info->aborted_queries;
      ++total_aborted_queries;
      // We reached max waiting limit. Error out
      res = Ac_result::AC_ABORTED;
    } else {
      bool timeout;
      enqueue(thd, ac_info, mode);
      /**
        Inserting or deleting in std::map will not invalidate existing
        iterators except of course if the current iterator is erased. If the
        db corresponding to this iterator is getting dropped, these waiting
        queries are given signal to abort before the iterator
        is erased. See AC::remove().
        So, we don't need LOCK_ac here. The motivation to unlock the read lock
        is that waiting queries here shouldn't block other operations
        modifying ac_map or max_running_queries/max_waiting_queries.
      */
      while (true) {
        mysql_rwlock_unlock(&LOCK_ac);
        timeout = wait_for_signal(thd, thd->ac_node, ac_info, mode);
        // Retake locks in correct lock order.
        mysql_rwlock_rdlock(&LOCK_ac);
        mysql_mutex_lock(&ac_info->lock);
        // Break out if query has timed out, was killed, or has started running.
        // KILLs will also signal our condition variable (see THD::enter_cond
        // for how a cv is installed and THD::awake for how it is signaled).
        if (timeout || thd->killed || thd->ac_node->running) break;
      }

      // If a query has been successfully admitted, then it has already
      // been dequeued by a different thread, with updated counters.
      //
      // The only reason it might be still queued is during error conditions.
      if (thd->ac_node->queued) {
        DBUG_ASSERT(timeout || thd->killed);
        dequeue(thd, ac_info);
      }

      if (timeout || thd->killed) {
        // It's possible that we've gotten an error after this thread was
        // passed admission control. If so, reset the running counters.
        if (thd->ac_node->running) {
          DBUG_ASSERT(ac_info->running_queries > 0);
          --ac_info->running_queries;
          DBUG_ASSERT(ac_info->queues[thd->ac_node->queue].running_queries > 0);
          --ac_info->queues[thd->ac_node->queue].running_queries;
          thd->ac_node->running = false;
        }

        if (timeout) {
          ++total_timeout_queries;
          ++ac_info->queues[thd->ac_node->queue].timeout_queries;
          ++ac_info->timeout_queries;
          res = Ac_result::AC_TIMEOUT;
        } else {
          res = Ac_result::AC_KILLED;
        }
      }
    }

    mysql_mutex_unlock(&ac_info->lock);
  }

  mysql_rwlock_unlock(&LOCK_ac);
  thd->proc_info = prev_proc_info;
  return res;
}

/**
  Wait for admission control slots to free up, or until timeout.

  @return False No timeout
          True  Timeout occurred

*/
bool AC::wait_for_signal(THD *thd, st_ac_node_ptr &ac_node,
                         const Ac_info_ptr &ac_info,
                         enum_admission_control_request_mode mode) {
  PSI_stage_info old_stage;
  int res = 0;
  mysql_mutex_lock(&ac_node->lock);
  /**
    The locking order followed during admission_control_enter() is
    lock ac_info
    lock ac_node
    unlock ac_info
    unlock ac_node
    The locks are interleaved to avoid possible races which makes
    this waiting thread miss the signal from admission_control_exit().
  */
  mysql_mutex_unlock(&ac_info->lock);

  auto stage = (mode >= AC_REQUEST_QUERY_READMIT_LOPRI)
                   ? &stage_waiting_for_readmission
                   : &stage_waiting_for_admission;
  thd->ENTER_COND(&ac_node->cond, &ac_node->lock, stage, &old_stage);
  if (thd->variables.admission_control_queue_timeout == 0) {
    // Don't bother waiting if timeout is 0.
    res = ETIMEDOUT;
  } else if (thd->variables.admission_control_queue_timeout < 0) {
    // Spurious wake-ups are checked by callers of wait_for_signal.
    mysql_cond_wait(&ac_node->cond, &ac_node->lock);
  } else {
    struct timespec wait_timeout;
    set_timespec_nsec(&wait_timeout,
                      thd->variables.admission_control_queue_timeout * 1000000);

    res = mysql_cond_timedwait(&ac_node->cond, &ac_node->lock, &wait_timeout);
    DBUG_ASSERT(res == 0 || res == ETIMEDOUT);
  }

  mysql_mutex_unlock(&ac_node->lock);
  thd->EXIT_COND(&old_stage);
  return res == ETIMEDOUT;
}

/**
  @param thd THD structure
  @param attrs session resource attributes

  Signals one waiting thread. Pops out the first THD in the queue.
*/
void AC::admission_control_exit(THD *thd) {
  // AC::admission_control_enter admits query when max_running_queries is 0.
  // Nothing to do here in that case.
  if (!thd->ac_node->running) return;

  const char *prev_proc_info = thd->proc_info;
  THD_STAGE_INFO(thd, stage_admission_control_exit);

  auto ac_info = thd->ac_node->ac_info;
  mysql_rwlock_rdlock(&LOCK_ac);
  mysql_mutex_lock(&ac_info->lock);

  DBUG_ASSERT(ac_info->running_queries > 0);
  --ac_info->running_queries;
  DBUG_ASSERT(ac_info->queues[thd->ac_node->queue].running_queries > 0);
  --ac_info->queues[thd->ac_node->queue].running_queries;
  thd->ac_node->running = false;

  // Assert that max_running_queries == 0 implies no waiting queries.
  DBUG_ASSERT(max_running_queries != 0 || ac_info->waiting_queries == 0);

  // We determine here which queue to pick from. For every queue, we
  // calculate a score based on the number of running queries, and its
  // weight. Inituitively, the weight determines how much of the running
  // pool a queue is allowed to occupy. For example, if queue A has weight 3
  // and queue B has weight 7, the we expect 30% of the pool to have queries
  // running from A.
  //
  // We calculate a score for all queues that have waiting queries, and pick
  // the queue with the minimum score. In case of ties, we arbitrarily pick
  // the first encountered queue.
  if (ac_info->waiting_queries > 0 &&
      ac_info->running_queries < max_running_queries) {
    double min_score = std::numeric_limits<double>::max();
    ulong min_queue = 0;
#ifndef DBUG_OFF
    ulong running_queries_sum = 0;
    ulong waiting_queries_sum = 0;
#endif

    for (ulong i = 0; i < MAX_AC_QUEUES; i++) {
      const auto &queue = ac_info->queues[i];
#ifndef DBUG_OFF
      running_queries_sum += queue.running_queries;
      waiting_queries_sum += queue.waiting_queries();
#endif
      // Skip queues that don't have waiting queries.
      if (queue.waiting_queries() == 0) continue;

      double score = queue.running_queries / (weights[i] ? weights[i] : 1);

      if (score < min_score) {
        min_queue = i;
        min_score = score;
      }
    }

    DBUG_ASSERT(ac_info->waiting_queries == waiting_queries_sum);
    DBUG_ASSERT(ac_info->running_queries == running_queries_sum);

    auto &candidate = ac_info->queues[min_queue].queue.front();
    dequeue_and_run(candidate->thd, ac_info);
  }

  mysql_mutex_unlock(&ac_info->lock);
  mysql_rwlock_unlock(&LOCK_ac);
  thd->proc_info = prev_proc_info;
}

/*
 * @param thd THD
 * @param ac_info AC info
 * @param mode how to re-enter admission control
 *
 * Enqueues thd onto its queue. The queue is taken from thd->ac_node->queue.
 */
void AC::enqueue(THD *thd, Ac_info_ptr ac_info,
                 enum_admission_control_request_mode mode) {
  mysql_mutex_assert_owner(&ac_info->lock);
  auto &ac_node = thd->ac_node;

  ulong queue = ac_node->queue;
  if (mode == AC_REQUEST_QUERY_READMIT_HIPRI) {
    ac_info->queues[queue].queue.push_front(ac_node);
    ac_node->pos = ac_info->queues[queue].queue.begin();
  } else {
    ac_info->queues[queue].queue.push_back(ac_node);
    ac_node->pos = --ac_info->queues[queue].queue.end();
  }

  ac_node->queued = true;
  ++ac_info->waiting_queries;
}

/*
 * @param thd THD
 * @param ac_info AC info
 *
 * Dequeues thd from its queue. The queue is taken from thd->ac_node->queue.
 */
void AC::dequeue(THD *thd, Ac_info_ptr ac_info) {
  mysql_mutex_assert_owner(&ac_info->lock);
  auto &ac_node = thd->ac_node;

  DBUG_ASSERT(ac_node->queued);
  ac_info->queues[ac_node->queue].queue.erase(ac_node->pos);
  ac_node->queued = false;
  --ac_info->waiting_queries;
}

/*
 * @param thd THD
 * @param ac_info AC info
 *
 * Dequeues thd from its queue. Sets its state to running, and signals
 * that thread to start running.
 */
void AC::dequeue_and_run(THD *thd, Ac_info_ptr ac_info) {
  mysql_mutex_assert_owner(&ac_info->lock);

  dequeue(thd, ac_info);

  ++ac_info->running_queries;
  ++ac_info->queues[thd->ac_node->queue].running_queries;
  DBUG_ASSERT(!thd->ac_node->running);
  thd->ac_node->running = true;

  mysql_mutex_lock(&thd->ac_node->lock);
  mysql_cond_signal(&thd->ac_node->cond);
  mysql_mutex_unlock(&thd->ac_node->lock);
}

/*
 * @param s comma delimited string containing the weights
 *
 * Updates AC::weights based on the comma delimited string passed in.
 *
 * @returns -1 on failure
 *          0 on success.
 */
int AC::update_queue_weights(char *s) {
  auto v = split_into_vector(s ? s : "", ',');
  auto tmp = weights;

  if (v.size() > MAX_AC_QUEUES) {
    return -1;
  }

  for (ulong i = 0; i < std::min(MAX_AC_QUEUES, v.size()); i++) {
    ulong value = 0;
    if (!stoul_noexcept(v[i], &value) && value > 0 && value < LONG_MAX) {
      tmp[i] = (ulong)value;
    } else {
      return -1;
    }
  }
  mysql_rwlock_wrlock(&LOCK_ac);
  weights = tmp;
  mysql_rwlock_unlock(&LOCK_ac);

  return 0;
}

/**
 * @param entity The entity being dropped
 *
 * Removes a dropped entity info from the global map.
 * Connections could still remain pointing to ac_info
 * of this entity but since the entity is dropped these
 * connections are expected to move away to another
 * entity.
 */
void AC::remove(const char *entity) {
  std::string str(entity);
  // First take a read lock to unblock any waiting queries.
  mysql_rwlock_rdlock(&LOCK_ac);
  auto it = ac_map.find(str);
  if (it != ac_map.end()) {
    auto ac_info = it->second;
    mysql_mutex_lock(&ac_info->lock);
    for (auto &q : ac_info->queues) {
      while (q.waiting_queries() > 0) {
        dequeue_and_run(q.queue.front()->thd, ac_info);
      }
    }
    mysql_mutex_unlock(&ac_info->lock);
  }
  mysql_rwlock_unlock(&LOCK_ac);
  mysql_rwlock_wrlock(&LOCK_ac);
  it = ac_map.find(std::string(str));
  if (it != ac_map.end()) {
    ac_map.erase(it);
  }
  mysql_rwlock_unlock(&LOCK_ac);
}

/*
 * @param entity The entity being inserted
 *
 * Inserts an entity into the global map.
 */
void AC::insert(const std::string &entity) {
  mysql_rwlock_wrlock(&LOCK_ac);
  if (ac_map.find(entity) == ac_map.end()) {
    ac_map.emplace(entity, std::make_shared<Ac_info>(entity));
  }
  mysql_rwlock_unlock(&LOCK_ac);
}

/*
 * @param val Value of max_running_queries
 *
 * Updates max_running_queries, waking up any threads if necessary.
 */
void AC::update_max_running_queries(ulong val) {
  // lock to protect against erasing map iterators.
  mysql_rwlock_wrlock(&LOCK_ac);
  ulong old_val = max_running_queries;
  max_running_queries = val;
  // Signal any waiting threads which are below the new limit. Note 0 is a
  // special case where every waiting thread needs to be signalled.
  //
  // We don't kill any queries if the max is lowered, so it's possible for
  // the number of running queries temporarily exceed the new max.
  if (val > old_val || !val) {
    for (auto &it : ac_map) {
      auto &ac_info = it.second;
      mysql_mutex_lock(&ac_info->lock);
      size_t signaled = 0;
      // If we're not signaling all threads, we should dequeue according to
      // score for fairness, but we're not for simplicity. This might be
      // fine since update_max_running_queries should be a relatively rare
      // operation, so we're still fair in steady state.
      for (auto &q : ac_info->queues) {
        while (q.waiting_queries() > 0) {
          dequeue_and_run(q.queue.front()->thd, ac_info);
          if (val && ++signaled >= val - old_val) {
            break;
          }
        }
      }
      mysql_mutex_unlock(&ac_info->lock);
    }
  }
  mysql_rwlock_unlock(&LOCK_ac);
}

void AC::update_max_waiting_queries(ulong val) {
  mysql_rwlock_wrlock(&LOCK_ac);
  max_waiting_queries = val;
  mysql_rwlock_unlock(&LOCK_ac);
}

void AC::update_max_connections(ulong val) {
  mysql_rwlock_wrlock(&LOCK_ac);
  max_connections = val;
  mysql_rwlock_unlock(&LOCK_ac);
}

ulong AC::get_max_running_queries() const {
  mysql_rwlock_rdlock(&LOCK_ac);
  ulong res = max_running_queries;
  mysql_rwlock_unlock(&LOCK_ac);
  return res;
}

ulong AC::get_max_waiting_queries() const {
  mysql_rwlock_rdlock(&LOCK_ac);
  ulong res = max_waiting_queries;
  mysql_rwlock_unlock(&LOCK_ac);
  return res;
}

ulong AC::get_total_running_queries() const {
  ulonglong res = 0;
  mysql_rwlock_rdlock(&LOCK_ac);
  for (const auto &it : ac_map) {
    auto &ac_info = it.second;
    mysql_mutex_lock(&ac_info->lock);
    res += ac_info->running_queries;
    mysql_mutex_unlock(&ac_info->lock);
  }
  mysql_rwlock_unlock(&LOCK_ac);
  return res;
}

ulong AC::get_total_waiting_queries() const {
  ulonglong res = 0;
  mysql_rwlock_rdlock(&LOCK_ac);
  for (const auto &it : ac_map) {
    auto &ac_info = it.second;
    mysql_mutex_lock(&ac_info->lock);
    res += ac_info->waiting_queries;
    mysql_mutex_unlock(&ac_info->lock);
  }
  mysql_rwlock_unlock(&LOCK_ac);
  return res;
}

/**
 * @brief Populate admission_control_queue table.
 * @param thd THD
 * @param tables contains the TABLE struct to populate
 * @retval 0 on success
 * @retval 1 on failure
 */
int fill_ac_queue(THD *thd, TABLE_LIST *tables, Item *) {
  DBUG_ENTER("fill_ac_queue");
  TABLE *table = tables->table;

  mysql_rwlock_rdlock(&db_ac->LOCK_ac);
  for (const auto &pair : db_ac->ac_map) {
    const std::string &db = pair.first;
    const auto &ac_info = pair.second;
    mysql_mutex_lock(&ac_info->lock);
    for (ulong i = 0; i < MAX_AC_QUEUES; i++) {
      const auto &q = ac_info->queues[i];

      auto waiting = q.waiting_queries();
      auto running = q.running_queries;
      auto timeout = q.timeout_queries;
      auto aborted = q.aborted_queries;
      // Skip queues with no waiting/running queries.
      if (waiting == 0 && running == 0 && timeout == 0 && aborted == 0)
        continue;

      int f = 0;

      // SCHEMA_NAME
      table->field[f++]->store(db.c_str(), db.size(), system_charset_info);

      // QUEUE_ID
      table->field[f++]->store((ulonglong)i, true);

      // WAITING_QUERIES
      table->field[f++]->store((ulonglong)waiting, true);

      // RUNNING_QUERIES
      table->field[f++]->store((ulonglong)running, true);

      // ABORTED_QUERIES
      table->field[f++]->store((ulonglong)aborted, true);

      // TIMEOUT_QUERIES
      table->field[f++]->store((ulonglong)timeout, true);

      if (schema_table_store_record(thd, table)) {
        mysql_mutex_unlock(&ac_info->lock);
        mysql_rwlock_unlock(&db_ac->LOCK_ac);

        DBUG_RETURN(1);
      }
    }
    mysql_mutex_unlock(&ac_info->lock);
  }
  mysql_rwlock_unlock(&db_ac->LOCK_ac);

  DBUG_RETURN(0);
}

/**
 * @brief Populate admission_control_entities table.
 * @param thd THD
 * @param tables contains the TABLE struct to populate
 * @retval 0 on success
 * @retval 1 on failure
 */
int fill_ac_entities(THD *thd, TABLE_LIST *tables, Item *) {
  DBUG_ENTER("fill_ac_entities");
  TABLE *table = tables->table;
  int result = 0;

  mysql_rwlock_rdlock(&db_ac->LOCK_ac);
  for (const auto &pair : db_ac->ac_map) {
    const std::string &db = pair.first;
    const auto &ac_info = pair.second;

    mysql_mutex_lock(&ac_info->lock);
    ulonglong waiting = ac_info->waiting_queries;
    ulonglong running = ac_info->running_queries;
    ulonglong timeout = ac_info->timeout_queries;
    ulonglong aborted = ac_info->aborted_queries;
    ulonglong connections = ac_info->connections;
    ulonglong rejected_connections = ac_info->rejected_connections;
    mysql_mutex_unlock(&ac_info->lock);

    int f = 0;

    // SCHEMA_NAME
    table->field[f++]->store(db.c_str(), db.size(), system_charset_info);

    // WAITING_QUERIES
    table->field[f++]->store(waiting, true);

    // RUNNING_QUERIES
    table->field[f++]->store(running, true);

    // ABORTED_QUERIES
    table->field[f++]->store(aborted, true);

    // TIMEOUT_QUERIES
    table->field[f++]->store(timeout, true);

    // CONNECTIONS
    table->field[f++]->store(connections, true);

    // REJECTED_CONNECTIONS
    table->field[f++]->store(rejected_connections, true);

    if (schema_table_store_record(thd, table)) {
      result = 1;
      break;
    }
  }

  mysql_rwlock_unlock(&db_ac->LOCK_ac);

  DBUG_RETURN(result);
}

st_ac_node::st_ac_node(THD *thd_arg)
    : running(false), queued(false), queue(0), thd(thd_arg) {
  mysql_mutex_init(key_LOCK_ac_node, &lock, MY_MUTEX_INIT_FAST);
  mysql_cond_init(key_COND_ac_node, &cond);
}

st_ac_node::~st_ac_node() {
  mysql_mutex_destroy(&lock);
  mysql_cond_destroy(&cond);
}

Ac_info::Ac_info(const std::string &_entity) : entity(_entity) {
  mysql_mutex_init(key_LOCK_ac_info, &lock, MY_MUTEX_INIT_FAST);
}

Ac_info::~Ac_info() { mysql_mutex_destroy(&lock); }

const std::string &Ac_info::get_entity() const { return entity; }

AC::AC() { mysql_rwlock_init(key_rwlock_LOCK_ac, &LOCK_ac); }

AC::~AC() { mysql_rwlock_destroy(&LOCK_ac); }

/**
  @brief Constructor.
*/
Ac_switch_guard::Ac_switch_guard(THD *_thd) {
  thd = _thd;

  if (thd->is_in_ac) multi_tenancy_exit_query(thd);

  // Save ac_info from thd to prepare for switch. If switch fails,
  // it will be restored on thd.
  if (thd->ac_node) ac_info.swap(thd->ac_node->ac_info);
}

/**
  @brief Destructor commits or rolls back the switch.
*/
Ac_switch_guard::~Ac_switch_guard() {
  if (thd->ac_node) {
    auto &thd_ac_info = thd->ac_node->ac_info;

    // If switch succeeded then put old connection on thd to be closed;
    // if failed then close new one which is already on thd.
    if (committed) ac_info.swap(thd_ac_info);

    if (thd_ac_info) db_ac->close_connection(thd);

    DBUG_ASSERT(!thd_ac_info);

    // Now ac_info is either new connection in successful case, or old one
    // in failed case. In either case, restore it on thd.
    if (ac_info) ac_info.swap(thd_ac_info);
  }

  // Make sure the guard is not left with a connection.
  DBUG_ASSERT(!ac_info);
}

/**
  @brief Switch to new db.

  @param new_db Name of db to switch to.

  @return Error codes below.
  @retval 0 Switch is performed, or not needed.
  @retval 1 Switch failed.
*/
int Ac_switch_guard::add_connection(const char *new_db) {
  int ret = 0;

  // Switch is needed in the following cases:
  //  1. Old db was not set. Covers new connection as well.
  //  2. New db is not set. Need to close old connection anyway.
  //  3. New db is not the same as old db.
  //  4. Current user is super. If previous user wasn't super then
  //     old connection needs to be closed.
  bool new_db_empty = !new_db || !new_db[0];
  do_switch = !ac_info || new_db_empty ||
              strcmp(ac_info->get_entity().c_str(), new_db) ||
              thd->security_context()->check_access(SUPER_ACL);

  if (do_switch) {
    ret = multi_tenancy_add_connection(thd, new_db);
  }

  return ret;
}
