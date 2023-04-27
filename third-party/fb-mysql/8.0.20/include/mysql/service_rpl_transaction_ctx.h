/* Copyright (c) 2014, 2017, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MYSQL_SERVICE_RPL_TRANSACTION_CTX_INCLUDED

/**
  @file include/mysql/service_rpl_transaction_ctx.h
  This service provides a function for plugins to report if a transaction of a
  given THD should continue or be aborted.

  SYNOPSIS
  set_transaction_ctx()
    should be called during RUN_HOOK macro, on which we know that thread is
    on plugin context and it is before
    Rpl_transaction_ctx::is_transaction_rollback() check.
*/

#ifndef MYSQL_ABI_CHECK
#include <stdlib.h>
#endif

struct Transaction_termination_ctx {
  unsigned long m_thread_id;
  unsigned int m_flags;  // reserved

  /*
    If the instruction is to rollback the transaction,
    then this flag is set to false.
   */
  bool m_rollback_transaction;

  /*
    If the plugin has generated a GTID, then the follwoing
    fields MUST be set.
   */
  bool m_generated_gtid;
  int m_sidno;
  long long int m_gno;
};

extern "C" struct rpl_transaction_ctx_service_st {
  int (*set_transaction_ctx)(
      Transaction_termination_ctx transaction_termination_ctx);
} * rpl_transaction_ctx_service;

#ifdef MYSQL_DYNAMIC_PLUGIN

#define set_transaction_ctx(transaction_termination_ctx) \
  (rpl_transaction_ctx_service->set_transaction_ctx(     \
      (transaction_termination_ctx)))

#else

int set_transaction_ctx(
    Transaction_termination_ctx transaction_termination_ctx);

#endif

#define MYSQL_SERVICE_RPL_TRANSACTION_CTX_INCLUDED
#endif
