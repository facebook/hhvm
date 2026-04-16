/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

#include "squangle/base/ConnectionKey.h"
#include "squangle/mysql_client/SyncMysqlClient.h"
#include "squangle/mysql_client/test/MockInternalConnection.h"

namespace facebook::common::mysql_client::test {

/**
 * MockConnection is a test Connection that uses MockInternalConnection
 * instead of a real MySQL connection.
 *
 * This allows testing Connection methods and Operation classes without
 * requiring a MySQL server.
 *
 * Usage:
 *   // Create a mock client
 *   auto client = std::make_shared<MockMysqlClient>();
 *
 *   // Configure mock connection behavior
 *   MockConnectionConfig mockConfig;
 *   mockConfig.queryResults["SELECT 1"] = MockQueryResult{
 *       .rows = {{int64_t{1}}},
 *   };
 *
 *   // Create connection with mock config
 *   auto conn = client->createConnection(connKey, mockConfig);
 *
 *   // Use connection normally - it will use mock data
 *   auto result = conn->query("SELECT 1");
 */

class MockMysqlClient;

/**
 * MockConnection overrides createInternalConnection() to inject mock behavior.
 */
class MockConnection : public SyncConnection {
 public:
  MockConnection(
      MysqlClientBase& client,
      std::shared_ptr<const ConnectionKey> conn_key,
      MockConnectionConfig mockConfig)
      : SyncConnection(client, std::move(conn_key)),
        mockConfig_(std::move(mockConfig)) {}

  // Access the mock connection for test verification
  MockInternalConnection* getMockInternalConnection() {
    return mockInternalConn_;
  }

  const MockInternalConnection* getMockInternalConnection() const {
    return mockInternalConn_;
  }

  // Convenience method to get the last executed query
  const std::string& lastQuery() const {
    if (mockInternalConn_) {
      return mockInternalConn_->lastQuery();
    }
    static const std::string empty;
    return empty;
  }

 protected:
  std::unique_ptr<InternalConnection> createInternalConnection() override {
    auto mockConn = std::make_unique<MockInternalConnection>(mockConfig_);
    mockInternalConn_ = mockConn.get();
    return mockConn;
  }

 private:
  MockConnectionConfig mockConfig_;
  MockInternalConnection* mockInternalConn_ = nullptr;
};

/**
 * MockMysqlClient is a test client that creates MockConnection instances.
 *
 * This is the main entry point for mock-based testing.
 */
class MockMysqlClient : public SyncMysqlClient {
 public:
  MockMysqlClient() : SyncMysqlClient() {}

  /**
   * Creates a MockConnection with the given configuration.
   *
   * The connection can be used to test Operation classes without MySQL.
   */
  std::unique_ptr<MockConnection> createMockConnection(
      std::shared_ptr<const ConnectionKey> conn_key,
      MockConnectionConfig mockConfig = {}) {
    return std::make_unique<MockConnection>(
        *this, std::move(conn_key), std::move(mockConfig));
  }

  /**
   * Creates a ConnectionKey for testing purposes.
   */
  static std::shared_ptr<MysqlConnectionKey> createTestConnectionKey(
      const std::string& host = "localhost",
      int port = 3306,
      const std::string& db = "test",
      const std::string& user = "test_user") {
    return std::make_shared<MysqlConnectionKey>(host, port, db, user);
  }
};

} // namespace facebook::common::mysql_client::test
