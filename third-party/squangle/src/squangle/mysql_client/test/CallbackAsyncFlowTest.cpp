/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

//
// Async Flow Tests
//
// Tests RowStream behavior with MockPendingResult to exercise the async
// code path through slurp(). When InternalResult::fetchRow() returns
// PENDING, slurp() returns false and hasNext() reports no data available.
// This simulates the production scenario where MySQL wire protocol data
// hasn't arrived yet.
//
// Direct MockPendingResult unit tests live in MockTest.cpp (D94587045).
// These tests exercise RowStream (production code) with async data.
//

#include <gtest/gtest.h>

#include "squangle/mysql_client/FetchOperation.h"
#include "squangle/mysql_client/test/MockInternalConnection.h"
#include "squangle/mysql_client/test/MockInternalResult.h"

namespace facebook::common::mysql_client::test {

// =============================================================================
// RowStream Async Tests
//
// Tests RowStream's slurp() behavior when the underlying InternalResult
// returns PENDING, simulating async MySQL I/O.
// =============================================================================

class RowStreamAsyncTest : public ::testing::Test {
 protected:
  std::unique_ptr<MockInternalRowMetadata> makeMetadata() {
    return std::make_unique<MockInternalRowMetadata>(std::vector<MockFieldInfo>{
        {"id", "t", MYSQL_TYPE_LONG, 0},
        {"name", "t", MYSQL_TYPE_STRING, 0},
    });
  }
};

TEST_F(RowStreamAsyncTest, HasNextReturnsFalseDuringPending) {
  std::vector<std::vector<MockColumnValue>> rows = {
      {int64_t{1}, std::string("Alice")},
  };

  // 2 PENDING calls before the row is available
  auto pendingResult = std::make_unique<MockPendingResult>(rows, 2);
  RowStream stream(std::move(pendingResult), makeMetadata());

  // First two hasNext() calls hit PENDING — no row available yet
  EXPECT_FALSE(stream.hasNext());
  EXPECT_EQ(stream.numRowsSeen(), 0);

  EXPECT_FALSE(stream.hasNext());
  EXPECT_EQ(stream.numRowsSeen(), 0);

  // Third call gets the row
  EXPECT_TRUE(stream.hasNext());
  EXPECT_EQ(stream.numRowsSeen(), 1);

  auto row = stream.consumeRow();
  EXPECT_EQ(row.getInt64(0), 1);
  EXPECT_EQ(row.getString(1), "Alice");
}

TEST_F(RowStreamAsyncTest, MultipleRowsWithPendingBetweenEach) {
  std::vector<std::vector<MockColumnValue>> rows = {
      {int64_t{1}, std::string("Alice")},
      {int64_t{2}, std::string("Bob")},
      {int64_t{3}, std::string("Charlie")},
  };

  constexpr int kPendingCount = 1;
  auto pendingResult = std::make_unique<MockPendingResult>(rows, kPendingCount);
  RowStream stream(std::move(pendingResult), makeMetadata());

  std::vector<std::string> names;

  // Drain the stream, retrying up to kPendingCount times for each row
  while (true) {
    bool found = false;
    for (int i = 0; i <= kPendingCount; ++i) {
      if (stream.hasNext()) {
        found = true;
        break;
      }
    }
    if (!found) {
      break;
    }
    names.emplace_back(stream.consumeRow().getString(1));
  }

  const std::vector<std::string> expected{"Alice", "Bob", "Charlie"};
  EXPECT_EQ(names, expected);
  EXPECT_EQ(stream.numRowsSeen(), 3);
}

TEST_F(RowStreamAsyncTest, ZeroPendingBehavesLikeSyncResult) {
  std::vector<std::vector<MockColumnValue>> rows = {
      {int64_t{1}, std::string("sync")},
      {int64_t{2}, std::string("result")},
  };

  // 0 pending = immediate delivery, same as MockInternalResult
  auto pendingResult = std::make_unique<MockPendingResult>(rows, 0);
  RowStream stream(std::move(pendingResult), makeMetadata());

  // Rows should be immediately available
  EXPECT_TRUE(stream.hasNext());
  EXPECT_EQ(stream.consumeRow().getString(1), "sync");

  EXPECT_TRUE(stream.hasNext());
  EXPECT_EQ(stream.consumeRow().getString(1), "result");

  EXPECT_FALSE(stream.hasNext());
  EXPECT_EQ(stream.numRowsSeen(), 2);
}

TEST_F(RowStreamAsyncTest, HasNextIsIdempotentWithoutConsume) {
  std::vector<std::vector<MockColumnValue>> rows = {
      {int64_t{42}, std::string("test")},
  };

  auto result = std::make_unique<MockInternalResult>(rows);
  RowStream stream(std::move(result), makeMetadata());

  // Multiple hasNext() calls without consuming should return true
  // and not advance the stream (slurp short-circuits when current_row_
  // has a value)
  EXPECT_TRUE(stream.hasNext());
  EXPECT_TRUE(stream.hasNext());
  EXPECT_TRUE(stream.hasNext());
  EXPECT_EQ(stream.numRowsSeen(), 1);

  // Should still get the same row
  auto row = stream.consumeRow();
  EXPECT_EQ(row.getInt64(0), 42);

  // Now finished
  EXPECT_FALSE(stream.hasNext());
}

TEST_F(RowStreamAsyncTest, QueryResultSizeAccumulatesWithPending) {
  std::vector<std::vector<MockColumnValue>> rows = {
      {int64_t{1}, std::string("short")},
      {int64_t{2}, std::string("a longer string value")},
  };

  auto pendingResult = std::make_unique<MockPendingResult>(rows, 1);
  RowStream stream(std::move(pendingResult), makeMetadata());

  EXPECT_EQ(stream.queryResultSize(), 0);

  // First row: PENDING then available
  EXPECT_FALSE(stream.hasNext()); // PENDING
  EXPECT_EQ(stream.queryResultSize(), 0);
  EXPECT_TRUE(stream.hasNext()); // got row
  EXPECT_GT(stream.queryResultSize(), 0);
  uint64_t sizeAfterFirstRow = stream.queryResultSize();
  stream.consumeRow();

  // Second row: PENDING then available
  EXPECT_FALSE(stream.hasNext()); // PENDING
  EXPECT_EQ(stream.queryResultSize(), sizeAfterFirstRow);
  EXPECT_TRUE(stream.hasNext()); // got row
  EXPECT_GT(stream.queryResultSize(), sizeAfterFirstRow);
  stream.consumeRow();

  EXPECT_EQ(stream.numRowsSeen(), 2);
}

} // namespace facebook::common::mysql_client::test
