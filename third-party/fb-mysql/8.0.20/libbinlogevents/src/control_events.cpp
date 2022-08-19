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

#include "control_events.h"
#include <sstream>
#include "codecs/factory.h"
#include "compression/base.h"
#include "event_reader_macros.h"
#include "my_dbug.h"

namespace binary_log {

Rotate_event::Rotate_event(const char *buf, const Format_description_event *fde)
    : Binary_log_event(&buf, fde), new_log_ident(nullptr), flags(DUP_NAME) {
  BAPI_ENTER("Rotate_event::Rotate_event(const char*, ...)");
  READER_TRY_INITIALIZATION;
#ifndef DBUG_OFF
  size_t header_size = fde->common_header_len;
#endif
  READER_ASSERT_POSITION(header_size);
  uint8_t post_header_len = fde->post_header_len[ROTATE_EVENT - 1];

  /*
    By default, an event start immediately after the magic bytes in the binary
    log, which is at offset 4. In case if the slave has to rotate to a
    different event instead of the first one, the binary log offset for that
    event is specified in the post header. Else, the position is set to 4.
  */
  if (post_header_len) {
    READER_ASSERT_POSITION(header_size + R_POS_OFFSET);
    READER_TRY_SET(pos, read_and_letoh<uint64_t>);
    READER_ASSERT_POSITION(header_size + post_header_len);
  } else
    pos = 4;

  ident_len = READER_CALL(available_to_read);
  if (ident_len == 0) READER_THROW("Event is smaller than expected");

  if (ident_len > FN_REFLEN - 1) ident_len = FN_REFLEN - 1;

  READER_TRY_SET(new_log_ident, strndup<const char *>, ident_len);
  if (new_log_ident == nullptr)
    READER_THROW("Invalid binary log file name in Rotate event");

  READER_CATCH_ERROR;
  BAPI_VOID_RETURN;
}

/**
  Format_description_event 1st constructor.
*/
Format_description_event::Format_description_event(uint8_t binlog_ver,
                                                   const char *server_ver)
    : Binary_log_event(FORMAT_DESCRIPTION_EVENT),
      created(0),
      binlog_version(BINLOG_VERSION),
      dont_set_created(false) {
  binlog_version = binlog_ver;
  switch (binlog_ver) {
    case 4: /* MySQL 5.0 and above*/
    {
      /*
       As we are copying from a char * it might be the case at times that some
       part of the array server_version remains uninitialized so memset will
       help in getting rid of the valgrind errors.
      */
      memset(server_version, 0, ST_SERVER_VER_LEN);
      snprintf(server_version, ST_SERVER_VER_LEN, "%.*s", ST_SERVER_VER_LEN - 1,
               server_ver);
      if (binary_log_debug::debug_pretend_version_50034_in_binlog)
        strcpy(server_version, "5.0.34");
      common_header_len = LOG_EVENT_HEADER_LEN;
      number_of_event_types = LOG_EVENT_TYPES;
      /**
        This will be used to initialze the post_header_len,
        for binlog version 4.
      */
      static uint8_t server_event_header_length[] = {
          0,
          QUERY_HEADER_LEN,
          STOP_HEADER_LEN,
          ROTATE_HEADER_LEN,
          INTVAR_HEADER_LEN,
          0,
          /*
            Unused because the code for Slave log event was removed.
            (15th Oct. 2010)
          */
          0,
          0,
          APPEND_BLOCK_HEADER_LEN,
          0,
          DELETE_FILE_HEADER_LEN,
          0,
          RAND_HEADER_LEN,
          USER_VAR_HEADER_LEN,
          FORMAT_DESCRIPTION_HEADER_LEN,
          XID_HEADER_LEN,
          BEGIN_LOAD_QUERY_HEADER_LEN,
          EXECUTE_LOAD_QUERY_HEADER_LEN,
          TABLE_MAP_HEADER_LEN,
          0,
          0,
          0,
          ROWS_HEADER_LEN_V1, /* WRITE_ROWS_EVENT_V1*/
          ROWS_HEADER_LEN_V1, /* UPDATE_ROWS_EVENT_V1*/
          ROWS_HEADER_LEN_V1, /* DELETE_ROWS_EVENT_V1*/
          INCIDENT_HEADER_LEN,
          0, /* HEARTBEAT_LOG_EVENT*/
          IGNORABLE_HEADER_LEN,
          IGNORABLE_HEADER_LEN,
          ROWS_HEADER_LEN_V2,
          ROWS_HEADER_LEN_V2,
          ROWS_HEADER_LEN_V2,
          Gtid_event::POST_HEADER_LENGTH, /*GTID_EVENT*/
          Gtid_event::POST_HEADER_LENGTH, /*ANONYMOUS_GTID_EVENT*/
          IGNORABLE_HEADER_LEN,
          TRANSACTION_CONTEXT_HEADER_LEN,
          VIEW_CHANGE_HEADER_LEN,
          XA_PREPARE_HEADER_LEN,
          ROWS_HEADER_LEN_V2,
          TRANSACTION_PAYLOAD_EVENT,
      };
      /*
        Allows us to sanity-check that all events initialized their
        events (see the end of this 'if' block).
     */
      post_header_len.insert(
          post_header_len.begin(), server_event_header_length,
          server_event_header_length + number_of_event_types);
      // Sanity-check that all post header lengths are initialized.
#ifndef DBUG_OFF
      for (int i = 0; i < number_of_event_types; i++)
        BAPI_ASSERT(post_header_len[i] != 255);
#endif
      break;
    }
    default: /* Includes binlog version < 4 */
      /*
        Will make the mysql-server variable *is_valid* defined in class
        Log_event to be set to false.
      */
      break;
  }
  calc_server_version_split();
}

/**
   This method populates the array server_version_split which is then
   used for lookups to find if the server which
   created this event has some known bug.
*/
void Format_description_event::calc_server_version_split() {
  do_server_version_split(server_version, server_version_split);
}

/**
   This method is used to find out the version of server that originated
   the current FD instance.
   @return the version of server
*/
unsigned long Format_description_event::get_product_version() const {
  return version_product(server_version_split);
}

/**
   This method checks the MySQL version to determine whether checksums may be
   present in the events contained in the bainry log.

   @retval true  if the event's version is earlier than one that introduced
                 the replication event checksum.
   @retval false otherwise.
*/
bool Format_description_event::is_version_before_checksum() const {
  return get_product_version() < checksum_version_product;
}

/**
  The problem with this constructor is that the fixed header may have a
  length different from this version, but we don't know this length as we
  have not read the Format_description_log_event which says it, yet. This
  length is in the post-header of the event, but we don't know where the
  post-header starts.

  So this type of event HAS to:
  - either have the header's length at the beginning (in the header, at a
  fixed position which will never be changed), not in the post-header. That
  would make the header be "shifted" compared to other events.
  - or have a header of size LOG_EVENT_MINIMAL_HEADER_LEN (19), in all future
  versions, so that we know for sure.

  I (Guilhem) chose the 2nd solution. Rotate has the same constraint (because
  it is sent before Format_description_log_event).

*/
Format_description_event::Format_description_event(
    const char *buf, const Format_description_event *fde)
    : Binary_log_event(&buf, fde), common_header_len(0) {
  BAPI_ENTER(
      "Format_description_event::"
      "Format_description_event(const char*, ...)");
  READER_TRY_INITIALIZATION;

  unsigned long ver_calc;
  unsigned long available_bytes;
  number_of_event_types = 0;

  READER_ASSERT_POSITION(LOG_EVENT_MINIMAL_HEADER_LEN + ST_BINLOG_VER_OFFSET);
  READER_TRY_SET(binlog_version, read_and_letoh<uint16_t>);

  READER_ASSERT_POSITION(LOG_EVENT_MINIMAL_HEADER_LEN + ST_SERVER_VER_OFFSET);
  READER_TRY_CALL(memcpy<char *>, server_version, ST_SERVER_VER_LEN);

  // prevent overrun if log is corrupted on disk
  server_version[ST_SERVER_VER_LEN - 1] = 0;

  READER_ASSERT_POSITION(LOG_EVENT_MINIMAL_HEADER_LEN + ST_CREATED_OFFSET);
  READER_TRY_SET(created, read_and_letoh<uint64_t>, 4);
  dont_set_created = true;

  READER_ASSERT_POSITION(LOG_EVENT_MINIMAL_HEADER_LEN +
                         ST_COMMON_HEADER_LEN_OFFSET);
  READER_TRY_SET(common_header_len, read<uint8_t>);

  if (common_header_len < LOG_EVENT_HEADER_LEN)
    READER_THROW("Invalid Format_description common header length");

  available_bytes = READER_CALL(available_to_read);
  if (available_bytes == 0)
    READER_THROW("Invalid Format_description common header length");

  calc_server_version_split();
  if ((ver_calc = get_product_version()) >= checksum_version_product) {
    /* the last bytes are the checksum alg desc and value (or value's room) */
    available_bytes -= BINLOG_CHECKSUM_ALG_DESC_LEN;
  }

  number_of_event_types = available_bytes;
  READER_TRY_CALL(assign, &post_header_len, number_of_event_types);

  if ((ver_calc = get_product_version()) >= checksum_version_product) {
    /*
      FD from the checksum-home version server (ver_calc ==
      checksum_version_product) must have
      number_of_event_types == LOG_EVENT_TYPES.
    */
    BAPI_ASSERT(ver_calc != checksum_version_product ||
                number_of_event_types == LOG_EVENT_TYPES);
    uint8_t alg;
    READER_TRY_SET(alg, read<uint8_t>);
    footer()->checksum_alg = static_cast<enum_binlog_checksum_alg>(alg);
  } else {
    footer()->checksum_alg = BINLOG_CHECKSUM_ALG_UNDEF;
  }

  if (!header_is_valid()) READER_THROW("Invalid Format_description header");

  if (!version_is_valid())
    READER_THROW("Invalid server version in Format_description event");

  READER_CATCH_ERROR;
  BAPI_VOID_RETURN;
}

Format_description_event::~Format_description_event() {}

Stop_event::Stop_event(const char *buf, const Format_description_event *fde)
    : Binary_log_event(&buf, fde) {
  BAPI_ENTER("Stop_event::Stop_event (const char*, ...)");
  BAPI_VOID_RETURN;
}

Incident_event::Incident_event(const char *buf,
                               const Format_description_event *fde)
    : Binary_log_event(&buf, fde) {
  BAPI_ENTER("Incident_event::Incident_event(const char *, ...)");
  READER_TRY_INITIALIZATION;
  READER_ASSERT_POSITION(fde->common_header_len);
  uint16_t incident_number;
  uint8_t len = 0;            // Assignment to keep compiler happy
  const char *str = nullptr;  // Assignment to keep compiler happy

  message = nullptr;
  message_length = 0;
  incident = INCIDENT_NONE;

  READER_TRY_SET(incident_number, read_and_letoh<uint16_t>);
  if (incident_number >= INCIDENT_COUNT || incident_number <= INCIDENT_NONE)
    /*
      If the incident is not recognized, this binlog event is
      invalid.
    */
    READER_THROW("Invalid incident number in INCIDENT");

  incident = static_cast<enum_incident>(incident_number);

  READER_ASSERT_POSITION(fde->common_header_len +
                         fde->post_header_len[INCIDENT_EVENT - 1]);
  READER_TRY_CALL(read_str_at_most_255_bytes, &str, &len);

  if (!(message = static_cast<char *>(bapi_malloc(len + 1, 16))))
    READER_THROW("Out of memory");

  strncpy(message, str, len);
  // Appending '\0' at the end.
  message[len] = '\0';
  message_length = len;

  READER_CATCH_ERROR;
  BAPI_VOID_RETURN;
}

Ignorable_event::Ignorable_event(const char *buf,
                                 const Format_description_event *fde)
    : Binary_log_event(&buf, fde) {
  BAPI_ENTER("Ignorable_event::Ignorable_event(const char*, ...)");
  BAPI_VOID_RETURN;
}

Xid_event::Xid_event(const char *buf, const Format_description_event *fde)
    : Binary_log_event(&buf, fde) {
  BAPI_ENTER("Xid_event::Xid_event(const char*, ...)");
  READER_TRY_INITIALIZATION;
  READER_ASSERT_POSITION(fde->common_header_len);
  READER_TRY_CALL(forward, fde->post_header_len[XID_EVENT - 1]);
  READER_TRY_SET(xid, memcpy<int64_t>);
  READER_CATCH_ERROR;
  BAPI_VOID_RETURN;
}

XA_prepare_event::XA_prepare_event(const char *buf,
                                   const Format_description_event *fde)
    : Binary_log_event(&buf, fde) {
  BAPI_ENTER("XA_prepare_event::XA_prepare_event(const char*, ...)");
  READER_TRY_INITIALIZATION;
  READER_ASSERT_POSITION(fde->common_header_len);
  READER_TRY_CALL(forward, fde->post_header_len[XA_PREPARE_LOG_EVENT - 1]);
  READER_TRY_SET(one_phase, read<bool>);
  READER_TRY_SET(my_xid.formatID, read_and_letoh<uint32_t>);
  READER_TRY_SET(my_xid.gtrid_length, read_and_letoh<uint32_t>);
  READER_TRY_SET(my_xid.bqual_length, read_and_letoh<uint32_t>);

  /* Sanity check */
  if (MY_XIDDATASIZE >= my_xid.gtrid_length + my_xid.bqual_length &&
      my_xid.gtrid_length >= 0 && my_xid.gtrid_length <= 64 &&
      my_xid.bqual_length >= 0 && my_xid.bqual_length <= 64) {
    READER_TRY_CALL(memcpy<char *>, my_xid.data,
                    my_xid.gtrid_length + my_xid.bqual_length);
  } else
    READER_THROW("Invalid XID information in XA Prepare");

  READER_CATCH_ERROR;
  BAPI_VOID_RETURN;
}

Transaction_payload_event::Transaction_payload_event(const char *payload,
                                                     uint64_t payload_size,
                                                     uint16_t compression_type,
                                                     uint64_t uncompressed_size)
    : Binary_log_event(TRANSACTION_PAYLOAD_EVENT),
      m_payload(payload),
      m_payload_size(payload_size),
      m_compression_type((transaction::compression::type)compression_type),
      m_uncompressed_size(uncompressed_size) {}

Transaction_payload_event::Transaction_payload_event(const char *payload,
                                                     uint64_t payload_size)
    : Transaction_payload_event(payload, payload_size,
                                transaction::compression::type::NONE,
                                payload_size) {}

Transaction_payload_event::~Transaction_payload_event() {}

Transaction_payload_event::Transaction_payload_event(
    const char *buf, const Format_description_event *fde)
    : Binary_log_event(&buf, fde) {
  if (header()->get_is_valid()) {
    auto codec = codecs::Factory::build_codec(header()->type_code);
    // decode the post LOG_EVENT header
    auto buffer = (const unsigned char *)reader().ptr();
    size_t buffer_size = reader().available_to_read();
    auto result = codec->decode(buffer, buffer_size, *this);

    header()->set_is_valid(result.second == false);
    if (result.second == false) {
      // move the reader position forward
      reader().forward(result.first);

      // set the payload to the rest of the input buffer
      set_payload(reader().ptr());
    }
  }
}

std::string Transaction_payload_event::to_string() const {
  std::ostringstream oss;
  std::string comp_type =
      binary_log::transaction::compression::type_to_string(m_compression_type);

  oss << "\tpayload_size=" << m_payload_size;
  oss << "\tcompression_type=" << comp_type;
  if (m_compression_type != binary_log::transaction::compression::type::NONE)
    oss << "\tuncompressed_size=" << m_uncompressed_size;

  return oss.str();
}

#ifndef HAVE_MYSYS
void Transaction_payload_event::print_event_info(std::ostream &os) {
  os << to_string();
}

void Transaction_payload_event::print_long_info(std::ostream &os) {
  print_event_info(os);
}
#endif

Metadata_event::Metadata_event(const char *buf,
                               const Format_description_event *fde)
    : Binary_log_event(&buf, fde) {
  BAPI_ENTER("Metadata_event::Metadata_event(const char*, ...)");
  READER_TRY_INITIALIZATION;
  READER_ASSERT_POSITION(fde->common_header_len);

  uint32_t read_len = fde->common_header_len;

  /* Read and intialize every type in the stream for this event */
  while (READER_CALL(available_to_read) > 0) {
    uint8_t type;
    READER_TRY_SET(type, read<uint8_t>);
    read_len += ENCODED_TYPE_SIZE;

    // Read the 'type'
    read_len += read_type(static_cast<Metadata_event_types>(type));
  }

  // Ensure we read everything in the event
  DBUG_ASSERT(read_len == (size_ + LOG_EVENT_HEADER_LEN));

  // Cannot contain both hlc and prev-hlc timestamp in the same metadata event
  DBUG_ASSERT(!(does_exist(Metadata_event_types::HLC_TYPE) &&
                does_exist(Metadata_event_types::PREV_HLC_TYPE)));

  READER_CATCH_ERROR;
  BAPI_VOID_RETURN;
}

void Metadata_event::set_hlc_time(uint64_t hlc_time_ns) {
  hlc_time_ns_ = hlc_time_ns;
  set_exist(Metadata_event_types::HLC_TYPE);
  // Update the size of the event when it gets serialized into the stream.
  size_ += (ENCODED_TYPE_SIZE + ENCODED_LENGTH_SIZE + ENCODED_HLC_SIZE);
}

void Metadata_event::set_exist(Metadata_event_types type) {
  std::size_t index = static_cast<std::size_t>(type);
  DBUG_ASSERT(index < existing_types_.size());
  existing_types_[index] = true;
}

bool Metadata_event::does_exist(Metadata_event_types type) const {
  std::size_t index = static_cast<std::size_t>(type);
  DBUG_ASSERT(index < existing_types_.size());
  return existing_types_[index];
}

uint64_t Metadata_event::get_hlc_time() const { return hlc_time_ns_; }

void Metadata_event::set_prev_hlc_time(uint64_t prev_hlc_time_ns) {
  prev_hlc_time_ns_ = prev_hlc_time_ns;
  set_exist(Metadata_event_types::PREV_HLC_TYPE);

  // Update the size of the event when it gets serialized into the stream.
  size_ += (ENCODED_TYPE_SIZE + ENCODED_LENGTH_SIZE + ENCODED_PREV_HLC_SIZE);
}

uint64_t Metadata_event::get_prev_hlc_time() const { return prev_hlc_time_ns_; }

void Metadata_event::set_raft_term_and_index(int64_t term, int64_t index) {
  raft_term_ = term;
  raft_index_ = index;

  set_exist(Metadata_event_types::RAFT_TERM_INDEX_TYPE);

  // Update the size of the event when it gets serialized into the stream.
  size_ +=
      (ENCODED_TYPE_SIZE + ENCODED_LENGTH_SIZE + ENCODED_RAFT_TERM_INDEX_SIZE);
}

int64_t Metadata_event::get_raft_term() const { return raft_term_; }

int64_t Metadata_event::get_raft_index() const { return raft_index_; }

void Metadata_event::set_raft_str(const std::string &raft_str) {
  raft_str_ = raft_str;
  set_exist(Metadata_event_types::RAFT_GENERIC_STR_TYPE);

  // Update the size of the event when it gets serialized into the stream.
  size_ += (ENCODED_TYPE_SIZE + ENCODED_LENGTH_SIZE + raft_str_.size());
}

const std::string &Metadata_event::get_raft_str() const { return raft_str_; }

void Metadata_event::set_raft_prev_opid(int64_t term, int64_t index) {
  prev_raft_term_ = term;
  prev_raft_index_ = index;

  set_exist(Metadata_event_types::RAFT_PREV_OPID_TYPE);
  // Update the size of the event when it gets serialized into the stream.
  size_ +=
      (ENCODED_TYPE_SIZE + ENCODED_LENGTH_SIZE + ENCODED_RAFT_PREV_OPID_SIZE);
}

void Metadata_event::set_raft_rotate_tag(
    Metadata_event::RAFT_ROTATE_EVENT_TAG t) {
  raft_rotate_tag_ = t;
  set_exist(Metadata_event_types::RAFT_ROTATE_TAG_TYPE);
  // Update the size of the event when it gets serialized into the stream.
  size_ +=
      (ENCODED_TYPE_SIZE + ENCODED_LENGTH_SIZE + ENCODED_RAFT_ROTATE_TAG_SIZE);
}

Metadata_event::RAFT_ROTATE_EVENT_TAG Metadata_event::get_rotate_tag() const {
  return raft_rotate_tag_;
}

std::string Metadata_event::get_rotate_tag_string() const {
  switch (raft_rotate_tag_) {
    case RRET_SIMPLE_ROTATE:
      return "Rotate";
    case RRET_NOOP:
      return "No-Op";
    case RRET_CONFIG_CHANGE:
      return "Config-Change";
    case RRET_NOT_ROTATE:
      return "Non-Rotate (Unexpected)";
    default:
      return "Invalid (Unexpected)";
  }
}

int64_t Metadata_event::get_raft_prev_opid_term() const {
  return prev_raft_term_;
}

int64_t Metadata_event::get_raft_prev_opid_index() const {
  return prev_raft_index_;
}

uint Metadata_event::read_type(Metadata_event_types type) {
  BAPI_ENTER("Metadata_event::read_type");
  using MET = Metadata_event_types;

  // Read the 'length' of the field's value
  uint16_t value_length = 0;
  uint64_t hlc_time = 0;
  uint64_t prev_hlc_time = 0;
  int64_t term = -1, index = -1;
  const char *ptr_raft_str = nullptr;
  int64_t prev_term = -1, prev_index = -1;
  int16_t raft_rotate_tag = RRET_NOT_ROTATE;
  READER_TRY_SET(value_length, read_and_letoh<uint16_t>);

  switch (type) {
    case MET::HLC_TYPE:
      // HLC is a 8 byte numerical field
      DBUG_ASSERT(value_length == 8);
      READER_TRY_SET(hlc_time, read_and_letoh<uint64_t>);
      set_hlc_time(hlc_time);
      break;
    case MET::PREV_HLC_TYPE:
      // Previous HLC is a 8 byte numerical field
      DBUG_ASSERT(value_length == 8);
      READER_TRY_SET(prev_hlc_time, read_and_letoh<uint64_t>);
      set_prev_hlc_time(prev_hlc_time);
      break;
    case MET::RAFT_TERM_INDEX_TYPE:
      DBUG_ASSERT(value_length == ENCODED_RAFT_TERM_INDEX_SIZE);
      READER_TRY_SET(term, read_and_letoh<int64_t>);
      READER_TRY_SET(index, read_and_letoh<int64_t>);
      set_raft_term_and_index(term, index);
      break;
    case MET::RAFT_GENERIC_STR_TYPE:
      ptr_raft_str = READER_TRY_CALL(ptr, value_length);
      set_raft_str(std::string(ptr_raft_str, value_length));
      break;
    case MET::RAFT_PREV_OPID_TYPE:
      DBUG_ASSERT(value_length == ENCODED_RAFT_PREV_OPID_SIZE);
      READER_TRY_SET(prev_term, read_and_letoh<int64_t>);
      READER_TRY_SET(prev_index, read_and_letoh<int64_t>);
      set_raft_prev_opid(prev_term, prev_index);
      break;
    case MET::RAFT_ROTATE_TAG_TYPE:
      DBUG_ASSERT(value_length == ENCODED_RAFT_ROTATE_TAG_SIZE);
      READER_TRY_SET(raft_rotate_tag, read_and_letoh<int16_t>);
      set_raft_rotate_tag((RAFT_ROTATE_EVENT_TAG)raft_rotate_tag);
      break;

    default:
      // This is a event which we do not know about. Just skip this
      READER_CALL(ptr, value_length);
      size_ += (ENCODED_TYPE_SIZE + ENCODED_LENGTH_SIZE + value_length);
      break;
  }

  // return total length read (ENCODED_LENGTH_SIZE and the length of the
  // 'VALUE' of the field)
  READER_CATCH_ERROR;
  BAPI_RETURN(value_length + ENCODED_LENGTH_SIZE);
}

Gtid_event::Gtid_event(const char *buf, const Format_description_event *fde)
    : Binary_log_event(&buf, fde),
      last_committed(SEQ_UNINIT),
      sequence_number(SEQ_UNINIT),
      may_have_sbr_stmts(true),
      original_commit_timestamp(0),
      immediate_commit_timestamp(0),
      transaction_length(0),
      original_server_version(0),
      immediate_server_version(0) {
  /*
    The layout of the buffer is as follows:

    +------------+
    |     1 byte | Flags
    +------------+
    |    16 bytes| Encoded SID
    +------------+
    |     8 bytes| Encoded GNO
    +------------+
    |     1 byte | lt_type
    +------------+
    |     8 bytes| last_committed
    +------------+
    |     8 bytes| sequence_number
    +------------+
    |  7/14 bytes| timestamps*
    +------------+
    |1 to 9 bytes| transaction_length (see net_length_size())
    +------------+
    |   4/8 bytes| original/immediate_server_version (see timestamps*)
    +------------+

    The 'Flags' field contains gtid flags.

    lt_type (for logical timestamp typecode) is always equal to the
    constant LOGICAL_TIMESTAMP_TYPECODE.

    5.6 did not have TS_TYPE and the following fields. 5.7.4 and
    earlier had a different value for TS_TYPE and a shorter length for
    the following TS fields. Both these cases are accepted and ignored.

   * The section titled "timestamps" contains commit timestamps on originating
     server and commit timestamp on the immediate master.

     This is how we write the timestamps section serialized to a memory buffer.

     if original_commit_timestamp != immediate_commit_timestamp:

       +-7 bytes, high bit (1<<55) set-----+-7 bytes----------+
       | immediate_commit_timestamp        |original_timestamp|
       +-----------------------------------+------------------+

     else:

       +-7 bytes, high bit (1<<55) cleared-+
       | immediate_commit_timestamp        |
       +-----------------------------------+
  */
  BAPI_ENTER("Gtid_event::Gtid_event(const char*, ...)");
  READER_TRY_INITIALIZATION;
  READER_ASSERT_POSITION(fde->common_header_len);
  unsigned char gtid_flags;
  READER_TRY_SET(gtid_flags, read<unsigned char>);
  may_have_sbr_stmts = gtid_flags & FLAG_MAY_HAVE_SBR;

  READER_TRY_CALL(memcpy<unsigned char *>, Uuid_parent_struct.bytes,
                  Uuid_parent_struct.BYTE_LENGTH);
  // SIDNO is only generated if needed, in get_sidno().
  gtid_info_struct.rpl_gtid_sidno = -1;

  READER_TRY_SET(gtid_info_struct.rpl_gtid_gno, read_and_letoh<int64_t>);

  /* GNO sanity check */
  if (header()->type_code == GTID_LOG_EVENT) {
    if (gtid_info_struct.rpl_gtid_gno < MIN_GNO ||
        gtid_info_struct.rpl_gtid_gno > MAX_GNO)
      READER_THROW("Invalid GNO");
  } else { /* Assume this is an ANONYMOUS_GTID_LOG_EVENT */
    BAPI_ASSERT(header()->type_code == ANONYMOUS_GTID_LOG_EVENT);
    if (gtid_info_struct.rpl_gtid_gno != 0) READER_THROW("Invalid GNO");
  }

  /*
    Fetch the logical clocks. Check the length before reading, to
    avoid out of buffer reads.
  */
  if (READER_CALL(can_read, LOGICAL_TIMESTAMP_TYPECODE_LENGTH)) {
    uint8_t lc_typecode = 0;
    READER_TRY_SET(lc_typecode, read<uint8_t>);
    if (lc_typecode == LOGICAL_TIMESTAMP_TYPECODE) {
      READER_TRY_SET(last_committed, read_and_letoh<uint64_t>);
      READER_TRY_SET(sequence_number, read_and_letoh<uint64_t>);

      /*
        Fetch the timestamps used to monitor replication lags with respect to
        the immediate master and the server that originated this transaction.
        Check that the timestamps exist before reading. Note that a master
        older than MySQL-5.8 will NOT send these timestamps. We should be
        able to ignore these fields in this case.
      */
      has_commit_timestamps =
          READER_CALL(can_read, IMMEDIATE_COMMIT_TIMESTAMP_LENGTH);
      if (has_commit_timestamps) {
        READER_TRY_SET(immediate_commit_timestamp, read_and_letoh<uint64_t>,
                       IMMEDIATE_COMMIT_TIMESTAMP_LENGTH);
        // Check the MSB to determine how to populate
        // original_commit_timestamps
        if ((immediate_commit_timestamp &
             (1ULL << ENCODED_COMMIT_TIMESTAMP_LENGTH)) != 0) {
          // Read the original_commit_timestamp
          immediate_commit_timestamp &=
              ~(1ULL << ENCODED_COMMIT_TIMESTAMP_LENGTH); /* Clear MSB. */
          READER_TRY_SET(original_commit_timestamp, read_and_letoh<uint64_t>,
                         ORIGINAL_COMMIT_TIMESTAMP_LENGTH);
        } else {
          // The transaction originated in the previous server
          original_commit_timestamp = immediate_commit_timestamp;
        }

        /* Fetch the transaction length if possible */
        if (READER_CALL(can_read, TRANSACTION_LENGTH_MIN_LENGTH)) {
          READER_TRY_SET(transaction_length, net_field_length_ll);
        }

        /**
          Fetch the original/immediate_server_version. Set it to
          UNDEFINED_SERVER_VERSION if no version can be fetched.
        */
        original_server_version = UNDEFINED_SERVER_VERSION;
        immediate_server_version = UNDEFINED_SERVER_VERSION;
        if (READER_CALL(can_read, IMMEDIATE_SERVER_VERSION_LENGTH)) {
          READER_TRY_SET(immediate_server_version, read_and_letoh<uint32_t>);
          // Check the MSB to determine how to populate original_server_version
          if ((immediate_server_version &
               (1ULL << ENCODED_SERVER_VERSION_LENGTH)) != 0) {
            // Read the original_server_version
            immediate_server_version &=
                ~(1ULL << ENCODED_SERVER_VERSION_LENGTH);  // Clear MSB
            READER_TRY_SET(original_server_version, read_and_letoh<uint32_t>,
                           ORIGINAL_SERVER_VERSION_LENGTH);
          } else
            original_server_version = immediate_server_version;
        }
      }
    }
  }

  READER_CATCH_ERROR;
  BAPI_VOID_RETURN;
}

Previous_gtids_event::Previous_gtids_event(const char *buffer,
                                           const Format_description_event *fde)
    : Binary_log_event(&buffer, fde) {
  BAPI_ENTER("Previous_gtids_event::Previous_gtids_event(const char*, ...)");
  READER_TRY_INITIALIZATION;
  READER_ASSERT_POSITION(fde->common_header_len);
  READER_TRY_CALL(forward, fde->post_header_len[PREVIOUS_GTIDS_LOG_EVENT - 1]);

  buf = (const unsigned char *)READER_CALL(ptr);
  buf_size = READER_CALL(available_to_read);
  if (buf_size < 8) READER_THROW("Invalid Previous_gtids information");
  READER_CATCH_ERROR;
  BAPI_VOID_RETURN;
}

Transaction_context_event::Transaction_context_event(
    const char *buffer, const Format_description_event *fde)
    : Binary_log_event(&buffer, fde) {
  BAPI_ENTER(
      "Transaction_context_event::"
      "Transaction_context_event(const char*, ...)");
  READER_TRY_INITIALIZATION;
  READER_ASSERT_POSITION(fde->common_header_len);

  server_uuid = nullptr;
  encoded_snapshot_version = nullptr;

  uint8_t server_uuid_len;
  uint32_t write_set_len;
  uint32_t read_set_len;

  READER_TRY_SET(server_uuid_len, read<uint8_t>);
  READER_TRY_SET(thread_id, read_and_letoh<uint32_t>);
  READER_TRY_SET(gtid_specified, read<bool>);
  READER_TRY_SET(encoded_snapshot_version_length, read_and_letoh<uint32_t>);
  READER_TRY_SET(write_set_len, read_and_letoh<uint32_t>);
  READER_TRY_SET(read_set_len, read_and_letoh<uint32_t>);

  READER_TRY_SET(server_uuid, strndup<const char *>, server_uuid_len);
  READER_TRY_SET(encoded_snapshot_version, strndup<const unsigned char *>,
                 encoded_snapshot_version_length);
  READER_TRY_CALL(read_data_set, write_set_len, &write_set);
  READER_TRY_CALL(read_data_set, read_set_len, &read_set);

  READER_CATCH_ERROR;
  BAPI_VOID_RETURN;
}

/**
  Function to clear the memory of the write_set and the read_set

  @param[in] set - pointer to write_set or read_set.
*/
void Transaction_context_event::clear_set(std::list<const char *> *set) {
  for (std::list<const char *>::iterator it = set->begin(); it != set->end();
       ++it)
    bapi_free(const_cast<char *>(*it));
  set->clear();
}

/**
  Destructor of the Transaction_context_event class.
*/
Transaction_context_event::~Transaction_context_event() {
  if (server_uuid) bapi_free(const_cast<char *>(server_uuid));
  server_uuid = nullptr;
  if (encoded_snapshot_version)
    bapi_free(const_cast<unsigned char *>(encoded_snapshot_version));
  encoded_snapshot_version = nullptr;
  clear_set(&write_set);
  clear_set(&read_set);
}

View_change_event::View_change_event(const char *raw_view_id)
    : Binary_log_event(VIEW_CHANGE_EVENT),
      view_id(),
      seq_number(0),
      certification_info() {
  strncpy(view_id, raw_view_id, sizeof(view_id) - 1);
  view_id[sizeof(view_id) - 1] = 0;
}

View_change_event::View_change_event(const char *buffer,
                                     const Format_description_event *fde)
    : Binary_log_event(&buffer, fde),
      view_id(),
      seq_number(0),
      certification_info() {
  BAPI_ENTER("View_change_event::View_change_event(const char*, ...)");
  READER_TRY_INITIALIZATION;
  READER_ASSERT_POSITION(fde->common_header_len);
  uint32_t cert_info_len;

  READER_TRY_CALL(memcpy<char *>, view_id, ENCODED_VIEW_ID_MAX_LEN);
  if (strlen(view_id) == 0) READER_THROW("Invalid View_change information");

  READER_TRY_SET(seq_number, read_and_letoh<uint64_t>);
  READER_TRY_SET(cert_info_len, read_and_letoh<uint32_t>);
  READER_TRY_CALL(read_data_map, cert_info_len, &certification_info);

  READER_CATCH_ERROR;
  BAPI_VOID_RETURN;
}

/**
  Destructor of the View_change_event class.
*/
View_change_event::~View_change_event() { certification_info.clear(); }

Heartbeat_event::Heartbeat_event(const char *buf,
                                 const Format_description_event *fde)
    : Binary_log_event(&buf, fde) {
  BAPI_ENTER("Heartbeat_event::Heartbeat_event(const char*, ...)");
  READER_TRY_INITIALIZATION;
  READER_ASSERT_POSITION(fde->common_header_len);

  READER_TRY_SET(log_ident, ptr);
  if (log_ident == nullptr || header()->log_pos < BIN_LOG_HEADER_SIZE)
    READER_THROW("Invalid Heartbeat information");

  ident_len = READER_CALL(available_to_read);
  if (ident_len > FN_REFLEN - 1) ident_len = FN_REFLEN - 1;

  READER_CATCH_ERROR;
  BAPI_VOID_RETURN;
}

#ifndef HAVE_MYSYS
void Rotate_event::print_event_info(std::ostream &info) {
  info << "Binlog Position: " << pos;
  info << ", Log name: " << new_log_ident;
}

void Rotate_event::print_long_info(std::ostream &info) {
  info << "Timestamp: " << header()->when.tv_sec;
  info << "\t";
  this->print_event_info(info);
}

void Format_description_event::print_event_info(std::ostream &info) {
  info << "Server ver: " << server_version;
  info << ", Binlog ver: " << binlog_version;
}

void Format_description_event::print_long_info(std::ostream &info) {
  this->print_event_info(info);
  info << "\nCreated timestamp: " << created;
  info << "\tCommon Header Length: " << common_header_len;
  info << "\nPost header length for events: \n";
}

void Incident_event::print_event_info(std::ostream &info) {
  info << get_message();
  info << get_incident_type();
}

void Incident_event::print_long_info(std::ostream &info) {
  this->print_event_info(info);
}

void Xid_event::print_event_info(std::ostream &info) {
  info << "Xid ID=" << xid;
}

void Xid_event::print_long_info(std::ostream &info) {
  info << "Timestamp: " << header()->when.tv_sec;
  info << "\t";
  this->print_event_info(info);
}

#endif  // end HAVE_MYSYS

}  // end namespace binary_log
