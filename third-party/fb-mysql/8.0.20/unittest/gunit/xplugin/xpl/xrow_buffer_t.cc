/* Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <gtest/gtest.h>
#include <stddef.h>

#include "plugin/x/ngs/include/ngs/protocol/protocol_protobuf.h"
#include "plugin/x/protocol/encoders/encoding_xrow.h"
#include "unittest/gunit/xplugin/xpl/encoder_validator.h"
#include "unittest/gunit/xplugin/xpl/protobuf_message.h"

namespace protocol {

namespace test {

class Row_encoder_validator_testsuite : public ::testing::Test {
 public:
  void TearDown() override {
    if (!HasFatalFailure()) m_validator.summarize_buffer("Summarize field");
  }

  Encoder_validator m_validator;
  protocol::XRow_encoder_base<Encoder_validator> m_encoder{&m_validator};
};

TEST_F(Row_encoder_validator_testsuite, encode_begin_end_row) {
  m_validator.configure_allow_bigger_buffers(true);
  m_encoder.begin_row();
  m_encoder.end_row();
}

TEST_F(Row_encoder_validator_testsuite, encode_begin_abort_row) {
  m_validator.configure_allow_bigger_buffers(true);
  m_encoder.begin_row();
  m_encoder.abort_row();
}

TEST_F(Row_encoder_validator_testsuite, encode_null) { m_encoder.field_null(); }

TEST_F(Row_encoder_validator_testsuite, encode_signed_longlong) {
  m_encoder.field_signed_longlong(0);
}

TEST_F(Row_encoder_validator_testsuite, encode_unsigned_longlong) {
  m_encoder.field_unsigned_longlong(0);
}

TEST_F(Row_encoder_validator_testsuite, encode_bit) {
  m_encoder.field_bit("", 0);
}

TEST_F(Row_encoder_validator_testsuite, encode_set_empty) {
  m_encoder.field_set("", 0);
}

TEST_F(Row_encoder_validator_testsuite, encode_set_one) {
  m_encoder.field_set("one", 3);
}

TEST_F(Row_encoder_validator_testsuite, encode_set_two) {
  m_encoder.field_set("one,two", 7);
}

TEST_F(Row_encoder_validator_testsuite, encode_string) {
  m_encoder.field_string("one,two", 7);
}

TEST_F(Row_encoder_validator_testsuite, encode_datetime_empty) {
  MYSQL_TIME date_time;
  date_time.year = 0;
  date_time.month = 0;
  date_time.day = 0;
  date_time.hour = 0;
  date_time.minute = 0;
  date_time.second = 0;
  date_time.second_part = 0;
  m_validator.configure_allow_bigger_buffers(true);

  m_encoder.field_datetime(&date_time);
}

TEST_F(Row_encoder_validator_testsuite, encode_datetime) {
  MYSQL_TIME date_time;
  date_time.year = 2012;
  date_time.month = 12;
  date_time.day = 28;
  date_time.hour = 23;
  date_time.minute = 59;
  date_time.second = 59;
  date_time.second_part = 999999;

  m_encoder.field_datetime(&date_time);
}

TEST_F(Row_encoder_validator_testsuite, encode_time_empty) {
  MYSQL_TIME date_time;
  date_time.neg = true;
  date_time.hour = 0;
  date_time.minute = 0;
  date_time.second = 0;
  date_time.second_part = 0;

  m_validator.configure_allow_bigger_buffers(true);

  m_encoder.field_time(&date_time);
}

TEST_F(Row_encoder_validator_testsuite, encode_time) {
  MYSQL_TIME date_time;
  date_time.neg = true;
  date_time.hour = 23;
  date_time.minute = 59;
  date_time.second = 59;
  date_time.second_part = 999999;

  m_encoder.field_time(&date_time);
}

TEST_F(Row_encoder_validator_testsuite, encode_date) {
  MYSQL_TIME date_time;
  date_time.year = 2012;
  date_time.month = 12;
  date_time.day = 28;

  m_encoder.field_date(&date_time);
}

TEST_F(Row_encoder_validator_testsuite, encode_float) {
  m_encoder.field_float(0.0f);
}

TEST_F(Row_encoder_validator_testsuite, encode_double) {
  m_encoder.field_double(0.0);
}

TEST_F(Row_encoder_validator_testsuite, encode_decimal) {
  const char *k_value = "0.0001";
  m_encoder.field_decimal(k_value, strlen(k_value));
}

TEST_F(Row_encoder_validator_testsuite, encode_decimal2) {
  decimal_digit_t buffer[1000];
  decimal_t d;
  d.buf = buffer;
  d.len = sizeof(buffer) / sizeof(buffer[0]);
  const char *k_value = "0.0001";
  const char *k_value_end = k_value + strlen(k_value);
  ASSERT_EQ(E_DEC_OK, string2decimal(k_value, &d, &k_value_end));

  m_encoder.field_decimal(&d);
}

}  // namespace test

}  // namespace protocol
