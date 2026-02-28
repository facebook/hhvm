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

#include "storage/temptable/include/temptable/cell.h"

namespace temptable_test {

TEST(Cell, ConstructorAndGetters) {
  std::array<unsigned char, 8> data{{8, 4, 6, 2, 0, 0, 5, 7}};

  const unsigned char *const valid_data = data.data();
  const uint32_t valid_data_length = static_cast<uint32_t>(data.size());

  const unsigned char *const null_data = nullptr;

  {
    temptable::Cell cell(false, valid_data_length, valid_data);
    EXPECT_EQ(cell.is_null(), false);
    EXPECT_EQ(cell.data_length(), valid_data_length);
    EXPECT_EQ(cell.data(), valid_data);
  }

  {
    temptable::Cell cell(true, valid_data_length - 1, valid_data + 1);
    EXPECT_EQ(cell.is_null(), true);
    EXPECT_EQ(cell.data_length(), valid_data_length - 1);
    EXPECT_EQ(cell.data(), valid_data + 1);
  }

  {
    temptable::Cell cell(true, 0, null_data);
    EXPECT_EQ(cell.is_null(), true);
    EXPECT_EQ(cell.data_length(), 0);
    EXPECT_EQ(cell.data(), null_data);
  }
}

}  // namespace temptable_test
