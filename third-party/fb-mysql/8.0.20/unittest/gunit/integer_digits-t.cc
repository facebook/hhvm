/*
   Copyright (c) 2019, 2020, Oracle and/or its affiliates. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "integer_digits.h"

#include <gtest/gtest.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <iterator>
#include <limits>
#include <tuple>

namespace integer_digits_unittest {

TEST(IntegerDigits, CountDigits) {
  EXPECT_EQ(1, count_digits(0U));
  EXPECT_EQ(1, count_digits(1U));
  EXPECT_EQ(3, count_digits(std::numeric_limits<uint8_t>::max()));
  EXPECT_EQ(5, count_digits(std::numeric_limits<uint16_t>::max()));
  EXPECT_EQ(10, count_digits(std::numeric_limits<uint32_t>::max()));
  EXPECT_EQ(20, count_digits(std::numeric_limits<uint64_t>::max()));

  uint64_t val = 1;
  for (int i = 1; i <= std::numeric_limits<uint64_t>::digits10; ++i) {
    val *= 10;
    EXPECT_EQ(i, count_digits(val - 1));
    EXPECT_EQ(i + 1, count_digits(val));
  }
}

TEST(IntegerDigits, WriteTwoDigits) {
  for (int i = 0; i < 100; ++i) {
    char buffer[2];
    EXPECT_EQ(std::end(buffer), write_two_digits(i, buffer));
    EXPECT_EQ('0' + i / 10, buffer[0]);
    EXPECT_EQ('0' + i % 10, buffer[1]);
  }
}

class IntegerDigits : public testing::TestWithParam<std::tuple<uint64_t, int>> {
};

TEST_P(IntegerDigits, WriteDigits) {
  const uint64_t value = std::get<0>(GetParam());
  const int digits = std::get<1>(GetParam());

  ASSERT_GT(digits, 0);

  // Without this, GCC 10 warns in release builds that write_digits() might
  // write outside the buffer, because it doesn't see that digits is always
  // positive.
  if (digits <= 0) return;

  char buffer[100];
  char *end = write_digits(value, digits, buffer);
  *end = '\0';  // write_digits does not zero-terminate

  char expected[100];
  int expected_length =
      snprintf(expected, sizeof(expected), "%0*" PRIu64, digits, value);

  EXPECT_EQ(buffer + expected_length, end);
  EXPECT_STREQ(expected, buffer);
}

// testing::Combine() is not available on Solaris.
#ifdef GTEST_HAS_COMBINE

// Test write_digits() with all input values from 0 to 1024, with a sufficiently
// large number of digits to hold all (4).
INSTANTIATE_TEST_CASE_P(SmallValues, IntegerDigits,
                        testing::Combine(testing::Range(uint64_t{0},
                                                        uint64_t{1024}),
                                         testing::Values(4)));

// Test write_digits() with the largest three values, with and without
// zero-padding.
INSTANTIATE_TEST_CASE_P(
    LargeValues, IntegerDigits,
    testing::Combine(testing::Values(std::numeric_limits<uint64_t>::max() - 2,
                                     std::numeric_limits<uint64_t>::max() - 1,
                                     std::numeric_limits<uint64_t>::max()),
                     testing::Values(20, 21, 22)));

// Test write_digits() with all single-digit numbers, with and without
// zero-padding.
INSTANTIATE_TEST_CASE_P(SingleDigits, IntegerDigits,
                        testing::Combine(testing::Range(uint64_t{0},
                                                        uint64_t{10}),
                                         testing::Values(1, 2, 3)));

#endif  // GTEST_HAS_COMBINE

}  // namespace integer_digits_unittest
