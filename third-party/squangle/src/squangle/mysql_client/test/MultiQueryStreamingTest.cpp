/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

//
// Multi-Query Streaming Tests
//
// Tests streaming result handling, column type coverage, and RowStream API
// surface using mock infrastructure.
//
// - Streaming large result sets and early termination
// - All EphemeralRow column types: bool, int64, uint64, double, string, null
// - EphemeralRow API: getType, numFields, convertToString, convertTo<T>
// - RowStream API: getCurrentRow, numRows, getEphemeralRowFieldsShared, close
// - Null column handling and convertTo<T> exception on null
// - Name-based column access via convertTo<T>(colName)
// - Transaction-like batch operations with GTID
//

#include <gtest/gtest.h>

#include <vector>

#include "squangle/mysql_client/FetchOperation.h"
#include "squangle/mysql_client/test/MockInternalConnection.h"
#include "squangle/mysql_client/test/MockInternalResult.h"

namespace facebook::common::mysql_client::test {

// Helper to create a RowStream from rows and fields
static RowStream makeStream(
    std::vector<std::vector<MockColumnValue>> rows,
    std::vector<MockFieldInfo> fields) {
  auto result = std::make_unique<MockInternalResult>(std::move(rows));
  auto metadata = std::make_unique<MockInternalRowMetadata>(std::move(fields));
  return RowStream(std::move(result), std::move(metadata));
}

// =============================================================================
// Streaming Result Tests
// =============================================================================

class StreamingResultTest : public ::testing::Test {};

TEST_F(StreamingResultTest, StreamMultipleRows) {
  std::vector<std::vector<MockColumnValue>> rows;
  rows.reserve(10);
  for (int i = 0; i < 10; i++) {
    rows.push_back({int64_t{i}, std::string("row_" + std::to_string(i))});
  }

  auto stream = makeStream(
      std::move(rows),
      {{"id", "t", MYSQL_TYPE_LONG, 0}, {"name", "t", MYSQL_TYPE_STRING, 0}});

  int rowCount = 0;
  while (stream.hasNext()) {
    stream.consumeRow();
    rowCount++;
  }

  EXPECT_EQ(rowCount, 10);
  EXPECT_EQ(stream.numRowsSeen(), 10);
}

TEST_F(StreamingResultTest, StreamWithEarlyTermination) {
  std::vector<std::vector<MockColumnValue>> rows;
  rows.reserve(100);
  for (int i = 0; i < 100; i++) {
    rows.push_back({int64_t{i}});
  }

  auto stream = makeStream(std::move(rows), {{"id", "t", MYSQL_TYPE_LONG, 0}});

  int consumed = 0;
  while (stream.hasNext() && consumed < 10) {
    stream.consumeRow();
    consumed++;
  }

  EXPECT_EQ(consumed, 10);
  EXPECT_TRUE(stream.hasNext());
}

TEST_F(StreamingResultTest, CloseStopsStreamMidIteration) {
  std::vector<std::vector<MockColumnValue>> rows = {
      {int64_t{1}},
      {int64_t{2}},
      {int64_t{3}},
  };

  auto stream = makeStream(std::move(rows), {{"id", "t", MYSQL_TYPE_LONG, 0}});

  EXPECT_TRUE(stream.hasNext());
  stream.consumeRow();
  EXPECT_EQ(stream.numRowsSeen(), 1);

  stream.close();
  EXPECT_FALSE(stream.hasNext());
}

// =============================================================================
// RowStream API Tests
// =============================================================================

class RowStreamAPITest : public ::testing::Test {};

TEST_F(RowStreamAPITest, NumRowsReportsTotal) {
  std::vector<std::vector<MockColumnValue>> rows = {
      {int64_t{1}},
      {int64_t{2}},
      {int64_t{3}},
  };

  auto stream = makeStream(std::move(rows), {{"id", "t", MYSQL_TYPE_LONG, 0}});

  // numRows() returns the total count from InternalResult, not rows consumed
  EXPECT_EQ(stream.numRows(), 3);
  EXPECT_EQ(stream.numRowsSeen(), 0);

  stream.hasNext();
  stream.consumeRow();
  // numRows unchanged, numRowsSeen advances
  EXPECT_EQ(stream.numRows(), 3);
  EXPECT_EQ(stream.numRowsSeen(), 1);
}

TEST_F(RowStreamAPITest, GetCurrentRowWithoutConsuming) {
  std::vector<std::vector<MockColumnValue>> rows = {
      {int64_t{42}, std::string("peek")},
  };

  auto stream = makeStream(
      std::move(rows),
      {{"id", "t", MYSQL_TYPE_LONG, 0}, {"val", "t", MYSQL_TYPE_STRING, 0}});

  EXPECT_TRUE(stream.hasNext());

  // getCurrentRow() inspects without consuming
  const auto& currentRow = stream.getCurrentRow();
  ASSERT_TRUE(currentRow.has_value());
  EXPECT_EQ(currentRow->getInt64(0), 42);

  // Row is still available for consumeRow
  auto row = stream.consumeRow();
  EXPECT_EQ(row.getString(1), "peek");

  // After consume, getCurrentRow is empty
  const auto& afterConsume = stream.getCurrentRow();
  EXPECT_FALSE(afterConsume.has_value());
}

TEST_F(RowStreamAPITest, GetEphemeralRowFieldsShared) {
  std::vector<std::vector<MockColumnValue>> rows = {
      {int64_t{1}},
  };

  auto stream =
      makeStream(std::move(rows), {{"id", "users", MYSQL_TYPE_LONG, 0}});

  auto fields = stream.getEphemeralRowFieldsShared();
  ASSERT_NE(fields, nullptr);
  EXPECT_EQ(fields->numFields(), 1);
  EXPECT_EQ(fields->fieldName(0), "id");
}

TEST_F(RowStreamAPITest, QueryResultSizeAccumulates) {
  std::vector<std::vector<MockColumnValue>> rows = {
      {int64_t{1}, std::string("short")},
      {int64_t{2}, std::string("a much longer string value here")},
  };

  auto stream = makeStream(
      std::move(rows),
      {{"id", "t", MYSQL_TYPE_LONG, 0}, {"val", "t", MYSQL_TYPE_STRING, 0}});

  EXPECT_EQ(stream.queryResultSize(), 0);

  stream.hasNext();
  uint64_t sizeAfterFirst = stream.queryResultSize();
  EXPECT_GT(sizeAfterFirst, 0);
  stream.consumeRow();

  stream.hasNext();
  uint64_t sizeAfterSecond = stream.queryResultSize();
  EXPECT_GT(sizeAfterSecond, sizeAfterFirst);
}

// =============================================================================
// EphemeralRow Column Type Tests
//
// Exercises all MockColumnValue types through the EphemeralRow API.
// =============================================================================

class EphemeralRowTypeTest : public ::testing::Test {};

TEST_F(EphemeralRowTypeTest, AllColumnTypes) {
  std::vector<std::vector<MockColumnValue>> rows = {
      {bool{true},
       int64_t{-42},
       uint64_t{18446744073709551615ULL},
       double{3.14159},
       std::string("hello"),
       std::monostate{}},
  };

  auto stream = makeStream(
      std::move(rows),
      {{"flag", "t", MYSQL_TYPE_TINY, 0},
       {"signed_val", "t", MYSQL_TYPE_LONGLONG, 0},
       {"unsigned_val", "t", MYSQL_TYPE_LONGLONG, UNSIGNED_FLAG},
       {"price", "t", MYSQL_TYPE_DOUBLE, 0},
       {"name", "t", MYSQL_TYPE_STRING, 0},
       {"nullable", "t", MYSQL_TYPE_STRING, 0}});

  ASSERT_TRUE(stream.hasNext());
  auto row = stream.consumeRow();

  EXPECT_EQ(row.numFields(), 6);

  // Bool
  EXPECT_TRUE(row.getBool(0));
  EXPECT_EQ(row.getType(0), InternalRow::Type::Bool);

  // Int64
  EXPECT_EQ(row.getInt64(1), -42);
  EXPECT_EQ(row.getType(1), InternalRow::Type::Int64);

  // UInt64
  EXPECT_EQ(row.getUInt64(2), 18446744073709551615ULL);
  EXPECT_EQ(row.getType(2), InternalRow::Type::UInt64);

  // Double
  EXPECT_DOUBLE_EQ(row.getDouble(3), 3.14159);
  EXPECT_EQ(row.getType(3), InternalRow::Type::Double);

  // String
  EXPECT_EQ(row.getString(4), "hello");
  EXPECT_EQ(row.getType(4), InternalRow::Type::String);

  // Null
  EXPECT_TRUE(row.isNull(5));
  EXPECT_EQ(row.getType(5), InternalRow::Type::Null);
  EXPECT_FALSE(row.isNull(0));
}

TEST_F(EphemeralRowTypeTest, ConvertToString) {
  std::vector<std::vector<MockColumnValue>> rows = {
      {int64_t{42}, double{2.5}, std::string("already a string")},
  };

  auto stream = makeStream(
      std::move(rows),
      {{"i", "t", MYSQL_TYPE_LONG, 0},
       {"d", "t", MYSQL_TYPE_DOUBLE, 0},
       {"s", "t", MYSQL_TYPE_STRING, 0}});

  ASSERT_TRUE(stream.hasNext());
  auto row = stream.consumeRow();

  EXPECT_EQ(row.convertToString(0), "42");
  EXPECT_EQ(row.convertToString(2), "already a string");
}

TEST_F(EphemeralRowTypeTest, CalculateRowLength) {
  std::vector<std::vector<MockColumnValue>> rows = {
      {int64_t{1}, std::string("test")},
  };

  auto stream = makeStream(
      std::move(rows),
      {{"id", "t", MYSQL_TYPE_LONG, 0}, {"val", "t", MYSQL_TYPE_STRING, 0}});

  ASSERT_TRUE(stream.hasNext());
  auto row = stream.consumeRow();

  uint64_t len = row.calculateRowLength();
  // Should include int64 (8 bytes) + string "test" (4 bytes) = at least 12
  EXPECT_GE(len, 12);
}

// =============================================================================
// convertTo<T> Tests
// =============================================================================

class ConvertToTest : public ::testing::Test {};

TEST_F(ConvertToTest, ConvertToByIndex) {
  std::vector<std::vector<MockColumnValue>> rows = {
      {int64_t{42}, std::string("hello")},
  };

  auto stream = makeStream(
      std::move(rows),
      {{"id", "t", MYSQL_TYPE_LONG, 0}, {"name", "t", MYSQL_TYPE_STRING, 0}});

  ASSERT_TRUE(stream.hasNext());
  auto row = stream.consumeRow();

  // int64 → string
  auto idStr = row.convertTo<std::string>(0);
  EXPECT_EQ(idStr, "42");

  // string → string (identity)
  auto name = row.convertTo<std::string>(1);
  EXPECT_EQ(name, "hello");
}

TEST_F(ConvertToTest, ConvertToByColumnName) {
  std::vector<std::vector<MockColumnValue>> rows = {
      {int64_t{99}, std::string("Alice")},
  };

  auto stream = makeStream(
      std::move(rows),
      {{"id", "users", MYSQL_TYPE_LONG, 0},
       {"name", "users", MYSQL_TYPE_STRING, 0}});

  ASSERT_TRUE(stream.hasNext());
  auto row = stream.consumeRow();

  EXPECT_EQ(row.convertTo<std::string>("id"), "99");
  EXPECT_EQ(row.convertTo<std::string>("name"), "Alice");
}

TEST_F(ConvertToTest, ConvertToThrowsOnNull) {
  std::vector<std::vector<MockColumnValue>> rows = {
      {std::monostate{}},
  };

  auto stream =
      makeStream(std::move(rows), {{"val", "t", MYSQL_TYPE_STRING, 0}});

  ASSERT_TRUE(stream.hasNext());
  auto row = stream.consumeRow();

  EXPECT_THROW(row.convertTo<std::string>(0), std::runtime_error);
}

TEST_F(ConvertToTest, ConvertToByNameThrowsOnBadColumn) {
  std::vector<std::vector<MockColumnValue>> rows = {
      {int64_t{1}},
  };

  auto stream = makeStream(std::move(rows), {{"id", "t", MYSQL_TYPE_LONG, 0}});

  ASSERT_TRUE(stream.hasNext());
  auto row = stream.consumeRow();

  EXPECT_THROW(row.convertTo<std::string>("nonexistent"), std::out_of_range);
}

// =============================================================================
// Null Handling Tests
// =============================================================================

class NullHandlingTest : public ::testing::Test {};

TEST_F(NullHandlingTest, MixedNullAndNonNull) {
  std::vector<std::vector<MockColumnValue>> rows = {
      {int64_t{1}, std::string("Alice")},
      {int64_t{2}, std::monostate{}},
      {int64_t{3}, std::string("Charlie")},
  };

  auto stream = makeStream(
      std::move(rows),
      {{"id", "t", MYSQL_TYPE_LONG, 0}, {"name", "t", MYSQL_TYPE_STRING, 0}});

  int nullCount = 0;
  int rowCount = 0;

  while (stream.hasNext()) {
    auto row = stream.consumeRow();
    rowCount++;
    if (row.isNull(1)) {
      nullCount++;
    }
  }

  EXPECT_EQ(rowCount, 3);
  EXPECT_EQ(nullCount, 1);
}

TEST_F(NullHandlingTest, AllNullRow) {
  std::vector<std::vector<MockColumnValue>> rows = {
      {std::monostate{}, std::monostate{}, std::monostate{}},
  };

  auto stream = makeStream(
      std::move(rows),
      {{"a", "t", MYSQL_TYPE_LONG, 0},
       {"b", "t", MYSQL_TYPE_STRING, 0},
       {"c", "t", MYSQL_TYPE_DOUBLE, 0}});

  ASSERT_TRUE(stream.hasNext());
  auto row = stream.consumeRow();

  EXPECT_TRUE(row.isNull(0));
  EXPECT_TRUE(row.isNull(1));
  EXPECT_TRUE(row.isNull(2));
  EXPECT_EQ(row.numFields(), 3);
}

// =============================================================================
// Transaction Batch Tests
// =============================================================================

class TransactionBatchTest : public ::testing::Test {};

TEST_F(TransactionBatchTest, BeginCommitWithGtid) {
  MockConnectionConfig config;
  config.defaultResult = MockQueryResult{};
  config.queryResults["COMMIT"] = MockQueryResult{
      .recvGtid = "gtid-transaction-123",
  };

  MockInternalConnection conn(std::move(config));

  EXPECT_NE(conn.runQuery("BEGIN"), nullptr);
  EXPECT_NE(conn.runQuery("INSERT INTO t VALUES (1)"), nullptr);
  EXPECT_NE(conn.runQuery("COMMIT"), nullptr);

  auto gtid = conn.getRecvGtid();
  ASSERT_TRUE(gtid.has_value());
  EXPECT_EQ(*gtid, "gtid-transaction-123");
}

TEST_F(TransactionBatchTest, BatchInsertsTrackLastInsertId) {
  MockConnectionConfig config;
  for (int i = 0; i < 5; i++) {
    auto query = "INSERT INTO t VALUES (" + std::to_string(i) + ")";
    config.queryResults[query] = MockQueryResult{
        .lastInsertId = static_cast<uint64_t>(100 + i),
        .affectedRows = 1,
    };
  }

  MockInternalConnection conn(std::move(config));

  for (int i = 0; i < 5; i++) {
    auto query = "INSERT INTO t VALUES (" + std::to_string(i) + ")";
    conn.runQuery(query);
    EXPECT_EQ(conn.getLastInsertId(), static_cast<uint64_t>(100 + i));
    EXPECT_EQ(conn.getAffectedRows(), 1);
  }
}

} // namespace facebook::common::mysql_client::test
