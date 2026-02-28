/*
   Copyright (c) 2006, 2017, Oracle and/or its affiliates. All rights reserved.

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

#include <gtest/gtest.h>

#include "my_inttypes.h"
#include "nullable.h"

namespace nullable_unittest {

using Mysql::Nullable;

class NullableTest : public ::testing::TestWithParam<uint> {
 protected:
  virtual void SetUp() {}
};

TEST(NullableTest, NullConstructor) {
  Nullable<int> nullable_int;
  EXPECT_FALSE(nullable_int.has_value());
}

TEST(NullableTest, ValueConstructor) {
  Nullable<int> nullable_int(42);
  EXPECT_TRUE(nullable_int.has_value());
  EXPECT_EQ(42, nullable_int.value());

  Nullable<int> nullable_int2 = 42;
  EXPECT_TRUE(nullable_int2.has_value());
  EXPECT_EQ(42, nullable_int2.value());
}

TEST(NullableTest, Assignment) {
  Nullable<int> ni(42);
  Nullable<int> ni2 = ni;
  EXPECT_TRUE(ni2.has_value());
  EXPECT_EQ(42, ni2.value());

  Nullable<int> mynull;
  Nullable<int> mynull2 = mynull;
  EXPECT_FALSE(mynull2.has_value());
}

TEST(NullableTest, Equals) {
  Nullable<int> ni1(42), ni2(123);
  Nullable<int> nn1, nn2;
  EXPECT_NE(ni1, ni2);
  EXPECT_EQ(nn1, nn2);
  EXPECT_EQ(Nullable<int>(), nn2);
}

}  // namespace nullable_unittest
