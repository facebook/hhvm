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

#include <chrono>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include <folly/portability/GTest.h>

#include <folly/Conv.h>
#include <folly/ExceptionWrapper.h>
#include <folly/Format.h>
#include <folly/Range.h>
#include <folly/SocketAddress.h>
#include <folly/Try.h>
#include <folly/executors/ManualExecutor.h>
#include <folly/experimental/coro/AsyncGenerator.h>
#include <folly/experimental/coro/BlockingWait.h>
#include <folly/experimental/coro/Task.h>
#include <folly/fibers/FiberManager.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/ScopedEventBaseThread.h>

#include <folly/io/async/AsyncSocket.h>
#include <thrift/lib/cpp/transport/TTransport.h>
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/async/ClientSinkBridge.h>
#include <thrift/lib/cpp2/async/FutureRequest.h>
#include <thrift/lib/cpp2/async/RequestCallback.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/async/Sink.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/transport/rocket/RocketException.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/transport/rocket/client/RocketClient.h>
#include <thrift/lib/cpp2/transport/rocket/framing/ErrorCode.h>
#include <thrift/lib/cpp2/transport/rocket/test/network/ClientServerTestUtil.h>
#include <thrift/lib/cpp2/transport/rocket/test/network/Mocks.h>
#include <thrift/lib/cpp2/transport/rocket/test/network/Util.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

using namespace apache::thrift;
using namespace apache::thrift::rocket;
using namespace apache::thrift::rocket::test;
using namespace apache::thrift::transport;
using namespace testing;

namespace {
// Used for testing RocketClient against RocketTestServer
class RocketNetworkTest : public testing::Test {
 protected:
  void SetUp() override {
    server_ = std::make_unique<RocketTestServer>();
    client_ = std::make_unique<RocketTestClient>(
        folly::SocketAddress("::1", server_->getListeningPort()));
  }

  void TearDown() override {
    if (client_) {
      client_->verifyVersion();
    }
    client_.reset();
    server_.reset();
  }

 public:
  void withClient(folly::Function<void(RocketTestClient&)> f) { f(*client_); }
  void withClients(folly::Function<void(RocketTestClient&, RocketClient&)> f) {
    RocketClient::Ptr client2;
    createAndConnectClient(client_->getEventBase(), client2);
    f(*client_, *client2.get());
    disconnectClient(client_->getEventBase(), client2);
  }

  void createAndConnectClient(
      folly::EventBase& evb, RocketClient::Ptr& client) {
    evb.runInEventBaseThreadAndWait([&] {
      folly::AsyncSocket::UniquePtr socket(new folly::AsyncSocket(
          &evb,
          folly::SocketAddress("::1", this->server_->getListeningPort())));
      client = RocketClient::create(
          evb,
          std::move(socket),
          std::make_unique<rocket::SetupFrame>(
              this->client_->makeTestSetupFrame()));
    });
  }

  void disconnectClient(folly::EventBase& evb, RocketClient::Ptr& client) {
    evb.runInEventBaseThread([cl = std::move(client)] {});
  }

  folly::ManualExecutor* getUserExecutor() { return &userExecutor_; }

  void unsetExpectedSetupMetadata() { server_->setExpectedSetupMetadata({}); }

 protected:
  std::unique_ptr<RocketTestServer> server_;
  std::unique_ptr<RocketTestClient> client_;
  folly::ManualExecutor userExecutor_;
};

struct OnWriteSuccess : RocketClient::WriteSuccessCallback {
  bool writeSuccess{false};

  void onWriteSuccess() noexcept override { writeSuccess = true; }
};
} // namespace

class StreamElementEncoderStub final
    : public apache::thrift::detail::StreamElementEncoder<int> {
  folly::Try<StreamPayload> operator()(int&& i) override {
    return folly::Try<StreamPayload>(
        StreamPayload(folly::IOBuf::copyBuffer(folly::to<std::string>(i)), {}));
  }

  folly::Try<StreamPayload> operator()(folly::exception_wrapper&&) override {
    return folly::Try<StreamPayload>(
        StreamPayload(folly::IOBuf::create(0), {}));
  }
};
static StreamElementEncoderStub encode;

TEST_F(RocketNetworkTest, FlushManager) {
  this->withClients([](RocketTestClient& client, RocketClient& client2) {
    constexpr folly::StringPiece kMetadata1("metadata1");
    constexpr folly::StringPiece kData1("test_request1");
    constexpr folly::StringPiece kMetadata2("metadata2");
    constexpr folly::StringPiece kData2("test_request2");

    auto& client1 = client.getRawClient();
    auto& eventBase = client.getEventBase();
    FlushManager* flushManager{nullptr};

    auto& fm = folly::fibers::getFiberManager(eventBase);

    OnWriteSuccess onWriteSuccess1, onWriteSuccess2;
    // Add a task that would initiate sending the requests from 2 clients
    fm.addTaskRemoteFuture([&] {
        auto reply1Fut = fm.addTaskEagerFuture([&] {
          return client1.sendRequestResponseSync(
              Payload::makeFromMetadataAndData(kMetadata1, kData1),
              std::chrono::milliseconds(250),
              &onWriteSuccess1);
        });

        auto reply2Fut = fm.addTaskEagerFuture([&] {
          return client2.sendRequestResponseSync(
              Payload::makeFromMetadataAndData(kMetadata2, kData2),
              std::chrono::milliseconds(250),
              &onWriteSuccess2);
        });

        flushManager = &FlushManager::getInstance(eventBase);
        EXPECT_EQ(flushManager->getNumPendingClients(), 2);
        EXPECT_FALSE(onWriteSuccess1.writeSuccess);
        EXPECT_FALSE(onWriteSuccess2.writeSuccess);
        ASSERT_FALSE(flushManager->isScheduled());

        auto reply1 = std::move(reply1Fut).getTry();
        auto reply2 = std::move(reply2Fut).getTry();
        EXPECT_TRUE(reply1.hasValue());
        EXPECT_TRUE(reply2.hasValue());

        EXPECT_EQ(flushManager->getNumPendingClients(), 0);
      }).get();
  });
}

TEST_F(RocketNetworkTest, FlushManagerLowMaxPendingFlushPolicy) {
  this->withClients([](RocketTestClient& client, RocketClient& client2) {
    constexpr folly::StringPiece kMetadata1("metadata1");
    constexpr folly::StringPiece kData1("test_request1");
    constexpr folly::StringPiece kMetadata2("metadata2");
    constexpr folly::StringPiece kData2("test_request2");

    auto& client1 = client.getRawClient();
    auto& eventBase = client.getEventBase();
    FlushManager* flushManager{nullptr};

    auto& fm = folly::fibers::getFiberManager(eventBase);
    // FlushPolicy of 1 basically means immediate flushes.
    client.getEventBase().runInEventBaseThreadAndWait([&] {
      flushManager = &FlushManager::getInstance(eventBase);
      flushManager->setFlushPolicy(1, std::chrono::milliseconds(1));
    });

    OnWriteSuccess onWriteSuccess1, onWriteSuccess2;
    // Add a task that would initiate sending the requests from 2 clients
    fm.addTaskRemoteFuture([&] {
        auto reply1Fut = fm.addTaskEagerFuture([&] {
          return client1.sendRequestResponseSync(
              Payload::makeFromMetadataAndData(kMetadata1, kData1),
              std::chrono::milliseconds(250),
              &onWriteSuccess1);
        });

        auto reply2Fut = fm.addTaskEagerFuture([&] {
          return client2.sendRequestResponseSync(
              Payload::makeFromMetadataAndData(kMetadata2, kData2),
              std::chrono::milliseconds(250),
              &onWriteSuccess2);
        });

        EXPECT_EQ(flushManager->getNumPendingClients(), 2);
        EXPECT_FALSE(flushManager->isScheduled());
        EXPECT_FALSE(onWriteSuccess1.writeSuccess);
        EXPECT_FALSE(onWriteSuccess2.writeSuccess);

        auto reply1 = std::move(reply1Fut).getTry();
        auto reply2 = std::move(reply2Fut).getTry();
        EXPECT_TRUE(reply1.hasValue());
        EXPECT_TRUE(reply2.hasValue());

        EXPECT_EQ(flushManager->getNumPendingClients(), 0);
      }).get();
  });
}

