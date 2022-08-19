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

#ifndef COMPONENTS_SERVICES_THR_RWLOCK_BITS_H
#define COMPONENTS_SERVICES_THR_RWLOCK_BITS_H

/**
  @file
  MySQL rwlock ABI.

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

#include <mysql/components/services/my_thread_bits.h>
#include <mysql/components/services/thr_cond_bits.h>
#include <mysql/components/services/thr_mutex_bits.h>

#ifdef _WIN32
struct native_rw_lock_t {
  SRWLOCK srwlock;             /* native reader writer lock */
  BOOL have_exclusive_srwlock; /* used for unlock */
};
#else
typedef pthread_rwlock_t native_rw_lock_t;
#endif

/**
  Portable implementation of special type of read-write locks.

  These locks have two properties which are unusual for rwlocks:
  1) They "prefer readers" in the sense that they do not allow
     situations in which rwlock is rd-locked and there is a
     pending rd-lock which is blocked (e.g. due to pending
     request for wr-lock).
     This is a stronger guarantee than one which is provided for
     PTHREAD_RWLOCK_PREFER_READER_NP rwlocks in Linux.
     MDL subsystem deadlock detector relies on this property for
     its correctness.
  2) They are optimized for uncontended wr-lock/unlock case.
     This is scenario in which they are most oftenly used
     within MDL subsystem. Optimizing for it gives significant
     performance improvements in some of tests involving many
     connections.

  Another important requirement imposed on this type of rwlock
  by the MDL subsystem is that it should be OK to destroy rwlock
  object which is in unlocked state even though some threads might
  have not yet fully left unlock operation for it (of course there
  is an external guarantee that no thread will try to lock rwlock
  which is destroyed).
  Putting it another way the unlock operation should not access
  rwlock data after changing its state to unlocked.

  TODO/FIXME: We should consider alleviating this requirement as
  it blocks us from doing certain performance optimizations.
*/

struct rw_pr_lock_t {
  /**
    Lock which protects the structure.
    Also held for the duration of wr-lock.
  */
  native_mutex_t lock;
  /**
    Condition variable which is used to wake-up
    writers waiting for readers to go away.
  */
  native_cond_t no_active_readers;
  /** Number of active readers. */
  unsigned int active_readers;
  /** Number of writers waiting for readers to go away. */
  unsigned int writers_waiting_readers;
  /** Indicates whether there is an active writer. */
  bool active_writer;
  /** Thread holding wr-lock (for debug purposes only). */
  my_thread_t writer_thread;
};

#endif /* COMPONENTS_SERVICES_THR_RWLOCK_BITS_H */
