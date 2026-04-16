/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

//
// Operation State Machine Tests
//
// These tests validate the state transitions and flow control in
// FetchOperation, QueryOperation, and related classes using mock data.
//

#include <gtest/gtest.h>

#include "squangle/mysql_client/DbResult.h"
#include "squangle/mysql_client/FetchOperation.h"
#include "squangle/mysql_client/test/MockInternalConnection.h"
#include "squangle/mysql_client/test/MockInternalResult.h"

namespace facebook::common::mysql_client::test {

// =============================================================================
// OperationResult Tests
//
// Tests OperationResult conversion behavior.
// =============================================================================

class OperationResultTest : public ::testing::Test {};

TEST_F(OperationResultTest, FailureReasonConversion) {
  // Each failure type maps to a distinct FailureReason
  auto failedReason = operationResultToFailureReason(OperationResult::Failed);
  auto cancelledReason =
      operationResultToFailureReason(OperationResult::Cancelled);
  auto timedOutReason =
      operationResultToFailureReason(OperationResult::TimedOut);

  EXPECT_NE(failedReason, cancelledReason);
  EXPECT_NE(cancelledReason, timedOutReason);
  EXPECT_NE(failedReason, timedOutReason);
}

// =============================================================================
// RowStream State Tests
//
// Tests RowStream state management during row iteration.
// =============================================================================

class RowStreamStateTest : public ::testing::Test {};

TEST_F(RowStreamStateTest, InitialStateIsNotFinished) {
  std::vector<std::vector<MockColumnValue>> rows = {
      {int64_t{1}},
  };
  auto mockResult = std::make_unique<MockInternalResult>(rows);
  auto mockMetadata = std::make_unique<MockInternalRowMetadata>(
      std::vector<MockFieldInfo>{{"id", "t", MYSQL_TYPE_LONG, 0}});

  RowStream stream(std::move(mockResult), std::move(mockMetadata));

  // Initially should have rows available
  EXPECT_TRUE(stream.hasNext());
  // Note: hasNext() fetches the first row, so numRowsSeen is already 1
  EXPECT_EQ(stream.numRowsSeen(), 1);
}

TEST_F(RowStreamStateTest, StateTransitionsToFinishedAfterAllRows) {
  std::vector<std::vector<MockColumnValue>> rows = {
      {int64_t{1}},
      {int64_t{2}},
  };
  auto mockResult = std::make_unique<MockInternalResult>(rows);
  auto mockMetadata = std::make_unique<MockInternalRowMetadata>(
      std::vector<MockFieldInfo>{{"id", "t", MYSQL_TYPE_LONG, 0}});

  RowStream stream(std::move(mockResult), std::move(mockMetadata));

  // Consume first row
  EXPECT_TRUE(stream.hasNext());
  stream.consumeRow();
  EXPECT_EQ(stream.numRowsSeen(), 1);

  // Consume second row
  EXPECT_TRUE(stream.hasNext());
  stream.consumeRow();
  EXPECT_EQ(stream.numRowsSeen(), 2);

  // No more rows
  EXPECT_FALSE(stream.hasNext());
}

TEST_F(RowStreamStateTest, EmptyResultIsImmediatelyFinished) {
  std::vector<std::vector<MockColumnValue>> rows = {};
  auto mockResult = std::make_unique<MockInternalResult>(rows);
  auto mockMetadata = std::make_unique<MockInternalRowMetadata>(
      std::vector<MockFieldInfo>{{"id", "t", MYSQL_TYPE_LONG, 0}});

  RowStream stream(std::move(mockResult), std::move(mockMetadata));

  EXPECT_FALSE(stream.hasNext());
  EXPECT_EQ(stream.numRowsSeen(), 0);
}

TEST_F(RowStreamStateTest, NumRowsSeenIncrementsCorrectly) {
  std::vector<std::vector<MockColumnValue>> rows = {
      {int64_t{1}},
      {int64_t{2}},
      {int64_t{3}},
      {int64_t{4}},
      {int64_t{5}},
  };
  auto mockResult = std::make_unique<MockInternalResult>(rows);
  auto mockMetadata = std::make_unique<MockInternalRowMetadata>(
      std::vector<MockFieldInfo>{{"id", "t", MYSQL_TYPE_LONG, 0}});

  RowStream stream(std::move(mockResult), std::move(mockMetadata));

  for (int i = 0; i < 5; ++i) {
    EXPECT_EQ(stream.numRowsSeen(), i);
    EXPECT_TRUE(stream.hasNext());
    stream.consumeRow();
  }

  EXPECT_EQ(stream.numRowsSeen(), 5);
  EXPECT_FALSE(stream.hasNext());
}

// =============================================================================
// Async State Simulation Tests
//
// Tests state transitions when simulating async behavior.
// =============================================================================

class AsyncStateSimulationTest : public ::testing::Test {};

TEST_F(AsyncStateSimulationTest, PendingStatusSimulatesAsyncWait) {
  std::vector<std::vector<MockColumnValue>> rows = {
      {int64_t{1}},
  };

  // 3 pending statuses before the row
  auto pendingResult = std::make_unique<MockPendingResult>(rows, 3);

  int pendingCount = 0;
  bool gotRow = false;

  while (!gotRow) {
    auto [status, row] = pendingResult->fetchRow();
    if (status == InternalStatus::PENDING) {
      pendingCount++;
      EXPECT_EQ(row, nullptr);
    } else if (row) {
      gotRow = true;
      EXPECT_EQ(status, InternalStatus::DONE);
    }
  }

  EXPECT_EQ(pendingCount, 3);
  EXPECT_TRUE(gotRow);
}

TEST_F(AsyncStateSimulationTest, MultipleRowsWithPending) {
  std::vector<std::vector<MockColumnValue>> rows = {
      {std::string("A")},
      {std::string("B")},
      {std::string("C")},
  };

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

  EXPECT_EQ(pendingCount, 3); // 1 pending per row
  const std::vector<std::string> expected{"A", "B", "C"};
  EXPECT_EQ(results, expected);
}

TEST_F(AsyncStateSimulationTest, ZeroPendingMeansImmediate) {
  std::vector<std::vector<MockColumnValue>> rows = {
      {int64_t{42}},
  };

  auto pendingResult = std::make_unique<MockPendingResult>(rows, 0);

  // Should get the row immediately
  auto [status, row] = pendingResult->fetchRow();
  EXPECT_EQ(status, InternalStatus::DONE);
  ASSERT_NE(row, nullptr);
  EXPECT_EQ(row->columnInt64(0), 42);
}

// =============================================================================
// QueryResult State Tests
//
// Tests QueryResult state management.
// =============================================================================

class QueryResultStateTest : public ::testing::Test {};

TEST_F(QueryResultStateTest, PartialAndOperationResultAffectOk) {
  QueryResult result(0);

  // Default: partial=true, result=Unknown → ok() is true (streaming in
  // progress)
  EXPECT_TRUE(result.partial());
  EXPECT_TRUE(result.ok());

  // partial=false, result=Unknown → ok() is false
  result.setPartial(false);
  EXPECT_FALSE(result.ok());

  // result=Succeeded → ok() is true regardless of partial
  result.setOperationResult(OperationResult::Succeeded);
  EXPECT_TRUE(result.ok());
  EXPECT_TRUE(result.succeeded());
}

TEST_F(QueryResultStateTest, FailedResultIsNotOk) {
  QueryResult result(0);

  result.setOperationResult(OperationResult::Failed);
  EXPECT_FALSE(result.ok());
  EXPECT_FALSE(result.succeeded());
}

// =============================================================================
// Error State Tests
//
// Tests error state handling in mock connections.
// =============================================================================

class ErrorStateTest : public ::testing::Test {};

TEST_F(ErrorStateTest, ErrorAffectsPing) {
  MockInternalConnection conn;

  // No error - ping succeeds
  EXPECT_TRUE(conn.ping());

  // With error - ping fails
  conn.setError(2013, "Lost connection");
  EXPECT_FALSE(conn.ping());

  // Clear error - ping succeeds again
  conn.clearError();
  EXPECT_TRUE(conn.ping());
}

TEST_F(ErrorStateTest, ErrorAffectsReset) {
  MockInternalConnection conn;

  // No error - reset succeeds
  EXPECT_EQ(conn.resetConn(), InternalConnection::Status::DONE);

  // With error - reset fails
  conn.setError(2006, "MySQL server has gone away");
  EXPECT_EQ(conn.resetConn(), InternalConnection::Status::ERROR);
}

TEST_F(ErrorStateTest, ErrorAffectsChangeUser) {
  MockInternalConnection conn;

  // No error - change user succeeds (using nullptr as connKey for testing)
  EXPECT_EQ(conn.changeUser(nullptr), InternalConnection::Status::DONE);

  // With error - change user fails
  conn.setError(1045, "Access denied");
  EXPECT_EQ(conn.changeUser(nullptr), InternalConnection::Status::ERROR);
}

// =============================================================================
// Connection State Tests
//
// Tests connection state (transaction, autocommit, reusable, etc.).
// =============================================================================

class ConnectionStateTest : public ::testing::Test {};

TEST_F(ConnectionStateTest, DefaultsAndConfiguredState) {
  // Verify defaults
  MockInternalConnection defaultConn;
  EXPECT_FALSE(defaultConn.inTransaction());
  EXPECT_TRUE(defaultConn.getAutocommit());
  EXPECT_TRUE(defaultConn.isReusable());
  EXPECT_FALSE(defaultConn.isSSL());
  EXPECT_FALSE(defaultConn.needResetBeforeReuse());

  // Verify configured state overrides defaults
  MockConnectionConfig config;
  config.inTransaction = true;
  config.autocommit = false;
  config.isSSL = true;

  MockInternalConnection conn(std::move(config));
  EXPECT_TRUE(conn.inTransaction());
  EXPECT_FALSE(conn.getAutocommit());
  EXPECT_TRUE(conn.isSSL());
}

TEST_F(ConnectionStateTest, ReusableAndResetStateTransitions) {
  MockInternalConnection conn;

  // Reusable can be toggled
  EXPECT_TRUE(conn.isReusable());
  conn.setReusable(false);
  EXPECT_FALSE(conn.isReusable());
  conn.setReusable(true);
  EXPECT_TRUE(conn.isReusable());

  // NeedReset is one-way (set, never cleared)
  EXPECT_FALSE(conn.needResetBeforeReuse());
  conn.setNeedResetBeforeReuse();
  EXPECT_TRUE(conn.needResetBeforeReuse());
}

// =============================================================================
// Query Execution State Tests
//
// Tests state changes during query execution in mock connection.
// =============================================================================

class QueryExecutionStateTest : public ::testing::Test {};

TEST_F(QueryExecutionStateTest, LastQueryTracking) {
  MockConnectionConfig config;
  config.defaultResult = MockQueryResult{
      .fields = {},
      .rows = {},
      .lastInsertId = 0,
      .affectedRows = 0,
      .warningCount = 0,
      .recvGtid = std::nullopt,
      .info = std::nullopt,
  };

  MockInternalConnection conn(std::move(config));

  conn.runQuery("SELECT 1");
  EXPECT_EQ(conn.lastQuery(), "SELECT 1");

  conn.runQuery("SELECT 2");
  EXPECT_EQ(conn.lastQuery(), "SELECT 2");

  conn.runQuery("INSERT INTO t VALUES (1)");
  EXPECT_EQ(conn.lastQuery(), "INSERT INTO t VALUES (1)");
}

TEST_F(QueryExecutionStateTest, CurrentResultMetadata) {
  MockConnectionConfig config;
  config.queryResults["SELECT id FROM users"] = MockQueryResult{
      .fields = {{"id", "users", MYSQL_TYPE_LONG, 0}},
      .rows = {{int64_t{1}}, {int64_t{2}}},
      .lastInsertId = 0,
      .affectedRows = 0,
      .warningCount = 1,
      .recvGtid = "gtid-abc",
      .info = std::nullopt,
  };

  MockInternalConnection conn(std::move(config));
  conn.runQuery("SELECT id FROM users");

  EXPECT_EQ(conn.warningCount(), 1);
  EXPECT_EQ(conn.getRecvGtid(), "gtid-abc");
}

TEST_F(QueryExecutionStateTest, InsertUpdatesLastInsertId) {
  MockConnectionConfig config;
  config.queryResults["INSERT INTO users (name) VALUES ('test')"] =
      MockQueryResult{
          .fields = {},
          .rows = {},
          .lastInsertId = 42,
          .affectedRows = 1,
          .warningCount = 0,
          .recvGtid = std::nullopt,
          .info = std::nullopt,
      };

  MockInternalConnection conn(std::move(config));
  conn.runQuery("INSERT INTO users (name) VALUES ('test')");

  EXPECT_EQ(conn.getLastInsertId(), 42);
  EXPECT_EQ(conn.getAffectedRows(), 1);
}

TEST_F(QueryExecutionStateTest, UpdateAffectsAffectedRows) {
  MockConnectionConfig config;
  config.queryResults["UPDATE users SET active = 1"] = MockQueryResult{
      .fields = {},
      .rows = {},
      .lastInsertId = 0,
      .affectedRows = 100,
      .warningCount = 0,
      .recvGtid = std::nullopt,
      .info = "Rows matched: 150  Changed: 100  Warnings: 0",
  };

  MockInternalConnection conn(std::move(config));
  conn.runQuery("UPDATE users SET active = 1");

  EXPECT_EQ(conn.getAffectedRows(), 100);
  auto info = conn.getMySQLInfo();
  EXPECT_TRUE(info.has_value());
  EXPECT_NE(info->find("150"), std::string::npos);
}

} // namespace facebook::common::mysql_client::test
