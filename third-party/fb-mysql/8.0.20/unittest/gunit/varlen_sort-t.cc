/* Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.

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
#include <stdlib.h>
#include <algorithm>

#include "my_byteorder.h"
#include "my_inttypes.h"
#include "varlen_sort.h"

TEST(VarlenSortTest, StdSort) {
  // Small integers sort as strings no matter the endianness.
  int data[8] = {1, 8, 2, 5, 3, 7, 6, 4};
  uchar *data_ptr = reinterpret_cast<uchar *>(data);
  varlen_sort(data_ptr, data_ptr + sizeof(data), sizeof(int),
              [](const uchar *a, const uchar *b) {
                return memcmp(a, b, sizeof(int)) < 0;
              });

  for (int i = 0; i < 8; ++i) {
    EXPECT_EQ(i + 1, data[i]);
  }
}

TEST(VarlenSortTest, LargeThreeByteSort) {
  srand(12345);
  std::unique_ptr<uchar[]> data(new uchar[1024 * 3]);
  for (int i = 0; i < 1024; ++i) {
    int3store(&data[i * 3], rand() & 0xffffff);
  }
  varlen_sort(data.get(), data.get() + 1024 * 3, 3,
              [](const uchar *a, const uchar *b) {
                return uint3korr(a) < uint3korr(b);
              });

  for (int i = 0; i < 1023; ++i) {
    EXPECT_LE(uint3korr(&data[i * 3]), uint3korr(&data[i * 3 + 3]));
  }
}
