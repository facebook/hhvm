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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stddef.h>

#include "my_inttypes.h"
#include "sql/sql_data_change.h"
#include "unittest/gunit/fake_table.h"
#include "unittest/gunit/mock_field_timestamp.h"
#include "unittest/gunit/test_utils.h"

namespace copy_info_unittest {

using my_testing::Mock_error_handler;
using my_testing::Server_initializer;

using ::testing::_;
using ::testing::StrictMock;

/*
  Tests for the functionality of the COPY_INFO class. We test all public
  interfaces, and some of the protected parts:

  - COPY_INFO::get_function_default_columns, and
  - COPY_INFO::get_cached_bitmap
*/
class CopyInfoTest : public ::testing::Test {
 protected:
  virtual void SetUp() { initializer.SetUp(); }
  virtual void TearDown() { initializer.TearDown(); }

  Server_initializer initializer;
};

/**
  This is a simple mock Field class, which verifies that store_timestamp is
  called. We inherit Field_long, but the data type does not matter.
*/
class Mock_field : public Field_long {
 public:
  explicit Mock_field(uchar auto_flags_arg)
      : Field_long(nullptr, 0, nullptr, 0, auto_flags_arg, "", false, false) {}

  MOCK_METHOD1(store_timestamp, void(const timeval *));
};

/*
  Convenience class for creating a COPY_INFO to represent an insert operation.
*/
class Mock_COPY_INFO : public COPY_INFO {
 public:
  Mock_COPY_INFO(operation_type optype, List<Item> *inserted_columns,
                 enum_duplicates duplicate_handling)
      : COPY_INFO(optype, inserted_columns,
                  true,  // manage_defaults
                  duplicate_handling) {}

