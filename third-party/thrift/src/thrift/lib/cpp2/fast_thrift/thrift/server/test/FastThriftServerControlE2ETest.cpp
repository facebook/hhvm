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
 * End-to-end test for FastThriftServer::setControlInterface.
 *
 * Boots a real FastThriftServer with a user handler plus a control handler
 * wired into the control slot, then drives both over real RocketClientChannel
 * clients to prove that:
 *
 *  - a control method reaches the control child and nothing else,
 *  - a user method reaches the user child and never leaks onto the control
 *    child,
 *  - the control child is handed the server's CPU executor rather than being
 *    silently left inline,
 *  - the user handler wins when both claim the same method name, which is
 *    what registering the control child after the user child buys.
 *
 * No control IDL ships with fast_thrift — the slot is generic, because the
 * canonical Control service depends on Meta-internal settings types — so
 * these cases stand in an arbitrary FastServer service for the control
 * handler.
 */

#include <gtest/gtest.h>

#include <atomic>
#include <memory>
#include <string>

#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/ScopedEventBaseThread.h>

#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/fast_thrift/interface/control/ControlServerInterface.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/FastThriftServer.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/AuxInterfaceProbeService.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/AuxInterfaceProbeServiceAsyncClient.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/FastThriftServer.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/FastThriftServerAsyncClient.h>

THRIFT_FLAG_DECLARE_bool(rocket_client_binary_rpc_metadata_encoding);

namespace apache::thrift::fast_thrift::thrift::test::integration::e2e {

namespace ftt = ::apache::thrift::fast_thrift::thrift;
namespace integration =
    ::apache::thrift::fast_thrift::thrift::test::integration;
using ::apache::thrift::FastServiceHandler;
using ::apache::thrift::fast_thrift::ControlServerInterface;
using ::apache::thrift::fast_thrift::thrift::test::integration::
    AuxInterfaceProbeService;
using ::apache::thrift::fast_thrift::thrift::test::integration::EchoResponse;

namespace {

// User-side handler. Counts per-method invocations so a control RPC that
// leaked onto the user child is visible.
class UserHandler : public FastServiceHandler<integration::FastThriftServer> {
 public:
  std::atomic<int> pingCount{0};

  void async_tm_ping(ftt::FastHandlerCallbackPtr<void> cb) override {
    pingCount.fetch_add(1, std::memory_order_relaxed);
    cb->done();
  }

  void async_tm_add(
      ftt::FastHandlerCallbackPtr<int64_t> cb, int64_t a, int64_t b) override {
    cb->result(a + b);
  }

  void async_tm_echo(
      ftt::FastHandlerCallbackPtr<std::unique_ptr<EchoResponse>> cb,
      std::unique_ptr<std::string> message) override {
    auto resp = std::make_unique<EchoResponse>();
    resp->message() = std::string("user:") + *message;
    cb->result(std::move(resp));
  }

  void async_tm_lookup(
      ftt::FastHandlerCallbackPtr<std::unique_ptr<EchoResponse>> cb,
      int32_t /*id*/) override {
    auto resp = std::make_unique<EchoResponse>();
    resp->message() = "user-lookup";
    cb->result(std::move(resp));
  }

