/* Copyright (c) 2019, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include <random>
#include <unordered_set>
#include <vector>

#include "extra/lz4/my_xxhash.h"
#include "include/my_murmur3.h"
#include "my_alloc.h"
#include "sql/hash_join_buffer.h"
#include "sql/hash_join_iterator.h"
#include "sql/item_cmpfunc.h"
#include "sql/row_iterator.h"
#include "sql/sql_executor.h"
#include "sql/sql_optimizer.h"
#include "sql_string.h"
#include "template_utils.h"
#include "unittest/gunit/benchmark.h"
#include "unittest/gunit/fake_integer_iterator.h"
#include "unittest/gunit/fake_string_iterator.h"
#include "unittest/gunit/fake_table.h"
#include "unittest/gunit/mock_field_long.h"
#include "unittest/gunit/parsertest.h"
#include "unittest/gunit/temptable/mock_field_varstring.h"
#include "unittest/gunit/test_utils.h"

namespace hash_join_unittest {

using std::vector;

static hash_join_buffer::TableCollection CreateTenTableJoin(
    const my_testing::Server_initializer &initializer, MEM_ROOT *mem_root,
    bool store_data) {
  constexpr int kNumColumns = 10;
  constexpr bool kColumnsNullable = true;
  constexpr int kNumTablesInJoin = 10;

  // Set up a ten-table join. For simplicity, allocate everything on a MEM_ROOT
  // that will take care of releasing allocated memory.
  SELECT_LEX *select_lex = parse(&initializer, "SELECT * FROM dummy", 0);
  JOIN join(initializer.thd(), select_lex);
  join.qep_tab = mem_root->ArrayAlloc<QEP_TAB>(kNumTablesInJoin);
  join.tables = kNumTablesInJoin;
  for (int i = 0; i < kNumTablesInJoin; ++i) {
    Fake_TABLE *fake_table =
        new (mem_root) Fake_TABLE(kNumColumns, kColumnsNullable);
    QEP_TAB *qep_tab = &join.qep_tab[i];
    qep_tab->set_qs(new (mem_root) QEP_shared);
    qep_tab->set_table(fake_table);
    qep_tab->table_ref = fake_table->pos_in_table_list;

    if (store_data) {
      bitmap_set_all(fake_table->write_set);

      for (uint j = 0; j < fake_table->s->fields; ++j) {
        fake_table->field[j]->store(1000, false /* is_unsigned */);
      }
    }
  }

  return hash_join_buffer::TableCollection(&join,
                                           TablesBetween(0, kNumTablesInJoin));
}

static void DestroyFakeTables(
    const hash_join_buffer::TableCollection &table_collection) {
  for (const hash_join_buffer::Table &table : table_collection.tables())
    destroy(pointer_cast<Fake_TABLE *>(table.qep_tab->table()));
}

static void BM_StoreFromTableBuffersNoData(size_t num_iterations) {
  StopBenchmarkTiming();

  my_testing::Server_initializer initializer;
  initializer.SetUp();
  MEM_ROOT mem_root;
  hash_join_buffer::TableCollection table_collection =
      CreateTenTableJoin(initializer, &mem_root, false);

  String buffer;
  buffer.reserve(1024);

  StartBenchmarkTiming();
  for (size_t i = 0; i < num_iterations; ++i) {
    ASSERT_FALSE(
        hash_join_buffer::StoreFromTableBuffers(table_collection, &buffer));
    ASSERT_GT(buffer.length(), 0);
  }
  StopBenchmarkTiming();

  DestroyFakeTables(table_collection);
  initializer.TearDown();
}
BENCHMARK(BM_StoreFromTableBuffersNoData)

static void BM_StoreFromTableBuffersWithData(size_t num_iterations) {
  StopBenchmarkTiming();

  my_testing::Server_initializer initializer;
  initializer.SetUp();

  MEM_ROOT mem_root;
  hash_join_buffer::TableCollection table_collection =
      CreateTenTableJoin(initializer, &mem_root, true);

  String buffer;
  buffer.reserve(1024);

  StartBenchmarkTiming();
  for (size_t i = 0; i < num_iterations; ++i) {
    ASSERT_FALSE(
        hash_join_buffer::StoreFromTableBuffers(table_collection, &buffer));
    ASSERT_GT(buffer.length(), 0);
  }
  StopBenchmarkTiming();

  DestroyFakeTables(table_collection);
  initializer.TearDown();
}
BENCHMARK(BM_StoreFromTableBuffersWithData)

