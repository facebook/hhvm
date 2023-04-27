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

   Without limiting anything contained in the foregoing, this file,
   which is part of C Driver for MySQL (Connector/C), is also subject to the
   Universal FOSS Exception, version 1.0, a copy of which can be found at
   http://oss.oracle.com/licenses/universal-foss-exception.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @file mysys/thr_lock.cc
Read and write locks for Posix threads. All tread must acquire
all locks it needs through thr_multi_lock_nsec() to avoid dead-locks.
A lock consists of a master lock (THR_LOCK), and lock instances
(THR_LOCK_DATA).
Any thread can have any number of lock instances (read and write:s) on
any lock. All lock instances must be freed.
Locks are prioritized according to:

The current lock types are:

TL_READ	 		# Low priority read
TL_READ_WITH_SHARED_LOCKS
TL_READ_HIGH_PRIORITY	# High priority read
TL_READ_NO_INSERT	# Read without concurrent inserts
TL_WRITE_ALLOW_WRITE	# Write lock that allows other writers
TL_WRITE_CONCURRENT_INSERT
                        # Insert that can be mixed when selects
                        # Allows lower locks to take over
TL_WRITE_LOW_PRIORITY	# Low priority write
TL_WRITE		# High priority write
TL_WRITE_ONLY		# High priority write
                        # Abort all new lock request with an error

Locks are prioritized according to:

WRITE_ALLOW_WRITE, WRITE_CONCURRENT_INSERT, WRITE_LOW_PRIORITY, READ,
WRITE, READ_HIGH_PRIORITY and WRITE_ONLY

Locks in the same privilege level are scheduled in first-in-first-out order.

To allow concurrent read/writes locks, with 'WRITE_CONCURRENT_INSERT' one
should put a pointer to the following functions in the lock structure:
(If the pointer is zero (default), the function is not called)

check_status:
         Before giving a lock of type TL_WRITE_CONCURRENT_INSERT,
         we check if this function exists and returns 0.
         If not, then the lock is upgraded to TL_WRITE_LOCK
         In MyISAM this is a simple check if the insert can be done
         at the end of the datafile.
update_status:
        Before a write lock is released, this function is called.
        In MyISAM this functions updates the count and length of the datafile
get_status:
        When one gets a lock this functions is called.
        In MyISAM this stores the number of rows and size of the datafile
        for concurrent reads.

The lock algorithm allows one to have one TL_WRITE_CONCURRENT_INSERT
lock at the same time as multiple read locks.
*/

#include "my_config.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_list.h"
#include "my_macros.h"
#include "my_sys.h"
#include "my_systime.h"
#include "my_thread.h"
#include "my_thread_local.h"
#include "mysql/psi/mysql_cond.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/psi/mysql_table.h"
#include "mysql/psi/mysql_thread.h"
#include "mysql/psi/psi_stage.h"
#include "mysql/psi/psi_table.h"
#include "mysys/mysys_priv.h"
#include "template_utils.h"
#include "thr_lock.h"
#include "thr_mutex.h"

ulong locks_immediate = 0L, locks_waited = 0L;
enum thr_lock_type thr_upgraded_concurrent_insert_lock = TL_WRITE;

/* The following constants are only for debug output */
#define MAX_THREADS 100
#define MAX_LOCKS 100

LIST *thr_lock_thread_list; /* List of threads in use */
ulong max_write_lock_count = ~(ulong)0L;

static void (*before_lock_wait)(void) = nullptr;
static void (*after_lock_wait)(void) = nullptr;

void thr_set_lock_wait_callback(void (*before_wait)(void),
                                void (*after_wait)(void)) {
  before_lock_wait = before_wait;
  after_lock_wait = after_wait;
}

static inline bool thr_lock_owner_equal(THR_LOCK_INFO *rhs,
                                        THR_LOCK_INFO *lhs) {
  return rhs == lhs;
}

#ifdef EXTRA_DEBUG
#define MAX_FOUND_ERRORS 10 /* Report 10 first errors */
static uint found_errors = 0;

static int check_lock(struct st_lock_list *list, const char *lock_type,
                      const char *where, bool same_owner, bool no_cond) {
  THR_LOCK_DATA *data, **prev;
  uint count = 0;
  THR_LOCK_INFO *first_owner = NULL;

  prev = &list->data;
  if (list->data) {
    enum thr_lock_type last_lock_type = list->data->type;

    if (same_owner && list->data) first_owner = list->data->owner;
    for (data = list->data; data && count++ < MAX_LOCKS; data = data->next) {
      if (data->type != last_lock_type) last_lock_type = TL_IGNORE;
      if (data->prev != prev) {
        fprintf(stderr,
                "prev link %d didn't point at "
                "previous lock at %s: %s",
                count, lock_type, where);
        return 1;
      }
      if (same_owner && !thr_lock_owner_equal(data->owner, first_owner) &&
          last_lock_type != TL_WRITE_ALLOW_WRITE) {
        fprintf(stderr,
                "Found locks from different threads "
                "in %s: %s",
                lock_type, where);
        return 1;
      }
      if (no_cond && data->cond) {
        fprintf(stderr,
                "Found active lock with not reset "
                "cond %s: %s",
                lock_type, where);
        return 1;
      }
      prev = &data->next;
    }
    if (data) {
      fprintf(stderr, "found too many locks at %s: %s", lock_type, where);
      return 1;
    }
  }
  if (prev != list->last) {
    fprintf(stderr, "last didn't point at last lock at %s: %s", lock_type,
            where);
    return 1;
  }
  return 0;
}

