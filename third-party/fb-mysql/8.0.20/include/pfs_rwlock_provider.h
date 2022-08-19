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

#ifndef PFS_RWLOCK_PROVIDER_H
#define PFS_RWLOCK_PROVIDER_H

/**
  @file include/pfs_rwlock_provider.h
  Performance schema instrumentation (declarations).
*/

#include <sys/types.h>

#include "my_psi_config.h"

#ifdef HAVE_PSI_RWLOCK_INTERFACE
#ifdef MYSQL_SERVER
#ifndef MYSQL_DYNAMIC_PLUGIN
#ifndef WITH_LOCK_ORDER

#include "my_inttypes.h"
#include "my_macros.h"
#include "mysql/psi/psi_rwlock.h"

#define PSI_RWLOCK_CALL(M) pfs_##M##_v2

void pfs_register_rwlock_v2(const char *category, PSI_rwlock_info_v1 *info,
                            int count);

PSI_rwlock *pfs_init_rwlock_v2(PSI_rwlock_key key, const void *identity);

void pfs_destroy_rwlock_v2(PSI_rwlock *rwlock);

PSI_rwlock_locker *pfs_start_rwlock_rdwait_v2(PSI_rwlock_locker_state *state,
                                              PSI_rwlock *rwlock,
                                              PSI_rwlock_operation op,
                                              const char *src_file,
                                              uint src_line);

PSI_rwlock_locker *pfs_start_rwlock_wrwait_v2(PSI_rwlock_locker_state *state,
                                              PSI_rwlock *rwlock,
                                              PSI_rwlock_operation op,
                                              const char *src_file,
                                              uint src_line);

void pfs_unlock_rwlock_v2(PSI_rwlock *rwlock, PSI_rwlock_operation op);

void pfs_end_rwlock_rdwait_v2(PSI_rwlock_locker *locker, int rc);

void pfs_end_rwlock_wrwait_v2(PSI_rwlock_locker *locker, int rc);

#endif /* WITH_LOCK_ORDER */
#endif /* MYSQL_DYNAMIC_PLUGIN */
#endif /* MYSQL_SERVER */
#endif /* HAVE_PSI_RWLOCK_INTERFACE */

#endif