// Return eight bytes of data.
static vector<uchar> GetShortData() { return {1, 2, 3, 4, 5, 6, 7, 8}; }

// Return 1024 bytes of data.
static vector<uchar> GetLongData() {
  constexpr int kDataSize = 1024;
  vector<uchar> data(kDataSize);
  for (int i = 0; i < kDataSize; ++i) {
    data.push_back(i);
  }
  return data;
}

static void BM_Murmur3ShortData(size_t num_iterations) {
  StopBenchmarkTiming();

  vector<uchar> data = GetShortData();
  StartBenchmarkTiming();

  size_t sum = 0;
  for (size_t i = 0; i < num_iterations; ++i) {
    sum += murmur3_32(&data[0], data.size(), 0);
  }
  StopBenchmarkTiming();

  // The sum variable is just to assure that the compiler doesn't optimize away
  // the entire for loop.
  EXPECT_NE(0, sum);
  SetBytesProcessed(num_iterations * data.size());
}
BENCHMARK(BM_Murmur3ShortData)

static void BM_MurmurLongData(size_t num_iterations) {
  StopBenchmarkTiming();

  vector<uchar> data = GetLongData();
  StartBenchmarkTiming();

  size_t sum = 0;
  for (size_t i = 0; i < num_iterations; ++i) {
    sum += murmur3_32(&data[0], data.size(), 0);
  }
  StopBenchmarkTiming();

  // The sum variable is just to assure that the compiler doesn't optimize away
  // the entire for loop.
  EXPECT_NE(0, sum);
  SetBytesProcessed(num_iterations * data.size());
}
BENCHMARK(BM_MurmurLongData)

static void BM_XXHash64ShortData(size_t num_iterations) {
  StopBenchmarkTiming();

  vector<uchar> data = GetShortData();
  StartBenchmarkTiming();

  size_t sum = 0;
  for (size_t i = 0; i < num_iterations; ++i) {
    sum += MY_XXH64(&data[0], data.size(), 0);
  }
  StopBenchmarkTiming();

  // The sum variable is just to assure that the compiler doesn't optimize away
  // the entire for loop.
  EXPECT_NE(0, sum);
  SetBytesProcessed(num_iterations * data.size());
}
BENCHMARK(BM_XXHash64ShortData)

static void BM_XXHash64LongData(size_t num_iterations) {
  StopBenchmarkTiming();

  vector<uchar> data = GetLongData();
  StartBenchmarkTiming();

  size_t sum = 0;
  for (size_t i = 0; i < num_iterations; ++i) {
    sum += MY_XXH64(&data[0], data.size(), 0);
  }
  StopBenchmarkTiming();

  // The sum variable is just to assure that the compiler doesn't optimize away
  // the entire for loop.
  EXPECT_NE(0, sum);
  SetBytesProcessed(num_iterations * data.size());
}
BENCHMARK(BM_XXHash64LongData)

// A class that takes care of setting up an environment for testing a hash join
// iterator. The constructors will set up two tables (left and right), as well
// as two (fake) iterators that reads data from these two tables. Both tables
// has only one column, and the join condition between the two tables is a
// simple equality between these two columns. There are two different
// constructors; one for integer data sets and one for string data sets.
//
// The user must provide the data contents for both tables in the constructor.
class HashJoinTestHelper {
 public:
  unique_ptr_destroy_only<RowIterator> left_iterator;
  unique_ptr_destroy_only<RowIterator> right_iterator;
  QEP_TAB *left_qep_tab;
  QEP_TAB *right_qep_tab;
  HashJoinCondition *join_condition = nullptr;
  std::vector<Item *> extra_conditions;