TEST_F(RocketNetworkTest, FlushManagerHighMaxPendingFlushPolicy) {
  this->withClients([](RocketTestClient& client, RocketClient& client2) {
    constexpr folly::StringPiece kMetadata1("metadata1");
    constexpr folly::StringPiece kData1("test_request1");
    constexpr folly::StringPiece kMetadata2("metadata2");
    constexpr folly::StringPiece kData2("test_request2");

    auto& client1 = client.getRawClient();
    auto& eventBase = client.getEventBase();
    FlushManager* flushManager{nullptr};

    auto& fm = folly::fibers::getFiberManager(eventBase);
    // FlushPolicy of 1000 to force the timeout to trigger flushes.
    client.getEventBase().runInEventBaseThreadAndWait([&] {
      flushManager = &FlushManager::getInstance(eventBase);
      flushManager->setFlushPolicy(1000, std::chrono::milliseconds(1));
    });

    OnWriteSuccess onWriteSuccess1, onWriteSuccess2;
    // Add a task that would initiate sending the requests from 2 clients
    fm.addTaskRemoteFuture([&] {
        auto reply1Fut = fm.addTaskEagerFuture([&] {
          return client1.sendRequestResponseSync(
              Payload::makeFromMetadataAndData(kMetadata1, kData1),
              std::chrono::milliseconds(250),
              &onWriteSuccess1);
        });

        auto reply2Fut = fm.addTaskEagerFuture([&] {
          return client2.sendRequestResponseSync(
              Payload::makeFromMetadataAndData(kMetadata2, kData2),
              std::chrono::milliseconds(250),
              &onWriteSuccess2);
        });

        EXPECT_EQ(flushManager->getNumPendingClients(), 2);
        EXPECT_TRUE(flushManager->isScheduled());
        EXPECT_FALSE(onWriteSuccess1.writeSuccess);
        EXPECT_FALSE(onWriteSuccess2.writeSuccess);

        auto reply1 = std::move(reply1Fut).getTry();
        auto reply2 = std::move(reply2Fut).getTry();
        EXPECT_TRUE(reply1.hasValue());
        EXPECT_TRUE(reply2.hasValue());

        EXPECT_EQ(flushManager->getNumPendingClients(), 0);
      }).get();
  });
}

TEST_F(RocketNetworkTest, FlushManagerHighMaxPendingFlushPolicyResetPolicy) {
  this->withClients([](RocketTestClient& client, RocketClient& client2) {
    constexpr folly::StringPiece kMetadata1("metadata1");
    constexpr folly::StringPiece kData1("test_request1");
    constexpr folly::StringPiece kMetadata2("metadata2");
    constexpr folly::StringPiece kData2("test_request2");

    auto& client1 = client.getRawClient();
    auto& eventBase = client.getEventBase();
    FlushManager* flushManager{nullptr};

    auto& fm = folly::fibers::getFiberManager(eventBase);
    // FlushPolicy of 1000 to force the timeout to trigger flushes.
    client.getEventBase().runInEventBaseThreadAndWait([&] {
      flushManager = &FlushManager::getInstance(eventBase);
      flushManager->setFlushPolicy(1000, std::chrono::milliseconds(1));
    });

    OnWriteSuccess onWriteSuccess1, onWriteSuccess2;
    // Add a task that would initiate sending the requests from 2 clients
    fm.addTaskRemoteFuture([&] {
        auto reply1Fut = fm.addTaskEagerFuture([&] {
          return client1.sendRequestResponseSync(
              Payload::makeFromMetadataAndData(kMetadata1, kData1),
              std::chrono::milliseconds(250),
              &onWriteSuccess1);
        });

        auto reply2Fut = fm.addTaskEagerFuture([&] {
          return client2.sendRequestResponseSync(
              Payload::makeFromMetadataAndData(kMetadata2, kData2),
              std::chrono::milliseconds(250),
              &onWriteSuccess2);
        });

        EXPECT_EQ(flushManager->getNumPendingClients(), 2);
        EXPECT_TRUE(flushManager->isScheduled());
        EXPECT_FALSE(onWriteSuccess1.writeSuccess);
        EXPECT_FALSE(onWriteSuccess2.writeSuccess);
        flushManager->resetFlushPolicy();
        EXPECT_FALSE(flushManager->isScheduled());
        EXPECT_FALSE(onWriteSuccess1.writeSuccess);
        EXPECT_FALSE(onWriteSuccess2.writeSuccess);

        auto reply1 = std::move(reply1Fut).getTry();
        auto reply2 = std::move(reply2Fut).getTry();
        EXPECT_TRUE(reply1.hasValue());
        EXPECT_TRUE(reply2.hasValue());

        EXPECT_EQ(flushManager->getNumPendingClients(), 0);
      }).get();
  });
}

TEST_F(RocketNetworkTest, FlushList) {
  this->withClient([](RocketTestClient& client) {
    constexpr folly::StringPiece kMetadata("metadata");
    constexpr folly::StringPiece kData("test_request");

    FlushManager::FlushList flushList;
    auto& rawClient = client.getRawClient();
    auto& eventBase = client.getEventBase();

    auto& fm = folly::fibers::getFiberManager(eventBase);

    OnWriteSuccess onWriteSuccess;
    // Add a task that would initiate sending the request.
    auto sendFuture = fm.addTaskRemoteFuture([&] {
      rawClient.setFlushList(&flushList);

      auto reply = rawClient.sendRequestResponseSync(
          Payload::makeFromMetadataAndData(kMetadata, kData),
          std::chrono::milliseconds(250),
          &onWriteSuccess);

      EXPECT_TRUE(reply.hasValue());
      return std::move(reply.value());
    });

    // Add another task that would ensure several event base loops are
    // performed, then flushes the list.
    fm.addTaskRemoteFuture([&] {
        EXPECT_FALSE(onWriteSuccess.writeSuccess);

        auto cbs = std::move(flushList);
        while (!cbs.empty()) {
          auto* callback = &cbs.front();
          cbs.pop_front();
          callback->runLoopCallback();
        }

        EXPECT_TRUE(onWriteSuccess.writeSuccess);
      }).wait();

    auto reply = std::move(sendFuture).get();

    EXPECT_TRUE(onWriteSuccess.writeSuccess);
    auto dam = splitMetadataAndData(reply);
    EXPECT_EQ(kData, getRange(*dam.second));
    EXPECT_TRUE(reply.hasNonemptyMetadata());
    EXPECT_EQ(kMetadata, getRange(*dam.first));
  });
}