static void check_locks(THR_LOCK *lock, const char *where,
                        bool allow_no_locks) {
  uint old_found_errors = found_errors;
  DBUG_TRACE;

  if (found_errors < MAX_FOUND_ERRORS) {
    if (check_lock(&lock->write, "write", where, 1, 1) |
        check_lock(&lock->write_wait, "write_wait", where, 0, 0) |
        check_lock(&lock->read, "read", where, 0, 1) |
        check_lock(&lock->read_wait, "read_wait", where, 0, 0))
      found_errors++;

    if (found_errors < MAX_FOUND_ERRORS) {
      uint count = 0;
      THR_LOCK_DATA *data;
      for (data = lock->read.data; data; data = data->next) {
        if ((int)data->type == (int)TL_READ_NO_INSERT) count++;
        /* Protect against infinite loop. */
        DBUG_ASSERT(count <= lock->read_no_write_count);
      }
      if (count != lock->read_no_write_count) {
        found_errors++;
        fprintf(stderr,
                "at '%s': Locks read_no_write_count "
                "was %u when it should have been %u",
                where, lock->read_no_write_count, count);
      }

      if (!lock->write.data) {
        if (!allow_no_locks && !lock->read.data &&
            (lock->write_wait.data || lock->read_wait.data)) {
          found_errors++;
          fprintf(stderr,
                  "at '%s': No locks in use but locks "
                  "are in wait queue",
                  where);
        }
        if (!lock->write_wait.data) {
          if (!allow_no_locks && lock->read_wait.data) {
            found_errors++;
            fprintf(stderr,
                    "at '%s': No write locks and "
                    "waiting read locks",
                    where);
          }
        } else {
          if (!allow_no_locks &&
              (((lock->write_wait.data->type == TL_WRITE_CONCURRENT_INSERT ||
                 lock->write_wait.data->type == TL_WRITE_ALLOW_WRITE) &&
                !lock->read_no_write_count))) {
            found_errors++;
            fprintf(stderr,
                    "at '%s': Write lock %d waiting "
                    "while no exclusive read locks",
                    where, (int)lock->write_wait.data->type);
          }
        }
      } else { /* Have write lock */
        if (lock->write_wait.data) {
          if (!allow_no_locks &&
              lock->write.data->type == TL_WRITE_ALLOW_WRITE &&
              lock->write_wait.data->type == TL_WRITE_ALLOW_WRITE) {
            found_errors++;
            fprintf(stderr,
                    "at '%s': Found WRITE_ALLOW_WRITE "
                    "lock waiting for WRITE_ALLOW_WRITE lock",
                    where);
          }
        }
        if (lock->read.data) {
          if (!thr_lock_owner_equal(lock->write.data->owner,
                                    lock->read.data->owner) &&
              ((lock->write.data->type > TL_WRITE_CONCURRENT_INSERT &&
                lock->write.data->type != TL_WRITE_ONLY) ||
               ((lock->write.data->type == TL_WRITE_CONCURRENT_INSERT ||
                 lock->write.data->type == TL_WRITE_ALLOW_WRITE) &&
                lock->read_no_write_count))) {
            found_errors++;
            fprintf(stderr,
                    "at '%s': Found lock of type %d "
                    "that is write and read locked",
                    where, lock->write.data->type);
            DBUG_PRINT("warning", ("At '%s': Found lock of type %d that is "
                                   "write and read locked\n",
                                   where, lock->write.data->type));
          }
        }
        if (lock->read_wait.data) {
          if (!allow_no_locks &&
              lock->write.data->type <= TL_WRITE_CONCURRENT_INSERT &&
              lock->read_wait.data->type <= TL_READ_HIGH_PRIORITY) {
            found_errors++;
            fprintf(stderr,
                    "at '%s': Found read lock of "
                    "type %d waiting for write lock of type %d",
                    where, (int)lock->read_wait.data->type,
                    (int)lock->write.data->type);
          }
        }
      }
    }
    if (found_errors != old_found_errors) {
      DBUG_PRINT("error", ("Found wrong lock"));
    }
  }
}

#else /* EXTRA_DEBUG */
#define check_locks(A, B, C)
#endif

/* Initialize a lock */

void thr_lock_init(THR_LOCK *lock) {
  DBUG_TRACE;
  new (lock) THR_LOCK();

  mysql_mutex_init(key_THR_LOCK_mutex, &lock->mutex, MY_MUTEX_INIT_FAST);
  lock->read.last = &lock->read.data;
  lock->read_wait.last = &lock->read_wait.data;
  lock->write_wait.last = &lock->write_wait.data;
  lock->write.last = &lock->write.data;

  mysql_mutex_lock(&THR_LOCK_lock); /* Add to locks in use */
  lock->list.data = (void *)lock;
  thr_lock_thread_list = list_add(thr_lock_thread_list, &lock->list);
  mysql_mutex_unlock(&THR_LOCK_lock);
}

void thr_lock_delete(THR_LOCK *lock) {
  DBUG_TRACE;
  mysql_mutex_lock(&THR_LOCK_lock);
  thr_lock_thread_list = list_delete(thr_lock_thread_list, &lock->list);
  mysql_mutex_unlock(&THR_LOCK_lock);
  mysql_mutex_destroy(&lock->mutex);
}

void thr_lock_info_init(THR_LOCK_INFO *info, my_thread_id thread_id,
                        mysql_cond_t *suspend) {
  info->thread_id = thread_id;
  info->suspend = suspend;
}

/* Initialize a lock instance */

void thr_lock_data_init(THR_LOCK *lock, THR_LOCK_DATA *data, void *param) {
  data->lock = lock;
  data->type = TL_UNLOCK;
  data->owner = nullptr; /* no owner yet */
  data->status_param = param;
  data->cond = nullptr;
}

static inline bool has_old_lock(THR_LOCK_DATA *data, THR_LOCK_INFO *owner) {
  for (; data; data = data->next) {
    if (thr_lock_owner_equal(data->owner, owner))
      return true; /* Already locked by thread */
  }
  return false;
}

static void wake_up_waiters(THR_LOCK *lock);

