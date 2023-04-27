/* Copyright (c) 2013, 2019, Oracle and/or its affiliates. All rights reserved.

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

// First include (the generated) my_config.h, to get correct platform defines.
#include "my_config.h"

#include <gtest/gtest.h>
#include <limits.h>

/*

  ==== Purpose ====

  Test if my_strtoll10 overflows values above unsigned long long
  limit correctly.

  ==== Related Bugs and Worklogs ====

  BUG#16997513: MY_STRTOLL10 ACCEPTING OVERFLOWED UNSIGNED LONG LONG
                VALUES AS NORMAL ONES

  ==== Implementation ====

  Check if my_strtoll10 returns the larger unsigned long long and raise
  the overflow error when receiving a number like 18446744073709551915

*/
#include "m_string.h"
#include "my_sys.h"

TEST(StringToULLTest, OverflowedNumber) {
  unsigned long long number;
  int error;
  const char *str = "18446744073709551915";
  number = my_strtoll10(str, nullptr, &error);
  EXPECT_EQ(number, ULLONG_MAX);
  EXPECT_EQ(error, MY_ERRNO_ERANGE);
}
