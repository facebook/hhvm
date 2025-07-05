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
#include <folly/Conv.h>
#include <folly/portability/GFlags.h>

#include <glog/logging.h>

#include <folly/ScopeGuard.h>
#include <folly/io/async/AsyncSSLSocket.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/synchronization/Baton.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/HTTPClientChannel.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include <thrift/lib/cpp2/async/PooledRequestChannel.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/cpp2/server/ServerFlags.h>
#include <thrift/lib/cpp2/transport/core/ThriftClient.h>
#include <thrift/lib/cpp2/transport/core/ThriftClientCallback.h>
#include <thrift/lib/cpp2/transport/core/testutil/MockCallback.h>
#include <thrift/lib/cpp2/transport/core/testutil/TAsyncSocketIntercepted.h>
#include <thrift/lib/cpp2/transport/core/testutil/TransportCompatibilityTest.h>
#include <thrift/lib/cpp2/transport/core/testutil/gen-cpp2/TestService.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketRoutingHandler.h>
#include <thrift/lib/cpp2/transport/rocket/server/RocketServerConnection.h>
#include <thrift/lib/cpp2/transport/rocket/server/ThriftRocketServerHandler.h>
#include <thrift/lib/cpp2/transport/util/ConnectionManager.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

DECLARE_bool(use_ssl);
DECLARE_string(transport);

DEFINE_string(host, "::1", "host to connect to");

THRIFT_FLAG_DECLARE_int64(thrift_client_checksum_sampling_rate);

// Timeout used for polling callCompleted_ to make the test more robust.
constexpr auto kPollTimeout = std::chrono::seconds(5);

void pollSleep() {
  std::this_thread::sleep_for(std::chrono::milliseconds(1)); // NOLINT
}

#define EXPECT_EQ_POLL(actual_ref, expected, timeout)                \
  {                                                                  \
    auto start = std::chrono::steady_clock::now();                   \
    while ((actual_ref) != (expected) &&                             \
           (std::chrono::steady_clock::now() - start) < (timeout)) { \
      pollSleep();                                                   \
    }                                                                \
  }                                                                  \
  EXPECT_EQ(actual_ref, expected)