static enum enum_thr_lock_result wait_for_lock_nsec(
    struct st_lock_list *wait, THR_LOCK_DATA *data, THR_LOCK_INFO *owner,
    bool in_wait_list, ulonglong lock_wait_timeout_nsec) {
  struct timespec wait_timeout;
  enum enum_thr_lock_result result = THR_LOCK_ABORTED;
  PSI_stage_info old_stage;
  DBUG_TRACE;

  /*
    One can use this to signal when a thread is going to wait for a lock.
    See debug_sync.cc.

    Beware of waiting for a signal here. The lock has aquired its mutex.
    While waiting on a signal here, the locking thread could not aquire
    the mutex to release the lock. One could lock up the table
    completely.

    In detail it works so: When thr_lock() tries to acquire a table
    lock, it locks the lock->mutex, checks if it can have the lock, and
    if not, it calls wait_for_lock(). Here it unlocks the table lock
    while waiting on a condition. The sync point is located before this
    wait for condition. If we have a waiting action here, we hold the
    the table locks mutex all the time. Any attempt to look at the table
    lock by another thread blocks it immediately on lock->mutex. This
    can easily become an unexpected and unobvious blockage. So be
    warned: Do not request a WAIT_FOR action for the 'wait_for_lock'
    sync point unless you really know what you do.
  */
  DEBUG_SYNC_C("wait_for_lock");

  if (!in_wait_list) {
    (*wait->last) = data; /* Wait for lock */
    data->prev = wait->last;
    wait->last = &data->next;
  }

  locks_waited++;

  /* Set up control struct to allow others to abort locks */
  data->cond = owner->suspend;

  enter_cond_hook(nullptr, data->cond, &data->lock->mutex,
                  &stage_waiting_for_table_level_lock, &old_stage, __func__,
                  __FILE__, __LINE__);

  /*
    Since before_lock_wait potentially can create more threads to
    scheduler work for, we don't want to call the before_lock_wait
    callback unless it will really start to wait.

    For similar reasons, we do not want to call before_lock_wait and
    after_lock_wait for each lap around the loop, so we restrict
    ourselves to call it before_lock_wait once before starting to wait
    and once after the thread has exited the wait loop.
   */
  if ((!is_killed_hook(nullptr) || in_wait_list) && before_lock_wait)
    (*before_lock_wait)();

  set_timespec_nsec(&wait_timeout, lock_wait_timeout_nsec);
  while (!is_killed_hook(nullptr) || in_wait_list) {
    int rc =
        mysql_cond_timedwait(data->cond, &data->lock->mutex, &wait_timeout);
    /*
      We must break the wait if one of the following occurs:
      - the connection has been aborted (!is_killed_hook()),
      - the lock has been granted (data->cond is set to NULL by the granter),
        or the waiting has been aborted (additionally data->type is set to
        TL_UNLOCK).
      - the wait has timed out (rc == ETIMEDOUT)
      Order of checks below is important to not report about timeout
      if the predicate is true.
    */
    if (data->cond == nullptr) {
      DBUG_PRINT("thr_lock", ("lock granted/aborted"));
      break;
    }
    if (is_timeout(rc)) {
      /* purecov: begin inspected */
      DBUG_PRINT("thr_lock", ("lock timed out"));
      result = THR_LOCK_WAIT_TIMEOUT;
      break;
      /* purecov: end */
    }
  }

  /*
    We call the after_lock_wait callback once the wait loop has
    finished.
   */
  if (after_lock_wait) (*after_lock_wait)();

  if (data->cond || data->type == TL_UNLOCK) {
    if (data->cond) /* aborted or timed out */
    {
      if (((*data->prev) = data->next)) /* remove from wait-list */
        data->next->prev = data->prev;
      else
        wait->last = data->prev;
      data->type = TL_UNLOCK; /* No lock */
      check_locks(data->lock, "killed or timed out wait_for_lock", 1);
      wake_up_waiters(data->lock);
    } else {
      DBUG_PRINT("thr_lock", ("lock aborted"));
      check_locks(data->lock, "aborted wait_for_lock", 0);
    }
  } else {
    result = THR_LOCK_SUCCESS;
    if (data->lock->get_status)
      (*data->lock->get_status)(data->status_param, 0);
    check_locks(data->lock, "got wait_for_lock", 0);
  }
  mysql_mutex_unlock(&data->lock->mutex);

  exit_cond_hook(nullptr, &old_stage, __func__, __FILE__, __LINE__);

  return result;
}

