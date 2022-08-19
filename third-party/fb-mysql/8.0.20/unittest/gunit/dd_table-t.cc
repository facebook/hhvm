/* Copyright (c) 2013, 2017, Oracle and/or its affiliates. All rights reserved.

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

#include "my_config.h"

#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "sql/dd/dd.h"
#include "sql/dd/impl/types/table_impl.h"
#include "sql/dd/properties.h"
#include "sql/dd/types/column.h"

namespace dd_columns_unittest {

class ColumnsTest : public ::testing::Test {
 protected:
  typedef dd::Collection<dd::Column *> Column_collection;
  Column_collection m_columns;
  dd::Table_impl *m_table;

  void SetUp() {
    m_table = dynamic_cast<dd::Table_impl *>(dd::create_object<dd::Table>());
  }

  void TearDown() { delete m_table; }

  dd::Column *add_column() { return m_table->add_column(); }

  const dd::Column *get_column(dd::String_type name) {
    return m_table->get_column(name);
  }

  const Column_collection &columns() { return *m_table->columns(); }

  ColumnsTest() {}

 private:
  GTEST_DISALLOW_COPY_AND_ASSIGN_(ColumnsTest);
};

TEST_F(ColumnsTest, ColumnsConstIterator) {
  dd::Column *c1 = add_column();
  c1->set_name("col1");

  dd::Column *c2 = add_column();
  c2->set_name("col2");

  dd::Column *c3 = add_column();
  c3->set_name("Col3");

  dd::Column *c4 = add_column();
  c4->set_name("col3");

  dd::Column *c5 = add_column();
  c5->set_name("col4");

  const dd::Column *found_c3 = get_column("Col3");
  // Column names are case insensitive
  const dd::Column *found_c3_2 = get_column("col3");
  const dd::Column *found_c5 = get_column("col4");

  EXPECT_TRUE(found_c3 == c3);
  EXPECT_TRUE(found_c3_2 == c3);
  EXPECT_TRUE(found_c5 == c5);
}

TEST_F(ColumnsTest, Collection) {
  dd::Column *c1 = add_column();
  c1->set_name("col1");

  dd::Column *c2 = add_column();
  c2->set_name("col2");

  dd::Column *c3 = add_column();
  c3->set_name("Col3");

  dd::Column *c4 = add_column();
  c4->set_name("col3");

  dd::Column *c5 = add_column();
  c5->set_name("col4");

  EXPECT_EQ(c1, columns().front());
  EXPECT_EQ(c5, columns().back());
  EXPECT_EQ(c1, columns()[0]);
  EXPECT_EQ(c2, columns()[1]);
  EXPECT_EQ(c3, columns()[2]);
  EXPECT_EQ(c4, columns()[3]);
  EXPECT_EQ(c5, columns()[4]);
}

}  // namespace dd_columns_unittest
