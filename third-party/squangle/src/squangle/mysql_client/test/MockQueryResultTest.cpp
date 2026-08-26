/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include <string>
#include <string_view>
#include <vector>

#include "squangle/mysql_client/test/MockQueryResult.h"

namespace facebook::common::mysql_client::test {

// The range overload must reject string-like types: their characters would
// each convert to an integer CellValue, silently producing a row of numbers.
// The types real callers pass must still be accepted.
template <typename Range>
concept RowAccepts =
    requires(MockQueryResult::Builder b, const Range& v) { b.row(v); };

static_assert(!RowAccepts<std::string>);
static_assert(!RowAccepts<std::string_view>);
static_assert(RowAccepts<std::vector<std::string>>);
static_assert(RowAccepts<std::vector<CellValue>>);

TEST(MockQueryResultTest, BuildsRowFieldsFromColumns) {
  auto fields = MockQueryResult::builder()
                    .column("id", MYSQL_TYPE_LONG)
                    .column("name", MYSQL_TYPE_VARCHAR, "users")
                    .buildRowFields();

  ASSERT_EQ(fields->numFields(), 2);
  EXPECT_EQ(fields->fieldName(0), "id");
  EXPECT_EQ(fields->fieldName(1), "name");
  EXPECT_EQ(fields->tableName(1), "users");
  EXPECT_EQ(fields->getFieldType("name"), MYSQL_TYPE_VARCHAR);
  EXPECT_EQ(fields->fieldIndex("name"), 1);
}

TEST(MockQueryResultTest, BuildsRowBlockWithValues) {
  auto block = MockQueryResult::builder()
                   .column("id", MYSQL_TYPE_LONG)
                   .column("name", MYSQL_TYPE_VARCHAR)
                   .row({1, "alice"})
                   .row({2, nullptr})
                   .buildRowBlock();

  ASSERT_EQ(block.numRows(), 2);
  EXPECT_EQ(block.getField<int64_t>(0, "id"), 1);
  EXPECT_EQ(block.getField<std::string>(0, "name"), "alice");
  EXPECT_TRUE(block.isNull(1, 1));
}

TEST(MockQueryResultTest, RowAcceptsRuntimeRange) {
  std::vector<CellValue> values{7, "bob"};
  auto block = MockQueryResult::builder()
                   .column("id", MYSQL_TYPE_LONG)
                   .column("name", MYSQL_TYPE_VARCHAR)
                   .row(values)
                   .buildRowBlock();

  ASSERT_EQ(block.numRows(), 1);
  EXPECT_EQ(block.getField<int64_t>(0, "id"), 7);
  EXPECT_EQ(block.getField<std::string>(0, "name"), "bob");
}

// A temporary string handed to row() must be copied into the block's storage,
// not referenced -- the builder outlives the statement that supplied it.
TEST(MockQueryResultTest, CopiesTemporaryStrings) {
  auto builder = MockQueryResult::builder();
  builder.column("name", MYSQL_TYPE_VARCHAR);
  builder.row({std::string("temporary") + "-value"});

  auto block = builder.buildRowBlock();
  EXPECT_EQ(block.getField<std::string>(0, "name"), "temporary-value");
}

TEST(MockQueryResultTest, QueryResultIsCompleteAndSucceededByDefault) {
  auto result = MockQueryResult::builder()
                    .column("id", MYSQL_TYPE_LONG)
                    .row({1})
                    .buildQueryResult();

  // A raw QueryResult starts partial with an Unknown result; a mock stands in
  // for a fetch that already finished.
  EXPECT_FALSE(result.partial());
  EXPECT_TRUE(result.succeeded());
  EXPECT_EQ(result.numRows(), 1);
}

TEST(MockQueryResultTest, QueryResultCarriesCountersAndIteratesRows) {
  auto result = MockQueryResult::builder()
                    .column("id", MYSQL_TYPE_LONG)
                    .row({1})
                    .row({2})
                    .rowsMatched(2)
                    .lastInsertId(99)
                    .numRowsAffected(2)
                    .queryNum(3)
                    .buildQueryResult();

  EXPECT_EQ(result.rowsMatched(), 2);
  EXPECT_EQ(result.lastInsertId(), 99);
  EXPECT_EQ(result.numRowsAffected(), 2);
  EXPECT_EQ(result.queryNum(), 3);

  std::vector<int64_t> ids;
  for (auto row : result) {
    ids.push_back(row.get<int64_t>("id"));
  }
  EXPECT_EQ(ids, (std::vector<int64_t>{1, 2}));
}

TEST(MockQueryResultTest, NextBlockSplitsRowsAcrossBlocks) {
  auto result = MockQueryResult::builder()
                    .column("id", MYSQL_TYPE_LONG)
                    .row({1})
                    .nextBlock()
                    .row({2})
                    .buildQueryResult();

  EXPECT_EQ(result.rows().size(), 2);
  EXPECT_EQ(result.numRows(), 2);
}

TEST(MockQueryResultTest, BuildsDbQueryResult) {
  auto result = MockQueryResult::builder()
                    .column("id", MYSQL_TYPE_LONG)
                    .row({1})
                    .numQueriesExecuted(2)
                    .resultSize(64)
                    .buildDbQueryResult();

  EXPECT_TRUE(result.ok());
  EXPECT_EQ(result.numQueriesExecuted(), 2);
  EXPECT_EQ(result.resultSize(), 64);
  EXPECT_EQ(result.queryResult().numRows(), 1);
}

TEST(MockQueryResultTest, FailedOperationResultPropagatesToDbQueryResult) {
  auto result = MockQueryResult::builder()
                    .column("id", MYSQL_TYPE_LONG)
                    .operationResult(OperationResult::Failed)
                    .buildDbQueryResult();

  EXPECT_FALSE(result.ok());
  EXPECT_EQ(result.operationResult(), OperationResult::Failed);
  EXPECT_FALSE(result.queryResult().succeeded());
}

TEST(MockQueryResultTest, BuilderWithoutCharsetsOmitsThem) {
  auto withCharsets =
      MockQueryResult::builder().column("id", MYSQL_TYPE_LONG).buildRowFields();
  EXPECT_TRUE(withCharsets->hasFieldCharsetnrs());

  auto without = MockQueryResult::builderWithoutCharsets()
                     .column("id", MYSQL_TYPE_LONG)
                     .buildRowFields();
  EXPECT_FALSE(without->hasFieldCharsetnrs());
}

TEST(MockQueryResultTest, WrongColumnCountThrows) {
  auto builder = MockQueryResult::builder();
  builder.column("id", MYSQL_TYPE_LONG).column("name", MYSQL_TYPE_VARCHAR);

  EXPECT_THROW(builder.row({1}), std::out_of_range);
  EXPECT_THROW(builder.row({1, "alice", "extra"}), std::out_of_range);
}

TEST(MockQueryResultTest, ColumnAfterRowThrows) {
  auto builder = MockQueryResult::builder();
  builder.column("id", MYSQL_TYPE_LONG).row({1});

  EXPECT_THROW(builder.column("late", MYSQL_TYPE_LONG), std::logic_error);
}

TEST(MockQueryResultTest, BuildRowBlockRejectsMultipleBlocks) {
  auto builder = MockQueryResult::builder();
  builder.column("id", MYSQL_TYPE_LONG).row({1}).nextBlock().row({2});

  EXPECT_THROW(builder.buildRowBlock(), std::logic_error);
}

// The schema alone is a valid result: a query can legitimately match no rows.
TEST(MockQueryResultTest, ZeroRowsIsValid) {
  auto result = MockQueryResult::builder()
                    .column("id", MYSQL_TYPE_LONG)
                    .buildQueryResult();

  EXPECT_EQ(result.numRows(), 0);
  EXPECT_EQ(result.begin(), result.end());
}

TEST(MockQueryResultTest, NextBlockOnEmptyBlockIsNoOp) {
  // Called before any row, and again between rows, it must not leave an empty
  // block behind for buildRowBlock() to trip over.
  auto builder = MockQueryResult::builder();
  builder.column("id", MYSQL_TYPE_LONG);
  builder.nextBlock();
  builder.row({1});
  builder.nextBlock().nextBlock();
  builder.row({2});

  auto result = builder.buildQueryResult();
  EXPECT_EQ(result.rows().size(), 2);
  EXPECT_EQ(result.numRows(), 2);
}

TEST(MockQueryResultTest, NextBlockBeforeAnyRowStillYieldsASingleBlock) {
  auto block = MockQueryResult::builder()
                   .column("id", MYSQL_TYPE_LONG)
                   .nextBlock()
                   .row({1})
                   .buildRowBlock();

  EXPECT_EQ(block.numRows(), 1);
}

TEST(MockQueryResultTest, BuilderIsSingleUse) {
  auto builder = MockQueryResult::builder();
  builder.column("id", MYSQL_TYPE_LONG).row({1});
  auto block = builder.buildRowBlock();
  ASSERT_EQ(block.numRows(), 1);

  // The rows were moved out; reusing the builder would hand back a moved-from
  // block or append to a vector that no longer owns them.
  EXPECT_THROW(builder.buildRowBlock(), std::logic_error);
  EXPECT_THROW(builder.buildQueryResult(), std::logic_error);
  EXPECT_THROW(builder.buildDbQueryResult(), std::logic_error);
  EXPECT_THROW(builder.row({2}), std::logic_error);
  EXPECT_THROW(builder.nextBlock(), std::logic_error);
  EXPECT_THROW(builder.column("late", MYSQL_TYPE_LONG), std::logic_error);
  // The setters would otherwise be silent no-ops: they mutate state the
  // consumed builder will never read again.
  EXPECT_THROW(builder.rowsMatched(1), std::logic_error);
  EXPECT_THROW(builder.lastInsertId(1), std::logic_error);
  EXPECT_THROW(builder.numRowsAffected(1), std::logic_error);
  EXPECT_THROW(builder.queryNum(1), std::logic_error);
  EXPECT_THROW(
      builder.operationResult(OperationResult::Failed), std::logic_error);
  EXPECT_THROW(builder.numQueriesExecuted(1), std::logic_error);
  EXPECT_THROW(builder.resultSize(1), std::logic_error);
  EXPECT_THROW(builder.connectionKey(nullptr), std::logic_error);
}

TEST(MockQueryResultTest, BuildQueryResultIsAlsoSingleUse) {
  auto builder = MockQueryResult::builder();
  builder.column("id", MYSQL_TYPE_LONG).row({1});
  auto result = builder.buildQueryResult();
  ASSERT_EQ(result.numRows(), 1);

  EXPECT_THROW(builder.buildQueryResult(), std::logic_error);
}

TEST(MockQueryResultTest, ColumnAfterAnySealingCallThrows) {
  // Sealed by row()...
  auto byRow = MockQueryResult::builder();
  byRow.column("id", MYSQL_TYPE_LONG).row({1});
  EXPECT_THROW(byRow.column("late", MYSQL_TYPE_LONG), std::logic_error);

  // ...by nextBlock()...
  auto byNextBlock = MockQueryResult::builder();
  byNextBlock.column("id", MYSQL_TYPE_LONG).nextBlock();
  EXPECT_THROW(byNextBlock.column("late", MYSQL_TYPE_LONG), std::logic_error);

  // ...and by buildRowFields(), which does not consume the builder.
  auto byBuildFields = MockQueryResult::builder();
  byBuildFields.column("id", MYSQL_TYPE_LONG);
  auto fields = byBuildFields.buildRowFields();
  ASSERT_EQ(fields->numFields(), 1);
  EXPECT_THROW(byBuildFields.column("late", MYSQL_TYPE_LONG), std::logic_error);
}

// A trailing nextBlock() leaves an empty block behind; buildRowBlock() drops
// it rather than reporting two blocks, matching buildQueryResult().
TEST(MockQueryResultTest, BuildRowBlockIgnoresTrailingEmptyBlock) {
  auto builder = MockQueryResult::builder();
  builder.column("id", MYSQL_TYPE_LONG).row({1}).nextBlock();

  auto block = builder.buildRowBlock();
  EXPECT_EQ(block.numRows(), 1);
  EXPECT_EQ(block.getField<int64_t>(0, "id"), 1);
}

// A result with no rows at all still yields a usable empty block.
TEST(MockQueryResultTest, BuildRowBlockWithNoRowsYieldsEmptyBlock) {
  auto block =
      MockQueryResult::builder().column("id", MYSQL_TYPE_LONG).buildRowBlock();

  EXPECT_EQ(block.numRows(), 0);
  EXPECT_EQ(block.numFields(), 1);
}

// Rejecting a build must not consume the builder -- the caller has not
// received anything, so they can still recover.
TEST(MockQueryResultTest, RejectedBuildRowBlockLeavesBuilderUsable) {
  auto builder = MockQueryResult::builder();
  builder.column("id", MYSQL_TYPE_LONG).row({1}).nextBlock().row({2});

  EXPECT_THROW(builder.buildRowBlock(), std::logic_error);

  auto result = builder.buildQueryResult();
  EXPECT_EQ(result.numRows(), 2);
}

// buildRowFields() hands back the schema without consuming the builder.
TEST(MockQueryResultTest, BuildRowFieldsDoesNotConsumeTheBuilder) {
  auto builder = MockQueryResult::builder();
  builder.column("id", MYSQL_TYPE_LONG);

  auto first = builder.buildRowFields();
  auto second = builder.buildRowFields();
  EXPECT_EQ(first, second); // the same shared schema, not a rebuild

  builder.row({1});
  auto result = builder.buildQueryResult();
  EXPECT_EQ(result.numRows(), 1);
  EXPECT_EQ(result.getSharedRowFields(), first);
}

TEST(MockQueryResultTest, ConnectionKeyDefaultsWhenUnset) {
  auto result = MockQueryResult::builder()
                    .column("id", MYSQL_TYPE_LONG)
                    .row({1})
                    .buildDbQueryResult();

  ASSERT_NE(result.getConnectionKey(), nullptr);
}

TEST(MockQueryResultTest, ConnectionKeyIsUsedWhenSupplied) {
  auto key = std::make_shared<const MysqlConnectionKey>(
      "myhost", 3306, "mydb", "myuser", "");
  auto result = MockQueryResult::builder()
                    .column("id", MYSQL_TYPE_LONG)
                    .row({1})
                    .connectionKey(key)
                    .buildDbQueryResult();

  EXPECT_EQ(result.getConnectionKey(), key);
}

} // namespace facebook::common::mysql_client::test
