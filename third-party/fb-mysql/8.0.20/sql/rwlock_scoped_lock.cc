/* Copyright (c) 2016, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "rwlock_scoped_lock.h"

#include <stddef.h>

#include "my_compiler.h"
#include "mysql/psi/mysql_rwlock.h"

struct mysql_rwlock_t;

/**
  Acquires lock on specified lock object.
  The lock may be NULL, in which case this is a no-op.

  @param lock Lock object to lock.
  @param lock_for_write Specifies if to lock for write or read.
  @param file File in which lock acquisition is to be presented.
  @param line Line of file in which lock acquisition is to be presented.
*/
rwlock_scoped_lock::rwlock_scoped_lock(mysql_rwlock_t *lock,
                                       bool lock_for_write,
                                       const char *file MY_ATTRIBUTE((unused)),
                                       int line MY_ATTRIBUTE((unused))) {
  if (lock_for_write) {
    if (!mysql_rwlock_wrlock_with_src(lock, file, line)) {
      m_lock = lock;
    } else
      m_lock = NULL;
  } else {
    if (!mysql_rwlock_rdlock_with_src(lock, file, line)) {
      m_lock = lock;
    } else
      m_lock = NULL;
  }
}

/**
  Moves lock from another object.

  @param lock Scoped lock object to move from.
*/
rwlock_scoped_lock::rwlock_scoped_lock(rwlock_scoped_lock &&lock)
    : m_lock(lock.m_lock) {
  lock.m_lock = nullptr;
}

rwlock_scoped_lock::~rwlock_scoped_lock() {
  /* If lock is NULL, then lock was set to remain locked when going out of
    scope or was moved to other object. */
  if (m_lock != nullptr) {
    mysql_rwlock_unlock(m_lock);
  }
}