TEST_F(RocketNetworkTest, FlushListFlushPolicyNoop) {
  this->withClient([](RocketTestClient& client) {
    constexpr folly::StringPiece kMetadata("metadata");
    constexpr folly::StringPiece kData("test_request");

    FlushManager::FlushList flushList;
    auto& rawClient = client.getRawClient();
    auto& eventBase = client.getEventBase();
    FlushManager* flushManager{nullptr};

    auto& fm = folly::fibers::getFiberManager(eventBase);

    client.getEventBase().runInEventBaseThreadAndWait([&] {
      flushManager = &FlushManager::getInstance(eventBase);
      flushManager->setFlushPolicy(1000, std::chrono::milliseconds(1));
    });

    OnWriteSuccess onWriteSuccess;
    // Add a task that would initiate sending the request.
    auto sendFuture = fm.addTaskRemoteFuture([&] {
      rawClient.setFlushList(&flushList);

      auto reply = rawClient.sendRequestResponseSync(
          Payload::makeFromMetadataAndData(kMetadata, kData),
          std::chrono::milliseconds(250),
          &onWriteSuccess);

      EXPECT_TRUE(reply.hasValue());
      return std::move(reply.value());
    });

    // Add another task that would ensure several event base loops are
    // performed, then flushes the list.
    fm.addTaskRemoteFuture([&] {
        EXPECT_FALSE(onWriteSuccess.writeSuccess);
        EXPECT_FALSE(flushManager->isScheduled());

        auto cbs = std::move(flushList);
        while (!cbs.empty()) {
          auto* callback = &cbs.front();
          cbs.pop_front();
          callback->runLoopCallback();
        }

        EXPECT_TRUE(onWriteSuccess.writeSuccess);
      }).wait();

    auto reply = std::move(sendFuture).get();

    EXPECT_TRUE(onWriteSuccess.writeSuccess);
    auto dam = splitMetadataAndData(reply);
    EXPECT_EQ(kData, getRange(*dam.second));
    EXPECT_TRUE(reply.hasNonemptyMetadata());
    EXPECT_EQ(kMetadata, getRange(*dam.first));
  });
}

/**
 * REQUEST_RESPONSE tests
 */
TEST_F(RocketNetworkTest, RequestResponseBasic) {
  this->withClient([](RocketTestClient& client) {
    auto& rawClient = client.getRawClient();
    rawClient.setOnDetachable([&rawClient]() {
      EXPECT_TRUE(rawClient.isDetachable());
      rawClient.detachEventBase();
      EXPECT_FALSE(rawClient.isDetachable());
    });
    constexpr folly::StringPiece kMetadata("metadata");
    constexpr folly::StringPiece kData("test_request");

    OnWriteSuccess onWriteSuccess;
    auto reply = client.sendRequestResponseSync(
        Payload::makeFromMetadataAndData(kMetadata, kData),
        std::chrono::milliseconds(250) /* timeout */,
        &onWriteSuccess);

    EXPECT_TRUE(onWriteSuccess.writeSuccess);
    EXPECT_TRUE(reply.hasValue());
    auto dam = splitMetadataAndData(*reply);
    EXPECT_EQ(kData, getRange(*dam.second));
    EXPECT_TRUE(reply->hasNonemptyMetadata());
    EXPECT_EQ(kMetadata, getRange(*dam.first));
  });
}

TEST_F(RocketNetworkTest, RequestResponseTimeout) {
  this->withClient([](RocketTestClient& client) {
    constexpr folly::StringPiece kMetadata("metadata");
    constexpr folly::StringPiece kData("sleep_ms:200");

    auto reply = client.sendRequestResponseSync(
        Payload::makeFromMetadataAndData(kMetadata, kData),
        std::chrono::milliseconds(100));

    EXPECT_TRUE(reply.hasException());
    expectTransportExceptionType(
        TTransportException::TTransportExceptionType::TIMED_OUT,
        std::move(reply.exception()));
  });
}

TEST_F(RocketNetworkTest, RequestResponseLargeMetadata) {
  this->withClient([](RocketTestClient& client) {
    // Ensure metadata will be split across multiple frames
    constexpr size_t kReplyMetadataSize = 0x2ffffff;
    constexpr folly::StringPiece kPattern =
        "abcdefghijklmnopqrstuvwxyz0123456789";

    constexpr folly::StringPiece kMetadata("metadata");
    const auto expectedMetadata = repeatPattern(kPattern, kReplyMetadataSize);
    const std::string data =
        folly::to<std::string>("metadata_echo:", expectedMetadata);

    auto reply = client.sendRequestResponseSync(
        Payload::makeFromMetadataAndData(kMetadata, folly::StringPiece{data}),
        std::chrono::seconds(5));

    EXPECT_TRUE(reply.hasValue());
    EXPECT_TRUE(reply->hasNonemptyMetadata());
    auto dam = splitMetadataAndData(*reply);
    EXPECT_EQ(expectedMetadata, getRange(*dam.first));
    EXPECT_EQ(data, getRange(*dam.second));
  });
}

TEST_F(RocketNetworkTest, RequestResponseLargeData) {
  this->withClient([](RocketTestClient& client) {
    // Ensure metadata will be split across multiple frames
    constexpr size_t kReplyDataSize = 0x2ffffff;
    constexpr folly::StringPiece kPattern =
        "abcdefghijklmnopqrstuvwxyz0123456789";

    constexpr folly::StringPiece kMetadata{"metadata"};
    const auto expectedData = repeatPattern(kPattern, kReplyDataSize);
    const std::string data = folly::to<std::string>("data_echo:", expectedData);

    auto reply = client.sendRequestResponseSync(
        Payload::makeFromMetadataAndData(kMetadata, folly::StringPiece{data}),
        std::chrono::seconds(5));

    EXPECT_TRUE(reply.hasValue());
    EXPECT_TRUE(reply->hasNonemptyMetadata());
    auto dam = splitMetadataAndData(*reply);
    EXPECT_EQ(kMetadata, getRange(*dam.first));
    EXPECT_EQ(expectedData, getRange(*dam.second));
  });
}

TEST_F(RocketNetworkTest, RequestResponseSendTimeout) {
  this->withClient([this](RocketTestClient& client) {
    // Ensure data is large enough to fill up the kernel buffer and trigger send
    // timeout
    constexpr size_t kDataSize = 0x8000000;
    constexpr folly::StringPiece kPattern =
        "abcdefghijklmnopqrstuvwxyz0123456789";

    constexpr folly::StringPiece kMetadata{"metadata"};
    constexpr folly::StringPiece kSmallData{"data"};
    const auto kLargeData = repeatPattern(kPattern, kDataSize);

    // Send a request to make sure the connection is live
    {
      auto reply = client.sendRequestResponseSync(
          Payload::makeFromMetadataAndData(kMetadata, kSmallData),
          std::chrono::seconds(5));
      EXPECT_TRUE(reply.hasValue());
    }

    // Deadlock the server IO thread to trigger send timeouts
    auto ioBlockBaton = std::make_shared<folly::Baton<>>();
    this->server_->getEventBase().add([=] { ioBlockBaton->wait(); });

    {
      // Make sure send timeout is lower than the recv timeout
      client.getEventBase().runInEventBaseThreadAndWait([&] {
        client.getRawClient().getTransportWrapper()->setSendTimeout(500);
      });
      auto reply = client.sendRequestResponseSync(
          Payload::makeFromMetadataAndData(
              kMetadata, folly::StringPiece{kLargeData}),
          std::chrono::seconds(5));
      EXPECT_TRUE(reply.hasException());
      EXPECT_TRUE(reply.withException([](const TTransportException& ex) {
        EXPECT_EQ(TTransportException::NOT_OPEN, ex.getType())
            << folly::exceptionStr(ex);
      }));
    }

    ioBlockBaton->post();
  });
}

TEST_F(RocketNetworkTest, RequestResponseEmptyMetadata) {
  this->withClient([](RocketTestClient& client) {
    constexpr folly::StringPiece kMetadata{"metadata"};
    constexpr folly::StringPiece kData{"metadata_echo:"};

    auto reply = client.sendRequestResponseSync(
        Payload::makeFromMetadataAndData(kMetadata, kData));

    EXPECT_TRUE(reply.hasValue());
    EXPECT_FALSE(reply->hasNonemptyMetadata());
    // Parser should never construct empty metadata
    EXPECT_FALSE(reply->hasNonemptyMetadata());
  });
}

