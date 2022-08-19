/* Copyright (c) 2013, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MYSQL_TRANSACTION_H
#define MYSQL_TRANSACTION_H

/**
  @file include/mysql/psi/mysql_transaction.h
  Instrumentation helpers for transactions.
*/

#include "mysql/psi/psi_transaction.h"

#include "my_inttypes.h"
#include "pfs_transaction_provider.h"

#ifndef PSI_TRANSACTION_CALL
#define PSI_TRANSACTION_CALL(M) psi_transaction_service->M
#endif

/**
  @defgroup psi_api_transaction Transaction Instrumentation (API)
  @ingroup psi_api
  @{
*/

#ifdef HAVE_PSI_TRANSACTION_INTERFACE
#define MYSQL_START_TRANSACTION(STATE, XID, TRXID, ISO, RO, AC)            \
  inline_mysql_start_transaction(STATE, XID, TRXID, ISO, RO, AC, __FILE__, \
                                 __LINE__)
#else
#define MYSQL_START_TRANSACTION(STATE, XID, TRXID, ISO, RO, AC) \
  do {                                                          \
  } while (0)
#endif

#ifdef HAVE_PSI_TRANSACTION_INTERFACE
#define MYSQL_SET_TRANSACTION_GTID(LOCKER, P1, P2) \
  inline_mysql_set_transaction_gtid(LOCKER, P1, P2)
#else
#define MYSQL_SET_TRANSACTION_GTID(LOCKER, P1, P2) \
  do {                                             \
  } while (0)
#endif

#ifdef HAVE_PSI_TRANSACTION_INTERFACE
#define MYSQL_SET_TRANSACTION_XID(LOCKER, P1, P2) \
  inline_mysql_set_transaction_xid(LOCKER, P1, P2)
#else
#define MYSQL_SET_TRANSACTION_XID(LOCKER, P1, P2) \
  do {                                            \
  } while (0)
#endif

#ifdef HAVE_PSI_TRANSACTION_INTERFACE
#define MYSQL_SET_TRANSACTION_XA_STATE(LOCKER, P1) \
  inline_mysql_set_transaction_xa_state(LOCKER, P1)
#else
#define MYSQL_SET_TRANSACTION_XA_STATE(LOCKER, P1) \
  do {                                             \
  } while (0)
#endif

#ifdef HAVE_PSI_TRANSACTION_INTERFACE
#define MYSQL_SET_TRANSACTION_TRXID(LOCKER, P1) \
  inline_mysql_set_transaction_trxid(LOCKER, P1)
#else
#define MYSQL_SET_TRANSACTION_TRXID(LOCKER, P1) \
  do {                                          \
  } while (0)
#endif

#ifdef HAVE_PSI_TRANSACTION_INTERFACE
#define MYSQL_INC_TRANSACTION_SAVEPOINTS(LOCKER, P1) \
  inline_mysql_inc_transaction_savepoints(LOCKER, P1)
#else
#define MYSQL_INC_TRANSACTION_SAVEPOINTS(LOCKER, P1) \
  do {                                               \
  } while (0)
#endif

#ifdef HAVE_PSI_TRANSACTION_INTERFACE
#define MYSQL_INC_TRANSACTION_ROLLBACK_TO_SAVEPOINT(LOCKER, P1) \
  inline_mysql_inc_transaction_rollback_to_savepoint(LOCKER, P1)
#else
#define MYSQL_INC_TRANSACTION_ROLLBACK_TO_SAVEPOINT(LOCKER, P1) \
  do {                                                          \
  } while (0)
#endif

#ifdef HAVE_PSI_TRANSACTION_INTERFACE
#define MYSQL_INC_TRANSACTION_RELEASE_SAVEPOINT(LOCKER, P1) \
  inline_mysql_inc_transaction_release_savepoint(LOCKER, P1)
