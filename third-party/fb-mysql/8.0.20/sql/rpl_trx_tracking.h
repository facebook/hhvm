#ifndef RPL_TRX_TRACKING_INCLUDED
/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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

#define RPL_TRX_TRACKING_INCLUDED

#include <sys/types.h>
#include <atomic>
#include <map>

#include "libbinlogevents/include/binlog_event.h"
#include "my_dbug.h"
#include "my_inttypes.h"

class THD;

/**
  Logical timestamp generator for logical timestamping binlog transactions.
  A transaction is associated with two sequence numbers see
  @c Transaction_ctx::last_committed and @c Transaction_ctx::sequence_number.
  The class provides necessary interfaces including that of
  generating a next consecutive value for the latter.
*/
class Logical_clock {
 private:
  std::atomic<int64> state;
  /*
    Offset is subtracted from the actual "absolute time" value at
    logging a replication event. That is the event holds logical
    timestamps in the "relative" format. They are meaningful only in
    the context of the current binlog.
    The member is updated (incremented) per binary log rotation.
  */
  int64 offset;

 public:
  Logical_clock();
  Logical_clock(const Logical_clock &other)
      : state(other.state.load()), offset(other.offset) {}

  int64 step();
  int64 set_if_greater(int64 new_val);
  int64 get_timestamp();
  int64 get_offset() { return offset; }
  /*
    Updates the offset.
    This operation is invoked when binlog rotates and at that time
    there can't any concurrent step() callers so no need to guard
    the assignement.
  */
  void update_offset(int64 new_offset) {
    DBUG_ASSERT(offset <= new_offset);

    offset = new_offset;
  }
  ~Logical_clock() {}
};

/**
  Generate logical timestamps for MTS using COMMIT_ORDER
  in the binlog-transaction-dependency-tracking option.
*/
class Commit_order_trx_dependency_tracker {
 public:
  /**
    Main function that gets the dependencies using the COMMIT_ORDER tracker.

    @param [in]     thd             THD of the caller.
    @param [in,out] sequence_number sequence_number initialized and returned.
    @param [in,out] commit_parent   commit_parent to be returned.
   */
  void get_dependency(THD *thd, int64 &sequence_number, int64 &commit_parent);

  void update_max_committed(int64 sequence_number);

  Logical_clock get_max_committed_transaction() {
    return m_max_committed_transaction;
  }

  int64 step();
  void rotate();

 private:
  /* Committed transactions timestamp */
  Logical_clock m_max_committed_transaction;

  /* "Prepared" transactions timestamp */
  Logical_clock m_transaction_counter;

  /*
    Stores the last sequence_number of the transaction which breaks the rule of
    lock based logical clock. commit_parent of the following transactions
    will be set to m_last_blocking_transaction if their last_committed is
    smaller than m_last_blocking_transaction.
  */
  int64 m_last_blocking_transaction = SEQ_UNINIT;
};

/**
  Generate logical timestamps for MTS using WRITESET
  in the binlog-transaction-dependency-tracking option.
*/
class Writeset_trx_dependency_tracker {
 public:
  Writeset_trx_dependency_tracker(ulong max_history_size)
      : m_opt_max_history_size(max_history_size), m_writeset_history_start(0) {}

  /**
    Main function that gets the dependencies using the WRITESET tracker.

    @param [in]     thd             THD of the caller.
    @param [in,out] sequence_number sequence_number initialized and returned.
    @param [in,out] commit_parent   commit_parent to be returned.
   */
  void get_dependency(THD *thd, int64 &sequence_number, int64 &commit_parent);

  void rotate(int64 start);

  /* option opt_binlog_transaction_dependency_history_size */
  ulong m_opt_max_history_size;

 private:
  /*
    Monitor the last transaction with write-set to use as the minimal
    commit parent when logical clock source is WRITE_SET, i.e., the most recent
    transaction that is not in the history, or 0 when the history is empty.

    The m_writeset_history_start must to be set to 0 initially and whenever the
    binlog_transaction_dependency_tracking variable is changed or the history
    is cleared, so that it is updated to the first transaction for which the
    dependencies are checked.
  */
  int64 m_writeset_history_start;

  /*
    Track the last transaction sequence number that changed each row
    in the database, using row hashes from the writeset as the index.
  */
  typedef std::map<uint64, int64> Writeset_history;
  Writeset_history m_writeset_history;
};

/**
  Generate logical timestamps for MTS using WRITESET_SESSION
  in the binlog-transaction-dependency-tracking option.
*/
class Writeset_session_trx_dependency_tracker {
 public:
  /**
    Main function that gets the dependencies using the WRITESET_SESSION tracker.

    @param [in]     thd             THD of the caller.
    @param [in,out] sequence_number sequence_number initialized and returned.
    @param [in,out] commit_parent   commit_parent to be returned.
   */
  void get_dependency(THD *thd, int64 &sequence_number, int64 &commit_parent);
};

/**
  Modes for parallel transaction dependency tracking
*/
enum enum_binlog_transaction_dependency_tracking {
  /**
    Tracks dependencies based on the commit order of transactions.
    The time intervals during which any transaction holds all its locks are
    tracked (the interval ends just before storage engine commit, when locks
    are released. For an autocommit transaction it begins just before storage
    engine prepare. For BEGIN..COMMIT transactions it begins at the end of the
    last statement before COMMIT). Two transactions are marked as
    non-conflicting if their respective intervals overlap. In other words,
    if trx1 appears before trx2 in the binlog, and trx2 had acquired all its
    locks before trx1 released its locks, then trx2 is marked such that the
    slave can schedule it in parallel with trx1.
  */
  DEPENDENCY_TRACKING_COMMIT_ORDER = 0,
  /**
    Tracks dependencies based on the set of rows updated. Any two transactions
    that change disjoint sets of rows, are said concurrent and non-contending.
  */
  DEPENDENCY_TRACKING_WRITESET = 1,
  /**
    Tracks dependencies based on the set of rows updated per session. Any two
    transactions that change disjoint sets of rows, on different sessions,
    are said concurrent and non-contending. Transactions from the same session
    are always said to be dependent, i.e., are never concurrent and
    non-contending.
  */
  DEPENDENCY_TRACKING_WRITESET_SESSION = 2
};

/**
  Dependency tracker is a container singleton that dispatches between the three
  methods associated with the binlog-transaction-dependency-tracking option.
  There is a singleton instance of each of these classes.
*/
class Transaction_dependency_tracker {
 public:
  Transaction_dependency_tracker()
      : m_opt_tracking_mode(DEPENDENCY_TRACKING_COMMIT_ORDER),
        m_writeset(25000) {}

  void get_dependency(THD *thd, int64 &sequence_number, int64 &commit_parent);

  void tracking_mode_changed();

  void update_max_committed(THD *thd);
  int64 get_max_committed_timestamp();

  int64 step();
  void rotate();

 public:
  /* option opt_binlog_transaction_dependency_tracking */
  long m_opt_tracking_mode;

  Writeset_trx_dependency_tracker *get_writeset() { return &m_writeset; }

 private:
  Writeset_trx_dependency_tracker m_writeset;
  Commit_order_trx_dependency_tracker m_commit_order;
  Writeset_session_trx_dependency_tracker m_writeset_session;
};

#endif /* RPL_TRX_TRACKING_INCLUDED */