namespace apache::thrift {

using namespace async;
using namespace testing;
using namespace apache::thrift::transport;
using namespace testutil::testservice;

TransportCompatibilityTest::TransportCompatibilityTest()
    : handler_(std::make_shared<
               StrictMock<testutil::testservice::TestServiceMock>>()),
      server_(std::make_unique<
              SampleServer<testutil::testservice::TestServiceMock>>(handler_)) {
}

template <typename Service>
SampleServer<Service>::SampleServer(std::shared_ptr<Service> handler)
    : handler_(std::move(handler)) {
  setupServer();
}

// Tears down after the test.
template <typename Service>
SampleServer<Service>::~SampleServer() {
  stopServer();
}

// Event handler to attach to the Thrift server so we know when it is
// ready to serve and also so we can determine the port it is
// listening on.
class TransportCompatibilityTestEventHandler
    : public server::TServerEventHandler {
 public:
  // This is a callback that is called when the Thrift server has
  // initialized and is ready to serve RPCs.
  void preServe(const folly::SocketAddress* address) override {
    port_ = address->getPort();
    baton_.post();
  }

  int32_t waitForPortAssignment() {
    baton_.wait();
    return port_;
  }

 private:
  folly::Baton<> baton_;
  int32_t port_;
};

template <typename Service>
void SampleServer<Service>::addRoutingHandler(
    std::unique_ptr<TransportRoutingHandler> routingHandler) {
  DCHECK(server_) << "First call setupServer() function";

  server_->addRoutingHandler(std::move(routingHandler));
}

template <typename Service>
ThriftServer* SampleServer<Service>::getServer() {
  DCHECK(server_) << "First call setupServer() function";

  return server_.get();
}

void TransportCompatibilityTest::addRoutingHandler(
    std::unique_ptr<TransportRoutingHandler> routingHandler) {
  return server_->addRoutingHandler(std::move(routingHandler));
}

ThriftServer* TransportCompatibilityTest::getServer() {
  return server_->getServer();
}

template <typename Service>
void SampleServer<Service>::setupServer() {
  DCHECK(!server_) << "First close the server with stopServer()";

  server_ = std::make_unique<ThriftServer>();
  observer_ = std::make_shared<FakeServerObserver>();
  server_->setObserver(observer_);
  server_->setPort(0);
  server_->setNumIOWorkerThreads(numIOThreads_);
  server_->setNumCPUWorkerThreads(numWorkerThreads_);
  if (useResourcePoolsFlagsSet()) {
    server_->ensureResourcePools();
    auto& resourcePool = server_->resourcePoolSet().resourcePool(
        ResourcePoolHandle::defaultAsync());
    dynamic_cast<folly::ThreadPoolExecutor&>(
        resourcePool.executor().value().get())
        .setNumThreads(numWorkerThreads_);
    resourcePool.concurrencyController()
        .value()
        .get()
        .setExecutionLimitRequests(numWorkerThreads_);
  }
  server_->setInterface(handler_);
  server_->setGetLoad(
      [](const std::string& metric) { return metric.empty() ? 123 : -1; });
}

template <typename Service>
void SampleServer<Service>::startServer() {
  DCHECK(server_) << "First call setupServer() function";
  auto eventHandler =
      std::make_shared<TransportCompatibilityTestEventHandler>();
  server_->setServerEventHandler(eventHandler);
  server_->setup();

  // Get the port that the server has bound to
  port_ = eventHandler->waitForPortAssignment();
}

void TransportCompatibilityTest::startServer() {
  server_->startServer();
}

template <typename Service>
void SampleServer<Service>::stopServer() {
  if (server_) {
    server_->cleanUp();
    server_.reset();
    handler_.reset();
  }
}

void TransportCompatibilityTest::connectToServer(
    folly::Function<void(std::unique_ptr<TestServiceAsyncClient>)> callMe) {
  connectToServer([callMe = std::move(callMe)](
                      std::unique_ptr<TestServiceAsyncClient> client,
                      auto) mutable { callMe(std::move(client)); });
}

void TransportCompatibilityTest::connectToServer(
    folly::Function<void(
        std::unique_ptr<TestServiceAsyncClient>,
        std::shared_ptr<ClientConnectionIf>)> callMe) {
  server_->connectToServer(
      FLAGS_transport,
      upgradeToRocketExpected_,
      [callMe = std::move(callMe)](
          std::shared_ptr<RequestChannel> channel,
          std::shared_ptr<ClientConnectionIf> connection) mutable {
        auto client =
            std::make_unique<TestServiceAsyncClient>(std::move(channel));
        callMe(std::move(client), std::move(connection));
      });
}

template <typename Service>
void SampleServer<Service>::connectToServer(
    std::string transport,
    bool withUpgrade,
    folly::Function<void(
        std::shared_ptr<RequestChannel>, std::shared_ptr<ClientConnectionIf>)>
        callMe) {
  ASSERT_GT(port_, 0) << "Check if the server has started already";
  if (transport == "header") {
    std::shared_ptr<ClientChannel> channel;
    evbThread_.getEventBase()->runInEventBaseThreadAndWait([&]() {
      if (withUpgrade) {
        channel = HeaderClientChannel::newChannel(
            folly::AsyncSocket::UniquePtr(new TAsyncSocketIntercepted(
                evbThread_.getEventBase(), FLAGS_host, port_)));
      } else {
        channel = HeaderClientChannel::newChannel(
            HeaderClientChannel::WithoutRocketUpgrade{},
            folly::AsyncSocket::UniquePtr(new TAsyncSocketIntercepted(
                evbThread_.getEventBase(), FLAGS_host, port_)));
      }
    });
    auto channelPtr = channel.get();
    std::shared_ptr<ClientChannel> destroyInEvbChannel(
        channelPtr,
        [channel = std::move(channel),
         eventBase = evbThread_.getEventBase()](ClientChannel*) mutable {
          eventBase->runImmediatelyOrRunInEventBaseThreadAndWait(
              [channel_ = std::move(channel)] {});
        });
    callMe(std::move(destroyInEvbChannel), nullptr);
  } else if (transport == "rocket") {
    std::shared_ptr<RocketClientChannel> channel;
    evbThread_.getEventBase()->runInEventBaseThreadAndWait([&]() {
      channel = RocketClientChannel::newChannel(
          folly::AsyncSocket::UniquePtr(new TAsyncSocketIntercepted(
              evbThread_.getEventBase(), FLAGS_host, port_)));
    });
    auto channelPtr = channel.get();
    std::shared_ptr<RocketClientChannel> destroyInEvbChannel(
        channelPtr,
        [channel_ = std::move(channel),
         eventBase = evbThread_.getEventBase()](RocketClientChannel*) mutable {
          eventBase->runImmediatelyOrRunInEventBaseThreadAndWait(
              [channel__ = std::move(channel_)] {});
        });
    callMe(std::move(destroyInEvbChannel), nullptr);
  } else if (transport == "legacy-http2") {
    // We setup legacy http2 for synchronous calls only - we do not
    // drive this event base.
    auto executor = std::make_shared<folly::ScopedEventBaseThread>();
    auto eventBase = executor->getEventBase();
    auto channel = PooledRequestChannel::newChannel(
        eventBase, executor, [port = std::move(port_)](folly::EventBase& evb) {
          folly::AsyncSocket::UniquePtr socket(
              new TAsyncSocketIntercepted(&evb, FLAGS_host, port));
          if (FLAGS_use_ssl) {
            auto sslContext = std::make_shared<folly::SSLContext>();
            sslContext->setAdvertisedNextProtocols({"h2", "http"});
            auto sslSocket = folly::AsyncSSLSocket::newSocket(
                sslContext, &evb, socket->detachNetworkSocket(), false);
            sslSocket->sslConn(nullptr);
            socket = std::move(sslSocket);
          }
          auto channel = HTTPClientChannel::newHTTP2Channel(std::move(socket));
          channel->setProtocolId(protocol::T_COMPACT_PROTOCOL);
          return channel;
        });
    callMe(std::move(channel), nullptr);
  } else if (transport == "http2") {
    auto mgr = ConnectionManager::getInstance();
    auto connection = mgr->getConnection(FLAGS_host, port_);
    auto channel = ThriftClient::Ptr(new ThriftClient(connection));
    channel->setProtocolId(apache::thrift::protocol::T_COMPACT_PROTOCOL);
    callMe(std::move(channel), std::move(connection));
  } else {
    throw std::runtime_error("unknown transport: " + transport);
  }
}

void TransportCompatibilityTest::callSleep(
    TestServiceAsyncClient* client, int32_t timeoutMs, int32_t sleepMs) {
  auto cb = std::make_unique<MockCallback>(false, timeoutMs < sleepMs);
  RpcOptions opts;
  opts.setTimeout(std::chrono::milliseconds(timeoutMs));
  opts.setQueueTimeout(std::chrono::milliseconds(5000));
  client->sleep(opts, std::move(cb), sleepMs);
}

void TransportCompatibilityTest::TestConnectionStats() {
  connectToServer([this](std::unique_ptr<TestServiceAsyncClient> client) {
    EXPECT_EQ(0, server_->observer_->connAccepted_);
    EXPECT_EQ(0, server_->observer_->connClosed_);
    EXPECT_EQ(0, server_->observer_->activeConns_);

    EXPECT_CALL(*handler_.get(), sumTwoNumbers_(1, 2)).Times(1);
    EXPECT_EQ(3, client->future_sumTwoNumbers(1, 2).get());

    if (upgradeToRocketExpected_) {
      // for transport upgrade there are both header and rocket connections
      EXPECT_EQ(2, server_->observer_->connAccepted_);
      EXPECT_EQ(2 * server_->numIOThreads_, server_->observer_->activeConns_);
    } else {
      EXPECT_EQ(1, server_->observer_->connAccepted_);
      EXPECT_EQ(server_->numIOThreads_, server_->observer_->activeConns_);
    }

    folly::Baton<> connCloseBaton;
    server_->observer_->connClosedNotifBaton = &connCloseBaton;

    // Close the connection to trigger connClosed event on the server.
    auto* channel = client->getChannel();
    auto* evb = channel->getEventBase();
    evb->runInEventBaseThread([client = std::move(client)] {
      dynamic_cast<ClientChannel*>(client->getChannel())->closeNow();
    });

    ASSERT_TRUE(connCloseBaton.try_wait_for(std::chrono::seconds(10)));

    if (upgradeToRocketExpected_) {
      // for transport upgrade there are both header and rocket connections
      EXPECT_EQ(2, server_->observer_->connAccepted_);
      EXPECT_EQ(2 * server_->numIOThreads_, server_->observer_->activeConns_);
    } else {
      EXPECT_EQ(1, server_->observer_->connAccepted_);
      EXPECT_EQ(server_->numIOThreads_, server_->observer_->activeConns_);
    }
  });
}

void TransportCompatibilityTest::TestObserverSendReceiveRequests() {
  connectToServer([this](std::unique_ptr<TestServiceAsyncClient> client) {
    EXPECT_CALL(*handler_.get(), sumTwoNumbers_(1, 2)).Times(2);
    EXPECT_CALL(*handler_.get(), add_(1));
    EXPECT_CALL(*handler_.get(), add_(2));
    EXPECT_CALL(*handler_.get(), add_(5));

    // Send a message
    EXPECT_EQ(3, client->future_sumTwoNumbers(1, 2).get());
    EXPECT_EQ(1, client->future_add(1).get());

    auto future = client->future_add(2);
    EXPECT_EQ(3, std::move(future).get());

    EXPECT_EQ(3, client->future_sumTwoNumbers(1, 2).get());
    EXPECT_EQ(8, client->future_add(5).get());

    // Now check the stats
    EXPECT_EQ(5, server_->observer_->sentReply_);
    EXPECT_EQ(5, server_->observer_->receivedRequest_);
    if (FLAGS_transport != "http2") {
      EXPECT_EQ_POLL(server_->observer_->callCompleted_, 5, kPollTimeout);
    }
  });
}

void TransportCompatibilityTest::TestConnectionContext() {
  connectToServer([this](std::unique_ptr<TestServiceAsyncClient> client) {
    auto channel = dynamic_cast<ClientChannel*>(client->getChannel());
    int32_t port{0};
    channel->getEventBase()->runInEventBaseThreadAndWait([&] {
      auto socket = dynamic_cast<folly::AsyncSocket*>(channel->getTransport());
      folly::SocketAddress localAddress;
      socket->getLocalAddress(&localAddress);
      port = localAddress.getPort();
    });
    EXPECT_NE(0, port);

    EXPECT_CALL(*handler_.get(), checkPort_(port));
    client->future_checkPort(port).get();
  });
}

void TransportCompatibilityTest::TestClientIdentityHook() {
  bool flag{false};
  auto hook = [&flag](
                  const folly::AsyncTransport* /* unused */,
                  const X509* /* unused */,
                  const folly::SocketAddress& /* unused */) {
    flag = true;
    return std::unique_ptr<void, void (*)(void*)>(nullptr, [](void*) {});
  };
  server_->getServer()->setClientIdentityHook(std::move(hook));
  connectToServer([&](std::unique_ptr<TestServiceAsyncClient> client) {
    EXPECT_CALL(*handler_.get(), sumTwoNumbers_(1, 2));

    EXPECT_EQ(3, client->future_sumTwoNumbers(1, 2).get());
    EXPECT_TRUE(flag);
  });
}

void TransportCompatibilityTest::TestRequestResponse_Simple() {
  connectToServer([this](std::unique_ptr<TestServiceAsyncClient> client) {
    EXPECT_CALL(*handler_.get(), sumTwoNumbers_(1, 2)).Times(2);
    EXPECT_CALL(*handler_.get(), add_(1));
    EXPECT_CALL(*handler_.get(), add_(2));
    EXPECT_CALL(*handler_.get(), add_(5));

    // Send a message
    EXPECT_EQ(3, client->future_sumTwoNumbers(1, 2).get());
    EXPECT_EQ(1, client->future_add(1).get());

    auto future = client->future_add(2);
    EXPECT_EQ(3, std::move(future).get());

    EXPECT_EQ(3, client->future_sumTwoNumbers(1, 2).get());
    EXPECT_EQ(8, client->future_add(5).get());
    if (FLAGS_transport != "http2") {
      EXPECT_EQ_POLL(server_->observer_->callCompleted_, 5, kPollTimeout);
    }
  });
}

void TransportCompatibilityTest::TestRequestResponse_Sync() {
  connectToServer([this](std::unique_ptr<TestServiceAsyncClient> client) {
    EXPECT_CALL(*handler_.get(), sumTwoNumbers_(1, 2)).Times(2);
    EXPECT_CALL(*handler_.get(), add_(1));
    EXPECT_CALL(*handler_.get(), add_(2));
    EXPECT_CALL(*handler_.get(), add_(5));

    // Send a message
    EXPECT_EQ(3, client->future_sumTwoNumbers(1, 2).get());
    EXPECT_EQ(1, client->future_add(1).get());
    EXPECT_EQ(3, client->future_add(2).get());
    EXPECT_EQ(3, client->future_sumTwoNumbers(1, 2).get());
    EXPECT_EQ(8, client->future_add(5).get());
  });
}

void TransportCompatibilityTest::TestRequestResponse_Destruction() {
  connectToServer([](std::unique_ptr<TestServiceAsyncClient> client) {
    auto future =
        client->future_sleep(100).thenTry([&](folly::Try<folly::Unit> t) {
          client.reset();
          EXPECT_TRUE(t.hasException());
        });

    auto channel = static_cast<ClientChannel*>(client->getChannel());
    channel->getEventBase()->runInEventBaseThreadAndWait(
        [&]() { channel->getTransport()->closeNow(); });

    std::move(future).get();
  });
}

void TransportCompatibilityTest::TestRequestResponse_MultipleClients() {
  const int clientCount = 10;
  EXPECT_CALL(*handler_.get(), sumTwoNumbers_(1, 2)).Times(2 * clientCount);
  EXPECT_CALL(*handler_.get(), add_(1)).Times(clientCount);
  EXPECT_CALL(*handler_.get(), add_(2)).Times(clientCount);
  EXPECT_CALL(*handler_.get(), add_(5)).Times(clientCount);

  auto lambda = [](std::unique_ptr<TestServiceAsyncClient> client) {
    // Send a message
    EXPECT_EQ(3, client->future_sumTwoNumbers(1, 2).get());
    EXPECT_LE(1, client->future_add(1).get());

    auto future = client->future_add(2);
    EXPECT_LE(3, std::move(future).get());

    EXPECT_EQ(3, client->future_sumTwoNumbers(1, 2).get());
    EXPECT_LE(8, client->future_add(5).get());
  };

  std::vector<folly::ScopedEventBaseThread> threads(clientCount);
  std::vector<folly::Promise<folly::Unit>> promises(clientCount);
  std::vector<folly::Future<folly::Unit>> futures;
  for (int i = 0; i < clientCount; ++i) {
    auto& promise = promises[i];
    futures.emplace_back(promise.getFuture());
    threads[i].getEventBase()->runInEventBaseThread([&promise, lambda, this]() {
      connectToServer(lambda);
      promise.setValue();
    });
  }
  folly::collectAll(futures).get();
  threads.clear();
}

void TransportCompatibilityTest::TestRequestResponse_ExpectedException() {
  EXPECT_THROW(
      connectToServer(
          [&](auto client) { client->future_throwExpectedException(1).get(); }),
      TestServiceException);

  EXPECT_THROW(
      connectToServer(
          [&](auto client) { client->future_throwExpectedException(1).get(); }),
      TestServiceException);
}

void TransportCompatibilityTest::TestRequestResponse_UnexpectedException() {
  EXPECT_THROW(
      connectToServer([&](auto client) {
        client->future_throwUnexpectedException(2).get();
      }),
      apache::thrift::TApplicationException);

  EXPECT_THROW(
      connectToServer([&](auto client) {
        client->future_throwUnexpectedException(2).get();
      }),
      apache::thrift::TApplicationException);
}

void TransportCompatibilityTest::TestRequestResponse_Timeout() {
  connectToServer([this](std::unique_ptr<TestServiceAsyncClient> client) {
    // These are all async calls.  The first batch of calls get
    // dispatched immediately, then there is a sleep, and then the
    // second batch of calls get dispatched.  All calls have separate
    // timeouts and different delays on the server side.
    callSleep(client.get(), 1, 100);
    callSleep(client.get(), 100, 0);
    callSleep(client.get(), 1, 100);
    callSleep(client.get(), 100, 0);
    callSleep(client.get(), 2000, 500);
    /* sleep override */
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    if (FLAGS_transport == "rocket") {
      EXPECT_EQ(5, server_->observer_->activeRequests_);
    }
    callSleep(client.get(), 100, 1000);
    callSleep(client.get(), 200, 0);
    /* Sleep to give time for all callbacks to be completed */
    /* sleep override */
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    if (FLAGS_transport == "rocket") {
      EXPECT_EQ(2, server_->observer_->activeRequests_);
    }
    EXPECT_EQ(3, server_->observer_->taskTimeout_);
    EXPECT_EQ(0, server_->observer_->queueTimeout_);
  });
}

void TransportCompatibilityTest::TestRequestResponse_Header() {
  connectToServer([](std::unique_ptr<TestServiceAsyncClient> client) {
    { // Future
      apache::thrift::RpcOptions rpcOptions;
      rpcOptions.setWriteHeader("header_from_client", "2");
      auto future = client->header_future_headers(rpcOptions);
      auto tHeader = std::move(future).get().second;
      auto keyValue = tHeader->getHeaders();
      EXPECT_NE(keyValue.end(), keyValue.find("header_from_server"));
      EXPECT_STREQ("1", keyValue.find("header_from_server")->second.c_str());
    }

    { // Callback
      apache::thrift::RpcOptions rpcOptions;
      rpcOptions.setWriteHeader("header_from_client", "2");
      auto executed = std::make_shared<folly::Promise<folly::Unit>>();
      auto future = executed->getFuture();
      client->headers(
          rpcOptions,
          std::make_unique<FunctionReplyCallback>(
              [executed](ClientReceiveState&& state) {
                auto keyValue = state.header()->getHeaders();
                EXPECT_NE(keyValue.end(), keyValue.find("header_from_server"));
                EXPECT_STREQ(
                    "1", keyValue.find("header_from_server")->second.c_str());

                auto exw = TestServiceAsyncClient::recv_wrapped_headers(state);
                EXPECT_FALSE(exw);
                executed->setValue();
              }));
      auto& waited = future.wait(folly::Duration(100));
      EXPECT_TRUE(waited.isReady());
    }
  });
}

void TransportCompatibilityTest::TestRequestResponse_Header_Load() {
  connectToServer([](std::unique_ptr<TestServiceAsyncClient> client) {
    RpcOptions rpcOptions;
    rpcOptions.setWriteHeader("header_from_client", "2");
    rpcOptions.setWriteHeader((std::string)THeader::QUERY_LOAD_HEADER, {});
    auto resultAndHeaders = client->header_future_headers(rpcOptions).get();
    const auto readHeaders =
        std::move(resultAndHeaders.second)->releaseHeaders();
    EXPECT_NE(readHeaders.end(), readHeaders.find("header_from_server"));
    auto load = [&]() -> int64_t {
      if (auto value = resultAndHeaders.second->getServerLoad()) {
        return *value;
      }
      auto* loadPtr = folly::get_ptr(readHeaders, THeader::QUERY_LOAD_HEADER);
      EXPECT_NE(nullptr, loadPtr);
      return folly::to<int64_t>(*loadPtr);
    }();
    EXPECT_EQ(123, load);
  });
}

void TransportCompatibilityTest::
    TestRequestResponse_Header_ExpectedException() {
  connectToServer([](std::unique_ptr<TestServiceAsyncClient> client) {
    { // Future
      apache::thrift::RpcOptions rpcOptions;
      rpcOptions.setWriteHeader("header_from_client", "2");
      rpcOptions.setWriteHeader("expected_exception", "1");
      auto future = client->header_future_headers(rpcOptions);
      auto& waited = future.wait();
      auto& ftry = waited.result();
      EXPECT_TRUE(ftry.hasException());
      EXPECT_THAT(
          ftry.tryGetExceptionObject()->what(),
          HasSubstr("TestServiceException"));
    }

    { // Callback
      apache::thrift::RpcOptions rpcOptions;
      rpcOptions.setWriteHeader("header_from_client", "2");
      rpcOptions.setWriteHeader("expected_exception", "1");
      auto executed = std::make_shared<folly::Promise<folly::Unit>>();
      auto future = executed->getFuture();
      client->headers(
          rpcOptions,
          std::make_unique<FunctionReplyCallback>(
              [executed](ClientReceiveState&& state) {
                auto exw = TestServiceAsyncClient::recv_wrapped_headers(state);
                EXPECT_TRUE(exw.get_exception());
                EXPECT_THAT(
                    exw.what().c_str(), HasSubstr("TestServiceException"));
                executed->setValue();
              }));
      auto& waited = future.wait(folly::Duration(100));
      ASSERT_TRUE(waited.isReady());
    }
  });
}

void TransportCompatibilityTest::
    TestRequestResponse_Header_UnexpectedException() {
  connectToServer([](std::unique_ptr<TestServiceAsyncClient> client) {
    { // Future
      apache::thrift::RpcOptions rpcOptions;
      rpcOptions.setWriteHeader("header_from_client", "2");
      rpcOptions.setWriteHeader("unexpected_exception", "1");
      auto future = client->header_future_headers(rpcOptions);
      EXPECT_THROW(
          std::move(future).get(), apache::thrift::TApplicationException);
    }

    { // Callback
      apache::thrift::RpcOptions rpcOptions;
      rpcOptions.setWriteHeader("header_from_client", "2");
      rpcOptions.setWriteHeader("unexpected_exception", "1");
      auto executed = std::make_shared<folly::Promise<folly::Unit>>();
      auto future = executed->getFuture();
      client->headers(
          rpcOptions,
          std::make_unique<FunctionReplyCallback>(
              [executed](ClientReceiveState&& state) {
                auto exw = TestServiceAsyncClient::recv_wrapped_headers(state);
                EXPECT_TRUE(exw.get_exception());
                EXPECT_THAT(
                    exw.what().c_str(), HasSubstr("TApplicationException"));
                executed->setValue();
              }));
      auto& waited = future.wait(folly::Duration(100));
      EXPECT_TRUE(waited.isReady());
    }
  });
}

void TransportCompatibilityTest::TestRequestResponse_Saturation() {
  connectToServer([this](auto client, auto connection) {
    EXPECT_CALL(*handler_.get(), add_(3)).Times(2);
    // note that no EXPECT_CALL for add_(5)

    connection->getEventBase()->runInEventBaseThreadAndWait(
        [&]() { connection->setMaxPendingRequests(0u); });
    EXPECT_THROW(client->semifuture_add(5).get(), TTransportException);

    connection->getEventBase()->runInEventBaseThreadAndWait(
        [&]() { connection->setMaxPendingRequests(1u); });
    EXPECT_EQ(3, client->semifuture_add(3).get());
    EXPECT_EQ(6, client->semifuture_add(3).get());
  });
}

void TransportCompatibilityTest::TestRequestResponse_IsOverloaded() {
  // make sure server is overloaded
  server_->getServer()->setIsOverloaded(
      [](const transport::THeader::StringToStringMap&, const std::string&) {
        return true;
      });
  connectToServer([this](std::unique_ptr<TestServiceAsyncClient> client) {
    try {
      RpcOptions rpcOptions;
      auto resultAndHeaders = client->header_future_headers(rpcOptions).get();
      EXPECT_TRUE(false) << "header_future_headers should have thrown";
    } catch (TApplicationException& ex) {
      EXPECT_EQ(TApplicationException::LOADSHEDDING, ex.getType());
      EXPECT_EQ(0, server_->observer_->taskKilled_);
      EXPECT_EQ(1, server_->observer_->serverOverloaded_);
    }
  });
}

void TransportCompatibilityTest::TestRequestResponse_Connection_CloseNow() {
  connectToServer([](std::unique_ptr<TestServiceAsyncClient> client) {
    // It should not reach to server: no EXPECT_CALL for add_(3)

    // Observe the behavior if the connection is closed already
    auto channel = static_cast<ClientChannel*>(client->getChannel());
    channel->getEventBase()->runInEventBaseThreadAndWait(
        [&]() { channel->closeNow(); });

    try {
      client->future_add(3).get();
      EXPECT_TRUE(false) << "future_add should have thrown";
    } catch (TTransportException& ex) {
      EXPECT_EQ(TTransportException::NOT_OPEN, ex.getType());
    }
  });
}

void TransportCompatibilityTest::TestRequestResponse_ServerQueueTimeout() {
  connectToServer([this](
                      std::unique_ptr<TestServiceAsyncClient> client) mutable {
    // Pidgeon-hole principle. Each client request causes a blocking sleep on a
    // server-side IO thread, due to server executing with inline executor. N
    // threads blocked with N + 1 requests leaves 1 request in the queue. By the
    // time the first N requests finish, the last queued request will be stale.
    int32_t callCount = 1 + server_->getServer()->getNumIOWorkerThreads();

    // Queue expiration - executes some of the tasks ( = thread count)
    server_->getServer()->setQueueTimeout(std::chrono::milliseconds(10));
    server_->getServer()->setTaskExpireTime(std::chrono::milliseconds(10));
    std::vector<folly::Future<folly::Unit>> futures(callCount);
    for (int i = 0; i < callCount; ++i) {
      RpcOptions opts;
      opts.setTimeout(std::chrono::milliseconds(10));
      futures[i] = client->future_sleep(100);
    }
    int taskTimeoutCount = 0;
    int successCount = 0;
    for (auto& future : futures) {
      auto& waitedFuture = future.wait();
      auto& triedFuture = waitedFuture.result();
      if (triedFuture.withException([](TApplicationException& ex) {
            EXPECT_EQ(
                TApplicationException::TApplicationExceptionType::TIMEOUT,
                ex.getType());
          })) {
        ++taskTimeoutCount;
      } else {
        ASSERT_FALSE(triedFuture.hasException());
        ++successCount;
      }
    }
    EXPECT_LE(1, taskTimeoutCount) << "at least 1 task is expected to timeout";
    EXPECT_LE(1, successCount) << "at least 1 task is expected to succeed";

    // Task expires - even though starts executing the tasks, all expires
    server_->getServer()->setQueueTimeout(std::chrono::milliseconds(1000));
    server_->getServer()->setUseClientTimeout(false);
    server_->getServer()->setTaskExpireTime(std::chrono::milliseconds(1));
    for (int i = 0; i < callCount; ++i) {
      futures[i] = client->future_sleep(100 + i);
    }
    taskTimeoutCount = 0;
    for (auto& future : futures) {
      auto& waitedFuture = future.wait();
      auto& triedFuture = waitedFuture.result();
      if (triedFuture.withException([](TApplicationException& ex) {
            EXPECT_EQ(
                TApplicationException::TApplicationExceptionType::TIMEOUT,
                ex.getType());
          })) {
        ++taskTimeoutCount;
      } else {
        ASSERT_FALSE(triedFuture.hasException());
      }
    }
    EXPECT_EQ(callCount, taskTimeoutCount)
        << "all tasks are expected to be timed out";
  });
}

void TransportCompatibilityTest::TestRequestResponse_ResponseSizeTooBig() {
  connectToServer([this](std::unique_ptr<TestServiceAsyncClient> client) {
    // Execute the function, but fail when sending the response
    EXPECT_CALL(*handler_.get(), hello_(_));

    server_->getServer()->setMaxResponseSize(1);
    try {
      std::string longName(1, 'f');
      auto result = client->future_hello(longName).get();
      EXPECT_TRUE(false) << "future_hello should have thrown";
    } catch (TApplicationException& ex) {
      EXPECT_EQ(TApplicationException::INTERNAL_ERROR, ex.getType());
    }
  });
}

void TransportCompatibilityTest::TestRequestResponse_Checksumming() {
  connectToServer([this](std::unique_ptr<TestServiceAsyncClient> client) {
    enum class CorruptionType : int {
      NONE = 0,
      REQUESTS = 1,
      RESPONSES = 2,
    };
    EXPECT_CALL(*handler_.get(), echo_(_)).Times(4);

    auto setCorruption = [&](CorruptionType corruptionType) {
      auto channel = static_cast<ClientChannel*>(client->getChannel());
      channel->getEventBase()->runInEventBaseThreadAndWait([&]() {
        auto p = std::make_shared<TAsyncSocketIntercepted::Params>();
        p->corruptLastWriteByte_ = corruptionType == CorruptionType::REQUESTS;
        p->corruptLastReadByte_ = corruptionType == CorruptionType::RESPONSES;
        dynamic_cast<TAsyncSocketIntercepted*>(channel->getTransport())
            ->setParams(p);
      });
    };

    auto setCompression = [&](bool compression) {
      auto channel = static_cast<ClientChannel*>(client->getChannel());
      channel->getEventBase()->runInEventBaseThreadAndWait([&]() {
        CompressionConfig compressionConfig;
        if (compression) {
          compressionConfig.codecConfig().ensure().set_zstdConfig();
        }
        channel->setDesiredCompressionConfig(compressionConfig);
      });
    };

    for (CorruptionType testType :
         {CorruptionType::NONE,
          CorruptionType::REQUESTS,
          CorruptionType::RESPONSES}) {
      for (auto compression : {false, true}) {
        static const int kSize = 32 << 10;
        std::string asString(kSize, 'a');
        std::unique_ptr<folly::IOBuf> payload =
            folly::IOBuf::copyBuffer(asString);
        setCorruption(testType);
        setCompression(compression);

        server_->observer_->taskKilled_ = 0;
        auto future =
            client->future_echo(RpcOptions().setEnableChecksum(true), *payload);

        if (testType == CorruptionType::NONE) {
          EXPECT_EQ(asString, std::move(future).get());
        } else {
          bool didThrow = false;
          try {
            auto res = std::move(future).get();
          } catch (TApplicationException& ex) {
            EXPECT_EQ(
                compression
                    ? ((testType == CorruptionType::RESPONSES)
                           ? TApplicationException::INVALID_TRANSFORM
                           : TApplicationException::UNSUPPORTED_CLIENT_TYPE)
                    : TApplicationException::CHECKSUM_MISMATCH,
                ex.getType());
            didThrow = true;
          }
          EXPECT_TRUE(didThrow)
              << "Expected an exception with corruption type: " << (int)testType
              << ", compression: " << compression;
        }
        EXPECT_EQ(
            testType == CorruptionType::REQUESTS ? 1 : 0,
            server_->observer_->taskKilled_);
      }
    }
    setCorruption(CorruptionType::NONE);
  });
}

void TransportCompatibilityTest::TestOneway_Simple() {
  connectToServer([this](std::unique_ptr<TestServiceAsyncClient> client) {
    EXPECT_CALL(*handler_.get(), add_(0));
    EXPECT_CALL(*handler_.get(), addAfterDelay_(0, 5));

    client->future_addAfterDelay(0, 5).get();
    // Sleep a bit for oneway call to complete on server
    /* sleep override */
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(5, client->future_add(0).get());
    // test stats
    EXPECT_EQ(2, server_->observer_->receivedRequest_);
    EXPECT_EQ(1, server_->observer_->sentReply_);
    if (FLAGS_transport != "http2") {
      EXPECT_EQ_POLL(server_->observer_->callCompleted_, 1, kPollTimeout);
    }
  });
}

void TransportCompatibilityTest::TestOneway_WithDelay() {
  connectToServer([this](std::unique_ptr<TestServiceAsyncClient> client) {
    EXPECT_CALL(*handler_.get(), add_(0)).Times(2);
    EXPECT_CALL(*handler_.get(), addAfterDelay_(800, 5));

    // Perform an add on the server after a delay
    client->future_addAfterDelay(800, 5).get();
    // Call add to get result before the previous addAfterDelay takes
    // place - this verifies that the addAfterDelay call is really
    // oneway.
    EXPECT_EQ(0, client->future_add(0).get());
    // Sleep to wait for oneway call to complete on server
    /* sleep override */
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    EXPECT_EQ(5, client->future_add(0).get());
  });
}

void TransportCompatibilityTest::TestOneway_Saturation() {
  connectToServer([this](auto client, auto connection) {
    EXPECT_CALL(*handler_.get(), add_(3));
    // note that no EXPECT_CALL for addAfterDelay_(0, 5)

    connection->getEventBase()->runInEventBaseThreadAndWait(
        [&]() { connection->setMaxPendingRequests(0u); });
    EXPECT_THROW(
        client->semifuture_addAfterDelay(0, 5).get(), TTransportException);

    connection->getEventBase()->runInEventBaseThreadAndWait(
        [&]() { connection->setMaxPendingRequests(1u); });
    EXPECT_EQ(3, client->semifuture_add(3).get());
  });
}

void TransportCompatibilityTest::TestOneway_UnexpectedException() {
  connectToServer([this](std::unique_ptr<TestServiceAsyncClient> client) {
    EXPECT_CALL(*handler_.get(), onewayThrowsUnexpectedException_(100));
    EXPECT_CALL(*handler_.get(), onewayThrowsUnexpectedException_(0));
    client->future_onewayThrowsUnexpectedException(100).get();
    client->future_onewayThrowsUnexpectedException(0).get();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  });
}

void TransportCompatibilityTest::TestOneway_Connection_CloseNow() {
  connectToServer([](std::unique_ptr<TestServiceAsyncClient> client) {
    // It should not reach server - no EXPECT_CALL for addAfterDelay_(0, 5)

    // Observe the behavior if the connection is closed already
    auto channel = static_cast<ClientChannel*>(client->getChannel());
    channel->getEventBase()->runInEventBaseThreadAndWait(
        [&]() { channel->closeNow(); });

    EXPECT_THROW(
        client->future_addAfterDelay(0, 5).get(),
        apache::thrift::TTransportException);
  });
}

void TransportCompatibilityTest::TestOneway_ServerQueueTimeout() {
  // TODO: Even though we observe that the timeout functionality works fine for
  // Oneway PRC calls, the AsyncProcesor still executes the `cancelled`
  // requests.
  connectToServer(
      [this](std::unique_ptr<TestServiceAsyncClient> client) mutable {
        int32_t numCores = sysconf(_SC_NPROCESSORS_ONLN);
        int callCount = numCores + 1; // more than the core count!

        // TODO: fixme T22871783: Oneway tasks don't get cancelled
        EXPECT_CALL(*handler_.get(), addAfterDelay_(100, 5))
            .Times(AtMost(2 * callCount));

        server_->getServer()->setQueueTimeout(std::chrono::milliseconds(1));
        for (int i = 0; i < callCount; ++i) {
          EXPECT_NO_THROW(client->future_addAfterDelay(100, 5).get());
        }

        server_->getServer()->setQueueTimeout(std::chrono::milliseconds(1000));
        server_->getServer()->setUseClientTimeout(false);
        server_->getServer()->setTaskExpireTime(std::chrono::milliseconds(1));
        for (int i = 0; i < callCount; ++i) {
          EXPECT_NO_THROW(client->future_addAfterDelay(100, 5).get());
        }
      });
}

void TransportCompatibilityTest::TestOneway_Checksumming(bool usingSampling) {
  if (usingSampling) {
    THRIFT_FLAG_SET_MOCK(thrift_client_checksum_sampling_rate, 1);
  }
  connectToServer(
      [this, usingSampling](std::unique_ptr<TestServiceAsyncClient> client) {
        EXPECT_CALL(*handler_.get(), onewayLogBlob_(_));

        auto setCorruption = [&](bool val) {
          auto channel = static_cast<ClientChannel*>(client->getChannel());
          channel->getEventBase()->runInEventBaseThreadAndWait([&]() {
            auto p = std::make_shared<TAsyncSocketIntercepted::Params>();
            p->corruptLastWriteByte_ = val;
            dynamic_cast<TAsyncSocketIntercepted*>(channel->getTransport())
                ->setParams(p);
          });
        };

        for (bool shouldCorrupt : {false, true}) {
          static const int kSize = 32 << 10; // > IOBuf buf sharing thresh
          std::string asString(kSize, 'a');

          setCorruption(shouldCorrupt);

          auto payload = folly::IOBuf::copyBuffer(asString);
          client
              ->future_onewayLogBlob(
                  RpcOptions().setEnableChecksum(!usingSampling), *payload)
              .get();
          // Unlike request/response case, no exception is thrown here for
          // a one-way RPC.
          /* sleep override */
          std::this_thread::sleep_for(std::chrono::milliseconds(200));

          if (shouldCorrupt) {
            EXPECT_EQ(1, server_->observer_->taskKilled_);
          }
        }
        setCorruption(false);
      });
}

void TransportCompatibilityTest::TestRequestContextIsPreserved() {
  EXPECT_CALL(*handler_.get(), add_(5)).Times(1);

  // A separate server/client is spun up to verify that a client backed by a new
  // transport behaves correctly and does not trample the currently set
  // RequestContext. In this case, a THeader server is spun up, which is known
  // to correctly set RequestContext. A request/response is made through the
  // transport being tested, and it's verified that the RequestContext doesn't
  // change.

  auto service = std::make_shared<StrictMock<IntermHeaderService>>(
      FLAGS_host, server_->port_);

  SampleServer<IntermHeaderService> server(service);
  server.startServer();

  auto addr = folly::SocketAddress(FLAGS_host, server.port_);
  folly::AsyncSocket::UniquePtr sock(new TAsyncSocketIntercepted(
      folly::EventBaseManager::get()->getEventBase(), addr));
  auto channel = HeaderClientChannel::newChannel(std::move(sock));
  auto client =
      std::make_unique<IntermHeaderServiceAsyncClient>(std::move(channel));
  EXPECT_EQ(5, client->sync_callAdd(5));

  server.stopServer();
}

void TransportCompatibilityTest::TestBadPayload() {
  connectToServer([](std::unique_ptr<TestServiceAsyncClient> client) {
    auto channel = static_cast<ClientChannel*>(client->getChannel());
    channel->getEventBase()->runInEventBaseThreadAndWait([&]() {
      RequestRpcMetadata metadata;
      metadata.clientTimeoutMs() = 10000;
      metadata.kind() = RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE;
      metadata.name() = "name";
      metadata.protocol() = ProtocolId::BINARY;

      // Put a bad payload!
      auto payload = std::make_unique<folly::IOBuf>();

      RpcOptions rpcOptions;
      auto header = std::make_shared<THeader>();
      class ErrorCallback : public apache::thrift::RequestClientCallback {
       public:
        static apache::thrift::RequestClientCallback::Ptr create() {
          static folly::Indestructible<ErrorCallback> instance;
          return apache::thrift::RequestClientCallback::Ptr(instance.get());
        }

        void onResponse(
            apache::thrift::ClientReceiveState&&) noexcept override {
          ADD_FAILURE();
        }
        void onResponseError(folly::exception_wrapper) noexcept override {}
      };

      if (auto envelopeAndRequest =
              apache::thrift::EnvelopeUtil::stripRequestEnvelope(
                  std::move(payload))) {
        channel->sendRequestResponse(
            rpcOptions,
            envelopeAndRequest->first.methodName,
            apache::thrift::SerializedRequest(
                std::move(envelopeAndRequest->second)),
            std::move(header),
            ErrorCallback::create(),
            nullptr);
      } else {
        ErrorCallback::create().release()->onResponseError(
            folly::make_exception_wrapper<
                apache::thrift::transport::TTransportException>(
                apache::thrift::transport::TTransportException::CORRUPTED_DATA,
                "Unexpected problem stripping envelope"));
      }
    });
  });
}

void TransportCompatibilityTest::TestEvbSwitch() {
  connectToServer([this](std::unique_ptr<TestServiceAsyncClient> client) {
    EXPECT_CALL(*handler_.get(), sumTwoNumbers_(1, 2)).Times(3);

    folly::ScopedEventBaseThread sevbt;

    EXPECT_EQ(3, client->future_sumTwoNumbers(1, 2).get());

    auto channel = static_cast<ClientChannel*>(client->getChannel());
    auto evb = channel->getEventBase();
    evb->runInEventBaseThreadAndWait([&]() {
      EXPECT_TRUE(channel->isDetachable());

      channel->detachEventBase();
    });

    sevbt.getEventBase()->runInEventBaseThreadAndWait(
        [&]() { channel->attachEventBase(sevbt.getEventBase()); });

    // Execution happens on the new event base
    EXPECT_EQ(3, client->future_sumTwoNumbers(1, 2).get());

    // Attach the old one back
    sevbt.getEventBase()->runInEventBaseThreadAndWait([&]() {
      EXPECT_TRUE(channel->isDetachable());
      channel->detachEventBase();
    });

    evb->runInEventBaseThreadAndWait([&]() { channel->attachEventBase(evb); });

    // Execution happens on the old event base, along with the destruction
    EXPECT_EQ(3, client->future_sumTwoNumbers(1, 2).get());
  });
}

void TransportCompatibilityTest::TestEvbSwitch_Failure() {
  connectToServer([this](std::unique_ptr<TestServiceAsyncClient> client) {
    auto channel = static_cast<ClientChannel*>(client->getChannel());
    auto evb = channel->getEventBase();
    // If isDetachable() is called when a function is executing, it should
    // not be detachable
    callSleep(client.get(), 5000, 1000);
    /* sleep override - make sure request is started */
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    evb->runInEventBaseThreadAndWait([&]() {
      // As we have an active request, it should not be detachable!
      EXPECT_FALSE(channel->isDetachable());
    });

    // Once the request finishes, it should be detachable again
    /* sleep override - make sure request is finished */
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    evb->runInEventBaseThreadAndWait([&]() {
      // As we have an active request, it should not be detachable!
      EXPECT_TRUE(channel->isDetachable());
    });

    // If the latest request is sent while previous ones are still finishing
    // it should still not be detachable
    EXPECT_CALL(*handler_.get(), sumTwoNumbers_(1, 2)).Times(1);
    callSleep(client.get(), 5000, 1000);
    /* sleep override - make sure request is started */
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    client->future_sumTwoNumbers(1, 2).get();
    evb->runInEventBaseThreadAndWait([&]() {
      // As we have an active request, it should not be detachable!
      EXPECT_FALSE(channel->isDetachable());
    });

    /* sleep override - make sure request is finished */
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    evb->runInEventBaseThreadAndWait([&]() {
      // Should be detachable now
      EXPECT_TRUE(channel->isDetachable());
      // Detach to prove that we can destroy the object even if evb is detached
      channel->detachEventBase();
    });
  });
}

class CloseCallbackTest : public CloseCallback {
 public:
  void channelClosed() override {
    EXPECT_FALSE(closed_);
    closed_ = true;
  }
  bool isClosed() { return closed_; }

