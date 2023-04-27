/*
   Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/binlog/tools/iterators.h"
#include "sql/mysqld.h"     // PSI_stage_info
#include "sql/sql_class.h"  // current_thd, THD

#include "libbinlogevents/include/compression/iterator.h"

namespace binlog {
namespace tools {

Iterator::Iterator(Binlog_file_reader *reader)
    : m_binlog_reader(reader),
      m_verify_checksum(false),
      m_copy_event_buffer(false),
      m_error_number(0),
      m_error_message("") {}

void Iterator::set_copy_event_buffer() { m_copy_event_buffer = true; }

void Iterator::unset_copy_event_buffer() {
  m_copy_event_buffer = false; /* purecov: inspected */
}

void Iterator::set_verify_checksum() {
  m_verify_checksum = true; /* purecov: inspected */
}

void Iterator::unset_verify_checksum() {
  m_verify_checksum = false; /* purecov: inspected */
}

Log_event *Iterator::do_next() {
  Log_event *ev = nullptr;

  // read in the next event object
  if (m_events.empty())
    ev = m_binlog_reader->read_event_object();
  else {
    ev = m_events.front();
    m_events.pop();
  }

  // expand the transaction payload log event
  if ((ev != nullptr) &&
      (ev->get_type_code() == binary_log::TRANSACTION_PAYLOAD_EVENT)) {
    PSI_stage_info old_stage;
    THD *thd{current_thd};
    Transaction_payload_log_event *tple =
        dynamic_cast<Transaction_payload_log_event *>(ev);

    // disable checksums for payload events
    auto fdle = *m_binlog_reader->format_description_event();
    fdle.footer()->checksum_alg = binary_log::BINLOG_CHECKSUM_ALG_OFF;

    binary_log::transaction::compression::Iterable_buffer it(
        tple->get_payload(), tple->get_payload_size(),
        tple->get_uncompressed_size(), tple->get_compression_type());

    if (tple->get_compression_type() !=
        binary_log::transaction::compression::type::NONE)
      // set the thread stage to compressing transaction
      if (thd != nullptr)
        thd->enter_stage(&stage_binlog_transaction_decompress, &old_stage,
                         __func__, __FILE__, __LINE__);

    // enqueue the decompressed event
    for (auto ptr : it) {
      Log_event *nev{nullptr};
      size_t event_len{uint4korr(ptr + EVENT_LEN_OFFSET)};
      const uchar *event_buffer{nullptr};
      char *copied_buffer{nullptr};
      if (m_copy_event_buffer) {
        copied_buffer = (char *)my_malloc(key_memory_log_event, event_len,
                                          MYF(MY_ZEROFILL));
        memcpy(copied_buffer, ptr, event_len);
        event_buffer = reinterpret_cast<const uchar *>(copied_buffer);
      } else
        event_buffer = reinterpret_cast<const uchar *>(ptr);

      if (binlog_event_deserialize(event_buffer, event_len, &fdle,
                                   m_verify_checksum, &nev)) {
        /* purecov: begin inspected */
        m_error_number = 1;
        m_error_message = "Unable to deserialize transaction payload event!";
        break;
        /* purecov: end */
      }

      nev->register_temp_buf(reinterpret_cast<char *>(copied_buffer),
                             m_copy_event_buffer);
      nev->common_header->log_pos = tple->header()->log_pos;

      m_events.push(nev);
    }
    // revert the stage if needed
    if (old_stage.m_key != 0) THD_STAGE_INFO(thd, old_stage);
  }
  return ev;
}

Iterator::~Iterator() {
  while (!m_events.empty()) {
    auto next = m_events.front();
    m_events.pop();
    delete next;
  }
}

bool Iterator::has_error() { return m_error_number != 0; }

int Iterator::get_error_number() { return m_error_number; }

std::string Iterator::get_error_message() { return m_error_message; }

Log_event *Iterator::begin() { return next(); }

Log_event *Iterator::next() {
  DBUG_ASSERT(m_binlog_reader != nullptr);
  return do_next();
}

Log_event *Iterator::end() { return nullptr; }

}  // namespace tools
}  // namespace binlog
