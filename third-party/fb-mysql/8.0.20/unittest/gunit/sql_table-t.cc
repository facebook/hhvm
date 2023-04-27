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

#include "sql/item.h"
#include "sql/rpl_handler.h"  // delegates_init()
#include "sql/sql_class.h"
#include "sql/sql_table.h"
#include "unittest/gunit/mock_create_field.h"
#include "unittest/gunit/mock_field_timestamp.h"
#include "unittest/gunit/test_utils.h"

namespace sql_table_unittest {

using my_testing::Mock_error_handler;
using my_testing::Server_initializer;

/*
  Test of functionality in the file sql_table.cc
 */
class SqlTableTest : public ::testing::Test {
 protected:
  virtual void SetUp() { initializer.SetUp(); }
  virtual void TearDown() { initializer.TearDown(); }

  THD *get_thd() { return initializer.thd(); }

  Server_initializer initializer;
};

/*
  Test of promote_first_timestamp_column(). We pass it a list of two TIMESTAMP
  NOT NULL columns, the first of which should be promoted to DEFAULT
  CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP. The second column should not
  be promoted.
 */
TEST_F(SqlTableTest, PromoteFirstTimestampColumn1) {
  Mock_create_field column_1_definition(MYSQL_TYPE_TIMESTAMP, nullptr, nullptr);
  Mock_create_field column_2_definition(MYSQL_TYPE_TIMESTAMP, nullptr, nullptr);
  column_1_definition.flags |= NOT_NULL_FLAG;
  column_2_definition.flags |= NOT_NULL_FLAG;
  List<Create_field> definitions;
  definitions.push_front(&column_1_definition);
  definitions.push_back(&column_2_definition);
  promote_first_timestamp_column(&definitions);
  EXPECT_EQ((Field::DEFAULT_NOW | Field::ON_UPDATE_NOW),
            column_1_definition.auto_flags);
  EXPECT_EQ(Field::NONE, column_2_definition.auto_flags);
}

/*
  Test of promote_first_timestamp_column(). We pass it a list of two TIMESTAMP
  NOT NULL columns, the first of which should be promoted to DEFAULT
  CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP. The second column should not
  be promoted.
 */
TEST_F(SqlTableTest, PromoteFirstTimestampColumn2) {
  Mock_create_field column_1_definition(MYSQL_TYPE_TIMESTAMP2, nullptr,
                                        nullptr);
  Mock_create_field column_2_definition(MYSQL_TYPE_TIMESTAMP2, nullptr,
                                        nullptr);
  column_1_definition.flags |= NOT_NULL_FLAG;
  column_2_definition.flags |= NOT_NULL_FLAG;
  List<Create_field> definitions;
  definitions.push_front(&column_1_definition);
  definitions.push_back(&column_2_definition);
  promote_first_timestamp_column(&definitions);
  EXPECT_EQ((Field::DEFAULT_NOW | Field::ON_UPDATE_NOW),
            column_1_definition.auto_flags);
  EXPECT_EQ(Field::NONE, column_2_definition.auto_flags);
}

/*
  Test of promote_first_timestamp_column(). We pass it a list of two columns,
  one TIMESTAMP NULL DEFAULT 1, and one TIMESTAMP NOT NULL. No promotion
  should take place.
 */
TEST_F(SqlTableTest, PromoteFirstTimestampColumn3) {
  Item_string *item_str = new Item_string("1", 1, &my_charset_latin1);
  Mock_create_field column_1_definition(MYSQL_TYPE_TIMESTAMP, item_str,
                                        nullptr);
  Mock_create_field column_2_definition(MYSQL_TYPE_TIMESTAMP, nullptr, nullptr);
  column_2_definition.flags |= NOT_NULL_FLAG;
  List<Create_field> definitions;
  definitions.push_front(&column_1_definition);
  definitions.push_back(&column_2_definition);
  promote_first_timestamp_column(&definitions);
  EXPECT_EQ(Field::NONE, column_1_definition.auto_flags);
  EXPECT_EQ(Field::NONE, column_2_definition.auto_flags);
}

/*
  This is a test case based on innobase_init()
  There was an out-of-bounds read when converting "-@" to a table name.
 */
TEST_F(SqlTableTest, FileNameToTableName) {
  struct PackStuff {
    char foo1;
    char str[3];
    char foo2;
  };
  PackStuff foo;
  memcpy(foo.str, "-@", 3);
  MEM_NOACCESS(&foo.foo1, 1);
  MEM_NOACCESS(&foo.foo2, 1);

  const char test_filename[] = "-@";
  char test_tablename[sizeof test_filename - 1];

  // This one used to fail with AddressSanitizer
  size_t name_length;
  name_length = filename_to_tablename(test_filename, test_tablename,
                                      sizeof(test_tablename)
#ifndef DBUG_OFF
                                          ,
                                      true
#endif
  );
  EXPECT_EQ((sizeof(test_tablename)) - 1, name_length);

  // This one used to fail if compiled with -DHAVE_VALGRIND
  name_length =
      filename_to_tablename(foo.str, test_tablename, sizeof(test_tablename)
#ifndef DBUG_OFF
                                                         ,
                            true
#endif
      );
  EXPECT_EQ((sizeof(test_tablename)) - 1, name_length);
}

}  // namespace sql_table_unittest
