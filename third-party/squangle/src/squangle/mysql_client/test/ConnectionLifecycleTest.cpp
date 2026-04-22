/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

//
// Connection Lifecycle Tests
//
// Tests the Connection class (production code) through MockConnection,
// exercising the lifecycle from creation through initialization, state
// management, and teardown. These tests verify that Connection methods
// correctly delegate through ConnectionHolder to InternalConnection,
// and that edge cases around null connections are handled properly.
//
// MockInternalConnection-only tests live in MockConnectionTest.cpp.
// ConnectionOptions getter/setter tests are in SquangleUnitTest.cpp.
//

#include <gtest/gtest.h>

#include "squangle/mysql_client/test/MockConnection.h"

namespace facebook::common::mysql_client::test {

class MockConnectionLifecycleTest : public ::testing::Test {
 protected:
  void SetUp() override {
    client_ = std::make_shared<MockMysqlClient>();
    connKey_ = MockMysqlClient::createTestConnectionKey(
        "testhost", 3307, "testdb", "testuser");
  }

  std::unique_ptr<MockConnection> createAndInit(
      MockConnectionConfig config = {}) {
    auto conn = client_->createMockConnection(connKey_, std::move(config));
    conn->initialize();
    return conn;
  }

  std::shared_ptr<MockMysqlClient> client_;
  std::shared_ptr<MysqlConnectionKey> connKey_;
};

TEST_F(MockConnectionLifecycleTest, InitializeWiresInternalConnection) {
  auto conn = client_->createMockConnection(connKey_);

  // Before initialize, mock internal connection is not yet created
  EXPECT_EQ(conn->getMockInternalConnection(), nullptr);
  EXPECT_FALSE(conn->hasInitialized());
  EXPECT_FALSE(conn->ok());

  conn->initialize();

  // After initialize, internal connection is wired in
  EXPECT_NE(conn->getMockInternalConnection(), nullptr);
  EXPECT_TRUE(conn->hasInitialized());
  EXPECT_TRUE(conn->ok());
}

TEST_F(MockConnectionLifecycleTest, ServerInfoFlowsThroughConnectionHolder) {
  MockConnectionConfig config;
  config.serverInfo = "MockMySQL 8.0.99-test";

  auto conn = createAndInit(std::move(config));

  // Connection::serverInfo() delegates through ConnectionHolder to
  // InternalConnection::serverInfo()
  EXPECT_EQ(conn->serverInfo(), "MockMySQL 8.0.99-test");
}

TEST_F(MockConnectionLifecycleTest, CloseResetsOk) {
  auto conn = createAndInit();
  EXPECT_TRUE(conn->ok());

  conn->close();
  EXPECT_FALSE(conn->ok());

  // serverInfo returns empty after close
  EXPECT_EQ(conn->serverInfo(), "");
}

TEST_F(MockConnectionLifecycleTest, WarningCountFlowsThrough) {
  MockConnectionConfig config;
  config.queryResults["SELECT 1"] = MockQueryResult{
      .fields = {{"1", "", MYSQL_TYPE_LONG, 0}},
      .rows = {{int64_t{1}}},
      .warningCount = 5,
  };

  auto conn = createAndInit(std::move(config));

  // Run query through mock internal connection to set warningCount
  auto* mock = conn->getMockInternalConnection();
  mock->runQuery("SELECT 1");

  // Connection::warningCount() delegates through ConnectionHolder
  EXPECT_EQ(conn->warningCount(), 5);
}

TEST_F(
    MockConnectionLifecycleTest,
    ReusableStateDelegatesToInternalConnection) {
  auto conn = createAndInit();

  // Default: connection is reusable
  EXPECT_TRUE(conn->isReusable());

  // Connection::setReusable delegates through ConnectionHolder
  conn->setReusable(false);
  EXPECT_FALSE(conn->isReusable());

  conn->setReusable(true);
  EXPECT_TRUE(conn->isReusable());
}

TEST_F(MockConnectionLifecycleTest, SetCurrentSchemaMarksNonReusable) {
  auto conn = createAndInit();
  EXPECT_TRUE(conn->isReusable());

  // setCurrentSchema marks the connection as non-reusable so it won't be
  // returned to the pool after a "USE <dbname>" command
  conn->setCurrentSchema("other_db");
  EXPECT_FALSE(conn->isReusable());
}

TEST_F(MockConnectionLifecycleTest, IsReusableReturnsFalseAfterClose) {
  auto conn = createAndInit();
  EXPECT_TRUE(conn->isReusable());

  conn->close();

  // isReusable() has a null-check guard — returns false when connection is gone
  EXPECT_FALSE(conn->isReusable());
}

TEST_F(MockConnectionLifecycleTest, SetReusableIsNoOpAfterClose) {
  auto conn = createAndInit();
  conn->close();

  // setReusable() silently no-ops when mysql_connection_ is null
  // (no crash, no exception)
  conn->setReusable(true);
  EXPECT_FALSE(conn->isReusable());
}

TEST_F(MockConnectionLifecycleTest, ConnectionDyingCallbackFiresOnDestruction) {
  bool callbackFired = false;
  std::unique_ptr<ConnectionHolder> recycledHolder;

  {
    auto conn = createAndInit();
    conn->setConnectionDyingCallback(
        [&](std::unique_ptr<ConnectionHolder> holder) {
          callbackFired = true;
          recycledHolder = std::move(holder);
        });
  } // conn destroyed here

  EXPECT_TRUE(callbackFired);
  EXPECT_NE(recycledHolder, nullptr);
}

TEST_F(MockConnectionLifecycleTest, ConnectionDyingCallbackNotFiredAfterClose) {
  bool callbackFired = false;

  {
    auto conn = createAndInit();
    conn->setConnectionDyingCallback(
        [&](std::unique_ptr<ConnectionHolder>) { callbackFired = true; });
    conn->close();
  } // conn destroyed here, but holder is already gone

  EXPECT_FALSE(callbackFired);
}

TEST_F(MockConnectionLifecycleTest, MultipleConnectionsShareClient) {
  auto conn1 = createAndInit();
  auto conn2 = createAndInit();

  // Each connection gets its own MockInternalConnection
  EXPECT_NE(conn1->getMockInternalConnection(), nullptr);
  EXPECT_NE(conn2->getMockInternalConnection(), nullptr);
  EXPECT_NE(
      conn1->getMockInternalConnection(), conn2->getMockInternalConnection());

  // Both share the same client
  EXPECT_EQ(&conn1->client(), &conn2->client());
}

} // namespace facebook::common::mysql_client::test