enum enum_thr_lock_result thr_lock_nsec(THR_LOCK_DATA *data,
                                        THR_LOCK_INFO *owner,
                                        enum thr_lock_type lock_type,
                                        ulonglong lock_wait_timeout_nsec) {
  THR_LOCK *lock = data->lock;
  enum enum_thr_lock_result result = THR_LOCK_SUCCESS;
  struct st_lock_list *wait_queue;
  MYSQL_TABLE_WAIT_VARIABLES(locker, state) /* no ';' */
  DBUG_TRACE;

  data->next = nullptr;
  data->cond = nullptr; /* safety */
  data->type = lock_type;
  data->owner = owner; /* Must be reset ! */

  MYSQL_START_TABLE_LOCK_WAIT(locker, &state, data->m_psi, PSI_TABLE_LOCK,
                              lock_type);

  mysql_mutex_lock(&lock->mutex);
  DBUG_PRINT("lock", ("data: %p  thread: 0x%x  lock: %p  type: %d", data,
                      data->owner->thread_id, lock, (int)lock_type));
  check_locks(lock,
              (uint)lock_type <= (uint)TL_READ_NO_INSERT ? "enter read_lock"
                                                         : "enter write_lock",
              0);
  if ((int)lock_type <= (int)TL_READ_NO_INSERT) {
    /* Request for READ lock */
    if (lock->write.data) {
      /*
        We can allow a read lock even if there is already a
        write lock on the table if they are owned by the same
        thread or if they satisfy the following lock
        compatibility matrix:

           Request
          /-------
         H|++++  WRITE_ALLOW_WRITE
         e|+++-  WRITE_CONCURRENT_INSERT
         l ||||
         d ||||
           |||\= READ_NO_INSERT
           ||\ = READ_HIGH_PRIORITY
           |\  = READ_WITH_SHARED_LOCKS
           \   = READ

        + = Request can be satisified.
        - = Request cannot be satisified.

        READ_NO_INSERT and WRITE_ALLOW_WRITE should in principle
        be incompatible. Before this could have caused starvation of
        LOCK TABLE READ in InnoDB under high write load. However
        now READ_NO_INSERT is only used for LOCK TABLES READ and this
        statement is handled by the MDL subsystem.
        See Bug#42147 for more information.
      */

      DBUG_PRINT("lock", ("write locked 1 by thread: 0x%x",
                          lock->write.data->owner->thread_id));
      if (thr_lock_owner_equal(data->owner, lock->write.data->owner) ||
          (lock->write.data->type < TL_WRITE_CONCURRENT_INSERT) ||
          ((lock->write.data->type == TL_WRITE_CONCURRENT_INSERT) &&
           ((int)lock_type <=
            (int)TL_READ_HIGH_PRIORITY))) { /* Already got a write lock */
        (*lock->read.last) = data;          /* Add to running FIFO */
        data->prev = lock->read.last;
        lock->read.last = &data->next;
        if (lock_type == TL_READ_NO_INSERT) lock->read_no_write_count++;
        check_locks(lock, "read lock with old write lock", 0);
        if (lock->get_status) (*lock->get_status)(data->status_param, 0);
        locks_immediate++;
        goto end;
      }
      if (lock->write.data->type == TL_WRITE_ONLY) {
        /* We are not allowed to get a READ lock in this case */
        data->type = TL_UNLOCK;
        result = THR_LOCK_ABORTED; /* Can't wait for this one */
        goto end;
      }
    } else if (!lock->write_wait.data ||
               lock->write_wait.data->type <= TL_WRITE_LOW_PRIORITY ||
               lock_type == TL_READ_HIGH_PRIORITY ||
               has_old_lock(lock->read.data,
                            data->owner)) /* Has old read lock */
    {                                     /* No important write-locks */
      (*lock->read.last) = data;          /* Add to running FIFO */
      data->prev = lock->read.last;
      lock->read.last = &data->next;
      if (lock->get_status) (*lock->get_status)(data->status_param, 0);
      if (lock_type == TL_READ_NO_INSERT) lock->read_no_write_count++;
      check_locks(lock, "read lock with no write locks", 0);
      locks_immediate++;
      goto end;
    }
    /*
      We're here if there is an active write lock or no write
      lock but a high priority write waiting in the write_wait queue.
      In the latter case we should yield the lock to the writer.
    */
    wait_queue = &lock->read_wait;
  } else /* Request for WRITE lock */
  {
    if (lock_type == TL_WRITE_CONCURRENT_INSERT && !lock->check_status)
      data->type = lock_type = thr_upgraded_concurrent_insert_lock;

    if (lock->write.data) /* If there is a write lock */
    {
      if (lock->write.data->type == TL_WRITE_ONLY) {
        /* purecov: begin tested */
        /* Allow lock owner to bypass TL_WRITE_ONLY. */
        if (!thr_lock_owner_equal(data->owner, lock->write.data->owner)) {
          /* We are not allowed to get a lock in this case */
          data->type = TL_UNLOCK;
          result = THR_LOCK_ABORTED; /* Can't wait for this one */
          goto end;
        }
        /* purecov: end */
      }

      /*
        The idea is to allow us to get a lock at once if we already have
        a write lock or if there is no pending write locks and if all
        write locks are of TL_WRITE_ALLOW_WRITE type.

        Note that, since lock requests for the same table are sorted in
        such way that requests with higher thr_lock_type value come first
        (with one exception (*)), lock being requested usually has
        equal or "weaker" type than one which thread might have already
        acquired.
        *)  The only exception to this rule is case when type of old lock
            is TL_WRITE_LOW_PRIORITY and type of new lock is changed inside
            of thr_lock() from TL_WRITE_CONCURRENT_INSERT to TL_WRITE since
            engine turns out to be not supporting concurrent inserts.
            Note that since TL_WRITE has the same compatibility rules as
            TL_WRITE_LOW_PRIORITY (their only difference is priority),
            it is OK to grant new lock without additional checks in such
            situation.

        Therefore it is OK to allow acquiring write lock on the table if
        this thread already holds some write lock on it.

        (INSERT INTO t1 VALUES (f1()), where f1() is stored function which
        tries to update t1, is an example of statement which requests two
        different types of write lock on the same table).
      */
      DBUG_ASSERT(!has_old_lock(lock->write.data, data->owner) ||
                  ((lock_type <= lock->write.data->type ||
                    (lock_type == TL_WRITE &&
                     lock->write.data->type == TL_WRITE_LOW_PRIORITY))));

      if ((lock_type == TL_WRITE_ALLOW_WRITE && !lock->write_wait.data &&
           lock->write.data->type == TL_WRITE_ALLOW_WRITE) ||
          has_old_lock(lock->write.data, data->owner)) {
        /*
          We have already got a write lock or all locks are
          TL_WRITE_ALLOW_WRITE
        */
        DBUG_PRINT("info", ("write_wait.data: %p  old_type: %d",
                            lock->write_wait.data, lock->write.data->type));

        (*lock->write.last) = data; /* Add to running fifo */
        data->prev = lock->write.last;
        lock->write.last = &data->next;
        check_locks(lock, "second write lock", 0);
        if (data->lock->get_status)
          (*data->lock->get_status)(data->status_param, 0);
        locks_immediate++;
        goto end;
      }
      DBUG_PRINT("lock", ("write locked 2 by thread: 0x%x",
                          lock->write.data->owner->thread_id));
    } else {
      DBUG_PRINT("info", ("write_wait.data: %p", lock->write_wait.data));
      if (!lock->write_wait.data) { /* no scheduled write locks */
        bool concurrent_insert = false;
        if (lock_type == TL_WRITE_CONCURRENT_INSERT) {
          concurrent_insert = true;
          if ((*lock->check_status)(data->status_param)) {
            concurrent_insert = false;
            data->type = lock_type = thr_upgraded_concurrent_insert_lock;
          }
        }

        if (!lock->read.data || (lock_type <= TL_WRITE_CONCURRENT_INSERT &&
                                 ((lock_type != TL_WRITE_CONCURRENT_INSERT &&
                                   lock_type != TL_WRITE_ALLOW_WRITE) ||
                                  !lock->read_no_write_count))) {
          (*lock->write.last) = data; /* Add as current write lock */
          data->prev = lock->write.last;
          lock->write.last = &data->next;
          if (data->lock->get_status)
            (*data->lock->get_status)(data->status_param, concurrent_insert);
          check_locks(lock, "only write lock", 0);
          locks_immediate++;
          goto end;
        }
      }
      DBUG_PRINT("lock", ("write locked 3 by thread: 0x%x  type: %d",
                          lock->read.data->owner->thread_id, data->type));
    }
    wait_queue = &lock->write_wait;
  }
  /* Can't get lock yet;  Wait for it */
  result = wait_for_lock_nsec(wait_queue, data, owner, false,
                              lock_wait_timeout_nsec);
  MYSQL_END_TABLE_LOCK_WAIT(locker);
  return result;
end:
  mysql_mutex_unlock(&lock->mutex);
  MYSQL_END_TABLE_LOCK_WAIT(locker);
  return result;
}

