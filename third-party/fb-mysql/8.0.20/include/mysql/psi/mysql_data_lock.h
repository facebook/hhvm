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

#ifndef MYSQL_DATA_LOCK_H
#define MYSQL_DATA_LOCK_H

/**
  @file include/mysql/psi/mysql_data_lock.h
  Instrumentation helpers for data locks.
*/

#include "mysql/psi/psi_data_lock.h"

#ifndef PSI_DATA_LOCK_CALL
#define PSI_DATA_LOCK_CALL(M) psi_data_lock_service->M
#endif

/**
  @defgroup psi_api_data_lock Data Lock Instrumentation (API)
  @ingroup psi_api
  @{
*/

#define mysql_data_lock_register(I) inline_mysql_data_lock_register(I)

void inline_mysql_data_lock_register(
#ifdef HAVE_PSI_DATA_LOCK_INTERFACE
    PSI_engine_data_lock_inspector *i
#else
    PSI_engine_data_lock_inspector *i MY_ATTRIBUTE((unused))
#endif /* HAVE_PSI_DATA_LOCK_INTERFACE */
) {
#ifdef HAVE_PSI_DATA_LOCK_INTERFACE
  PSI_DATA_LOCK_CALL(register_data_lock)(i);
#endif /* HAVE_PSI_DATA_LOCK_INTERFACE */
}

#define mysql_data_lock_unregister(I) inline_mysql_data_lock_unregister(I)

void inline_mysql_data_lock_unregister(
#ifdef HAVE_PSI_DATA_LOCK_INTERFACE
    PSI_engine_data_lock_inspector *i
#else
    PSI_engine_data_lock_inspector *i MY_ATTRIBUTE((unused))
#endif /* HAVE_PSI_DATA_LOCK_INTERFACE */
) {
#ifdef HAVE_PSI_DATA_LOCK_INTERFACE
  PSI_DATA_LOCK_CALL(unregister_data_lock)(i);
#endif /* HAVE_PSI_DATA_LOCK_INTERFACE */
}

/** @} (end of group psi_api_data_lock) */

#endif
