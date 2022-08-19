/* Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.

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

// First include (the generated) my_config.h, to get correct platform defines.
#include "my_config.h"

#include <gtest/gtest.h>
#include <stddef.h>
#include <string>

#include "sql/item_func.h"
#include "sql/sql_lex.h"
#include "template_utils.h"
#include "thr_lock.h"
#include "unittest/gunit/parsertest.h"
#include "unittest/gunit/test_utils.h"

namespace join_syntax_unittest {

using my_testing::Mock_error_handler;
using my_testing::Server_initializer;

class JoinSyntaxTest : public ParserTest {};

void check_name_resolution_tables(std::initializer_list<const char *> aliases,
                                  SQL_I_List<TABLE_LIST> tables) {
  TABLE_LIST *table_list = tables.first;
  for (auto alias : aliases) {
    ASSERT_FALSE(table_list == nullptr);
    EXPECT_STREQ(alias, table_list->alias)
        << "Wrong table alias " << table_list->alias << ", expected " << alias
        << ".";
    table_list = table_list->next_name_resolution_table;
  }
}

TEST_F(JoinSyntaxTest, CrossJoin) {
  SELECT_LEX *query_block = parse("SELECT * FROM t1 JOIN t2 JOIN t3");
  check_name_resolution_tables({"t1", "t2", "t3"}, query_block->table_list);
}

TEST_F(JoinSyntaxTest, CrossJoinOn) {
  SELECT_LEX *query_block = parse("SELECT * FROM t1 JOIN t2 JOIN t3 ON 1");
  check_name_resolution_tables({"t1", "t2", "t3"}, query_block->table_list);
}

}  // namespace join_syntax_unittest