static inline void free_all_read_locks(THR_LOCK *lock,
                                       bool using_concurrent_insert) {
  THR_LOCK_DATA *data = lock->read_wait.data;

  check_locks(lock, "before freeing read locks", 1);

  /* move all locks from read_wait list to read list */
  (*lock->read.last) = data;
  data->prev = lock->read.last;
  lock->read.last = lock->read_wait.last;

  /* Clear read_wait list */
  lock->read_wait.last = &lock->read_wait.data;

  do {
    mysql_cond_t *cond = data->cond;
    if ((int)data->type == (int)TL_READ_NO_INSERT) {
      if (using_concurrent_insert) {
        /*
          We can't free this lock;
          Link lock away from read chain back into read_wait chain
        */
        if (((*data->prev) = data->next))
          data->next->prev = data->prev;
        else
          lock->read.last = data->prev;
        *lock->read_wait.last = data;
        data->prev = lock->read_wait.last;
        lock->read_wait.last = &data->next;
        continue;
      }
      lock->read_no_write_count++;
    }
    /* purecov: begin inspected */
    DBUG_PRINT("lock",
               ("giving read lock to thread: 0x%x", data->owner->thread_id));
    /* purecov: end */
    data->cond = nullptr; /* Mark thread free */
    mysql_cond_signal(cond);
  } while ((data = data->next));
  *lock->read_wait.last = nullptr;
  if (!lock->read_wait.data) lock->write_lock_count = 0;
  check_locks(lock, "after giving read locks", 0);
}

/* Unlock lock and free next thread on same lock */

void thr_unlock(THR_LOCK_DATA *data) {
  THR_LOCK *lock = data->lock;
  enum thr_lock_type lock_type = data->type;
  DBUG_TRACE;
  DBUG_PRINT("lock", ("data: %p  thread: 0x%x  lock: %p", data,
                      data->owner->thread_id, lock));
  mysql_mutex_lock(&lock->mutex);
  check_locks(lock, "start of release lock", 0);

  if (((*data->prev) = data->next)) /* remove from lock-list */
    data->next->prev = data->prev;
  else if (lock_type <= TL_READ_NO_INSERT)
    lock->read.last = data->prev;
  else
    lock->write.last = data->prev;
  if (lock_type >= TL_WRITE_CONCURRENT_INSERT) {
    if (lock->update_status) (*lock->update_status)(data->status_param);
  } else {
    if (lock->restore_status) (*lock->restore_status)(data->status_param);
  }
  if (lock_type == TL_READ_NO_INSERT) lock->read_no_write_count--;
  data->type = TL_UNLOCK; /* Mark unlocked */
  MYSQL_UNLOCK_TABLE(data->m_psi);
  check_locks(lock, "after releasing lock", 1);
  wake_up_waiters(lock);
  mysql_mutex_unlock(&lock->mutex);
}

/**
  @brief  Wake up all threads which pending requests for the lock
          can be satisfied.

  @param  lock  Lock for which threads should be woken up

*/

static void wake_up_waiters(THR_LOCK *lock) {
  THR_LOCK_DATA *data;
  enum thr_lock_type lock_type;

  DBUG_TRACE;

  if (!lock->write.data) /* If no active write locks */
  {
    data = lock->write_wait.data;
    if (!lock->read.data) /* If no more locks in use */
    {
      /* Release write-locks with TL_WRITE or TL_WRITE_ONLY priority first */
      if (data &&
          (data->type != TL_WRITE_LOW_PRIORITY || !lock->read_wait.data ||
           lock->read_wait.data->type < TL_READ_HIGH_PRIORITY)) {
        if (lock->write_lock_count++ > max_write_lock_count) {
          /* Too many write locks in a row;  Release all waiting read locks */
          lock->write_lock_count = 0;
          if (lock->read_wait.data) {
            DBUG_PRINT(
                "info",
                ("Freeing all read_locks because of max_write_lock_count"));
            free_all_read_locks(lock, false);
            goto end;
          }
        }
        for (;;) {
          if (((*data->prev) = data->next)) /* remove from wait-list */
            data->next->prev = data->prev;
          else
            lock->write_wait.last = data->prev;
          (*lock->write.last) = data; /* Put in execute list */
          data->prev = lock->write.last;
          data->next = nullptr;
          lock->write.last = &data->next;
          if (data->type == TL_WRITE_CONCURRENT_INSERT &&
              (*lock->check_status)(data->status_param))
            data->type = TL_WRITE; /* Upgrade lock */
                                   /* purecov: begin inspected */
          DBUG_PRINT("lock", ("giving write lock of type %d to thread: 0x%x",
                              data->type, data->owner->thread_id));
          /* purecov: end */
          {
            mysql_cond_t *cond = data->cond;
            data->cond = nullptr;    /* Mark thread free */
            mysql_cond_signal(cond); /* Start waiting thread */
          }
          if (data->type != TL_WRITE_ALLOW_WRITE || !lock->write_wait.data ||
              lock->write_wait.data->type != TL_WRITE_ALLOW_WRITE)
            break;
          data = lock->write_wait.data; /* Free this too */
        }
        if (data->type >= TL_WRITE_LOW_PRIORITY) goto end;
        /* Release possible read locks together with the write lock */
      }
      if (lock->read_wait.data)
        free_all_read_locks(lock,
                            data && (data->type == TL_WRITE_CONCURRENT_INSERT ||
                                     data->type == TL_WRITE_ALLOW_WRITE));
      else {
        DBUG_PRINT("lock", ("No waiting read locks to free"));
      }
    } else if (data && (lock_type = data->type) <= TL_WRITE_CONCURRENT_INSERT &&
               ((lock_type != TL_WRITE_CONCURRENT_INSERT &&
                 lock_type != TL_WRITE_ALLOW_WRITE) ||
                !lock->read_no_write_count)) {
      /*
        For ALLOW_READ, WRITE_ALLOW_WRITE or CONCURRENT_INSERT locks
        start WRITE locks together with the READ locks
      */
      if (lock_type == TL_WRITE_CONCURRENT_INSERT &&
          (*lock->check_status)(data->status_param)) {
        data->type = TL_WRITE; /* Upgrade lock */
        if (lock->read_wait.data) free_all_read_locks(lock, false);
        goto end;
      }
      do {
        mysql_cond_t *cond = data->cond;
        if (((*data->prev) = data->next)) /* remove from wait-list */
          data->next->prev = data->prev;
        else
          lock->write_wait.last = data->prev;
        (*lock->write.last) = data; /* Put in execute list */
        data->prev = lock->write.last;
        lock->write.last = &data->next;
        data->next = nullptr;    /* Only one write lock */
        data->cond = nullptr;    /* Mark thread free */
        mysql_cond_signal(cond); /* Start waiting thread */
      } while (lock_type == TL_WRITE_ALLOW_WRITE &&
               (data = lock->write_wait.data) &&
               data->type == TL_WRITE_ALLOW_WRITE);
      if (lock->read_wait.data)
        free_all_read_locks(lock, (lock_type == TL_WRITE_CONCURRENT_INSERT ||
                                   lock_type == TL_WRITE_ALLOW_WRITE));
    } else if (!data && lock->read_wait.data)
      free_all_read_locks(lock, false);
  }
end:
  check_locks(lock, "after waking up waiters", 0);
}

