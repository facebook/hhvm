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

#include <gtest/gtest.h>
#include <stddef.h>
#include <stdint.h>
#include <limits>

#include "m_string.h"
#include "unittest/gunit/benchmark.h"

namespace m_string_unittest {

TEST(MString, HumanReadableSize) {
  char data_size_str[32];
  double data_size = 1.0;
  human_readable_num_bytes(data_size_str, 32, data_size);
  EXPECT_STREQ("1", data_size_str);

  data_size = 1024.0;
  human_readable_num_bytes(data_size_str, 32, data_size);
  EXPECT_STREQ("1024", data_size_str);

  data_size = 1024.1;
  human_readable_num_bytes(data_size_str, 32, data_size);
  EXPECT_STREQ("1K", data_size_str);

  data_size = 1025.0;
  human_readable_num_bytes(data_size_str, 32, data_size);
  EXPECT_STREQ("1K", data_size_str);

  data_size *= 1024;
  human_readable_num_bytes(data_size_str, 32, data_size);
  EXPECT_STREQ("1M", data_size_str);

  data_size *= 1024;
  human_readable_num_bytes(data_size_str, 32, data_size);
  EXPECT_STREQ("1G", data_size_str);

  data_size *= 1024;
  human_readable_num_bytes(data_size_str, 32, data_size);
  EXPECT_STREQ("1T", data_size_str);

  data_size *= 1024;
  human_readable_num_bytes(data_size_str, 32, data_size);
  EXPECT_STREQ("1P", data_size_str);

  data_size *= 1024;
  human_readable_num_bytes(data_size_str, 32, data_size);
  EXPECT_STREQ("1E", data_size_str);

  data_size *= 1024;
  human_readable_num_bytes(data_size_str, 32, data_size);
  EXPECT_STREQ("1Z", data_size_str);

  data_size *= 1024;
  human_readable_num_bytes(data_size_str, 32, data_size);
  EXPECT_STREQ("1Y", data_size_str);

  data_size *= 1024;
  human_readable_num_bytes(data_size_str, 32, data_size);
  EXPECT_STREQ("1025Y", data_size_str);

  data_size *= 1000;
  human_readable_num_bytes(data_size_str, 32, data_size);
  EXPECT_STREQ("1025000Y", data_size_str);

  data_size *= 1000;
  human_readable_num_bytes(data_size_str, 32, data_size);
  EXPECT_STREQ("1025000000Y", data_size_str);

  data_size *=
      static_cast<double>(std::numeric_limits<unsigned long long>::max());
  human_readable_num_bytes(data_size_str, 32, data_size);
  EXPECT_STREQ("+INF", data_size_str);
}

static void BM_longlong10_to_str(size_t num_iterations) {
  StopBenchmarkTiming();
  const int64_t value = 1234567890123456789;

  StartBenchmarkTiming();
  for (size_t i = 0; i < num_iterations; ++i) {
    char buffer[std::numeric_limits<int64_t>::digits10 + 2];
    longlong10_to_str(value, buffer, -10);
  }
}
BENCHMARK(BM_longlong10_to_str)

static void BM_longlong2str(size_t num_iterations) {
  StopBenchmarkTiming();
  const int64_t value = 1234567890123456789;

  StartBenchmarkTiming();
  for (size_t i = 0; i < num_iterations; ++i) {
    char buffer[100];
    longlong2str(value, buffer, -36);
  }
}
BENCHMARK(BM_longlong2str)

}  // namespace m_string_unittest