  HashJoinTestHelper(Server_initializer *initializer,
                     const vector<int> &left_dataset,
                     const vector<int> &right_dataset) {
    m_left_table_field.reset(
        new (&m_mem_root) Mock_field_long("column1", false /* is_nullable */));
    m_left_table.reset(new (&m_mem_root) Fake_TABLE(m_left_table_field.get()));

    m_right_table_field.reset(
        new (&m_mem_root) Mock_field_long("column1", false /* is_nullable */));
    m_right_table.reset(new (&m_mem_root)
                            Fake_TABLE(m_right_table_field.get()));
    SetupFakeTables(initializer);

    left_iterator.reset(new (&m_mem_root) FakeIntegerIterator(
        initializer->thd(), m_left_table.get(),
        down_cast<Field_long *>(m_left_table->field[0]), move(left_dataset)));
    right_iterator.reset(new (&m_mem_root) FakeIntegerIterator(
        initializer->thd(), m_right_table.get(),
        down_cast<Field_long *>(m_right_table->field[0]), move(right_dataset)));
  }

  HashJoinTestHelper(Server_initializer *initializer,
                     const vector<std::string> &left_dataset,
                     const vector<std::string> &right_dataset)
      : extra_conditions(PSI_NOT_INSTRUMENTED) {
    m_left_table_field.reset(new (&m_mem_root) Mock_field_varstring(
        nullptr, "column1", 255 /* length */, false /* is_nullable */));
    m_left_table.reset(new (&m_mem_root) Fake_TABLE(m_left_table_field.get()));

    m_right_table_field.reset(new (&m_mem_root) Mock_field_varstring(
        nullptr, "column1", 255 /* length */, false /* is_nullable */));
    m_right_table.reset(new (&m_mem_root)
                            Fake_TABLE(m_right_table_field.get()));
    SetupFakeTables(initializer);

    left_iterator.reset(new (&m_mem_root) FakeStringIterator(
        initializer->thd(), m_left_table.get(),
        down_cast<Field_varstring *>(m_left_table->field[0]),
        move(left_dataset)));
    right_iterator.reset(new (&m_mem_root) FakeStringIterator(
        initializer->thd(), m_right_table.get(),
        down_cast<Field_varstring *>(m_right_table->field[0]),
        move(right_dataset)));
  }

 private:
  void SetupFakeTables(Server_initializer *initializer) {
    bitmap_set_all(m_left_table->write_set);
    bitmap_set_all(m_left_table->read_set);
    bitmap_set_all(m_right_table->write_set);
    bitmap_set_all(m_right_table->read_set);

    SELECT_LEX *select_lex =
        parse(initializer,
              "SELECT * FROM t1 JOIN t2 ON (t1.column1 = t2.column1);", 0);
    JOIN *join = new (&m_mem_root) JOIN(initializer->thd(), select_lex);
    join->tables = 2;
    join->qep_tab = m_mem_root.ArrayAlloc<QEP_TAB>(join->tables);

    left_qep_tab = &join->qep_tab[0];
    left_qep_tab->set_qs(new (&m_mem_root) QEP_shared);
    left_qep_tab->set_idx(0);
    left_qep_tab->set_table(m_left_table.get());
    left_qep_tab->table_ref = m_left_table->pos_in_table_list;
    left_qep_tab->set_join(join);

    right_qep_tab = &join->qep_tab[1];
    right_qep_tab->set_qs(new (&m_mem_root) QEP_shared);
    right_qep_tab->set_idx(1);
    right_qep_tab->set_table(m_right_table.get());
    right_qep_tab->table_ref = m_right_table->pos_in_table_list;
    right_qep_tab->set_join(join);

    Item_func_eq *eq =
        new Item_func_eq(new Item_field(m_left_table->field[0]),
                         new Item_field(m_right_table->field[0]));
    eq->set_cmp_func();
    join_condition = new (&m_mem_root) HashJoinCondition(eq, &m_mem_root);
  }

  // For simplicity, we allocate everything on a MEM_ROOT that takes care of
  // releasing any memory. However, we must ensure that the destructor is called
  // for Mock_field_varstring. Wrapping the fields in a unique_ptr_destroy_only
  // will ensure this.
  MEM_ROOT m_mem_root;
  unique_ptr_destroy_only<Fake_TABLE> m_left_table;
  unique_ptr_destroy_only<Fake_TABLE> m_right_table;
  unique_ptr_destroy_only<Field> m_left_table_field;
  unique_ptr_destroy_only<Field> m_right_table_field;
};

