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

#include "squangle/mysql_client/OperationHelpers.h"
#include "squangle/mysql_client/Query.h"
#include "squangle/mysql_client/Row.h"
#include "squangle/mysql_client/test/MockInternalConnection.h"
#include "squangle/mysql_client/test/MockInternalResult.h"
#include "squangle/mysql_client/test/MockInternalRow.h"
#include "squangle/mysql_client/test/MockInternalRowMetadata.h"

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

// =============================================================================
// RowBlock Build-Path Tests
//
// Covers the RowBlock building API (startRow/appendValue/appendNull/finishRow)
// and the bounds checks on the read path. These run inside a noexcept libevent
// callback, so they must throw a diagnosable exception rather than abort, and
// must never leave a partially built row visible to readers.
// =============================================================================

namespace {

std::shared_ptr<RowFields> makeRowFields(std::vector<std::string> fieldNames) {
  const auto numFields = fieldNames.size();
  folly::F14NodeMap<std::string, int> fieldNameMap;
  for (size_t i = 0; i < numFields; ++i) {
    fieldNameMap[fieldNames[i]] = static_cast<int>(i);
  }
  return std::make_shared<RowFields>(
      std::move(fieldNameMap),
      std::move(fieldNames),
      std::vector<std::string>(numFields, "test_table"),
      std::vector<uint64_t>(numFields, 0),
      std::vector<enum_field_types>(numFields, MYSQL_TYPE_VAR_STRING));
}

} // namespace

TEST(RowBlockTest, BuildRows_TwoCompletedRows_StoresExactlyTwoRows) {
  RowBlock block(makeRowFields({"id", "name"}));

  block.startRow();
  block.appendValue(folly::StringPiece("1"));
  block.appendValue(folly::StringPiece("alice"));
  block.finishRow();

  block.startRow();
  block.appendValue(folly::StringPiece("2"));
  block.appendNull();
  block.finishRow();

  EXPECT_EQ(block.numRows(), size_t{2});
  EXPECT_EQ(block.getField<std::string>(0, 0), "1");
  EXPECT_EQ(block.getField<std::string>(0, 1), "alice");
  EXPECT_EQ(block.getField<std::string>(1, 0), "2");
  EXPECT_TRUE(block.isNull(1, 1));
}

TEST(RowBlockTest, AppendValue_PastFieldCount_ThrowsOutOfRange) {
  RowBlock block(makeRowFields({"only"}));

  block.startRow();
  block.appendValue(folly::StringPiece("a"));

  // Overrunning the declared column count is an out_of_range, not a
  // runtime_error -- anything catching this must widen accordingly.
  EXPECT_THROW(block.appendValue(folly::StringPiece("b")), std::out_of_range);
}

TEST(RowBlockTest, AppendValue_WithoutStartRow_ThrowsLogicError) {
  RowBlock block(makeRowFields({"id"}));

  EXPECT_THROW(block.appendValue(folly::StringPiece("a")), std::logic_error);
  EXPECT_THROW(block.appendNull(), std::logic_error);
}

TEST(RowBlockTest, StartRow_CalledTwiceWithoutFinish_ThrowsLogicError) {
  RowBlock block(makeRowFields({"id"}));

  block.startRow();

  EXPECT_THROW(block.startRow(), std::logic_error);
}

TEST(RowBlockTest, FinishRow_WithMissingFields_ThrowsAndStoresNoRow) {
  RowBlock block(makeRowFields({"id", "name"}));

  block.startRow();
  block.appendValue(folly::StringPiece("1")); // only 1 of the 2 fields

  EXPECT_THROW(block.finishRow(), std::logic_error);
  // The incomplete row must not become visible to readers.
  EXPECT_EQ(block.numRows(), size_t{0});
  EXPECT_TRUE(block.empty());
}

TEST(RowBlockTest, FinishRow_UnstartedRow_ThrowsLogicError) {
  RowBlock block(makeRowFields({"id"}));

  EXPECT_THROW(block.finishRow(), std::logic_error);
}

TEST(RowBlockTest, IsNull_FieldPastEnd_ThrowsOutOfRange) {
  RowBlock block(makeRowFields({"id"}));
  block.startRow();
  block.appendValue(folly::StringPiece("1"));
  block.finishRow();

  // StorageRow::isNull only DCHECKs its column index, so without an explicit
  // check here an out-of-range field would be an unchecked read in opt builds.
  EXPECT_THROW(block.isNull(0, 5), std::out_of_range);
  EXPECT_THROW(block.isNull(5, 0), std::out_of_range);
}

