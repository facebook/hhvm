/* Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef UNITTEST_GUNIT_XPLUGIN_XPL_ENCODER_VALIDATOR_H_
#define UNITTEST_GUNIT_XPLUGIN_XPL_ENCODER_VALIDATOR_H_

#include <gtest/gtest.h>
#include <stddef.h>
#include <string>

#include "plugin/x/protocol/encoders/encoding_xprotocol.h"

namespace protocol {

namespace test {

class Encoder_validator {
 private:
  std::string get_location() {
    if (m_message_started) {
      return "(field: " + std::to_string(m_field_number) + ")";
    }
    return "(operation-index:" +
           std::to_string(m_field_number + m_raw_data_number) + ")";
  }

  void check_size(const char *info) {
    if (m_buffer_left < 0)
      FAIL() << "Buffer underflow at " << info << get_location()
             << ", please increase the buffer in begin_xmessage or "
                "ensure_buffer_size call";
  }

 public:
  void summarize_buffer(const char *info) {
    if (m_buffer_left > 0 && !m_allow_not_used_data) {
      FAIL() << "Buffer was not filled to its boundaries at " << info
             << ", not used space: " << m_buffer_left;
    }

    if (m_buffer_left < 0) {
      FAIL() << "Buffer is too small at " << info
             << ", we need additional: " << -m_buffer_left
             << " bytes, in total: " << -m_buffer_left + m_buffer_size;
    }
  }

  using Position = XProtocol_encoder::Position;

  template <uint32_t t>
  using Field_delimiter = XProtocol_encoder::Field_delimiter<t>;

  void configure_allow_bigger_buffers(const bool allow_bigger_buffers) {
    m_allow_not_used_data = allow_bigger_buffers;
  }

  template <uint32_t size>
  void ensure_buffer_size() {
    m_buffer_size = m_buffer_left = size;
  }

  template <uint32_t id, uint32_t needed_size>
  Position begin_xmessage() {
    if (m_message_started) {
      ADD_FAILURE() << "Message already started";
      return {};
    }

    m_buffer_size = m_buffer_left = needed_size;
    m_buffer_left -= k_xmsg_header_size;

    return {};
  }

  template <uint32_t id, uint32_t needed_size>
  void begin_xmessage(Position *position) {
    *position = begin_xmessage<id, needed_size>();
  }

  void end_xmessage(const Position &) {
    if (m_message_ended) {
      FAIL() << "Message already finished";
    }

    summarize_buffer("end_xmessage");
  }

  void abort_xmessage(const Position &position) { end_xmessage(position); }

  void encode_fixed_uint32(const uint32_t) {
    ++m_raw_data_number;
    m_buffer_left -= k_fixed32_size;

    check_size("encode_raw_data_fixed_uint32");
  }

  void encode_fixed_uint64(const uint64_t) {
    ++m_raw_data_number;
    m_buffer_left -= k_fixed64_size;

    check_size("encode_raw_data_fixed_uint64");
  }

  void encode_fixedvar16_uint32(const uint32_t) {
    ++m_raw_data_number;
    m_buffer_left -= k_fixed16_size;

    check_size("encode_raw_data_fixedvar16_uint32");
  }

  void encode_fixedvar8_uint8(const uint8_t) {
    ++m_raw_data_number;
    m_buffer_left -= k_fixed8_size;

    check_size("encode_raw_data_fixedvar8_uint8");
  }

  void encode_var_uint32(const uint32_t) {
    ++m_raw_data_number;
    m_buffer_left -= (k_varint32_size);

    check_size("encode_raw_data_uint32");
  }

  void encode_var_uint64(const uint64_t) {
    ++m_raw_data_number;
    m_buffer_left -= (k_varint64_size);

    check_size("encode_raw_data_uint64");
  }

  void encode_var_sint64(const int64_t) {
    ++m_raw_data_number;
    m_buffer_left -= (k_varint64_size);

    check_size("encode_raw_data_sint64");
  }

  template <uint64_t value>
  void encode_const_var_uint() {
    ++m_raw_data_number;
    m_buffer_left -= (k_varint64_size);

    check_size("encode_raw_data_uint64_template");
  }

  template <uint32_t field_id>
  void encode_field_delimited_header() {
    ++m_field_number;
    m_buffer_left -= (k_varint_field_prefix_size);

    check_size("encode_field_delimited");
  }

  template <uint32_t field_id>
  void encode_field_enum(const int32_t) {
    ++m_field_number;
    m_buffer_left -= (k_varint_field_prefix_size + k_varint32_size);

    check_size("encode_field_enum");
  }

  template <uint32_t field_id>
  void encode_optional_field_var_uint64(const uint64_t *) {
    ++m_field_number;
    m_buffer_left -= (k_varint_field_prefix_size + k_varint64_size);

    check_size("encode_optional_field_var_uint64");
  }

  template <uint32_t field_id>
  void encode_optional_field_var_uint32(const uint32_t *) {
    ++m_field_number;
    m_buffer_left -= (k_varint_field_prefix_size + k_varint32_size);

    check_size("encode_optional_field_var_uint32");
  }

  template <uint32_t field_id>
  void encode_field_string(const std::string &) {
    ++m_field_number;
    summarize_buffer("encode_field_string");
    m_buffer_left = 0;
  }

  template <uint32_t field_id, uint64_t value>
  void encode_field_const_var_uint() {
    ++m_field_number;
    m_buffer_left -= (k_varint_field_prefix_size + k_varint64_size);

    check_size("encode_field_const_var_uint");
  }

  template <uint32_t field_id, int64_t value>
  void encode_field_const_enum() {
    ++m_field_number;
    m_buffer_left -= (k_varint_field_prefix_size + k_varint64_size);

    check_size("encode_field_const_enum");
  }

  template <uint32_t field_id>
  void encode_field_var_uint32(const uint32_t) {
    ++m_field_number;
    m_buffer_left -= (k_varint_field_prefix_size + k_varint64_size);

    check_size("k_varint32_size");
  }

  template <uint32_t id, uint32_t delimiter_length = 1>
  Field_delimiter<delimiter_length> begin_delimited_field() {
    ++m_field_number;
    m_buffer_left -= (k_varint_field_prefix_size + k_varint64_size);

    check_size("begin_delimited_field");

    return {};
  }

  template <uint32_t delimiter_length>
  void end_delimited_field(const Field_delimiter<delimiter_length> &) {
    // check_size("end_delimited_field");
  }

  template <uint32_t field_id>
  void encode_field_var_uint64(const uint64_t) {
    ++m_field_number;
    m_buffer_left -= (k_varint_field_prefix_size + k_varint64_size);

    check_size("encode_field_var_uint64");
  }

  void encode_raw(const uint8_t *, uint32_t) { m_buffer_left = 0; }

  template <uint32_t id>
  void empty_xmessage() {
    if (m_message_started) {
      FAIL() << "Message already started";
    }

    if (m_message_ended) {
      FAIL() << "Message already ended";
    }

    m_message_started = true;
    m_message_ended = true;
    check_size("empty_xmessage");
  }

 private:
  const int k_varint_field_prefix_size = 10;
  const int k_varint32_size = 5;
  const int k_varint64_size = 10;
  const int k_fixed8_size = 1;
  const int k_fixed16_size = 2;
  const int k_fixed32_size = 4;
  const int k_fixed64_size = 8;
  const int k_xmsg_header_size = 5;

  bool m_message_started = false;
  bool m_message_ended = false;
  int64_t m_buffer_left = 0;
  int64_t m_buffer_size = 0;
  int m_field_number = 0;
  int m_raw_data_number = 0;
  bool m_allow_not_used_data = false;
};

}  // namespace test

}  // namespace protocol

#endif  // UNITTEST_GUNIT_XPLUGIN_XPL_ENCODER_VALIDATOR_H_
