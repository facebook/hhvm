/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <wangle/acceptor/ConnectionManager.h>

#include <folly/portability/GFlags.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

using namespace testing;
using namespace wangle;

namespace {

class ConnectionManagerTest;

class MockConnection : public ManagedConnection {
 public:
  using Mock = StrictMock<MockConnection>;
  using UniquePtr = folly::DelayedDestructionUniquePtr<Mock>;

  static UniquePtr makeUnique(ConnectionManagerTest* test) {
    UniquePtr p(new StrictMock<MockConnection>(test));
    return p;
  }

  explicit MockConnection(ConnectionManagerTest* test) : test_(test) {
    EXPECT_CALL(*this, isBusy()).WillRepeatedly(Return(false));
    EXPECT_CALL(*this, dumpConnectionState(testing::_))
        .WillRepeatedly(Return());
    ON_CALL(*this, closeWhenIdle()).WillByDefault(Invoke([this] {
      closeWhenIdle_ = true;
      closeWhenIdleImpl();
    }));
  }

  MOCK_METHOD0(timeoutExpired_, void());
  void timeoutExpired() noexcept override {
    timeoutExpired_();
  }

  MOCK_CONST_METHOD1(describe, void(std::ostream& os));

  MOCK_CONST_METHOD0(isBusy, bool());

  MOCK_CONST_METHOD0(getIdleTime, std::chrono::milliseconds());
  MOCK_CONST_METHOD0(
      getLastActivityElapsedTime,
      folly::Optional<std::chrono::milliseconds>());

  MOCK_METHOD0(notifyPendingShutdown, void());
  MOCK_METHOD0(closeWhenIdle, void());
  MOCK_METHOD1(dropConnection, void(const std::string&));
  MOCK_METHOD1(dumpConnectionState, void(uint8_t));
  MOCK_METHOD2(drainConnections, void(double, std::chrono::milliseconds));

  void setIdle(bool idle) {
    idle_ = idle;
    closeWhenIdleImpl();
  }

  void closeWhenIdleImpl();

  ConnectionManagerTest* test_{nullptr};
  bool idle_{false};
  bool closeWhenIdle_{false};
};

class ConnectionManagerTest : public testing::Test {
 public:
  ConnectionManagerTest() {
    cm_ = ConnectionManager::makeUnique(
        &eventBase_, std::chrono::milliseconds(100), nullptr);
    addConns(65);
  }

  void SetUp() override {}

  void addConns(uint64_t n) {
    for (size_t i = 0; i < n; i++) {
      conns_.insert(conns_.begin(), MockConnection::makeUnique(this));
      cm_->addConnection(conns_.front().get());
    }
  }

  void removeConn(MockConnection* connection) {
    for (auto& conn : conns_) {
      if (conn.get() == connection) {
        cm_->removeConnection(connection);
        conn.reset();
      }
    }
  }

 protected:
  void testAddDuringCloseWhenIdle(bool deactivate);

