/* Copyright (c) 2014, Oracle and/or its affiliates. All rights reserved.

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
#include <item.h>
#include "my_config.h"

#include "test_utils.h"

namespace item_param_unittest {
using my_testing::Server_initializer;

class ItemParamTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    m_initializer.SetUp();
    // An Item expects to be owned by current_thd->free_list, so allocate with
    // new, and do not delete it.
    m_item_param = new Item_param(POS(), 1);
  }

  virtual void TearDown() { m_initializer.TearDown(); }

  Server_initializer m_initializer;
  Item_param *m_item_param;
};

TEST_F(ItemParamTest, convert_str_value) {
  m_item_param->state = Item_param::LONG_DATA_VALUE;
  m_item_param->value.cs_info.final_character_set_of_str_value = NULL;
  EXPECT_TRUE(m_item_param->convert_str_value(m_initializer.thd()));
}
}  // namespace item_param_unittest
