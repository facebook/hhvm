/* Copyright (c) 2014, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef RPL_SLAVE_COMMIT_ORDER_MANAGER
#define RPL_SLAVE_COMMIT_ORDER_MANAGER

#include <stddef.h>
#include <memory>
#include <vector>

#include "my_dbug.h"
#include "my_inttypes.h"
#include "mysql/components/services/mysql_cond_bits.h"
#include "mysql/components/services/mysql_mutex_bits.h"
#include "sql/rpl_rli_pdb.h"  // get_thd_worker

class Commit_order_manager {
 public:
  Commit_order_manager(uint32 worker_numbers);
  ~Commit_order_manager();

  /**
    Register the worker into commit order queue when coordinator dispatches a
    transaction to the worker.

    @param[in] worker The worker which the transaction will be dispatched to.
  */
  void register_trx(Slave_worker *worker);

 private:
  /**
    Wait for its turn to commit or unregister.

    @param[in] worker The worker which is executing the transaction.

    @retval false  All previous transactions succeed, so this transaction can
                   go ahead and commit.
    @retval true   One or more previous transactions rollback, so this
                   transaction should rollback.
  */
  bool wait(Slave_worker *worker);

  /**
    Unregister the thread from the commit order queue and signal
    the next thread to awake.

    @param[in] worker     The worker which is executing the transaction.
  */
  void finish_one(Slave_worker *worker);

  /**
    Unregister the transaction from the commit order queue and signal the next
    one to go ahead.

    @param[in] worker     The worker which is executing the transaction.
  */
  void finish(Slave_worker *worker);

  /**
    Reset server_status value of the commit group.

    @param[in] first_thd  The first thread of the commit group that needs
                          server_status to be updated.
  */
  void reset_server_status(THD *first_thd);

  /**
    Get rollback status.

    @retval true   Transactions in the queue should rollback.
    @retval false  Transactions in the queue shouldn't rollback.
  */
  bool get_rollback_status();

  /**
    Set rollback status to true.
  */
  void set_rollback_status();

  /**
    Unset rollback status to false.
  */
  void unset_rollback_status();

  void report_deadlock(Slave_worker *worker);

  enum class enum_transaction_stage { REGISTERED, WAITED, FINISHED };

  struct worker_info {
    uint32 next;
    mysql_cond_t cond;
    enum_transaction_stage stage;
  };

  mysql_mutex_t m_mutex;
  std::atomic<bool> m_rollback_trx;

  /* It stores order commit information of all workers. */
  std::vector<worker_info> m_workers;

  /*
    They are used to construct a transaction queue per DB with trx_info::next
    together.  both head and tail point to a slot of m_trx_vector, when the
    queue is not empty, otherwise their value are QUEUE_EOF.
  */
  std::unordered_map<std::string, uint32> queue_heads;
  std::unordered_map<std::string, uint32> queue_tails;
  static const uint32 QUEUE_EOF = 0xFFFFFFFF;

  uint32 queue_head(const std::string &db) const {
    mysql_mutex_assert_owner(&m_mutex);
    const auto elem = queue_heads.find(db);
    return (elem == queue_heads.end() ? QUEUE_EOF : elem->second);
  }

  uint32 queue_tail(const std::string &db) const {
    mysql_mutex_assert_owner(&m_mutex);
    const auto elem = queue_tails.find(db);
    return (elem == queue_tails.end() ? QUEUE_EOF : elem->second);
  }

  bool queue_empty(const std::string &db) const {
    mysql_mutex_assert_owner(&m_mutex);
    return queue_head(db) == QUEUE_EOF;
  }

  void queue_pop(const std::string &db) {
    mysql_mutex_assert_owner(&m_mutex);
    DBUG_ASSERT(!queue_empty(db));
    queue_heads[db] = m_workers[queue_head(db)].next;
    if (queue_head(db) == QUEUE_EOF) queue_tails[db] = QUEUE_EOF;
  }

  void queue_push(const std::string &db, uint32 index) {
    mysql_mutex_assert_owner(&m_mutex);
    DBUG_ASSERT(index < m_workers.size());
    if (queue_head(db) == QUEUE_EOF)
      queue_heads[db] = index;
    else
      m_workers[queue_tail(db)].next = index;
    queue_tails[db] = index;
    m_workers[index].next = QUEUE_EOF;
  }

  uint32 queue_front(const std::string &db) const {
    mysql_mutex_assert_owner(&m_mutex);
    return queue_head(db);
  }

  /**
    Flush record of transactions for all the waiting threads and then
    awake them from their wait. It also calls gtid_state->update_commit_group()
    which updates both the THD and the Gtid_state for whole commit group to
    reflect that the transaction set of transactions has ended.

    @param[in] worker  The worker which is executing the transaction.
  */
  void flush_engine_and_signal_threads(Slave_worker *worker);

 public:
  /**
     Check if order commit deadlock happens.

     Worker1(trx1)                     Worker2(trx2)
     =============                     =============
     ...                               ...
     Engine acquires lock A
     ...                               Engine acquires lock A(waiting for
                                       trx1 to release it.
     COMMIT(waiting for
     trx2 to commit first).

     Currently, there are two corner cases can cause the deadlock.
     - Case 1
       CREATE TABLE t1(c1 INT PRIMARY KEY, c2 INT, INDEX(c2)) ENGINE = InnoDB;
       INSERT INTO t1 VALUES(1, NULL),(2, 2), (3, NULL), (4, 4), (5, NULL), (6,
     6)

       INSERT INTO t1 VALUES(7, NULL);
       DELETE FROM t1 WHERE c2 <= 3;

     - Case 2
       ANALYZE TABLE t1;
       INSERT INTO t2 SELECT * FROM mysql.innodb_table_stats

     Since this is not a real lock deadlock, it could not be handled by engine.
     slave need to handle it separately.
     Worker1(trx1)                     Worker2(trx2)
     =============                     =============
     ...                               ...
     Engine acquires lock A
     ...                               Engine acquires lock A.
                                       1. found trx1 is holding the lock.
                                       2. report the lock wait to server code by
                                          calling thd_report_row_lock_wait().
                                          Then this function is called to check
                                          if it causes a order commit deadlock.
                                          Report the deadlock to worker1.
                                       3. waiting for trx1 to release it.
     COMMIT(waiting for
     trx2 to commit first).
     Found the deadlock flag set
     by worker2 and then
     return with ER_LOCK_DEADLOCK.

     Rollback the transaction
                                      Get lock A and go ahead.
                                      ...
     Retry the transaction

     To conclude, The transaction A which is waiting for transaction B to commit
     and is holding a lock which is required by transaction B will be rolled
     back and try again later.

     @param[in] thd_self     The THD object of self session which is acquiring
                             a lock hold by another session.
     @param[in] thd_wait_for The THD object of a session which is holding
                             a lock being acquired by current session.
  */
  static void check_and_report_deadlock(THD *thd_self, THD *thd_wait_for);

  /**
    Wait for its turn to commit or unregister.

    @param[in] thd  The THD object of current thread.

    @retval false  All previous transactions succeed, so this transaction can
                   go ahead and commit.
    @retval true   The transaction is marked to rollback.
  */
  static bool wait(THD *thd);

  /**
    Wait for its turn to unregister and signal the next one to go ahead. In case
    error happens while processing transaction, notify the following transaction
    to rollback.

    @param[in] thd    The THD object of current thread.
    @param[in] error  If true failure in transaction execution
  */
  static void wait_and_finish(THD *thd, bool error);

  /**
    Get transaction rollback status.

    @param[in] thd    The THD object of current thread.

    @retval true   Current transaction should rollback.
    @retval false  Current transaction shouldn't rollback.
  */
  static bool get_rollback_status(THD *thd);

  /**
    Unregister the thread from the commit order queue and signal
    the next thread to awake.

    @param[in] thd    The THD object of current thread.
  */
  static void finish_one(THD *thd);

  /**
    Determines whether current thread needs to wait for its turn to commit and
    unregister from the commit order queue. The sql commands ALTER TABLE, DROP
    TABLE, DROP DB, OPTIMIZE TABLE, ANALYZE TABLE and REPAIR TABLE are allowed
    to wait for its turn to commit and unregister from the commit order queue as
    exception in MYSQL_BIN_LOG::ordered_commit(), as these transactions have
    multiple commits and so not determined if the call is ending transaction.

    @param[in] thd  The THD object of current thread.

    @retval true   Allow thread to wait for it turn
    @retval false  Do not allow thread to wait for it turn
  */
  static bool wait_for_its_turn_before_flush_stage(THD *thd);
};

/**
  Determines whether current thread shall run the procedure here
  to check whether it waits for its turn (and when its turn comes
  unregister from the commit order queue).

  The sql commands ALTER TABLE, ANALYZE TABLE, DROP DB, DROP EVENT,
  DROP FUNCTION, DROP PROCEDURE, DROP TRIGGER, DROP TABLE, DROP VIEW,
  OPTIMIZE TABLE and REPAIR TABLE shall run this procedure here, as
  an exception, because these transactions have multiple intermediate
  commits. Therefore cannot predetermine when the last commit is
  done.

  @param[in] thd  The THD object of current thread.

  @retval false  Commit_order_manager object is not intialized
  @retval true   Commit_order_manager object is intialized
*/
bool has_commit_order_manager(THD *thd);

#endif /*RPL_SLAVE_COMMIT_ORDER_MANAGER*/
