/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include "squangle/mysql_client/test/MockInternalConnection.h"
#include "squangle/mysql_client/test/MockInternalResult.h"
#include "squangle/mysql_client/test/MockInternalRow.h"
#include "squangle/mysql_client/test/MockInternalRowMetadata.h"

namespace facebook::common::mysql_client::test {

// =============================================================================
// MockInternalRow Tests
// =============================================================================

TEST(MockInternalRowTest, StringColumn) {
  std::vector<MockColumnValue> values = {std::string("hello")};
  MockInternalRow row(std::move(values));

  EXPECT_EQ(row.columnString(0), "hello");
  EXPECT_EQ(row.columnType(0), InternalRow::Type::String);
  EXPECT_EQ(row.columnLength(0), 5);
}

TEST(MockInternalRowTest, IntColumn) {
  std::vector<MockColumnValue> values = {int64_t{42}};
  MockInternalRow row(std::move(values));

  EXPECT_EQ(row.columnInt64(0), 42);
  EXPECT_EQ(row.columnType(0), InternalRow::Type::Int64);
}

TEST(MockInternalRowTest, MultipleColumns) {
  std::vector<MockColumnValue> values = {
      std::string("John"),
      int64_t{25},
      double{3.14},
      true,
  };
  MockInternalRow row(std::move(values));

  EXPECT_EQ(row.columnString(0), "John");
  EXPECT_EQ(row.columnInt64(1), 25);
  EXPECT_DOUBLE_EQ(row.columnDouble(2), 3.14);
  EXPECT_EQ(row.columnBool(3), true);
}

TEST(MockInternalRowTest, NullColumn) {
  std::vector<MockColumnValue> values = {std::monostate{}};
  MockInternalRow row(std::move(values));

  EXPECT_EQ(row.columnType(0), InternalRow::Type::Null);
}

TEST(MockInternalRowTest, OutOfBoundsReturnsDefaults) {
  std::vector<MockColumnValue> values = {std::string("only one")};
  MockInternalRow row(std::move(values));

  EXPECT_EQ(row.columnString(99), "");
  EXPECT_EQ(row.columnInt64(99), 0);
  EXPECT_EQ(row.columnType(99), InternalRow::Type::Null);
}

// =============================================================================
// MockInternalRowMetadata Tests
// =============================================================================

TEST(MockInternalRowMetadataTest, FieldInfo) {
  std::vector<MockFieldInfo> fields = {
      {"id", "users", MYSQL_TYPE_LONG, 0},
      {"name", "users", MYSQL_TYPE_STRING, 0},
      {"email", "users", MYSQL_TYPE_STRING, 0},
  };
  MockInternalRowMetadata metadata(std::move(fields));

  EXPECT_EQ(metadata.numFields(), 3);
  EXPECT_EQ(metadata.getFieldName(0), "id");
  EXPECT_EQ(metadata.getFieldName(1), "name");
  EXPECT_EQ(metadata.getFieldName(2), "email");
  EXPECT_EQ(metadata.getTableName(0), "users");
  EXPECT_EQ(metadata.getFieldType(0), MYSQL_TYPE_LONG);
  EXPECT_EQ(metadata.getFieldType(1), MYSQL_TYPE_STRING);
}

// =============================================================================
// MockInternalResult Tests
// =============================================================================

TEST(MockInternalResultTest, IterateRows) {
  std::vector<std::vector<MockColumnValue>> rows = {
      {std::string("Alice"), int64_t{30}},
      {std::string("Bob"), int64_t{25}},
  };
  MockInternalResult result(std::move(rows));

  EXPECT_EQ(result.numRows(), 2);

  // Fetch first row
  auto [status1, row1] = result.fetchRow();
  EXPECT_EQ(status1, InternalStatus::DONE);
  ASSERT_NE(row1, nullptr);
  EXPECT_EQ(row1->columnString(0), "Alice");
  EXPECT_EQ(row1->columnInt64(1), 30);

  // Fetch second row
  auto [status2, row2] = result.fetchRow();
  EXPECT_EQ(status2, InternalStatus::DONE);
  ASSERT_NE(row2, nullptr);
  EXPECT_EQ(row2->columnString(0), "Bob");

  // No more rows
  auto [status3, row3] = result.fetchRow();
  EXPECT_EQ(status3, InternalStatus::DONE);
  EXPECT_EQ(row3, nullptr);
}

TEST(MockInternalResultTest, EmptyResult) {
  std::vector<std::vector<MockColumnValue>> rows = {};
  MockInternalResult result(std::move(rows));

  EXPECT_EQ(result.numRows(), 0);

  auto [status, row] = result.fetchRow();
  EXPECT_EQ(status, InternalStatus::DONE);
  EXPECT_EQ(row, nullptr);
}

TEST(MockInternalResultTest, CloseStopsFetching) {
  std::vector<std::vector<MockColumnValue>> rows = {
      {std::string("data")},
  };
  MockInternalResult result(std::move(rows));

  result.close();

  auto [status, row] = result.fetchRow();
  EXPECT_EQ(row, nullptr);
}

// =============================================================================
// MockPendingResult Tests (Async simulation)
// =============================================================================

TEST(MockPendingResultTest, SimulatesPendingStatus) {
  std::vector<std::vector<MockColumnValue>> rows = {
      {std::string("row1")},
      {std::string("row2")},
  };
  MockPendingResult result(std::move(rows), 2); // 2 PENDING calls per row

  // First row: 2 PENDING, then DONE
  auto [s1, r1] = result.fetchRow();
  EXPECT_EQ(s1, InternalStatus::PENDING);
  EXPECT_EQ(r1, nullptr);

  auto [s2, r2] = result.fetchRow();
  EXPECT_EQ(s2, InternalStatus::PENDING);
  EXPECT_EQ(r2, nullptr);

  auto [s3, r3] = result.fetchRow();
  EXPECT_EQ(s3, InternalStatus::DONE);
  ASSERT_NE(r3, nullptr);
  EXPECT_EQ(r3->columnString(0), "row1");

  // Second row: 2 PENDING, then DONE
  auto [s4, r4] = result.fetchRow();
  EXPECT_EQ(s4, InternalStatus::PENDING);

  auto [s5, r5] = result.fetchRow();
  EXPECT_EQ(s5, InternalStatus::PENDING);

  auto [s6, r6] = result.fetchRow();
  EXPECT_EQ(s6, InternalStatus::DONE);
  ASSERT_NE(r6, nullptr);
  EXPECT_EQ(r6->columnString(0), "row2");
}

// =============================================================================
// MockInternalConnection Tests
// =============================================================================

TEST(MockInternalConnectionTest, DefaultConfig) {
  MockInternalConnection conn;

  EXPECT_EQ(conn.getErrno(), 0);
  EXPECT_EQ(conn.getErrorMessage(), "");
  EXPECT_EQ(conn.serverInfo(), "MockMySQLServer 8.0.0");
  EXPECT_TRUE(conn.ping());
}

TEST(MockInternalConnectionTest, ErrorInjection) {
  MockInternalConnection conn;
  conn.setError(1045, "Access denied");

  EXPECT_EQ(conn.getErrno(), 1045);
  EXPECT_EQ(conn.getErrorMessage(), "Access denied");
  EXPECT_FALSE(conn.ping());

  conn.clearError();
  EXPECT_EQ(conn.getErrno(), 0);
  EXPECT_TRUE(conn.ping());
}

TEST(MockInternalConnectionTest, QueryResultLookup) {
  MockConnectionConfig config;
  config.queryResults["SELECT 1"] = MockQueryResult{
      .fields = {{"1", "", MYSQL_TYPE_LONG, 0}},
      .rows = {{int64_t{1}}},
      .lastInsertId = 0,
      .affectedRows = 0,
      .warningCount = 0,
      .recvGtid = std::nullopt,
      .info = std::nullopt,
  };

  MockInternalConnection conn(std::move(config));

  auto* result = conn.runQuery("SELECT 1");
  ASSERT_NE(result, nullptr);
  EXPECT_EQ(result->rows.size(), 1);

  auto mockResult = conn.getResult();
  ASSERT_NE(mockResult, nullptr);
  auto [status, row] = mockResult->fetchRow();
  EXPECT_EQ(row->columnInt64(0), 1);
}

TEST(MockInternalConnectionTest, QueryNotFound) {
  MockInternalConnection conn;

  auto* result = conn.runQuery("SELECT unknown");
  EXPECT_EQ(result, nullptr);
}

TEST(MockInternalConnectionTest, DefaultResult) {
  MockConnectionConfig config;
  config.defaultResult = MockQueryResult{
      .fields = {{"default", "", MYSQL_TYPE_STRING, 0}},
      .rows = {{std::string("default value")}},
      .lastInsertId = 0,
      .affectedRows = 0,
      .warningCount = 0,
      .recvGtid = std::nullopt,
      .info = std::nullopt,
  };

  MockInternalConnection conn(std::move(config));

  auto* result = conn.runQuery("ANY QUERY");
  ASSERT_NE(result, nullptr);

  auto mockResult = conn.getResult();
  auto [status, row] = mockResult->fetchRow();
  EXPECT_EQ(row->columnString(0), "default value");
}

TEST(MockInternalConnectionTest, QueryAttributes) {
  MockInternalConnection conn;

  AttributeMap attrs = {{"key1", "value1"}, {"key2", "value2"}};
  conn.setQueryAttributes(attrs);

  EXPECT_EQ(conn.lastQueryAttributes().size(), 2);
  EXPECT_EQ(conn.lastQueryAttributes().at("key1"), "value1");
}

TEST(MockInternalConnectionTest, TransactionState) {
  MockConnectionConfig config;
  config.inTransaction = true;

  MockInternalConnection conn(std::move(config));
  EXPECT_TRUE(conn.inTransaction());

  conn.config().inTransaction = false;
  EXPECT_FALSE(conn.inTransaction());
}

TEST(MockInternalConnectionTest, ResetAndChangeUser) {
  MockInternalConnection conn;

  EXPECT_EQ(conn.resetConn(), InternalConnection::Status::DONE);
  EXPECT_EQ(conn.changeUser(nullptr), InternalConnection::Status::DONE);

  conn.setError(1000, "error");
  EXPECT_EQ(conn.resetConn(), InternalConnection::Status::ERROR);
  EXPECT_EQ(conn.changeUser(nullptr), InternalConnection::Status::ERROR);
}

} // namespace facebook::common::mysql_client::test