  void async_tm_secureLookup(
      ftt::FastHandlerCallbackPtr<std::unique_ptr<EchoResponse>> cb,
      int32_t /*id*/,
      std::unique_ptr<std::string> /*user*/) override {
    auto resp = std::make_unique<EchoResponse>();
    resp->message() = "user-secure";
    cb->result(std::move(resp));
  }
};

// Control-side handler. Method names are disjoint from the user service, so
// routing is unambiguous. Also derives from the ControlServerInterface marker
// that the typed setControlInterface requires.
class ControlHandler : public FastServiceHandler<AuxInterfaceProbeService>,
                       public ControlServerInterface {
 public:
  std::atomic<int> greetCount{0};
  // Thread placement observed by the most recent greet, so the test can tell
  // whether the control child was handed the server's CPU executor.
  std::atomic<bool> greetRanOnEventBase{false};

  void async_tm_probePing(ftt::FastHandlerCallbackPtr<void> cb) override {
    cb->done();
  }

  void async_tm_probeGreet(
      ftt::FastHandlerCallbackPtr<std::unique_ptr<std::string>> cb,
      std::unique_ptr<std::string> name) override {
    greetCount.fetch_add(1, std::memory_order_relaxed);
    greetRanOnEventBase.store(
        cb->getEventBase()->isInEventBaseThread(), std::memory_order_relaxed);
    cb->result(std::make_unique<std::string>("control:" + *name));
  }
};

// Control handler that serves the *user* service, so every method name
// collides. Used to pin down precedence between the two children.
class ShadowingControlHandler
    : public FastServiceHandler<integration::FastThriftServer>,
      public ControlServerInterface {
 public:
  std::atomic<int> pingCount{0};

  void async_tm_ping(ftt::FastHandlerCallbackPtr<void> cb) override {
    pingCount.fetch_add(1, std::memory_order_relaxed);
    cb->done();
  }

  void async_tm_add(
      ftt::FastHandlerCallbackPtr<int64_t> cb,
      int64_t /*a*/,
      int64_t /*b*/) override {
    cb->result(-1);
  }

  void async_tm_echo(
      ftt::FastHandlerCallbackPtr<std::unique_ptr<EchoResponse>> cb,
      std::unique_ptr<std::string> /*message*/) override {
    auto resp = std::make_unique<EchoResponse>();
    resp->message() = "control";
    cb->result(std::move(resp));
  }

  void async_tm_lookup(
      ftt::FastHandlerCallbackPtr<std::unique_ptr<EchoResponse>> cb,
      int32_t /*id*/) override {
    auto resp = std::make_unique<EchoResponse>();
    resp->message() = "control";
    cb->result(std::move(resp));
  }

  void async_tm_secureLookup(
      ftt::FastHandlerCallbackPtr<std::unique_ptr<EchoResponse>> cb,
      int32_t /*id*/,
      std::unique_ptr<std::string> /*user*/) override {
    auto resp = std::make_unique<EchoResponse>();
    resp->message() = "control";
    cb->result(std::move(resp));
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

// Parameterized on numCPUThreads so every case runs twice: once with handlers
// inline on the IO thread, once dispatched to a CPU pool. The control child
// must behave identically either way.
class FastThriftServerControlE2ETest
    : public ::testing::TestWithParam<uint32_t> {
 protected:
  bool usingCPUPool() const { return GetParam() > 0; }

  void SetUp() override {
    THRIFT_FLAG_SET_MOCK(rocket_client_binary_rpc_metadata_encoding, true);
    userHandler_ = std::make_shared<UserHandler>();
    clientThread_ = std::make_unique<folly::ScopedEventBaseThread>();
  }

  void TearDown() override {
    clientThread_.reset();
    if (server_) {
      server_->stop();
      server_.reset();
    }
  }

  // Boot a server with the user handler plus `controlHandler` in the control
  // slot. Separate from SetUp because the precedence case needs a different
  // handler in that slot.
  void startServer(std::shared_ptr<ControlServerInterface> controlHandler) {
    ftt::FastThriftServerConfig config;
    config.address = folly::SocketAddress("::1", 0);
    config.numIOThreads = 1;
    config.numCPUThreads = GetParam();

    server_ = std::make_unique<ftt::FastThriftServer>(std::move(config));
    server_->setInterface(userHandler_);
    server_->setControlInterface(std::move(controlHandler));
    server_->start();
  }

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
  std::unique_ptr<ftt::FastThriftServer> server_;
  std::unique_ptr<folly::ScopedEventBaseThread> clientThread_;
};

// A user-only method and a control-only method issued against the same
// server. Each must reach exactly one handler. The thread-placement check on
// the control child proves it was handed the CPU executor rather than
// silently left inline.
TEST_P(FastThriftServerControlE2ETest, RoutesUserAndControlMethods) {
  auto controlHandler = std::make_shared<ControlHandler>();
  startServer(controlHandler);
  ASSERT_TRUE(server_->hasControlHandler());

  auto userClient = createClient<integration::FastThriftServer>();
  auto ctlClient = createClient<AuxInterfaceProbeService>();

  userClient->semifuture_ping().get();
  EXPECT_EQ(userHandler_->pingCount.load(), 1);
  EXPECT_EQ(controlHandler->greetCount.load(), 0)
      << "user RPC must not reach the control handler";

  auto greeting = ctlClient->semifuture_probeGreet("world").get();
  EXPECT_EQ(greeting, "control:world");
  EXPECT_EQ(controlHandler->greetCount.load(), 1);
  EXPECT_EQ(userHandler_->pingCount.load(), 1)
      << "control RPC must not reach the user handler";
  EXPECT_EQ(controlHandler->greetRanOnEventBase.load(), !usingCPUPool());

  destroyClientOnEvb(userClient);
  destroyClientOnEvb(ctlClient);
}

// The control child is registered after the user child, so on a method-name
// collision the user handler serves the request and the control handler is
// unreachable. Flipping the registration order would silently let a control
// handler shadow the user service.
TEST_P(FastThriftServerControlE2ETest, UserHandlerWinsMethodNameConflict) {
  auto controlHandler = std::make_shared<ShadowingControlHandler>();
  startServer(controlHandler);

  auto userClient = createClient<integration::FastThriftServer>();

  userClient->semifuture_ping().get();
  EXPECT_EQ(userHandler_->pingCount.load(), 1);
  EXPECT_EQ(controlHandler->pingCount.load(), 0)
      << "control handler must not shadow a user method";

  EXPECT_EQ(userClient->semifuture_add(2, 3).get(), 5)
      << "shadowed methods must still resolve to the user handler";

  destroyClientOnEvb(userClient);
}

// A server with no control handler must still serve the user interface, and
// must report the slot as empty rather than installing something by default —
// neither stack wires a control handler unless the service asks for one.
TEST_P(FastThriftServerControlE2ETest, SlotIsEmptyUnlessWired) {
  ftt::FastThriftServerConfig config;
  config.address = folly::SocketAddress("::1", 0);
  config.numIOThreads = 1;
  config.numCPUThreads = GetParam();

  server_ = std::make_unique<ftt::FastThriftServer>(std::move(config));
  server_->setInterface(userHandler_);
  server_->start();

  EXPECT_FALSE(server_->hasControlHandler());

  auto userClient = createClient<integration::FastThriftServer>();
  userClient->semifuture_ping().get();
  EXPECT_EQ(userHandler_->pingCount.load(), 1);

  destroyClientOnEvb(userClient);
}

INSTANTIATE_TEST_SUITE_P(
    Inline,
    FastThriftServerControlE2ETest,
    ::testing::Values(uint32_t{0}),
    [](const auto&) { return "InlineOnIOThread"; });

INSTANTIATE_TEST_SUITE_P(
    CPUPool,
    FastThriftServerControlE2ETest,
    ::testing::Values(uint32_t{4}),
    [](const auto&) { return "CPUThreadPool"; });
