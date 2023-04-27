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

#ifndef BACKUP_LOCK_SERVICE_H
#define BACKUP_LOCK_SERVICE_H
#include <mysql/components/service.h>
#include <stddef.h>
#include "my_inttypes.h"

/**
  Kind of Backup Lock to be acquired. In future we might
  want to have weaker kinds of Backup Lock (e.g. one that
  will allow concurrent TRUNCATE TABLE).
*/

typedef enum enum_backup_lock_service_lock_kind {
  BACKUP_LOCK_SERVICE_DEFAULT = 0
} backup_lock_service_lock_kind;

#ifdef __cplusplus
class THD;
#define MYSQL_THD THD *
#else
#define MYSQL_THD void *
#endif

DEFINE_SERVICE_HANDLE(Backup_lock_handle);

BEGIN_SERVICE_DEFINITION(mysql_backup_lock)

/**
  Service API to acquire shared Backup Lock.

  @param opaque_thd    Current thread context.
  @param lock_kind     Kind of lock to acquire - BACKUP_LOCK_SERVICE_DEFAULT
                       or weaker.
  @param lock_timeout  Number of seconds to wait before giving up.

  @return Operation status.
    @retval false Success
    @retval true  Failure
*/

DECLARE_BOOL_METHOD(acquire_nsec,
                    (MYSQL_THD, enum enum_backup_lock_service_lock_kind,
                     ulonglong /* lock_timeout*/));

/**
  Service API to release Backup Lock.

  @param opaque_thd    Current thread context.

  @return Operation status.
    @retval false Success
    @retval true  Failure
*/

DECLARE_BOOL_METHOD(release, (MYSQL_THD));

END_SERVICE_DEFINITION(mysql_backup_lock)

#endif /* BACKUP_LOCK_SERVICE_H */
