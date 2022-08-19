/* Copyright (c) 2016, 2017, Oracle and/or its affiliates. All rights reserved.

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

#include "my_config.h"

#include <gtest/gtest.h>
#include <limits.h>

#include "sql/val_int_compare.h"

namespace val_int_compare_unittest {

TEST(ValIntCompare, Equality) {
  EXPECT_TRUE(Integer_value(UINT_MAX, false) == Integer_value(UINT_MAX, true));
  EXPECT_TRUE(Integer_value(0, false) == Integer_value(0, true));
  EXPECT_TRUE(Integer_value(1, false) == Integer_value(1, true));
  EXPECT_FALSE(Integer_value(ULLONG_MAX, false) ==
               Integer_value(ULLONG_MAX, true));
  EXPECT_FALSE(Integer_value(-1, false) ==
               Integer_value(0xffffffffffffffff, true));
}

TEST(ValIntCompare, LessThan) {
  EXPECT_TRUE(Integer_value(INT_MIN, false) < Integer_value(INT_MAX, false));
  EXPECT_TRUE(Integer_value(INT_MIN, false) < Integer_value(INT_MAX, true));
  EXPECT_FALSE(Integer_value(INT_MAX, false) < Integer_value(INT_MAX, true));
  EXPECT_TRUE(Integer_value(LLONG_MAX, true) < Integer_value(ULLONG_MAX, true));
  EXPECT_FALSE(Integer_value(ULLONG_MAX, true) <
               Integer_value(LLONG_MAX, true));
  EXPECT_TRUE(Integer_value(-1, false) < Integer_value(0, false));
  EXPECT_TRUE(Integer_value(-1, false) < Integer_value(0, true));
  EXPECT_TRUE(Integer_value(-1, false) < Integer_value(1, true));
  EXPECT_TRUE(Integer_value(0, true) < Integer_value(1, false));
  EXPECT_TRUE(Integer_value(0, true) < Integer_value(1, true));
  EXPECT_TRUE(Integer_value(0, false) < Integer_value(1, false));
  EXPECT_TRUE(Integer_value(0, false) < Integer_value(1, true));
}

TEST(ValIntCompare, LessThanOrEqual) {
  EXPECT_TRUE(Integer_value(INT_MIN, false) <= Integer_value(INT_MAX, false));
  EXPECT_TRUE(Integer_value(INT_MIN, false) <= Integer_value(INT_MAX, true));
  EXPECT_TRUE(Integer_value(INT_MAX, false) <= Integer_value(INT_MAX, true));
}

}  // namespace val_int_compare_unittest