/*
** Get all locks in a specific order to avoid dead-locks
** Sort acording to lock position and put write_locks before read_locks if
** lock on same lock.
*/

#define LOCK_CMP(A, B)                      \
  ((uchar *)(A->lock) - (uint)((A)->type) < \
   (uchar *)(B->lock) - (uint)((B)->type))

static void sort_locks(THR_LOCK_DATA **data, uint count) {
  THR_LOCK_DATA **pos, **end, **prev, *tmp;

  /* Sort locks with insertion sort (fast because almost always few locks) */

  for (pos = data + 1, end = data + count; pos < end; pos++) {
    tmp = *pos;
    if (LOCK_CMP(tmp, pos[-1])) {
      prev = pos;
      do {
        prev[0] = prev[-1];
      } while (--prev != data && LOCK_CMP(tmp, prev[-1]));
      prev[0] = tmp;
    }
  }
}

/**
  Locks everything given in 'data', which has length 'count'. If locking
  has failed, then 'error_pos' points to the element in 'data' that has
  failed.
  The caller controls the lifetime of 'data', so the caller is responsible
  for accessing 'error_pos' within the lifetime of 'data'.
*/

enum enum_thr_lock_result thr_multi_lock_nsec(THR_LOCK_DATA **data, uint count,
                                              THR_LOCK_INFO *owner,
                                              ulonglong lock_wait_timeout_nsec,
                                              THR_LOCK_DATA **error_pos) {
  THR_LOCK_DATA **pos, **end;
  DBUG_TRACE;
  DBUG_PRINT("lock", ("data: %p  count: %d", data, count));
  if (count > 1) sort_locks(data, count);
  /* lock everything */
  for (pos = data, end = data + count; pos < end; pos++) {
    enum enum_thr_lock_result result =
        thr_lock_nsec(*pos, owner, (*pos)->type, lock_wait_timeout_nsec);
    if (result != THR_LOCK_SUCCESS) { /* Aborted */
      thr_multi_unlock(data, (uint)(pos - data));
      if (error_pos) *error_pos = *pos;
      return result;
    }
    DEBUG_SYNC_C("thr_multi_lock_after_thr_lock");
#ifdef MAIN
    printf("Thread: T@%u  Got lock: %p  type: %d\n", pos[0]->owner->thread_id,
           pos[0]->lock, pos[0]->type);
    fflush(stdout);
#endif
  }
  thr_lock_merge_status(data, count);
  return THR_LOCK_SUCCESS;
}

/**
  Ensure that all locks for a given table have the same
  status_param.

  This is a MyISAM and possibly Maria specific crutch. MyISAM
  engine stores data file length, record count and other table
  properties in status_param member of handler. When a table is
  locked, connection-local copy is made from a global copy
  (myisam_share) by mi_get_status(). When a table is unlocked,
  the changed status is transferred back to the global share by
  mi_update_status().

  One thing MyISAM doesn't do is to ensure that when the same
  table is opened twice in a connection all instances share the
  same status_param. This is necessary, however: for one, to keep
  all instances of a connection "on the same page" with regard to
  the current state of the table. For other, unless this is done,
  myisam_share will always get updated from the last unlocked
  instance (in mi_update_status()), and when this instance was not
  the one that was used to update data, records may be lost.

  For each table, this function looks up the last lock_data in the
  list of acquired locks, and makes sure that all other instances
  share status_param with it.
*/

void thr_lock_merge_status(THR_LOCK_DATA **data, uint count) {
  THR_LOCK_DATA **pos = data;
  THR_LOCK_DATA **end = data + count;
  if (count > 1) {
    THR_LOCK_DATA *last_lock = end[-1];
    pos = end - 1;
    do {
      pos--;
      if (last_lock->lock == (*pos)->lock && last_lock->lock->copy_status) {
        if (last_lock->type <= TL_READ_NO_INSERT) {
          THR_LOCK_DATA **read_lock;
          /*
            If we are locking the same table with read locks we must ensure
            that all tables share the status of the last write lock or
            the same read lock.
          */
          for (; (*pos)->type <= TL_READ_NO_INSERT && pos != data &&
                 pos[-1]->lock == (*pos)->lock;
               pos--)
            ;

          read_lock = pos + 1;
          do {
            (last_lock->lock->copy_status)((*read_lock)->status_param,
                                           (*pos)->status_param);
          } while (*(read_lock++) != last_lock);
          last_lock = (*pos); /* Point at last write lock */
        } else
          (*last_lock->lock->copy_status)((*pos)->status_param,
                                          last_lock->status_param);
      } else
        last_lock = (*pos);
    } while (pos != data);
  }
}

/* free all locks */

void thr_multi_unlock(THR_LOCK_DATA **data, uint count) {
  THR_LOCK_DATA **pos, **end;
  DBUG_TRACE;
  DBUG_PRINT("lock", ("data: %p  count: %d", data, count));

  for (pos = data, end = data + count; pos < end; pos++) {
#ifdef MAIN
    printf("Thread: T@%u  Rel lock: %p  type: %d\n", pos[0]->owner->thread_id,
           pos[0]->lock, pos[0]->type);
    fflush(stdout);
#endif
    if ((*pos)->type != TL_UNLOCK)
      thr_unlock(*pos);
    else {
      DBUG_PRINT("lock", ("Free lock: data: %p  thread: 0x%x  lock: %p", *pos,
                          (*pos)->owner->thread_id, (*pos)->lock));
    }
  }
}

/*
  Abort all locks for specific table/thread combination

  This is used to abort all locks for a specific thread
*/

