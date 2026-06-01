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

#include <folly/SocketAddress.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/portability/GMock.h>
#include <folly/synchronization/Baton.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>

#define private public
#define protected public
#include <thrift/lib/cpp2/fast_thrift/rocket/server/connection/ConnectionHandler.h>

namespace apache::thrift::fast_thrift::rocket::server::connection {

using namespace apache::thrift::fast_thrift::channel_pipeline;
using namespace apache::thrift::fast_thrift::channel_pipeline::test;
using MockAppHandler = MockHeadHandler;
using namespace testing;

class MockTransportHandler : public folly::DelayedDestruction {
 public:
  using Ptr = std::
      unique_ptr<MockTransportHandler, folly::DelayedDestruction::Destructor>;

  static Ptr create(folly::AsyncTransport::UniquePtr /* socket */) {
    return Ptr(new MockTransportHandler());
  }

  void setPipeline(PipelineImpl& pipeline) { pipeline_ = &pipeline; }

  void pauseRead() noexcept { readPaused_ = true; }

  void resumeRead() noexcept { readPaused_ = false; }

  // Test hook: called from onConnect() to signal that the server has
  // accepted a connection. Used by connectAndWait() for synchronization.
  static inline folly::Synchronized<std::function<void()>> onConnectHook_;

  void onConnect() noexcept {
    connectCalled_ = true;
    resumeRead();
    if (auto hook = onConnectHook_.rlock(); *hook) {
      (*hook)();
    }
  }

  bool readPaused() const { return readPaused_; }

  bool connectCalled() const { return connectCalled_; }

  apache::thrift::fast_thrift::channel_pipeline::Result onMessage(
      apache::thrift::fast_thrift::channel_pipeline::
          TypeErasedBox&& /* msg */) noexcept {
    return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
  }

  void onException(folly::exception_wrapper&&) noexcept {}

  void onClose(folly::exception_wrapper&& /* ew */) noexcept {
    closeCalled_ = true;
    if (closeCallback_) {
      closeCallback_();
    }
  }

  void setCloseCallback(std::function<void()> callback) {
    closeCallback_ = std::move(callback);
  }

  bool closeCalled() const { return closeCalled_; }

  PipelineImpl* pipeline() const { return pipeline_; }

 private:
  MockTransportHandler() = default;

  PipelineImpl* pipeline_{nullptr};
  bool readPaused_{true};
  bool closeCalled_{false};
  bool connectCalled_{false};
  std::function<void()> closeCallback_;
};

using TestConnectionHandler = ConnectionHandler<MockTransportHandler>;
using TestConnectionHandlerPtr = std::
    unique_ptr<TestConnectionHandler, folly::DelayedDestruction::Destructor>;

class ConnectionHandlerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    evbThread_ = std::make_unique<folly::ScopedEventBaseThread>();
    evb_ = evbThread_->getEventBase();
  }

  void TearDown() override {
    clientConnections_.clear();
    evbThread_.reset();
  }

  PipelineFactory<MockTransportHandler> createPipelineFactory() {
    return [this](
               folly::EventBase* evb,
               MockTransportHandler* transportHandler) -> PipelineImpl::Ptr {
      return PipelineBuilder<
                 MockTransportHandler,
                 MockAppHandler,
                 SimpleBufferAllocator>()
          .setEventBase(evb)
          .setHead(transportHandler)
          .setTail(&appHandler_)
          .setAllocator(&allocator_)
          .setHeadToTailOp(HeadToTailOp::Read)
          .build();
    };
  }

  TestConnectionHandlerPtr createConnectionHandler() {
    return TestConnectionHandlerPtr(
        new TestConnectionHandler(*evb_, createPipelineFactory()));
  }

  std::pair<folly::NetworkSocket, folly::NetworkSocket> createSocketPair() {
    std::array<int, 2> sockets{};
    int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, sockets.data());
    EXPECT_EQ(ret, 0) << "Failed to create socket pair";
    return {folly::NetworkSocket(sockets[0]), folly::NetworkSocket(sockets[1])};
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
  // connectionAccepted() -> onConnect() has fired, using onConnectHook_.
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
    *MockTransportHandler::onConnectHook_.wlock() = [&accepted] {
      accepted.post();
    };

    auto clientThread = std::make_unique<folly::ScopedEventBaseThread>();
    auto* clientEvb = clientThread->getEventBase();
    std::shared_ptr<folly::AsyncSocket> clientSocket;
    clientEvb->runInEventBaseThreadAndWait([&] {
      clientSocket = folly::AsyncSocket::newSocket(clientEvb);
      clientSocket->connect(&cb, address, 1000);
    });

    connected.wait();
    accepted.wait();
    *MockTransportHandler::onConnectHook_.wlock() = nullptr;

    clientConnections_.emplace_back(
        std::move(clientSocket), std::move(clientThread));
  }

  std::unique_ptr<folly::ScopedEventBaseThread> evbThread_;
  folly::EventBase* evb_{nullptr};
  MockAppHandler appHandler_;
  SimpleBufferAllocator allocator_;
  std::vector<ClientConnection> clientConnections_;
};

