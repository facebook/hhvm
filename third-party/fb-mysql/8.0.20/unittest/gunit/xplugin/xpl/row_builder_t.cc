/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stddef.h>
#include <stdio.h>
#include <limits>
#include <set>
#include <string>

#include "decimal.h"

#include "plugin/x/client/mysqlxclient/xdatetime.h"
#include "plugin/x/client/mysqlxclient/xdecimal.h"
#include "plugin/x/client/mysqlxclient/xrow.h"
#include "plugin/x/client/xrow_impl.h"
#include "plugin/x/ngs/include/ngs/protocol/page_pool.h"
#include "plugin/x/ngs/include/ngs/protocol/protocol_protobuf.h"
#include "plugin/x/protocol/encoders/encoding_xrow.h"
#include "unittest/gunit/xplugin/xpl/protobuf_message.h"

namespace protocol {
namespace test {

using ::xpl::test::message_with_header_from_buffer;

template <typename Expected_value_type, typename Method_type>
void assert_row_getter(const Expected_value_type &expected_value,
                       const Method_type &method, const std::string &buffer) {
  Expected_value_type value;

  const bool result = method(buffer, &value);

  ASSERT_TRUE(result);
  ASSERT_EQ(expected_value, value);
}

class Row_builder_testsuite : public testing::Test {
 public:
  std::string get_buffer_as_string() {
    std::string result;
    auto page = m_buffer.m_front;

    while (page) {
      result += std::string(reinterpret_cast<const char *>(page->m_begin_data),
                            page->get_used_bytes());
      page = page->m_next_page;
    }

    return result;
  }