TEST(HashJoinTest, InnerJoinIntOneToOneMatch) {
  my_testing::Server_initializer initializer;
  initializer.SetUp();

  vector<int> left_data;
  left_data.push_back(3);

  vector<int> right_data;
  right_data.push_back(3);

  HashJoinTestHelper test_helper(&initializer, left_data, right_data);

  HashJoinIterator hash_join_iterator(
      initializer.thd(), std::move(test_helper.left_iterator),
      test_helper.left_qep_tab->idx_map(),
      std::move(test_helper.right_iterator),
      test_helper.right_qep_tab->idx_map(), 10 * 1024 * 1024 /* 10 MB */,
      {*test_helper.join_condition}, true, JoinType::INNER,
      test_helper.left_qep_tab->join(), test_helper.extra_conditions);

  ASSERT_FALSE(hash_join_iterator.Init());

  EXPECT_EQ(0, hash_join_iterator.Read());
  EXPECT_EQ(3, test_helper.left_qep_tab->table()->field[0]->val_int());
  EXPECT_EQ(-1, hash_join_iterator.Read());

  initializer.TearDown();
}

TEST(HashJoinTest, InnerJoinIntNoMatch) {
  my_testing::Server_initializer initializer;
  initializer.SetUp();

  HashJoinTestHelper test_helper(&initializer, {2, 4}, {3, 5});

  HashJoinIterator hash_join_iterator(
      initializer.thd(), std::move(test_helper.left_iterator),
      test_helper.left_qep_tab->idx_map(),
      std::move(test_helper.right_iterator),
      test_helper.right_qep_tab->idx_map(), 10 * 1024 * 1024 /* 10 MB */,
      {*test_helper.join_condition}, true, JoinType::INNER,
      test_helper.left_qep_tab->join(), test_helper.extra_conditions);

  ASSERT_FALSE(hash_join_iterator.Init());
  EXPECT_EQ(-1, hash_join_iterator.Read());
  initializer.TearDown();
}

TEST(HashJoinTest, InnerJoinIntOneToManyMatch) {
  my_testing::Server_initializer initializer;
  initializer.SetUp();

  HashJoinTestHelper test_helper(&initializer, {2}, {2, 2});

  HashJoinIterator hash_join_iterator(
      initializer.thd(), std::move(test_helper.left_iterator),
      test_helper.left_qep_tab->idx_map(),
      std::move(test_helper.right_iterator),
      test_helper.right_qep_tab->idx_map(), 10 * 1024 * 1024 /* 10 MB */,
      {*test_helper.join_condition}, true, JoinType::INNER,
      test_helper.left_qep_tab->join(), test_helper.extra_conditions);

  ASSERT_FALSE(hash_join_iterator.Init());

  // We expect two result rows before the iterator should return -1 (EOF).
  EXPECT_EQ(0, hash_join_iterator.Read());
  EXPECT_EQ(2, test_helper.left_qep_tab->table()->field[0]->val_int());

  EXPECT_EQ(0, hash_join_iterator.Read());
  EXPECT_EQ(2, test_helper.left_qep_tab->table()->field[0]->val_int());

  EXPECT_EQ(-1, hash_join_iterator.Read());
  initializer.TearDown();
}

TEST(HashJoinTest, InnerJoinStringOneToOneMatch) {
  my_testing::Server_initializer initializer;
  initializer.SetUp();

  HashJoinTestHelper test_helper(&initializer, {"abc"}, {"abc"});

  HashJoinIterator hash_join_iterator(
      initializer.thd(), std::move(test_helper.left_iterator),
      test_helper.left_qep_tab->idx_map(),
      std::move(test_helper.right_iterator),
      test_helper.right_qep_tab->idx_map(), 10 * 1024 * 1024 /* 10 MB */,
      {*test_helper.join_condition}, true, JoinType::INNER,
      test_helper.left_qep_tab->join(), test_helper.extra_conditions);

  ASSERT_FALSE(hash_join_iterator.Init());

  EXPECT_EQ(0, hash_join_iterator.Read());
  String buffer;
  String *result =
      test_helper.left_qep_tab->table()->field[0]->val_str(&buffer);
  EXPECT_EQ(std::string(result->ptr(), result->length()), std::string("abc"));

  EXPECT_EQ(-1, hash_join_iterator.Read());
  initializer.TearDown();
}