TEST_F(ConnectionHandlerTest, ConstructWithSocketAndFactory) {
  auto handler = createConnectionHandler();
  EXPECT_NE(handler, nullptr);
  EXPECT_EQ(handler->connectionCount(), 0);
}

TEST_F(ConnectionHandlerTest, ConnectionAccepted) {
  auto handler = createConnectionHandler();
  auto [clientSocket, serverSocket] = createSocketPair();

  handler->state_ = TestConnectionHandler::State::ACCEPTING;
  evb_->runInEventBaseThreadAndWait([&] {
    handler->connectionAccepted(serverSocket, folly::SocketAddress(), {});
  });

  EXPECT_EQ(handler->connectionCount(), 1);
  EXPECT_TRUE(handler->connections_[0].transportHandler->connectCalled());
  EXPECT_FALSE(handler->connections_[0].transportHandler->readPaused());

  handler->closeAllConnections();
  handler->state_ = TestConnectionHandler::State::STOPPED;
  close(clientSocket.toFd());
}

TEST_F(ConnectionHandlerTest, AcceptMultipleConnections) {
  auto handler = createConnectionHandler();
  handler->state_ = TestConnectionHandler::State::ACCEPTING;

  std::vector<folly::NetworkSocket> clientSockets;
  for (int i = 0; i < 3; ++i) {
    auto [clientSocket, serverSocket] = createSocketPair();
    clientSockets.push_back(clientSocket);
    evb_->runInEventBaseThreadAndWait([&] {
      handler->connectionAccepted(serverSocket, folly::SocketAddress(), {});
    });
  }

  EXPECT_EQ(handler->connectionCount(), 3);

  handler->closeAllConnections();
  handler->state_ = TestConnectionHandler::State::STOPPED;
  for (auto& socket : clientSockets) {
    close(socket.toFd());
  }
}

TEST_F(ConnectionHandlerTest, CloseAllConnectionsClearsConnections) {
  auto handler = createConnectionHandler();
  handler->state_ = TestConnectionHandler::State::ACCEPTING;

  std::vector<folly::NetworkSocket> clientSockets;
  for (int i = 0; i < 3; ++i) {
    auto [clientSocket, serverSocket] = createSocketPair();
    clientSockets.push_back(clientSocket);
    evb_->runInEventBaseThreadAndWait([&] {
      handler->connectionAccepted(serverSocket, folly::SocketAddress(), {});
    });
  }

  EXPECT_EQ(handler->connectionCount(), 3);
  handler->closeAllConnections();
  EXPECT_EQ(handler->connectionCount(), 0);

  handler->state_ = TestConnectionHandler::State::STOPPED;
  for (auto& socket : clientSockets) {
    close(socket.toFd());
  }
}

TEST_F(ConnectionHandlerTest, CloseCallback) {
  auto handler = createConnectionHandler();
  handler->state_ = TestConnectionHandler::State::ACCEPTING;

  std::vector<folly::NetworkSocket> clientSockets;
  for (int i = 0; i < 3; ++i) {
    auto [clientSocket, serverSocket] = createSocketPair();
    clientSockets.push_back(clientSocket);
    evb_->runInEventBaseThreadAndWait([&] {
      handler->connectionAccepted(serverSocket, folly::SocketAddress(), {});
    });
  }

  EXPECT_EQ(handler->connectionCount(), 3);

  auto transportHandler = handler->connections_[1].transportHandler.get();
  transportHandler->onClose(folly::exception_wrapper{});

  EXPECT_EQ(handler->connectionCount(), 2);
  for (auto& connection : handler->connections_) {
    EXPECT_NE(connection.transportHandler.get(), transportHandler);
  }

  handler->closeAllConnections();
  EXPECT_EQ(handler->connectionCount(), 0);
  handler->state_ = TestConnectionHandler::State::STOPPED;

  for (auto& socket : clientSockets) {
    close(socket.toFd());
  }
}

