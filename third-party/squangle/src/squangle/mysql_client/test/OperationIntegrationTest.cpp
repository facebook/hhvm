/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

//
// Operation Integration Tests
//
// Tests RowStream, pending result simulation, and query attribute persistence.
// Basic mock behavior (error injection, query attributes, transaction state,
// reset, ping, server info) is covered in MockTest.cpp (D94587045).
//

#include <gtest/gtest.h>

#include "squangle/mysql_client/FetchOperation.h"
#include "squangle/mysql_client/Row.h"
#include "squangle/mysql_client/test/MockInternalConnection.h"
#include "squangle/mysql_client/test/MockInternalResult.h"

namespace facebook::common::mysql_client::test {

// =============================================================================
// RowStream Tests
//
// Tests the RowStream class which wraps InternalResult for row iteration.
// =============================================================================

class RowStreamTest : public ::testing::Test {};

TEST_F(RowStreamTest, BasicRowIteration) {
  // Create mock result with 3 rows
  std::vector<std::vector<MockColumnValue>> rows = {
      {int64_t{1}, std::string("Alice")},
      {int64_t{2}, std::string("Bob")},
      {int64_t{3}, std::string("Charlie")},
  };

  auto mockResult = std::make_unique<MockInternalResult>(rows);

  // Create mock metadata
  auto mockMetadata =
      std::make_unique<MockInternalRowMetadata>(std::vector<MockFieldInfo>{
          {"id", "users", MYSQL_TYPE_LONG, 0},
          {"name", "users", MYSQL_TYPE_STRING, 0},
      });

  // Create RowStream
  RowStream stream(std::move(mockResult), std::move(mockMetadata));

  // Verify we can iterate through rows
  std::vector<std::string> names;
  while (stream.hasNext()) {
    EphemeralRow row = stream.consumeRow();
    names.emplace_back(row.getString(1));
  }

  const std::vector<std::string> expected{"Alice", "Bob", "Charlie"};
  EXPECT_EQ(names, expected);
}

TEST_F(RowStreamTest, EmptyResult) {
  std::vector<std::vector<MockColumnValue>> rows = {};
  auto mockResult = std::make_unique<MockInternalResult>(rows);
  auto mockMetadata = std::make_unique<MockInternalRowMetadata>(
      std::vector<MockFieldInfo>{{"id", "test", MYSQL_TYPE_LONG, 0}});

  RowStream stream(std::move(mockResult), std::move(mockMetadata));

  EXPECT_FALSE(stream.hasNext());
  EXPECT_EQ(stream.numRowsSeen(), 0);
}

TEST_F(RowStreamTest, RowFieldsAccess) {
  std::vector<std::vector<MockColumnValue>> rows = {
      {int64_t{1}, std::string("test")},
  };

  auto mockResult = std::make_unique<MockInternalResult>(rows);
  auto mockMetadata =
      std::make_unique<MockInternalRowMetadata>(std::vector<MockFieldInfo>{
          {"id", "users", MYSQL_TYPE_LONG, 0},
          {"name", "users", MYSQL_TYPE_STRING, 0},
      });

  RowStream stream(std::move(mockResult), std::move(mockMetadata));

  auto* rowFields = stream.getEphemeralRowFields();
  ASSERT_NE(rowFields, nullptr);
  EXPECT_EQ(rowFields->numFields(), 2);
  EXPECT_EQ(rowFields->fieldName(0), "id");
  EXPECT_EQ(rowFields->fieldName(1), "name");
}

TEST_F(RowStreamTest, QueryResultSize) {
  std::vector<std::vector<MockColumnValue>> rows = {
      {int64_t{1}, std::string("short")},
      {int64_t{2}, std::string("medium length string")},
      {int64_t{3}, std::string("this is a much longer string for testing")},
  };

  auto mockResult = std::make_unique<MockInternalResult>(rows);
  auto mockMetadata =
      std::make_unique<MockInternalRowMetadata>(std::vector<MockFieldInfo>{
          {"id", "test", MYSQL_TYPE_LONG, 0},
          {"data", "test", MYSQL_TYPE_STRING, 0},
      });

  RowStream stream(std::move(mockResult), std::move(mockMetadata));

  // Consume all rows to trigger size calculation
  while (stream.hasNext()) {
    stream.consumeRow();
  }

  EXPECT_EQ(stream.numRowsSeen(), 3);
  EXPECT_GT(stream.queryResultSize(), 0);
}

// =============================================================================
// Pending Result Tests (Async Simulation)
//
// Tests MockPendingResult which simulates async behavior.
// =============================================================================

class PendingResultTest : public ::testing::Test {};

TEST_F(PendingResultTest, MultiplePendingCycles) {
  std::vector<std::vector<MockColumnValue>> rows = {
      {std::string("row1")},
      {std::string("row2")},
  };

  // 1 pending status before each row
  auto pendingResult = std::make_unique<MockPendingResult>(rows, 1);

  std::vector<std::string> results;
  int pendingCount = 0;

  while (true) {
    auto [status, row] = pendingResult->fetchRow();
    if (status == InternalStatus::PENDING) {
      pendingCount++;
      continue;
    }
    if (!row) {
      break;
    }
    results.emplace_back(row->columnString(0));
  }

  EXPECT_EQ(pendingCount, 2); // 1 pending per row
  const std::vector<std::string> expected{"row1", "row2"};
  EXPECT_EQ(results, expected);
}

// =============================================================================
// Query Attribute Persistence Tests
// =============================================================================

class QueryAttributeTest : public ::testing::Test {};

TEST_F(QueryAttributeTest, AttributesPersistedAcrossQueries) {
  MockConnectionConfig config;
  config.queryResults["SELECT 1"] = MockQueryResult{
      .fields = {{"val", "test", MYSQL_TYPE_LONG, 0}},
      .rows = {{int64_t{1}}},
  };

  MockInternalConnection conn(std::move(config));

  // Set attributes
  conn.setQueryAttribute("session_id", "sess-789");

  // Run query
  conn.runQuery("SELECT 1");

  // Attributes should still be accessible
  EXPECT_EQ(conn.lastQueryAttributes().at("session_id"), "sess-789");
}

// =============================================================================
// Connection Metadata Tests
//
// Tests query result metadata (GTID, MySQL info) via mock execution.
// Basic mock behavior (server info, error injection, reset, ping) is
// covered in MockTest.cpp (D94587045).
// =============================================================================

class ConnectionMetadataTest : public ::testing::Test {};

TEST_F(ConnectionMetadataTest, RecvGtid) {
  MockConnectionConfig config;
  config.queryResults["COMMIT"] = MockQueryResult{
      .recvGtid = "3E11FA47-71CA-11E1-9E33-C80AA9429562:23",
  };

  MockInternalConnection conn(std::move(config));
  conn.runQuery("COMMIT");

  auto gtid = conn.getRecvGtid();
  EXPECT_TRUE(gtid.has_value());
  EXPECT_EQ(*gtid, "3E11FA47-71CA-11E1-9E33-C80AA9429562:23");
}

TEST_F(ConnectionMetadataTest, MySQLInfo) {
  MockConnectionConfig config;
  config.queryResults["UPDATE users SET active = 1"] = MockQueryResult{
      .affectedRows = 10,
      .info = "Rows matched: 15  Changed: 10  Warnings: 0",
  };

  MockInternalConnection conn(std::move(config));
  conn.runQuery("UPDATE users SET active = 1");

  auto info = conn.getMySQLInfo();
  EXPECT_TRUE(info.has_value());
  EXPECT_EQ(*info, "Rows matched: 15  Changed: 10  Warnings: 0");
}

} // namespace facebook::common::mysql_client::test