 private:
  bool closed_{false};
};

void TransportCompatibilityTest::TestCloseCallback() {
  connectToServer([](std::unique_ptr<TestServiceAsyncClient> client) {
    auto closeCb = std::make_unique<CloseCallbackTest>();
    auto channel = static_cast<ClientChannel*>(client->getChannel());
    channel->setCloseCallback(closeCb.get());

    EXPECT_FALSE(closeCb->isClosed());
    auto evb = channel->getEventBase();
    evb->runInEventBaseThreadAndWait([&]() { channel->closeNow(); });
    EXPECT_TRUE(closeCb->isClosed());
  });
}

void TransportCompatibilityTest::TestCustomAsyncProcessor() {
  class TestSendCallback : public MessageChannel::SendCallback {
   public:
    explicit TestSendCallback(MessageChannel::SendCallback* cb) : cb_(cb) {}
    void sendQueued() override {
      if (cb_) {
        cb_->sendQueued();
      }
    }

    void messageSent() override {
      if (cb_) {
        cb_->messageSent();
      }
      delete this;
    }

    void messageSendError(folly::exception_wrapper&& ex) override {
      if (cb_) {
        cb_->messageSendError(std::move(ex));
      }
      delete this;
    }

   private:
    MessageChannel::SendCallback* cb_;
  };

  class TestResponseChannelRequest
      : public apache::thrift::ResponseChannelRequest {
   public:
    explicit TestResponseChannelRequest(
        apache::thrift::ResponseChannelRequest::UniquePtr req)
        : req_(std::move(req)) {}

    bool isActive() const override { return req_->isActive(); }

    bool isOneway() const override { return req_->isOneway(); }

    bool includeEnvelope() const override { return req_->includeEnvelope(); }

    void sendReply(
        ResponsePayload&& response,
        MessageChannel::SendCallback* cb,
        folly::Optional<uint32_t> crc32) override {
      req_->sendReply(std::move(response), new TestSendCallback(cb), crc32);
    }

    void sendException(
        ResponsePayload&& response, MessageChannel::SendCallback* cb) override {
      req_->sendException(std::move(response), cb);
    }

    void sendErrorWrapped(
        folly::exception_wrapper ex, std::string exCode) override {
      req_->sendErrorWrapped(std::move(ex), std::move(exCode));
    }

   protected:
    bool tryStartProcessing() override {
      return callTryStartProcessing(req_.get());
    }

   private:
    apache::thrift::ResponseChannelRequest::UniquePtr req_;
  };

  class TestAsyncProcessor : public apache::thrift::AsyncProcessor {
   public:
    TestAsyncProcessor(
        std::unique_ptr<apache::thrift::AsyncProcessor> processor)
        : underlyingProcessor_(std::move(processor)) {}

    void processSerializedCompressedRequestWithMetadata(
        apache::thrift::ResponseChannelRequest::UniquePtr req,
        apache::thrift::SerializedCompressedRequest&& serializedRequest,
        const apache::thrift::AsyncProcessorFactory::MethodMetadata&
            methodMetadata,
        apache::thrift::protocol::PROTOCOL_TYPES protType,
        apache::thrift::Cpp2RequestContext* context,
        folly::EventBase* eb,
        apache::thrift::concurrency::ThreadManager* tm) override {
      underlyingProcessor_->processSerializedCompressedRequestWithMetadata(
          apache::thrift::ResponseChannelRequest::UniquePtr(
              new TestResponseChannelRequest(std::move(req))),
          std::move(serializedRequest),
          methodMetadata,
          protType,
          context,
          eb,
          tm);
    }

    void processInteraction(apache::thrift::ServerRequest&&) override {
      LOG(FATAL)
          << "This AsyncProcessor doesn't support Thrift interactions. "
          << "Please implement processInteraction to support interactions.";
    }

   private:
    std::unique_ptr<apache::thrift::AsyncProcessor> underlyingProcessor_;
  };

  class TestAsyncProcessorFactory
      : public apache::thrift::AsyncProcessorFactory {
   public:
    explicit TestAsyncProcessorFactory(
        std::shared_ptr<testutil::testservice::TestServiceMock> fac)
        : underlyingFac_(std::move(fac)) {}
    std::unique_ptr<apache::thrift::AsyncProcessor> getProcessor() override {
      return std::make_unique<TestAsyncProcessor>(
          underlyingFac_->getProcessor());
    }

    std::vector<apache::thrift::ServiceHandlerBase*> getServiceHandlers()
        override {
      return underlyingFac_->getServiceHandlers();
    }

    CreateMethodMetadataResult createMethodMetadata() override {
      return underlyingFac_->createMethodMetadata();
    }

   private:
    std::shared_ptr<testutil::testservice::TestServiceMock> underlyingFac_;
  };

  server_->getServer()->setInterface(handler_);
  startServer();
  TestRequestResponse_Simple();
}

void TransportCompatibilityTest::TestOnWriteQuiescence() {
  struct State {
    folly::fibers::Baton baton;
    folly::Optional<rocket::RocketServerConnection::ReadResumableHandle>
        resumeHandle;
  };

  class TestOnWriteQuiescenceRoutingHandler : public RocketRoutingHandler {
   public:
    TestOnWriteQuiescenceRoutingHandler(ThriftServer& server, State& state)
        : RocketRoutingHandler(server), state_(state) {}

   protected:
    void onConnection(rocket::RocketServerConnection& connection) override {
      connection.setOnWriteQuiescenceCallback(
          [this,
           callCounter = 0](rocket::RocketServerConnection::ReadPausableHandle
                                handle) mutable {
            if (callCounter == 0) {
              ++callCounter;
              state_.resumeHandle.emplace(std::move(handle).pause());
              state_.baton.post();
            }
          });
    }

   private:
    State& state_;
  };

  State s;
  server_->getServer()->clearRoutingHandlers();
  server_->getServer()->addRoutingHandler(
      std::make_unique<TestOnWriteQuiescenceRoutingHandler>(
          *server_->getServer(), s));
  startServer();
  connectToServer([this, &s](std::unique_ptr<TestServiceAsyncClient> client) {
    EXPECT_CALL(*handler_.get(), sumTwoNumbers_(1, 2)).Times(AtLeast(2));
    // wait for first response to complete, write quiescence callback expected
    // to pause further read.
    EXPECT_EQ(3, client->future_sumTwoNumbers(1, 2).get());
    s.baton.wait();
    EXPECT_TRUE(s.resumeHandle.hasValue());
    // further request-response won't get through as read paused
    RpcOptions options;
    options.setTimeout(std::chrono::milliseconds(80));
    EXPECT_THROW(
        client->future_sumTwoNumbers(options, 1, 2).get(), TTransportException);
    auto& eb = s.resumeHandle->getEventBase();
    eb.runInEventBaseThreadAndWait(
        [&s]() { std::move(s.resumeHandle).value().resume(); });
    // after resume handle called request-response get through again
    EXPECT_EQ(3, client->future_sumTwoNumbers(1, 2).get());
  });
}

} // namespace apache::thrift
