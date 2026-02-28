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

#include <gtest/gtest.h>
#include <stddef.h>
#include "decimal.h"
#include "m_ctype.h"
#include "mysql_time.h"
#include "mysys_util.h"
#include "sql/my_decimal.h"
#include "sql/protocol_classic.h"
#include "sql/sql_class.h"
#include "sql_string.h"
#include "unittest/gunit/benchmark.h"
#include "unittest/gunit/test_utils.h"

namespace protocol_classic_unittest {

/**
 * Initializes a Protocol_classic instance before a microbenchmark.
 */
static void SetupProtocolForBenchmark(Protocol_classic *protocol) {
  // Simulate sending results to a client that expects UTF-8 strings.
  protocol->set_result_character_set(&my_charset_utf8mb4_0900_ai_ci);

  // Make sure there is room for a row in the packet buffer without further
  // allocations.
  protocol->get_output_packet()->reserve(1024);
}

static void BM_Protocol_binary_store_date(size_t num_iterations) {
  StopBenchmarkTiming();

  my_testing::Server_initializer initializer;
  initializer.SetUp();
  Protocol_binary *const protocol = initializer.thd()->protocol_binary.get();
  SetupProtocolForBenchmark(protocol);
  String *const packet = protocol->get_output_packet();

  const MysqlTime date(2020, 2, 29, 0, 0, 0, 0, false, MYSQL_TIMESTAMP_DATE);

  StartBenchmarkTiming();

  for (size_t i = 0; i < num_iterations; ++i) {
    packet->length(0);
    protocol->store_date(date);
  }

  StopBenchmarkTiming();
  initializer.TearDown();
}
BENCHMARK(BM_Protocol_binary_store_date)

static void BM_Protocol_binary_store_time(size_t num_iterations) {
  StopBenchmarkTiming();

  my_testing::Server_initializer initializer;
  initializer.SetUp();
  Protocol_binary *const protocol = initializer.thd()->protocol_binary.get();
  SetupProtocolForBenchmark(protocol);
  String *const packet = protocol->get_output_packet();

  const MysqlTime time(0, 0, 0, 123, 59, 59, 670000, false,
                       MYSQL_TIMESTAMP_TIME);

  StartBenchmarkTiming();

  for (size_t i = 0; i < num_iterations; ++i) {
    packet->length(0);
    protocol->store_time(time, 6);
  }

  StopBenchmarkTiming();
  initializer.TearDown();
}
BENCHMARK(BM_Protocol_binary_store_time)

static void BM_Protocol_binary_store_datetime(size_t num_iterations) {
  StopBenchmarkTiming();

  my_testing::Server_initializer initializer;
  initializer.SetUp();
  Protocol_binary *const protocol = initializer.thd()->protocol_binary.get();
  SetupProtocolForBenchmark(protocol);
  String *const packet = protocol->get_output_packet();

  const MysqlTime datetime(2020, 2, 29, 23, 59, 59, 670000, false,
                           MYSQL_TIMESTAMP_DATETIME);

  StartBenchmarkTiming();

  for (size_t i = 0; i < num_iterations; ++i) {
    packet->length(0);
    protocol->store_datetime(datetime, 6);
  }

  StopBenchmarkTiming();
  initializer.TearDown();
}
BENCHMARK(BM_Protocol_binary_store_datetime)

static void BM_Protocol_binary_store_decimal(size_t num_iterations) {
  StopBenchmarkTiming();

  my_testing::Server_initializer initializer;
  initializer.SetUp();
  Protocol_binary *const protocol = initializer.thd()->protocol_binary.get();
  SetupProtocolForBenchmark(protocol);
  String *const packet = protocol->get_output_packet();

  const char decimal_string[] =
      "12345678901234567890123456789012345678901234567890123456789012345";
  my_decimal decimal;
  str2my_decimal(E_DEC_FATAL_ERROR, decimal_string, sizeof(decimal_string) - 1,
                 &my_charset_utf8mb4_bin, &decimal);

  StartBenchmarkTiming();

  for (size_t i = 0; i < num_iterations; ++i) {
    packet->length(0);
    protocol->store_decimal(&decimal, 0, 0);
  }

  StopBenchmarkTiming();
  initializer.TearDown();
}
BENCHMARK(BM_Protocol_binary_store_decimal)

static void BM_Protocol_text_store_tiny(size_t num_iterations) {
  StopBenchmarkTiming();

  my_testing::Server_initializer initializer;
  initializer.SetUp();
  Protocol_text *const protocol = initializer.thd()->protocol_text.get();
  SetupProtocolForBenchmark(protocol);
  String *const packet = protocol->get_output_packet();

  const int value = 123;

  StartBenchmarkTiming();

  for (size_t i = 0; i < num_iterations; ++i) {
    packet->length(0);
    protocol->store_tiny(value, 0);
  }

  StopBenchmarkTiming();
  initializer.TearDown();
}
BENCHMARK(BM_Protocol_text_store_tiny)

static void BM_Protocol_text_store_longlong(size_t num_iterations) {
  StopBenchmarkTiming();

  my_testing::Server_initializer initializer;
  initializer.SetUp();
  Protocol_text *const protocol = initializer.thd()->protocol_text.get();
  SetupProtocolForBenchmark(protocol);
  String *const packet = protocol->get_output_packet();

  const int64_t value = 1234567890123456789;

  StartBenchmarkTiming();

  for (size_t i = 0; i < num_iterations; ++i) {
    packet->length(0);
    protocol->store_longlong(value, false, 0);
  }

  StopBenchmarkTiming();
  initializer.TearDown();
}
BENCHMARK(BM_Protocol_text_store_longlong)

static void BM_Protocol_text_store_date(size_t num_iterations) {
  StopBenchmarkTiming();

  my_testing::Server_initializer initializer;
  initializer.SetUp();
  Protocol_text *const protocol = initializer.thd()->protocol_text.get();
  SetupProtocolForBenchmark(protocol);
  String *const packet = protocol->get_output_packet();

  const MysqlTime date(2020, 2, 29, 0, 0, 0, 0, false, MYSQL_TIMESTAMP_DATE);

  StartBenchmarkTiming();

  for (size_t i = 0; i < num_iterations; ++i) {
    packet->length(0);
    protocol->store_date(date);
  }

  StopBenchmarkTiming();
  initializer.TearDown();
}
BENCHMARK(BM_Protocol_text_store_date)

static void BM_Protocol_text_store_time(size_t num_iterations) {
  StopBenchmarkTiming();

  my_testing::Server_initializer initializer;
  initializer.SetUp();
  Protocol_text *const protocol = initializer.thd()->protocol_text.get();
  SetupProtocolForBenchmark(protocol);
  String *const packet = protocol->get_output_packet();

  const MysqlTime time(0, 0, 0, 123, 59, 59, 670000, false,
                       MYSQL_TIMESTAMP_TIME);

  StartBenchmarkTiming();

  for (size_t i = 0; i < num_iterations; ++i) {
    packet->length(0);
    protocol->store_time(time, 6);
  }

  StopBenchmarkTiming();
  initializer.TearDown();
}
BENCHMARK(BM_Protocol_text_store_time)

static void BM_Protocol_text_store_datetime(size_t num_iterations) {
  StopBenchmarkTiming();

  my_testing::Server_initializer initializer;
  initializer.SetUp();
  Protocol_text *const protocol = initializer.thd()->protocol_text.get();
  SetupProtocolForBenchmark(protocol);
  String *const packet = protocol->get_output_packet();

  const MysqlTime datetime(2020, 2, 29, 23, 59, 59, 670000, false,
                           MYSQL_TIMESTAMP_DATETIME);

  StartBenchmarkTiming();

  for (size_t i = 0; i < num_iterations; ++i) {
    packet->length(0);
    protocol->store_datetime(datetime, 6);
  }

  StopBenchmarkTiming();
  initializer.TearDown();
}
BENCHMARK(BM_Protocol_text_store_datetime)

}  // namespace protocol_classic_unittest
