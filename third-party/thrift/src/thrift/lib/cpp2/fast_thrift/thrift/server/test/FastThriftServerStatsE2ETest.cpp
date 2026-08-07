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
 * End-to-end test for FastThriftServer::setStats.
 *
 * Boots a real FastThriftServer with a ServerStats attached, drives it with a
 * real RocketClientChannel, and asserts on the resulting counter values —
 * a metrics handler that is wired into the pipeline but never increments
 * anything would otherwise go unnoticed.
 *
 * Kept separate from FastThriftServerMetricsTest (which covers the legacy
 * FastThriftServerT) because FastThriftChannelServer.h declares its own
 * FastThriftServerConfig, so the two servers cannot share a translation unit.
 */

#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <string>

#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/synchronization/Baton.h>

#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/fast_thrift/common/ServerStats.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/FastThriftServer.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/FastThriftServer.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/FastThriftServerAsyncClient.h>

THRIFT_FLAG_DECLARE_bool(rocket_client_binary_rpc_metadata_encoding);

namespace apache::thrift::fast_thrift::thrift::test::integration::stats {

namespace ftt = ::apache::thrift::fast_thrift::thrift;
namespace integration =
    ::apache::thrift::fast_thrift::thrift::test::integration;

using ::apache::thrift::FastServiceHandler;
using ::apache::thrift::fast_thrift::ServerStats;
using ::apache::thrift::fast_thrift::ServerStatsShard;
using ::apache::thrift::fast_thrift::thrift::test::integration::EchoResponse;

namespace {

class TestHandler : public FastServiceHandler<integration::FastThriftServer> {
 public:
  void async_eb_ping(ftt::FastHandlerCallbackPtr<void> cb) override {
    cb->done();
  }

  void async_eb_add(
      ftt::FastHandlerCallbackPtr<int64_t> cb, int64_t a, int64_t b) override {
    cb->result(a + b);
  }

  void async_eb_echo(
      ftt::FastHandlerCallbackPtr<std::unique_ptr<EchoResponse>> cb,
      std::unique_ptr<std::string> message) override {
    auto resp = std::make_unique<EchoResponse>();
    resp->message() = *message;
    cb->result(std::move(resp));
  }

  void async_eb_lookup(
      ftt::FastHandlerCallbackPtr<std::unique_ptr<EchoResponse>> cb,
      int32_t /*id*/) override {
    cb->result(std::make_unique<EchoResponse>());
  }

  void async_eb_secureLookup(
      ftt::FastHandlerCallbackPtr<std::unique_ptr<EchoResponse>> cb,
      int32_t /*id*/,
      std::unique_ptr<std::string> /*user*/) override {
    cb->result(std::make_unique<EchoResponse>());
  }
};

} // namespace
} // namespace apache::thrift::fast_thrift::thrift::test::integration::stats

using namespace apache::thrift::fast_thrift::thrift::test::
    integration:: // NOLINT
    stats;
namespace ftt = ::apache::thrift::fast_thrift::thrift;
namespace integration =
    ::apache::thrift::fast_thrift::thrift::test::integration;

class FastThriftServerStatsE2ETest : public ::testing::Test {
 protected:
  // Not in SetUp: one test needs the server started without stats.
  void startServer(bool withStats) {
    THRIFT_FLAG_SET_MOCK(rocket_client_binary_rpc_metadata_encoding, true);

    handler_ = std::make_shared<TestHandler>();

    ftt::FastThriftServerConfig config;
    config.address = folly::SocketAddress("::1", 0);
    config.numIOThreads = 1;

    server_ = std::make_unique<ftt::FastThriftServer>(std::move(config));
    server_->setInterface(handler_);
    if (withStats) {
      stats_ = std::make_shared<ServerStats>();
      server_->setStats(stats_);
    }
    server_->start();

    clientThread_ = std::make_unique<folly::ScopedEventBaseThread>();
  }

  void TearDown() override {
    clientThread_.reset();
    if (server_) {
      server_->stop();
      server_.reset();
    }
  }

  std::unique_ptr<apache::thrift::Client<integration::FastThriftServer>>
  createClient() {
    auto* evb = clientThread_->getEventBase();
    std::unique_ptr<apache::thrift::Client<integration::FastThriftServer>>
        client;
    evb->runInEventBaseThreadAndWait([&] {
      auto socket = folly::AsyncSocket::newSocket(evb, server_->getAddress());
      auto channel =
          apache::thrift::RocketClientChannel::newChannel(std::move(socket));
      client = std::make_unique<
          apache::thrift::Client<integration::FastThriftServer>>(
          std::move(channel));
    });
    return client;
  }