TEST_F(ConnectionHandlerTest, StartAccepting) {
  auto handler = createConnectionHandler();
  folly::SocketAddress address("::1", 0);

  evb_->runInEventBaseThreadAndWait([&] { handler->startAccepting(address); });

  folly::SocketAddress boundAddress;
  handler->getAddress(&boundAddress);
  EXPECT_NE(boundAddress.getPort(), 0);

  evb_->runInEventBaseThreadAndWait([&] { handler->stopAccepting(); });
  // Drain the EVB to let acceptStopped() fire before handler is destroyed.
  evb_->runInEventBaseThreadAndWait([&] {});
}

TEST_F(ConnectionHandlerTest, StopAccepting) {
  auto handler = createConnectionHandler();
  folly::SocketAddress address("::1", 0);

  evb_->runInEventBaseThreadAndWait([&] { handler->startAccepting(address); });

  folly::SocketAddress boundAddress;
  handler->getAddress(&boundAddress);

  connectAndWait(boundAddress);

  EXPECT_EQ(handler->connectionCount(), 1);

  evb_->runInEventBaseThreadAndWait([&] { handler->stopAccepting(); });
  // stopAccepting() calls removeAcceptCallback() which enqueues
  // acceptStopped() via RemoteAcceptor. Drain the EVB to let
  // acceptStopped() -> closeAllConnections() execute.
  evb_->runInEventBaseThreadAndWait([&] {});

  EXPECT_EQ(handler->connectionCount(), 0);
}

TEST_F(ConnectionHandlerTest, AcceptMutlipleConnections) {
  auto handler = createConnectionHandler();
  folly::SocketAddress address("::1", 0);

  evb_->runInEventBaseThreadAndWait([&] { handler->startAccepting(address); });

  folly::SocketAddress boundAddress;
  handler->getAddress(&boundAddress);

  for (int i = 0; i < 3; ++i) {
    connectAndWait(boundAddress);
  }

  EXPECT_EQ(handler->connectionCount(), 3);

  evb_->runInEventBaseThreadAndWait([&] { handler->stopAccepting(); });
  // Drain the EVB to let acceptStopped() fire before handler is destroyed.
  evb_->runInEventBaseThreadAndWait([&] {});
}

TEST_F(ConnectionHandlerTest, DestroyInNoneState) {
  auto handler = createConnectionHandler();
  EXPECT_EQ(handler->state_, TestConnectionHandler::State::NONE);
  handler.reset();
}

TEST_F(ConnectionHandlerTest, DestroyInStoppedState) {
  auto handler = createConnectionHandler();
  folly::SocketAddress address("::1", 0);

  evb_->runInEventBaseThreadAndWait([&] {
    handler->startAccepting(address);
    handler->stopAccepting();
  });
  // Drain the EVB to let acceptStopped() fire (see StopAccepting comment).
  evb_->runInEventBaseThreadAndWait([&] {});

  EXPECT_EQ(handler->state_, TestConnectionHandler::State::STOPPED);
  handler.reset();
}

TEST_F(ConnectionHandlerTest, DestroyWithActiveConnections) {
  auto handler = createConnectionHandler();
  folly::SocketAddress address("::1", 0);

  evb_->runInEventBaseThreadAndWait([&] { handler->startAccepting(address); });

  folly::SocketAddress boundAddress;
  handler->getAddress(&boundAddress);

  connectAndWait(boundAddress);

  EXPECT_EQ(handler->connectionCount(), 1);

  evb_->runInEventBaseThreadAndWait([&] {
    auto* rawHandler = handler.release();
    rawHandler->destroy();
  });
}

TEST_F(ConnectionHandlerTest, ObjectStaysAliveUntilAcceptStopped) {
  auto handler = createConnectionHandler();
  folly::SocketAddress address("::1", 0);

  evb_->runInEventBaseThreadAndWait([&] { handler->startAccepting(address); });

  EXPECT_EQ(handler->state_, TestConnectionHandler::State::ACCEPTING);

  evb_->runInEventBaseThreadAndWait([&] { handler->stopAccepting(); });
  // Drain the EVB to let acceptStopped() fire (see StopAccepting comment).
  evb_->runInEventBaseThreadAndWait([&] {});

  evb_->runInEventBaseThreadAndWait([&] {
    EXPECT_EQ(handler->state_, TestConnectionHandler::State::STOPPED);
    auto* rawHandler = handler.release();
    rawHandler->destroy();
  });
}

} // namespace apache::thrift::fast_thrift::rocket::server::connection
