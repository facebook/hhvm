#ifndef PARTITIONED_RWLOCK_INCLUDED
#define PARTITIONED_RWLOCK_INCLUDED

/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <new>

#include "my_compiler.h"
#include "my_psi_config.h"
#include "mysql/psi/mysql_rwlock.h"

/**
  Implementation of read-write lock partitioned by thread id.

  This rwlock provides better scalability in read-heavy environments by
  employing the following simple trick:
  *) Read lock is acquired only on one of its partitions. The specific
     partition is chosen according to thread id.
  *) Write lock is acquired on all partitions.

  This way concurrent request for read lock made by different threads
  have a good chance not to disturb each other by doing cache invalidation
  and atomic operations. As result scalability in this scenario improves.
  OTOH acquisition of write lock becomes more expensive. So this rwlock
  is not supposed to be used in cases when number of write requests is
  significant.
*/

class Partitioned_rwlock {
 public:
  Partitioned_rwlock() {}

  /**
    @param parts    Number of partitions.
    @param psi_key  P_S instrumentation key to use for rwlock instances
                    for partitions.
  */
  bool init(uint parts
#ifdef HAVE_PSI_INTERFACE
            ,
            PSI_rwlock_key psi_key MY_ATTRIBUTE((unused))
#endif
  ) {
    m_parts = parts;
    if (!(m_locks_array = new (std::nothrow) mysql_rwlock_t[m_parts]))
      return true;
    for (uint i = 0; i < m_parts; ++i)
      mysql_rwlock_init(psi_key, &m_locks_array[i]);
    return false;
  }
  void destroy() {
    for (uint i = 0; i < m_parts; ++i) mysql_rwlock_destroy(&m_locks_array[i]);
    delete[] m_locks_array;
  }
  void wrlock() {
    for (uint i = 0; i < m_parts; ++i) mysql_rwlock_wrlock(&m_locks_array[i]);
  }
  void wrunlock() {
    for (uint i = 0; i < m_parts; ++i) mysql_rwlock_unlock(&m_locks_array[i]);
  }
  void rdlock(uint thread_id) {
    mysql_rwlock_rdlock(&m_locks_array[thread_id % m_parts]);
  }
  /*
    One should use the same thread number for releasing read lock
    as was used for acquiring it,
  */
  void rdunlock(uint thread_id) {
    mysql_rwlock_unlock(&m_locks_array[thread_id % m_parts]);
  }

 private:
  mysql_rwlock_t *m_locks_array;
  uint m_parts;

  Partitioned_rwlock(const Partitioned_rwlock &);             // Non-copyable
  Partitioned_rwlock &operator=(const Partitioned_rwlock &);  // Non-copyable
};

/**
  Read lock guard class for Partitioned_rwlock. Supports early unlocking.
*/

class Partitioned_rwlock_read_guard {
 public:
  /**
    Acquires read lock on partitioned rwlock on behalf of thread.
    Automatically release lock in destructor.
  */
  Partitioned_rwlock_read_guard(Partitioned_rwlock *rwlock, uint thread_id)
      : m_rwlock(rwlock), m_thread_id(thread_id) {
    m_rwlock->rdlock(m_thread_id);
  }

  ~Partitioned_rwlock_read_guard() {
    if (m_rwlock) m_rwlock->rdunlock(m_thread_id);
  }

  /** Release read lock. Optional method for early unlocking. */
  void unlock() {
    m_rwlock->rdunlock(m_thread_id);
    m_rwlock = nullptr;
  }

 private:
  /**
    Pointer to partitioned rwlock which was acquired. NULL if lock was
    released early so destructor should not do anything.
  */
  Partitioned_rwlock *m_rwlock;
  /**
    Id of thread on which behalf lock was acquired and which is to be used for
    unlocking.
  */
  uint m_thread_id;

  // Non-copyable
  Partitioned_rwlock_read_guard(const Partitioned_rwlock_read_guard &);
  Partitioned_rwlock_read_guard &operator=(
      const Partitioned_rwlock_read_guard &);
};

/**
  Write lock guard class for Partitioned_rwlock. Supports early unlocking.
*/

class Partitioned_rwlock_write_guard {
 public:
  /**
    Acquires write lock on partitioned rwlock.
    Automatically release it in destructor.
  */
  explicit Partitioned_rwlock_write_guard(Partitioned_rwlock *rwlock)
      : m_rwlock(rwlock) {
    m_rwlock->wrlock();
  }

  ~Partitioned_rwlock_write_guard() {
    if (m_rwlock) m_rwlock->wrunlock();
  }

  /** Release write lock. Optional method for early unlocking. */
  void unlock() {
    m_rwlock->wrunlock();
    m_rwlock = nullptr;
  }

 private:
  /**
    Pointer to partitioned rwlock which was acquired. NULL if lock was
    released early so destructor should not do anything.
  */
  Partitioned_rwlock *m_rwlock;

  // Non-copyable
  Partitioned_rwlock_write_guard(const Partitioned_rwlock_write_guard &);
  Partitioned_rwlock_write_guard &operator=(
      const Partitioned_rwlock_write_guard &);
};

#endif /* PARTITIONED_RWLOCK_INCLUDED */
