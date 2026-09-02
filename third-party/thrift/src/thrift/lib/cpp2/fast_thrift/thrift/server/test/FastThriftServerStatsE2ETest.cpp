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
 * End-to-end test for FastThriftServer's counters, reached either by
 * setStats/setConnectionStats or by FastThriftServerConfig::enableStats.
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

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <thread>

#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/synchronization/Baton.h>

#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/fast_thrift/common/ServerStats.h>
#include <thrift/lib/cpp2/fast_thrift/connection/common/ConnectionStats.h>
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
using ::apache::thrift::fast_thrift::connection::ConnectionStats;
using ::apache::thrift::fast_thrift::connection::ConnectionStatsShard;
using ::apache::thrift::fast_thrift::thrift::test::integration::EchoResponse;

namespace {

class TestHandler : public FastServiceHandler<integration::FastThriftServer> {
 public:
  void async_tm_ping(ftt::FastHandlerCallbackPtr<void> cb) override {
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
    resp->message() = *message;
    cb->result(std::move(resp));
  }

  void async_tm_lookup(
      ftt::FastHandlerCallbackPtr<std::unique_ptr<EchoResponse>> cb,
      int32_t /*id*/) override {
    cb->result(std::make_unique<EchoResponse>());
  }

  void async_tm_secureLookup(
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
  // Where the server under test gets its counters from.
  enum class StatsMode {
    // Neither route, so no metrics handler is built into any pipeline.
    kNone,
    // Embedder-supplied instances, via the setStats family.
    kSetters,
    // Server-materialized, via FastThriftServerConfig::enableStats.
    kConfigFlag,
  };

  // Not in SetUp: the mode differs per test.
  void startServer(StatsMode mode) {
    THRIFT_FLAG_SET_MOCK(rocket_client_binary_rpc_metadata_encoding, true);

    handler_ = std::make_shared<TestHandler>();

    ftt::FastThriftServerConfig config;
    config.address = folly::SocketAddress("::1", 0);
    config.numIOThreads = 1;
    config.enableStats = mode == StatsMode::kConfigFlag;

    server_ = std::make_unique<ftt::FastThriftServer>(std::move(config));
    server_->setInterface(handler_);
    if (mode == StatsMode::kSetters) {
      stats_ = std::make_shared<ServerStats>();
      server_->setStats(stats_);
      connectionStats_ = std::make_shared<ConnectionStats>();
      server_->setConnectionStats(connectionStats_);
    }
    server_->start();
    if (mode == StatsMode::kConfigFlag) {
      // start() is what materializes them, so this is the earliest the
      // fixture can hold them.
      stats_ = server_->getStats();
      connectionStats_ = server_->getConnectionStats();
    }

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

  // Sums every EventBase's connection shard, reading each from its owning
  // thread as ConnectionStats requires.
  ConnectionStatsShard connectionTotals() {
    ConnectionStatsShard total;
    for (auto& ka : server_->getIOThreadPool()->getAllEventBases()) {
      auto* evb = ka.get();
      evb->runInEventBaseThreadAndWait([&] {
        const auto& shard = connectionStats_->currentThreadShard();
        total.connectionsAccepted.incrementValue(
            shard.connectionsAccepted.value());
        total.connectionsActive.incrementValue(shard.connectionsActive.value());
      });
    }
    return total;
  }

  // A connection tears down asynchronously after the client goes away, so the
  // active gauge settles some time after the call that triggered it. Polls
  // rather than sleeping a fixed interval, and fails loudly on timeout so a
  // gauge that never comes down cannot pass as a slow one.
  void waitForActiveConnections(int64_t expected) {
    constexpr auto kTimeout = std::chrono::seconds(5);
    const auto deadline = std::chrono::steady_clock::now() + kTimeout;
    int64_t last = -1;
    while (std::chrono::steady_clock::now() < deadline) {
      last = connectionTotals().connectionsActive.value();
      if (last == expected) {
        return;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    ADD_FAILURE() << "active connections never reached " << expected
                  << "; last read " << last;
  }

  std::shared_ptr<TestHandler> handler_;
  std::shared_ptr<ServerStats> stats_;
  std::shared_ptr<ConnectionStats> connectionStats_;
  std::unique_ptr<ftt::FastThriftServer> server_;
  std::unique_ptr<folly::ScopedEventBaseThread> clientThread_;
};

// A connection is counted once it is established — the tail of the acceptance
// pipeline, past TLS — which is what the classic server's connAccepted() also
// means, so a socket that dies mid-handshake is counted by neither.
TEST_F(FastThriftServerStatsE2ETest, CountsAcceptedConnections) {
  startServer(StatsMode::kSetters);
  auto client = createClient();
  // The completed RPC is what guarantees the connection is established and
  // its pipeline built; the socket alone would leave that racing.
  syncCall([&] { return client->semifuture_ping(); });

  const auto total = connectionTotals();
  EXPECT_EQ(total.connectionsAccepted.value(), 1);
  EXPECT_EQ(total.connectionsActive.value(), 1);

  destroyClientOnEvb(client);
}

// The gauge is only useful if it comes back down. Its decrement lives beside
// the connection map's own prune, so the two cannot disagree.
TEST_F(FastThriftServerStatsE2ETest, ActiveConnectionsFallsOnDisconnect) {
  startServer(StatsMode::kSetters);
  auto client = createClient();
  syncCall([&] { return client->semifuture_ping(); });
  ASSERT_EQ(connectionTotals().connectionsActive.value(), 1);

  destroyClientOnEvb(client);
  waitForActiveConnections(0);

  // The connection is gone, but having existed is not undone.
  EXPECT_EQ(connectionTotals().connectionsAccepted.value(), 1);
}

// Two clients means two connections — the gauge tracks concurrency, not a
// single connection's presence.
TEST_F(FastThriftServerStatsE2ETest, CountsConcurrentConnections) {
  startServer(StatsMode::kSetters);
  auto first = createClient();
  auto second = createClient();
  syncCall([&] { return first->semifuture_ping(); });
  syncCall([&] { return second->semifuture_ping(); });

  const auto total = connectionTotals();
  EXPECT_EQ(total.connectionsAccepted.value(), 2);
  EXPECT_EQ(total.connectionsActive.value(), 2);

  destroyClientOnEvb(first);
  waitForActiveConnections(1);
  EXPECT_EQ(connectionTotals().connectionsAccepted.value(), 2);

  destroyClientOnEvb(second);
}

TEST_F(FastThriftServerStatsE2ETest, CountsRequestsAndResponses) {
  startServer(StatsMode::kSetters);
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
  startServer(StatsMode::kSetters);

  const auto total = totals();
  EXPECT_EQ(total.thriftInbound.value(), 0);
  EXPECT_EQ(total.rocketInbound.value(), 0);
}

// Leaving stats unset omits both metrics handlers; the server must be
// unaffected.
TEST_F(FastThriftServerStatsE2ETest, ServesNormallyWithoutStats) {
  startServer(StatsMode::kNone);
  auto client = createClient();

  EXPECT_EQ(syncCall([&] { return client->semifuture_add(1, 2); }), 3);
  EXPECT_EQ(server_->getStats(), nullptr);

  destroyClientOnEvb(client);
}

// The flag stands in for the whole setStats family, TLS included — an embedder
// that sets it should not then have to discover which layers it missed.
TEST_F(FastThriftServerStatsE2ETest, ConfigFlagMaterializesEveryLayer) {
  startServer(StatsMode::kConfigFlag);

  EXPECT_NE(server_->getStats(), nullptr);
  EXPECT_NE(server_->getConnectionStats(), nullptr);
  EXPECT_NE(server_->getTLSStats(), nullptr);
}

// Allocating the counters is only half the flag's job: it has to reach the
// handler wiring too, which nothing but real traffic proves.
TEST_F(FastThriftServerStatsE2ETest, ConfigFlagCountsRealTraffic) {
  startServer(StatsMode::kConfigFlag);
  auto client = createClient();

  EXPECT_EQ(syncCall([&] { return client->semifuture_add(10, 20); }), 30);

  EXPECT_EQ(totals().thriftInbound.value(), 1);
  EXPECT_EQ(totals().thriftOutbound.value(), 1);
  EXPECT_EQ(connectionTotals().connectionsAccepted.value(), 1);

  destroyClientOnEvb(client);
}

// The flag fills empty slots rather than claiming them, so an embedder that
// wants to hold its own instance — to publish it, say — still can.
TEST_F(FastThriftServerStatsE2ETest, ExplicitStatsSurviveTheConfigFlag) {
  THRIFT_FLAG_SET_MOCK(rocket_client_binary_rpc_metadata_encoding, true);
  handler_ = std::make_shared<TestHandler>();

  ftt::FastThriftServerConfig config;
  config.address = folly::SocketAddress("::1", 0);
  config.numIOThreads = 1;
  config.enableStats = true;

  server_ = std::make_unique<ftt::FastThriftServer>(std::move(config));
  server_->setInterface(handler_);
  auto mine = std::make_shared<ServerStats>();
  server_->setStats(mine);
  server_->start();

  EXPECT_EQ(server_->getStats(), mine);
  // The layers left empty are still filled.
  EXPECT_NE(server_->getConnectionStats(), nullptr);
}
