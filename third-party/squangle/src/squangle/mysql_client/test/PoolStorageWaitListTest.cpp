/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

//
// PoolStorage wait-list tests
//
// An operation waiting for a pooled connection lives in PoolStorage's wait
// list until exactly one of two things takes it out: popOperation(), when a
// connection becomes available for it, or dequeueOperation(), when its owner
// gives up on it. Callers treat those as mutually exclusive -- dequeueOperation
// returning false is how a timing-out owner learns that another thread has
// already taken the operation and is fulfilling it.
//
// That only holds if both really do remove the entry. These tests pin that.
//

#include <gtest/gtest.h>

#include "squangle/mysql_client/PoolStorage.h"
#include "squangle/mysql_client/SyncConnectionPool.h"
#include "squangle/mysql_client/SyncMysqlClient.h"
#include "squangle/mysql_client/test/MockConnection.h"

namespace facebook::common::mysql_client::test {

namespace {

constexpr size_t kConnLimit = 10;
constexpr auto kMaxIdleTime = std::chrono::seconds(60);

class PoolStorageWaitListTest : public ::testing::Test {
 protected:
  void SetUp() override {
    pool_ = SyncConnectionPool::makePool(std::make_shared<SyncMysqlClient>());
  }

  // Operations are only ever constructed by a pool, but nothing here runs
  // them; they exist to be put in and taken out of the wait list.
  std::shared_ptr<ConnectPoolOperation<SyncMysqlClient>> makeOperation(
      const std::string& db = "test") {
    return std::dynamic_pointer_cast<ConnectPoolOperation<SyncMysqlClient>>(
        pool_->beginConnection(
            MockMysqlClient::createTestConnectionKey(
                "localhost", 3306, db, "test_user")));
  }

  static PoolKey keyFor(ConnectPoolOperation<SyncMysqlClient>& op) {
    return PoolKey(op.getKey(), op.getConnectionOptions());
  }

  std::shared_ptr<SyncConnectionPool> pool_;
};

} // namespace

// Regression test. dequeueOperation() used to search the wait list and report
// whether it found the operation without ever erasing it, despite its name and
// its comment. A timed-out operation therefore stayed queued, and if it went on
// to retry it would be queued a second time -- so the pool could hand it two
// connections, the second of them on whichever thread happened to produce it.
TEST_F(PoolStorageWaitListTest, DequeuedOperationIsRemovedFromTheWaitList) {
  PoolStorageData<SyncMysqlClient> storage(kConnLimit, kMaxIdleTime);
  auto op = makeOperation();
  ASSERT_NE(op, nullptr);
  const auto key = keyFor(*op);

  storage.queueOperation(key, op);
  ASSERT_TRUE(storage.dequeueOperation(key, *op));

  // It is gone, so nothing can hand it a connection afterwards.
  EXPECT_EQ(storage.popOperation(key), nullptr);
}

// The bool is how a timing-out owner decides whether it may act on the
// operation, so a second dequeue of the same operation must report false
// rather than claiming it again.
TEST_F(PoolStorageWaitListTest, DequeueReportsFalseOnceTheEntryIsGone) {
  PoolStorageData<SyncMysqlClient> storage(kConnLimit, kMaxIdleTime);
  auto op = makeOperation();
  ASSERT_NE(op, nullptr);
  const auto key = keyFor(*op);

  storage.queueOperation(key, op);
  ASSERT_TRUE(storage.dequeueOperation(key, *op));
  EXPECT_FALSE(storage.dequeueOperation(key, *op));
}

// An operation already taken by popOperation() -- i.e. another thread is
// fulfilling it -- must report false, which is what tells its owner to leave it
// alone.
TEST_F(PoolStorageWaitListTest, DequeueReportsFalseAfterPop) {
  PoolStorageData<SyncMysqlClient> storage(kConnLimit, kMaxIdleTime);
  auto op = makeOperation();
  ASSERT_NE(op, nullptr);
  const auto key = keyFor(*op);

  storage.queueOperation(key, op);
  ASSERT_EQ(storage.popOperation(key), op);
  EXPECT_FALSE(storage.dequeueOperation(key, *op));
}

// Removing one waiter must not disturb the others queued behind it.
TEST_F(PoolStorageWaitListTest, DequeueLeavesOtherWaitersQueued) {
  PoolStorageData<SyncMysqlClient> storage(kConnLimit, kMaxIdleTime);
  auto first = makeOperation();
  auto second = makeOperation();
  ASSERT_NE(first, nullptr);
  ASSERT_NE(second, nullptr);

  // Both operations share a pool key, so they queue behind each other.
  const auto key = keyFor(*first);
  storage.queueOperation(key, first);
  storage.queueOperation(key, second);

  ASSERT_TRUE(storage.dequeueOperation(key, *first));

  EXPECT_EQ(storage.popOperation(key), second);
  EXPECT_EQ(storage.popOperation(key), nullptr);
}

} // namespace facebook::common::mysql_client::test
