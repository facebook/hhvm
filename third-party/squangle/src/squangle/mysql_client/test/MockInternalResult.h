/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <vector>

#include "squangle/mysql_client/InternalConnection.h"
#include "squangle/mysql_client/test/MockInternalRow.h"

namespace facebook::common::mysql_client::test {

/**
 * MockInternalResult implements InternalResult for unit testing.
 *
 * Stores a collection of rows that can be iterated via fetchRow().
 *
 * Usage:
 *   std::vector<std::vector<MockColumnValue>> rows = {
 *       {"John", int64_t{25}},
 *       {"Jane", int64_t{30}},
 *   };
 *   MockInternalResult result(std::move(rows));
 *
 *   auto [status1, row1] = result.fetchRow();
 *   EXPECT_EQ(status1, InternalStatus::DONE);
 *   EXPECT_EQ(row1->columnString(0), "John");
 *
 *   auto [status2, row2] = result.fetchRow();
 *   EXPECT_EQ(status2, InternalStatus::DONE);
 *
 *   auto [status3, row3] = result.fetchRow();
 *   EXPECT_EQ(row3, nullptr);  // No more rows
 */
class MockInternalResult : public InternalResult {
 public:
  explicit MockInternalResult(std::vector<std::vector<MockColumnValue>> rows)
      : rows_(std::move(rows)), currentIndex_(0), closed_(false) {}

  FetchRowRet fetchRow() override {
    if (closed_ || currentIndex_ >= rows_.size()) {
      return {InternalStatus::DONE, nullptr};
    }

    auto row =
        std::make_unique<MockInternalRow>(std::move(rows_[currentIndex_]));
    ++currentIndex_;
    return {InternalStatus::DONE, std::move(row)};
  }

  size_t numRows() const override {
    return rows_.size();
  }

  void close() override {
    closed_ = true;
  }

  folly::coro::Task<void> co_crank() override {
    co_return;
  }

  // Test helper: check if all rows have been consumed
  bool isExhausted() const {
    return currentIndex_ >= rows_.size();
  }

 private:
  std::vector<std::vector<MockColumnValue>> rows_;
  size_t currentIndex_;
  bool closed_;
};

/**
 * MockPendingResult simulates async behavior by returning PENDING status
 * a configurable number of times before returning data.
 *
 * Useful for testing async/polling logic in the operation classes.
 */
class MockPendingResult : public InternalResult {
 public:
  explicit MockPendingResult(
      std::vector<std::vector<MockColumnValue>> rows,
      int pendingCountPerRow = 1)
      : rows_(std::move(rows)),
        currentIndex_(0),
        pendingCountPerRow_(pendingCountPerRow),
        pendingRemaining_(pendingCountPerRow),
        closed_(false) {}

  FetchRowRet fetchRow() override {
    if (closed_ || currentIndex_ >= rows_.size()) {
      return {InternalStatus::DONE, nullptr};
    }

    // Simulate PENDING status for async operations
    if (pendingRemaining_ > 0) {
      --pendingRemaining_;
      return {InternalStatus::PENDING, nullptr};
    }

    auto row =
        std::make_unique<MockInternalRow>(std::move(rows_[currentIndex_]));
    ++currentIndex_;
    pendingRemaining_ = pendingCountPerRow_;
    return {InternalStatus::DONE, std::move(row)};
  }

  size_t numRows() const override {
    return rows_.size();
  }

  void close() override {
    closed_ = true;
  }

 private:
  std::vector<std::vector<MockColumnValue>> rows_;
  size_t currentIndex_;
  int pendingCountPerRow_;
  int pendingRemaining_;
  bool closed_;
};

} // namespace facebook::common::mysql_client::test
