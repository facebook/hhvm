/* Copyright (c) 2015, 2017, Oracle and/or its affiliates. All rights reserved.

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
#include <stdio.h>

#include "plugin/x/client/mysqlxclient/xdatetime.h"

namespace xpl {

namespace test {

TEST(xdatetime, date) {
  EXPECT_TRUE(xcl::DateTime(2015, 12, 5));
  EXPECT_EQ("2015-12-05", xcl::DateTime(2015, 12, 5).to_string());
  EXPECT_EQ("2015-12-05", xcl::DateTime(2015, 12, 5).to_string());
  EXPECT_EQ("0001-01-01", xcl::DateTime(1, 1, 1).to_string());
  EXPECT_EQ("0000-00-00", xcl::DateTime(0, 0, 0).to_string());
  EXPECT_EQ("9999-12-31", xcl::DateTime(9999, 12, 31).to_string());
  EXPECT_FALSE(xcl::DateTime(0, 50, 60));
}

TEST(xdatetime, datetime) {
  EXPECT_EQ("2015-12-05 00:00:00.123456",
            xcl::DateTime(2015, 12, 5, 0, 0, 0, 123456).to_string());
  EXPECT_EQ("0001-01-01 23:45:59.99",
            xcl::DateTime(1, 1, 1, 23, 45, 59, 990000).to_string());
  EXPECT_EQ("0000-00-00 00:00:00",
            xcl::DateTime(0, 0, 0, 0, 0, 0, 0).to_string());
  EXPECT_EQ("9999-12-31 23:59:59.999999",
            xcl::DateTime(9999, 12, 31, 23, 59, 59, 999999).to_string());

  EXPECT_EQ("23:59:59.999999",
            xcl::DateTime(9999, 12, 31, 23, 59, 59, 999999).time().to_string());

  EXPECT_FALSE(xcl::DateTime(0, 50, 60, 24, 60, 60, 0));
}

TEST(xdatetime, time) {
  EXPECT_EQ("00:00:00.123456", xcl::Time(false, 0, 0, 0, 123456).to_string());
  EXPECT_EQ("00:00:00.1234", xcl::Time(false, 0, 0, 0, 123400).to_string());
  EXPECT_EQ("23:45:59.99", xcl::Time(false, 23, 45, 59, 990000).to_string());
  EXPECT_EQ("00:00:00", xcl::Time(false, 0, 0, 0).to_string());
  EXPECT_EQ("23:59:59.999999",
            xcl::Time(false, 23, 59, 59, 999999).to_string());
  EXPECT_EQ("00:00:00.33", xcl::Time(false, 0, 0, 0, 330000).to_string());
  EXPECT_EQ("-00:00:00.33", xcl::Time(true, 0, 0, 0, 330000).to_string());
  EXPECT_EQ("-821:00:00.33", xcl::Time(true, 821, 0, 0, 330000).to_string());

  EXPECT_FALSE(xcl::Time(false, 24, 60, 60));
}

TEST(xdatetime, useconds_representation) {
  EXPECT_EQ("00:00:00", xcl::Time(false, 0, 0, 0, 0).to_string());
  EXPECT_EQ("00:00:00.000001", xcl::Time(false, 0, 0, 0, 1).to_string());
  EXPECT_EQ("00:00:00.001234", xcl::Time(false, 0, 0, 0, 1234).to_string());
  EXPECT_EQ("00:00:00.999999",
            xcl::Time(false, 00, 00, 00, 999999).to_string());

  EXPECT_EQ("00:00:00",
            xcl::DateTime(0000, 00, 00, 00, 00, 00, 0).time().to_string());
  EXPECT_EQ("00:00:00.000001",
            xcl::DateTime(0000, 00, 00, 00, 00, 00, 1).time().to_string());
  EXPECT_EQ("00:00:00.001234",
            xcl::DateTime(0000, 00, 00, 00, 00, 00, 1234).time().to_string());
  EXPECT_EQ("00:00:00.999999",
            xcl::DateTime(0000, 00, 00, 00, 00, 00, 999999).time().to_string());
}

}  // namespace test

}  // namespace xpl
