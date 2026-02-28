/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/rpl_transaction_ctx.h"

#include <stddef.h>
#include <sys/types.h>

#include "my_dbug.h"
#include "my_inttypes.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysqld_error.h"
#include "sql/mysqld_thd_manager.h"  // Global_THD_manager
#include "sql/rpl_gtid.h"            // rpl_sidno
#include "sql/sql_class.h"           // THD
#include "sql/transaction_info.h"

Rpl_transaction_ctx::Rpl_transaction_ctx() {
  DBUG_TRACE;
  cleanup();
}

void Rpl_transaction_ctx::cleanup() {
  DBUG_TRACE;
  m_transaction_ctx.m_thread_id = 0;
  m_transaction_ctx.m_flags = 0;
  m_transaction_ctx.m_rollback_transaction = false;
  m_transaction_ctx.m_generated_gtid = false;
  m_transaction_ctx.m_sidno = 0;
  m_transaction_ctx.m_gno = 0;
}

int Rpl_transaction_ctx::set_rpl_transaction_ctx(
    Transaction_termination_ctx transaction_termination_ctx) {
  DBUG_TRACE;

  if (transaction_termination_ctx.m_generated_gtid) {
    if (transaction_termination_ctx.m_rollback_transaction ||
        transaction_termination_ctx.m_sidno <= 0 ||
        transaction_termination_ctx.m_gno <= 0)
      return 1;
  }

  m_transaction_ctx = transaction_termination_ctx;
  return 0;
}

bool Rpl_transaction_ctx::is_transaction_rollback() {
  DBUG_TRACE;
  return m_transaction_ctx.m_rollback_transaction;
}

bool Rpl_transaction_ctx::is_generated_gtid() {
  DBUG_TRACE;
  return m_transaction_ctx.m_generated_gtid;
}

rpl_sidno Rpl_transaction_ctx::get_sidno() {
  DBUG_TRACE;
  return m_transaction_ctx.m_sidno;
}

rpl_gno Rpl_transaction_ctx::get_gno() {
  DBUG_TRACE;
  return m_transaction_ctx.m_gno;
}

/**
   Implementation of service_transaction_veredict, see
   @file include/mysql/service_rpl_transaction_ctx.h
*/
int set_transaction_ctx(
    Transaction_termination_ctx transaction_termination_ctx) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("thread_id=%lu, rollback_transaction=%d, "
                       "generated_gtid=%d, sidno=%d, gno=%lld",
                       transaction_termination_ctx.m_thread_id,
                       transaction_termination_ctx.m_rollback_transaction,
                       transaction_termination_ctx.m_generated_gtid,
                       transaction_termination_ctx.m_sidno,
                       transaction_termination_ctx.m_gno));

  THD *thd = nullptr;
  uint error = ER_NO_SUCH_THREAD;
  Find_thd_with_id find_thd_with_id(transaction_termination_ctx.m_thread_id);

  thd = Global_THD_manager::get_instance()->find_thd(&find_thd_with_id);
  if (thd) {
    error = thd->get_transaction()
                ->get_rpl_transaction_ctx()
                ->set_rpl_transaction_ctx(transaction_termination_ctx);
    mysql_mutex_unlock(&thd->LOCK_thd_data);
  }
  return error;
}
