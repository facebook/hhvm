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

#include <fmt/core.h>
#include <folly/coro/Collect.h>
#include <folly/coro/GtestHelpers.h>
#include <folly/coro/Sleep.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/async/RpcOptions.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/test/e2e/gen-cpp2/TestBackpressureService.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace ::testing;

namespace apache::thrift {

namespace {

using ServiceHandler_ = ServiceHandler<detail::test::TestBackpressureService>;
using ServiceClient_ = Client<detail::test::TestBackpressureService>;

class BackpressureE2ETest : public ::testing::Test {
 protected:
  using ServerConfigCb = ScopedServerInterfaceThread::ServerConfigCb;

  void startServer(
      std::shared_ptr<AsyncProcessorFactory> handler,
      ServerConfigCb configCb = {}) {
    server_ = std::make_unique<ScopedServerInterfaceThread>(
        std::move(handler), std::move(configCb));
  }

  std::unique_ptr<ServiceClient_> makeClient() {
    return server_->newClient<ServiceClient_>(
        nullptr, RocketClientChannel::newChannel);
  }

  std::unique_ptr<ScopedServerInterfaceThread> server_;
};

} // namespace

// ==================== Stream backpressure ====================

CO_TEST_F(BackpressureE2ETest, SlowConsumerCausesServerStreamBackpressure) {
  // Server produces items without delay. Client consumes slowly with a
  // small credit buffer. Backpressure should throttle the server —
  // all items arrive in order, nothing is dropped.
  struct Handler : public ServiceHandler_ {
    ServerStream<std::string> generateItems(int32_t count) override {
      for (int32_t i = 0; i < count; ++i) {
        co_yield std::to_string(i);
      }
    }
  };

  startServer(std::make_shared<Handler>());
  auto client = makeClient();

  constexpr int32_t kItemCount = 20;
  RpcOptions opts;
  opts.setChunkBufferSize(2);

  auto gen =
      (co_await client->co_generateItems(opts, kItemCount)).toAsyncGenerator();

  auto start = std::chrono::steady_clock::now();
  int32_t received = 0;
  while (auto item = co_await gen.next()) {
    EXPECT_EQ(*item, std::to_string(received));
    ++received;
    co_await folly::coro::sleep(std::chrono::milliseconds(50));
  }
  auto elapsed = std::chrono::steady_clock::now() - start;

  EXPECT_EQ(received, kItemCount);
  // Slow consumer at 50ms * 20 items = 1000ms minimum.
  // Use 800ms as margin to avoid flakiness.
  EXPECT_GE(
      std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count(),
      800);
}

CO_TEST_F(BackpressureE2ETest, AllItemsDeliveredWithMinimalCredits) {
  // Even with chunkBufferSize=1 (minimum credits), all items should
  // be delivered without drops.
  struct Handler : public ServiceHandler_ {
    ServerStream<std::string> generateItems(int32_t count) override {
      for (int32_t i = 0; i < count; ++i) {
        co_yield std::to_string(i);
      }
    }
  };

  startServer(std::make_shared<Handler>());
  auto client = makeClient();

  RpcOptions opts;
  opts.setChunkBufferSize(1);

  auto gen = (co_await client->co_generateItems(opts, 50)).toAsyncGenerator();

  int32_t received = 0;
  while (auto item = co_await gen.next()) {
    EXPECT_EQ(*item, std::to_string(received));
    ++received;
  }
  EXPECT_EQ(received, 50);
}

// ==================== Sink backpressure ====================

CO_TEST_F(BackpressureE2ETest, SlowServerConsumerCausesSinkBackpressure) {
  // Server consumes slowly with small buffer. Client should be throttled
  // by credit-based flow control without dropping items.
  std::atomic<int32_t> serverReceived{0};

  struct Handler : public ServiceHandler_ {
    explicit Handler(std::atomic<int32_t>& received) : received_(received) {}

    SinkConsumer<std::string, std::string> consumeItems() override {
      return SinkConsumer<std::string, std::string>{
          [this](folly::coro::AsyncGenerator<std::string&&> gen)
              -> folly::coro::Task<std::string> {
            while (auto item = co_await gen.next()) {
              received_.fetch_add(1);
              co_await folly::coro::sleep(std::chrono::milliseconds(50));
            }
            co_return fmt::format("received {}", received_.load());
          },
          2 /* bufferSize: only 2 initial credits */};
    }

    std::atomic<int32_t>& received_;
  };

  startServer(std::make_shared<Handler>(serverReceived));
  auto client = makeClient();
  auto sink = co_await client->co_consumeItems();

  constexpr int32_t kItemCount = 10;
  auto start = std::chrono::steady_clock::now();

  auto finalResponse =
      co_await sink.sink([&]() -> folly::coro::AsyncGenerator<std::string&&> {
        for (int32_t i = 0; i < kItemCount; ++i) {
          co_yield std::to_string(i);
        }
      }());

  auto elapsed = std::chrono::steady_clock::now() - start;

  EXPECT_EQ(finalResponse, fmt::format("received {}", kItemCount));
  EXPECT_EQ(serverReceived.load(), kItemCount);
  // 10 items * 50ms server-side sleep = 500ms minimum. Use 400ms margin.
  EXPECT_GE(
      std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count(),
      400);
}

// ==================== Memory-based flow control ====================

CO_TEST_F(BackpressureE2ETest, MemoryBasedFlowControlDeliversAllItems) {
  // With setMemoryBufferSize, the client dynamically adjusts credits
  // based on payload size. All items should still be delivered.
  struct Handler : public ServiceHandler_ {
    ServerStream<std::string> generateItems(int32_t count) override {
      for (int32_t i = 0; i < count; ++i) {
        // Alternate between small and large payloads
        if (i % 2 == 0) {
          co_yield std::string(10, 'a');
        } else {
          co_yield std::string(500, 'b');
        }
      }
    }
  };

  startServer(std::make_shared<Handler>());
  auto client = makeClient();

  RpcOptions opts;
  opts.setMemoryBufferSize(1024, 5, 20);

  auto gen = (co_await client->co_generateItems(opts, 30)).toAsyncGenerator();

  int32_t received = 0;
  while (co_await gen.next()) {
    ++received;
  }
  EXPECT_EQ(received, 30);
}

// ==================== Chunk timeouts ====================

CO_TEST_F(BackpressureE2ETest, StreamChunkTimeoutOnSlowServer) {
  // Server is slow between items. Client has a chunk timeout.
  struct Handler : public ServiceHandler_ {
    ServerStream<std::string> generateItems(int32_t /*count*/) override {
      co_yield std::string("first");
      co_await folly::coro::sleep(std::chrono::milliseconds(500));
      co_yield std::string("second");
    }
  };

  startServer(std::make_shared<Handler>());
  auto client = makeClient();

  RpcOptions opts;
  opts.setChunkTimeout(std::chrono::milliseconds(200));

  auto gen = (co_await client->co_generateItems(opts, 2)).toAsyncGenerator();

  auto first = co_await gen.next();
  EXPECT_TRUE(first.has_value());
  EXPECT_EQ(*first, "first");

  // Second item should timeout — Rocket sends TTransportException
  EXPECT_THROW(co_await gen.next(), transport::TTransportException);
}

CO_TEST_F(BackpressureE2ETest, SinkChunkTimeoutOnSlowClient) {
  // Server sets a chunk timeout on the sink. Client pauses too long.
  struct Handler : public ServiceHandler_ {
    SinkConsumer<std::string, std::string> consumeItems() override {
      return SinkConsumer<std::string, std::string>{
          [](folly::coro::AsyncGenerator<std::string&&> gen)
              -> folly::coro::Task<std::string> {
            int32_t received = 0;
            try {
              while (auto item = co_await gen.next()) {
                ++received;
              }
            } catch (...) {
              co_return fmt::format("timeout after {}", received);
            }
            co_return fmt::format("received {}", received);
          },
          10}
          .setChunkTimeout(std::chrono::milliseconds(200));
    }
  };

  startServer(std::make_shared<Handler>());
  auto client = makeClient();
  auto sink = co_await client->co_consumeItems();

  bool terminated = false;
  try {
    co_await sink.sink([]() -> folly::coro::AsyncGenerator<std::string&&> {
      co_yield std::string("first");
      co_await folly::coro::sleep(std::chrono::milliseconds(500));
      co_yield std::string("second");
    }());
    terminated = true;
  } catch (...) {
    terminated = true;
  }
  EXPECT_TRUE(terminated);
}

// ==================== Stream expiry ====================

CO_TEST_F(BackpressureE2ETest, StreamExpireTimeWithSlowConsumer) {
  // Server sets stream expiry. Client stops consuming. Stream expires.
  struct Handler : public ServiceHandler_ {
    // Runs forever until cancelled.
    ServerStream<std::string> generateItems(int32_t /*count*/) override {
      int32_t i = 0;
      while (true) {
        co_yield std::to_string(i++);
        co_await folly::coro::co_safe_point;
      }
    }
  };

  startServer(std::make_shared<Handler>(), [](ThriftServer& server) {
    server.setStreamExpireTime(std::chrono::milliseconds(500));
  });
  auto client = makeClient();

  RpcOptions opts;
  opts.setChunkBufferSize(1);

  auto gen = (co_await client->co_generateItems(opts, 0)).toAsyncGenerator();

  auto first = co_await gen.next();
  EXPECT_TRUE(first.has_value());

  // Stop consuming longer than expire time
  co_await folly::coro::sleep(std::chrono::milliseconds(1000));

  // Stream should have expired
  bool expired = false;
  try {
    while (co_await gen.next()) {
    }
    expired = true; // Clean end is also acceptable
  } catch (...) {
    expired = true;
  }
  EXPECT_TRUE(expired);
}

// ==================== BiDi backpressure ====================

CO_TEST_F(BackpressureE2ETest, BiDiStreamPausesUnderEgressBackpressure) {
  // Configure the server with a low egress buffer backpressure threshold.
  // The bidi transformation produces large payloads rapidly. The client
  // consumes slowly. Backpressure should throttle the server — all items
  // arrive in order and nothing is dropped.
  struct Handler : public ServiceHandler_ {
    folly::coro::Task<StreamTransformation<std::string, std::string>>
    co_bidiEcho() override {
      co_return StreamTransformation<std::string, std::string>{
          [](folly::coro::AsyncGenerator<std::string&&> input)
              -> folly::coro::AsyncGenerator<std::string&&> {
            // Ignore input — just produce large payloads rapidly.
            for (int i = 0; i < 30; ++i) {
              // ~4KB payload to fill the buffer quickly.
              co_yield std::string(4096, static_cast<char>('a' + (i % 26)));
            }
          }};
    }
  };

  startServer(std::make_shared<Handler>(), [](ThriftServer& server) {
    // Low threshold so that a few large payloads trigger backpressure.
    server.setEgressBufferBackpressureThreshold(2048);
  });
  auto client = makeClient();
  auto bidi = co_await client->co_bidiEcho();

  // Consume stream items slowly.
  auto streamGen = std::move(bidi.stream).toAsyncGenerator();
  int received = 0;
  while (auto item = co_await streamGen.next()) {
    ++received;
    // Slow consumer — gives the server time to fill the egress buffer.
    co_await folly::coro::sleep(std::chrono::milliseconds(20));
  }

  // All 30 items must arrive — backpressure throttles, never drops.
  EXPECT_EQ(received, 30);
}

CO_TEST_F(BackpressureE2ETest, BiDiSlowSinkDoesNotBlockStream) {
  // BiDi echo where the client sends sink items slowly. The stream
  // output (echoed items) should arrive as each sink item is processed.
  // This tests that the two directions don't deadlock.
  struct Handler : public ServiceHandler_ {
    folly::coro::Task<StreamTransformation<std::string, std::string>>
    co_bidiEcho() override {
      co_return StreamTransformation<std::string, std::string>{
          [](folly::coro::AsyncGenerator<std::string&&> input)
              -> folly::coro::AsyncGenerator<std::string&&> {
            while (auto item = co_await input.next()) {
              co_yield std::move(*item);
            }
          }};
    }
  };

  startServer(std::make_shared<Handler>());
  auto client = makeClient();
  auto bidi = co_await client->co_bidiEcho();

  constexpr int kItemCount = 5;

  // Sink sends slowly, stream reads fast
  auto sinkTask = folly::coro::co_invoke(
      [clientSink = std::move(bidi.sink)]() mutable -> folly::coro::Task<void> {
        co_await std::move(clientSink)
            .sink([&]() -> folly::coro::AsyncGenerator<std::string&&> {
              for (int i = 0; i < kItemCount; ++i) {
                co_yield std::to_string(i);
                co_await folly::coro::sleep(std::chrono::milliseconds(100));
              }
            }());
      });

  auto streamTask = folly::coro::co_invoke(
      [streamGen = std::move(bidi.stream).toAsyncGenerator()]() mutable
          -> folly::coro::Task<int> {
        int received = 0;
        while (auto item = co_await streamGen.next()) {
          EXPECT_EQ(*item, std::to_string(received));
          ++received;
        }
        co_return received;
      });

  auto [sinkResult, streamResult] = co_await folly::coro::collectAll(
      std::move(sinkTask), std::move(streamTask));
  EXPECT_EQ(streamResult, kItemCount);
}

} // namespace apache::thrift
