/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

//
// Error Recovery Tests
//
// Tests behaviors not covered by other test files in this stack:
// - Query routing: default vs specific match priority, misses not poisoning
//   the connection
// - RowStream close: partial consumption then close on production RowStream
// - Metadata replacement: subsequent queries replace (not accumulate) metadata
//

#include <gtest/gtest.h>

#include <vector>

#include "squangle/mysql_client/FetchOperation.h"
#include "squangle/mysql_client/test/MockInternalConnection.h"
#include "squangle/mysql_client/test/MockInternalResult.h"

namespace facebook::common::mysql_client::test {

// =============================================================================
// Query Routing
//
// Tests query dispatch logic: specific matches take priority over the default
// fallback, and a miss does not poison subsequent matches.
// =============================================================================

class QueryRoutingTest : public ::testing::Test {};

TEST_F(QueryRoutingTest, SpecificMatchTakesPriorityOverDefault) {
  MockConnectionConfig config;
  config.queryResults["SELECT special"] = MockQueryResult{
      .fields = {{"val", "", MYSQL_TYPE_STRING, 0}},
      .rows = {{std::string("special")}},
  };
  config.defaultResult = MockQueryResult{
      .fields = {{"val", "", MYSQL_TYPE_STRING, 0}},
      .rows = {{std::string("default")}},
  };

  MockInternalConnection conn(std::move(config));

  auto special = conn.runQuery("SELECT special");
  ASSERT_NE(special, nullptr);
  EXPECT_EQ(special->rows[0][0], MockColumnValue(std::string("special")));

  auto other = conn.runQuery("SELECT anything");
  ASSERT_NE(other, nullptr);
  EXPECT_EQ(other->rows[0][0], MockColumnValue(std::string("default")));
}

TEST_F(QueryRoutingTest, MissDoesNotAffectSubsequentMatches) {
  MockConnectionConfig config;
  config.queryResults["SELECT 1"] = MockQueryResult{
      .fields = {{"1", "", MYSQL_TYPE_LONG, 0}},
      .rows = {{int64_t{1}}},
  };

  MockInternalConnection conn(std::move(config));

  auto result = conn.runQuery("SELECT 1");
  ASSERT_NE(result, nullptr);

  // Miss — no matching query, no default
  result = conn.runQuery("SELECT 2");
  EXPECT_EQ(result, nullptr);

  // Original match still works — miss didn't poison the connection
  result = conn.runQuery("SELECT 1");
  EXPECT_NE(result, nullptr);
  EXPECT_EQ(conn.getErrno(), 0);
}

// =============================================================================
// RowStream Close
//
// Tests close on the production RowStream class after partial consumption.
// =============================================================================

class RowStreamCloseTest : public ::testing::Test {};

TEST_F(RowStreamCloseTest, PartialConsumptionThenClose) {
  std::vector<std::vector<MockColumnValue>> rows = {
      {int64_t{1}, std::string("Alice")},
      {int64_t{2}, std::string("Bob")},
      {int64_t{3}, std::string("Charlie")},
  };

  auto result = std::make_unique<MockInternalResult>(rows);
  auto metadata =
      std::make_unique<MockInternalRowMetadata>(std::vector<MockFieldInfo>{
          {"id", "t", MYSQL_TYPE_LONG, 0},
          {"name", "t", MYSQL_TYPE_STRING, 0},
      });
  RowStream stream(std::move(result), std::move(metadata));

  EXPECT_TRUE(stream.hasNext());
  auto row = stream.consumeRow();
  EXPECT_EQ(row.getString(1), "Alice");
  EXPECT_EQ(stream.numRowsSeen(), 1);

  stream.close();
  EXPECT_FALSE(stream.hasNext());
}

// =============================================================================
// Metadata Replacement Across Queries
//
// Tests that metadata is replaced per-query, not accumulated — each query's
// result overwrites the previous metadata values.
// =============================================================================

class MetadataUpdateTest : public ::testing::Test {};

TEST_F(MetadataUpdateTest, SubsequentQueryReplacesMetadata) {
  MockConnectionConfig config;
  config.queryResults["INSERT INTO t1 VALUES (1)"] = MockQueryResult{
      .lastInsertId = 100,
      .affectedRows = 1,
      .warningCount = 0,
      .recvGtid = "gtid-first",
  };
  config.queryResults["INSERT INTO t2 VALUES (1)"] = MockQueryResult{
      .lastInsertId = 200,
      .affectedRows = 3,
      .warningCount = 2,
      .recvGtid = "gtid-second",
  };

  MockInternalConnection conn(std::move(config));

  conn.runQuery("INSERT INTO t1 VALUES (1)");
  EXPECT_EQ(conn.getLastInsertId(), 100);
  EXPECT_EQ(conn.getAffectedRows(), 1);
  EXPECT_EQ(conn.warningCount(), 0);
  ASSERT_TRUE(conn.getRecvGtid().has_value());
  EXPECT_EQ(*conn.getRecvGtid(), "gtid-first");

  // Second query replaces all metadata from the first
  conn.runQuery("INSERT INTO t2 VALUES (1)");
  EXPECT_EQ(conn.getLastInsertId(), 200);
  EXPECT_EQ(conn.getAffectedRows(), 3);
  EXPECT_EQ(conn.warningCount(), 2);
  ASSERT_TRUE(conn.getRecvGtid().has_value());
  EXPECT_EQ(*conn.getRecvGtid(), "gtid-second");
}

} // namespace facebook::common::mysql_client::test