// Do a benchmark of HashJoinIterator::Init(). This function is responsible for
// building the hash table, and this step is also known as the "build phase".
//
// The table that the hash table is built from is a single-column table with
// 10000 uniformly distributed values between [0, 10000). We give the hash table
// enough memory so that it doesn't spill out to disk.
static void BM_HashTableIteratorBuild(size_t num_iterations) {
  StopBenchmarkTiming();

  my_testing::Server_initializer initializer;
  initializer.SetUp();

  const int min_value = 0;
  const int max_value = 10000;
  const int seed = 8834245;
  std::mt19937 generator(seed);
  std::uniform_int_distribution<> distribution(min_value, max_value);

  vector<int> left_dataset;
  vector<int> right_dataset;
  for (int i = 0; i < (max_value - min_value); ++i) {
    left_dataset.push_back(distribution(generator));
    right_dataset.push_back(distribution(generator));
  }

  HashJoinTestHelper test_helper(&initializer, left_dataset, right_dataset);

  HashJoinIterator hash_join_iterator(
      initializer.thd(), std::move(test_helper.left_iterator),
      test_helper.left_qep_tab->idx_map(),
      std::move(test_helper.right_iterator),
      test_helper.right_qep_tab->idx_map(), 10 * 1024 * 1024 /* 10 MB */,
      {*test_helper.join_condition}, true, JoinType::INNER,
      test_helper.left_qep_tab->join(), test_helper.extra_conditions);

  StartBenchmarkTiming();
  for (size_t i = 0; i < num_iterations; ++i) {
    ASSERT_FALSE(hash_join_iterator.Init());
  }
  StopBenchmarkTiming();

  initializer.TearDown();
}
BENCHMARK(BM_HashTableIteratorBuild)

// Do a benchmark of HashJoinIterator::Read(). This function will read a row
// from the right table, and look for a matching row in the hash table. This is
// also known as the "probe phase".
//
// The table that the hash table is built from is a single-column table with
// 10000 uniformly distributed values between [0, 10000). We give the hash table
// enough memory so that is doesn't spill out to disk.
static void BM_HashTableIteratorProbe(size_t num_iterations) {
  StopBenchmarkTiming();

  my_testing::Server_initializer initializer;
  initializer.SetUp();

  const int min_value = 0;
  const int max_value = 10000;
  const int seed = 8834245;
  std::mt19937 generator(seed);
  std::uniform_int_distribution<> distribution(min_value, max_value);

  vector<int> left_dataset;
  vector<int> right_dataset;
  for (int i = 0; i < (max_value - min_value); ++i) {
    left_dataset.push_back(distribution(generator));
    right_dataset.push_back(distribution(generator));
  }
  HashJoinTestHelper test_helper(&initializer, left_dataset, right_dataset);

  HashJoinIterator hash_join_iterator(
      initializer.thd(), std::move(test_helper.left_iterator),
      test_helper.left_qep_tab->idx_map(),
      std::move(test_helper.right_iterator),
      test_helper.right_qep_tab->idx_map(), 10 * 1024 * 1024 /* 10 MB */,
      {*test_helper.join_condition}, true, JoinType::INNER,
      test_helper.left_qep_tab->join(), test_helper.extra_conditions);

  for (size_t i = 0; i < num_iterations; ++i) {
    ASSERT_FALSE(hash_join_iterator.Init());
    StartBenchmarkTiming();
    int result;
    do {
      result = hash_join_iterator.Read();
    } while (result == 0);
    StopBenchmarkTiming();
  }

  initializer.TearDown();
}
BENCHMARK(BM_HashTableIteratorProbe)

