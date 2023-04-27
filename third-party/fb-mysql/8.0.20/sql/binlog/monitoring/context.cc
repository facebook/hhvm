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

#include "sql/binlog/monitoring/context.h"

#include <utility>  // std::pair

namespace binlog {
namespace monitoring {

/** Compression_stats --------------------------------------------------- */

/* Constructors and destructors. */
const Compression_stats::Compression_stats_trx_row &
Compression_stats::ZERO_TRX_ROW() {
  static const Compression_stats::Compression_stats_trx_row instance =
      std::make_tuple("", 0, 0, 0);
  return instance;
}

Compression_stats::Compression_stats(
    log_type log, binary_log::transaction::compression::type t)
    : m_log_type(log),
      m_type(t),
      m_counter_transactions(0),
      m_counter_compressed_bytes(0),
      m_counter_uncompressed_bytes(0),
      m_first_transaction_stats{new Compression_stats_trx_row("", 0, 0, 0)},
      m_last_transaction_stats{new Compression_stats_trx_row("", 0, 0, 0)} {}

Compression_stats::Compression_stats(Compression_stats &rhs)
    : m_log_type(rhs.get_log_type()),
      m_type(rhs.get_type()),
      m_counter_transactions(rhs.get_counter_transactions()),
      m_counter_compressed_bytes(rhs.get_counter_compressed_bytes()),
      m_counter_uncompressed_bytes(rhs.get_counter_uncompressed_bytes()),
      m_first_transaction_stats{
          new Compression_stats_trx_row(rhs.get_first_transaction_stats())},
      m_last_transaction_stats{
          new Compression_stats_trx_row(rhs.get_last_transaction_stats())} {}

Compression_stats::~Compression_stats() { destroy(); }

/* Getters */

log_type Compression_stats::get_log_type() const { return m_log_type; }

binary_log::transaction::compression::type Compression_stats::get_type() const {
  return m_type;
}

uint64_t Compression_stats::get_counter_transactions() const {
  return m_counter_transactions;
}

uint64_t Compression_stats::get_counter_compressed_bytes() const {
  return m_counter_compressed_bytes;
}

uint64_t Compression_stats::get_counter_uncompressed_bytes() const {
  return m_counter_uncompressed_bytes;
}

Compression_stats::Compression_stats_trx_row
Compression_stats::get_transaction_stats(enum_trx_type type) {
  Compression_stats_trx_row *null{nullptr};
  Compression_stats_trx_row *current{nullptr};
  Compression_stats_trx_row to_return{"", 0, 0, 0};

  std::atomic<Compression_stats_trx_row *> *ptr{nullptr};

  ptr = type == LAST ? &m_last_transaction_stats : &m_first_transaction_stats;

  do {
    current = ptr->load();
    if (current == nullptr || !ptr->compare_exchange_strong(current, null)) {
      my_thread_yield();
      continue;
    }
    break;
  } while (true);

  // this copies (assignment copy) the the contents of current
  to_return = *current;
  ptr->exchange(current);
  return to_return;
}

Compression_stats::Compression_stats_trx_row
Compression_stats::get_last_transaction_stats() {
  return get_transaction_stats(LAST);
}

Compression_stats::Compression_stats_trx_row
Compression_stats::get_first_transaction_stats() {
  return get_transaction_stats(FIRST);
}

/* Updaters */

void Compression_stats::add(std::string gtid, uint64_t transaction_timestamp,
                            uint64_t compressed_bytes,
                            uint64_t uncompressed_bytes) {
  Compression_stats_trx_row *current{nullptr};
  Compression_stats_trx_row *null{nullptr};
  Compression_stats_trx_row updated{gtid, compressed_bytes, uncompressed_bytes,
                                    transaction_timestamp};

  // set the value of the first seen atomically if needed
  do {
    current = m_first_transaction_stats.load();
    if (current == nullptr ||
        !m_first_transaction_stats.compare_exchange_strong(current, null)) {
      my_thread_yield();
      continue;
    }
    break;
  } while (true);

  // if timestamp is set to 0, this has not been updated
  if (unlikely(std::get<3>(*current) == 0)) {
    Compression_stats_trx_row first{gtid, compressed_bytes, uncompressed_bytes,
                                    transaction_timestamp};
    *current = updated;
  }
  m_first_transaction_stats.exchange(current);

  // set the value of the last seen atomically if needed
  do {
    current = m_last_transaction_stats.load();
    if (current == nullptr ||
        !m_last_transaction_stats.compare_exchange_strong(current, null)) {
      my_thread_yield();
      continue;
    }
    break;
  } while (true);
  *current = updated;
  m_last_transaction_stats.exchange(current);

  // update the counters
  m_counter_transactions++;
  m_counter_compressed_bytes += compressed_bytes;
  m_counter_uncompressed_bytes += uncompressed_bytes;
}

void Compression_stats::destroy_transaction_stats(enum_trx_type type) {
  Compression_stats_trx_row *current{nullptr};
  Compression_stats_trx_row *null{nullptr};
  std::atomic<Compression_stats_trx_row *> *ptr = nullptr;
  ptr = type == LAST ? &m_last_transaction_stats : &m_first_transaction_stats;

  do {
    current = ptr->load();
    if (current == nullptr || !ptr->compare_exchange_strong(current, null)) {
      my_thread_yield();
      continue;
    }
    break;
  } while (true);
  delete current;
}

void Compression_stats::reset() {
  Compression_stats_trx_row *current{nullptr};
  Compression_stats_trx_row *first{nullptr};
  Compression_stats_trx_row *last{nullptr};
  Compression_stats_trx_row *null{nullptr};

  do {
    current = m_first_transaction_stats.load();
    if (current == nullptr ||
        !m_first_transaction_stats.compare_exchange_strong(current, null)) {
      my_thread_yield();
      continue;
    }
    break;
  } while (true);

  first = current;

  do {
    current = m_last_transaction_stats.load();
    if (current == nullptr ||
        !m_last_transaction_stats.compare_exchange_strong(current, null)) {
      my_thread_yield();
      continue;
    }
    break;
  } while (true);

  last = current;

  *first = ZERO_TRX_ROW();
  *last = ZERO_TRX_ROW();
  m_counter_transactions = 0;
  m_counter_compressed_bytes = 0;
  m_counter_uncompressed_bytes = 0;

  m_last_transaction_stats.exchange(last);
  m_first_transaction_stats.exchange(first);
}

/* Destroyers */

void Compression_stats::destroy() {
  destroy_transaction_stats(LAST);
  destroy_transaction_stats(FIRST);
}

/** Transaction_compression -------------------------------------------- */

Transaction_compression::Transaction_compression() {
  DBUG_TRACE;
  init();
}

Transaction_compression::~Transaction_compression() {
  DBUG_TRACE;
  destroy();
}

void Transaction_compression::init() {
  DBUG_TRACE;
  auto comp_types = std::set<binary_log::transaction::compression::type>();
  auto log_types = std::set<binlog::monitoring::log_type>();

  comp_types.insert(binary_log::transaction::compression::type::NONE);
  comp_types.insert(binary_log::transaction::compression::type::ZSTD);

  log_types.insert(binlog::monitoring::log_type::BINARY);
  log_types.insert(binlog::monitoring::log_type::RELAY);

  // statically initialize the stats collector
  for (auto log : log_types) {
    for (auto compression : comp_types) {
      m_stats[{log, compression}] = new Compression_stats(log, compression);
    }
  }
}

void Transaction_compression::destroy() {
  DBUG_TRACE;
  for (auto &entry : m_stats) delete entry.second;
  // clear the map
  m_stats.clear();
}

void Transaction_compression::reset() {
  DBUG_TRACE;
  for (auto &entry : m_stats) entry.second->reset();
}

void Transaction_compression::update(
    log_type log_type, binary_log::transaction::compression::type comp_type,
    Gtid &gtid, uint64_t transaction_timestamp, uint64_t comp_bytes,
    uint64_t uncomp_bytes, Sid_map *sid_map) {
  DBUG_TRACE;
  Gtid_specification spec;
  char gtid_buf[Gtid::MAX_TEXT_LENGTH + 1];
  if (gtid.sidno <= 0)
    spec.set_anonymous();
  else
    spec.set(gtid);
  auto gtid_buf_len = spec.to_string(sid_map, gtid_buf, true);
  std::string gtid_string(gtid_buf, gtid_buf_len);

#ifndef DBUG_OFF
  auto key = std::make_pair<binlog::monitoring::log_type &,
                            binary_log::transaction::compression::type &>(
      log_type, comp_type);

  DBUG_ASSERT(m_stats.find(key) != m_stats.end() && m_stats[key] != nullptr);
#endif
  m_stats[{log_type, comp_type}]->add(gtid_string, transaction_timestamp,
                                      comp_bytes, uncomp_bytes);
}

void Transaction_compression::get_stats(std::vector<Compression_stats *> &v) {
  for (auto const &entry : m_stats) {
    // if first seen transaction is set
    if (std::get<3>(entry.second->get_first_transaction_stats()) != 0) {
      v.push_back(new Compression_stats(*entry.second));
    }
  }
}

int Transaction_compression::number_stats_rows() {
  int res = 0;
  for (auto const &entry : m_stats) {
    // if first seen transaction is set
    if (std::get<3>(entry.second->get_first_transaction_stats()) != 0) {
      res++;
    }
  }
  return res;
}

Transaction_compression &Context::transaction_compression() {
  return m_transaction_compression_ctx;
}

}  // namespace monitoring
}  // namespace binlog
