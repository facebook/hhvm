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

/**
 * End-to-end test for FastThriftServer::setMonitoringInterface.
 *
 * Boots a real FastThriftServer with two handlers — a user FastThriftServer
 * service and a monitoring handler implementing the canonical Monitor
 * service from interface/monitor/if/monitor.thrift — connected via the
 * composite app adapter, then exercises real RocketClientChannel clients to
 * prove that:
 *  - user methods route to the user handler,
 *  - monitoring methods route to the monitoring handler,
 *  - the SETUP frame's negotiated metadata protocol propagates to BOTH
 *    children (verified by a successful RPC under each protocol).
 */

#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <string>

#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/synchronization/Baton.h>

#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/fast_thrift/interface/monitor/MonitoringServerInterface.h>
#include <thrift/lib/cpp2/fast_thrift/interface/monitor/if/gen-cpp2/Monitor.h>
#include <thrift/lib/cpp2/fast_thrift/interface/monitor/if/gen-cpp2/MonitorAsyncClient.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/FastThriftServer.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/FastThriftServer.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/FastThriftServerAsyncClient.h>

THRIFT_FLAG_DECLARE_bool(rocket_client_binary_rpc_metadata_encoding);

namespace apache::thrift::fast_thrift::thrift::test::integration::e2e {

namespace ftt = ::apache::thrift::fast_thrift::thrift;
namespace integration =
    ::apache::thrift::fast_thrift::thrift::test::integration;
using ::apache::thrift::FastServiceHandler;
using ::apache::thrift::fast_thrift::Monitor;
using ::apache::thrift::fast_thrift::MonitoringServerInterface;
using ::apache::thrift::fast_thrift::thrift::test::integration::EchoResponse;

namespace {

// User-side handler. Tracks per-method invocation so we can prove the user
// channel of the composite is actually being driven (not the monitoring
// one).
class UserHandler : public FastServiceHandler<integration::FastThriftServer> {
 public:
  std::atomic<int> pingCount{0};
  std::atomic<int> addCount{0};

  void async_eb_ping(ftt::FastHandlerCallbackPtr<void> cb) override {
    pingCount.fetch_add(1, std::memory_order_relaxed);
    cb->done();
  }

  void async_eb_add(
      ftt::FastHandlerCallbackPtr<int64_t> cb, int64_t a, int64_t b) override {
    addCount.fetch_add(1, std::memory_order_relaxed);
    cb->result(a + b);
  }

  void async_eb_echo(
      ftt::FastHandlerCallbackPtr<std::unique_ptr<EchoResponse>> cb,
      std::unique_ptr<std::string> message) override {
    auto resp = std::make_unique<EchoResponse>();
    resp->message() = std::string("user:") + *message;
    cb->result(std::move(resp));
  }

  void async_eb_lookup(
      ftt::FastHandlerCallbackPtr<std::unique_ptr<EchoResponse>> cb,
      int32_t /*id*/) override {
    auto resp = std::make_unique<EchoResponse>();
    resp->message() = "user-lookup";
    cb->result(std::move(resp));
  }

  void async_eb_secureLookup(
      ftt::FastHandlerCallbackPtr<std::unique_ptr<EchoResponse>> cb,
      int32_t /*id*/,
      std::unique_ptr<std::string> /*user*/) override {
    auto resp = std::make_unique<EchoResponse>();
    resp->message() = "user-secure";
    cb->result(std::move(resp));
  }
};

// Monitoring-side handler. Implements the canonical Monitor service
// (interface/monitor/if/monitor.thrift) but only overrides aliveSince —
// the cheapest pure-compute method, distinct from anything FastThriftServer
// exposes, so routing is unambiguously testable. Also derives from the
// MonitoringServerInterface marker which the typed setMonitoringInterface
// requires.
class MonitoringHandler : public FastServiceHandler<Monitor>,
                          public MonitoringServerInterface {
 public:
  static constexpr std::int64_t kAliveSinceValue = 1717171717;
  std::atomic<int> aliveSinceCount{0};

  void async_eb_aliveSince(
      ftt::FastHandlerCallbackPtr<std::int64_t> cb) override {
    aliveSinceCount.fetch_add(1, std::memory_order_relaxed);
    cb->result(kAliveSinceValue);
  }
};

} // namespace
} // namespace apache::thrift::fast_thrift::thrift::test::integration::e2e

// gtest macros must live outside a namespace ending in `e2e` for the same
// reason FastThriftServerIntegrationTest puts its TEST_F outside the
// `integration` namespace — bring symbols into scope via using-directive.
using namespace apache::thrift::fast_thrift::thrift::test::integration::
    e2e; // NOLINT
