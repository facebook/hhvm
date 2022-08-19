/* Copyright (c) 2014, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MUTEX_LOCK_INCLUDED
#define MUTEX_LOCK_INCLUDED

/**
  @file include/mutex_lock.h
*/

#include <mysql/psi/mysql_mutex.h>

/**
  A simple wrapper around a mutex:
  Grabs the mutex in the CTOR, releases it in the DTOR.
  The mutex may be NULL, in which case this is a no-op.
*/
class Mutex_lock {
 public:
  explicit Mutex_lock(mysql_mutex_t *mutex, const char *src_file, int src_line)
      : m_mutex(mutex), m_src_file(src_file), m_src_line(src_line) {
    if (m_mutex) {
      mysql_mutex_lock_with_src(m_mutex, m_src_file, m_src_line);
    }
  }
  ~Mutex_lock() {
    if (m_mutex) {
      mysql_mutex_unlock_with_src(m_mutex, m_src_file, m_src_line);
    }
  }

 private:
  mysql_mutex_t *m_mutex;
  const char *m_src_file;
  int m_src_line;

  Mutex_lock(const Mutex_lock &);     /* Not copyable. */
  void operator=(const Mutex_lock &); /* Not assignable. */
};

#define MUTEX_LOCK(NAME, X) Mutex_lock NAME(X, __FILE__, __LINE__)

#endif  // MUTEX_LOCK_INCLUDED
