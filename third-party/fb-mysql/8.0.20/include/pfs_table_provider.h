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

#ifndef PFS_TABLE_PROVIDER_H
#define PFS_TABLE_PROVIDER_H

/**
  @file include/pfs_table_provider.h
  Performance schema instrumentation (declarations).
*/

#include <sys/types.h>

#include "my_psi_config.h"

#ifdef HAVE_PSI_TABLE_INTERFACE
#ifdef MYSQL_SERVER
#ifndef MYSQL_DYNAMIC_PLUGIN
#ifndef WITH_LOCK_ORDER

#include "my_inttypes.h"
#include "my_macros.h"
#include "mysql/psi/psi_table.h"

#define PSI_TABLE_CALL(M) pfs_##M##_v1

PSI_table_share *pfs_get_table_share_v1(bool temporary,
                                        struct TABLE_SHARE *share);

void pfs_release_table_share_v1(PSI_table_share *share);

void pfs_drop_table_share_v1(bool temporary, const char *schema_name,
                             int schema_name_length, const char *table_name,
                             int table_name_length);

PSI_table *pfs_open_table_v1(PSI_table_share *share, const void *identity);

void pfs_unbind_table_v1(PSI_table *table);

PSI_table *pfs_rebind_table_v1(PSI_table_share *share, const void *identity,
                               PSI_table *table);

void pfs_close_table_v1(struct TABLE_SHARE *server_share, PSI_table *table);

PSI_table_locker *pfs_start_table_io_wait_v1(PSI_table_locker_state *state,
                                             PSI_table *table,
                                             PSI_table_io_operation op,
                                             uint index, const char *src_file,
                                             uint src_line);

PSI_table_locker *pfs_start_table_lock_wait_v1(PSI_table_locker_state *state,
                                               PSI_table *table,
                                               PSI_table_lock_operation op,
                                               ulong op_flags,
                                               const char *src_file,
                                               uint src_line);

void pfs_end_table_io_wait_v1(PSI_table_locker *locker, ulonglong numrows);

void pfs_end_table_lock_wait_v1(PSI_table_locker *locker);

void pfs_unlock_table_v1(PSI_table *table);

#endif /* WITH_LOCK_ORDER */
#endif /* MYSQL_DYNAMIC_PLUGIN */
#endif /* MYSQL_SERVER */
#endif /* HAVE_PSI_TABLE_INTERFACE */

#endif
