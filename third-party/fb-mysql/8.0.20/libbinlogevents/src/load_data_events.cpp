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

#include "load_data_events.h"
#include "event_reader_macros.h"

namespace binary_log {
/**
  The constructor is called by MySQL slave, while applying the events.
*/
Execute_load_query_event::Execute_load_query_event(
    uint32_t file_id_arg, uint32_t fn_pos_start_arg, uint32_t fn_pos_end_arg,
    enum_load_dup_handling dup_arg)
    : Query_event(EXECUTE_LOAD_QUERY_EVENT),
      file_id(file_id_arg),
      fn_pos_start(fn_pos_start_arg),
      fn_pos_end(fn_pos_end_arg),
      dup_handling(dup_arg) {}

/**
  The constructor used in order to decode EXECUTE_LOAD_QUERY_EVENT from a
  packet. It is used on the MySQL server acting as a slave.
*/
Execute_load_query_event::Execute_load_query_event(
    const char *buf, const Format_description_event *fde)
    : Query_event(buf, fde, EXECUTE_LOAD_QUERY_EVENT),
      file_id(0),
      fn_pos_start(0),
      fn_pos_end(0) {
  uint8_t dup_temp;
  BAPI_ENTER(
      "Execute_load_query_event::Execute_load_query_event(const char*, const "
      "Format_description_event*)");
  READER_TRY_INITIALIZATION;

  READER_TRY_CALL(go_to, fde->common_header_len);
  READER_TRY_CALL(forward, ELQ_FILE_ID_OFFSET);
  READER_TRY_SET(file_id, read_and_letoh<int32_t>);
  READER_TRY_SET(fn_pos_start, read_and_letoh<uint32_t>);
  READER_TRY_SET(fn_pos_end, read_and_letoh<uint32_t>);
  READER_TRY_SET(dup_temp, read<uint8_t>);
  dup_handling = (enum_load_dup_handling)(dup_temp);

  /* Sanity check */
  if (fn_pos_start > q_len || fn_pos_end > q_len ||
      dup_handling > LOAD_DUP_REPLACE)
    READER_THROW("Invalid Execute_load_query_event.");

  READER_CATCH_ERROR;
  BAPI_VOID_RETURN;
}

Delete_file_event::Delete_file_event(const char *buf,
                                     const Format_description_event *fde)
    : Binary_log_event(&buf, fde), file_id(0) {
  BAPI_ENTER(
      "Delete_file_event::Delete_file_event(const char*, const "
      "Format_description_event*)");
  READER_TRY_INITIALIZATION;

  READER_ASSERT_POSITION(fde->common_header_len);
  READER_TRY_SET(file_id, read_and_letoh<uint32_t>);
  if (file_id == 0) READER_THROW("Invalid file_id");

  READER_CATCH_ERROR;
  BAPI_VOID_RETURN;
}

Append_block_event::Append_block_event(const char *buf,
                                       const Format_description_event *fde)
    : Binary_log_event(&buf, fde), block(nullptr) {
  BAPI_ENTER(
      "Append_block_event::Append_block_event(const char*, const "
      "Format_description_event*)");
  READER_TRY_INITIALIZATION;
  READER_ASSERT_POSITION(fde->common_header_len);

  READER_TRY_SET(file_id, read_and_letoh<uint32_t>);
  block_len = READER_CALL(available_to_read);
  block = const_cast<unsigned char *>(
      reinterpret_cast<const unsigned char *>(READER_CALL(ptr, block_len)));

  READER_CATCH_ERROR;
  BAPI_VOID_RETURN;
}

Begin_load_query_event::Begin_load_query_event(
    const char *buf, const Format_description_event *fde)
    : Append_block_event(buf, fde) {
  BAPI_ENTER(
      "Begin_load_query_event::Begin_load_query_event(const char*, const "
      "Format_description_event*)");
  BAPI_VOID_RETURN;
}
}  // end namespace binary_log
