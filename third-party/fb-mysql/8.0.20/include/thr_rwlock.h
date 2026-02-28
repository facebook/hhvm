#ifndef THR_RWLOCK_INCLUDED
#define THR_RWLOCK_INCLUDED

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

/**
  @file include/thr_rwlock.h
  MySQL rwlock implementation.

  There are two "layers":
  1) native_rw_*()
       Functions that map directly down to OS primitives.
       Windows    - SRWLock
       Other OSes - pthread
  2) mysql_rw*()
       Functions that include Performance Schema instrumentation.
       See include/mysql/psi/mysql_thread.h

  This file also includes rw_pr_*(), which implements a special
  version of rwlocks that prefer readers. The P_S version of these
  are mysql_prlock_*() - see include/mysql/psi/mysql_thread.h
*/

#include <stddef.h>
#include <sys/types.h>
#ifdef _WIN32
#include <windows.h>
#endif

#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_macros.h"
#include "my_thread.h"
#include "mysql/components/services/thr_rwlock_bits.h"
#include "thr_cond.h"
#include "thr_mutex.h"

static inline int native_rw_init(native_rw_lock_t *rwp) {
#ifdef _WIN32
  InitializeSRWLock(&rwp->srwlock);
  rwp->have_exclusive_srwlock = false;
  return 0;
#else
  /* pthread_rwlockattr_t is not used in MySQL */
  return pthread_rwlock_init(rwp, nullptr);
#endif
}

static inline int native_rw_destroy(
    native_rw_lock_t *rwp MY_ATTRIBUTE((unused))) {
#ifdef _WIN32
  return 0; /* no destroy function */
#else
  return pthread_rwlock_destroy(rwp);
#endif
}

static inline int native_rw_rdlock(native_rw_lock_t *rwp) {
#ifdef _WIN32
  AcquireSRWLockShared(&rwp->srwlock);
  return 0;
#else
  return pthread_rwlock_rdlock(rwp);
#endif
}

static inline int native_rw_tryrdlock(native_rw_lock_t *rwp) {
#ifdef _WIN32
  if (!TryAcquireSRWLockShared(&rwp->srwlock)) return EBUSY;
  return 0;
#else
  return pthread_rwlock_tryrdlock(rwp);
#endif
}

static inline int native_rw_wrlock(native_rw_lock_t *rwp) {
#ifdef _WIN32
  AcquireSRWLockExclusive(&rwp->srwlock);
  rwp->have_exclusive_srwlock = true;
  return 0;
#else
  return pthread_rwlock_wrlock(rwp);
#endif
}

static inline int native_rw_trywrlock(native_rw_lock_t *rwp) {
#ifdef _WIN32
  if (!TryAcquireSRWLockExclusive(&rwp->srwlock)) return EBUSY;
  rwp->have_exclusive_srwlock = true;
  return 0;
#else
  return pthread_rwlock_trywrlock(rwp);
#endif
}

static inline int native_rw_unlock(native_rw_lock_t *rwp) {
#ifdef _WIN32
  if (rwp->have_exclusive_srwlock) {
    rwp->have_exclusive_srwlock = false;
    ReleaseSRWLockExclusive(&rwp->srwlock);
  } else
    ReleaseSRWLockShared(&rwp->srwlock);
  return 0;
#else
  return pthread_rwlock_unlock(rwp);
#endif
}

extern int rw_pr_init(rw_pr_lock_t *);
extern int rw_pr_rdlock(rw_pr_lock_t *);
extern int rw_pr_wrlock(rw_pr_lock_t *);
extern int rw_pr_unlock(rw_pr_lock_t *);
extern int rw_pr_destroy(rw_pr_lock_t *);

#ifdef SAFE_MUTEX
static inline void rw_pr_lock_assert_write_owner(const rw_pr_lock_t *rwlock) {
  DBUG_ASSERT(rwlock->active_writer &&
              my_thread_equal(my_thread_self(), rwlock->writer_thread));
}

static inline void rw_pr_lock_assert_not_write_owner(
    const rw_pr_lock_t *rwlock) {
  DBUG_ASSERT(!rwlock->active_writer ||
              !my_thread_equal(my_thread_self(), rwlock->writer_thread));
}
#endif

#endif /* THR_RWLOCK_INCLUDED */