  ngs::Memory_block_pool m_memory_block_pool{{20, k_minimum_page_size}};
  Encoding_pool m_pool{10, &m_memory_block_pool};
  Encoding_buffer m_buffer{&m_pool};
  XMessage_encoder m_encoder{&m_buffer};
  XRow_encoder m_row{&m_encoder};
};

TEST_F(Row_builder_testsuite, row_start) {
  m_row.begin_row();

  m_row.field_null();
  m_row.field_null();

  ASSERT_EQ(2u, m_row.get_num_fields());

  m_row.begin_row();
  m_row.end_row();

  ASSERT_EQ(0u, m_row.get_num_fields());
}

TEST_F(Row_builder_testsuite, row_msg_size) {
  m_row.begin_row();
  m_row.field_null();
  m_row.end_row();

  auto row_data = get_buffer_as_string();
  // 1 byte for msg tag + 1 byte for field header + 1 byte
  // for field value (NULL)
  ASSERT_EQ(7, row_data.length());
  ASSERT_EQ(3u, row_data[0]);
  ASSERT_EQ(0u, row_data[1]);
  ASSERT_EQ(0u, row_data[2]);
  ASSERT_EQ(0u, row_data[3]);

  m_row.begin_row();
  m_row.field_null();
  m_row.field_null();
  m_row.end_row();

  // offset of the size is 7 (3 bytes for prev msg + 4 for its size)
  auto two_row_data = get_buffer_as_string();
  // 1 byte for msg tag + 2*(1 byte for field header + 1 byte
  // for field value (NULL))
  ASSERT_EQ(16, two_row_data.length());
  ASSERT_EQ(5u, two_row_data[7]);
  ASSERT_EQ(0u, two_row_data[8]);
  ASSERT_EQ(0u, two_row_data[9]);
  ASSERT_EQ(0u, two_row_data[10]);
}

TEST_F(Row_builder_testsuite, row_abort) {
  m_row.begin_row();

  m_row.field_null();
  m_row.field_null();

  m_row.abort_row();
  m_row.end_row();

  auto row_data = get_buffer_as_string();
  ASSERT_EQ(0u, row_data.length());
}

TEST_F(Row_builder_testsuite, fields_qty) {
  m_row.begin_row();

  ASSERT_EQ(0u, m_row.get_num_fields());

  m_row.field_null();
  m_row.field_null();

  ASSERT_EQ(2u, m_row.get_num_fields());

  m_row.field_unsigned_longlong(0);
  m_row.field_float(0.0f);
  m_row.field_float(0.0f);

  ASSERT_EQ(5u, m_row.get_num_fields());

  m_row.end_row();
}

TEST_F(Row_builder_testsuite, null_field) {
  m_row.begin_row();

  m_row.field_null();

  m_row.end_row();

  std::unique_ptr<Mysqlx::Resultset::Row> row(
      message_with_header_from_buffer<Mysqlx::Resultset::Row>(
          get_buffer_as_string()));

  ASSERT_EQ(1, row->field_size());
  ASSERT_EQ(0u, row->field(0).length());
}

TEST_F(Row_builder_testsuite, unsigned64_field) {
  std::string *buffer;
  int idx = 0;

  m_row.begin_row();

  m_row.field_unsigned_longlong(0);
  m_row.field_unsigned_longlong(500);
  m_row.field_unsigned_longlong(10000000);
  m_row.field_unsigned_longlong(0x7fffffffffffffffLL);
  m_row.field_unsigned_longlong(1);
  m_row.field_unsigned_longlong(0xffffffffffffffffLL);

  m_row.end_row();
  std::unique_ptr<Mysqlx::Resultset::Row> row(
      message_with_header_from_buffer<Mysqlx::Resultset::Row>(
          get_buffer_as_string()));

  buffer = row->mutable_field(idx++);
  assert_row_getter<uint64_t>(static_cast<uint64_t>(0),
                              &xcl::row_decoder::buffer_to_u64, *buffer);
  buffer = row->mutable_field(idx++);
  assert_row_getter<uint64_t>(500, &xcl::row_decoder::buffer_to_u64, *buffer);
  buffer = row->mutable_field(idx++);
  assert_row_getter<uint64_t>(10000000, &xcl::row_decoder::buffer_to_u64,
                              *buffer);
  buffer = row->mutable_field(idx++);
  assert_row_getter<uint64_t>(0x7fffffffffffffffULL,
                              &xcl::row_decoder::buffer_to_u64, *buffer);
  buffer = row->mutable_field(idx++);
  assert_row_getter<uint64_t>(1, &xcl::row_decoder::buffer_to_u64, *buffer);
  buffer = row->mutable_field(idx++);
  assert_row_getter<uint64_t>(0xffffffffffffffffULL,
                              &xcl::row_decoder::buffer_to_u64, *buffer);
}

TEST_F(Row_builder_testsuite, signed64_field) {
  std::string *buffer;
  int idx = 0;

  m_row.begin_row();

  m_row.field_signed_longlong(0);
  m_row.field_signed_longlong(-500);
  m_row.field_signed_longlong(-10000000);
  m_row.field_signed_longlong(0x7fffffffffffffffLL);
  m_row.field_signed_longlong(-1);

  m_row.end_row();
  std::unique_ptr<Mysqlx::Resultset::Row> row(
      message_with_header_from_buffer<Mysqlx::Resultset::Row>(
          get_buffer_as_string()));

  buffer = row->mutable_field(idx++);
  assert_row_getter<int64_t>(0, &xcl::row_decoder::buffer_to_s64, *buffer);
  buffer = row->mutable_field(idx++);
  assert_row_getter<int64_t>(-500, &xcl::row_decoder::buffer_to_s64, *buffer);
  buffer = row->mutable_field(idx++);
  assert_row_getter<int64_t>(-10000000, &xcl::row_decoder::buffer_to_s64,
                             *buffer);
  buffer = row->mutable_field(idx++);
  assert_row_getter<int64_t>(0x7fffffffffffffffLL,
                             &xcl::row_decoder::buffer_to_s64, *buffer);
  buffer = row->mutable_field(idx++);
  assert_row_getter<int64_t>(-1, &xcl::row_decoder::buffer_to_s64, *buffer);
}

TEST_F(Row_builder_testsuite, float_field) {
  std::string *buffer;
  int idx = 0;

  m_row.begin_row();

  m_row.field_float(0.0f);
  m_row.field_float(0.0001f);
  m_row.field_float(-10000000.1f);
  m_row.field_float(9999.91992f);
  m_row.field_float(std::numeric_limits<float>::min());
  m_row.field_float(std::numeric_limits<float>::max());

  m_row.end_row();
  std::unique_ptr<Mysqlx::Resultset::Row> row(
      message_with_header_from_buffer<Mysqlx::Resultset::Row>(
          get_buffer_as_string()));

  buffer = row->mutable_field(idx++);
  assert_row_getter<float>(0.0f, &xcl::row_decoder::buffer_to_float, *buffer);
  buffer = row->mutable_field(idx++);
  assert_row_getter<float>(0.0001f, &xcl::row_decoder::buffer_to_float,
                           *buffer);
  buffer = row->mutable_field(idx++);
  assert_row_getter<float>(-10000000.1f, &xcl::row_decoder::buffer_to_float,
                           *buffer);
  buffer = row->mutable_field(idx++);
  assert_row_getter<float>(9999.91992f, &xcl::row_decoder::buffer_to_float,
                           *buffer);

  buffer = row->mutable_field(idx++);
  assert_row_getter<float>(std::numeric_limits<float>::min(),
                           &xcl::row_decoder::buffer_to_float, *buffer);

  buffer = row->mutable_field(idx++);
  assert_row_getter<float>(std::numeric_limits<float>::max(),
                           &xcl::row_decoder::buffer_to_float, *buffer);
}

TEST_F(Row_builder_testsuite, double_field) {
  std::string *buffer;
  int idx = 0;

  m_row.begin_row();

  m_row.field_double(0.0);
  m_row.field_double(0.0001);
  m_row.field_double(-10000000.1);
  m_row.field_double(9999.91992);
  m_row.field_double(std::numeric_limits<double>::min());
  m_row.field_double(std::numeric_limits<double>::max());

  m_row.end_row();
  std::unique_ptr<Mysqlx::Resultset::Row> row(
      message_with_header_from_buffer<Mysqlx::Resultset::Row>(
          get_buffer_as_string()));

  buffer = row->mutable_field(idx++);
  assert_row_getter<double>(0.0, &xcl::row_decoder::buffer_to_double, *buffer);
  buffer = row->mutable_field(idx++);
  assert_row_getter<double>(0.0001, &xcl::row_decoder::buffer_to_double,
                            *buffer);
  buffer = row->mutable_field(idx++);
  assert_row_getter<double>(-10000000.1, &xcl::row_decoder::buffer_to_double,
                            *buffer);
  buffer = row->mutable_field(idx++);
  assert_row_getter<double>(9999.91992, &xcl::row_decoder::buffer_to_double,
                            *buffer);
  buffer = row->mutable_field(idx++);
  assert_row_getter<double>(std::numeric_limits<double>::min(),
                            &xcl::row_decoder::buffer_to_double, *buffer);
  buffer = row->mutable_field(idx++);
  assert_row_getter<double>(std::numeric_limits<double>::max(),
                            &xcl::row_decoder::buffer_to_double, *buffer);
}

TEST_F(Row_builder_testsuite, string_field) {
  std::string *buffer;
  int idx = 0;
  const char *pstr;
  size_t len;
  const char *const STR1 = "ABBABABBBAAA-09-0900--==0,\0\0\0\0\0";
  m_row.begin_row();

  m_row.field_string("", 0);
  m_row.field_string(STR1, strlen(STR1));

  m_row.end_row();
  std::unique_ptr<Mysqlx::Resultset::Row> row(
      message_with_header_from_buffer<Mysqlx::Resultset::Row>(
          get_buffer_as_string()));

  buffer = row->mutable_field(idx++);
  ASSERT_TRUE(xcl::row_decoder::buffer_to_string(*buffer, &pstr, &len));
  ASSERT_STREQ("", pstr);
  ASSERT_EQ(len, 0u);

  buffer = row->mutable_field(idx++);
  ASSERT_TRUE(xcl::row_decoder::buffer_to_string(*buffer, &pstr, &len));
  for (size_t i = 0; i < len; i++) {
    ASSERT_EQ(STR1[i], pstr[i]);
  }
  ASSERT_EQ(len, strlen(STR1));
}

TEST_F(Row_builder_testsuite, date_field) {
  std::string *buffer;
  int idx = 0;

  MYSQL_TIME time;
  time.year = 2006;
  time.month = 3;
  time.day = 24;

  m_row.begin_row();

  m_row.field_date(&time);

  m_row.end_row();
  std::unique_ptr<Mysqlx::Resultset::Row> row(
      message_with_header_from_buffer<Mysqlx::Resultset::Row>(
          get_buffer_as_string()));

  buffer = row->mutable_field(idx++);
  xcl::DateTime xtime;
  bool has_time = false;
  ASSERT_TRUE(xcl::row_decoder::buffer_to_datetime(*buffer, &xtime, has_time));
  ASSERT_EQ(time.year, xtime.year());
  ASSERT_EQ(time.month, xtime.month());
  ASSERT_EQ(time.day, xtime.day());
}

TEST_F(Row_builder_testsuite, time_field) {
  std::string *buffer;
  int idx = 0;

  MYSQL_TIME time;
  time.neg = false;
  time.hour = 12;
  time.minute = 0;
  time.second = 0;
  time.second_part = 999999;

  MYSQL_TIME time2;
  time2.neg = false;
  time2.hour = 0;
  time2.minute = 0;
  time2.second = 0;
  time2.second_part = 0;

  MYSQL_TIME time3;
  time3.neg = true;
  time3.hour = 811;
  time3.minute = 0;
  time3.second = 0;
  time3.second_part = 0;

  m_row.begin_row();

  m_row.field_time(&time);
  m_row.field_time(&time2);
  m_row.field_time(&time3);

  m_row.end_row();
  std::unique_ptr<Mysqlx::Resultset::Row> row{
      message_with_header_from_buffer<Mysqlx::Resultset::Row>(
          get_buffer_as_string())};

  buffer = row->mutable_field(idx++);
  xcl::Time xtime;

  ASSERT_TRUE(xcl::row_decoder::buffer_to_time(*buffer, &xtime));
  ASSERT_TRUE(false == xtime.is_negate());
  ASSERT_EQ(time.hour, xtime.hour());
  ASSERT_EQ(time.minute, xtime.minutes());
  ASSERT_EQ(time.second, xtime.seconds());
  ASSERT_EQ(time.second_part, xtime.useconds());

  buffer = row->mutable_field(idx++);
  ASSERT_TRUE(xcl::row_decoder::buffer_to_time(*buffer, &xtime));
  ASSERT_TRUE(false == xtime.is_negate());
  ASSERT_EQ(time2.hour, xtime.hour());
  ASSERT_EQ(time2.minute, xtime.minutes());
  ASSERT_EQ(time2.second, xtime.seconds());
  ASSERT_EQ(time2.second_part, xtime.useconds());

  buffer = row->mutable_field(idx++);
  ASSERT_TRUE(xcl::row_decoder::buffer_to_time(*buffer, &xtime));
  ASSERT_TRUE(true == xtime.is_negate());
  ASSERT_EQ(time3.hour, xtime.hour());
  ASSERT_EQ(time3.minute, xtime.minutes());
  ASSERT_EQ(time3.second, xtime.seconds());
  ASSERT_EQ(time3.second_part, xtime.useconds());
}

TEST_F(Row_builder_testsuite, datetime_field) {
  std::string *buffer;
  int idx = 0;

  MYSQL_TIME time;
  time.year = 2016;
  time.month = 12;
  time.day = 24;
  time.hour = 13;
  time.minute = 55;
  time.second = 55;
  time.second_part = 999999;
  time.time_type = MYSQL_TIMESTAMP_DATETIME;

  MYSQL_TIME time2;
  time2.year = 2000;
  time2.month = 1;
  time2.day = 1;
  time2.hour = 0;
  time2.minute = 0;
  time2.second = 0;
  time2.second_part = 0;
  time2.time_type = MYSQL_TIMESTAMP_DATETIME;

  m_row.begin_row();

  m_row.field_datetime(&time);
  m_row.field_datetime(&time2);

  m_row.end_row();
  std::unique_ptr<Mysqlx::Resultset::Row> row{
      message_with_header_from_buffer<Mysqlx::Resultset::Row>(
          get_buffer_as_string())};

  buffer = row->mutable_field(idx++);
  xcl::DateTime xtime;
  bool has_time = true;
  ASSERT_TRUE(xcl::row_decoder::buffer_to_datetime(*buffer, &xtime, has_time));
  ASSERT_EQ(time.year, xtime.year());
  ASSERT_EQ(time.month, xtime.month());
  ASSERT_EQ(time.day, xtime.day());
  ASSERT_EQ(time.hour, xtime.hour());
  ASSERT_EQ(time.minute, xtime.minutes());
  ASSERT_EQ(time.second, xtime.seconds());
  ASSERT_EQ(time.second_part, xtime.useconds());

  buffer = row->mutable_field(idx++);
  ASSERT_TRUE(xcl::row_decoder::buffer_to_datetime(*buffer, &xtime, has_time));
  ASSERT_EQ(time2.year, xtime.year());
  ASSERT_EQ(time2.month, xtime.month());
  ASSERT_EQ(time2.day, xtime.day());
  ASSERT_EQ(time2.hour, xtime.hour());
  ASSERT_EQ(time2.minute, xtime.minutes());
  ASSERT_EQ(time2.second, xtime.seconds());
  ASSERT_EQ(time2.second_part, xtime.useconds());
}

TEST_F(Row_builder_testsuite, decimal_field) {
  std::string *buffer;
  int idx = 0;
  xcl::Decimal xdecimal;

  m_row.begin_row();

  decimal_digit_t arr1[] = {1, 0};
  decimal_t dec1 = {1, 1, 2, true, arr1};
  m_row.field_decimal(&dec1);

  decimal_digit_t arr2[] = {1, 0};
  decimal_t dec2 = {1, 1, 2, false, arr2};
  m_row.field_decimal(&dec2);

  m_row.end_row();
  std::unique_ptr<Mysqlx::Resultset::Row> row{
      message_with_header_from_buffer<Mysqlx::Resultset::Row>(
          get_buffer_as_string())};

  buffer = row->mutable_field(idx++);
  ASSERT_TRUE(xcl::row_decoder::buffer_to_decimal(*buffer, &xdecimal));
  ASSERT_EQ("-1.0", xdecimal.to_string());

  buffer = row->mutable_field(idx++);
  ASSERT_TRUE(xcl::row_decoder::buffer_to_decimal(*buffer, &xdecimal));
  ASSERT_EQ("1.0", xdecimal.to_string());
}

TEST_F(Row_builder_testsuite, set_field) {
  std::string *buffer;
  int idx = 0;
  m_row.begin_row();

  const char *const str_abcd = "A,B,C,D";
  const char *const str_a = "A";
  m_row.field_set(str_abcd, strlen(str_abcd));
  m_row.field_set("", 0);  // empty SET case
  m_row.field_set(str_a, strlen(str_a));

  m_row.end_row();
  std::unique_ptr<Mysqlx::Resultset::Row> row(
      message_with_header_from_buffer<Mysqlx::Resultset::Row>(
          get_buffer_as_string()));

  buffer = row->mutable_field(idx++);
  std::set<std::string> elems;
  std::string elems_string;
  ASSERT_TRUE(xcl::row_decoder::buffer_to_string_set(*buffer, &elems_string));
  ASSERT_STREQ("A,B,C,D", elems_string.c_str());
  ASSERT_TRUE(xcl::row_decoder::buffer_to_set(*buffer, &elems));
  ASSERT_EQ(1u, elems.count("A"));
  ASSERT_EQ(1u, elems.count("B"));
  ASSERT_EQ(1u, elems.count("C"));
  ASSERT_EQ(1u, elems.count("D"));
  ASSERT_EQ(4u, elems.size());

  buffer = row->mutable_field(idx++);
  xcl::row_decoder::buffer_to_set(*buffer, &elems);
  ASSERT_EQ(true, elems.empty());
  ASSERT_TRUE(xcl::row_decoder::buffer_to_string_set(*buffer, &elems_string));
  ASSERT_STREQ("", elems_string.c_str());

  buffer = row->mutable_field(idx++);
  xcl::row_decoder::buffer_to_set(*buffer, &elems);
  ASSERT_EQ(1u, elems.size());
  ASSERT_EQ(1u, elems.count("A"));
  ASSERT_TRUE(xcl::row_decoder::buffer_to_string_set(*buffer, &elems_string));
  ASSERT_STREQ("A", elems_string.c_str());
}

TEST_F(Row_builder_testsuite, bit_field) {
  std::string *buffer;
  int idx = 0;

  m_row.begin_row();

  m_row.field_bit("\x00", 1);
  m_row.field_bit("\x01", 1);
  m_row.field_bit("\xff\x00", 2);
  m_row.field_bit("\x00\x00\x00\x00\x00\x00\x00\x00", 8);
  m_row.field_bit("\xff\xff\xff\xff\xff\xff\xff\xff", 8);

  m_row.end_row();
  std::unique_ptr<Mysqlx::Resultset::Row> row{
      message_with_header_from_buffer<Mysqlx::Resultset::Row>(
          get_buffer_as_string())};

  buffer = row->mutable_field(idx++);
  assert_row_getter<uint64_t>(0x0u, &xcl::row_decoder::buffer_to_u64, *buffer);
  buffer = row->mutable_field(idx++);
  assert_row_getter<uint64_t>(0x1u, &xcl::row_decoder::buffer_to_u64, *buffer);
  buffer = row->mutable_field(idx++);
  assert_row_getter<uint64_t>(0xff00u, &xcl::row_decoder::buffer_to_u64,
                              *buffer);
  buffer = row->mutable_field(idx++);
  assert_row_getter<uint64_t>(0x0000000000000000ULL,
                              &xcl::row_decoder::buffer_to_u64, *buffer);
  buffer = row->mutable_field(idx++);
  assert_row_getter<uint64_t>(0xffffffffffffffffULL,
                              &xcl::row_decoder::buffer_to_u64, *buffer);
}

TEST_F(Row_builder_testsuite, datetime_content_type_set) {
  xcl::XRow_impl::Metadata metadata;
  xcl::XRow_impl::Metadata::value_type metadata_row;
  metadata_row.type = xcl::Column_type::DATETIME;
  metadata_row.length = 19;
  metadata_row.has_content_type = true;
  metadata_row.content_type =
      static_cast<uint32_t>(Mysqlx::Resultset::DATETIME);
  metadata.push_back(metadata_row);

  xcl::Context context;

  ::testing::StrictMock<xcl::XRow_impl> row_mock(&metadata, &context);

  MYSQL_TIME time;
  time.year = 2016;
  time.month = 12;
  time.day = 24;
  time.hour = 13;
  time.minute = 55;
  time.second = 55;
  time.second_part = 999999;
  time.time_type = MYSQL_TIMESTAMP_DATETIME;

  m_row.begin_row();
  m_row.field_datetime(&time);
  m_row.end_row();
  std::unique_ptr<Mysqlx::Resultset::Row> row{
      message_with_header_from_buffer<Mysqlx::Resultset::Row>(
          get_buffer_as_string())};
  row_mock.set_row(std::move(row));

  xcl::DateTime result;
  row_mock.get_datetime(0, &result);
  EXPECT_TRUE(result.has_time());
  EXPECT_EQ(result.year(), time.year);
  EXPECT_EQ(result.month(), time.month);
  EXPECT_EQ(result.day(), time.day);
  EXPECT_EQ(result.hour(), time.hour);
  EXPECT_EQ(result.minutes(), time.minute);
  EXPECT_EQ(result.seconds(), time.second);
  EXPECT_EQ(result.useconds(), time.second_part);
}

TEST_F(Row_builder_testsuite, datetime_content_type_not_set_and_has_time_part) {
  xcl::XRow_impl::Metadata metadata;
  xcl::XRow_impl::Metadata::value_type metadata_row;
  metadata_row.type = xcl::Column_type::DATETIME;
  metadata_row.length = 19;
  metadata_row.has_content_type = false;
  metadata.push_back(metadata_row);

  xcl::Context context;

  ::testing::StrictMock<xcl::XRow_impl> row_mock(&metadata, &context);

  MYSQL_TIME time;
  time.year = 2016;
  time.month = 12;
  time.day = 24;
  time.hour = 13;
  time.minute = 55;
  time.second = 55;
  time.second_part = 0;
  time.time_type = MYSQL_TIMESTAMP_DATETIME;

  m_row.begin_row();
  m_row.field_datetime(&time);
  m_row.end_row();
  std::unique_ptr<Mysqlx::Resultset::Row> row{
      message_with_header_from_buffer<Mysqlx::Resultset::Row>(
          get_buffer_as_string())};
  row_mock.set_row(std::move(row));

  xcl::DateTime result;
  row_mock.get_datetime(0, &result);
  EXPECT_EQ(row_mock.get_number_of_fields(), 1);
  EXPECT_TRUE(result.has_time());
  EXPECT_EQ(result.year(), time.year);
  EXPECT_EQ(result.month(), time.month);
  EXPECT_EQ(result.day(), time.day);
  EXPECT_EQ(result.hour(), time.hour);
  EXPECT_EQ(result.minutes(), time.minute);
  EXPECT_EQ(result.seconds(), time.second);
  EXPECT_EQ(result.useconds(), time.second_part);
}

TEST_F(Row_builder_testsuite,
       datetime_content_type_not_set_and_not_contains_time_part) {
  xcl::XRow_impl::Metadata metadata;
  xcl::XRow_impl::Metadata::value_type metadata_row;
  metadata_row.type = xcl::Column_type::DATETIME;
  metadata_row.length = 10;
  metadata_row.has_content_type = false;
  metadata.push_back(metadata_row);

  xcl::Context context;

  ::testing::StrictMock<xcl::XRow_impl> row_mock(&metadata, &context);

  MYSQL_TIME time;
  time.year = 2016;
  time.month = 12;
  time.day = 24;
  time.hour = 0;
  time.minute = 0;
  time.second = 0;
  time.second_part = 0;
  time.time_type = MYSQL_TIMESTAMP_DATE;

  m_row.begin_row();
  m_row.field_datetime(&time);
  m_row.end_row();
  std::unique_ptr<Mysqlx::Resultset::Row> row{
      message_with_header_from_buffer<Mysqlx::Resultset::Row>(
          get_buffer_as_string())};
  row_mock.set_row(std::move(row));

  xcl::DateTime result;
  row_mock.get_datetime(0, &result);
  EXPECT_EQ(result.year(), time.year);
  EXPECT_EQ(result.month(), time.month);
  EXPECT_EQ(result.day(), time.day);

  EXPECT_FALSE(result.has_time());
  EXPECT_EQ(result.hour(), 0xFF);
  EXPECT_EQ(result.minutes(), 0xFF);
  EXPECT_EQ(result.seconds(), 0xFF);
  EXPECT_EQ(result.useconds(), 0xFFFFFF);
}

}  // namespace test

}  // namespace protocol
