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

#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/synchronization/Baton.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/FastThriftServer.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/BackwardsCompatibilityTestService.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/BackwardsCompatibilityTestServiceAsyncClient.h>

THRIFT_FLAG_DECLARE_bool(rocket_client_binary_rpc_metadata_encoding);

namespace apache::thrift::fast_thrift::thrift::server::test {

using apache::thrift::fast_thrift::thrift::test::
    BackwardsCompatibilityTestService;

// Mock stats type for testing that FastThriftServer works with custom stats
struct MockStats {
  struct MockCounter {
    void incrementValue(int64_t delta) noexcept { value_ += delta; }
    int64_t value() const noexcept { return value_; }
    std::atomic<int64_t> value_{0};
  };

  MockCounter rocketInbound;
  MockCounter rocketOutbound;
  MockCounter rocketErrors;
  MockCounter rocketActive;
  MockCounter thriftInbound;
  MockCounter thriftOutbound;
  MockCounter thriftErrors;
  MockCounter thriftActive;
};

class TestHandler
    : public apache::thrift::ServiceHandler<BackwardsCompatibilityTestService> {
 public:
  void echo(
      std::string& response, std::unique_ptr<std::string> message) override {
    response = *message;
  }
  int64_t add(int64_t a, int64_t b) override { return a + b; }
  void ping() override {}
};

class FastThriftServerMetricsTest : public ::testing::Test {
 protected:
  void SetUp() override {
    THRIFT_FLAG_SET_MOCK(rocket_client_binary_rpc_metadata_encoding, true);

    handler_ = std::make_shared<TestHandler>();

    thrift::FastThriftServerConfig config;
    config.address = folly::SocketAddress("::1", 0);
    config.numIOThreads = 1;

    // Use FastThriftServerT with MockStats to test metrics functionality
    server_ = std::make_unique<thrift::FastThriftServerT<MockStats>>(
        std::move(config), handler_);
    server_->start();

    clientThread_ = std::make_unique<folly::ScopedEventBaseThread>();
  }

  void TearDown() override {
    clientThread_.reset();
    server_->stop();
    server_.reset();
  }

  std::unique_ptr<apache::thrift::Client<BackwardsCompatibilityTestService>>
  createClient() {
    auto* evb = clientThread_->getEventBase();
    std::unique_ptr<apache::thrift::Client<BackwardsCompatibilityTestService>>
        client;
    evb->runInEventBaseThreadAndWait([&] {
      auto socket = folly::AsyncSocket::newSocket(evb, server_->getAddress());
      auto channel =
          apache::thrift::RocketClientChannel::newChannel(std::move(socket));
      client = std::make_unique<
          apache::thrift::Client<BackwardsCompatibilityTestService>>(
          std::move(channel));
    });
    return client;
  }

  void destroyClientOnEvb(
      std::unique_ptr<
          apache::thrift::Client<BackwardsCompatibilityTestService>>& client) {
    clientThread_->getEventBase()->runInEventBaseThreadAndWait(
        [&] { client.reset(); });
  }

  template <typename Fn>
  auto syncCall(Fn&& fn) {
    using SemiFuture = std::invoke_result_t<Fn>;
    using T = typename SemiFuture::value_type;
    if constexpr (std::is_void_v<T>) {
      folly::Baton<> baton;
      auto* evb = clientThread_->getEventBase();
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
      folly::Baton<> baton;
      T result{};
      auto* evb = clientThread_->getEventBase();
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

  std::shared_ptr<TestHandler> handler_;
  std::unique_ptr<thrift::FastThriftServerT<MockStats>> server_;
  std::unique_ptr<folly::ScopedEventBaseThread> clientThread_;
};

TEST_F(FastThriftServerMetricsTest, ServerWorksWithCustomStats) {
  auto client = createClient();

  // Send a request-response RPC - this should work with custom stats
  auto result =
      syncCall([&] { return client->semifuture_echo("metrics test"); });
  EXPECT_EQ(result, "metrics test");

  // Send another request to verify multiple requests work
  auto result2 = syncCall([&] { return client->semifuture_add(10, 20); });
  EXPECT_EQ(result2, 30);

  destroyClientOnEvb(client);
}

TEST_F(FastThriftServerMetricsTest, ServerHandlesMultipleRequests) {
  auto client = createClient();

  // Send multiple requests
  for (int i = 0; i < 5; ++i) {
    auto result = syncCall([&] { return client->semifuture_ping(); });
    (void)result;
  }

  destroyClientOnEvb(client);
}

} // namespace apache::thrift::fast_thrift::thrift::server::test
