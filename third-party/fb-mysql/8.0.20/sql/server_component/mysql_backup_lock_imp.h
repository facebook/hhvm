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

#ifndef MYSQL_BACKUP_LOCK_INCLUDED
#define MYSQL_BACKUP_LOCK_INCLUDED

#include <mysql/components/component_implementation.h>
#include <mysql/components/services/backup_lock_service.h>

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

DEFINE_BOOL_METHOD(mysql_acquire_backup_lock_nsec,
                   (MYSQL_THD opaque_thd,
                    enum enum_backup_lock_service_lock_kind lock_kind,
                    ulonglong lock_timeout));

/**
  Service API to release Backup Lock.

  @param opaque_thd    Current thread context.

  @return Operation status.
    @retval false Success
    @retval true  Failure
*/

DEFINE_BOOL_METHOD(mysql_release_backup_lock, (MYSQL_THD opaque_thd));

#endif /* COMPONENTS_MYSQL_SERVER_MYSQL_BACKUP_LOCK_H_ */