void thr_abort_locks_for_thread(THR_LOCK *lock, my_thread_id thread_id) {
  THR_LOCK_DATA *data;
  DBUG_TRACE;

  mysql_mutex_lock(&lock->mutex);
  for (data = lock->read_wait.data; data; data = data->next) {
    if (data->owner->thread_id == thread_id) /* purecov: tested */
    {
      DBUG_PRINT("info", ("Aborting read-wait lock"));
      data->type = TL_UNLOCK; /* Mark killed */
      /* It's safe to signal the cond first: we're still holding the mutex. */
      mysql_cond_signal(data->cond);
      data->cond = nullptr; /* Removed from list */

      if (((*data->prev) = data->next))
        data->next->prev = data->prev;
      else
        lock->read_wait.last = data->prev;
    }
  }
  for (data = lock->write_wait.data; data; data = data->next) {
    if (data->owner->thread_id == thread_id) /* purecov: tested */
    {
      DBUG_PRINT("info", ("Aborting write-wait lock"));
      data->type = TL_UNLOCK;
      mysql_cond_signal(data->cond);
      data->cond = nullptr;

      if (((*data->prev) = data->next))
        data->next->prev = data->prev;
      else
        lock->write_wait.last = data->prev;
    }
  }
  wake_up_waiters(lock);
  mysql_mutex_unlock(&lock->mutex);
}

static void thr_print_lock(const char *name, struct st_lock_list *list) {
  THR_LOCK_DATA *data, **prev;
  uint count = 0;

  if (list->data) {
    printf("%-10s: ", name);
    prev = &list->data;
    for (data = list->data; data && count++ < MAX_LOCKS; data = data->next) {
      printf("%p (%u:%d); ", data, data->owner->thread_id, (int)data->type);
      if (data->prev != prev)
        printf("\nWarning: prev didn't point at previous lock\n");
      prev = &data->next;
    }
    puts("");
    if (prev != list->last) printf("Warning: last didn't point at last lock\n");
  }
}

void thr_print_locks(void) {
  LIST *list;
  uint count = 0;

  mysql_mutex_lock(&THR_LOCK_lock);
  puts("Current locks:");
  for (list = thr_lock_thread_list; list && count++ < MAX_THREADS;
       list = list_rest(list)) {
    THR_LOCK *lock = (THR_LOCK *)list->data;
    mysql_mutex_lock(&lock->mutex);
    printf("lock: %p:", lock);
    if ((lock->write_wait.data || lock->read_wait.data) &&
        (!lock->read.data && !lock->write.data))
      printf(" WARNING: ");
    if (lock->write.data) printf(" write");
    if (lock->write_wait.data) printf(" write_wait");
    if (lock->read.data) printf(" read");
    if (lock->read_wait.data) printf(" read_wait");
    puts("");
    thr_print_lock("write", &lock->write);
    thr_print_lock("write_wait", &lock->write_wait);
    thr_print_lock("read", &lock->read);
    thr_print_lock("read_wait", &lock->read_wait);
    mysql_mutex_unlock(&lock->mutex);
    puts("");
  }
  fflush(stdout);
  mysql_mutex_unlock(&THR_LOCK_lock);
}

/*****************************************************************************
** Test of thread locks
****************************************************************************/

#ifdef MAIN

struct st_test {
  uint lock_nr;
  enum thr_lock_type lock_type;
};

THR_LOCK locks[5]; /* 4 locks */

struct st_test test_0[] = {{0, TL_READ}}; /* One lock */
struct st_test test_1[] = {{0, TL_READ},
                           {0, TL_WRITE}}; /* Read and write lock of lock 0 */
struct st_test test_2[] = {{1, TL_WRITE}, {0, TL_READ}, {2, TL_READ}};
struct st_test test_3[] = {
    {2, TL_WRITE}, {1, TL_READ}, {0, TL_READ}}; /* Deadlock with test_2 ? */
struct st_test test_4[] = {
    {0, TL_WRITE}, {0, TL_READ}, {0, TL_WRITE}, {0, TL_READ}};
struct st_test test_5[] = {
    {0, TL_READ}, {1, TL_READ}, {2, TL_READ}, {3, TL_READ}}; /* Many reads */
struct st_test test_6[] = {{0, TL_WRITE},
                           {1, TL_WRITE},
                           {2, TL_WRITE},
                           {3, TL_WRITE}}; /* Many writes */
struct st_test test_7[] = {{3, TL_READ}};
struct st_test test_8[] = {{1, TL_READ_NO_INSERT},
                           {2, TL_READ_NO_INSERT},
                           {3, TL_READ_NO_INSERT}}; /* Should be quick */
struct st_test test_9[] = {{4, TL_READ_HIGH_PRIORITY}};
struct st_test test_10[] = {{4, TL_WRITE}};
struct st_test test_11[] = {{0, TL_WRITE_LOW_PRIORITY},
                            {1, TL_WRITE_LOW_PRIORITY},
                            {2, TL_WRITE_LOW_PRIORITY},
                            {3, TL_WRITE_LOW_PRIORITY}}; /* Many writes */
struct st_test test_12[] = {{0, TL_WRITE_CONCURRENT_INSERT},
                            {1, TL_WRITE_CONCURRENT_INSERT},
                            {2, TL_WRITE_CONCURRENT_INSERT},
                            {3, TL_WRITE_CONCURRENT_INSERT}};
struct st_test test_13[] = {{0, TL_WRITE_CONCURRENT_INSERT}, {1, TL_READ}};
struct st_test test_14[] = {{0, TL_WRITE_ALLOW_WRITE}, {1, TL_READ}};
struct st_test test_15[] = {{0, TL_WRITE_ALLOW_WRITE},
                            {1, TL_WRITE_ALLOW_WRITE}};

struct st_test *tests[] = {test_0,  test_1,  test_2,  test_3, test_4,  test_5,
                           test_6,  test_7,  test_8,  test_9, test_10, test_11,
                           test_12, test_13, test_14, test_15};
int lock_counts[] = {sizeof(test_0) / sizeof(struct st_test),
                     sizeof(test_1) / sizeof(struct st_test),
                     sizeof(test_2) / sizeof(struct st_test),
                     sizeof(test_3) / sizeof(struct st_test),
                     sizeof(test_4) / sizeof(struct st_test),
                     sizeof(test_5) / sizeof(struct st_test),
                     sizeof(test_6) / sizeof(struct st_test),
                     sizeof(test_7) / sizeof(struct st_test),
                     sizeof(test_8) / sizeof(struct st_test),
                     sizeof(test_9) / sizeof(struct st_test),
                     sizeof(test_10) / sizeof(struct st_test),
                     sizeof(test_11) / sizeof(struct st_test),
                     sizeof(test_12) / sizeof(struct st_test),
                     sizeof(test_13) / sizeof(struct st_test),
                     sizeof(test_14) / sizeof(struct st_test),
                     sizeof(test_15) / sizeof(struct st_test)};

