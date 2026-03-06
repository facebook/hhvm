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

#include <atomic>
#include <folly/coro/GtestHelpers.h>
#include <folly/coro/Sleep.h>
#include <folly/synchronization/Baton.h>
#include <thrift/lib/cpp2/test/e2e/E2ETestFixture.h>
#include <thrift/lib/cpp2/test/e2e/gen-cpp2/TestOnewayService.h>

using namespace ::testing;

namespace apache::thrift {

namespace {

class OnewayServiceE2ETest : public test::E2ETestFixture {};

} // namespace

CO_TEST_F(OnewayServiceE2ETest, BasicFireAndForget) {
  folly::Baton<> received;

  struct Handler : public ServiceHandler<detail::test::TestOnewayService> {
    explicit Handler(folly::Baton<>& baton) : baton_(baton) {}

    folly::coro::Task<void> co_fireAndForget(
        std::unique_ptr<std::string> /*message*/) override {
      baton_.post();
      co_return;
    }

    folly::Baton<>& baton_;
  };

  testConfig({std::make_shared<Handler>(received)});
  auto client = makeClient<detail::test::TestOnewayService>();
  co_await client->co_fireAndForget("hello");

  // Wait for the handler to receive the message
  EXPECT_TRUE(received.try_wait_for(std::chrono::seconds(5)));
}

CO_TEST_F(OnewayServiceE2ETest, HandlerThrowsDoesNotPropagateToClient) {
  struct Handler : public ServiceHandler<detail::test::TestOnewayService> {
    folly::coro::Task<void> co_onewayCanThrow() override {
      throw std::runtime_error{"handler error"};
    }
  };

  testConfig({std::make_shared<Handler>()});
  auto client = makeClient<detail::test::TestOnewayService>();

  // Oneway calls should not propagate exceptions to the client
  co_await client->co_onewayCanThrow();
}

CO_TEST_F(OnewayServiceE2ETest, MultipleOnewayCalls) {
  struct Handler : public ServiceHandler<detail::test::TestOnewayService> {
    folly::coro::Task<void> co_fireAndForget(
        std::unique_ptr<std::string> /*message*/) override {
      counter.fetch_add(1, std::memory_order_relaxed);
      co_return;
    }

    std::atomic<int> counter{0};
  };

  auto handler = std::make_shared<Handler>();
  testConfig({handler});
  auto client = makeClient<detail::test::TestOnewayService>();

  constexpr int kNumCalls = 50;
  for (int i = 0; i < kNumCalls; ++i) {
    co_await client->co_fireAndForget(fmt::format("msg-{}", i));
  }

  // Wait for all messages to be processed
  for (int attempts = 0; attempts < 100; ++attempts) {
    if (handler->counter.load(std::memory_order_relaxed) == kNumCalls) {
      break;
    }
    co_await folly::coro::sleep(std::chrono::milliseconds(50));
  }
  EXPECT_EQ(handler->counter.load(std::memory_order_relaxed), kNumCalls);
}

} // namespace apache::thrift
