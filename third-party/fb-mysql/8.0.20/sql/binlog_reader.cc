/* Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/binlog_reader.h"
#include "my_byteorder.h"
#include "sql/log_event.h"

unsigned char *Default_binlog_event_allocator::allocate(size_t size) {
  DBUG_EXECUTE_IF("simulate_allocate_failure", return nullptr;);
  return static_cast<unsigned char *>(
      my_malloc(key_memory_log_event, size + 1, MYF(MY_WME)));
}

void Default_binlog_event_allocator::deallocate(unsigned char *ptr) {
  my_free(ptr);
}

#ifndef DBUG_OFF
static void debug_corrupt_event(unsigned char *buffer, unsigned int event_len) {
  /*
    Corrupt the event.
    Dump threads need to exclude Format_description_log_event,
    Previous_gtids_log_event and Gtid_log_event
    events from injected corruption to allow dump thread to move forward
    on binary log until the missing transactions from slave when
    MASTER_AUTO_POSITION= 1.
  */
  DBUG_EXECUTE_IF(
      "corrupt_read_log_event", unsigned char type = buffer[EVENT_TYPE_OFFSET];
      if (type != binary_log::FORMAT_DESCRIPTION_EVENT &&
          type != binary_log::PREVIOUS_GTIDS_LOG_EVENT &&
          type != binary_log::GTID_LOG_EVENT) {
        int cor_pos = rand() % (event_len - BINLOG_CHECKSUM_LEN -
                                LOG_EVENT_MINIMAL_HEADER_LEN) +
                      LOG_EVENT_MINIMAL_HEADER_LEN;
        buffer[cor_pos] = buffer[cor_pos] + 1;
        DBUG_PRINT("info", ("Corrupt the event on position %d", cor_pos));
      });
}
#endif  // ifdef DBUG_OFF

Binlog_event_data_istream::Binlog_event_data_istream(
    Binlog_read_error *error, Basic_istream *istream,
    unsigned int max_event_size)
    : m_error(error), m_istream(istream), m_max_event_size(max_event_size) {}

bool Binlog_event_data_istream::read_event_header() {
  return read_fixed_length<Binlog_read_error::READ_EOF>(
      m_header, LOG_EVENT_MINIMAL_HEADER_LEN);
}

bool Binlog_event_data_istream::fill_event_data(
    unsigned char *event_data, bool verify_checksum,
    enum_binlog_checksum_alg checksum_alg) {
  memcpy(event_data, m_header, LOG_EVENT_MINIMAL_HEADER_LEN);
  if (read_fixed_length<Binlog_read_error::TRUNC_EVENT>(
          event_data + LOG_EVENT_MINIMAL_HEADER_LEN,
          m_event_length - LOG_EVENT_MINIMAL_HEADER_LEN))
    return true;

#ifndef DBUG_OFF
  debug_corrupt_event(event_data, m_event_length);
#endif

  if (verify_checksum) {
    if (event_data[EVENT_TYPE_OFFSET] == binary_log::FORMAT_DESCRIPTION_EVENT)
      checksum_alg = Log_event_footer::get_checksum_alg(
          reinterpret_cast<char *>(event_data), m_event_length);

    if (Log_event_footer::event_checksum_test(event_data, m_event_length,
                                              checksum_alg) &&
        !DBUG_EVALUATE_IF("simulate_unknown_ignorable_log_event", 1, 0)) {
      return m_error->set_type(Binlog_read_error::CHECKSUM_FAILURE);
    }
  }
  return false;
}

bool Binlog_event_data_istream::check_event_header() {
  m_event_length = uint4korr(m_header + EVENT_LEN_OFFSET);

  if (m_event_length < LOG_EVENT_MINIMAL_HEADER_LEN)
    return m_error->set_type(Binlog_read_error::BOGUS);
  if (m_event_length > m_max_event_size)
    return m_error->set_type(Binlog_read_error::EVENT_TOO_LARGE);

  return false;
}