  void destroyClientOnEvb(
      std::unique_ptr<apache::thrift::Client<integration::FastThriftServer>>&
          client) {
    clientThread_->getEventBase()->runInEventBaseThreadAndWait(
        [&] { client.reset(); });
  }

  template <typename Fn>
  auto syncCall(Fn&& fn) {
    using SemiFuture = std::invoke_result_t<Fn>;
    using T = typename SemiFuture::value_type;
    auto* evb = clientThread_->getEventBase();
    folly::Baton<> baton;
    if constexpr (std::is_void_v<T>) {
      evb->runInEventBaseThread([&] {
        fn().via(evb)
            .thenValue([&](folly::Unit) { baton.post(); })
            .thenError([&](const folly::exception_wrapper& ew) {
              ADD_FAILURE() << "RPC failed: " << folly::exceptionStr(ew);
              baton.post();
            });
      });
      baton.wait();
    } else {
      T result{};
      evb->runInEventBaseThread([&] {
        fn().via(evb)
            .thenValue([&](T val) {
              result = std::move(val);
              baton.post();
            })
            .thenError([&](const folly::exception_wrapper& ew) {
              ADD_FAILURE() << "RPC failed: " << folly::exceptionStr(ew);
              baton.post();
            });
      });
      baton.wait();
      return result;
    }
  }

  // Sums every EventBase's shard, reading each from its owning thread as
  // ServerStats requires. Mirrors what FastThriftStatsPublisher does.
  ServerStatsShard totals() {
    ServerStatsShard total;
    for (auto& ka : server_->getIOThreadPool()->getAllEventBases()) {
      auto* evb = ka.get();
      evb->runInEventBaseThreadAndWait([&] {
        const auto& shard = stats_->currentThreadShard();
        total.rocketInbound.incrementValue(shard.rocketInbound.value());
        total.rocketOutbound.incrementValue(shard.rocketOutbound.value());
        total.rocketErrors.incrementValue(shard.rocketErrors.value());
        total.rocketActive.incrementValue(shard.rocketActive.value());
        total.thriftInbound.incrementValue(shard.thriftInbound.value());
        total.thriftOutbound.incrementValue(shard.thriftOutbound.value());
        total.thriftErrors.incrementValue(shard.thriftErrors.value());
        total.thriftActive.incrementValue(shard.thriftActive.value());
      });
    }
    return total;
  }

  std::shared_ptr<TestHandler> handler_;
  std::shared_ptr<ServerStats> stats_;
  std::unique_ptr<ftt::FastThriftServer> server_;
  std::unique_ptr<folly::ScopedEventBaseThread> clientThread_;
};

TEST_F(FastThriftServerStatsE2ETest, CountsRequestsAndResponses) {
  startServer(/*withStats=*/true);
  auto client = createClient();

  EXPECT_EQ(syncCall([&] { return client->semifuture_add(10, 20); }), 30);
  syncCall([&] { return client->semifuture_ping(); });

  // Each response reaching the client means the server already ran onWrite
  // for it, so both directions have settled — no polling required.
  const auto total = totals();

  // One message in and one out per request, so these are exact.
  EXPECT_EQ(total.thriftInbound.value(), 2);
  EXPECT_EQ(total.thriftOutbound.value(), 2);
  EXPECT_EQ(total.thriftErrors.value(), 0);
  // Incremented on the way in, decremented on the way out.
  EXPECT_EQ(total.thriftActive.value(), 0);

  // Rocket additionally carries connection-setup traffic, so only a lower
  // bound is stable.
  EXPECT_GE(total.rocketInbound.value(), 2);
  EXPECT_GE(total.rocketOutbound.value(), 2);
  EXPECT_EQ(total.rocketErrors.value(), 0);

  destroyClientOnEvb(client);
}

// Shards are created lazily per EventBase, on connection build.
TEST_F(FastThriftServerStatsE2ETest, NoCountsBeforeFirstConnection) {
  startServer(/*withStats=*/true);

  const auto total = totals();
  EXPECT_EQ(total.thriftInbound.value(), 0);
  EXPECT_EQ(total.rocketInbound.value(), 0);
}

// Leaving stats unset omits both metrics handlers; the server must be
// unaffected.
TEST_F(FastThriftServerStatsE2ETest, ServesNormallyWithoutStats) {
  startServer(/*withStats=*/false);
  auto client = createClient();

  EXPECT_EQ(syncCall([&] { return client->semifuture_add(1, 2); }), 3);
  EXPECT_EQ(server_->getStats(), nullptr);

  destroyClientOnEvb(client);
}
