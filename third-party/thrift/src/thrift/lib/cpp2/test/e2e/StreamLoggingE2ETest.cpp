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

#include <folly/coro/GtestHelpers.h>
#include <folly/coro/Sleep.h>
#include <thrift/lib/cpp2/logging/IThriftRequestLogging.h>
#include <thrift/lib/cpp2/logging/IThriftServerCounters.h>
#include <thrift/lib/cpp2/server/ThriftServerInternals.h>
#include <thrift/lib/cpp2/test/e2e/E2ETestFixture.h>
#include <thrift/lib/cpp2/test/e2e/gen-cpp2/TestStreamE2EService.h>

using namespace ::testing;

namespace apache::thrift {

namespace {

class RecordingCounters : public IThriftServerCounters {
 public:
  std::atomic<int> subscribes{0};
  std::atomic<int> nexts{0};
  std::atomic<int> completes{0};
  std::atomic<int> pauses{0};
  std::atomic<int> resumes{0};
  std::atomic<int> credits{0};
  std::atomic<detail::StreamEndReason> lastEndReason{};

  void onStreamSubscribe(std::string_view /*methodName*/) override {
    ++subscribes;
  }
  void onStreamNext(std::string_view /*methodName*/) override { ++nexts; }
  void onStreamCredit(
      std::string_view /*methodName*/,
      uint32_t /*c*/,
      uint32_t /*after*/) override {
    ++credits;
  }
  void onStreamPause(
      std::string_view /*methodName*/,
      detail::StreamPauseReason /*reason*/) override {
    ++pauses;
  }
  void onStreamResume(
      std::string_view /*methodName*/,
      detail::StreamPauseReason /*reason*/,
      std::chrono::milliseconds /*pauseDuration*/) override {
    ++resumes;
  }
  void onStreamComplete(
      std::string_view /*methodName*/,
      detail::StreamEndReason reason,
      uint32_t /*totalPauseEvents*/,
      std::chrono::milliseconds /*totalPauseDuration*/) override {
    ++completes;
    lastEndReason.store(reason);
  }
  void onStreamChunkGenerationInterval(
      std::string_view /*methodName*/,
      std::chrono::milliseconds /*interval*/) override {}
  void onStreamChunkSendDelay(
      std::string_view /*methodName*/,
      std::chrono::milliseconds /*delay*/) override {}
};

class RecordingLogging : public IThriftRequestLogging {
 public:
  std::atomic<int> streamCompletes{0};
  std::mutex mutex;
  StreamSummary lastSummary{};

  void onStreamComplete(const StreamSummary& summary) override {
    ++streamCompletes;
    std::lock_guard lock(mutex);
    lastSummary = summary;
  }
};

class StreamLoggingE2ETest : public test::E2ETestFixture {
 protected:
  std::shared_ptr<RecordingCounters> counters_ =
      std::make_shared<RecordingCounters>();
  std::shared_ptr<RecordingLogging> logging_ =
      std::make_shared<RecordingLogging>();

