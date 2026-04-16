# Squangle Mock Test Infrastructure

This directory provides mock implementations of Squangle's internal interfaces for unit testing **without requiring a real MySQL server**.

## Overview

Squangle's architecture has a clean abstraction layer (`InternalConnection`, `InternalResult`, `InternalRow`) that separates the Operation logic from the MySQL wire protocol. The mock interfaces in this directory implement the same abstraction layer, allowing you to:
- Test Operation state machines and business logic in isolation
- Inject specific query results for deterministic testing
- Simulate error conditions and edge cases
- Test async behavior without network delays

## Quick Start

```cpp
#include "squangle/mysql_client/test/MockInternalConnection.h"

TEST(MyTest, QueryReturnsExpectedRows) {
  // Configure mock connection with expected query result
  MockConnectionConfig config;
  config.queryResults["SELECT id, name FROM users"] = MockQueryResult{
      .fields = {
          {"id", "users", MYSQL_TYPE_LONG, 0},
          {"name", "users", MYSQL_TYPE_STRING, 0},
      },
      .rows = {
          {int64_t{1}, std::string("Alice")},
          {int64_t{2}, std::string("Bob")},
      },
  };

  MockInternalConnection conn(std::move(config));

  // Run query and verify results
  conn.runQuery("SELECT id, name FROM users");
  auto result = conn.getResult();

  auto [status, row] = result->fetchRow();
  EXPECT_EQ(row->columnInt64(0), 1);
  EXPECT_EQ(row->columnString(1), "Alice");
}
```

## Mock Classes

### MockInternalRow

Implements `InternalRow` for accessing column values by type.

```cpp
std::vector<MockColumnValue> values = {
    std::string("John"),  // String column
    int64_t{25},          // Integer column
    double{3.14},         // Double column
    true,                 // Boolean column
    std::monostate{},     // NULL column
};
MockInternalRow row(std::move(values));

EXPECT_EQ(row.columnString(0), "John");
EXPECT_EQ(row.columnInt64(1), 25);
EXPECT_EQ(row.columnType(4), InternalRow::Type::Null);
```

### MockInternalRowMetadata

Implements `InternalRowMetadata` for field information.

```cpp
std::vector<MockFieldInfo> fields = {
    {"id", "users", MYSQL_TYPE_LONG, 0},
    {"name", "users", MYSQL_TYPE_STRING, 0},
    {"email", "users", MYSQL_TYPE_STRING, NOT_NULL_FLAG},
};
MockInternalRowMetadata metadata(std::move(fields));

EXPECT_EQ(metadata.numFields(), 3);
EXPECT_EQ(metadata.getFieldName(0), "id");
EXPECT_EQ(metadata.getFieldType(0), MYSQL_TYPE_LONG);
```

### MockInternalResult

Implements `InternalResult` for iterating over query results.

```cpp
std::vector<std::vector<MockColumnValue>> rows = {
    {std::string("Alice"), int64_t{30}},
    {std::string("Bob"), int64_t{25}},
};
MockInternalResult result(std::move(rows));

// Iterate rows
while (true) {
  auto [status, row] = result.fetchRow();
  if (!row) break;
  // Process row...
}
```

### MockPendingResult

Simulates async behavior by returning `PENDING` status before data.

```cpp
// Simulate 2 PENDING calls before each row is ready
MockPendingResult result(rows, /*pendingCountPerRow=*/2);

auto [s1, r1] = result.fetchRow();  // PENDING, nullptr
auto [s2, r2] = result.fetchRow();  // PENDING, nullptr
auto [s3, r3] = result.fetchRow();  // DONE, row data
```

### MockInternalConnection

Full mock of `InternalConnection` with query result injection.

```cpp
MockConnectionConfig config;

// Configure server info
config.serverInfo = "MockMySQL 8.0";

// Configure query results (exact match)
config.queryResults["SELECT 1"] = MockQueryResult{
    .fields = {{"1", "", MYSQL_TYPE_LONG, 0}},
    .rows = {{int64_t{1}}},
    .lastInsertId = 0,
    .affectedRows = 0,
};

// Configure default result for unmatched queries
config.defaultResult = MockQueryResult{
    .fields = {{"error", "", MYSQL_TYPE_STRING, 0}},
    .rows = {{std::string("Unknown query")}},
};

MockInternalConnection conn(std::move(config));
```

## Common Testing Patterns

### Testing Error Handling