TEST_F(RocketNetworkTest, RequestResponseEmptyData) {
  this->withClient([](RocketTestClient& client) {
    constexpr folly::StringPiece kMetadata{"metadata"};
    constexpr folly::StringPiece kData{"data_echo:"};

    auto reply = client.sendRequestResponseSync(
        Payload::makeFromMetadataAndData(kMetadata, kData));

    EXPECT_TRUE(reply.hasValue());
    EXPECT_TRUE(reply->hasNonemptyMetadata());
    auto dam = splitMetadataAndData(*reply);
    EXPECT_EQ(kMetadata, getRange(*dam.first));
    EXPECT_TRUE(dam.second->empty());
  });
}

TEST_F(RocketNetworkTest, RequestResponseError) {
  this->withClient([](RocketTestClient& client) {
    constexpr folly::StringPiece kMetadata{"metadata"};
    constexpr folly::StringPiece kData{"error:application"};

    auto reply = client.sendRequestResponseSync(
        Payload::makeFromMetadataAndData(kMetadata, kData));

    EXPECT_TRUE(reply.hasException());
    expectRocketExceptionType(
        ErrorCode::APPLICATION_ERROR, std::move(reply.exception()));
  });
}

TEST_F(RocketNetworkTest, RequestResponseDeadServer) {
  constexpr folly::StringPiece kMetadata{"metadata"};
  constexpr folly::StringPiece kData{"data"};

  this->server_.reset();

  OnWriteSuccess onWriteSuccess;
  auto reply = this->client_->sendRequestResponseSync(
      Payload::makeFromMetadataAndData(kMetadata, kData),
      std::chrono::milliseconds(250),
      &onWriteSuccess);

  EXPECT_FALSE(onWriteSuccess.writeSuccess);
  EXPECT_TRUE(reply.hasException());
  expectTransportExceptionType(
      TTransportException::TTransportExceptionType::NOT_OPEN,
      std::move(reply.exception()));
}

TEST_F(RocketNetworkTest, ServerShutdown) {
  this->withClient(
      [server = std::move(server_)](RocketTestClient& client) mutable {
        constexpr folly::StringPiece kMetadata{"metadata"};
        constexpr folly::StringPiece kData{"data_echo:"};

        auto reply = client.sendRequestResponseSync(
            Payload::makeFromMetadataAndData(kMetadata, kData));

        EXPECT_TRUE(reply.hasValue());
        EXPECT_TRUE(reply->hasNonemptyMetadata());

        server.reset();

        auto tew =
            folly::via(&client.getEventBase(), [&] {
              return std::make_optional(client.getRawClient().getLastError());
            }).get();
        auto tex = tew->get_exception<transport::TTransportException>();
        ASSERT_NE(nullptr, tex);
        EXPECT_EQ(
            TTransportException::TTransportExceptionType::END_OF_FILE,
            tex->getType());
        EXPECT_EQ("Connection closed by server", std::string(tex->what()));
      });
}

TEST_F(RocketNetworkTest, RocketClientEventBaseDestruction) {
  auto evb = std::make_unique<folly::EventBase>();
  folly::AsyncSocket::UniquePtr socket(new folly::AsyncSocket(
      evb.get(),
      folly::SocketAddress("::1", this->server_->getListeningPort())));
  auto client = RocketClient::create(
      *evb,
      std::move(socket),
      std::make_unique<SetupFrame>(this->client_->makeTestSetupFrame()));
  EXPECT_NE(nullptr, client->getTransportWrapper());

  evb.reset();
  EXPECT_EQ(nullptr, client->getTransportWrapper());
}

/**
 * REQUEST_FNF tests
 */
TEST_F(RocketNetworkTest, RequestFnfBasic) {
  this->withClient([](RocketTestClient& client) {
    constexpr folly::StringPiece kMetadata("metadata");
    constexpr folly::StringPiece kData("test_request");

    auto reply = client.sendRequestFnfSync(
        Payload::makeFromMetadataAndData(kMetadata, kData));

    EXPECT_TRUE(reply.hasValue());
  });
}

/**
 * REQUEST_STREAM tests
 */
TEST_F(RocketNetworkTest, RequestStreamBasic) {
  // TODO (T62211580)
  return;
  this->withClient([this](RocketTestClient& client) {
    // stream should closed properly in this test so 0 stream should maintain
    // on server when the connection is closed
    this->server_->setExpectedRemainingStreams(0);

    constexpr size_t kNumRequestedPayloads = 200;
    constexpr folly::StringPiece kMetadata("metadata");
    const auto data =
        folly::to<std::string>("generate:", kNumRequestedPayloads);

    auto stream = client.sendRequestStreamSync(
        Payload::makeFromMetadataAndData(kMetadata, folly::StringPiece{data}));
    EXPECT_TRUE(stream.hasValue());
    this->getUserExecutor()->drain();

    size_t received = 0;
    auto subscription = std::move(*stream).subscribeExTry(
        this->getUserExecutor(), [&received](auto&& payload) {
          if (payload.hasValue()) {
            auto dam = splitMetadataAndData(*payload);
            const auto x = folly::to<size_t>(getRange(*dam.second));
            EXPECT_EQ(++received, x);
          } else if (payload.hasException()) {
            FAIL() << payload.exception().what();
          }
        });

    std::move(subscription)
        .futureJoin()
        .via(this->getUserExecutor())
        .waitVia(this->getUserExecutor());
    EXPECT_EQ(kNumRequestedPayloads, received);
  });
}

TEST_F(RocketNetworkTest, RequestStreamError) {
  this->withClient([](RocketTestClient& client) {
    constexpr folly::StringPiece kMetadata("metadata");
    constexpr folly::StringPiece kData("error:application");

    auto stream = client.sendRequestStreamSync(
        Payload::makeFromMetadataAndData(kMetadata, kData));
    EXPECT_TRUE(stream.hasException());
    expectEncodedError(stream.exception());
  });
}

TEST_F(RocketNetworkTest, RequestStreamSmallInitialRequestN) {
  // TODO (T62211580)
  return;
  this->withClient([this](RocketTestClient& client) {
    constexpr size_t kNumRequestedPayloads = 200;
    constexpr folly::StringPiece kMetadata("metadata");
    const auto data =
        folly::to<std::string>("generate:", kNumRequestedPayloads);

    auto stream = client.sendRequestStreamSync(
        Payload::makeFromMetadataAndData(kMetadata, folly::StringPiece{data}));
    EXPECT_TRUE(stream.hasValue());

    size_t received = 0;
    auto subscription = std::move(*stream).subscribeExTry(
        this->getUserExecutor(), [&received](auto&& payload) {
          if (payload.hasValue()) {
            auto dam = splitMetadataAndData(*payload);
            const auto x = folly::to<size_t>(getRange(*dam.second));
            EXPECT_EQ(++received, x);
          } else if (payload.hasException()) {
            FAIL() << payload.exception().what();
          }
        });

    std::move(subscription)
        .futureJoin()
        .via(this->getUserExecutor())
        .waitVia(this->getUserExecutor());
    EXPECT_EQ(kNumRequestedPayloads, received);
  });
}

