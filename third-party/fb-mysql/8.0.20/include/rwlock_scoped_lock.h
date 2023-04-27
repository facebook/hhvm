/* Copyright (c) 2016, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MYSQL_RWLOCK_SCOPED_LOCK_H
#define MYSQL_RWLOCK_SCOPED_LOCK_H

#include "mysql/psi/mysql_rwlock.h"

/**
  Locks RW-lock and releases lock on scope exit.
*/
class rwlock_scoped_lock {
 public:
  rwlock_scoped_lock(mysql_rwlock_t *lock, bool lock_for_write,
                     const char *file, int line);
  rwlock_scoped_lock(rwlock_scoped_lock &&lock);
  rwlock_scoped_lock(const rwlock_scoped_lock &) = delete;
  ~rwlock_scoped_lock();

  rwlock_scoped_lock &operator=(const rwlock_scoped_lock &) = delete;

 private:
  mysql_rwlock_t *m_lock;
};

#endif /* MYSQL_RWLOCK_SCOPE_LOCK_H */
