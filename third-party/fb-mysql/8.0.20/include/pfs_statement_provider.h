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

#ifndef PFS_STATEMENT_PROVIDER_H
#define PFS_STATEMENT_PROVIDER_H

/**
  @file include/pfs_statement_provider.h
  Performance schema instrumentation (declarations).
*/

#include <sys/types.h>

#include "my_psi_config.h"

#ifdef HAVE_PSI_STATEMENT_INTERFACE
#ifdef MYSQL_SERVER
#ifndef MYSQL_DYNAMIC_PLUGIN
#ifndef WITH_LOCK_ORDER

#include "my_inttypes.h"
#include "my_macros.h"
#include "mysql/psi/psi_statement.h"
#include "sql/sql_digest.h"

struct PSI_digest_locker;
struct sql_digest_storage;

#define PSI_STATEMENT_CALL(M) pfs_##M##_v2
#define PSI_DIGEST_CALL(M) pfs_##M##_v2

void pfs_register_statement_v2(const char *category, PSI_statement_info *info,
                               int count);

PSI_statement_locker *pfs_get_thread_statement_locker_v2(
    PSI_statement_locker_state *state, PSI_statement_key key,
    const void *charset, PSI_sp_share *sp_share);

PSI_statement_locker *pfs_refine_statement_v2(PSI_statement_locker *locker,
                                              PSI_statement_key key);

void pfs_start_statement_v2(PSI_statement_locker *locker, const char *db,
                            uint db_len, const char *src_file, uint src_line);

void pfs_set_statement_text_v2(PSI_statement_locker *locker, const char *text,
                               uint text_len);

void pfs_set_statement_query_id_v2(PSI_statement_locker *locker,
                                   ulonglong count);

void pfs_set_statement_lock_time_v2(PSI_statement_locker *locker,
                                    ulonglong count);

void pfs_set_statement_cpu_time_v2(PSI_statement_locker *locker,
                                   ulonglong count);

void pfs_set_statement_rows_sent_v2(PSI_statement_locker *locker,
                                    ulonglong count);

void pfs_set_statement_rows_examined_v2(PSI_statement_locker *locker,
                                        ulonglong count);

void pfs_inc_statement_rows_deleted_v2(PSI_statement_locker *locker,
                                       ulonglong count);

void pfs_inc_statement_rows_inserted_v2(PSI_statement_locker *locker,
                                        ulonglong count);

void pfs_inc_statement_rows_updated_v2(PSI_statement_locker *locker,
                                       ulonglong count);

void pfs_inc_statement_tmp_table_bytes_written_v2(PSI_statement_locker *locker,
                                                  ulonglong count);

void pfs_inc_statement_filesort_bytes_written_v2(PSI_statement_locker *locker,
                                                 ulonglong count);

void pfs_inc_statement_index_dive_count_v2(PSI_statement_locker *locker,
                                           ulong count);

void pfs_inc_statement_index_dive_cpu_v2(PSI_statement_locker *locker,
                                         ulonglong count);

void pfs_inc_statement_compilation_cpu_v2(PSI_statement_locker *locker,
                                          ulonglong count);

void pfs_inc_statement_created_tmp_disk_tables_v2(PSI_statement_locker *locker,
                                                  ulong count);

void pfs_inc_statement_created_tmp_tables_v2(PSI_statement_locker *locker,
                                             ulong count);

void pfs_inc_statement_select_full_join_v2(PSI_statement_locker *locker,
                                           ulong count);

void pfs_inc_statement_select_full_range_join_v2(PSI_statement_locker *locker,
                                                 ulong count);

void pfs_inc_statement_select_range_v2(PSI_statement_locker *locker,
                                       ulong count);

void pfs_inc_statement_select_range_check_v2(PSI_statement_locker *locker,
                                             ulong count);

void pfs_inc_statement_select_scan_v2(PSI_statement_locker *locker,
                                      ulong count);

void pfs_inc_statement_sort_merge_passes_v2(PSI_statement_locker *locker,
                                            ulong count);

void pfs_inc_statement_sort_range_v2(PSI_statement_locker *locker, ulong count);

void pfs_inc_statement_sort_rows_v2(PSI_statement_locker *locker, ulong count);

void pfs_inc_statement_sort_scan_v2(PSI_statement_locker *locker, ulong count);

void pfs_set_statement_no_index_used_v2(PSI_statement_locker *locker);

void pfs_set_statement_no_good_index_used_v2(PSI_statement_locker *locker);

void pfs_update_statement_filesort_disk_usage_v2(PSI_statement_locker *locker,
                                                 ulonglong value);

void pfs_update_statement_tmp_table_disk_usage_v2(PSI_statement_locker *locker,
                                                  ulonglong value);

void pfs_end_statement_v2(PSI_statement_locker *locker, void *stmt_da);

PSI_digest_locker *pfs_digest_start_v2(PSI_statement_locker *locker);

void pfs_digest_end_v2(PSI_digest_locker *locker,
                       const sql_digest_storage *digest);

#endif /* WITH_LOCK_ORDER */
#endif /* MYSQL_DYNAMIC_PLUGIN */
#endif /* MYSQL_SERVER */
#endif /* HAVE_PSI_STATEMENT_INTERFACE */

#endif