TEST(RowBlockTest, GetField_RowPastEnd_ThrowsOutOfRangeNamingIndexAndSize) {
  RowBlock block(makeRowFields({"id"}));
  block.startRow();
  block.appendValue(folly::StringPiece("1"));
  block.finishRow();

  try {
    block.getField<std::string>(7, 0);
    FAIL() << "expected std::out_of_range for a row index past the end";
  } catch (const std::out_of_range& ex) {
    // The overrun magnitude is the primary diagnostic for the malformed-result
    // cases this check exists for, so index and size must both survive.
    EXPECT_EQ(std::string(ex.what()), "row index 7 out of range (size 1)");
  }
}

// A result set whose rows carry more columns than the RowFields describe is
// rejected as MalformedResultError rather than escaping as the raw
// std::out_of_range that RowBlock throws. The fetch loop runs inside a
// noexcept libevent callback and relies on this type to tell a bad result set
// apart from an exception thrown by a consumer callback.
TEST(MalformedResultTest, RowWiderThanRowFieldsThrowsMalformedResult) {
  // Stream metadata declares two columns...
  auto metadata =
      std::make_unique<MockInternalRowMetadata>(std::vector<MockFieldInfo>{
          {.name = "id", .tableName = "t", .type = MYSQL_TYPE_LONGLONG},
          {.name = "name", .tableName = "t", .type = MYSQL_TYPE_VARCHAR}});
  auto result = std::make_unique<MockInternalResult>(
      std::vector<std::vector<MockColumnValue>>{
          {int64_t{1}, std::string("alice")}});
  RowStream stream(std::move(result), std::move(metadata));

  // ...but the RowFields the rows are copied into describe only one.
  auto narrowFields = std::make_shared<RowFields>(
      folly::F14NodeMap<std::string, int>{{"id", 0}},
      std::vector<std::string>{"id"},
      std::vector<std::string>{"t"},
      std::vector<uint64_t>{0},
      std::vector<enum_field_types>{MYSQL_TYPE_LONGLONG});

  EXPECT_THROW(
      makeRowBlockFromStream(narrowFields, &stream), MalformedResultError);
}

// The originating diagnostic survives the retag -- index and size are what
// make a malformed result set actionable.
TEST(MalformedResultTest, MalformedResultPreservesUnderlyingMessage) {
  auto metadata =
      std::make_unique<MockInternalRowMetadata>(std::vector<MockFieldInfo>{
          {.name = "id", .tableName = "t", .type = MYSQL_TYPE_LONGLONG},
          {.name = "name", .tableName = "t", .type = MYSQL_TYPE_VARCHAR}});
  auto result = std::make_unique<MockInternalResult>(
      std::vector<std::vector<MockColumnValue>>{
          {int64_t{1}, std::string("alice")}});
  RowStream stream(std::move(result), std::move(metadata));

  auto narrowFields = std::make_shared<RowFields>(
      folly::F14NodeMap<std::string, int>{{"id", 0}},
      std::vector<std::string>{"id"},
      std::vector<std::string>{"t"},
      std::vector<uint64_t>{0},
      std::vector<enum_field_types>{MYSQL_TYPE_LONGLONG});

  try {
    makeRowBlockFromStream(narrowFields, &stream);
    FAIL() << "expected MalformedResultError";
  } catch (const MalformedResultError& ex) {
    // Which layer rejects the row -- and so the exact wording -- depends on
    // how copyRowToRowBlock() appends values, so match on the substance: the
    // message must still describe the column mismatch rather than a generic
    // "malformed result".
    EXPECT_NE(std::string(ex.what()).find("column"), std::string::npos)
        << ex.what();
  }
}

// A well-formed result set is unaffected by the retag.
TEST(MalformedResultTest, WellFormedResultBuildsNormally) {
  auto metadata =
      std::make_unique<MockInternalRowMetadata>(std::vector<MockFieldInfo>{
          {.name = "id", .tableName = "t", .type = MYSQL_TYPE_LONGLONG},
          {.name = "name", .tableName = "t", .type = MYSQL_TYPE_VARCHAR}});
  auto result = std::make_unique<MockInternalResult>(
      std::vector<std::vector<MockColumnValue>>{
          {int64_t{1}, std::string("alice")},
          {int64_t{2}, std::string("bob")}});
  RowStream stream(std::move(result), std::move(metadata));

  auto fields = stream.getEphemeralRowFields()->makeBufferedFields();
  auto block = makeRowBlockFromStream(fields, &stream);

  ASSERT_EQ(block.numRows(), 2);
  EXPECT_EQ(block.getField<int64_t>(0, "id"), 1);
  EXPECT_EQ(block.getField<std::string>(1, "name"), "bob");
}

} // namespace facebook::common::mysql_client::test
