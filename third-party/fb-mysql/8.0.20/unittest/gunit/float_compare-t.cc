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
#include <climits>

#include "sql/float_compare.h"

namespace float_compare_unittest {

TEST(FloatCompare, AlmostEqualDouble) {
  double val1 = 0.0;
  double val2 = 1.0;
  EXPECT_FALSE(Float_compare::almost_equal(val1, val2));

  val1 = -0.0;
  val2 = 0.0;
  EXPECT_TRUE(Float_compare::almost_equal(val1, val2));

  val1 = 0.0;
  val2 = 0.0;
  EXPECT_TRUE(Float_compare::almost_equal(val1, val2));

  val1 = std::numeric_limits<double>::max();
  val2 = std::numeric_limits<double>::infinity();
  EXPECT_FALSE(Float_compare::almost_equal(val1, val2));

  val1 = 1.0;
  val2 = 1.0 + std::numeric_limits<double>::epsilon();
  EXPECT_TRUE(Float_compare::almost_equal(val1, val2));

  val1 = 99999999.0000001;
  val2 = 99999999.00000013;
  EXPECT_TRUE(Float_compare::almost_equal(val1, val2));

  val1 = std::numeric_limits<double>::infinity();
  val2 = -std::numeric_limits<double>::infinity();
  EXPECT_FALSE(Float_compare::almost_equal(val1, val2));

  val1 = std::numeric_limits<double>::infinity();
  val2 = std::numeric_limits<double>::infinity();
  EXPECT_TRUE(Float_compare::almost_equal(val1, val2));

  val1 = 0.0 - std::numeric_limits<double>::epsilon();
  val2 = 0.0 + std::numeric_limits<double>::epsilon();
  EXPECT_FALSE(Float_compare::almost_equal(val1, val2));

  val1 = std::numeric_limits<double>::quiet_NaN();
  val2 = std::numeric_limits<double>::quiet_NaN();
  EXPECT_FALSE(Float_compare::almost_equal(val1, val2));

  val1 = std::numeric_limits<double>::quiet_NaN();
  val2 = 0.0;
  EXPECT_FALSE(Float_compare::almost_equal(val1, val2));

  val1 = 1.0e+18;
  val2 = val1 + 100.0;
  EXPECT_TRUE(Float_compare::almost_equal(val1, val2));
  EXPECT_TRUE(Float_compare::almost_equal(val1, val2, 4));
  EXPECT_FALSE(Float_compare::almost_equal(val1, val2, 0));

  val1 = 0.1 + 0.1 + 0.1 + 0.1 + 0.1 + 0.1 + 0.1 + 0.1 + 0.1 + 0.1;
  val2 = 1.0;
  EXPECT_TRUE(Float_compare::almost_equal(val1, val2));

  val1 = 99999999.0000001;
  val2 = 99999999.00000016;
  EXPECT_FALSE(Float_compare::almost_equal(val1, val2, 4));
  EXPECT_TRUE(Float_compare::almost_equal(val1, val2, 6));
}

TEST(FloatCompare, AlmostEqualFloat) {
  float val1 = 0.0;
  float val2 = 1.0;
  EXPECT_FALSE(Float_compare::almost_equal(val1, val2));

  val1 = -0.0;
  val2 = 0.0;
  EXPECT_TRUE(Float_compare::almost_equal(val1, val2));

  val1 = 0.0;
  val2 = 0.0;
  EXPECT_TRUE(Float_compare::almost_equal(val1, val2));

  val1 = std::numeric_limits<float>::max();
  val2 = std::numeric_limits<float>::infinity();
  EXPECT_FALSE(Float_compare::almost_equal(val1, val2));

  val1 = 1.0;
  val2 = 1.0 + std::numeric_limits<float>::epsilon();
  EXPECT_TRUE(Float_compare::almost_equal(val1, val2));

  val1 = 99999999.0000001;
  val2 = 99999999.00000013;
  EXPECT_TRUE(Float_compare::almost_equal(val1, val2));

  val1 = std::numeric_limits<float>::infinity();
  val2 = -std::numeric_limits<float>::infinity();
  EXPECT_FALSE(Float_compare::almost_equal(val1, val2));

  val1 = std::numeric_limits<float>::infinity();
  val2 = std::numeric_limits<float>::infinity();
  EXPECT_TRUE(Float_compare::almost_equal(val1, val2));

  val1 = 0.0 - std::numeric_limits<float>::epsilon();
  val2 = 0.0 + std::numeric_limits<float>::epsilon();
  EXPECT_FALSE(Float_compare::almost_equal(val1, val2));

  val1 = std::numeric_limits<float>::quiet_NaN();
  val2 = std::numeric_limits<float>::quiet_NaN();
  EXPECT_FALSE(Float_compare::almost_equal(val1, val2));

  val1 = std::numeric_limits<float>::quiet_NaN();
  val2 = 0.0;
  EXPECT_FALSE(Float_compare::almost_equal(val1, val2));

  val1 = 1.0e+18;
  val2 = val1 + 100.0;
  EXPECT_TRUE(Float_compare::almost_equal(val1, val2));
  EXPECT_TRUE(Float_compare::almost_equal(val1, val2, 4));
  EXPECT_FALSE(Float_compare::almost_equal(val1, val2, 0));

  val1 = 0.1 + 0.1 + 0.1 + 0.1 + 0.1 + 0.1 + 0.1 + 0.1 + 0.1 + 0.1;
  val2 = 1.0;
  EXPECT_TRUE(Float_compare::almost_equal(val1, val2));

  val1 = 99999970.0;
  val2 = 100000000.0;
  EXPECT_FALSE(Float_compare::almost_equal(val1, val2, 4));
  EXPECT_TRUE(Float_compare::almost_equal(val1, val2, 6));
}

}  // namespace float_compare_unittest