  void setupWithHandler(std::shared_ptr<AsyncProcessorFactory> handler) {
    testConfig(
        {.handler = std::move(handler),
         .serverConfigCb = [counters = counters_,
                            logging = logging_](ThriftServer& server) {
           detail::ThriftServerInternals internals(server);
           internals.setThriftServerCounters(counters);
           internals.setThriftRequestLogging(logging);
         }});
  }
};

} // namespace

CO_TEST_F(StreamLoggingE2ETest, BasicStreamLogsSubscribeNextAndComplete) {
  struct Handler : public ServiceHandler<detail::test::TestStreamE2EService> {
    ServerStream<int32_t> range(int32_t from, int32_t to) override {
      for (int32_t i = from; i <= to; ++i) {
        co_yield int32_t(i);
      }
    }
  };

  setupWithHandler(std::make_shared<Handler>());
  auto client = makeClient<detail::test::TestStreamE2EService>();
  auto gen = (co_await client->co_range(0, 4)).toAsyncGenerator();

  for (int32_t expected = 0; expected <= 4; ++expected) {
    auto item = co_await gen.next();
    EXPECT_TRUE(item.has_value());
    EXPECT_EQ(*item, expected);
  }
  EXPECT_FALSE((co_await gen.next()).has_value());

  // Allow async logging to settle
  /* sleep override */ co_await folly::coro::sleep(
      std::chrono::milliseconds(100));

  EXPECT_EQ(counters_->subscribes.load(), 1);
  EXPECT_GE(counters_->nexts.load(), 5);
  EXPECT_EQ(counters_->completes.load(), 1);
  EXPECT_EQ(counters_->lastEndReason.load(), detail::StreamEndReason::COMPLETE);
  EXPECT_EQ(logging_->streamCompletes.load(), 1);

  {
    std::lock_guard lock(logging_->mutex);
    EXPECT_GE(logging_->lastSummary.chunksGenerated, 5);
    EXPECT_EQ(
        logging_->lastSummary.endReason, detail::StreamEndReason::COMPLETE);
  }
}

CO_TEST_F(StreamLoggingE2ETest, EmptyStreamLogsSubscribeAndComplete) {
  struct Handler : public ServiceHandler<detail::test::TestStreamE2EService> {
    ServerStream<int32_t> range(int32_t /*from*/, int32_t /*to*/) override {
      co_return;
    }
  };

  setupWithHandler(std::make_shared<Handler>());
  auto client = makeClient<detail::test::TestStreamE2EService>();
  auto gen = (co_await client->co_range(0, 0)).toAsyncGenerator();
  EXPECT_FALSE((co_await gen.next()).has_value());

  /* sleep override */ co_await folly::coro::sleep(
      std::chrono::milliseconds(100));

  EXPECT_EQ(counters_->subscribes.load(), 1);
  EXPECT_EQ(counters_->completes.load(), 1);
  EXPECT_EQ(counters_->lastEndReason.load(), detail::StreamEndReason::COMPLETE);
  EXPECT_EQ(logging_->streamCompletes.load(), 1);

  {
    std::lock_guard lock(logging_->mutex);
    EXPECT_EQ(logging_->lastSummary.chunksGenerated, 0);
  }
}

CO_TEST_F(StreamLoggingE2ETest, StreamErrorLogsErrorReason) {
  struct Handler : public ServiceHandler<detail::test::TestStreamE2EService> {
    ServerStream<int32_t> canThrow(int32_t from, int32_t to) override {
      for (int32_t i = from; i < to; ++i) {
        co_yield int32_t(i);
      }
      throw detail::test::StreamDeclaredException{"stream error"};
    }
  };

  setupWithHandler(std::make_shared<Handler>());
  auto client = makeClient<detail::test::TestStreamE2EService>();
  auto gen = (co_await client->co_canThrow(0, 3)).toAsyncGenerator();

  for (int32_t expected = 0; expected < 3; ++expected) {
    auto item = co_await gen.next();
    EXPECT_TRUE(item.has_value());
    EXPECT_EQ(*item, expected);
  }
  EXPECT_THROW(co_await gen.next(), detail::test::StreamDeclaredException);

  /* sleep override */ co_await folly::coro::sleep(
      std::chrono::milliseconds(100));

  EXPECT_EQ(counters_->subscribes.load(), 1);
  EXPECT_EQ(counters_->completes.load(), 1);
  EXPECT_EQ(counters_->lastEndReason.load(), detail::StreamEndReason::ERROR);
  EXPECT_EQ(logging_->streamCompletes.load(), 1);

  {
    std::lock_guard lock(logging_->mutex);
    EXPECT_EQ(logging_->lastSummary.endReason, detail::StreamEndReason::ERROR);
    EXPECT_GE(logging_->lastSummary.chunksGenerated, 3);
  }
}

CO_TEST_F(StreamLoggingE2ETest, ClientCancelLogsCancel) {
  struct Handler : public ServiceHandler<detail::test::TestStreamE2EService> {
    ServerStream<int32_t> range(int32_t from, int32_t /*to*/) override {
      int32_t i = from;
      while (true) {
        co_yield int32_t(i++);
        co_await folly::coro::co_safe_point;
      }
    }
  };

  setupWithHandler(std::make_shared<Handler>());
  auto client = makeClient<detail::test::TestStreamE2EService>();

  {
    auto gen = (co_await client->co_range(0, 0)).toAsyncGenerator();
    auto first = co_await gen.next();
    EXPECT_TRUE(first.has_value());
    EXPECT_EQ(*first, 0);
    // gen destroyed here — cancels the stream
  }

  /* sleep override */ co_await folly::coro::sleep(
      std::chrono::milliseconds(100));

  EXPECT_EQ(counters_->subscribes.load(), 1);
  EXPECT_EQ(counters_->completes.load(), 1);
  EXPECT_EQ(counters_->lastEndReason.load(), detail::StreamEndReason::CANCEL);
}

CO_TEST_F(StreamLoggingE2ETest, MultipleStreamsLogIndependently) {
  struct Handler : public ServiceHandler<detail::test::TestStreamE2EService> {
    ServerStream<int32_t> range(int32_t from, int32_t to) override {
      for (int32_t i = from; i <= to; ++i) {
        co_yield int32_t(i);
      }
    }
  };

  setupWithHandler(std::make_shared<Handler>());
  auto client = makeClient<detail::test::TestStreamE2EService>();

  // First stream
  auto gen1 = (co_await client->co_range(0, 2)).toAsyncGenerator();
  while (co_await gen1.next()) {
  }

  // Second stream
  auto gen2 = (co_await client->co_range(0, 1)).toAsyncGenerator();
  while (co_await gen2.next()) {
  }

  /* sleep override */ co_await folly::coro::sleep(
      std::chrono::milliseconds(100));

  EXPECT_EQ(counters_->subscribes.load(), 2);
  EXPECT_EQ(counters_->completes.load(), 2);
  EXPECT_EQ(logging_->streamCompletes.load(), 2);
}

CO_TEST_F(StreamLoggingE2ETest, StreamWithResponseLogsCorrectly) {
  struct Handler : public ServiceHandler<detail::test::TestStreamE2EService> {
    ResponseAndServerStream<int32_t, int32_t> sync_rangeWithResponse(
        int32_t from, int32_t to) override {
      auto stream = folly::coro::co_invoke(
          [from, to]() -> folly::coro::AsyncGenerator<int32_t&&> {
            for (int32_t i = from; i <= to; ++i) {
              co_yield int32_t(i);
            }
          });
      return {to - from + 1, std::move(stream)};
    }
  };

  setupWithHandler(std::make_shared<Handler>());
  auto client = makeClient<detail::test::TestStreamE2EService>();
  auto result = co_await client->co_rangeWithResponse(0, 2);
  EXPECT_EQ(result.response, 3);

  auto gen = std::move(result.stream).toAsyncGenerator();
  while (co_await gen.next()) {
  }

  /* sleep override */ co_await folly::coro::sleep(
      std::chrono::milliseconds(100));

  EXPECT_EQ(counters_->subscribes.load(), 1);
  EXPECT_EQ(counters_->completes.load(), 1);
  EXPECT_EQ(counters_->lastEndReason.load(), detail::StreamEndReason::COMPLETE);
  EXPECT_EQ(logging_->streamCompletes.load(), 1);
}

CO_TEST_F(StreamLoggingE2ETest, PublisherStreamLogsSubscribeNextAndComplete) {
  struct Handler : public ServiceHandler<detail::test::TestStreamE2EService> {
    ServerStream<int32_t> range(int32_t from, int32_t to) override {
      auto [stream, publisher] = ServerStream<int32_t>::createPublisher([] {});
      for (int32_t i = from; i <= to; ++i) {
        publisher.next(i);
      }
      std::move(publisher).complete();
      return std::move(stream);
    }
  };

  setupWithHandler(std::make_shared<Handler>());
  auto client = makeClient<detail::test::TestStreamE2EService>();
  auto gen = (co_await client->co_range(0, 4)).toAsyncGenerator();

  for (int32_t expected = 0; expected <= 4; ++expected) {
    auto item = co_await gen.next();
    EXPECT_TRUE(item.has_value());
    EXPECT_EQ(*item, expected);
  }
  EXPECT_FALSE((co_await gen.next()).has_value());

  /* sleep override */ co_await folly::coro::sleep(
      std::chrono::milliseconds(100));

  EXPECT_EQ(counters_->subscribes.load(), 1);
  EXPECT_GE(counters_->nexts.load(), 5);
  EXPECT_EQ(counters_->completes.load(), 1);
  EXPECT_EQ(counters_->lastEndReason.load(), detail::StreamEndReason::COMPLETE);
  EXPECT_EQ(logging_->streamCompletes.load(), 1);

  {
    std::lock_guard lock(logging_->mutex);
    EXPECT_GE(logging_->lastSummary.chunksGenerated, 5);
    EXPECT_EQ(
        logging_->lastSummary.endReason, detail::StreamEndReason::COMPLETE);
    EXPECT_GT(logging_->lastSummary.totalBytes, 0);
  }
}

CO_TEST_F(StreamLoggingE2ETest, PublisherStreamCancelLogsCancel) {
  struct Handler : public ServiceHandler<detail::test::TestStreamE2EService> {
    folly::Synchronized<std::optional<ServerStreamPublisher<int32_t>>>
        publisher_;

    ServerStream<int32_t> range(int32_t /*from*/, int32_t /*to*/) override {
      auto [stream, publisher] = ServerStream<int32_t>::createPublisher([] {});
      publisher.next(0);
      publisher.next(1);
      publisher_.withWLock(
          [&](auto& pub) { pub.emplace(std::move(publisher)); });
      return std::move(stream);
    }
  };

  auto handler = std::make_shared<Handler>();
  setupWithHandler(handler);
  auto client = makeClient<detail::test::TestStreamE2EService>();

  {
    auto gen = (co_await client->co_range(0, 0)).toAsyncGenerator();
    auto first = co_await gen.next();
    EXPECT_TRUE(first.has_value());
    EXPECT_EQ(*first, 0);
    // gen destroyed here — cancels the stream
  }

  /* sleep override */ co_await folly::coro::sleep(
      std::chrono::milliseconds(100));

  // Clean up the publisher to avoid the CHECK failure in ~ServerStreamPublisher
  handler->publisher_.withWLock([](auto& pub) { pub.reset(); });

  EXPECT_EQ(counters_->subscribes.load(), 1);
  EXPECT_EQ(counters_->completes.load(), 1);
  EXPECT_EQ(counters_->lastEndReason.load(), detail::StreamEndReason::CANCEL);
}

} // namespace apache::thrift
