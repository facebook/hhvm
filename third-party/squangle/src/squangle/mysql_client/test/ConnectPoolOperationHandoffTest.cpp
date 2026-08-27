/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

//
// ConnectPoolOperation handoff handshake tests
//
// A pooled connect operation and the thread that produces its connection agree
// on who completes it, so that the operation -- and the ConnectionContext it
// shares with the ConnectOperation that produced the connection -- is only ever
// touched by one thread. The agreement is a pair of CASes over the operation's
// handoff state: prepWait() arms it, then exactly one of connectionCallback()
// and the owning thread moves it, and whoever moves it owns the completion.
//
// These tests drive that state machine directly. The race it guards against is
// not reproducible on demand, so the invariants are asserted rather than
// stressed. No MySQL instance is involved: the pool and operation are
// constructed but never run.
//

#include <gtest/gtest.h>

#include "squangle/mysql_client/SyncConnectionPool.h"
#include "squangle/mysql_client/SyncMysqlClient.h"
#include "squangle/mysql_client/test/MockConnection.h"

namespace facebook::common::mysql_client::test {

namespace {

class ConnectPoolOperationHandoffTest : public ::testing::Test {
 protected:
  std::shared_ptr<ConnectPoolOperation<SyncMysqlClient>> makeOperation() {
    pool_ = SyncConnectionPool::makePool(std::make_shared<SyncMysqlClient>());
    auto op =
        pool_->beginConnection(MockMysqlClient::createTestConnectionKey());
    return std::dynamic_pointer_cast<ConnectPoolOperation<SyncMysqlClient>>(op);
  }

  std::shared_ptr<SyncConnectionPool> pool_;
};

} // namespace

// An operation that has not armed a wait must never be treated as having one.
// This is what keeps a pool hit -- which reaches connectionCallback() directly
// from registerForConnection(), with no openNewConnectionFinish() to come back
// for it -- from handing off a completion that nobody would ever collect.
TEST_F(ConnectPoolOperationHandoffTest, UnarmedOperationHasNoWaiter) {
  auto op = makeOperation();
  ASSERT_NE(op, nullptr);

  EXPECT_FALSE(op->abandonWait());
}

// prepWait() arms the handshake, and the owning thread can then claim it once.
TEST_F(ConnectPoolOperationHandoffTest, PrepWaitArmsTheHandshake) {
  auto op = makeOperation();
  ASSERT_NE(op, nullptr);

  op->prepWait();
  EXPECT_TRUE(op->abandonWait());

  // Only one side may claim it, so a second attempt loses.
  EXPECT_FALSE(op->abandonWait());
}

// Each connect attempt gets its own handshake.
//
// Regression test. attemptFailed() retries by calling specializedRun() again,
// which runs openNewConnection() and so prepWait() a second time. The handoff
// state used to survive that, so a retry after a timed-out attempt started out
// already Abandoned: the retry's connectionCallback() would find it unarmed,
// conclude that no owner was coming, and complete the operation itself -- on
// the fulfilling thread. That is exactly the cross-thread completion the
// handshake exists to prevent, and it was silent, because completing on the
// wrong thread still produces a correct-looking result.
TEST_F(ConnectPoolOperationHandoffTest, EachAttemptReArmsTheHandshake) {
  auto op = makeOperation();
  ASSERT_NE(op, nullptr);

  // First attempt: armed, then abandoned when the wait times out.
  op->prepWait();
  ASSERT_TRUE(op->abandonWait());

  // The retry must arm again rather than inherit the abandonment.
  op->prepWait();
  EXPECT_TRUE(op->abandonWait())
      << "the retry inherited the previous attempt's handoff state, so its "
         "connection would be handed back on the fulfilling thread";
}

// A retry that lands on a pool hit does not arm, because prepWait() only runs
// on the miss path -- so it must still report no waiter even though an earlier
// attempt armed and resolved one.
TEST_F(ConnectPoolOperationHandoffTest, RetryWithoutPrepWaitHasNoWaiter) {
  auto op = makeOperation();
  ASSERT_NE(op, nullptr);

  op->prepWait();
  ASSERT_TRUE(op->abandonWait());

  EXPECT_FALSE(op->abandonWait());
}

} // namespace facebook::common::mysql_client::test