  folly::EventBase eventBase_;
  ConnectionManager::UniquePtr cm_;
  std::vector<MockConnection::UniquePtr> conns_;
};

void MockConnection::closeWhenIdleImpl() {
  if (idle_ && closeWhenIdle_) {
    test_->removeConn(this);
  }
}

TEST_F(ConnectionManagerTest, testShutdownSequence) {
  InSequence enforceOrder;

  // activate one connection, it should not be exempt from notifyPendingShutdown
  cm_->onActivated(*conns_.front());
  // make sure the idleIterator points to !end
  cm_->onDeactivated(*conns_.back());
  for (const auto& conn : conns_) {
    EXPECT_CALL(*conn, notifyPendingShutdown());
  }
  cm_->initiateGracefulShutdown(std::chrono::milliseconds(50));
  eventBase_.loopOnce();
  for (const auto& conn : conns_) {
    EXPECT_CALL(*conn, closeWhenIdle());
  }

  eventBase_.loop();
}

TEST_F(ConnectionManagerTest, testRemoveDrainIterator) {
  addConns(1);
  InSequence enforceOrder;

  // activate one connection, it should not be exempt from notifyPendingShutdown
  cm_->onActivated(*conns_.front());
  for (size_t i = 0; i < conns_.size() - 1; i++) {
    EXPECT_CALL(*conns_[i], notifyPendingShutdown());
  }
  auto conn65 = conns_[conns_.size() - 2].get();
  auto conn66 = conns_[conns_.size() - 1].get();
  eventBase_.runInLoop([&] {
    // deactivate the drain iterator
    cm_->onDeactivated(*conn65);
    // remove the drain iterator
    cm_->removeConnection(conn66);
    // deactivate the new drain iterator, now it's the end of the list
    cm_->onDeactivated(*conn65);
  });
  cm_->initiateGracefulShutdown(std::chrono::milliseconds(50));
  // Schedule a loop callback to remove the connection pointed to by the drain
  // iterator
  eventBase_.loopOnce();
  for (size_t i = 0; i < conns_.size() - 1; i++) {
    EXPECT_CALL(*conns_[i], closeWhenIdle());
  }

  eventBase_.loop();
}

TEST_F(ConnectionManagerTest, testIdleGraceTimeout) {
  InSequence enforceOrder;

  // Slow down the notifyPendingShutdown calls enough so that the idle grace
  // timeout fires before the end of the loop.
  // I would prefer a non-sleep solution to this, but I can't think how to do it
  // without changing the class to expose internal details
  for (const auto& conn : conns_) {
    EXPECT_CALL(*conn, notifyPendingShutdown()).WillOnce(Invoke([] {
      /* sleep override */
      usleep(1000);
    }));
  }
  cm_->initiateGracefulShutdown(std::chrono::milliseconds(1));
  eventBase_.loopOnce();
  for (const auto& conn : conns_) {
    EXPECT_CALL(*conn, closeWhenIdle());
  }

  eventBase_.loop();
}

TEST_F(ConnectionManagerTest, testDropAll) {
  InSequence enforceOrder;

  for (const auto& conn : conns_) {
    EXPECT_CALL(*conn, dropConnection(_))
        .WillOnce(Invoke(
            [&](const std::string&) { cm_->removeConnection(conn.get()); }));
  }
  cm_->dropAllConnections();
}

TEST_F(ConnectionManagerTest, testDropPercent) {
  InSequence enforceOrder;

  // Make sure we have exactly 100 connections.
  const size_t numToAdd = 100 - conns_.size();
  addConns(numToAdd);
  const size_t numToRemove = conns_.size() - 100;
  for (size_t i = 0; i < numToRemove; i++) {
    removeConn(conns_.begin()->get());
  }
  EXPECT_EQ(100, cm_->getNumConnections());

  // Drop 20% of connections.
  double pct = 0.2;
  int numToDrop = 100 * pct;
  auto connIter = conns_.begin();
  while (connIter != conns_.end() && numToDrop > 0) {
    EXPECT_CALL(*(*connIter), dropConnection(_));
    --numToDrop;
    ++connIter;
  }
  cm_->dropConnections(pct);

  // Make sure they are gone.
  EXPECT_EQ(0, numToDrop);
  EXPECT_EQ(80, cm_->getNumConnections());

  // Then drop 50% of the remaining 80 connections.
  pct = 0.5;
  numToDrop = 80 * pct;
  while (connIter != conns_.end() && numToDrop > 0) {
    EXPECT_CALL(*(*connIter), dropConnection(_));
    --numToDrop;
    ++connIter;
  }
  cm_->dropConnections(pct);

  // Make sure those are gone as well.
  EXPECT_EQ(0, numToDrop);
  EXPECT_EQ(40, cm_->getNumConnections());
}

TEST_F(ConnectionManagerTest, testDrainPercent) {
  InSequence enforceOrder;
  double drain_percentage = .123;

  for (size_t i = 58 /* tail .123 of all conns */; i < conns_.size(); ++i) {
    EXPECT_CALL(*conns_[i], notifyPendingShutdown());
  }

  cm_->drainConnections(drain_percentage, std::chrono::milliseconds(50));

  for (size_t i = 58; i < conns_.size(); ++i) {
    EXPECT_CALL(*conns_[i], closeWhenIdle());
  }

  eventBase_.loop();
}

TEST_F(ConnectionManagerTest, testDrainPctAfterAll) {
  InSequence enforceOrder;
  double drain_percentage = 0.1;

  for (const auto& conn : conns_) {
    EXPECT_CALL(*conn, notifyPendingShutdown());
  }

  cm_->initiateGracefulShutdown(std::chrono::milliseconds(50));
  cm_->drainConnections(drain_percentage, std::chrono::milliseconds(50));
  eventBase_.loopOnce();

  for (const auto& conn : conns_) {
    EXPECT_CALL(*conn, closeWhenIdle());
  }

  eventBase_.loop();
}

TEST_F(ConnectionManagerTest, testDrainAllAfterPct) {
  InSequence enforceOrder;
  double drain_pct = 0.8;

  for (auto i = conns_.size() - static_cast<int>(conns_.size() * drain_pct);
       i < conns_.size();
       ++i) {
    EXPECT_CALL(*conns_[i], notifyPendingShutdown());
  }

  cm_->drainConnections(drain_pct, std::chrono::milliseconds(50));

  for (size_t i = 0;
       i < conns_.size() - static_cast<size_t>(conns_.size() * drain_pct);
       ++i) {
    EXPECT_CALL(*conns_[i], notifyPendingShutdown());
  }

  cm_->initiateGracefulShutdown(std::chrono::milliseconds(50));
  eventBase_.loopOnce();

  for (const auto& conn : conns_) {
    EXPECT_CALL(*conn, closeWhenIdle());
  }

  eventBase_.loop();
}

TEST_F(ConnectionManagerTest, testDropIdle) {
  for (const auto& conn : conns_) {
    // Set everyone to be idle for 100ms
    EXPECT_CALL(*conn, getIdleTime())
        .WillRepeatedly(Return(std::chrono::milliseconds(100)));
  }

  // Mark the first half of the connections idle
  for (size_t i = 0; i < conns_.size() / 2; i++) {
    cm_->onDeactivated(*conns_[i]);
  }
  // reactivate conn 0
  cm_->onActivated(*conns_[0]);
  // remove the first idle conn
  cm_->removeConnection(conns_[1].get());

  InSequence enforceOrder;

  // Expect the remaining idle conns to drop
  for (size_t i = 2; i < conns_.size() / 2; i++) {
    EXPECT_CALL(*conns_[i], dropConnection(_))
        .WillOnce(Invoke([this, i](const std::string&) {
          cm_->removeConnection(conns_[i].get());
        }));
  }

  cm_->dropIdleConnections(conns_.size());
}

TEST_F(ConnectionManagerTest, testDropActive) {
  for (const auto& conn : conns_) {
    // Set everyone to be idle for 100ms
    EXPECT_CALL(*conn, getLastActivityElapsedTime())
        .WillRepeatedly(Return(std::chrono::milliseconds(100)));
  }

  // Mark the first half of the connections idle
  for (size_t i = 0; i < conns_.size(); i++) {
    if (i < conns_.size() / 2) {
      cm_->onDeactivated(*conns_[i]);
    } else {
      cm_->onActivated(*conns_[i]);
    }
  }
  // remove the first idle conn
  cm_->removeConnection(conns_[1].get());

  InSequence enforceOrder;

  // Expect all active connections to be dropped
  for (size_t i = conns_.size() / 2; i < conns_.size(); i++) {
    EXPECT_CALL(*conns_[i], dropConnection(_))
        .WillOnce(Invoke([this, i](const std::string&) {
          cm_->removeConnection(conns_[i].get());
        }));
  }
  cm_->dropActiveConnections(conns_.size(), std::chrono::milliseconds(50));
}

TEST_F(ConnectionManagerTest, testAddDuringShutdown) {
  auto extraConn = MockConnection::makeUnique(this);
  InSequence enforceOrder;

  // activate one connection, it should not be exempt from notifyPendingShutdown
  cm_->onActivated(*conns_.front());
  for (const auto& conn : conns_) {
    EXPECT_CALL(*conn, notifyPendingShutdown());
  }
  cm_->initiateGracefulShutdown(std::chrono::milliseconds(50));
  eventBase_.loopOnce();
  conns_.insert(conns_.begin(), std::move(extraConn));
  EXPECT_CALL(*conns_.front(), notifyPendingShutdown());
  cm_->addConnection(conns_.front().get());
  for (const auto& conn : conns_) {
    EXPECT_CALL(*conn, closeWhenIdle());
  }

  eventBase_.loop();
}

TEST_F(ConnectionManagerTest, testAddDuringShutdownWithoutIdleGrace) {
  auto extraConn = MockConnection::makeUnique(this);
  InSequence enforceOrder;

  cm_->onActivated(*conns_.front());
  for (const auto& conn : conns_) {
    EXPECT_CALL(*conn, closeWhenIdle());
  }
  cm_->initiateGracefulShutdown(std::chrono::milliseconds(0));
  eventBase_.loopOnce();

  conns_.insert(conns_.begin(), std::move(extraConn));
  EXPECT_CALL(*conns_.front().get(), closeWhenIdle());
  cm_->addConnection(conns_.front().get());
  eventBase_.loop();
}

void ConnectionManagerTest::testAddDuringCloseWhenIdle(bool deactivate) {
  auto extraConn = MockConnection::makeUnique(this);
  InSequence enforceOrder;

  // All conns will get closeWhenIdle
  for (const auto& conn : conns_) {
    conn->setIdle(true);
    EXPECT_CALL(*conn, closeWhenIdle());
  }
  cm_->initiateGracefulShutdown(std::chrono::milliseconds(0));
  // Add the extra conn in this state
  extraConn->setIdle(true);
  conns_.insert(conns_.begin(), std::move(extraConn));
  cm_->addConnection(conns_.front().get());
  // Shouldn't be deleted yet, call is delayed
  ASSERT_TRUE(conns_.front().get() != nullptr);

  // Mark the connection as active
  conns_.front()->setIdle(false);
  if (deactivate) {
    // mark it idle and move to the end of the list.  The regular
    // drainAllConnections code will find it and call closeWhenIdle.  The
    // second loop callback won't find the conn and be a no-op
    cm_->onDeactivated(*conns_.front());
    conns_.front()->setIdle(true);
  }
  EXPECT_CALL(*conns_.front(), closeWhenIdle());
  eventBase_.loop();
  if (!deactivate) {
    // drainAllConnections didn't find it, closeWhenIdle was invoked by the
    // second loop callback.
    cm_->onDeactivated(*conns_.front());
    conns_.front()->setIdle(true);
  }
  ASSERT_TRUE(conns_.front().get() == nullptr);
}

TEST_F(ConnectionManagerTest, testAddDuringCloseWhenIdleActive) {
  testAddDuringCloseWhenIdle(false);
}

TEST_F(ConnectionManagerTest, testAddDuringCloseWhenIdleInactive) {
  testAddDuringCloseWhenIdle(true);
}
} // namespace