```cpp
TEST(ErrorHandling, QueryFailsWithError) {
  MockInternalConnection conn;

  // Inject error state
  conn.setError(1045, "Access denied for user 'root'@'localhost'");

  EXPECT_EQ(conn.getErrno(), 1045);
  EXPECT_EQ(conn.getErrorMessage(), "Access denied for user 'root'@'localhost'");
  EXPECT_FALSE(conn.ping());

  // Query returns nullptr when error is set
  auto* result = conn.runQuery("SELECT 1");
  EXPECT_EQ(result, nullptr);

  // Clear error
  conn.clearError();
  EXPECT_TRUE(conn.ping());
}
```

### Testing Query Attributes

```cpp
TEST(QueryAttributes, AttributesArePassedThrough) {
  MockInternalConnection conn;

  AttributeMap attrs = {
      {"trace_id", "abc123"},
      {"client_name", "test"},
  };
  conn.setQueryAttributes(attrs);

  // Verify attributes were captured
  EXPECT_EQ(conn.lastQueryAttributes().at("trace_id"), "abc123");
}
```

### Testing Transaction State

```cpp
TEST(Transactions, InTransactionState) {
  MockConnectionConfig config;
  config.inTransaction = true;

  MockInternalConnection conn(std::move(config));
  EXPECT_TRUE(conn.inTransaction());

  // Modify state
  conn.config().inTransaction = false;
  EXPECT_FALSE(conn.inTransaction());
}
```

### Testing INSERT/UPDATE Results

```cpp
TEST(DML, InsertReturnsLastInsertId) {
  MockConnectionConfig config;
  config.queryResults["INSERT INTO users (name) VALUES ('Alice')"] = MockQueryResult{
      .fields = {},
      .rows = {},
      .lastInsertId = 42,
      .affectedRows = 1,
  };

  MockInternalConnection conn(std::move(config));
  conn.runQuery("INSERT INTO users (name) VALUES ('Alice')");

  EXPECT_EQ(conn.getLastInsertId(), 42);
  EXPECT_EQ(conn.getAffectedRows(), 1);
}
```

### Testing Multiple Result Sets

```cpp
TEST(MultiQuery, HandlesMultipleResults) {
  // For multi-query, configure each query separately
  MockConnectionConfig config;
  config.queryResults["SELECT 1"] = MockQueryResult{
      .rows = {{int64_t{1}}},
  };
  config.queryResults["SELECT 2"] = MockQueryResult{
      .rows = {{int64_t{2}}},
  };

  MockInternalConnection conn(std::move(config));

  conn.runQuery("SELECT 1");
  auto result1 = conn.getResult();
  // ... process first result

  conn.runQuery("SELECT 2");
  auto result2 = conn.getResult();
  // ... process second result
}
```

## Running Tests

```bash
# Run mock interface tests
buck test //squangle/mysql_client/test:mock_test

# Build the mock library (header-only)
buck build //squangle/mysql_client/test:mock_interfaces
```

## Architecture

```text
┌─────────────────────────────────────────────────────────────────────┐
│  Your Tests                                                         │
│  - Use MockInternalConnection to configure expected behavior        │
│  - Verify Operation logic without network I/O                       │
└──────────────────────────────────┬──────────────────────────────────┘
                                   │
┌──────────────────────────────────▼──────────────────────────────────┐
│  InternalConnection Interface   (squangle/mysql_client/)            │
│  - InternalConnection, InternalResult, InternalRow                  │
│  - Pure virtual interface for all DB operations                     │
└──────────────────────────────────┬──────────────────────────────────┘
                                   │
                    ┌──────────────┴──────────────┐
                    │                             │
          ┌─────────▼─────────┐       ┌──────────▼──────────┐
          │ MysqlConnection   │       │ MockInternal-       │
          │ (mysql_protocol/) │       │ Connection          │
          │                   │       │ (test/)             │
          │ Uses: libmysqlc   │       │ Uses: Test Data     │
          └───────────────────┘       └─────────────────────┘
```

## Future Work

- **MockConnection**: Full Connection subclass that injects MockInternalConnection
- **Query pattern matching**: Regex-based query matching instead of exact strings
- **Response delays**: Configurable latency for timeout testing
- **Operation integration**: Direct testing of QueryOperation, FetchOperation, etc.

## See Also

- `InternalConnection.h` - The interface being mocked
- `mysql_protocol/MysqlConnection.h` - Real MySQL implementation