// Do a benchmark of lookup in the hash table for semijoin. This is to see if
// there is any difference between equal_range() and find(),
//
// The table that the hash table is built from is a single-column table with
// 10000 uniformly distributed values between [0, 5000). We give the hash table
// enough memory so that is doesn't spill out to disk.
static void BM_HashTableIteratorProbeSemiJoin(size_t num_iterations) {
  StopBenchmarkTiming();

  my_testing::Server_initializer initializer;
  initializer.SetUp();

  const int num_value = 10000;
  const int min_value = 0;
  const int max_value = 5000;
  const int seed = 8834245;
  std::mt19937 generator(seed);
  std::uniform_int_distribution<> distribution(min_value, max_value);

  vector<int> left_dataset;
  vector<int> right_dataset;
  for (int i = 0; i < num_value; ++i) {
    left_dataset.push_back(distribution(generator));
    right_dataset.push_back(distribution(generator));
  }
  HashJoinTestHelper test_helper(&initializer, left_dataset, right_dataset);

  HashJoinIterator hash_join_iterator(
      initializer.thd(), std::move(test_helper.left_iterator),
      test_helper.left_qep_tab->idx_map(),
      std::move(test_helper.right_iterator),
      test_helper.right_qep_tab->idx_map(), 10 * 1024 * 1024 /* 10 MB */,
      {*test_helper.join_condition}, true, JoinType::SEMI,
      test_helper.left_qep_tab->join(), test_helper.extra_conditions);

  for (size_t i = 0; i < num_iterations; ++i) {
    ASSERT_FALSE(hash_join_iterator.Init());
    StartBenchmarkTiming();
    int result;
    do {
      result = hash_join_iterator.Read();
    } while (result == 0);
    StopBenchmarkTiming();
  }

  initializer.TearDown();
}
BENCHMARK(BM_HashTableIteratorProbeSemiJoin)

TEST(HashJoinTest, SemiJoinInt) {
  my_testing::Server_initializer initializer;
  initializer.SetUp();

  // The iterator will execute something that is equivalent to the query
  // "SELECT * FROM probe_data WHERE a IN (SELECT b FROM build_data);"
  vector<int> build_data;
  build_data.push_back(3);
  build_data.push_back(3);
  build_data.push_back(4);
  build_data.push_back(5);

  vector<int> probe_data;
  probe_data.push_back(3);
  probe_data.push_back(5);
  probe_data.push_back(6);

  HashJoinTestHelper test_helper(&initializer, build_data, probe_data);

  HashJoinIterator hash_join_iterator(
      initializer.thd(), std::move(test_helper.left_iterator),
      test_helper.left_qep_tab->idx_map(),
      std::move(test_helper.right_iterator),
      test_helper.right_qep_tab->idx_map(), 10 * 1024 * 1024 /* 10 MB */,
      {*test_helper.join_condition}, true, JoinType::SEMI,
      test_helper.left_qep_tab->join(), test_helper.extra_conditions);

  ASSERT_FALSE(hash_join_iterator.Init());

  std::unordered_set<longlong> expected_result;
  expected_result.emplace(3);
  expected_result.emplace(5);

  EXPECT_EQ(0, hash_join_iterator.Read());
  longlong result = test_helper.right_qep_tab->table()->field[0]->val_int();
  EXPECT_EQ(1, expected_result.erase(result));

  EXPECT_EQ(0, hash_join_iterator.Read());
  result = test_helper.right_qep_tab->table()->field[0]->val_int();
  EXPECT_EQ(1, expected_result.erase(result));

  EXPECT_EQ(-1, hash_join_iterator.Read());
  EXPECT_TRUE(expected_result.empty());

  initializer.TearDown();
}

TEST(HashJoinTest, AntiJoinInt) {
  my_testing::Server_initializer initializer;
  initializer.SetUp();

  // The iterator will execute something that is equivalent to the query
  // "SELECT * FROM probe_data WHERE a NOT IN (SELECT b FROM build_data);"
  vector<int> build_data;
  build_data.push_back(3);
  build_data.push_back(3);
  build_data.push_back(4);
  build_data.push_back(5);

  vector<int> probe_data;
  probe_data.push_back(3);
  probe_data.push_back(5);
  probe_data.push_back(6);

  HashJoinTestHelper test_helper(&initializer, build_data, probe_data);

  HashJoinIterator hash_join_iterator(
      initializer.thd(), std::move(test_helper.left_iterator),
      test_helper.left_qep_tab->idx_map(),
      std::move(test_helper.right_iterator),
      test_helper.right_qep_tab->idx_map(), 10 * 1024 * 1024 /* 10 MB */,
      {*test_helper.join_condition}, true, JoinType::ANTI,
      test_helper.left_qep_tab->join(), test_helper.extra_conditions);

  ASSERT_FALSE(hash_join_iterator.Init());

  EXPECT_EQ(0, hash_join_iterator.Read());
  EXPECT_EQ(6, test_helper.right_qep_tab->table()->field[0]->val_int());
  EXPECT_EQ(-1, hash_join_iterator.Read());

  initializer.TearDown();
}

