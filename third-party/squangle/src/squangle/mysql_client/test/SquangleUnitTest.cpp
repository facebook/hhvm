/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

//
// Unit tests for Squangle components using mock interfaces.
// Tests Query, QueryArgument, EphemeralRowFields, and end-to-end
// query rendering + mock execution without requiring a MySQL server.
//

#include <gtest/gtest.h>

#include "squangle/mysql_client/Query.h"
#include "squangle/mysql_client/Row.h"
#include "squangle/mysql_client/test/MockInternalConnection.h"

namespace facebook::common::mysql_client::test {

// =============================================================================
// Query Class Tests
//
// Tests query parsing, rendering, and parameter binding.
// Uses renderInsecure() which requires no database connection.
// Note: renderInsecure() uses double quotes for strings, not single quotes.
// =============================================================================

class QueryTest : public ::testing::Test {};

TEST_F(QueryTest, SimpleQueryRender) {
  Query q("SELECT * FROM users");
  auto rendered = q.renderInsecure();
  EXPECT_EQ(rendered, "SELECT * FROM users");
}

TEST_F(QueryTest, QueryWithIntParameter) {
  Query q("SELECT * FROM users WHERE id = %d", 42);
  auto rendered = q.renderInsecure();
  EXPECT_EQ(rendered, "SELECT * FROM users WHERE id = 42");
}

TEST_F(QueryTest, QueryWithStringParameter) {
  Query q("SELECT * FROM users WHERE name = %s", "Alice");
  auto rendered = q.renderInsecure();
  // renderInsecure() uses double quotes for strings
  EXPECT_EQ(rendered, "SELECT * FROM users WHERE name = \"Alice\"");
}

TEST_F(QueryTest, QueryWithTableName) {
  Query q("SELECT * FROM %T WHERE id = 1", "users");
  auto rendered = q.renderInsecure();
  // Table names use backticks
  EXPECT_EQ(rendered, "SELECT * FROM `users` WHERE id = 1");
}

TEST_F(QueryTest, QueryWithColumnName) {
  Query q("SELECT %C FROM users", "email");
  auto rendered = q.renderInsecure();
  EXPECT_EQ(rendered, "SELECT `email` FROM users");
}

TEST_F(QueryTest, QueryWithMultipleParameters) {
  Query q(
      "INSERT INTO users (name, age, active) VALUES (%s, %d, %d)",
      "Bob",
      30,
      1);
  auto rendered = q.renderInsecure();
  // Strings use double quotes
  EXPECT_EQ(
      rendered,
      "INSERT INTO users (name, age, active) VALUES (\"Bob\", 30, 1)");
}

TEST_F(QueryTest, QueryAppend) {
  Query q1("SELECT * FROM users");
  Query q2("WHERE active = 1");
  q1.append(q2);
  auto rendered = q1.renderInsecure();
  // append() adds a space between the two queries
  EXPECT_EQ(rendered, "SELECT * FROM users WHERE active = 1");
}

TEST_F(QueryTest, QueryConcatenation) {
  Query q1("SELECT * FROM users");
  Query q2("ORDER BY id");
  Query combined = q1 + q2;
  auto rendered = combined.renderInsecure();
  // operator+ adds a space between the two queries
  EXPECT_EQ(rendered, "SELECT * FROM users ORDER BY id");
}

TEST_F(QueryTest, UnsafeQuery) {
  auto q = Query::unsafe("SELECT * FROM users; DROP TABLE users;");
  EXPECT_TRUE(q.isUnsafe());
}

TEST_F(QueryTest, SafeQueryIsNotUnsafe) {
  Query q("SELECT * FROM users WHERE id = %d", 1);
  EXPECT_FALSE(q.isUnsafe());
}

TEST_F(QueryTest, QueryGetFormat) {
  Query q("SELECT * FROM %T WHERE id = %d", "users", 42);
  EXPECT_EQ(q.getQueryFormat(), "SELECT * FROM %T WHERE id = %d");
}

TEST_F(QueryTest, QueryWithBoolAsInt) {
  // Booleans are converted to int64_t internally
  Query q("SELECT * FROM users WHERE active = %d", true);
  auto rendered = q.renderInsecure();
  EXPECT_EQ(rendered, "SELECT * FROM users WHERE active = 1");

  Query q2("SELECT * FROM users WHERE active = %d", false);
  auto rendered2 = q2.renderInsecure();
  EXPECT_EQ(rendered2, "SELECT * FROM users WHERE active = 0");
}

TEST_F(QueryTest, QueryWithNullParameter) {
  Query q("UPDATE users SET deleted_at = %s WHERE id = %d", nullptr, 1);
  auto rendered = q.renderInsecure();
  // nullptr becomes NULL
  EXPECT_TRUE(rendered.find("NULL") != std::string::npos);
}

TEST_F(QueryTest, QueryWithDoubleParameter) {
  Query q("SELECT * FROM products WHERE price < %f", 19.99);
  auto rendered = q.renderInsecure();
  EXPECT_TRUE(rendered.find("19.99") != std::string::npos);
}

// =============================================================================
// QueryArgument Tests
//
// Tests query parameter type system and conversions.
// =============================================================================

class QueryArgumentTest : public ::testing::Test {};

TEST_F(QueryArgumentTest, AsStringConversion) {
  QueryArgument intArg(42);
  EXPECT_EQ(intArg.asString(), "42");

  QueryArgument strArg("hello");
  EXPECT_EQ(strArg.asString(), "hello");

  QueryArgument dblArg(3.14);
  // Double conversion may have precision variations
  EXPECT_TRUE(dblArg.asString().find("3.14") != std::string::npos);
}

// =============================================================================
// EphemeralRowFields Tests
//
// Tests the EphemeralRowFields class using MockInternalRowMetadata.
// =============================================================================

class EphemeralRowFieldsTest : public ::testing::Test {};

TEST_F(EphemeralRowFieldsTest, FieldLookupByNameIndexAndType) {
  auto metadata =
      std::make_unique<MockInternalRowMetadata>(std::vector<MockFieldInfo>{
          {"id", "users", MYSQL_TYPE_LONG, 0},
          {"name", "users", MYSQL_TYPE_STRING, 0},
          {"price", "products", MYSQL_TYPE_DOUBLE, 0},
      });

  EphemeralRowFields fields(std::move(metadata));

  EXPECT_EQ(fields.numFields(), 3);

  // Index lookup by name
  EXPECT_EQ(fields.fieldIndex("id"), 0);
  EXPECT_EQ(fields.fieldIndex("name"), 1);

  // Optional index lookup
  EXPECT_EQ(*fields.fieldIndexOpt("id"), 0);
  EXPECT_FALSE(fields.fieldIndexOpt("nonexistent").has_value());

  // Field names and types
  EXPECT_EQ(fields.fieldName(0), "id");
  EXPECT_EQ(fields.fieldName(1), "name");
  EXPECT_EQ(fields.fieldType(0), MYSQL_TYPE_LONG);
  EXPECT_EQ(fields.fieldType(2), MYSQL_TYPE_DOUBLE);
}

TEST_F(EphemeralRowFieldsTest, MakeBufferedFields) {
  auto metadata =
      std::make_unique<MockInternalRowMetadata>(std::vector<MockFieldInfo>{
          {"id", "users", MYSQL_TYPE_LONG, 0},
          {"name", "users", MYSQL_TYPE_STRING, 0},
      });

  EphemeralRowFields ephemeralFields(std::move(metadata));
  auto bufferedFields = ephemeralFields.makeBufferedFields();

  EXPECT_NE(bufferedFields, nullptr);
  EXPECT_EQ(bufferedFields->numFields(), 2);
  EXPECT_EQ(bufferedFields->fieldName(0), "id");
  EXPECT_EQ(bufferedFields->fieldName(1), "name");
}

// =============================================================================
// MockInternalConnection Tests
//
// Tests using mock infrastructure to validate query execution flow.
// Basic mock behavior (error injection, query attributes, transaction state,
// reset, default results) is covered in MockTest.cpp (D94587045).
// =============================================================================

class MockIntegrationTest : public ::testing::Test {};

TEST_F(MockIntegrationTest, SimulateSelectQuery) {
  MockConnectionConfig config;
  config.queryResults["SELECT id, name FROM users"] = MockQueryResult{
      .fields =
          {
              {"id", "users", MYSQL_TYPE_LONG, 0},
              {"name", "users", MYSQL_TYPE_STRING, 0},
          },
      .rows =
          {
              {int64_t{1}, std::string("Alice")},
              {int64_t{2}, std::string("Bob")},
          },
  };

  MockInternalConnection conn(std::move(config));

  auto* queryResult = conn.runQuery("SELECT id, name FROM users");
  ASSERT_NE(queryResult, nullptr);

  auto result = conn.getResult();
  ASSERT_NE(result, nullptr);

  auto [status1, row1] = result->fetchRow();
  EXPECT_EQ(status1, InternalStatus::DONE);
  ASSERT_NE(row1, nullptr);
  EXPECT_EQ(row1->columnInt64(0), 1);
  EXPECT_EQ(row1->columnString(1), "Alice");

  auto [status2, row2] = result->fetchRow();
  ASSERT_NE(row2, nullptr);
  EXPECT_EQ(row2->columnString(1), "Bob");

  auto [status3, row3] = result->fetchRow();
  EXPECT_EQ(row3, nullptr);
}

TEST_F(MockIntegrationTest, SimulateQueryError) {
  MockConnectionConfig config;
  config.errorNumber = 1146;
  config.errorMessage = "Table 'test.nonexistent' doesn't exist";

  MockInternalConnection conn(std::move(config));

  auto* result = conn.runQuery("SELECT * FROM nonexistent");
  EXPECT_EQ(result, nullptr);
  EXPECT_EQ(conn.getErrno(), 1146);
}

// =============================================================================
// End-to-End Tests: Query Rendering + Mock Execution
// =============================================================================

TEST(EndToEndTest, QueryBuildAndMockExecute) {
  // Build a parameterized query
  Query query("SELECT id, name FROM %T WHERE active = %d", "users", 1);

  auto rendered = query.renderInsecure();
  EXPECT_EQ(rendered, "SELECT id, name FROM `users` WHERE active = 1");

  // Configure mock with this query
  MockConnectionConfig config;
  config.queryResults[rendered.toStdString()] = MockQueryResult{
      .fields =
          {
              {"id", "users", MYSQL_TYPE_LONG, 0},
              {"name", "users", MYSQL_TYPE_STRING, 0},
          },
      .rows =
          {
              {int64_t{1}, std::string("Alice")},
              {int64_t{2}, std::string("Bob")},
          },
  };

  MockInternalConnection conn(std::move(config));
  conn.runQuery(rendered.toStdString());

  auto result = conn.getResult();
  ASSERT_NE(result, nullptr);

  std::vector<std::string> names;
  while (true) {
    auto [status, row] = result->fetchRow();
    if (!row) {
      break;
    }
    names.emplace_back(row->columnString(1));
  }

  const std::vector<std::string> expected{"Alice", "Bob"};
  EXPECT_EQ(names, expected);
}

TEST(EndToEndTest, InsertQueryWithLastInsertId) {
  Query query("INSERT INTO %T (name) VALUES (%s)", "users", "NewUser");

  auto rendered = query.renderInsecure();
  // String values use double quotes in renderInsecure()
  EXPECT_EQ(rendered, "INSERT INTO `users` (name) VALUES (\"NewUser\")");

  MockConnectionConfig config;
  config.queryResults[rendered.toStdString()] = MockQueryResult{
      .lastInsertId = 999,
      .affectedRows = 1,
  };

  MockInternalConnection conn(std::move(config));
  conn.runQuery(rendered.toStdString());

  EXPECT_EQ(conn.getLastInsertId(), 999);
  EXPECT_EQ(conn.getAffectedRows(), 1);
}

TEST(EndToEndTest, ComplexQueryWithMultipleTypes) {
  // Test a more complex query with different parameter types
  Query query(
      "UPDATE %T SET name = %s, age = %d, score = %f WHERE id = %d",
      "users",
      "UpdatedName",
      25,
      99.5,
      42);

  auto rendered = query.renderInsecure();

  // Verify all parts are present
  EXPECT_TRUE(rendered.find("`users`") != std::string::npos);
  EXPECT_TRUE(rendered.find("\"UpdatedName\"") != std::string::npos);
  EXPECT_TRUE(rendered.find("25") != std::string::npos);
  EXPECT_TRUE(rendered.find("99.5") != std::string::npos);
  EXPECT_TRUE(rendered.find("42") != std::string::npos);
}

} // namespace facebook::common::mysql_client::test
