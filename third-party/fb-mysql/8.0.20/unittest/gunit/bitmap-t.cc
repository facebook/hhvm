/* Copyright (c) 2013, 2018, Oracle and/or its affiliates. All rights reserved.

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
#include <algorithm>

#include "my_inttypes.h"
#include "my_sys.h"
#include "sql/sql_bitmap.h"

namespace bitmap_unittest {

const int BITMAP_SIZE = 128;

class BitmapTest : public ::testing::Test {
 protected:
  BitmapTest() {}

  virtual void SetUp() { bitmap.init(); }

  Bitmap<BITMAP_SIZE> bitmap;
};

TEST_F(BitmapTest, IntersectTest) {
  bitmap.set_prefix(4);
  bitmap.intersect(0xBBBBULL);
  EXPECT_TRUE(bitmap.is_set(0));
  EXPECT_TRUE(bitmap.is_set(1));
  EXPECT_FALSE(bitmap.is_set(2));
  EXPECT_TRUE(bitmap.is_set(3));
  bitmap.clear_bit(0);
  bitmap.clear_bit(1);
  bitmap.clear_bit(3);
  EXPECT_TRUE(bitmap.is_clear_all());
}

TEST_F(BitmapTest, ULLTest) {
  bitmap.set_all();
  bitmap.intersect(0x0123456789ABCDEFULL);
  ulonglong ull = bitmap.to_ulonglong();
  EXPECT_TRUE(ull == 0x0123456789ABCDEFULL);

  Bitmap<24> bitmap24;
  bitmap24.init();
  bitmap24.set_all();
  bitmap24.intersect(0x47BULL);
  ulonglong ull24 = bitmap24.to_ulonglong();
  EXPECT_TRUE(ull24 == 0x47BULL);
}

}  // namespace bitmap_unittest
