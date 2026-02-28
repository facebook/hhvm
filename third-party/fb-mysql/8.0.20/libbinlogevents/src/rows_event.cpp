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

#include "rows_event.h"

#include <stdlib.h>
#include <cstring>
#include <string>

#include "../sql/log_event.h"
#include "event_reader_macros.h"

namespace binary_log {

Table_map_event::Table_map_event(const char *buf,
                                 const Format_description_event *fde)
    : Binary_log_event(&buf, fde),
      m_table_id(0),
      m_flags(0),
      m_fb_format(true),
      m_data_size(0),
      m_dbnam(""),
      m_dblen(0),
      m_tblnam(""),
      m_tbllen(0),
      m_colcnt(0),
      m_coltype(nullptr),
      m_field_metadata_size(0),
      m_field_metadata(nullptr),
      m_null_bits(nullptr),
      m_optional_metadata_len(0),
      m_optional_metadata(nullptr),
      m_primary_key_fields_size(0),
      m_primary_key_fields(nullptr),
      m_sign_bits_size(0),
      m_sign_bits(nullptr),
      m_column_names_size(0),
      m_column_names(nullptr) {
  BAPI_ENTER("Table_map_event::Table_map_event(const char*, ...)");
  const char *ptr_dbnam = nullptr;
  const char *ptr_tblnam = nullptr;
  unsigned long long bytes_avail;
  READER_TRY_INITIALIZATION;
  READER_ASSERT_POSITION(fde->common_header_len);

  /* Read the post-header */

  READER_TRY_CALL(forward, TM_MAPID_OFFSET);
  if (fde->post_header_len[TABLE_MAP_EVENT - 1] == 6) {
    /* Master is of an intermediate source tree before 5.1.4. Id is 4 bytes */
    READER_TRY_SET(m_table_id, read_and_letoh<uint64_t>, 4);
  } else {
    BAPI_ASSERT(fde->post_header_len[TABLE_MAP_EVENT - 1] ==
                TABLE_MAP_HEADER_LEN);
    READER_TRY_SET(m_table_id, read_and_letoh<uint64_t>, 6);
  }
  READER_TRY_SET(m_flags, read_and_letoh<uint16_t>);

  m_fb_format = !(m_flags & Table_map_log_event::TM_METADATA_NOT_FB_FORMAT_F);

  /* Read the variable part of the event */

  READER_TRY_SET(m_dblen, read<uint8_t>);
  if (m_dblen > 64 /* NAME_CHAR_LEN */)
    READER_THROW("Database name length too long.")
  ptr_dbnam = READER_TRY_CALL(ptr, m_dblen + 1);
  m_dbnam = std::string(ptr_dbnam, m_dblen);

  READER_TRY_SET(m_tbllen, read<uint8_t>);
  if (m_tbllen > 64 /* NAME_CHAR_LEN */)
    READER_THROW("Table name length too long.")
  ptr_tblnam = READER_TRY_CALL(ptr, m_tbllen + 1);
  m_tblnam = std::string(ptr_tblnam, m_tbllen);

  READER_TRY_SET(m_colcnt, net_field_length_ll);
  READER_TRY_CALL(alloc_and_memcpy, &m_coltype, m_colcnt, 16);

  if (READER_CALL(available_to_read) > 0) {
    READER_TRY_SET(m_field_metadata_size, net_field_length_ll);
    if (m_field_metadata_size > (m_colcnt * 4))
      READER_THROW("Invalid m_field_metadata_size");
    unsigned int num_null_bytes = (m_colcnt + 7) / 8;

    m_sign_bits_size = num_null_bytes;

    READER_TRY_CALL(alloc_and_memcpy, &m_field_metadata, m_field_metadata_size,
                    0);
    READER_TRY_CALL(alloc_and_memcpy, &m_null_bits, num_null_bytes, 0);
  }

  /* After null_bits field, there are some new fields for extra metadata. */
  bytes_avail = READER_CALL(available_to_read);
  if (bytes_avail > 0) {
    /*
      FB format conflicts with upstream 8.0 table map format. This needs
      to be resolved eventually. For now, just ignore upstream format.
     */
    if (m_fb_format) {
      READER_TRY_SET(m_primary_key_fields_size, net_field_length_ll);
      if (m_primary_key_fields_size) {
        READER_TRY_CALL(alloc_and_memcpy, &m_primary_key_fields,
                        m_primary_key_fields_size, 0);
      }

      bytes_avail = READER_CALL(available_to_read);
      if (bytes_avail >= m_sign_bits_size) {
        if (m_sign_bits_size) {
          READER_TRY_CALL(alloc_and_memcpy, &m_sign_bits, m_sign_bits_size, 0);
        }

        m_column_names_size = READER_CALL(available_to_read);
        if (m_column_names_size) {
          READER_TRY_CALL(alloc_and_memcpy, &m_column_names,
                          m_column_names_size, 0);
        }
      }
    } else {
      m_optional_metadata_len = bytes_avail;
      READER_TRY_CALL(alloc_and_memcpy, &m_optional_metadata,
                      m_optional_metadata_len, 0);
    }
  }

  READER_CATCH_ERROR;
  BAPI_VOID_RETURN;
}

Table_map_event::~Table_map_event() {
  bapi_free(m_null_bits);
  m_null_bits = nullptr;
  bapi_free(m_field_metadata);
  m_field_metadata = nullptr;
  bapi_free(m_coltype);
  m_coltype = nullptr;
  bapi_free(m_primary_key_fields);
  m_primary_key_fields = nullptr;
  bapi_free(m_sign_bits);
  m_sign_bits = nullptr;
  bapi_free(m_column_names);
  m_column_names = nullptr;
  bapi_free(m_optional_metadata);
  m_optional_metadata = nullptr;
}

/**
   Parses SIGNEDNESS field.

   @param[out] vec        stores the signedness flags extracted from field.
   @param[in]  reader_obj the Event_reader object containing the serialized
                          field.
   @param[in]  length     length of the field
 */
static void parse_signedness(std::vector<bool> &vec, Event_reader &reader_obj,
                             unsigned int length) {
  for (unsigned int i = 0; i < length; i++) {
    char field = reader_obj.read<unsigned char>();
    if (reader_obj.has_error()) return;
    for (unsigned char c = 0x80; c != 0; c >>= 1) vec.push_back(field & c);
  }
}

/**
   Parses DEFAULT_CHARSET field.

   @param[out] default_charset  stores collation numbers extracted from field.
   @param[in]  reader_obj       the Event_reader object containing the
                                serialized field.
   @param[in]  length           length of the field
 */
static void parse_default_charset(
    Table_map_event::Optional_metadata_fields::Default_charset &default_charset,
    Event_reader &reader_obj, unsigned int length) {
  const char *field = reader_obj.ptr();

  default_charset.default_charset = reader_obj.net_field_length_ll();
  if (reader_obj.has_error()) return;
  while (reader_obj.ptr() < field + length) {
    unsigned int col_index = reader_obj.net_field_length_ll();
    if (reader_obj.has_error()) return;
    unsigned int col_charset = reader_obj.net_field_length_ll();
    if (reader_obj.has_error()) return;

    default_charset.charset_pairs.push_back(
        std::make_pair(col_index, col_charset));
  }
}

/**
   Parses COLUMN_CHARSET field.

   @param[out] vec        stores collation numbers extracted from field.
   @param[in]  reader_obj the Event_reader object containing the serialized
                          field.
   @param[in]  length     length of the field
 */
static void parse_column_charset(std::vector<unsigned int> &vec,
                                 Event_reader &reader_obj,
                                 unsigned int length) {
  const char *field = reader_obj.ptr();

  while (reader_obj.ptr() < field + length) {
    vec.push_back(reader_obj.net_field_length_ll());
    if (reader_obj.has_error()) return;
  }
}

/**
   Parses COLUMN_NAME field.

   @param[out] vec        stores column names extracted from field.
   @param[in]  reader_obj the Event_reader object containing the serialized
                          field.
   @param[in]  length     length of the field
 */
static void parse_column_name(std::vector<std::string> &vec,
                              Event_reader &reader_obj, unsigned int length) {
  const char *field = reader_obj.ptr();

  while (reader_obj.ptr() < field + length) {
    unsigned len = reader_obj.net_field_length_ll();
    if (reader_obj.has_error()) return;

    if (!reader_obj.can_read(len)) {
      reader_obj.set_error("Cannot point to out of buffer bounds");
      return;
    }
    vec.push_back(std::string(reader_obj.ptr(len), len));
  }
}

/**
   Parses SET_STR_VALUE/ENUM_STR_VALUE field.

   @param[out] vec        stores SET/ENUM column's string values extracted from
                          field. Each SET/ENUM column's string values are stored
                          into a string separate vector. All of them are stored
                          in 'vec'.
   @param[in]  reader_obj the Event_reader object containing the serialized
                          field.
   @param[in]  length     length of the field
 */
static void parse_set_str_value(
    std::vector<Table_map_event::Optional_metadata_fields::str_vector> &vec,
    Event_reader &reader_obj, unsigned int length) {
  const char *field = reader_obj.ptr();

  while (reader_obj.ptr() < field + length) {
    unsigned int count = reader_obj.net_field_length_ll();
    if (reader_obj.has_error()) return;

    vec.push_back(std::vector<std::string>());
    for (unsigned int i = 0; i < count; i++) {
      unsigned len1 = reader_obj.net_field_length_ll();
      if (reader_obj.has_error()) return;

      if (!reader_obj.can_read(len1)) {
        reader_obj.set_error("Cannot point to out of buffer bounds");
        return;
      }

      vec.back().push_back(std::string(reader_obj.ptr(len1), len1));
    }
  }
}

/**
   Parses GEOMETRY_TYPE field.

   @param[out] vec        stores geometry column's types extracted from field.
   @param[in]  reader_obj the Event_reader object containing the serialized
                          field.
   @param[in]  length     length of the field
 */
static void parse_geometry_type(std::vector<unsigned int> &vec,
                                Event_reader &reader_obj, unsigned int length) {
  const char *field = reader_obj.ptr();

  while (reader_obj.ptr() < field + length) {
    vec.push_back(reader_obj.net_field_length_ll());
    if (reader_obj.has_error()) return;
  }
}

/**
   Parses SIMPLE_PRIMARY_KEY field.

   @param[out] vec        stores primary key's column information extracted from
                          field. Each column has an index and a prefix which are
                          stored as a unit_pair. prefix is always 0 for
                          SIMPLE_PRIMARY_KEY field.
   @param[in]  reader_obj the Event_reader object containing the serialized
                          field.
   @param[in]  length     length of the field
 */
static void parse_simple_pk(
    std::vector<Table_map_event::Optional_metadata_fields::uint_pair> &vec,
    Event_reader &reader_obj, unsigned int length) {
  const char *field = reader_obj.ptr();

  while (reader_obj.ptr() < field + length) {
    vec.push_back(std::make_pair(reader_obj.net_field_length_ll(), 0));
    if (reader_obj.has_error()) return;
  }
}

/**
   Parses PRIMARY_KEY_WITH_PREFIX field.

   @param[out] vec        stores primary key's column information extracted from
                          field. Each column has an index and a prefix which are
                          stored as a unit_pair.
   @param[in]  reader_obj the Event_reader object containing the serialized
                          field.
   @param[in]  length  length of the field
 */

static void parse_pk_with_prefix(
    std::vector<Table_map_event::Optional_metadata_fields::uint_pair> &vec,
    Event_reader &reader_obj, unsigned int length) {
  const char *field = reader_obj.ptr();

  while (reader_obj.ptr() < field + length) {
    unsigned int col_index = reader_obj.net_field_length_ll();
    if (reader_obj.has_error()) return;
    unsigned int col_prefix = reader_obj.net_field_length_ll();
    if (reader_obj.has_error()) return;
    vec.push_back(std::make_pair(col_index, col_prefix));
  }
}

Table_map_event::Optional_metadata_fields::Optional_metadata_fields(
    unsigned char *optional_metadata, unsigned int optional_metadata_len) {
  char *field = reinterpret_cast<char *>(optional_metadata);
  is_valid = false;

  if (optional_metadata == nullptr) return;

  Event_reader reader_obj(field, optional_metadata_len);
  while (reader_obj.available_to_read()) {
    unsigned int len;
    Optional_metadata_field_type type =
        static_cast<Optional_metadata_field_type>(
            reader_obj.read<unsigned char>());
    if (reader_obj.has_error()) return;

    // Get length and move field to the value.
    len = reader_obj.net_field_length_ll();
    if (reader_obj.has_error()) return;
    switch (type) {
      case SIGNEDNESS:
        parse_signedness(m_signedness, reader_obj, len);
        break;
      case DEFAULT_CHARSET:
        parse_default_charset(m_default_charset, reader_obj, len);
        break;
      case COLUMN_CHARSET:
        parse_column_charset(m_column_charset, reader_obj, len);
        break;
      case COLUMN_NAME:
        parse_column_name(m_column_name, reader_obj, len);
        break;
      case SET_STR_VALUE:
        parse_set_str_value(m_set_str_value, reader_obj, len);
        break;
      case ENUM_STR_VALUE:
        parse_set_str_value(m_enum_str_value, reader_obj, len);
        break;
      case GEOMETRY_TYPE:
        parse_geometry_type(m_geometry_type, reader_obj, len);
        break;
      case SIMPLE_PRIMARY_KEY:
        parse_simple_pk(m_primary_key, reader_obj, len);
        break;
      case PRIMARY_KEY_WITH_PREFIX:
        parse_pk_with_prefix(m_primary_key, reader_obj, len);
        break;
      case ENUM_AND_SET_DEFAULT_CHARSET:
        parse_default_charset(m_enum_and_set_default_charset, reader_obj, len);
        break;
      case ENUM_AND_SET_COLUMN_CHARSET:
        parse_column_charset(m_enum_and_set_column_charset, reader_obj, len);
        break;
      default:
        BAPI_ASSERT(0);
    }
    if (reader_obj.has_error()) return;
  }
  is_valid = true;
}

Rows_event::Rows_event(const char *buf, const Format_description_event *fde)
    : Binary_log_event(&buf, fde),
      m_table_id(0),
      m_width(0),
      columns_before_image(0),
      columns_after_image(0),
      row(0) {
  BAPI_ENTER("Rows_event::Rows_event(const char*, ...)");
  READER_TRY_INITIALIZATION;
  READER_ASSERT_POSITION(fde->common_header_len);
  Log_event_type event_type = header()->type_code;
  size_t var_header_len = 0;
  size_t data_size = 0;
  uint8_t const post_header_len = fde->post_header_len[event_type - 1];
  m_type = event_type;

  if (post_header_len == 6) {
    /* Master is of an intermediate source tree before 5.1.4. Id is 4 bytes */
    READER_TRY_SET(m_table_id, read_and_letoh<uint64_t>, 4);
  } else {
    READER_TRY_SET(m_table_id, read_and_letoh<uint64_t>, 6);
  }
  READER_TRY_SET(m_flags, read_and_letoh<uint16_t>);

  if (post_header_len == ROWS_HEADER_LEN_V2) {
    /*
      Have variable length header, check length,
      which includes length bytes
    */
    READER_TRY_SET(var_header_len, read_and_letoh<uint16_t>);
    var_header_len -= 2;

    /* Iterate over var-len header, extracting 'chunks' */
    uint64_t end = READER_CALL(position) + var_header_len;
    while (READER_CALL(position) < end) {
      int type_placeholder;
      READER_TRY_SET(type_placeholder, read<uint8_t>);

      enum_extra_row_info_typecode type;
      type = (enum_extra_row_info_typecode)type_placeholder;
      switch (type) {
        case enum_extra_row_info_typecode::NDB: {
          /* Have an 'extra info' section, read it in */
          size_t ndb_infolen = 0;
          READER_TRY_SET(ndb_infolen, read<uint8_t>);
          /* ndb_infolen is part of the buffer to be copied below */
          READER_CALL(go_to, READER_CALL(position) - 1);

          /* Just store/use the first tag of this type, skip others */
          if (!m_extra_row_info.have_ndb_info()) {
            const char *ndb_info;
            READER_TRY_SET(ndb_info, ptr, ndb_infolen);
            m_extra_row_info.set_ndb_info(
                reinterpret_cast<const unsigned char *>(ndb_info), ndb_infolen);
            ndb_info = nullptr;
          } else {
            READER_TRY_CALL(forward, ndb_infolen);
          }
          break;
        }
        case enum_extra_row_info_typecode::PART: {
          int part_id_placeholder = 0;
          READER_TRY_SET(part_id_placeholder, read<uint16_t>);
          m_extra_row_info.set_partition_id(part_id_placeholder);
          if (event_type == UPDATE_ROWS_EVENT ||
              event_type == UPDATE_ROWS_EVENT_V1 ||
              event_type == PARTIAL_UPDATE_ROWS_EVENT) {
            READER_TRY_SET(part_id_placeholder, read<uint16_t>);
            m_extra_row_info.set_source_partition_id(part_id_placeholder);
          }
          break;
        }
        default:
          /* Unknown code, we will not understand anything further here */
          READER_CALL(go_to, end); /* Break loop */
      }
      if (READER_CALL(position) > end)
        READER_THROW("Invalid extra rows info header");
    }
  }

  READER_TRY_SET(m_width, net_field_length_ll);
  if (m_width == 0) READER_THROW("Invalid m_width");
  n_bits_len = (m_width + 7) / 8;
  READER_TRY_CALL(assign, &columns_before_image, n_bits_len);

  if (event_type == UPDATE_ROWS_EVENT || event_type == UPDATE_ROWS_EVENT_V1 ||
      event_type == PARTIAL_UPDATE_ROWS_EVENT) {
    READER_TRY_CALL(assign, &columns_after_image, n_bits_len);
  } else
    columns_after_image = columns_before_image;

  data_size = READER_CALL(available_to_read);
  READER_TRY_CALL(assign, &row, data_size);
  // JAG: TODO: Investigate and comment here about the need of this extra byte
  row.push_back(0);

  READER_CATCH_ERROR;
  BAPI_VOID_RETURN;
}

Rows_event::~Rows_event() {}

bool Rows_event::Extra_row_info::compare_extra_row_info(
    const unsigned char *ndb_info_arg, int part_id_arg,
    int source_part_id_arg) {
  const unsigned char *ndb_row_info = m_extra_row_ndb_info;
  bool ndb_info = ((ndb_info_arg == ndb_row_info) ||
                   ((ndb_info_arg != nullptr) && (ndb_row_info != nullptr) &&
                    (ndb_info_arg[EXTRA_ROW_INFO_LEN_OFFSET] ==
                     ndb_row_info[EXTRA_ROW_INFO_LEN_OFFSET]) &&
                    (memcmp(ndb_info_arg, ndb_row_info,
                            ndb_row_info[EXTRA_ROW_INFO_LEN_OFFSET]) == 0)));

  bool part_info = (part_id_arg == m_partition_id) &&
                   (source_part_id_arg == m_source_partition_id);
  return part_info && ndb_info;
}

size_t Rows_event::Extra_row_info::get_ndb_length() {
  if (have_ndb_info())
    return m_extra_row_ndb_info[EXTRA_ROW_INFO_LEN_OFFSET];
  else
    return 0;
}

size_t Rows_event::Extra_row_info::get_part_length() {
  if (have_part()) {
    if (m_source_partition_id != UNDEFINED)
      return EXTRA_ROW_PART_INFO_VALUE_LENGTH * 2;
    return EXTRA_ROW_PART_INFO_VALUE_LENGTH;
  }
  return 0;
}

Rows_event::Extra_row_info::~Extra_row_info() {
  if (have_ndb_info()) {
    bapi_free(m_extra_row_ndb_info);
    m_extra_row_ndb_info = nullptr;
  }
}

Rows_query_event::Rows_query_event(const char *buf,
                                   const Format_description_event *fde)
    : Ignorable_event(buf, fde) {
  BAPI_ENTER("Rows_query_event::Rows_query_event(const char*, ...)");
  READER_TRY_INITIALIZATION;
  READER_ASSERT_POSITION(fde->common_header_len);
  unsigned int len = 0;
  uint8_t const post_header_len =
      fde->post_header_len[ROWS_QUERY_LOG_EVENT - 1];

  m_rows_query = nullptr;

  /*
   m_rows_query length is stored using only one byte (the +1 below), but that
   length is ignored and the complete query is read.
  */
  READER_TRY_CALL(forward, post_header_len + 1);
  len = READER_CALL(available_to_read);
  READER_TRY_CALL(alloc_and_strncpy, &m_rows_query, len, 16);

  READER_CATCH_ERROR;
  BAPI_VOID_RETURN;
}

Rows_query_event::~Rows_query_event() {
  if (m_rows_query) bapi_free(m_rows_query);
}

Write_rows_event::Write_rows_event(const char *buf,
                                   const Format_description_event *fde)
    : Rows_event(buf, fde) {
  BAPI_ENTER("Write_rows_event::Write_rows_event(const char*, ...)");
  READER_TRY_INITIALIZATION;
  this->header()->type_code = m_type;
  BAPI_VOID_RETURN;
}

Update_rows_event::Update_rows_event(const char *buf,
                                     const Format_description_event *fde)
    : Rows_event(buf, fde) {
  BAPI_ENTER("Update_rows_event::Update_rows_event(const char*, ...)");
  READER_TRY_INITIALIZATION;
  this->header()->type_code = m_type;
  BAPI_VOID_RETURN;
}

Delete_rows_event::Delete_rows_event(const char *buf,
                                     const Format_description_event *fde)
    : Rows_event(buf, fde) {
  BAPI_ENTER("Delete_rows_event::Delete_rows_event(const char*, ...)");
  READER_TRY_INITIALIZATION;
  this->header()->type_code = m_type;
  BAPI_VOID_RETURN;
}

#ifndef HAVE_MYSYS
void Table_map_event::print_event_info(std::ostream &info) {
  info << "table id: " << m_table_id << " (" << m_dbnam.c_str() << "."
       << m_tblnam.c_str() << ")";
}

void Table_map_event::print_long_info(std::ostream &info) {
  info << "Timestamp: " << this->header()->when.tv_sec;
  info << "\tFlags: " << m_flags;
  info << "\tColumn Type: ";
  /*
    TODO: Column types are stored as integers. To be
    replaced by string representation of types.
  */
  for (unsigned int i = 0; i < m_colcnt; i++) {
    info << "\t" << (int)m_coltype[i];
  }
  info << "\n";
  this->print_event_info(info);
}

void Rows_event::print_event_info(std::ostream &info) {
  info << "table id: " << m_table_id << " flags: ";
  info << get_flag_string(static_cast<enum_flag>(m_flags));
}

void Rows_event::print_long_info(std::ostream &info) {
  info << "Timestamp: " << this->header()->when.tv_sec;
  info << "\n";

  this->print_event_info(info);

  // TODO: Extract table names and column data.
  if (this->get_event_type() == WRITE_ROWS_EVENT_V1 ||
      this->get_event_type() == WRITE_ROWS_EVENT)
    info << "\nType: Insert";

  if (this->get_event_type() == DELETE_ROWS_EVENT_V1 ||
      this->get_event_type() == DELETE_ROWS_EVENT)
    info << "\nType: Delete";

  if (this->get_event_type() == UPDATE_ROWS_EVENT_V1 ||
      this->get_event_type() == UPDATE_ROWS_EVENT ||
      this->get_event_type() == PARTIAL_UPDATE_ROWS_EVENT)
    info << "\nType: Update";
}
#endif
}  // end namespace binary_log
