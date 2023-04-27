/* Copyright (c) 2012, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef PFS_MUTEX_PROVIDER_H
#define PFS_MUTEX_PROVIDER_H

/**
  @file include/pfs_mutex_provider.h
  Performance schema instrumentation (declarations).
*/

#include <sys/types.h>

#include "my_psi_config.h"

#ifdef HAVE_PSI_MUTEX_INTERFACE
#ifdef MYSQL_SERVER
#ifndef MYSQL_DYNAMIC_PLUGIN
#ifndef WITH_LOCK_ORDER

#include "my_inttypes.h"
#include "my_macros.h"
#include "mysql/psi/psi_mutex.h"

#define PSI_MUTEX_CALL(M) pfs_##M##_v1

void pfs_register_mutex_v1(const char *category, PSI_mutex_info_v1 *info,
                           int count);

PSI_mutex *pfs_init_mutex_v1(PSI_mutex_key key, const void *identity);

void pfs_destroy_mutex_v1(PSI_mutex *mutex);

PSI_mutex_locker *pfs_start_mutex_wait_v1(PSI_mutex_locker_state *state,
                                          PSI_mutex *mutex,
                                          PSI_mutex_operation op,
                                          const char *src_file, uint src_line);

void pfs_unlock_mutex_v1(PSI_mutex *mutex);

void pfs_end_mutex_wait_v1(PSI_mutex_locker *locker, int rc);

#endif /* WITH_LOCK_ORDER */
#endif /* MYSQL_DYNAMIC_PLUGIN */
#endif /* MYSQL_SERVER */
#endif /* HAVE_PSI_MUTEX_INTERFACE */

#endif /* PFS_MUTEX_PROVIDER_H */
