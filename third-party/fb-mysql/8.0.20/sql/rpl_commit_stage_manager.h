/* Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef RPL_COMMIT_STAGE_MANAGER
#define RPL_COMMIT_STAGE_MANAGER

#include <atomic>
#include <utility>

#include "my_dbug.h"
#include "mysql/psi/mysql_cond.h"
#include "mysql/psi/mysql_mutex.h"
#include "sql/sql_class.h"
#include "thr_mutex.h"

class THD;

/**
  Class for maintaining the commit stages for binary log group commit.
 */
class Commit_stage_manager {
 public:
  class Mutex_queue {
    friend class Commit_stage_manager;

   public:
    Mutex_queue() : m_first(nullptr), m_last(&m_first), m_size(0) {}

    void init(mysql_mutex_t *lock) { m_lock = lock; }

    bool is_empty() const { return m_first == nullptr; }

    /**
      Append a linked list of threads to the queue.

      @param[in]  first  Linked list of threads to be appended to queue

      @retval true The queue was empty before this operation.
      @retval false The queue was non-empty before this operation.
    */
    bool append(THD *first);

    /**
      Fetch the entire queue for a stage. It is a wrapper over
      fetch_and_empty() and acquires queue lock before fetching
      and emptying the queue threads.

      @return  Pointer to the first session of the queue.
    */
    THD *fetch_and_empty_acquire_lock();

    /**
      Fetch the entire queue for a stage. It is a wrapper over
      fetch_and_empty(). The caller must acquire queue lock before
      calling this function.

      @return  Pointer to the first session of the queue.
    */
    THD *fetch_and_empty_skip_acquire_lock();

    /**
      Remove first member from the queue

      @retval  Returns std::pair<bool, THD *> object.
               The first boolean value of pair if true determines queue
               is not empty, and false determines queue is empty.
               The second value returns the first removed member.
    */
    std::pair<bool, THD *> pop_front();

    /**
      Get number of elements in the queue.

      @retval  Returns number of element in the queue.
    */
    inline int32 get_size() { return m_size.load(); }

    /**
      Fetch the first thread of the queue.

      @return first thread of the queue.
    */
    THD *get_leader() { return m_first; }

    void lock() {
      mysql_mutex_assert_not_owner(m_lock);
      mysql_mutex_lock(m_lock);
    }

    void unlock() { mysql_mutex_unlock(m_lock); }

    void assert_owner() { mysql_mutex_assert_owner(m_lock); }

   private:
    /**
      Fetch the entire queue for a stage.

      @retval  This will fetch the entire queue in one go.
    */
    THD *fetch_and_empty();

    /**
       Pointer to the first thread in the queue, or nullptr if the queue is
       empty.
    */
    THD *m_first;

    /**
       Pointer to the location holding the end of the queue.

       This is either @c &first, or a pointer to the @c next_to_commit of
       the last thread that is enqueued.
    */
    THD **m_last;

    /** size of the queue */
    std::atomic<int32> m_size;

    /** Lock for protecting the queue. */
    mysql_mutex_t *m_lock;

    /*
      This attribute did not have the desired effect, at least not according
      to -fsanitize=undefined with gcc 5.2.1
     */
  };  // MY_ATTRIBUTE((aligned(CPU_LEVEL1_DCACHE_LINESIZE)));

 private:
  Commit_stage_manager() : m_is_initialized(false) {}

  Commit_stage_manager(const Commit_stage_manager &) = delete;

  const Commit_stage_manager &operator=(const Commit_stage_manager &) = delete;

 public:
  /**
    Fetch Commit_stage_manager class instance.

    @return Reference to the Commit_stage_manager class instance.
  */
  static Commit_stage_manager &get_instance();

  /**
     Constants for queues for different stages.
   */
  enum StageID {
    BINLOG_FLUSH_STAGE,
    SYNC_STAGE,
    COMMIT_STAGE,
    COMMIT_ORDER_FLUSH_STAGE,
    STAGE_COUNTER
  };

  /**
    Initializes m_stage_cond_binlog, m_stage_cond_commit_order,
    m_stage_cond_leader condition variables and m_lock_done mutex.

    The binlog follower threads blocks on m_stage_cond_binlog condition
    variable till signalled to wake up from leader thread. And similarly
    commit order follower threads blocks on m_stage_cond_commit_order
    condition variable till signalled to wake up from leader thread.

    The first binlog thread supposed to be leader finds that commit order queue
    is not empty then it blocks on m_stage_cond_leader till commit order leader
    signals it to awake and become new leader.

    m_lock_done mutex is shared by all three stages.

    @param key_LOCK_flush_queue mutex instrumentation key
    @param key_LOCK_sync_queue mutex instrumentation key
    @param key_LOCK_commit_queue mutex instrumentation key
    @param key_LOCK_done mutex instrumentation key
    @param key_COND_done cond instrumentation key
  */
  void init(PSI_mutex_key key_LOCK_flush_queue,
            PSI_mutex_key key_LOCK_sync_queue,
            PSI_mutex_key key_LOCK_commit_queue, PSI_mutex_key key_LOCK_done,
            PSI_cond_key key_COND_done);

  /**
    Deinitializes m_stage_cond_binlog, m_stage_cond_commit_order,
    m_stage_cond_leader condition variables and m_lock_done mutex.
  */
  void deinit();

