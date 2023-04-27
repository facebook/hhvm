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

#include "sql/intrusive_list_iterator.h"
#include "sql/item_func.h"
#include "sql/sql_lex.h"
#include "template_utils.h"
#include "thr_lock.h"
#include "unittest/gunit/parsertest.h"
#include "unittest/gunit/test_utils.h"

namespace locking_clause_syntax_unittest {

using my_testing::Mock_error_handler;
using my_testing::Server_initializer;

class LockingClauseSyntaxTest : public ParserTest {};

/// Class that lets us use the indexing operator on intrusive lists.
template <typename T>
class Intrusive_list_indexer {
 public:
  Intrusive_list_indexer(SQL_I_List<T> list) : m_list(list) {}
  T *operator[](int i) {
    int curr = 0;
    for (T *t = m_list.first; t != nullptr; t = t->next_local, ++curr)
      if (curr == i) return t;
    EXPECT_TRUE(false);
    return nullptr;
  }

 private:
  SQL_I_List<T> m_list;
};

typedef Intrusive_list_indexer<TABLE_LIST> Table_list_indexer;

using Local_tables_iterator =
    IntrusiveListIterator<TABLE_LIST, &TABLE_LIST::next_local>;

/// A list interface over the TABLE_LIST::next_local pointer.
using Local_tables_list = IteratorContainer<Local_tables_iterator>;

TEST_F(LockingClauseSyntaxTest, LegacyForUpdate) {
  SELECT_LEX *term = parse("SELECT * FROM t0, t1, t2 FOR UPDATE");

  for (auto table : Local_tables_list(term->table_list.first)) {
    EXPECT_EQ(TL_WRITE, table->lock_descriptor().type);
    EXPECT_EQ(THR_WAIT, table->lock_descriptor().action);
  }
}

TEST_F(LockingClauseSyntaxTest, LegacyShared) {
  SELECT_LEX *term = parse("SELECT * FROM t0, t1, t2 LOCK IN SHARE MODE");

  for (auto table : Local_tables_list(term->table_list.first)) {
    EXPECT_EQ(TL_READ_WITH_SHARED_LOCKS, table->lock_descriptor().type);
    EXPECT_EQ(THR_WAIT, table->lock_descriptor().action);
  }
}

TEST_F(LockingClauseSyntaxTest, NameResolution) {
  SELECT_LEX *term = parse(
      "SELECT * "
      "FROM t0 t0_alias, "
      "test.t1, "
      "t2 t2_alias, "
      "t3, "
      "test.t4 t4_alias, "
      "t5, t6, "
      "t7, t8, "
      "t9, t10, "
      "t11 "
      "FOR UPDATE OF t0_alias "
      "FOR UPDATE OF test.t1 SKIP LOCKED "
      "FOR UPDATE OF t2_alias "
      "FOR UPDATE OF t3 NOWAIT "
      "FOR SHARE OF t4_alias "
      "FOR SHARE OF t5, t6 SKIP LOCKED "
      "FOR SHARE OF t7, t8 "
      "FOR SHARE OF t9, t10 NOWAIT ");

  Table_list_indexer tables(term->table_list);

  EXPECT_EQ(TL_WRITE, tables[0]->lock_descriptor().type);
  EXPECT_EQ(THR_WAIT, tables[0]->lock_descriptor().action);

  EXPECT_EQ(TL_WRITE, tables[1]->lock_descriptor().type);
  EXPECT_EQ(THR_SKIP, tables[1]->lock_descriptor().action);

  EXPECT_EQ(TL_WRITE, tables[2]->lock_descriptor().type);
  EXPECT_EQ(THR_WAIT, tables[2]->lock_descriptor().action);

  EXPECT_EQ(TL_WRITE, tables[3]->lock_descriptor().type);
  EXPECT_EQ(THR_NOWAIT, tables[3]->lock_descriptor().action);

  EXPECT_EQ(TL_READ_WITH_SHARED_LOCKS, tables[4]->lock_descriptor().type);
  EXPECT_EQ(THR_WAIT, tables[4]->lock_descriptor().action);

  EXPECT_EQ(TL_READ_WITH_SHARED_LOCKS, tables[5]->lock_descriptor().type);
  EXPECT_EQ(THR_SKIP, tables[5]->lock_descriptor().action);

  EXPECT_EQ(TL_READ_WITH_SHARED_LOCKS, tables[6]->lock_descriptor().type);
  EXPECT_EQ(THR_SKIP, tables[6]->lock_descriptor().action);

  EXPECT_EQ(TL_READ_WITH_SHARED_LOCKS, tables[7]->lock_descriptor().type);
  EXPECT_EQ(THR_WAIT, tables[7]->lock_descriptor().action);

  EXPECT_EQ(TL_READ_WITH_SHARED_LOCKS, tables[8]->lock_descriptor().type);
  EXPECT_EQ(THR_WAIT, tables[8]->lock_descriptor().action);

  EXPECT_EQ(TL_READ_WITH_SHARED_LOCKS, tables[9]->lock_descriptor().type);
  EXPECT_EQ(THR_NOWAIT, tables[9]->lock_descriptor().action);

  EXPECT_EQ(TL_READ_WITH_SHARED_LOCKS, tables[10]->lock_descriptor().type);
  EXPECT_EQ(THR_NOWAIT, tables[10]->lock_descriptor().action);
}

// safe_to_cache_query should be false if there's a locking clause.
TEST_F(LockingClauseSyntaxTest, SafeToCacheQuery) {
  parse("SELECT * FROM t1");
  EXPECT_TRUE(thd()->lex->safe_to_cache_query);

  parse("SELECT * FROM t1 FOR UPDATE");
  EXPECT_FALSE(thd()->lex->safe_to_cache_query);

  parse("SELECT * FROM t1 FOR UPDATE OF t1");
  EXPECT_FALSE(thd()->lex->safe_to_cache_query);

  parse("SELECT * FROM t1, t2 FOR UPDATE OF t1");
  EXPECT_FALSE(thd()->lex->safe_to_cache_query);

  parse("SELECT * FROM t1 LOCK IN SHARE MODE");
  EXPECT_FALSE(thd()->lex->safe_to_cache_query);

  parse("SELECT * FROM t1 FOR SHARE OF t1");
  EXPECT_FALSE(thd()->lex->safe_to_cache_query);

  parse("SELECT * FROM t1, t2 FOR SHARE OF t1");
  EXPECT_FALSE(thd()->lex->safe_to_cache_query);

  init_sql_command_flags();

  // The locking clause should not get contextualized for EXPLAIN.
  parse("EXPLAIN SELECT * FROM t1 FOR UPDATE");
  EXPECT_TRUE(thd()->lex->safe_to_cache_query);

  parse("EXPLAIN SELECT * FROM t1 FOR UPDATE OF t1");
  EXPECT_TRUE(thd()->lex->safe_to_cache_query);

  parse("EXPLAIN SELECT * FROM t1, t2 FOR UPDATE OF t1");
  EXPECT_TRUE(thd()->lex->safe_to_cache_query);

  parse("EXPLAIN SELECT * FROM t1 LOCK IN SHARE MODE");
  EXPECT_TRUE(thd()->lex->safe_to_cache_query);

  parse("EXPLAIN SELECT * FROM t1 FOR SHARE OF t1");
  EXPECT_TRUE(thd()->lex->safe_to_cache_query);

  parse("EXPLAIN SELECT * FROM t1, t2 FOR SHARE OF t1");
  EXPECT_TRUE(thd()->lex->safe_to_cache_query);
}

TEST_F(LockingClauseSyntaxTest, BinlogSafety) {
  const auto unsafe_skip_locked =
      Query_tables_list::BINLOG_STMT_UNSAFE_SKIP_LOCKED;
  const auto unsafe_nowait = Query_tables_list::BINLOG_STMT_UNSAFE_NOWAIT;

  parse("INSERT INTO t1 SELECT * FROM t1 FOR UPDATE");
  EXPECT_FALSE(thd()->lex->is_stmt_unsafe(unsafe_skip_locked));
  EXPECT_FALSE(thd()->lex->is_stmt_unsafe(unsafe_nowait));

  parse("INSERT INTO t1 SELECT * FROM t1 FOR UPDATE SKIP LOCKED");
  EXPECT_TRUE(thd()->lex->is_stmt_unsafe(unsafe_skip_locked));
  EXPECT_FALSE(thd()->lex->is_stmt_unsafe(unsafe_nowait));

  parse("INSERT INTO t1 SELECT * FROM t1 FOR UPDATE NOWAIT");
  EXPECT_FALSE(thd()->lex->is_stmt_unsafe(unsafe_skip_locked));
  EXPECT_TRUE(thd()->lex->is_stmt_unsafe(unsafe_nowait));

  parse(
      "INSERT INTO t1 SELECT * FROM t1, t2 "
      "FOR UPDATE OF t1 SKIP LOCKED FOR UPDATE OF t2 NOWAIT");
  EXPECT_TRUE(thd()->lex->is_stmt_unsafe(unsafe_skip_locked));
  EXPECT_TRUE(thd()->lex->is_stmt_unsafe(unsafe_nowait));

  parse(
      "UPDATE t3 SET a = "
      "(SELECT b FROM t1 WHERE a >= 2 LIMIT 1 FOR UPDATE SKIP LOCKED)");
  EXPECT_TRUE(thd()->lex->is_stmt_unsafe(unsafe_skip_locked));
  EXPECT_FALSE(thd()->lex->is_stmt_unsafe(unsafe_nowait));

  parse(
      "UPDATE t3 SET a = "
      "(SELECT b FROM t1 WHERE a >= 2 LIMIT 1 FOR UPDATE NOWAIT)");
  EXPECT_FALSE(thd()->lex->is_stmt_unsafe(unsafe_skip_locked));
  EXPECT_TRUE(thd()->lex->is_stmt_unsafe(unsafe_nowait));
}

}  // namespace locking_clause_syntax_unittest