  // Import protected member functions, so we can test them.
  using COPY_INFO::get_cached_bitmap;
  using COPY_INFO::get_function_default_columns;
};

/*
  Convenience class for creating a COPY_INFO to represent an insert operation.
*/
class Mock_COPY_INFO_insert : public COPY_INFO {
 public:
  Mock_COPY_INFO_insert()
      : COPY_INFO(COPY_INFO::INSERT_OPERATION,
                  static_cast<List<Item> *>(nullptr),
                  true,  // manage_defaults
                  DUP_UPDATE) {}
  Mock_COPY_INFO_insert(List<Item> *fields)
      : COPY_INFO(COPY_INFO::INSERT_OPERATION, fields,
                  true,  // manage_defaults
                  DUP_UPDATE) {}
  // Import protected member functions, so we can test them.
  using COPY_INFO::get_cached_bitmap;
  using COPY_INFO::get_function_default_columns;
};

/*
  Convenience class for creating a COPY_INFO to represent an update
  operation.
*/
class Mock_COPY_INFO_update : public COPY_INFO {
 public:
  Mock_COPY_INFO_update()
      : COPY_INFO(COPY_INFO::UPDATE_OPERATION, nullptr, nullptr) {}
  // Import protected member functions, so we can test them.
  using COPY_INFO::get_cached_bitmap;
  using COPY_INFO::get_function_default_columns;
};

/*
  Tests that constuctors initialize the stats object properly.
*/
TEST_F(CopyInfoTest, constructors) {
  List<Item> inserted_columns;

  COPY_INFO insert(COPY_INFO::INSERT_OPERATION, &inserted_columns,
                   true,  // manage_defaults
                   DUP_UPDATE);

  EXPECT_EQ(0U, insert.stats.records);
  EXPECT_EQ(0U, insert.stats.deleted);
  EXPECT_EQ(0U, insert.stats.updated);
  EXPECT_EQ(0U, insert.stats.copied);
  EXPECT_EQ(0U, insert.stats.error_count);
  EXPECT_EQ(0U, insert.stats.touched);

  List<Item> columns;
  List<Item> values;
  COPY_INFO update(COPY_INFO::UPDATE_OPERATION, &columns, &values);

  EXPECT_EQ(0U, update.stats.records);
  EXPECT_EQ(0U, update.stats.deleted);
  EXPECT_EQ(0U, update.stats.updated);
  EXPECT_EQ(0U, update.stats.copied);
  EXPECT_EQ(0U, update.stats.error_count);
  EXPECT_EQ(0U, update.stats.touched);
}

/*
  Tests the accessors when the COPY_INFO represents an insert operation.
*/
TEST_F(CopyInfoTest, insertAccessors) {
  List<Item> inserted_columns;

  COPY_INFO insert(COPY_INFO::INSERT_OPERATION, &inserted_columns,
                   true,  // manage_defaults
                   DUP_REPLACE);

  EXPECT_EQ(COPY_INFO::INSERT_OPERATION, insert.get_operation_type());
  EXPECT_EQ(&inserted_columns, insert.get_changed_columns());
  EXPECT_EQ(static_cast<List<Item> *>(nullptr), insert.get_changed_columns2());
  EXPECT_TRUE(insert.get_manage_defaults());
  EXPECT_EQ(DUP_REPLACE, insert.get_duplicate_handling());
}

/*
  Tests the accessors when the COPY_INFO represents a load data infile
  operation.
*/
TEST_F(CopyInfoTest, loadDataAccessors) {
  List<Item> inserted_columns;
  List<Item> inserted_columns2;

  COPY_INFO load_data(COPY_INFO::INSERT_OPERATION, &inserted_columns,
                      &inserted_columns2,
                      true,  // manage_defaults
                      DUP_UPDATE, 123);

  EXPECT_EQ(COPY_INFO::INSERT_OPERATION, load_data.get_operation_type());
  EXPECT_EQ(&inserted_columns, load_data.get_changed_columns());
  EXPECT_EQ(&inserted_columns2, load_data.get_changed_columns2());
  EXPECT_TRUE(load_data.get_manage_defaults());
  EXPECT_EQ(DUP_UPDATE, load_data.get_duplicate_handling());
}

/*
  Tests the accessors when the COPY_INFO represents an update operation.
*/
TEST_F(CopyInfoTest, updateAccessors) {
  List<Item> columns;
  List<Item> values;

  COPY_INFO update(COPY_INFO::UPDATE_OPERATION, &columns, &values);

  EXPECT_EQ(COPY_INFO::UPDATE_OPERATION, update.get_operation_type());
  EXPECT_EQ(&columns, update.get_changed_columns());
  EXPECT_EQ(static_cast<List<Item> *>(nullptr), update.get_changed_columns2());
  EXPECT_TRUE(update.get_manage_defaults());
  EXPECT_EQ(DUP_ERROR, update.get_duplicate_handling());
}

static Field_long make_field() {
  Field_long a(nullptr, 0, nullptr, 0, Field::DEFAULT_NOW, "a", false, false);
  return a;
}

/*
  Test of the lazy instantiation performed by get_function_default_columns().

  - The bitmap pointer is initially NULL.

  - That calling get_function_default_columns() indeed points the member to a
    lazily instantiated bitmap.

  - That on a second call to get_function_default_columns(), a new bitmap is
    not allocated.

    We repeat the test for insert and update operations.
*/
TEST_F(CopyInfoTest, getFunctionDefaultColumns) {
  Mock_COPY_INFO_insert insert;
  Mock_COPY_INFO_update update;

  Field_long a = make_field();
  Fake_TABLE table(&a);

  MY_BITMAP *initial_value = nullptr;

  EXPECT_EQ(initial_value, insert.get_cached_bitmap());

  insert.get_function_default_columns(&table);
  EXPECT_NE(initial_value, insert.get_cached_bitmap())
      << "The output parameter must be set!";

  const MY_BITMAP *function_default_columns = insert.get_cached_bitmap();
  insert.get_function_default_columns(&table);
  EXPECT_EQ(function_default_columns, insert.get_cached_bitmap())
      << "Not supposed to allocate a new bitmap on second call.";

  EXPECT_EQ(initial_value, update.get_cached_bitmap());
  update.get_function_default_columns(&table);
  EXPECT_NE(initial_value, update.get_cached_bitmap())
      << "The output parameter must be set!";

  function_default_columns = update.get_cached_bitmap();
  update.get_function_default_columns(&table);
  EXPECT_EQ(function_default_columns, update.get_cached_bitmap())
      << "Not supposed to allocate a new bitmap on second call.";
}

/*
  Here we test that calling COPY_INFO::set_function_defaults() indeed causes
  store_timestamp to be called on the columns that are not on the list of
  assigned_columns. We seize the opportunity to test
  COPY_INFO::function_defaults_apply() since we have to call it anyways in
  order for set_function_defaults() not to assert.
*/
TEST_F(CopyInfoTest, setFunctionDefaults) {
  StrictMock<Mock_field> a(Field::ON_UPDATE_NOW);
  StrictMock<Mock_field> b(Field::DEFAULT_NOW | Field::ON_UPDATE_NOW);
  StrictMock<Mock_field> c(Field::DEFAULT_NOW | Field::ON_UPDATE_NOW);

  EXPECT_TRUE(a.has_update_default_datetime_value_expression());
  EXPECT_TRUE(b.has_update_default_datetime_value_expression());
  EXPECT_TRUE(c.has_update_default_datetime_value_expression());

  Fake_TABLE table(&a, &b, &c);

  List<Item> assigned_columns;
  assigned_columns.push_front(new Item_field(&a));

  Mock_COPY_INFO insert(COPY_INFO::INSERT_OPERATION, &assigned_columns,
                        DUP_ERROR);

  ASSERT_FALSE(insert.get_function_default_columns(&table)) << "Out of memory";

  insert.ignore_last_columns(&table, 1);  // 'c'
  insert.add_function_default_columns(&table, table.write_set);
  EXPECT_FALSE(bitmap_is_set(table.write_set, 0));
  EXPECT_TRUE(bitmap_is_set(table.write_set, 1));
  EXPECT_FALSE(bitmap_is_set(table.write_set, 2));

  EXPECT_TRUE(insert.function_defaults_apply(&table)) << "They do apply";

  // We expect store_timestamp() to be called for b and not for c.
  // We do not care about the argument to store_timestamp().
  EXPECT_CALL(b, store_timestamp(_)).Times(1);
  EXPECT_CALL(c, store_timestamp(_)).Times(0);
  insert.set_function_defaults(&table);
}

}  // namespace copy_info_unittest