#else
#define MYSQL_INC_TRANSACTION_RELEASE_SAVEPOINT(LOCKER, P1) \
  do {                                                      \
  } while (0)
#endif

#ifdef HAVE_PSI_TRANSACTION_INTERFACE
#define MYSQL_ROLLBACK_TRANSACTION(LOCKER) \
  inline_mysql_rollback_transaction(LOCKER)
#else
#define MYSQL_ROLLBACK_TRANSACTION(LOCKER) NULL
#endif

#ifdef HAVE_PSI_TRANSACTION_INTERFACE
#define MYSQL_COMMIT_TRANSACTION(LOCKER) inline_mysql_commit_transaction(LOCKER)
#else
#define MYSQL_COMMIT_TRANSACTION(LOCKER) NULL
#endif

#ifdef HAVE_PSI_TRANSACTION_INTERFACE
static inline struct PSI_transaction_locker *inline_mysql_start_transaction(
    PSI_transaction_locker_state *state, const void *xid,
    const ulonglong *trxid, int isolation_level, bool read_only,
    bool autocommit, const char *src_file, int src_line) {
  PSI_transaction_locker *locker;
  locker = PSI_TRANSACTION_CALL(get_thread_transaction_locker)(
      state, xid, trxid, isolation_level, read_only, autocommit);
  if (likely(locker != nullptr)) {
    PSI_TRANSACTION_CALL(start_transaction)(locker, src_file, src_line);
  }
  return locker;
}

static inline void inline_mysql_set_transaction_gtid(
    PSI_transaction_locker *locker, const void *sid, const void *gtid_spec) {
  if (likely(locker != nullptr)) {
    PSI_TRANSACTION_CALL(set_transaction_gtid)(locker, sid, gtid_spec);
  }
}

static inline void inline_mysql_set_transaction_xid(
    PSI_transaction_locker *locker, const void *xid, int xa_state) {
  if (likely(locker != nullptr)) {
    PSI_TRANSACTION_CALL(set_transaction_xid)(locker, xid, xa_state);
  }
}

static inline void inline_mysql_set_transaction_xa_state(
    PSI_transaction_locker *locker, int xa_state) {
  if (likely(locker != nullptr)) {
    PSI_TRANSACTION_CALL(set_transaction_xa_state)(locker, xa_state);
  }
}

static inline void inline_mysql_set_transaction_trxid(
    PSI_transaction_locker *locker, const ulonglong *trxid) {
  if (likely(locker != nullptr)) {
    PSI_TRANSACTION_CALL(set_transaction_trxid)(locker, trxid);
  }
}

static inline void inline_mysql_inc_transaction_savepoints(
    PSI_transaction_locker *locker, ulong count) {
  if (likely(locker != nullptr)) {
    PSI_TRANSACTION_CALL(inc_transaction_savepoints)(locker, count);
  }
}

static inline void inline_mysql_inc_transaction_rollback_to_savepoint(
    PSI_transaction_locker *locker, ulong count) {
  if (likely(locker != nullptr)) {
    PSI_TRANSACTION_CALL(inc_transaction_rollback_to_savepoint)(locker, count);
  }
}

static inline void inline_mysql_inc_transaction_release_savepoint(
    PSI_transaction_locker *locker, ulong count) {
  if (likely(locker != nullptr)) {
    PSI_TRANSACTION_CALL(inc_transaction_release_savepoint)(locker, count);
  }
}

static inline void inline_mysql_rollback_transaction(
    struct PSI_transaction_locker *locker) {
  if (likely(locker != nullptr)) {
    PSI_TRANSACTION_CALL(end_transaction)(locker, false);
  }
}

static inline void inline_mysql_commit_transaction(
    struct PSI_transaction_locker *locker) {
  if (likely(locker != nullptr)) {
    PSI_TRANSACTION_CALL(end_transaction)(locker, true);
  }
}
#endif

/** @} (end of group psi_api_transaction) */

#endif
