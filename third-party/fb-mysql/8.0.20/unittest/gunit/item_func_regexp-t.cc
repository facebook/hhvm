/* Copyright (c) 2011, 2018, Oracle and/or its affiliates. All rights reserved.

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

#include "my_config.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "unittest/gunit/mock_parse_tree.h"
#include "unittest/gunit/test_utils.h"

#include "sql/item_regexp_func.h"
#include "sql/parse_tree_items.h"

namespace item_func_regexp_unittest {

using my_testing::Mock_pt_item_list;
using my_testing::Server_initializer;

class ItemFuncRegexpTest : public ::testing::Test {
 protected:
  virtual void SetUp() { initializer.SetUp(); }
  virtual void TearDown() { initializer.TearDown(); }

  THD *thd() { return initializer.thd(); }

  Server_initializer initializer;

  template <typename Item_type>
  void test_print(const char *arg1, const char *arg2, const char *expected) {
    auto args = new (thd()->mem_root) Mock_pt_item_list{arg1, arg2};
    Item *item = new Item_type(POS(), args);
    Parse_context pc(thd(), thd()->lex->select_lex);
    item->itemize(&pc, &item);
    item->fix_fields(thd(), nullptr);
    String buf;
    item->print(thd(), &buf, QT_ORDINARY);
    EXPECT_STREQ(expected, buf.c_ptr_safe());
  }
};

TEST_F(ItemFuncRegexpTest, Print) {
  test_print<Item_func_regexp_instr>("abc", "def", "regexp_instr('abc','def')");
  test_print<Item_func_regexp_like>("abc", "def", "regexp_like('abc','def')");
  test_print<Item_func_regexp_replace>("ab", "c", "regexp_replace('ab','c')");
  test_print<Item_func_regexp_substr>("x", "y", "regexp_substr('x','y')");
}

}  // namespace item_func_regexp_unittest
