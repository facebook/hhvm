/*
   Copyright (c) 2013, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/transaction_info.h"

#include <string.h>

#include "mysqld_error.h"          // ER_*
#include "sql/derror.h"            // ER_THD
#include "sql/mysqld.h"            // global_system_variables
#include "sql/psi_memory_key.h"    // key_memory_thd_transactions
#include "sql/sql_error.h"         // Sql_condition
#include "sql/system_variables.h"  // System_variables
#include "sql/thr_malloc.h"

struct CHANGED_TABLE_LIST {
  struct CHANGED_TABLE_LIST *next;
  char *key;
  uint32 key_length;
};

Transaction_ctx::Transaction_ctx()
    : m_savepoints(nullptr),
      m_xid_state(),
      last_committed(0),
      sequence_number(0),
      m_rpl_transaction_ctx(),
      m_transaction_write_set_ctx(),
      trans_begin_hook_invoked(false) {
  memset(&m_scope_info, 0, sizeof(m_scope_info));
  memset(&m_flags, 0, sizeof(m_flags));
  init_sql_alloc(key_memory_thd_transactions, &m_mem_root,
                 global_system_variables.trans_alloc_block_size,
                 global_system_variables.trans_prealloc_size);
}

void Transaction_ctx::push_unsafe_rollback_warnings(THD *thd) {
  if (m_scope_info[SESSION].has_modified_non_trans_table())
    push_warning(thd, Sql_condition::SL_WARNING,
                 ER_WARNING_NOT_COMPLETE_ROLLBACK,
                 ER_THD(thd, ER_WARNING_NOT_COMPLETE_ROLLBACK));

  if (m_scope_info[SESSION].has_created_temp_table())
    push_warning(
        thd, Sql_condition::SL_WARNING,
        ER_WARNING_NOT_COMPLETE_ROLLBACK_WITH_CREATED_TEMP_TABLE,
        ER_THD(thd, ER_WARNING_NOT_COMPLETE_ROLLBACK_WITH_CREATED_TEMP_TABLE));

  if (m_scope_info[SESSION].has_dropped_temp_table())
    push_warning(
        thd, Sql_condition::SL_WARNING,
        ER_WARNING_NOT_COMPLETE_ROLLBACK_WITH_DROPPED_TEMP_TABLE,
        ER_THD(thd, ER_WARNING_NOT_COMPLETE_ROLLBACK_WITH_DROPPED_TEMP_TABLE));
}

void Transaction_ctx::register_ha(enum_trx_scope scope, Ha_trx_info *ha_info,
                                  handlerton *ht) {
  ha_info->register_ha(&m_scope_info[scope], ht);
}
