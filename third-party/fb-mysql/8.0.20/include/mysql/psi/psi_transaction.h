/* Copyright (c) 2008, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MYSQL_PSI_TRANSACTION_H
#define MYSQL_PSI_TRANSACTION_H

/**
  @file include/mysql/psi/psi_transaction.h
  Performance schema instrumentation interface.

  @defgroup psi_abi_transaction Transaction Instrumentation (ABI)
  @ingroup psi_abi
  @{
*/

#include "my_inttypes.h"
#include "my_macros.h"
#include "my_psi_config.h"  // IWYU pragma: keep
#include "my_sharedlib.h"
#include "mysql/components/services/psi_transaction_bits.h"

/**
  @def PSI_TRANSACTION_VERSION_1
  Performance Schema Transaction Interface number for version 1.
  This version is supported.
*/
#define PSI_TRANSACTION_VERSION_1 1

/**
  @def PSI_CURRENT_TRANSACTION_VERSION
  Performance Schema Transaction Interface number for the most recent version.
  The most current version is @c PSI_TRANSACTION_VERSION_1
*/
#define PSI_CURRENT_TRANSACTION_VERSION 1

/** Entry point for the performance schema interface. */
struct PSI_transaction_bootstrap {
  /**
    ABI interface finder.
    Calling this method with an interface version number returns either
    an instance of the ABI for this version, or NULL.
    @sa PSI_TRANSACTION_VERSION_1
    @sa PSI_TRANSACTION_VERSION_2
    @sa PSI_CURRENT_TRANSACTION_VERSION
  */
  void *(*get_interface)(int version);
};
typedef struct PSI_transaction_bootstrap PSI_transaction_bootstrap;

#ifdef HAVE_PSI_TRANSACTION_INTERFACE

/**
  Performance Schema Transaction Interface, version 1.
  @since PSI_TRANSACTION_VERSION_1
*/
struct PSI_transaction_service_v1 {
  /** @sa get_thread_transaction_locker_v1_t. */
  get_thread_transaction_locker_v1_t get_thread_transaction_locker;
  /** @sa start_transaction_v1_t. */
  start_transaction_v1_t start_transaction;
  /** @sa set_transaction_xid_v1_t. */
  set_transaction_xid_v1_t set_transaction_xid;
  /** @sa set_transaction_xa_state_v1_t. */
  set_transaction_xa_state_v1_t set_transaction_xa_state;
  /** @sa set_transaction_gtid_v1_t. */
  set_transaction_gtid_v1_t set_transaction_gtid;
  /** @sa set_transaction_trxid_v1_t. */
  set_transaction_trxid_v1_t set_transaction_trxid;
  /** @sa inc_transaction_savepoints_v1_t. */
  inc_transaction_savepoints_v1_t inc_transaction_savepoints;
  /** @sa inc_transaction_rollback_to_savepoint_v1_t. */
  inc_transaction_rollback_to_savepoint_v1_t
      inc_transaction_rollback_to_savepoint;
  /** @sa inc_transaction_release_savepoint_v1_t. */
  inc_transaction_release_savepoint_v1_t inc_transaction_release_savepoint;
  /** @sa end_transaction_v1_t. */
  end_transaction_v1_t end_transaction;
};

typedef struct PSI_transaction_service_v1 PSI_transaction_service_t;

extern MYSQL_PLUGIN_IMPORT PSI_transaction_service_t *psi_transaction_service;

#endif /* HAVE_PSI_TRANSACTION_INTERFACE */

/** @} (end of group psi_abi_transaction) */

#endif /* MYSQL_PSI_TRANSACTION_H */
