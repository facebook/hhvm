/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>

#include "squangle/mysql_client/test/MockConnection.h"

namespace facebook::common::mysql_client::test {

// =============================================================================
// MockConnection Unit Tests
//
// These tests verify MockConnection and MockInternalConnection behavior
// by executing queries and checking results, error handling, and state.
// =============================================================================

class MockConnectionTest : public ::testing::Test {
 protected:
  void SetUp() override {
    client_ = std::make_unique<MockMysqlClient>();
    connKey_ = MockMysqlClient::createTestConnectionKey();
  }

  std::unique_ptr<MockMysqlClient> client_;
  std::shared_ptr<ConnectionKey> connKey_;
};

// -----------------------------------------------------------------------------
// Basic MockConnection Tests
// -----------------------------------------------------------------------------

TEST_F(MockConnectionTest, CreateMockConnection) {
  auto conn = client_->createMockConnection(connKey_);
  ASSERT_NE(conn, nullptr);
  // Before initMysql, the internal connection doesn't exist yet
  EXPECT_EQ(conn->getMockInternalConnection(), nullptr);
}

// -----------------------------------------------------------------------------
// Query Execution Tests
// These tests create a MockInternalConnection directly to verify that
// configured queries return expected results when executed.
// -----------------------------------------------------------------------------

TEST_F(MockConnectionTest, SelectQueryReturnsConfiguredRows) {
  MockConnectionConfig config;
  config.queryResults["SELECT id, name FROM users WHERE active = 1"] =
      MockQueryResult{
          .fields =
              {
                  {"id", "users", MYSQL_TYPE_LONG, 0},
                  {"name", "users", MYSQL_TYPE_STRING, 0},
              },
          .rows =
              {
                  {int64_t{1}, std::string("Alice")},
                  {int64_t{2}, std::string("Bob")},
                  {int64_t{3}, std::string("Charlie")},
              },
      };

  MockInternalConnection conn(std::move(config));
  auto* queryResult =
      conn.runQuery("SELECT id, name FROM users WHERE active = 1");
  ASSERT_NE(queryResult, nullptr);
  EXPECT_EQ(queryResult->rows.size(), 3);

  auto result = conn.getResult();
  ASSERT_NE(result, nullptr);

  auto [status1, row1] = result->fetchRow();
  ASSERT_NE(row1, nullptr);
  EXPECT_EQ(row1->columnInt64(0), 1);
  EXPECT_EQ(row1->columnString(1), "Alice");

  auto [status2, row2] = result->fetchRow();
  ASSERT_NE(row2, nullptr);
  EXPECT_EQ(row2->columnInt64(0), 2);
  EXPECT_EQ(row2->columnString(1), "Bob");

  auto [status3, row3] = result->fetchRow();
  ASSERT_NE(row3, nullptr);
  EXPECT_EQ(row3->columnInt64(0), 3);
  EXPECT_EQ(row3->columnString(1), "Charlie");

  auto [status4, row4] = result->fetchRow();
  EXPECT_EQ(row4, nullptr);
}

TEST_F(MockConnectionTest, InsertQueryReturnsMetadata) {
  MockConnectionConfig config;
  config.queryResults["INSERT INTO users (name) VALUES ('NewUser')"] =
      MockQueryResult{
          .lastInsertId = 42,
          .affectedRows = 1,
      };

  MockInternalConnection conn(std::move(config));
  auto* queryResult =
      conn.runQuery("INSERT INTO users (name) VALUES ('NewUser')");
  ASSERT_NE(queryResult, nullptr);

  EXPECT_EQ(conn.getLastInsertId(), 42);
  EXPECT_EQ(conn.getAffectedRows(), 1);
}

TEST_F(MockConnectionTest, UpdateQueryReturnsAffectedRows) {
  MockConnectionConfig config;
  config.queryResults["UPDATE users SET status = 'active' WHERE id > 100"] =
      MockQueryResult{
          .affectedRows = 50,
          .info = "Rows matched: 50  Changed: 50  Warnings: 0",
      };

  MockInternalConnection conn(std::move(config));
  conn.runQuery("UPDATE users SET status = 'active' WHERE id > 100");

  EXPECT_EQ(conn.getAffectedRows(), 50);
  auto info = conn.getMySQLInfo();
  ASSERT_TRUE(info.has_value());
  EXPECT_EQ(*info, "Rows matched: 50  Changed: 50  Warnings: 0");
}

TEST_F(MockConnectionTest, DeleteQueryReturnsAffectedRows) {
  MockConnectionConfig config;
  config.queryResults["DELETE FROM logs WHERE created_at < '2024-01-01'"] =
      MockQueryResult{
          .affectedRows = 1000,
      };

  MockInternalConnection conn(std::move(config));
  conn.runQuery("DELETE FROM logs WHERE created_at < '2024-01-01'");

  EXPECT_EQ(conn.getAffectedRows(), 1000);
}

// -----------------------------------------------------------------------------
// Error Handling Tests
// -----------------------------------------------------------------------------

TEST_F(MockConnectionTest, ConnectionErrorBlocksQueries) {
  MockConnectionConfig config;
  config.errorNumber = 2003;
  config.errorMessage = "Can't connect to MySQL server on 'localhost'";
  config.queryResults["SELECT 1"] = MockQueryResult{
      .rows = {{int64_t{1}}},
  };

  MockInternalConnection conn(std::move(config));

  // Query should fail because connection has an error
  auto* result = conn.runQuery("SELECT 1");
  EXPECT_EQ(result, nullptr);
  EXPECT_EQ(conn.getErrno(), 2003u);
  EXPECT_EQ(
      conn.getErrorMessage(), "Can't connect to MySQL server on 'localhost'");
  EXPECT_FALSE(conn.ping());
}

TEST_F(MockConnectionTest, ClearErrorAllowsQueries) {
  MockConnectionConfig config;
  config.errorNumber = 1045;
  config.errorMessage = "Access denied";
  config.queryResults["SELECT 1"] = MockQueryResult{
      .rows = {{int64_t{1}}},
  };

  MockInternalConnection conn(std::move(config));

  EXPECT_EQ(conn.runQuery("SELECT 1"), nullptr);

  conn.clearError();
  EXPECT_EQ(conn.getErrno(), 0u);
  EXPECT_TRUE(conn.ping());

  auto* result = conn.runQuery("SELECT 1");
  ASSERT_NE(result, nullptr);
  EXPECT_EQ(result->rows.size(), 1);
}

// -----------------------------------------------------------------------------
// Query Routing Tests
// -----------------------------------------------------------------------------

TEST_F(MockConnectionTest, MultipleQueriesRouteCorrectly) {
  MockConnectionConfig config;
  config.queryResults["SELECT COUNT(*) FROM users"] = MockQueryResult{
      .fields = {{"COUNT(*)", "", MYSQL_TYPE_LONGLONG, 0}},
      .rows = {{int64_t{100}}},
  };
  config.queryResults["SELECT MAX(id) FROM users"] = MockQueryResult{
      .fields = {{"MAX(id)", "", MYSQL_TYPE_LONGLONG, 0}},
      .rows = {{int64_t{999}}},
  };
  config.queryResults["SHOW TABLES"] = MockQueryResult{
      .fields = {{"Tables_in_test", "", MYSQL_TYPE_STRING, 0}},
      .rows =
          {
              {std::string("users")},
              {std::string("orders")},
              {std::string("products")},
          },
  };

  MockInternalConnection conn(std::move(config));

  // Each query should return its own configured result
  auto* countResult = conn.runQuery("SELECT COUNT(*) FROM users");
  ASSERT_NE(countResult, nullptr);
  EXPECT_EQ(countResult->rows.size(), 1);

  auto* maxResult = conn.runQuery("SELECT MAX(id) FROM users");
  ASSERT_NE(maxResult, nullptr);
  EXPECT_EQ(maxResult->rows.size(), 1);

  auto* showResult = conn.runQuery("SHOW TABLES");
  ASSERT_NE(showResult, nullptr);
  EXPECT_EQ(showResult->rows.size(), 3);

  // Verify last query tracking
  EXPECT_EQ(conn.lastQuery(), "SHOW TABLES");
}

TEST_F(MockConnectionTest, DefaultResultFallback) {
  MockConnectionConfig config;
  config.defaultResult = MockQueryResult{
      .fields = {{"result", "", MYSQL_TYPE_STRING, 0}},
      .rows = {{std::string("default response")}},
  };

  MockInternalConnection conn(std::move(config));

  // Any unmatched query should return the default result
  auto* result = conn.runQuery("SELECT anything FROM anywhere");
  ASSERT_NE(result, nullptr);

  auto mockResult = conn.getResult();
  ASSERT_NE(mockResult, nullptr);
  auto [status, row] = mockResult->fetchRow();
  ASSERT_NE(row, nullptr);
  EXPECT_EQ(row->columnString(0), "default response");
}

TEST_F(MockConnectionTest, UnmatchedQueryWithNoDefaultReturnsNull) {
  MockConnectionConfig config;
  config.queryResults["SELECT 1"] = MockQueryResult{
      .rows = {{int64_t{1}}},
  };

  MockInternalConnection conn(std::move(config));

  auto* result = conn.runQuery("SELECT 2");
  EXPECT_EQ(result, nullptr);
}

// -----------------------------------------------------------------------------
// Complex Data Type Tests
// -----------------------------------------------------------------------------

TEST_F(MockConnectionTest, ComplexQueryWithAllTypes) {
  MockConnectionConfig config;
  config.queryResults["SELECT * FROM mixed_types"] = MockQueryResult{
      .fields =
          {
              {"id", "mixed_types", MYSQL_TYPE_LONG, 0},
              {"name", "mixed_types", MYSQL_TYPE_STRING, 0},
              {"balance", "mixed_types", MYSQL_TYPE_DOUBLE, 0},
              {"active", "mixed_types", MYSQL_TYPE_TINY, 0},
              {"notes", "mixed_types", MYSQL_TYPE_STRING, 0},
          },
      .rows =
          {
              {int64_t{1},
               std::string("Test User"),
               double{123.45},
               true,
               std::monostate{}}, // NULL
              {int64_t{2},
               std::string("Another"),
               double{0.0},
               false,
               std::string("Some notes")},
          },
  };

  MockInternalConnection conn(std::move(config));
  conn.runQuery("SELECT * FROM mixed_types");

  auto result = conn.getResult();
  ASSERT_NE(result, nullptr);

  auto [s1, row1] = result->fetchRow();
  ASSERT_NE(row1, nullptr);
  EXPECT_EQ(row1->columnInt64(0), 1);
  EXPECT_EQ(row1->columnString(1), "Test User");
  EXPECT_DOUBLE_EQ(row1->columnDouble(2), 123.45);
  EXPECT_TRUE(row1->columnBool(3));
  EXPECT_EQ(row1->columnType(4), InternalRow::Type::Null);

  auto [s2, row2] = result->fetchRow();
  ASSERT_NE(row2, nullptr);
  EXPECT_EQ(row2->columnInt64(0), 2);
  EXPECT_FALSE(row2->columnBool(3));
  EXPECT_EQ(row2->columnString(4), "Some notes");
}

TEST_F(MockConnectionTest, EmptyResultSetReturnsNoRows) {
  MockConnectionConfig config;
  config.queryResults["SELECT * FROM empty_table"] = MockQueryResult{
      .fields =
          {
              {"id", "empty_table", MYSQL_TYPE_LONG, 0},
              {"value", "empty_table", MYSQL_TYPE_STRING, 0},
          },
  };

  MockInternalConnection conn(std::move(config));
  conn.runQuery("SELECT * FROM empty_table");

  auto result = conn.getResult();
  ASSERT_NE(result, nullptr);
  EXPECT_EQ(result->numRows(), 0);

  auto [status, row] = result->fetchRow();
  EXPECT_EQ(row, nullptr);
}

// -----------------------------------------------------------------------------
// Query Attribute Tests
// -----------------------------------------------------------------------------

TEST_F(MockConnectionTest, QueryAttributesAreRecorded) {
  MockInternalConnection conn;

  conn.setQueryAttribute("caller", "test_service");
  conn.setQueryAttribute("request_id", "abc123");

  const auto& attrs = conn.lastQueryAttributes();
  EXPECT_EQ(attrs.at("caller"), "test_service");
  EXPECT_EQ(attrs.at("request_id"), "abc123");
}

// -----------------------------------------------------------------------------
// Metadata Tests
// -----------------------------------------------------------------------------

TEST_F(MockConnectionTest, QueryMetadataReturnsFieldInfo) {
  MockConnectionConfig config;
  config.queryResults["SELECT id, name FROM users"] = MockQueryResult{
      .fields =
          {
              {"id", "users", MYSQL_TYPE_LONG, 0},
              {"name", "users", MYSQL_TYPE_STRING, 0},
          },
      .rows = {{int64_t{1}, std::string("Alice")}},
  };

  MockInternalConnection conn(std::move(config));
  conn.runQuery("SELECT id, name FROM users");

  auto metadata = conn.getMetadata();
  ASSERT_NE(metadata, nullptr);
  EXPECT_EQ(metadata->numFields(), 2);
  EXPECT_EQ(metadata->getFieldName(0), "id");
  EXPECT_EQ(metadata->getFieldName(1), "name");
  EXPECT_EQ(metadata->getTableName(0), "users");
}

} // namespace facebook::common::mysql_client::test
