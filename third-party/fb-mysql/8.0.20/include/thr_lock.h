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

/* For use with thr_lock:s */

/**
  @file include/thr_lock.h
*/

#ifndef _thr_lock_h
#define _thr_lock_h

#include <sys/types.h>

#include "my_inttypes.h"
#include "my_list.h"
#include "my_thread_local.h"
#include "mysql/psi/mysql_cond.h"
#include "mysql/psi/mysql_mutex.h"

extern mysql_mutex_t THR_LOCK_lock;

struct THR_LOCK;
struct TABLE;

extern ulong locks_immediate, locks_waited;

/*
  Important: if a new lock type is added, a matching lock description
             must be added to sql_test.cc's lock_descriptions array.
*/
enum thr_lock_type {
  TL_IGNORE = -1,
  TL_UNLOCK, /* UNLOCK ANY LOCK */
  /*
    Parser only! At open_tables() becomes TL_READ or
    TL_READ_NO_INSERT depending on the binary log format
    (SBR/RBR) and on the table category (log table).
    Used for tables that are read by statements which
    modify tables.
  */
  TL_READ_DEFAULT,
  TL_READ, /* Read lock */
  TL_READ_WITH_SHARED_LOCKS,
  /* High prior. than TL_WRITE. Allow concurrent insert */
  TL_READ_HIGH_PRIORITY,
  /* READ, Don't allow concurrent insert */
  TL_READ_NO_INSERT,
  /*
     Write lock, but allow other threads to read / write.
     Used by BDB tables in MySQL to mark that someone is
     reading/writing to the table.
   */
  TL_WRITE_ALLOW_WRITE,
  /*
    parser only! Late bound low_priority_flag.
    At open_tables() becomes thd->insert_lock_default.
  */
  TL_WRITE_CONCURRENT_DEFAULT,
  /*
    WRITE lock used by concurrent insert. Will allow
    READ, if one could use concurrent insert on table.
  */
  TL_WRITE_CONCURRENT_INSERT,
  /*
    parser only! Late bound low_priority flag.
    At open_tables() becomes thd->update_lock_default.
  */
  TL_WRITE_DEFAULT,
  /* WRITE lock that has lower priority than TL_READ */
  TL_WRITE_LOW_PRIORITY,
  /* Normal WRITE lock */
  TL_WRITE,
  /* Abort new lock request with an error */
  TL_WRITE_ONLY
};

enum thr_locked_row_action { THR_DEFAULT, THR_WAIT, THR_NOWAIT, THR_SKIP };

struct Lock_descriptor {
  thr_lock_type type{TL_UNLOCK};
  thr_locked_row_action action{THR_DEFAULT};
};

enum enum_thr_lock_result {
  THR_LOCK_SUCCESS = 0,
  THR_LOCK_ABORTED = 1,
  THR_LOCK_WAIT_TIMEOUT = 2,
  THR_LOCK_DEADLOCK = 3
};

extern ulong max_write_lock_count;
extern enum thr_lock_type thr_upgraded_concurrent_insert_lock;

/*
  A description of the thread which owns the lock. The address
  of an instance of this structure is used to uniquely identify the thread.
*/

struct THR_LOCK_INFO {
  my_thread_id thread_id;
  mysql_cond_t *suspend;
};

struct THR_LOCK_DATA {
  THR_LOCK_INFO *owner{nullptr};
  THR_LOCK_DATA *next{nullptr}, **prev{nullptr};
  THR_LOCK *lock{nullptr};
  mysql_cond_t *cond{nullptr};
  thr_lock_type type{TL_IGNORE};
  void *status_param{nullptr}; /* Param to status functions */
  struct TABLE *table{nullptr};
  struct PSI_table *m_psi{nullptr};
};

struct st_lock_list {
  THR_LOCK_DATA *data{nullptr}, **last{nullptr};
};

struct THR_LOCK {
  LIST list;
  mysql_mutex_t mutex;
  struct st_lock_list read_wait;
  struct st_lock_list read;
  struct st_lock_list write_wait;
  struct st_lock_list write;
  /* write_lock_count is incremented for write locks and reset on read locks */
  ulong write_lock_count{0};
  uint read_no_write_count{0};
  void (*get_status)(void *, int){nullptr}; /* When one gets a lock */
  void (*copy_status)(void *, void *){nullptr};
  void (*update_status)(void *){nullptr};  /* Before release of write */
  void (*restore_status)(void *){nullptr}; /* Before release of read */
  bool (*check_status)(void *){nullptr};
};

extern LIST *thr_lock_thread_list;

void thr_lock_info_init(THR_LOCK_INFO *info, my_thread_id thread_id,
                        mysql_cond_t *suspend);
void thr_lock_init(THR_LOCK *lock);
void thr_lock_delete(THR_LOCK *lock);
void thr_lock_data_init(THR_LOCK *lock, THR_LOCK_DATA *data,
                        void *status_param);
enum enum_thr_lock_result thr_lock_nsec(THR_LOCK_DATA *data,
                                        THR_LOCK_INFO *owner,
                                        enum thr_lock_type lock_type,
                                        ulonglong lock_wait_timeout_nsec);
void thr_unlock(THR_LOCK_DATA *data);
enum enum_thr_lock_result thr_multi_lock_nsec(THR_LOCK_DATA **data, uint count,
                                              THR_LOCK_INFO *owner,
                                              ulonglong lock_wait_timeout_nsec,
                                              THR_LOCK_DATA **error_pos);
void thr_multi_unlock(THR_LOCK_DATA **data, uint count);
void thr_lock_merge_status(THR_LOCK_DATA **data, uint count);
void thr_abort_locks_for_thread(THR_LOCK *lock, my_thread_id thread);
void thr_print_locks(void); /* For debugging */
void thr_set_lock_wait_callback(void (*before_wait)(void),
                                void (*after_wait)(void));
#endif /* _thr_lock_h */
