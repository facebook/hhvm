/* Copyright (c) 2011, 2019, Oracle and/or its affiliates. All rights reserved.

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

// First include (the generated) my_config.h, to get correct platform defines,
// then gtest.h (before any other MySQL headers), to avoid min() macros etc ...
#include "my_config.h"

#include <gtest/gtest.h>
#include <stddef.h>

#include "sql/item_timefunc.h"  // Item_func_now_local
#include "unittest/gunit/mock_create_field.h"
#include "unittest/gunit/test_utils.h"

namespace create_field_unittest {

using my_testing::Mock_error_handler;
using my_testing::Server_initializer;

class CreateFieldTest : public ::testing::Test {
 protected:
  virtual void SetUp() { initializer.SetUp(); }
  virtual void TearDown() { initializer.TearDown(); }

  Server_initializer initializer;
};

TEST_F(CreateFieldTest, init) {
  // To do: Add all possible precisions.
  Item_func_now_local *now = new Item_func_now_local(0);

  Mock_create_field field_definition_none(MYSQL_TYPE_TIMESTAMP, nullptr,
                                          nullptr);
  EXPECT_EQ(Field::NONE, field_definition_none.auto_flags);

  Mock_create_field field_definition_dn(MYSQL_TYPE_TIMESTAMP, now, nullptr);
  EXPECT_EQ(Field::DEFAULT_NOW, field_definition_dn.auto_flags);

  Mock_create_field field_definition_dnun(MYSQL_TYPE_TIMESTAMP, now, now);
  EXPECT_EQ((Field::DEFAULT_NOW | Field::ON_UPDATE_NOW),
            field_definition_dnun.auto_flags);

  Mock_create_field field_definition_un(MYSQL_TYPE_TIMESTAMP, nullptr, now);
  EXPECT_EQ(Field::ON_UPDATE_NOW, field_definition_un.auto_flags);
}

}  // namespace create_field_unittest