TEST_F(RocketNetworkTest, RequestStreamCancelSubscription) {
  this->withClient([this](RocketTestClient& client) {
    // Open an essentially infinite stream and ensure stream is able to be
    // canceled within a reasonable amount of time.
    constexpr size_t kNumRequestedPayloads =
        std::numeric_limits<int32_t>::max();
    constexpr folly::StringPiece kMetadata("metadata");
    const auto data =
        folly::to<std::string>("generate:", kNumRequestedPayloads);

    auto stream = client.sendRequestStreamSync(
        Payload::makeFromMetadataAndData(kMetadata, folly::StringPiece{data}));
    EXPECT_TRUE(stream.hasValue());

    size_t received = 0;
    auto subscription = std::move(*stream).subscribeExTry(
        this->getUserExecutor(), [&received](auto&& payload) {
          if (payload.hasValue()) {
            auto dam = splitMetadataAndData(*payload);
            const auto x = folly::to<size_t>(getRange(*dam.second));
            EXPECT_EQ(++received, x);
          } else if (payload.hasException()) {
            FAIL() << payload.exception().what();
          }
        });

    subscription.cancel();
    std::move(subscription)
        .futureJoin()
        .via(this->getUserExecutor())
        .waitVia(this->getUserExecutor());
    EXPECT_LT(received, kNumRequestedPayloads);
  });
}

TEST_F(RocketNetworkTest, RequestStreamNeverSubscribe) {
  this->withClient([](RocketTestClient& client) {
    constexpr size_t kNumRequestedPayloads = 200;
    constexpr folly::StringPiece kMetadata("metadata");
    const auto data =
        folly::to<std::string>("generate:", kNumRequestedPayloads);

    {
      auto stream =
          client.sendRequestStreamSync(Payload::makeFromMetadataAndData(
              kMetadata, folly::StringPiece{data}));
      EXPECT_TRUE(stream.hasValue());
    }
  });
}

TEST_F(RocketNetworkTest, RequestStreamCloseClient) {
  constexpr size_t kNumRequestedPayloads = 200;
  constexpr folly::StringPiece kMetadata("metadata");
  const auto data = folly::to<std::string>("generate:", kNumRequestedPayloads);

  auto stream = this->client_->sendRequestStreamSync(
      Payload::makeFromMetadataAndData(kMetadata, folly::StringPiece{data}));
  EXPECT_TRUE(stream.hasValue());

  bool onErrorCalled = false;
  auto subscription = std::move(*stream).subscribeExTry(
      this->getUserExecutor(), [&](auto&& payload) {
        if (payload.hasException()) {
          onErrorCalled = true;
          expectTransportExceptionType(
              TTransportException::TTransportExceptionType::UNKNOWN,
              std::move(payload.exception()));
        }
      });

  this->client_.reset();

  std::move(subscription)
      .futureJoin()
      .via(this->getUserExecutor())
      .waitVia(this->getUserExecutor());
  EXPECT_TRUE(onErrorCalled);
}

TEST_F(RocketNetworkTest, ClientCreationAndReconnectStreamOutlivesClient) {
  this->withClient([](RocketTestClient& client) {
    constexpr size_t kNumRequestedPayloads = 1000000;
    constexpr folly::StringPiece kMetadata("metadata");
    const auto data =
        folly::to<std::string>("generate:", kNumRequestedPayloads);

    // Open a stream and reconnect many times, having each stream slightly
    // outlive its associated RocketClient.
    for (size_t i = 0; i < 1000; ++i) {
      auto stream =
          client.sendRequestStreamSync(Payload::makeFromMetadataAndData(
              kMetadata, folly::StringPiece{data}));
      EXPECT_TRUE(stream.hasValue());
      client.reconnect();
    }
  });
}

TEST_F(
    RocketNetworkTest, ClientCreationAndReconnectSubscriptionOutlivesClient) {
  this->withClient([this](RocketTestClient& client) {
    constexpr size_t kNumRequestedPayloads = 1000000;
    constexpr folly::StringPiece kMetadata("metadata");
    const auto data =
        folly::to<std::string>("generate:", kNumRequestedPayloads);

    // Open a stream and reconnect many times, subscribing to the stream before
    // and having the subscription outlive the client.
    for (size_t i = 0; i < 1000; ++i) {
      auto stream =
          client.sendRequestStreamSync(Payload::makeFromMetadataAndData(
              kMetadata, folly::StringPiece{data}));
      EXPECT_TRUE(stream.hasValue());
      size_t received = 0;
      auto subscription = std::move(*stream).subscribeExTry(
          this->getUserExecutor(), [&received](auto&& payload) {
            if (payload.hasValue()) {
              auto dam = splitMetadataAndData(*payload);
              const auto x = folly::to<size_t>(getRange(*dam.second));
              EXPECT_EQ(++received, x);
            }
          });
      client.reconnect();
      subscription.cancel();
      std::move(subscription)
          .futureJoin()
          .via(this->getUserExecutor())
          .waitVia(this->getUserExecutor());
    }
  });
}

TEST_F(RocketNetworkTest, ClientCreationAndReconnectClientOutlivesStream) {
  this->withClient([](RocketTestClient& client) {
    constexpr size_t kNumRequestedPayloads = 1000000;
    constexpr folly::StringPiece kMetadata("metadata");
    const auto data =
        folly::to<std::string>("generate:", kNumRequestedPayloads);

    // Open a stream and reconnect many times, having each RocketClient slightly
    // outlive the associated stream.
    for (size_t i = 0; i < 1000; ++i) {
      {
        auto stream =
            client.sendRequestStreamSync(Payload::makeFromMetadataAndData(
                kMetadata, folly::StringPiece{data}));
        EXPECT_TRUE(stream.hasValue());
      }
      client.reconnect();
    }
  });
}

namespace {
class TestClientCallback : public StreamClientCallback {
 public:
  TestClientCallback(
      folly::EventBase& evb,
      uint64_t requested,
      uint64_t requestedHeaders = 0,
      uint64_t echoHeaders = 0)
      : evb_(evb),
        requested_(requested),
        requestedHeaders_(requestedHeaders),
        echoHeaders_(echoHeaders) {}

  bool onFirstResponse(
      FirstResponsePayload&& firstResponsePayload,
      folly::EventBase* evb,
      StreamServerCallback* subscription) override {
    EXPECT_EQ(&evb_, evb);
    if (getRange(*firstResponsePayload.payload) == "error:application") {
      ew_ = folly::make_exception_wrapper<apache::thrift::detail::EncodedError>(
          std::move(firstResponsePayload.payload));
      evb_.terminateLoopSoon();
      firstResponseError_ = true;
      return true;
    }

    subscription_ = subscription;
    // First response does not count towards requested payloads count.
    EXPECT_EQ(
        folly::to<std::string>(0),
        folly::StringPiece{firstResponsePayload.payload->coalesce()});
    if (requested_ != 0) {
      request(requested_);
    }
    for (size_t i = 1; i <= echoHeaders_; ++i) {
      HeadersPayloadContent header;
      header.otherMetadata_ref() = {
          {"expected_header", folly::to<std::string>(i)}};
      auto alive = subscription_->onSinkHeaders({std::move(header), {}});
      DCHECK(alive);
    }
    return true;
  }

  void onFirstResponseError(folly::exception_wrapper ew) override {
    subscription_ = nullptr;
    ew_ = std::move(ew);
    evb_.terminateLoopSoon();
  }

  bool onStreamNext(StreamPayload&& payload) override {
    DCHECK(!firstResponseError_);
    EXPECT_EQ(
        folly::to<std::string>(++received_),
        folly::StringPiece{payload.payload->coalesce()});
    EXPECT_LE(received_, requested_);
    return true;
  }
  void onStreamError(folly::exception_wrapper ew) override {
    subscription_ = nullptr;
    ew_ = std::move(ew);
    evb_.terminateLoopSoon();
  }
  void onStreamComplete() override {
    if (firstResponseError_) {
      return;
    }
    EXPECT_EQ(requested_, received_);
    subscription_ = nullptr;
    evb_.terminateLoopSoon();
  }
  bool onStreamHeaders(HeadersPayload&& payload) override {
    auto metadata_ref = payload.payload.otherMetadata_ref();
    EXPECT_TRUE(metadata_ref);
    if (metadata_ref) {
      EXPECT_EQ(
          folly::to<std::string>(++receivedHeaders_),
          (*metadata_ref)["expected_header"]);
    }
    EXPECT_LE(receivedHeaders_, requestedHeaders_);
    return true;
  }

