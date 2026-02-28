/* Copyright (c) 2013, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef PFS_TRANSACTION_PROVIDER_H
#define PFS_TRANSACTION_PROVIDER_H

/**
  @file include/pfs_transaction_provider.h
  Performance schema instrumentation (declarations).
*/

#include "my_psi_config.h"

#ifdef HAVE_PSI_TRANSACTION_INTERFACE
#ifdef MYSQL_SERVER
#ifndef MYSQL_DYNAMIC_PLUGIN
#ifndef WITH_LOCK_ORDER

#include <sys/types.h>

#include "my_inttypes.h"
#include "my_macros.h"
#include "mysql/psi/psi_transaction.h"

struct PSI_transaction_locker;

#define PSI_TRANSACTION_CALL(M) pfs_##M##_v1

PSI_transaction_locker *pfs_get_thread_transaction_locker_v1(
    PSI_transaction_locker_state *state, const void *xid,
    const ulonglong *trxid, int isolation_level, bool read_only,
    bool autocommit);

void pfs_start_transaction_v1(PSI_transaction_locker *locker,
                              const char *src_file, uint src_line);

void pfs_set_transaction_xid_v1(PSI_transaction_locker *locker, const void *xid,
                                int xa_state);

void pfs_set_transaction_xa_state_v1(PSI_transaction_locker *locker,
                                     int xa_state);

void pfs_set_transaction_gtid_v1(PSI_transaction_locker *locker,
                                 const void *sid, const void *gtid_spec);

void pfs_set_transaction_trxid_v1(PSI_transaction_locker *locker,
                                  const ulonglong *trxid);

void pfs_inc_transaction_savepoints_v1(PSI_transaction_locker *locker,
                                       ulong count);

void pfs_inc_transaction_rollback_to_savepoint_v1(
    PSI_transaction_locker *locker, ulong count);

void pfs_inc_transaction_release_savepoint_v1(PSI_transaction_locker *locker,
                                              ulong count);

void pfs_end_transaction_v1(PSI_transaction_locker *locker, bool commit);

#endif /* WITH_LOCK_ORDER */
#endif /* MYSQL_DYNAMIC_PLUGIN */
#endif /* MYSQL_SERVER */
#endif /* HAVE_PSI_TRANSACTION_INTERFACE */

#endif
