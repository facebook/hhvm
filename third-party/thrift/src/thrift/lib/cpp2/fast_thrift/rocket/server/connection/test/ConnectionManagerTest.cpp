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

#include <gtest/gtest.h>

#include <folly/Executor.h>
#include <folly/SocketAddress.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/portability/GMock.h>
#include <folly/synchronization/Baton.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>

#define private public
#define protected public
#include <thrift/lib/cpp2/fast_thrift/rocket/server/connection/ConnectionManager.h>

namespace apache::thrift::fast_thrift::rocket::server::connection {

using namespace apache::thrift::fast_thrift::channel_pipeline;
using namespace apache::thrift::fast_thrift::channel_pipeline::test;
using MockAppHandler = MockTailHandler; // App receives reads (Tail)
using namespace testing;

class ConnectionManagerTest : public ::testing::Test {
 protected:
  static constexpr size_t kNumIOThreads = 10;

  void SetUp() override {
    connectionManager_ = ConnectionManager::create(
        folly::SocketAddress("::1", 0),
        folly::getKeepAliveToken(executor_),
        createConnectionFactory());
    connectionManager_->start();
  }

  void TearDown() override {
    if (connectionManager_ &&
        connectionManager_->state_ == ConnectionManager::State::STARTED) {
      connectionManager_->stop();
    }
    clientConnections_.clear();
    connectionManager_.reset();
    executor_.join();
  }

  ConnectionFactory createConnectionFactory() {
    return
        [this](folly::AsyncSocket::UniquePtr socket) -> RocketServerConnection {
          auto* evb = socket->getEventBase();
          auto transportHandler =
              transport::TransportHandler::create(std::move(socket));

          auto pipeline = PipelineBuilder<
                              transport::TransportHandler,
                              MockAppHandler,
                              SimpleBufferAllocator>()
                              .setEventBase(evb)
                              .setHead(transportHandler.get())
                              .setTail(&appHandler_)
                              .setAllocator(&allocator_)
                              .build();

          transportHandler->setPipeline(*pipeline);

          if (auto hook = onConnectionCreatedHook_.rlock(); *hook) {
            (*hook)();
          }

          return RocketServerConnection{
              .transportHandler = std::move(transportHandler),
              .pipeline = std::move(pipeline),
              .allocator = {},
          };
        };
  }

  folly::SocketAddress getBoundAddress() {
    folly::SocketAddress address;
    auto handlerMap = connectionManager_->connectionHandlers_.rlock();
    handlerMap->begin()->second->socket_->getAddress(&address);
    return address;
  }

  size_t getConnectionCount() {
    size_t count = 0;
    connectionManager_->connectionHandlers_.withRLock(
        [&count](const auto& handlerMap) {
          for (const auto& [evb, connHandler] : handlerMap) {
            count += connHandler->connectionCount();
          }
        });
    return count;
  }

  // Owns a client socket and its EVB thread. The destructor ensures the
  // socket is destroyed on its own EVB thread (AsyncSocket asserts this).
  struct ClientConnection {
    std::shared_ptr<folly::AsyncSocket> socket;
    std::unique_ptr<folly::ScopedEventBaseThread> thread;

    ClientConnection(
        std::shared_ptr<folly::AsyncSocket> s,
        std::unique_ptr<folly::ScopedEventBaseThread> t)
        : socket(std::move(s)), thread(std::move(t)) {}

    ~ClientConnection() {
      if (thread && socket) {
        thread->getEventBase()->runInEventBaseThreadAndWait(
            [s = std::move(socket)]() mutable { s.reset(); });
      }
    }

    ClientConnection(ClientConnection&&) = default;
    ClientConnection& operator=(ClientConnection&&) = default;
  };