namespace integration =
    ::apache::thrift::fast_thrift::thrift::test::integration;
namespace ftt = ::apache::thrift::fast_thrift::thrift;

class FastThriftServerMonitoringE2ETest : public ::testing::Test {
 protected:
  void SetUp() override {
    THRIFT_FLAG_SET_MOCK(rocket_client_binary_rpc_metadata_encoding, true);

    userHandler_ = std::make_shared<UserHandler>();
    monitoringHandler_ = std::make_shared<MonitoringHandler>();

    ftt::FastThriftServerConfig config;
    config.address = folly::SocketAddress("::1", 0);
    config.numIOThreads = 1;

    server_ = std::make_unique<ftt::FastThriftServer>(std::move(config));
    server_->setInterface(userHandler_);
    server_->setMonitoringInterface(monitoringHandler_);
    server_->start();

    clientThread_ = std::make_unique<folly::ScopedEventBaseThread>();
  }

  void TearDown() override {
    clientThread_.reset();
    server_->stop();
    server_.reset();
  }

  // Build a client of `Service` over a fresh RocketClientChannel pointed at
  // the running server. Both client templates instantiate over the same
  // physical connection model — distinct typed clients land at the same
  // server port, and the composite picks per-method.
  template <typename Service>
  std::unique_ptr<apache::thrift::Client<Service>> createClient() {
    auto* evb = clientThread_->getEventBase();
    std::unique_ptr<apache::thrift::Client<Service>> client;
    evb->runInEventBaseThreadAndWait([&] {
      auto socket = folly::AsyncSocket::newSocket(evb, server_->getAddress());
      auto channel =
          apache::thrift::RocketClientChannel::newChannel(std::move(socket));
      client =
          std::make_unique<apache::thrift::Client<Service>>(std::move(channel));
    });
    return client;
  }

  template <typename Client>
  void destroyClientOnEvb(std::unique_ptr<Client>& client) {
    clientThread_->getEventBase()->runInEventBaseThreadAndWait(
        [&] { client.reset(); });
  }

  std::shared_ptr<UserHandler> userHandler_;
  std::shared_ptr<MonitoringHandler> monitoringHandler_;
  std::unique_ptr<ftt::FastThriftServer> server_;
  std::unique_ptr<folly::ScopedEventBaseThread> clientThread_;
};

// User-only method (ping) and monitoring-only method (aliveSince) issued
// over the same physical server. Both must succeed AND only their
// respective handler counter must increment — proves the composite routes
// by method name without crosstalk.
TEST_F(FastThriftServerMonitoringE2ETest, RoutesUserAndMonitoringMethods) {
  auto userClient = createClient<integration::FastThriftServer>();
  auto monClient = createClient<Monitor>();

  userClient->semifuture_ping().get();
  EXPECT_EQ(userHandler_->pingCount.load(), 1);
  EXPECT_EQ(monitoringHandler_->aliveSinceCount.load(), 0)
      << "user RPC must not reach the monitoring handler";

  auto aliveSince = monClient->semifuture_aliveSince().get();
  EXPECT_EQ(aliveSince, MonitoringHandler::kAliveSinceValue);
  EXPECT_EQ(monitoringHandler_->aliveSinceCount.load(), 1);
  EXPECT_EQ(userHandler_->pingCount.load(), 1)
      << "monitoring RPC must not reach the user handler";

  destroyClientOnEvb(userClient);
  destroyClientOnEvb(monClient);
}

// Both handlers must end up with the negotiated metadata protocol pushed
// through the composite's setMetadataProtocol fan-out — otherwise one of
// these RPCs would fail with REQUEST_PARSING_FAILURE.
TEST_F(FastThriftServerMonitoringE2ETest, BothHandlersHonorNegotiatedProtocol) {
  auto userClient = createClient<integration::FastThriftServer>();
  auto monClient = createClient<Monitor>();

  // Multiple round-trips on each: covers both children's serialization +
  // deserialization paths after SETUP completes.
  EXPECT_EQ(userClient->semifuture_add(2, 3).get(), 5);
  EXPECT_EQ(userClient->semifuture_add(10, 20).get(), 30);
  EXPECT_EQ(
      monClient->semifuture_aliveSince().get(),
      MonitoringHandler::kAliveSinceValue);
  EXPECT_EQ(
      monClient->semifuture_aliveSince().get(),
      MonitoringHandler::kAliveSinceValue);

  EXPECT_EQ(userHandler_->addCount.load(), 2);
  EXPECT_EQ(monitoringHandler_->aliveSinceCount.load(), 2);

  destroyClientOnEvb(userClient);
  destroyClientOnEvb(monClient);
}