Binlog_read_error::Error_type binlog_event_deserialize(
    const unsigned char *buffer, unsigned int event_len,
    const Format_description_event *fde, bool verify_checksum,
    Log_event **event) {
  const char *buf = reinterpret_cast<const char *>(buffer);
  Log_event *ev = nullptr;
  enum_binlog_checksum_alg alg;

  DBUG_TRACE;

  DBUG_ASSERT(fde != nullptr);
  DBUG_PRINT("info", ("binlog_version: %d", fde->binlog_version));
  DBUG_DUMP("data", buffer, event_len);

  /* Check the integrity */
  if (event_len < LOG_EVENT_MINIMAL_HEADER_LEN) {
    DBUG_PRINT("error", ("event_len=%u", event_len));
    return Binlog_read_error::TRUNC_EVENT;
  }

  if (event_len != uint4korr(buf + EVENT_LEN_OFFSET)) {
    DBUG_PRINT("error",
               ("event_len=%u EVENT_LEN_OFFSET=%d "
                "buf[EVENT_TYPE_OFFSET]=%d ENUM_END_EVENT=%d "
                "uint4korr(buf+EVENT_LEN_OFFSET)=%d",
                event_len, EVENT_LEN_OFFSET, buf[EVENT_TYPE_OFFSET],
                binary_log::ENUM_END_EVENT, uint4korr(buf + EVENT_LEN_OFFSET)));
    return event_len > uint4korr(buf + EVENT_LEN_OFFSET)
               ? Binlog_read_error::BOGUS
               : Binlog_read_error::TRUNC_EVENT;
  }

  uint event_type = buf[EVENT_TYPE_OFFSET];

  /*
    Sanity check for Format description event. This is needed because
    get_checksum_alg will assume that Format_description_event is well-formed
  */
  if (event_type == binary_log::FORMAT_DESCRIPTION_EVENT) {
    if (event_len <= LOG_EVENT_MINIMAL_HEADER_LEN + ST_COMMON_HEADER_LEN_OFFSET)
      return Binlog_read_error::TRUNC_FD_EVENT;

    uint tmp_header_len =
        buf[LOG_EVENT_MINIMAL_HEADER_LEN + ST_COMMON_HEADER_LEN_OFFSET];
    if (event_len < tmp_header_len + ST_SERVER_VER_OFFSET + ST_SERVER_VER_LEN)
      return Binlog_read_error::TRUNC_FD_EVENT;
  }

  /*
    If it is a FD event, then uses the checksum algorithm in it. Otherwise use
    the checksum algorithm in fde provided by caller.

    Notice, a pre-checksum FD version forces alg := BINLOG_CHECKSUM_ALG_UNDEF.
  */
  alg = (event_type != binary_log::FORMAT_DESCRIPTION_EVENT)
            ? fde->footer()->checksum_alg
            : Log_event_footer::get_checksum_alg(buf, event_len);

#ifndef DBUG_OFF
  binary_log_debug::debug_checksum_test =
      DBUG_EVALUATE_IF("simulate_checksum_test_failure", true, false);
#endif

  if (verify_checksum &&
      Log_event_footer::event_checksum_test(const_cast<uchar *>(buffer),
                                            event_len, alg) &&
      /* Skip the crc check when simulating an unknown ignorable log event. */
      !DBUG_EVALUATE_IF("simulate_unknown_ignorable_log_event", 1, 0)) {
    return Binlog_read_error::CHECKSUM_FAILURE;
  }

  if (event_type > fde->number_of_event_types &&
      /*
        Skip the event type check when simulating an unknown ignorable event.
      */
      !DBUG_EVALUATE_IF("simulate_unknown_ignorable_log_event", 1, 0)) {
    /*
      It is unsafe to use the fde if its post_header_len
      array does not include the event type.
    */
    DBUG_PRINT("error", ("event type %d found, but the current "
                         "Format_description_event supports only %d event "
                         "types",
                         event_type, fde->number_of_event_types));
    return Binlog_read_error::INVALID_EVENT;
  }

  /* Remove checksum length from event_len */
  if (alg != binary_log::BINLOG_CHECKSUM_ALG_UNDEF &&
      (event_type == binary_log::FORMAT_DESCRIPTION_EVENT ||
       alg != binary_log::BINLOG_CHECKSUM_ALG_OFF))
    event_len = event_len - BINLOG_CHECKSUM_LEN;

  switch (event_type) {
    case binary_log::QUERY_EVENT:
#ifndef DBUG_OFF
      binary_log_debug::debug_query_mts_corrupt_db_names =
          DBUG_EVALUATE_IF("query_log_event_mts_corrupt_db_names", true, false);
#endif
      ev = new Query_log_event(buf, fde, binary_log::QUERY_EVENT);
      break;
    case binary_log::ROTATE_EVENT:
      ev = new Rotate_log_event(buf, fde);
      break;
    case binary_log::APPEND_BLOCK_EVENT:
      ev = new Append_block_log_event(buf, fde);
      break;
    case binary_log::DELETE_FILE_EVENT:
      ev = new Delete_file_log_event(buf, fde);
      break;
    case binary_log::STOP_EVENT:
      ev = new Stop_log_event(buf, fde);
      break;
    case binary_log::INTVAR_EVENT:
      ev = new Intvar_log_event(buf, fde);
      break;
    case binary_log::XID_EVENT:
      ev = new Xid_log_event(buf, fde);
      break;
    case binary_log::RAND_EVENT:
      ev = new Rand_log_event(buf, fde);
      break;
    case binary_log::USER_VAR_EVENT:
      ev = new User_var_log_event(buf, fde);
      break;
    case binary_log::FORMAT_DESCRIPTION_EVENT:
      ev = new Format_description_log_event(buf, fde);
      break;
    case binary_log::WRITE_ROWS_EVENT_V1:
      if (!(fde->post_header_len.empty()))
        ev = new Write_rows_log_event(buf, fde);
      break;
    case binary_log::UPDATE_ROWS_EVENT_V1:
      if (!(fde->post_header_len.empty()))
        ev = new Update_rows_log_event(buf, fde);
      break;
    case binary_log::DELETE_ROWS_EVENT_V1:
      if (!(fde->post_header_len.empty()))
        ev = new Delete_rows_log_event(buf, fde);
      break;
    case binary_log::TABLE_MAP_EVENT:
      if (!(fde->post_header_len.empty()))
        ev = new Table_map_log_event(buf, fde);
      break;
    case binary_log::BEGIN_LOAD_QUERY_EVENT:
      ev = new Begin_load_query_log_event(buf, fde);
      break;
    case binary_log::EXECUTE_LOAD_QUERY_EVENT:
      ev = new Execute_load_query_log_event(buf, fde);
      break;
    case binary_log::INCIDENT_EVENT:
      ev = new Incident_log_event(buf, fde);
      break;
    case binary_log::ROWS_QUERY_LOG_EVENT:
      ev = new Rows_query_log_event(buf, fde);
      break;
    case binary_log::GTID_LOG_EVENT:
    case binary_log::ANONYMOUS_GTID_LOG_EVENT:
      ev = new Gtid_log_event(buf, fde);
      break;
    case binary_log::PREVIOUS_GTIDS_LOG_EVENT:
      ev = new Previous_gtids_log_event(buf, fde);
      break;
    case binary_log::WRITE_ROWS_EVENT:
      ev = new Write_rows_log_event(buf, fde);
      break;
    case binary_log::UPDATE_ROWS_EVENT:
      ev = new Update_rows_log_event(buf, fde);
      break;
    case binary_log::DELETE_ROWS_EVENT:
      ev = new Delete_rows_log_event(buf, fde);
      break;
    case binary_log::TRANSACTION_CONTEXT_EVENT:
      ev = new Transaction_context_log_event(buf, fde);
      break;
    case binary_log::VIEW_CHANGE_EVENT:
      ev = new View_change_log_event(buf, fde);
      break;
    case binary_log::XA_PREPARE_LOG_EVENT:
      ev = new XA_prepare_log_event(buf, fde);
      break;
    case binary_log::PARTIAL_UPDATE_ROWS_EVENT:
      ev = new Update_rows_log_event(buf, fde);
      break;
    case binary_log::TRANSACTION_PAYLOAD_EVENT:
      ev = new Transaction_payload_log_event(buf, fde);
      break;
    case binary_log::METADATA_EVENT:
      ev = new Metadata_log_event(buf, fde);
      break;
    default:
      /*
        Create an object of Ignorable_log_event for unrecognized sub-class.
        So that SLAVE SQL THREAD will only update the position and continue.
      */
      if (uint2korr(buf + FLAGS_OFFSET) & LOG_EVENT_IGNORABLE_F) {
        ev = new Ignorable_log_event(buf, fde);
      } else {
        DBUG_PRINT("error",
                   ("Unknown event code: %d", (int)buf[EVENT_TYPE_OFFSET]));
        ev = nullptr;
      }
      break;
  }

  /*
    is_valid is used for small event-specific sanity tests which are
    important; for example there are some my_malloc() in constructors
    (e.g. Query_log_event::Query_log_event(char*...)); when these
    my_malloc() fail we can't return an error out of the constructor
    (because constructor is "void") ; so instead we leave the pointer we
    wanted to allocate (e.g. 'query') to 0 and we test it and set the
    value of is_valid to true or false based on the test.
    Same for Format_description_log_event, member 'post_header_len'.
  */
  if (!ev || !ev->is_valid()) {
    delete ev;
    return Binlog_read_error::INVALID_EVENT;
  }

  ev->common_footer->checksum_alg = alg;
  if (ev->common_footer->checksum_alg != binary_log::BINLOG_CHECKSUM_ALG_OFF &&
      ev->common_footer->checksum_alg != binary_log::BINLOG_CHECKSUM_ALG_UNDEF)
    ev->crc = uint4korr(buf + event_len);

  DBUG_PRINT("read_event", ("%s(type_code: %d; event_len: %d)",
                            ev ? ev->get_type_str() : "<unknown>",
                            buf[EVENT_TYPE_OFFSET], event_len));
  *event = ev;
  return Binlog_read_error::SUCCESS;
}