  void resetServerCallback(StreamServerCallback& serverCallback) override {
    subscription_ = &serverCallback;
  }

  void cancel() {
    if (auto* subscription = std::exchange(subscription_, nullptr)) {
      subscription->onStreamCancel();
    }
  }
  void request(uint64_t tokens) {
    if (subscription_) {
      std::ignore = subscription_->onStreamRequestN(tokens);
    }
  }

  uint64_t payloadsReceived() const { return received_; }
  uint64_t headersReceived() const { return receivedHeaders_; }
  folly::exception_wrapper getError() const { return ew_; }

 private:
  folly::EventBase& evb_;
  StreamServerCallback* subscription_{nullptr};
  folly::exception_wrapper ew_;
  const uint64_t requested_;
  const uint64_t requestedHeaders_;
  const uint64_t echoHeaders_;
  uint64_t received_{0};
  uint64_t receivedHeaders_{0};
  bool firstResponseError_{false};
};
} // namespace

TEST_F(RocketNetworkTest, RequestStreamNewApiBasic) {
  folly::EventBase evb;

  this->unsetExpectedSetupMetadata();

  auto socket = folly::AsyncSocket::UniquePtr(
      new folly::AsyncSocket(&evb, "::1", this->server_->getListeningPort()));
  auto channel = RocketClientChannel::newChannel(std::move(socket));

  constexpr uint64_t kNumRequestedPayloads = 200;
  auto payload = folly::IOBuf::copyBuffer(
      folly::sformat("generate:{}", kNumRequestedPayloads));

  RpcOptions rpcOptions;
  rpcOptions.setChunkBufferSize(0);
  TestClientCallback clientCallback(evb, kNumRequestedPayloads);

  channel->sendRequestStream(
      rpcOptions,
      "dummy",
      apache::thrift::SerializedRequest(std::move(payload)),
      std::make_shared<THeader>(),
      &clientCallback);

  evb.loop();

  EXPECT_FALSE(clientCallback.getError());
  EXPECT_EQ(kNumRequestedPayloads, clientCallback.payloadsReceived());
}

TEST_F(RocketNetworkTest, RequestStreamNewApiError) {
  folly::EventBase evb;

  this->unsetExpectedSetupMetadata();

  auto socket = folly::AsyncSocket::UniquePtr(
      new folly::AsyncSocket(&evb, "::1", this->server_->getListeningPort()));
  auto channel = RocketClientChannel::newChannel(std::move(socket));

  constexpr uint64_t kNumRequestedPayloads = 200;
  auto payload = folly::IOBuf::copyBuffer("error:application");

  RpcOptions rpcOptions;
  TestClientCallback clientCallback(evb, kNumRequestedPayloads);

  channel->sendRequestStream(
      rpcOptions,
      "dummy",
      apache::thrift::SerializedRequest(std::move(payload)),
      std::make_shared<THeader>(),
      &clientCallback);

  evb.loop();

  EXPECT_TRUE(clientCallback.getError());
  EXPECT_EQ(0, clientCallback.payloadsReceived());
}

TEST_F(RocketNetworkTest, RequestStreamNewApiHeadersPush) {
  folly::EventBase evb;

  this->unsetExpectedSetupMetadata();

  auto socket = folly::AsyncSocket::UniquePtr(
      new folly::AsyncSocket(&evb, "::1", this->server_->getListeningPort()));
  auto channel = RocketClientChannel::newChannel(std::move(socket));

  {
    constexpr uint64_t kNumRequestedHeaders = 200;
    auto payload = folly::IOBuf::copyBuffer(
        folly::sformat("generateheaders:{}", kNumRequestedHeaders));

    RpcOptions rpcOptions;
    rpcOptions.setChunkBufferSize(0);
    TestClientCallback clientCallback(evb, 0, kNumRequestedHeaders);

    channel->sendRequestStream(
        rpcOptions,
        "dummy",
        apache::thrift::SerializedRequest(std::move(payload)),
        std::make_shared<THeader>(),
        &clientCallback);

    evb.loop();

    EXPECT_FALSE(clientCallback.getError());
    EXPECT_EQ(0, clientCallback.payloadsReceived());
    EXPECT_EQ(kNumRequestedHeaders, clientCallback.headersReceived());
  }

  {
    constexpr uint64_t kNumEchoHeaders = 200;
    auto payload = folly::IOBuf::copyBuffer(
        folly::sformat("echoheaders:{}", kNumEchoHeaders));

    RpcOptions rpcOptions;
    rpcOptions.setChunkBufferSize(0);
    TestClientCallback clientCallback(evb, 0, kNumEchoHeaders, kNumEchoHeaders);

    channel->sendRequestStream(
        rpcOptions,
        "dummy",
        apache::thrift::SerializedRequest(std::move(payload)),
        std::make_shared<THeader>(),
        &clientCallback);

    evb.loop();

    EXPECT_FALSE(clientCallback.getError());
    EXPECT_EQ(0, clientCallback.payloadsReceived());
    EXPECT_EQ(kNumEchoHeaders, clientCallback.headersReceived());
  }
}

struct SinkFirstResponse {
  folly::coro::Task<folly::Try<FirstResponsePayload>> getFirstThriftResponse() {
    co_await baton_;
    co_return std::move(payload_);
  }

  folly::coro::Baton baton_;
  folly::Try<FirstResponsePayload> payload_;
  apache::thrift::detail::ClientSinkBridge::ClientPtr sinkBridge_;
};

struct FirstResponseCallback
    : apache::thrift::detail::ClientSinkBridge::FirstResponseCallback {
 public:
  explicit FirstResponseCallback(SinkFirstResponse& firstResponse)
      : firstResponse_(firstResponse) {}

  void onFirstResponse(
      apache::thrift::FirstResponsePayload&& firstPayload,
      apache::thrift::detail::ClientSinkBridge::ClientPtr sinkBridge) override {
    firstResponse_.payload_.emplace(std::move(firstPayload));
    firstResponse_.sinkBridge_ = std::move(sinkBridge);
    firstResponse_.baton_.post();
    delete this;
  }

  void onFirstResponseError(folly::exception_wrapper ew) override {
    firstResponse_.payload_.emplaceException(std::move(ew));
    firstResponse_.baton_.post();
    delete this;
  }

 private:
  SinkFirstResponse& firstResponse_;
};

TEST_F(RocketNetworkTest, SinkBasic) {
  this->withClient([](RocketTestClient& client) {
    constexpr size_t kNumUploadPayloads = 200;
    constexpr folly::StringPiece kMetadata("metadata");
    // instruct server to append A on each payload client sents, and
    // sends the appended payload back to client
    const auto data = "upload:";

    folly::coro::blockingWait(
        folly::coro::co_invoke([&]() -> folly::coro::Task<void> {
          SinkFirstResponse response;
          auto callback = new FirstResponseCallback(response);
          client.sendRequestSink(
              apache::thrift::detail::ClientSinkBridge::create(callback),
              Payload::makeFromMetadataAndData(
                  kMetadata,
                  folly::StringPiece{
                      folly::to<std::string>(data, kNumUploadPayloads)}));
          co_await response.getFirstThriftResponse();
          auto clientSink = ClientSink<int, int>(
              std::move(response.sinkBridge_),
              &encode,
              [](folly::Try<StreamPayload>&& payload) -> folly::Try<int> {
                if (payload.hasValue()) {
                  return folly::Try<int>(folly::to<int>(
                      folly::StringPiece{payload->payload->coalesce()}));
                } else {
                  return folly::Try<int>(payload.exception());
                }
              });

          int finalResponse = co_await clientSink.sink(folly::coro::co_invoke(
              []() -> folly::coro::AsyncGenerator<int&&> {
                for (size_t i = 0; i < kNumUploadPayloads; i++) {
                  co_yield i;
                }
              }));
          EXPECT_EQ(kNumUploadPayloads, finalResponse);
        }));
  });
}

