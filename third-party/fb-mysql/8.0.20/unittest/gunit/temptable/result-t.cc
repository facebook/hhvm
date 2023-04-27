/* Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.

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
#include <array>
#include <cstring>
#include <memory>
#include <vector>

#include "storage/temptable/include/temptable/result.h"

namespace temptable_test {

void check_result(temptable::Result r) {
  const char *result_str = temptable::result_to_string(r);

  EXPECT_TRUE(result_str != nullptr);
  EXPECT_GT(strlen(result_str), 0);
  EXPECT_NE(strcmp(result_str, "UNKNOWN"), 0);
}

TEST(Result, ToString) {
  using namespace temptable;

  check_result(Result::END_OF_FILE);
  check_result(Result::FOUND_DUPP_KEY);
  check_result(Result::KEY_NOT_FOUND);
  check_result(Result::NO_SUCH_TABLE);

  EXPECT_EQ(std::string(result_to_string(Result::OK)), std::string("OK"));

  check_result(Result::OUT_OF_MEM);
  check_result(Result::RECORD_FILE_FULL);
  check_result(Result::TABLE_CORRUPT);
  check_result(Result::TABLE_EXIST);
  check_result(Result::UNSUPPORTED);
  check_result(Result::WRONG_COMMAND);
  check_result(Result::WRONG_INDEX);
}

}  // namespace temptable_test
