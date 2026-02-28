/*
   Copyright (c) 2019, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef BINLOG_MONITORING_CONTEXT_H
#define BINLOG_MONITORING_CONTEXT_H

#include <map>

#include <sql/log_event.h>
#include <sql/rpl_gtid.h>
#include <stddef.h>
#include <sys/types.h>

namespace binlog {
namespace monitoring {

enum log_type { BINARY = 1, RELAY, UKNOWN };

/**
  This class represents the compression stats collected for a given combination
  of log type and compression type.
 */
class Compression_stats {
 public:
  /**
    This tuple contains information about a transaction:
    - transaction id : string
    - compressed bytes : uint64
    - uncompressed bytes : uint64
    - timestamp : uint64
   */
  using Compression_stats_trx_row =
      std::tuple<std::string, uint64_t, uint64_t, uint64_t>;

  /**
    A constant and static instance of the transaction compression stats.
   */
  static const Compression_stats_trx_row &ZERO_TRX_ROW();

 protected:
  /**
    Enum stating whether FIRST or LAST transaction.
   */
  enum enum_trx_type { FIRST = 0, LAST };

  /**
    The log type.
   */
  log_type m_log_type{BINARY};

  /**
    The compression type.
   */
  binary_log::transaction::compression::type m_type{
      binary_log::transaction::compression::type::NONE};

  /**
    Counter that tracks how many transactions have been observed.
   */
  std::atomic<uint64_t> m_counter_transactions{0};

  /**
    Sum of all compressed bytes for all transactions observed through this
    object instance.
   */
  std::atomic<uint64_t> m_counter_compressed_bytes{0};

  /**
    Sum of all uncompressed bytes for all transactions observed through this
    object instance.
   */
  std::atomic<uint64_t> m_counter_uncompressed_bytes{0};

  /**
   This tuple contains information about the first transaction.
   */
  std::atomic<Compression_stats_trx_row *> m_first_transaction_stats{nullptr};

  /**
    This tuple contains information about the last transaction.
   */
  std::atomic<Compression_stats_trx_row *> m_last_transaction_stats{nullptr};

 protected:
  /**
    This member function shall claim memory used for tracking
    transaction stats.

    @param type Whether the FIRST or the LAST transaction.
   */
  void destroy_transaction_stats(enum_trx_type type);

  /**
    This member function shall destroy the object data structures.
    Used by the object destroyer.
  */
  void destroy();

  /**
    This member function shall return the compression stats for the given
    transaction.

    @param type the transaction to get the status for (either FIRST or LAST).

    @return the compression stats for the given transaction.
   */
  Compression_stats_trx_row get_transaction_stats(enum_trx_type type);

 public:
  Compression_stats() = delete;
  /**
    Initializes the compression stats for the given log type and
    compression type. It initializes the counters and transaction
    stats to 0.
   */
  Compression_stats(log_type log, binary_log::transaction::compression::type);

  /**
    Copies the contents of the object referenced as a parameter.

    @param rhs The object to copy.
   */
  Compression_stats(Compression_stats &rhs);

  /**
    The destructor of this row.
   */
  virtual ~Compression_stats();

  /**
    Updates the existing stats with the ones passed as argument.

    @param gtid the transaction identifier as a string.
    @param transaction_timestamp The transaction timestamp as seconds since the
                                 epoch.
    @param comp_bytes The compressed bytes counter for the given transaction.
    @param uncomp_bytes The uncompressed bytes counter for the given
    transaction.
   */
  void add(std::string gtid, uint64_t transaction_timestamp,
           uint64_t comp_bytes, uint64_t uncomp_bytes);

  /**
    This member function shall reset the counters to zero and
    clear the transaction stats for both FIRST and LAST transactions.
  */
  void reset();

  /**
    Gets the log type that this object instance is tracking.
   */
  log_type get_log_type() const;

  /**
    Gets the commpression type that this object instance is tracking.
   */
  binary_log::transaction::compression::type get_type() const;

  /**
    Gets the number of transactions counted.
    @return number of transactions counted.
   */
  uint64_t get_counter_transactions() const;

  /**
    Gets the sum of compressed bytes accounted for by this object instance.
    @return sum of compressed bytes for this object instance.
   */
  uint64_t get_counter_compressed_bytes() const;

  /**
    Gets the sum of uncompressed bytes accounted for by this object instance.
    @return sum of uncompressed bytes for this object instance.
   */
  uint64_t get_counter_uncompressed_bytes() const;

  /**
    Gets the stats for the last transaction.
    @return the stats for the last transaction.
   */
  Compression_stats_trx_row get_last_transaction_stats();

  /**
    Gets the stats of the first transaction.
    @return the stats for the first transaction.
   */
  Compression_stats_trx_row get_first_transaction_stats();
};

class Transaction_compression {
 protected:
  /**
    The map that contains rows of stats in the probe. A stats row is a
    combination of log type and compression type.
   */
  std::map<std::pair<log_type, binary_log::transaction::compression::type>,
           Compression_stats *>
      m_stats;

  /**
    Allocates this probe's internal structures.
   */
  void init();

  /**
    Claims this probe's internal resources.
   */
  void destroy();

 public:
  /**
    Update this probe's stats.

    @param log_type the type of the log that this invocation refers to.
    @param comp_type the compression type for this invocation.
    @param gtid the transaction identifier for this invocation.
    @param transaction_timestamp the transaction commit timestamp in seconds
                                 since the UNIX epoch.
    @param comp_bytes the bytes compressed by this transaction.
    @param uncomp_bytes the bytes uncompressed by this transaction.
    @param sid_map the Sid_map to use to create a string representation from the
                   transaction identifier provided.
   */
  void update(log_type log_type,
              binary_log::transaction::compression::type comp_type, Gtid &gtid,
              uint64_t transaction_timestamp, uint64_t comp_bytes,
              uint64_t uncomp_bytes, Sid_map *sid_map = global_sid_map);

  /**
    Gets the contents of the probe. The contents are a copy of the internal
    stats and as such, the caller must free the resources in stats once they are
    no longer needed.

    @param stats the container to fill in with copies of the stats in the probe.
   */
  void get_stats(std::vector<Compression_stats *> &stats);

  /**
    Gets the number of stats in the probe. Each combination of log_type and
    comp_type creates a row. Only those rows that have stats collected are
    considered.

    @return the number of combinations between log_type and comp_type that have
    stats collected.
   */
  int number_stats_rows();

  /**
    Resets the stats of this probe to zero.
   */
  void reset();

  /**
    Constructor. The constructed object is reset after this returns.
   */
  Transaction_compression();

  /**
    Destructor. Once the destructor returns the internal data structures have
    been destroyed.
   */
  virtual ~Transaction_compression();
};

/**
  The global context for binary/relay log monitoring.

  @todo migrate the monitoring parts that are scattered all around
        this this entry point.
 */
class Context {
 protected:
  Transaction_compression m_transaction_compression_ctx;

 public:
  Context(const Context &rhs) = delete;
  Context &operator=(const Context &rhs) = delete;

  Context() = default;
  virtual ~Context() = default;

  Transaction_compression &transaction_compression();
};

}  // namespace monitoring
}  // namespace binlog

#endif