TEST(HashJoinTest, LeftHashJoinInt) {
  my_testing::Server_initializer initializer;
  initializer.SetUp();

  // The iterator will execute something that is equivalent to the query
  // "SELECT * FROM left_data p LEFT JOIN right_data b ON p.col = b.col;"
  vector<int> left_data;
  left_data.push_back(3);

  vector<int> right_data;

  HashJoinTestHelper test_helper(&initializer, left_data, right_data);

  HashJoinIterator hash_join_iterator(
      initializer.thd(), std::move(test_helper.right_iterator),
      test_helper.right_qep_tab->idx_map(),
      std::move(test_helper.left_iterator), test_helper.left_qep_tab->idx_map(),
      10 * 1024 * 1024 /* 10 MB */, {*test_helper.join_condition}, true,
      JoinType::OUTER, test_helper.left_qep_tab->join(),
      test_helper.extra_conditions);

  ASSERT_FALSE(hash_join_iterator.Init());

  EXPECT_EQ(0, hash_join_iterator.Read());
  EXPECT_EQ(3, test_helper.left_qep_tab->table()->field[0]->val_int());
  EXPECT_FALSE(test_helper.left_qep_tab->table()->field[0]->is_null());

  test_helper.right_qep_tab->table()->field[0]->val_int();
  EXPECT_TRUE(test_helper.right_qep_tab->table()->field[0]->is_null());

  EXPECT_EQ(-1, hash_join_iterator.Read());

  initializer.TearDown();
}

TEST(HashJoinTest, HashJoinResetNullFlagBeforeBuild) {
  my_testing::Server_initializer initializer;
  initializer.SetUp();

  // The iterator will execute something that is equivalent to the query
  // "SELECT * FROM left_data p LEFT JOIN right_data b ON p.col = b.col;"
  vector<int> left_data;
  left_data.push_back(3);

  vector<int> right_data;
  right_data.push_back(3);

  HashJoinTestHelper test_helper(&initializer, left_data, right_data);

  // Explicitly set the NULL row flag for the right/build input. The hash join
  // iterator should reset this flag before building the hash table.
  test_helper.right_iterator->SetNullRowFlag(/*is_null_row=*/true);

  HashJoinIterator hash_join_iterator(
      initializer.thd(), std::move(test_helper.right_iterator),
      test_helper.right_qep_tab->idx_map(),
      std::move(test_helper.left_iterator), test_helper.left_qep_tab->idx_map(),
      10 * 1024 * 1024 /* 10 MB */, {*test_helper.join_condition}, true,
      JoinType::OUTER, test_helper.left_qep_tab->join(),
      test_helper.extra_conditions);

  ASSERT_FALSE(hash_join_iterator.Init());

  // Verify that we do not get any NULL value back, even though we explicitly
  // set the NULL row flag before Init was called; Init() should reset the NULL
  // row flag before reading from the build table.
  EXPECT_EQ(0, hash_join_iterator.Read());
  EXPECT_EQ(3, test_helper.left_qep_tab->table()->field[0]->val_int());
  EXPECT_FALSE(test_helper.left_qep_tab->table()->field[0]->is_null());

  EXPECT_EQ(3, test_helper.right_qep_tab->table()->field[0]->val_int());
  EXPECT_FALSE(test_helper.right_qep_tab->table()->field[0]->is_null());

  EXPECT_EQ(-1, hash_join_iterator.Read());

  initializer.TearDown();
}

}  // namespace hash_join_unittest