  // Connects a client to the server and waits until the server's
  // connectionAccepted() has fired and the connection factory has run.
  // The client socket and thread are stored in clientConnections_ and
  // cleaned up in TearDown.
  void connectAndWait(const folly::SocketAddress& address) {
    struct ConnectCb : public folly::AsyncSocket::ConnectCallback {
      folly::Baton<>& baton;
      explicit ConnectCb(folly::Baton<>& b) : baton(b) {}
      void connectSuccess() noexcept override { baton.post(); }
      void connectErr(const folly::AsyncSocketException&) noexcept override {}
    };

    folly::Baton<> connected;
    ConnectCb cb(connected);

    folly::Baton<> accepted;
    *onConnectionCreatedHook_.wlock() = [&accepted] { accepted.post(); };

    auto clientThread = std::make_unique<folly::ScopedEventBaseThread>();
    auto* clientEvb = clientThread->getEventBase();
    std::shared_ptr<folly::AsyncSocket> clientSocket;
    clientEvb->runInEventBaseThreadAndWait([&] {
      clientSocket = folly::AsyncSocket::newSocket(clientEvb);
      clientSocket->connect(&cb, address, 1000);
    });

    connected.wait();
    accepted.wait();
    *onConnectionCreatedHook_.wlock() = nullptr;

    clientConnections_.emplace_back(
        std::move(clientSocket), std::move(clientThread));
  }

  ConnectionManager::Ptr connectionManager_;
  MockAppHandler appHandler_;
  SimpleBufferAllocator allocator_;
  folly::IOThreadPoolExecutor executor_{kNumIOThreads};
  std::vector<ClientConnection> clientConnections_;
  folly::Synchronized<std::function<void()>> onConnectionCreatedHook_;
};

TEST_F(ConnectionManagerTest, CreateAndDestroy) {
  EXPECT_NE(connectionManager_, nullptr);
}

TEST_F(ConnectionManagerTest, AcceptConnection) {
  auto serverAddress = getBoundAddress();

  EXPECT_EQ(getConnectionCount(), 0);

  connectAndWait(serverAddress);

  EXPECT_EQ(getConnectionCount(), 1);
}

TEST_F(ConnectionManagerTest, AcceptMultipleConnections) {
  auto serverAddress = getBoundAddress();

  for (int i = 0; i < 3; i++) {
    connectAndWait(serverAddress);
  }

  EXPECT_EQ(getConnectionCount(), 3);
}

TEST_F(ConnectionManagerTest, StopAndRestart) {
  auto serverAddress = getBoundAddress();

  connectAndWait(serverAddress);
  EXPECT_EQ(getConnectionCount(), 1);

  connectionManager_->stop();
  clientConnections_.clear();

  connectionManager_->start();

  auto serverAddress2 = getBoundAddress();
  connectAndWait(serverAddress2);

  EXPECT_GE(getConnectionCount(), 1);
}

TEST_F(ConnectionManagerTest, StopClosesConnections) {
  auto serverAddress = getBoundAddress();

  for (int i = 0; i < 3; i++) {
    connectAndWait(serverAddress);
  }
  EXPECT_EQ(getConnectionCount(), 3);

  connectionManager_->stop();

  auto handlerMap = connectionManager_->connectionHandlers_.rlock();
  EXPECT_TRUE(handlerMap->empty());
}

TEST_F(ConnectionManagerTest, ConnectionHandlerRegistration) {
  folly::SocketAddress address("::1", 0);
  connectionManager_ = ConnectionManager::create(
      address, folly::getKeepAliveToken(executor_), createConnectionFactory());

  // Before start, no handlers should be registered
  {
    auto handlerMap = connectionManager_->connectionHandlers_.rlock();
    EXPECT_TRUE(handlerMap->empty());
  }

  // Start the server
  connectionManager_->start();

  // Verify each IO thread has its own ConnectionHandler
  {
    auto handlerMap = connectionManager_->connectionHandlers_.rlock();
    EXPECT_EQ(handlerMap->size(), kNumIOThreads);
  }
}

TEST_F(ConnectionManagerTest, DestructorStopsServer) {
  folly::SocketAddress address("::1", 0);
  auto manager = ConnectionManager::create(
      address, folly::getKeepAliveToken(executor_), createConnectionFactory());
  manager->start();

  // Verify handlers are registered
  {
    auto handlerMap = manager->connectionHandlers_.rlock();
    EXPECT_FALSE(handlerMap->empty());
  }

  // Destroy the manager - should auto-stop via onDelayedDestroy
  manager.reset();
}

} // namespace apache::thrift::fast_thrift::rocket::server::connection