  /**
    Enroll a set of sessions for a stage.

    This will queue the session thread for writing and flushing.

    If the thread being queued is assigned as stage leader, it will
    return immediately.

    If wait_if_follower is true the thread is not the stage leader,
    the thread will be wait for the queue to be processed by the
    leader before it returns.
    In DBUG-ON version the follower marks is preempt status as ready.

    The sesssion threads entering this function acquires mutexes, and few of
    them are not released while exiting based on thread and stage type.
    - A binlog leader (returning true when stage!=COMMIT_ORDER_FLUSH_STAGE) will
      acquire the stage mutex in this function and not release it.
    - A commit order leader of the flush stage (returning true when
      stage==COMMIT_ORDER_FLUSH_STAGE) will acquire both the stage mutex and the
      flush queue mutex in this function, and not release them.
    - A follower (returning false) will release any mutexes it takes, before
      returning from the function.

    @param[in] stage Stage identifier for the queue to append to.
    @param[in] first Queue to append.
    @param[in] stage_mutex
                 Pointer to the currently held stage mutex, or nullptr if we're
                 not in a stage, that will be released when changing stage.
    @param[in] enter_mutex
                 Pointer to the mutex that will be taken when changing stage.

    @retval true  Thread is stage leader.
    @retval false Thread was not stage leader and processing has been done.
   */
  bool enroll_for(StageID stage, THD *first, mysql_mutex_t *stage_mutex,
                  mysql_mutex_t *enter_mutex);

  /**
    Remove first member from the queue for given stage

    @retval  Returns std::pair<bool, THD *> object.
             The first boolean value of pair if true determines queue
             is not empty, and false determines queue is empty.
             The second value returns the first removed member.
  */
  std::pair<bool, THD *> pop_front(StageID stage) {
    return m_queue[stage].pop_front();
  }

#ifndef DBUG_OFF
  /**
     The method ensures the follower's execution path can be preempted
     by the leader's thread.
     Preempt status of @c head follower is checked to engange the leader
     into waiting when set.

     @param head  THD* of a follower thread
  */
  void clear_preempt_status(THD *head);
#endif

  /**
    Fetch the entire queue and empty it. It acquires queue lock before fetching
    and emptying the queue threads.

    @param[in]  stage             Stage identifier for the queue to append to.

    @return Pointer to the first session of the queue.
  */
  THD *fetch_queue_acquire_lock(StageID stage);

  /**
    Fetch the entire queue and empty it. The caller must acquire queue lock
    before calling this function.

    @param[in]  stage             Stage identifier for the queue to append to.

    @return Pointer to the first session of the queue.
  */
  THD *fetch_queue_skip_acquire_lock(StageID stage);

  /**
    Introduces a wait operation on the executing thread.  The
    waiting is done until the timeout elapses or count is
    reached (whichever comes first).

    If count == 0, then the session will wait until the timeout
    elapses. If timeout == 0, then there is no waiting.

    @param usec     the number of microseconds to wait.
    @param count    wait for as many as count to join the queue the
                    session is waiting on
    @param stage    which stage queue size to compare count against.
   */
  void wait_count_or_timeout(ulong count, long usec, StageID stage);

  /**
    The function is called after follower thread are processed by leader,
    to unblock follower threads.

    @param queue   the thread list which needs to ne unblocked
    @param stage   Stage identifier current thread belong to.
  */
  void signal_done(THD *queue, StageID stage = BINLOG_FLUSH_STAGE);

  /**
    This function gets called after transactions are flushed to the engine
    i.e. after calling ha_flush_logs, to unblock commit order thread list
    which are not needed to wait for other stages.

    @param first     the thread list which needs to ne unblocked
  */
  void process_final_stage_for_ordered_commit_group(THD *first);

  /**
    Wrapper on Mutex_queue lock(), acquires lock on stage queue.

    @param[in]  stage  Stage identifier for the queue to append to.
  */
  void lock_queue(StageID stage) { m_queue[stage].lock(); }

  /**
    Wrapper on Mutex_queue unlock(), releases lock on stage queue.

    @param[in]  stage  Stage identifier for the queue to append to.
  */
  void unlock_queue(StageID stage) { m_queue[stage].unlock(); }

 private:
  /** check if Commit_stage_manager variables already initalized. */
  bool m_is_initialized;

  /**
     Queues for sessions.

     We need four queues:
     - Binlog flush queue: transactions that are going to be flushed to the
                           engine and written to the binary log.
     - Commit order flush queue: transactions that are not going to write the
                                 binlog at all, but participate in the beginning
                                 of the group commit, up to and including the
                                 engine flush.
     - Sync queue: transactions that are going to be synced to disk
     - Commit queue: transactions that are going to to be committed
                     (when binlog_order_commit=1).
  */
  Mutex_queue m_queue[STAGE_COUNTER];

  /**
     The binlog leader waits on this condition variable till it is indicated
     to wake up. If binlog flush queue gets first thread in the queue but
     by then commit order flush queue has already elected leader. The the
     first thread of binlog queue waits on this condition variable and get
     signalled to wake up from commit order flush queue leader later.
  */
  mysql_cond_t m_stage_cond_leader;

  /**
     Condition variable to indicate that the binlog threads can wake up
     and continue.
  */
  mysql_cond_t m_stage_cond_binlog;

  /**
     Condition variable to indicate that the flush to storage engine
     is done and commit order threads can again wake up and continue.
  */
  mysql_cond_t m_stage_cond_commit_order;

  /** Mutex used for the condition variable above */
  mysql_mutex_t m_lock_done;

  /** Mutex used for the stage level locks */
  mysql_mutex_t m_queue_lock[STAGE_COUNTER - 1];

#ifndef DBUG_OFF
  /** Save pointer to leader thread which is used later to awake leader */
  THD *leader_thd;

  /** Flag is set by Leader when it starts waiting for follower's all-clear */
  bool leader_await_preempt_status;

  /** Condition variable to indicate a follower started waiting for commit */
  mysql_cond_t m_cond_preempt;
#endif
};

#endif /*RPL_COMMIT_STAGE_MANAGER*/