TEST_F(RocketNetworkTest, SinkCloseClient) {
  this->withClient([](RocketTestClient& client) {
    constexpr size_t kNumUploadPayloads = 200;
    constexpr folly::StringPiece kMetadata("metadata");
    // instruct server to append A on each payload client sents, and
    // sends the appended payload back to client
    const auto data = "upload:";
    SinkFirstResponse response;
    auto callback = new FirstResponseCallback(response);
    client.sendRequestSink(
        apache::thrift::detail::ClientSinkBridge::create(callback),
        Payload::makeFromMetadataAndData(
            kMetadata,
            folly::StringPiece{
                folly::to<std::string>(data, kNumUploadPayloads)}));

    folly::coro::blockingWait(
        folly::coro::co_invoke([&]() -> folly::coro::Task<void> {
          co_await response.getFirstThriftResponse();
        }));

    auto sink = ClientSink<int, int>(
        std::move(response.sinkBridge_),
        &encode,
        [](folly::Try<StreamPayload>&& payload) -> folly::Try<int> {
          if (payload.hasValue()) {
            return folly::Try<int>(folly::to<int>(
                folly::StringPiece{payload->payload->coalesce()}));
          } else {
            return folly::Try<int>(payload.exception());
          }
        });

    client.disconnect();
    bool exceptionThrows = false;
    folly::coro::blockingWait(folly::coro::co_invoke(
        [&, sink = std::move(sink)]() mutable -> folly::coro::Task<void> {
          try {
            co_await sink.sink(folly::coro::co_invoke(
                [&]() -> folly::coro::AsyncGenerator<int&&> {
                  for (size_t i = 0; i < kNumUploadPayloads; i++) {
                    co_yield i;
                  }
                }));
          } catch (const std::exception&) {
            exceptionThrows = true;
          }
        }));
    EXPECT_TRUE(exceptionThrows);
  });
}

TEST_F(RocketNetworkTest, CloseNowWithPendingWriteCallback) {
  class FakeTransport final : public folly::AsyncTransport {
   public:
    explicit FakeTransport(folly::EventBase* e) : eventBase_(e) {}
    void setReadCB(ReadCallback*) override {}
    ReadCallback* getReadCallback() const override { return nullptr; }
    void write(
        WriteCallback* cb, const void*, size_t, folly::WriteFlags) override {
      callbacks_.push_back(cb);
    }
    void writev(
        WriteCallback* cb, const iovec*, size_t, folly::WriteFlags) override {
      callbacks_.push_back(cb);
    }
    void writeChain(
        WriteCallback* cb,
        std::unique_ptr<folly::IOBuf>&&,
        folly::WriteFlags) override {
      callbacks_.push_back(cb);
    }
    folly::EventBase* getEventBase() const override { return eventBase_; }
    void getAddress(folly::SocketAddress*) const override {}
    void close() override {}
    void closeNow() override {
      // delaying callback to be fullfilled during closeNow()
      for (auto cb : callbacks_) {
        folly::AsyncSocketException ex(
            folly::AsyncSocketException::AsyncSocketExceptionType::UNKNOWN,
            "test");
        cb->writeErr(0, ex);
      }
    }
    void shutdownWrite() override {}
    void shutdownWriteNow() override {}
    bool good() const override { return true; }
    bool readable() const override { return true; }
    bool connecting() const override { return true; }
    bool error() const override { return true; }
    void attachEventBase(folly::EventBase*) override {}
    void detachEventBase() override {}
    bool isDetachable() const override { return true; }
    void setSendTimeout(uint32_t) override {}
    uint32_t getSendTimeout() const override { return 0u; }
    void getLocalAddress(folly::SocketAddress*) const override {}
    void getPeerAddress(folly::SocketAddress*) const override {}
    bool isEorTrackingEnabled() const override { return true; }
    void setEorTracking(bool) override {}
    size_t getAppBytesWritten() const override { return 0u; }
    size_t getRawBytesWritten() const override { return 0u; }
    size_t getAppBytesReceived() const override { return 0u; }
    size_t getRawBytesReceived() const override { return 0u; }

   private:
    folly::EventBase* eventBase_;
    std::vector<WriteCallback*> callbacks_;
  };

  auto evb = std::make_unique<folly::EventBase>();
  auto sock = folly::AsyncTransport::UniquePtr(new FakeTransport(evb.get()));
  auto client = RocketClient::create(
      *evb,
      std::move(sock),
      std::make_unique<SetupFrame>(this->client_->makeTestSetupFrame()));
  // write something to the socket, without holding keepalive of evb and waiting
  // for write callback
  client->cancelStream(StreamId(1));
  evb->loopOnce();
  // will invoke closeNow() with some write callbacks not fullfilled yet, that
  // should not in any invalid state
  evb.reset();
}

/**
 * Rocket connection observer tests
 */

TEST_F(RocketNetworkTest, ObserverIsNotInstalledWhenFlagIsFalse) {
  auto observer =
      std::make_unique<NiceMock<MockRocketServerConnectionObserver>>();

  this->server_->getEventBase().runInEventBaseThreadAndWait([&] {
    auto connCount = 0;
    server_->getConnectionManager()->forEachConnection(
        [&](wangle::ManagedConnection* connection) {
          if (auto conn = dynamic_cast<RocketServerConnection*>(connection)) {
            EXPECT_EQ(conn->numObservers(), 0);
            conn->addObserver(observer.get());
            EXPECT_EQ(conn->numObservers(), 0);
            connCount += 1;
          }
        });
    EXPECT_EQ(connCount, 1);
  });
}

MATCHER_P3(WriteStartingMatcher, id, bytes, offset, "") {
  return arg.streamId == StreamId(id) &&
      arg.totalBytesInWrite == (size_t)bytes &&
      arg.batchOffset == (size_t)offset;
}

MATCHER_P3(WriteSuccessMatcher, id, bytes, offset, "") {
  return arg.streamId == StreamId(id) &&
      arg.totalBytesInWrite == (size_t)bytes &&
      arg.batchOffset == (size_t)offset;
}

MATCHER_P2(WriteEventContextMatcher, startRawOffset, endRawOffset, "") {
  return arg.startRawByteOffset.has_value() &&
      arg.startRawByteOffset.value() == startRawOffset &&
      arg.endRawByteOffset.has_value() &&
      arg.endRawByteOffset.value() == endRawOffset;
}

