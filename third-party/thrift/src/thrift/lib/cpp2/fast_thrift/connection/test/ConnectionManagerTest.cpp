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

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <vector>

#include <folly/Executor.h>
#include <folly/SocketAddress.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/init/Init.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/synchronization/Baton.h>
#include <thrift/lib/cpp2/fast_thrift/connection/ConnectionManager.h>

namespace apache::thrift::fast_thrift::connection {
namespace {

// Test connection — keeps the transport alive, bumps a shared close
// counter on teardown, and treats drain() as immediate close (firing
// the close callback so the handler can erase the entry).
struct TestConnection {
  folly::AsyncTransport::UniquePtr transport;
  std::shared_ptr<std::atomic<size_t>> closeCount;
  std::function<void()> closeCb;
  bool closed{false};

  void setCloseCallback(std::function<void()> cb) { closeCb = std::move(cb); }

  void drain() noexcept { close(); }

  void close() noexcept {
    if (closed) {
      return;
    }
    closed = true;
    if (transport) {
      transport->closeNow();
    }
    if (closeCount) {
      closeCount->fetch_add(1, std::memory_order_relaxed);
    }
    if (closeCb) {
      auto cb = std::move(closeCb);
      cb();
    }
  }
};

class TestConnectionFactory {
 public:
  explicit TestConnectionFactory(
      std::shared_ptr<std::atomic<size_t>> closeCount) noexcept
      : closeCount_(std::move(closeCount)) {}

  TestConnection getConnection(folly::AsyncTransport::UniquePtr socket) {
    return TestConnection{
        .transport = std::move(socket),
        .closeCount = closeCount_,
        .closeCb = {}};
  }

 private:
  std::shared_ptr<std::atomic<size_t>> closeCount_;
};

} // namespace

class ConnectionManagerTest : public ::testing::Test {
 protected:
  static constexpr size_t kNumIOThreads = 10;

  void SetUp() override {
    connectionManager_ = ConnectionManager::create(
        folly::SocketAddress("::1", 0),
        folly::getKeepAliveToken(executor_),
        fast_security::SSLPolicy::DISABLED,
        /*tlsParams=*/nullptr,
        SocketOptions{});
    connectionManager_->setConnectionFactory(
        TestConnectionFactory{closeCount_},
        [this](TestConnection&) noexcept { acceptCount_.fetch_add(1); });
    connectionManager_->start();
  }

  void TearDown() override {
    clientConnections_.clear();
    connectionManager_.reset();
    executor_.join();
  }

  // Owns a client socket + its EVB thread. Dtor destroys the socket on its
  // own EVB thread (AsyncSocket asserts this).
  struct ClientConnection {
    std::shared_ptr<folly::AsyncSocket> socket;
    std::unique_ptr<folly::ScopedEventBaseThread> thread;

    ClientConnection() = default;
    ClientConnection(ClientConnection&&) = default;
    ClientConnection& operator=(ClientConnection&&) = default;

    ~ClientConnection() {
      if (thread && socket) {
        thread->getEventBase()->runInEventBaseThreadAndWait(
            [s = std::move(socket)]() mutable { s.reset(); });
      }
    }
  };

  // Connects a client to the server and waits until the server's accept
  // callback has fired and the factory has produced a connection.
  void connectAndWait(const folly::SocketAddress& address) {
    struct ConnectCb : folly::AsyncSocket::ConnectCallback {
      folly::Baton<>& baton;
      explicit ConnectCb(folly::Baton<>& b) : baton(b) {}
      void connectSuccess() noexcept override { baton.post(); }
      void connectErr(const folly::AsyncSocketException&) noexcept override {}
    };

    const size_t before = acceptCount_.load();

    folly::Baton<> connected;
    ConnectCb cb(connected);
    auto clientThread = std::make_unique<folly::ScopedEventBaseThread>();
    auto* clientEvb = clientThread->getEventBase();
    std::shared_ptr<folly::AsyncSocket> clientSocket;
    clientEvb->runInEventBaseThreadAndWait([&] {
      clientSocket = folly::AsyncSocket::newSocket(clientEvb);
      clientSocket->connect(&cb, address, 1000);
    });
    connected.wait();

    const auto deadline =
        std::chrono::steady_clock::now() + std::chrono::seconds{5};
    while (acceptCount_.load() == before) {
      ASSERT_LT(std::chrono::steady_clock::now(), deadline)
          << "timed out waiting for server to accept connection";
      // Short poll; server-side accept is asynchronous and has no synchronous
      // hand-off, so we wait on its observable side-effect (acceptCount_).
      // NOLINTNEXTLINE(facebook-hte-BadCall-sleep_for)
      std::this_thread::sleep_for(std::chrono::milliseconds{5});
    }

    ClientConnection cc;
    cc.socket = std::move(clientSocket);
    cc.thread = std::move(clientThread);
    clientConnections_.push_back(std::move(cc));
  }

  folly::IOThreadPoolExecutor executor_{kNumIOThreads};
  ConnectionManager::Ptr connectionManager_;
  std::shared_ptr<std::atomic<size_t>> closeCount_ =
      std::make_shared<std::atomic<size_t>>(0);
  std::atomic<size_t> acceptCount_{0};
  std::vector<ClientConnection> clientConnections_;
};

TEST_F(ConnectionManagerTest, CreateAndDestroy) {
  EXPECT_NE(connectionManager_, nullptr);
}

TEST_F(ConnectionManagerTest, AcceptConnection) {
  auto addr = connectionManager_->getAddress();
  EXPECT_EQ(connectionManager_->connectionCount(), 0);
  connectAndWait(addr);
  EXPECT_EQ(connectionManager_->connectionCount(), 1);
}

TEST_F(ConnectionManagerTest, AcceptMultipleConnections) {
  auto addr = connectionManager_->getAddress();
  for (int i = 0; i < 3; ++i) {
    connectAndWait(addr);
  }
  EXPECT_EQ(connectionManager_->connectionCount(), 3);
}

TEST_F(ConnectionManagerTest, StopClosesConnections) {
  auto addr = connectionManager_->getAddress();
  for (int i = 0; i < 3; ++i) {
    connectAndWait(addr);
  }
  EXPECT_EQ(connectionManager_->connectionCount(), 3);

  connectionManager_->stop();

  // stop() drains every handler then tears down the IOObserver, which
  // fans unregisterEventBase across every IO thread; each unregister
  // drops the handler from handlers_.
  EXPECT_EQ(connectionManager_->numHandlers(), 0);
  EXPECT_EQ(closeCount_->load(), 3);
}

TEST_F(ConnectionManagerTest, ConnectionHandlerRegistration) {
  // Per-IO-thread ConnectionHandler should be registered after start().
  EXPECT_EQ(connectionManager_->numHandlers(), kNumIOThreads);
}

TEST_F(ConnectionManagerTest, DestructorStopsServer) {
  EXPECT_GT(connectionManager_->numHandlers(), 0);
  // Dtor should clean up without manual stop().
  connectionManager_.reset();
}

} // namespace apache::thrift::fast_thrift::connection

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  folly::Init init(&argc, &argv);
  return RUN_ALL_TESTS();
}
