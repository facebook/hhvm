/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/item_cmpfunc.h"
#include "unittest/gunit/test_utils.h"

namespace item_func_case_unittest {

using my_testing::Server_initializer;

class ItemFuncCaseTest : public ::testing::Test {
 protected:
  virtual void SetUp() { initializer.SetUp(); }

  virtual void TearDown() { initializer.TearDown(); }

  THD *thd() { return initializer.thd(); }

  Server_initializer initializer;
};

/*
  Bug#21381060
  A "CASE WHEN" EXPRESSION WITH NULL AND AN UNSIGNED TYPE GIVES A SIGNED RESULT

  Original test case:
    create table MyTable (`n` tinyint unsigned not null);
    insert into MyTable (n) values (180);
    select (case when 1 then n else null end) as value from MyTable;

  The returned value was signed, rather than unsigned.

  This unit test verifies that the bug is fixed in 5.7 and up.
*/
TEST_F(ItemFuncCaseTest, CaseWhenElseNull) {
  Item_int *int_one = new Item_int(1);
  Item_int *int_n = new Item_int(180ULL);
  List<Item> list;
  list.push_back(int_one);
  list.push_back(int_n);
  Item_func_case *item_case =
      new Item_func_case(POS(), list, nullptr, new Item_null());
  EXPECT_FALSE(item_case->fix_fields(thd(), nullptr));

  EXPECT_FALSE(int_one->unsigned_flag);
  EXPECT_TRUE(int_n->unsigned_flag);
  EXPECT_EQ(180, item_case->val_int());
  // Result should be unsigned.
  EXPECT_TRUE(item_case->unsigned_flag);
}

}  // namespace item_func_case_unittest
