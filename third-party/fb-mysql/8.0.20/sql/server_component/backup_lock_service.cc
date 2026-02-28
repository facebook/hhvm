/* Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "mysql/components/services/backup_lock_service.h"
#include "mysql/components/service_implementation.h"
#include "sql/current_thd.h"      // current_thd
#include "sql/sql_backup_lock.h"  // acquire_exclusive_backup_lock,

class THD;
// release_backup_lock

DEFINE_BOOL_METHOD(mysql_acquire_backup_lock_nsec,
                   (MYSQL_THD opaque_thd,
                    enum enum_backup_lock_service_lock_kind lock_kind,
                    ulonglong lock_timeout_nsec)) {
  THD *thd;
  if (opaque_thd)
    thd = static_cast<THD *>(opaque_thd);
  else
    thd = current_thd;

  if (lock_kind == BACKUP_LOCK_SERVICE_DEFAULT)
    return acquire_exclusive_backup_lock_nsec(thd, lock_timeout_nsec, false);

  /*
    Return error in case lock_kind has an unexpected value.
    As new kind of lock be added into the enumeration
    enum_backup_lock_service_lock_kind its handling should be added here.
  */
  return true;
}

DEFINE_BOOL_METHOD(mysql_release_backup_lock, (MYSQL_THD opaque_thd)) {
  THD *thd;
  if (opaque_thd)
    thd = static_cast<THD *>(opaque_thd);
  else
    thd = current_thd;

  release_backup_lock(thd);

  return false;
}
