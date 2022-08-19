#ifndef INCLUDE_GUARD_H_
#define INCLUDE_GUARD_H_
/* Copyright (c) 2019, 2020, Oracle and/or its affiliates. All rights reserved.

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
  @file guard.h RAII classes for locking/unlocking, inspired by the guard
  class of Boost fame.
*/

#include <mysql/psi/mysql_mutex.h>
#include <mysql/psi/mysql_rwlock.h>
#include <mysql/psi/mysql_thread.h>

/// Guards a mutex.
class Mutex_guard {
 public:
  Mutex_guard(const Mutex_guard &lock) = delete;

  /**
    Object construction that locks specified mutex.

    @param mutex Mutex pointer to be locked.
  */
  explicit Mutex_guard(mysql_mutex_t *mutex) : m_mutex(mutex) {
    mysql_mutex_lock(m_mutex);
  }

  /// Destroys object and unlocks the mutex.
  ~Mutex_guard() { mysql_mutex_unlock(m_mutex); }

 private:
  mysql_mutex_t *m_mutex;
};

#endif  // INCLUDE_GUARD_H_