static mysql_cond_t COND_thread_count;
static mysql_mutex_t LOCK_thread_count;
static uint thread_count;
static ulong sum = 0;

#define MAX_LOCK_COUNT 8
#define TEST_TIMEOUT 100000
#define TEST_TIMEOUT_NSEC (100000 * 1000000000ULL)

/* The following functions is for WRITE_CONCURRENT_INSERT */

static void test_get_status(void *param MY_ATTRIBUTE((unused)),
                            int concurrent_insert MY_ATTRIBUTE((unused))) {}

static void test_update_status(void *param MY_ATTRIBUTE((unused))) {}

static void test_copy_status(void *to MY_ATTRIBUTE((unused)),
                             void *from MY_ATTRIBUTE((unused))) {}

static bool test_check_status(void *param MY_ATTRIBUTE((unused))) {
  return false;
}

static void *test_thread(void *arg) {
  int i, j, param = *((int *)arg);
  THR_LOCK_DATA data[MAX_LOCK_COUNT];
  THR_LOCK_INFO lock_info;
  THR_LOCK_DATA *multi_locks[MAX_LOCK_COUNT];
  my_thread_id id;
  mysql_cond_t COND_thr_lock;

  id = param + 1; /* Main thread uses value 0. */
  mysql_cond_init(0, &COND_thr_lock);

  printf("Thread T@%d started\n", id);
  fflush(stdout);

  thr_lock_info_init(&lock_info, id, &COND_thr_lock);
  for (i = 0; i < lock_counts[param]; i++) {
    thr_lock_data_init(locks + tests[param][i].lock_nr, data + i, nullptr);
    data[i].m_psi = nullptr;
  }
  for (j = 1; j < 10; j++) /* try locking 10 times */
  {
    for (i = 0; i < lock_counts[param]; i++) { /* Init multi locks */
      multi_locks[i] = &data[i];
      data[i].type = tests[param][i].lock_type;
    }
    thr_multi_lock_nsec(multi_locks, lock_counts[param], &lock_info,
                        TEST_TIMEOUT_NSEC, nullptr);
    mysql_mutex_lock(&LOCK_thread_count);
    {
      int tmp = rand() & 7; /* Do something from 0-2 sec */
      if (tmp == 0)
        sleep(1);
      else if (tmp == 1)
        sleep(2);
      else {
        ulong k;
        for (k = 0; k < (ulong)(tmp - 2) * 100000L; k++) sum += k;
      }
    }
    mysql_mutex_unlock(&LOCK_thread_count);
    thr_multi_unlock(multi_locks, lock_counts[param]);
  }

  printf("Thread T@%d ended\n", id);
  fflush(stdout);
  thr_print_locks();
  mysql_mutex_lock(&LOCK_thread_count);
  thread_count--;
  mysql_cond_signal(&COND_thread_count); /* Tell main we are ready */
  mysql_mutex_unlock(&LOCK_thread_count);
  mysql_cond_destroy(&COND_thr_lock);
  free((uchar *)arg);
  return nullptr;
}

int main(int argc, char **argv) {
  my_thread_handle tid;
  my_thread_attr_t thr_attr;
  int i, *param, error;
  MY_INIT(argv[0]);
  if (argc > 1 && argv[1][0] == '-' && argv[1][1] == '#')
    DBUG_PUSH(argv[1] + 2);

  printf("Main thread: T@%u\n", 0); /* 0 for main thread, 1+ for test_thread */

  if ((error = mysql_cond_init(0, &COND_thread_count))) {
    my_message_stderr(0, "Got error %d from mysql_cond_init", errno);
    exit(1);
  }
  if ((error = mysql_mutex_init(0, &LOCK_thread_count, MY_MUTEX_INIT_FAST))) {
    my_message_stderr(0, "Got error %d from mysql_cond_init", errno);
    exit(1);
  }

  for (i = 0; i < (int)array_elements(locks); i++) {
    thr_lock_init(locks + i);
    locks[i].check_status = test_check_status;
    locks[i].update_status = test_update_status;
    locks[i].copy_status = test_copy_status;
    locks[i].get_status = test_get_status;
  }
  if ((error = my_thread_attr_init(&thr_attr))) {
    my_message_stderr(0, "Got error %d from pthread_attr_init", errno);
    exit(1);
  }
  if ((error = my_thread_attr_setdetachstate(&thr_attr,
                                             MY_THREAD_CREATE_DETACHED))) {
    my_message_stderr(0,
                      "Got error %d from "
                      "my_thread_attr_setdetachstate",
                      errno);
    exit(1);
  }
  if ((error = my_thread_attr_setstacksize(&thr_attr, 65536L))) {
    my_message_stderr(0,
                      "Got error %d from "
                      "my_thread_attr_setstacksize",
                      error);
    exit(1);
  }
  for (i = 0; i < (int)array_elements(lock_counts); i++) {
    param = (int *)malloc(sizeof(int));
    *param = i;

    if ((error = mysql_mutex_lock(&LOCK_thread_count))) {
      my_message_stderr(0, "Got error %d from mysql_mutex_lock", errno);
      exit(1);
    }
    if ((error = mysql_thread_create(0, &tid, &thr_attr, test_thread,
                                     (void *)param))) {
      my_message_stderr(0, "Got error %d from mysql_thread_create", errno);
      mysql_mutex_unlock(&LOCK_thread_count);
      exit(1);
    }
    thread_count++;
    mysql_mutex_unlock(&LOCK_thread_count);
  }

  my_thread_attr_destroy(&thr_attr);
  if ((error = mysql_mutex_lock(&LOCK_thread_count)))
    my_message_stderr(0, "Got error %d from mysql_mutex_lock", error);
  while (thread_count) {
    if ((error = mysql_cond_wait(&COND_thread_count, &LOCK_thread_count)))
      my_message_stderr(0, "Got error %d from mysql_cond_wait", error);
  }
  if ((error = mysql_mutex_unlock(&LOCK_thread_count)))
    my_message_stderr(0, "Got error %d from mysql_mutex_unlock", error);
  for (i = 0; i < (int)array_elements(locks); i++) thr_lock_delete(locks + i);
#ifdef EXTRA_DEBUG
  if (found_errors)
    printf("Got %d warnings\n", found_errors);
  else
#endif
    printf("Test succeeded\n");
  return 0;
}

#endif /* MAIN */
