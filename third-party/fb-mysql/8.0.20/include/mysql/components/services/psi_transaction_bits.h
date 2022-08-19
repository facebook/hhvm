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

#ifndef COMPONENTS_SERVICES_PSI_TRANSACTION_BITS_H
#define COMPONENTS_SERVICES_PSI_TRANSACTION_BITS_H

/**
  @file
  Performance schema instrumentation interface.

  @defgroup psi_abi_transaction Transaction Instrumentation (ABI)
  @ingroup psi_abi
  @{
*/

/**
  Interface for an instrumented transaction.
  This is an opaque structure.
*/
struct PSI_transaction_locker;
typedef struct PSI_transaction_locker PSI_transaction_locker;

/**
  State data storage for @c get_thread_transaction_locker_v1_t,
  @c get_thread_transaction_locker_v1_t.
  This structure provide temporary storage to a transaction locker.
  The content of this structure is considered opaque,
  the fields are only hints of what an implementation
  of the psi interface can use.
  This memory is provided by the instrumented code for performance reasons.
  @sa get_thread_transaction_locker_v1_t
*/
struct PSI_transaction_locker_state_v1 {
  /** Internal state. */
  unsigned int m_flags;
  /** Instrumentation class. */
  void *m_class;
  /** Current thread. */
  struct PSI_thread *m_thread;
  /** Timer start. */
  unsigned long long m_timer_start;
  /** Timer function. */
  unsigned long long (*m_timer)(void);
  /** Internal data. */
  void *m_transaction;
  /** True if read-only transaction, false if read-write. */
  bool m_read_only;
  /** True if transaction is autocommit. */
  bool m_autocommit;
  /** Number of statements. */
  unsigned long m_statement_count;
  /** Total number of savepoints. */
  unsigned long m_savepoint_count;
  /** Number of rollback_to_savepoint. */
  unsigned long m_rollback_to_savepoint_count;
  /** Number of release_savepoint. */
  unsigned long m_release_savepoint_count;
};
typedef struct PSI_transaction_locker_state_v1 PSI_transaction_locker_state_v1;

/**
  Get a transaction instrumentation locker.
  @param state data storage for the locker
  @param xid the xid for this transaction
  @param trxid the InnoDB transaction id
  @param isolation_level isolation level for this transaction
  @param read_only true if transaction access mode is read-only
  @param autocommit true if transaction is autocommit
  @return a transaction locker, or NULL
*/
typedef struct PSI_transaction_locker *(*get_thread_transaction_locker_v1_t)(
    struct PSI_transaction_locker_state_v1 *state, const void *xid,
    const unsigned long long *trxid, int isolation_level, bool read_only,
    bool autocommit);

/**
  Start a new transaction event.
  @param locker the transaction locker for this event
  @param src_file source file name
  @param src_line source line number
*/
typedef void (*start_transaction_v1_t)(struct PSI_transaction_locker *locker,
                                       const char *src_file,
                                       unsigned int src_line);

/**
  Set the transaction xid.
  @param locker the transaction locker for this event
  @param xid the id of the XA transaction
  @param xa_state the state of the XA transaction
*/
typedef void (*set_transaction_xid_v1_t)(struct PSI_transaction_locker *locker,
                                         const void *xid, int xa_state);

/**
  Set the state of the XA transaction.
  @param locker the transaction locker for this event
  @param xa_state the new state of the xa transaction
*/
typedef void (*set_transaction_xa_state_v1_t)(
    struct PSI_transaction_locker *locker, int xa_state);

/**
  Set the transaction gtid.
  @param locker the transaction locker for this event
  @param sid the source id for the transaction, mapped from sidno
  @param gtid_spec the gtid specifier for the transaction
*/
typedef void (*set_transaction_gtid_v1_t)(struct PSI_transaction_locker *locker,
                                          const void *sid,
                                          const void *gtid_spec);

/**
  Set the transaction trx_id.
  @param locker the transaction locker for this event
  @param trxid the storage engine transaction ID
*/
typedef void (*set_transaction_trxid_v1_t)(
    struct PSI_transaction_locker *locker, const unsigned long long *trxid);

/**
  Increment a transaction event savepoint count.
  @param locker the transaction locker
  @param count the increment value
*/
typedef void (*inc_transaction_savepoints_v1_t)(
    struct PSI_transaction_locker *locker, unsigned long count);

/**
  Increment a transaction event rollback to savepoint count.
  @param locker the transaction locker
  @param count the increment value
*/
typedef void (*inc_transaction_rollback_to_savepoint_v1_t)(
    struct PSI_transaction_locker *locker, unsigned long count);

/**
  Increment a transaction event release savepoint count.
  @param locker the transaction locker
  @param count the increment value
*/
typedef void (*inc_transaction_release_savepoint_v1_t)(
    struct PSI_transaction_locker *locker, unsigned long count);

/**
  Commit or rollback the transaction.
  @param locker the transaction locker for this event
  @param commit true if transaction was committed, false if rolled back
*/
typedef void (*end_transaction_v1_t)(struct PSI_transaction_locker *locker,
                                     bool commit);

typedef struct PSI_transaction_locker_state_v1 PSI_transaction_locker_state;

/** @} (end of group psi_abi_transaction) */

#endif /* COMPONENTS_SERVICES_PSI_TRANSACTION_BITS_H */