TEST_F(RocketNetworkTest, ObserverIsNotifiedOnWriteSuccessRequestResponse) {
  THRIFT_FLAG_SET_MOCK(enable_rocket_connection_observers, true);
  this->withClient([&](RocketTestClient& client) {
    RocketServerConnection::ManagedObserver::EventSet eventSet;
    eventSet.enable(
        RocketServerConnection::ManagedObserver::Events::WriteEvents);
    auto observer =
        std::make_unique<NiceMock<MockRocketServerConnectionObserver>>(
            eventSet);

    this->server_->getEventBase().runInEventBaseThreadAndWait([&] {
      auto connCount = 0;
      server_->getConnectionManager()->forEachConnection(
          [&](wangle::ManagedConnection* connection) {
            if (auto conn = dynamic_cast<RocketServerConnection*>(connection)) {
              EXPECT_EQ(conn->numObservers(), 0);
              conn->addObserver(observer.get());
              EXPECT_EQ(conn->numObservers(), 1);
              connCount += 1;
            }
          });
      EXPECT_EQ(connCount, 1);
    });

    constexpr size_t kSetupFrameSize(14);
    constexpr size_t kSetupFrameStreamId(0);

    // send a request and check the event notifications when the
    // response is ready and written to the socket
    {
      constexpr folly::StringPiece kMetadata("metadata");
      constexpr folly::StringPiece kData("data");
      const unsigned int startOffsetBatch1 = 0;
      const unsigned int endOffsetBatch1 = kSetupFrameSize + 24 - 1;

      // responses to setup frame (stream id = 0) and to the first
      // request (stream id = 1) are batched together
      EXPECT_CALL(
          *observer,
          writeStarting(
              _,
              WriteStartingMatcher(
                  kSetupFrameStreamId /* streamId */,
                  kSetupFrameSize /* totalBytesWritten */,
                  0 /* batchOffset */)));
      EXPECT_CALL(
          *observer,
          writeSuccess(
              _,
              WriteSuccessMatcher(
                  kSetupFrameStreamId /* streamId */,
                  kSetupFrameSize /* totalBytesWritten */,
                  0 /* batchOffset */),
              WriteEventContextMatcher(
                  startOffsetBatch1 /* startRawByteOffset */,
                  endOffsetBatch1 /* endRawByteOffset */)));
      EXPECT_CALL(
          *observer,
          writeStarting(
              _,
              WriteStartingMatcher(
                  kSetupFrameStreamId + 1 /* streamId */,
                  24 /* totalBytesWritten */,
                  kSetupFrameSize /* batchOffset */)));
      EXPECT_CALL(
          *observer,
          writeSuccess(
              _,
              WriteSuccessMatcher(
                  kSetupFrameStreamId + 1 /* streamId */,
                  24 /* totalBytesWritten */,
                  kSetupFrameSize /* batchOffset */),
              WriteEventContextMatcher(
                  startOffsetBatch1 /* startRawByteOffset */,
                  endOffsetBatch1 /* endRawByteOffset */)));

      client.sendRequestResponseSync(
          Payload::makeFromMetadataAndData(kMetadata, kData),
          std::chrono::milliseconds(250) /* timeout */);

      Mock::VerifyAndClearExpectations(observer.get());
    }

    // send another request and check again the event notifications
    {
      constexpr folly::StringPiece kNewMetadata("new_metadata");
      constexpr folly::StringPiece kNewData("new_data");
      const unsigned int startOffsetBatch2 =
          kSetupFrameSize + 24 /* 24 is the size of the first response */;
      const unsigned int endOffsetBatch2 = startOffsetBatch2 + 32 -
          1 /* 32 is the size of the second response */;

      EXPECT_CALL(
          *observer,
          writeStarting(
              _,
              WriteStartingMatcher(
                  kSetupFrameStreamId + 3 /* streamId */,
                  32 /* totalBytesWritten */,
                  0 /* batchOffset */)));
      EXPECT_CALL(
          *observer,
          writeSuccess(
              _,
              WriteSuccessMatcher(
                  kSetupFrameStreamId + 3 /* streamId */,
                  32 /* totalBytesWritten */,
                  0 /* batchOffset */),
              WriteEventContextMatcher(
                  startOffsetBatch2 /* startRawByteOffset */,
                  endOffsetBatch2 /* endRawByteOffset */)));

      client.sendRequestResponseSync(
          Payload::makeFromMetadataAndData(kNewMetadata, kNewData),
          std::chrono::milliseconds(250) /* timeout */);
    }

    server_->getEventBase().runInEventBaseThreadAndWait([&] {
      server_->getConnectionManager()->forEachConnection(
          [&](wangle::ManagedConnection* connection) {
            if (auto conn = dynamic_cast<RocketServerConnection*>(connection)) {
              conn->removeObserver(observer.get());
              EXPECT_EQ(conn->numObservers(), 0);
            }
          });
    });
  });
}

TEST_F(RocketNetworkTest, ObserverIsNotifiedOnWriteSuccessRequestStream) {
  THRIFT_FLAG_SET_MOCK(enable_rocket_connection_observers, true);
  this->withClient([this](RocketTestClient& client) {
    RocketServerConnection::ManagedObserver::EventSet eventSet;
    eventSet.enable(
        RocketServerConnection::ManagedObserver::Events::WriteEvents);
    auto observer =
        std::make_unique<NiceMock<MockRocketServerConnectionObserver>>(
            eventSet);

    this->server_->getEventBase().runInEventBaseThreadAndWait([&] {
      auto connCount = 0;
      server_->getConnectionManager()->forEachConnection(
          [&](wangle::ManagedConnection* connection) {
            if (auto conn = dynamic_cast<RocketServerConnection*>(connection)) {
              EXPECT_EQ(conn->numObservers(), 0);
              conn->addObserver(observer.get());
              EXPECT_EQ(conn->numObservers(), 1);
              connCount += 1;
            }
          });
      EXPECT_EQ(connCount, 1);
    });

    constexpr size_t kSetupFrameSize(14);
    constexpr size_t kSetupFrameStreamId(0);
    constexpr size_t kNumRequestedPayloads = 200;
    constexpr folly::StringPiece kMetadata("metadata");
    const auto data =
        folly::to<std::string>("generate:", kNumRequestedPayloads);
    const unsigned int startOffsetBatch = 0;
    const unsigned int endOffsetBatch = kSetupFrameSize + 16 - 1;

    EXPECT_CALL(
        *observer,
        writeStarting(
            _,
            WriteStartingMatcher(
                kSetupFrameStreamId /* streamId */,
                kSetupFrameSize /* totalBytesWritten */,
                0 /* batchOffset */)));
    EXPECT_CALL(
        *observer,
        writeSuccess(
            _,
            WriteSuccessMatcher(
                kSetupFrameStreamId /* streamId */,
                kSetupFrameSize /* totalBytesWritten */,
                0 /* batchOffset */),
            WriteEventContextMatcher(
                startOffsetBatch /* startRawByteOffset */,
                endOffsetBatch /* endRawByteOffset */)));
    EXPECT_CALL(
        *observer,
        writeStarting(
            _,
            WriteStartingMatcher(
                kSetupFrameStreamId + 1 /* streamId */,
                16 /* totalBytesWritten */,
                kSetupFrameSize /* batchOffset */)));
    EXPECT_CALL(
        *observer,
        writeSuccess(
            _,
            WriteSuccessMatcher(
                kSetupFrameStreamId + 1 /* streamId */,
                16 /* totalBytesWritten */,
                kSetupFrameSize /* batchOffset */),
            WriteEventContextMatcher(
                startOffsetBatch /* startRawByteOffset */,
                endOffsetBatch /* endRawByteOffset */)));

    auto stream = client.sendRequestStreamSync(
        Payload::makeFromMetadataAndData(kMetadata, folly::StringPiece{data}));

    server_->getEventBase().runInEventBaseThreadAndWait([&] {
      server_->getConnectionManager()->forEachConnection(
          [&](wangle::ManagedConnection* connection) {
            if (auto conn = dynamic_cast<RocketServerConnection*>(connection)) {
              conn->removeObserver(observer.get());
              EXPECT_EQ(conn->numObservers(), 0);
            }
          });
    });
  });
}
