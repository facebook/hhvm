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
#include <folly/coro/Collect.h>
#include <folly/coro/GtestHelpers.h>
#include <thrift/lib/cpp2/async/tests/util/gen-cpp2/TestBiDiService.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace ::testing;

namespace apache::thrift {

namespace {

class BiDiServiceE2ETest : public Test {
  using MakeChannelFunc = ScopedServerInterfaceThread::MakeChannelFunc;

 public:
  struct TestConfig {
    std::shared_ptr<AsyncProcessorFactory> handler;
    MakeChannelFunc channelFunc =
        [](folly::AsyncSocket::UniquePtr socket) -> RequestChannel::Ptr {
      return RocketClientChannel::newChannel(std::move(socket));
    };
  };

  void testConfig(TestConfig&& config) {
    server_ = std::make_unique<ScopedServerInterfaceThread>(
        std::move(config.handler));
    channelFunc_ = std::move(config.channelFunc);
  }

  template <typename ServiceTag>
  std::unique_ptr<Client<ServiceTag>> makeClient() {
    return server_->newClient<Client<ServiceTag>>(
        /* callbackExecutor */ nullptr,
        [&](folly::AsyncSocket::UniquePtr socket) -> RequestChannel::Ptr {
          return channelFunc_(std::move(socket));
        });
  }

 private:
  std::unique_ptr<ScopedServerInterfaceThread> server_;
  MakeChannelFunc channelFunc_;
};

} // namespace

CO_TEST_F(BiDiServiceE2ETest, BasicStreamTransformation) {
  struct Handler : public ServiceHandler<detail::test::TestBiDiService> {
    folly::coro::Task<StreamTransformation<std::string, std::string>> co_echo()
        override {
      co_return StreamTransformation<std::string, std::string>{
          [](folly::coro::AsyncGenerator<std::string&&> input)
              -> folly::coro::AsyncGenerator<std::string&&> {
            while (auto item = co_await input.next()) {
              co_yield std::move(*item);
            }
          }};
    }
  };

  testConfig({std::make_shared<Handler>()});

  // Sink (input)
  auto client = makeClient<detail::test::TestBiDiService>();
  BidirectionalStream<std::string, std::string> stream =
      co_await client->co_echo();
  auto sinkGen = folly::coro::co_invoke(
      []() -> folly::coro::AsyncGenerator<std::string&&> {
        co_yield "Hello";
        co_yield "World";
      });
  co_await stream.sink.sink(std::move(sinkGen));

  // Stream (output)
  auto streamGen = std::move(stream.stream).toAsyncGenerator();
  auto firstItem = co_await streamGen.next();
  EXPECT_TRUE(firstItem.has_value());
  EXPECT_EQ(*firstItem, "Hello");

  auto secondItem = co_await streamGen.next();
  EXPECT_TRUE(secondItem.has_value());
  EXPECT_EQ(*secondItem, "World");

  auto exhausted = co_await streamGen.next();
  EXPECT_FALSE(exhausted.has_value());
}

CO_TEST_F(BiDiServiceE2ETest, BasicResponseAndStreamTransformation) {
  struct Handler : public ServiceHandler<detail::test::TestBiDiService> {
    folly::coro::Task<
        ResponseAndStreamTransformation<std::string, std::string, std::string>>
    co_echoWithResponse(std::unique_ptr<std::string> initial) override {
      co_return ResponseAndStreamTransformation<
          std::string,
          std::string,
          std::string>{
          std::move(*initial),
          {[](folly::coro::AsyncGenerator<std::string&&> input)
               -> folly::coro::AsyncGenerator<std::string&&> {
            while (auto item = co_await input.next()) {
              co_yield std::move(*item);
            }
          }}};
    }
  };
  testConfig({std::make_shared<Handler>()});

  auto client = makeClient<detail::test::TestBiDiService>();
  ResponseAndBidirectionalStream<std::string, std::string, std::string> result =
      co_await client->co_echoWithResponse("Test");
  EXPECT_EQ(result.response, "Test");

  // Sink (input)
  auto sinkGen = folly::coro::co_invoke(
      []() -> folly::coro::AsyncGenerator<std::string&&> {
        co_yield "Hello";
        co_yield "World";
      });
  co_await result.sink.sink(std::move(sinkGen));

  // Stream (output)
  auto streamGen = std::move(result.stream).toAsyncGenerator();
  auto firstItem = co_await streamGen.next();
  EXPECT_TRUE(firstItem.has_value());
  EXPECT_EQ(*firstItem, "Hello");

  auto secondItem = co_await streamGen.next();
  EXPECT_TRUE(secondItem.has_value());
  EXPECT_EQ(*secondItem, "World");

  auto exhausted = co_await streamGen.next();
  EXPECT_FALSE(exhausted.has_value());
}

CO_TEST_F(BiDiServiceE2ETest, MethodThrowsDeclaredException) {
  struct Handler : public ServiceHandler<detail::test::TestBiDiService> {
    folly::coro::Task<StreamTransformation<int64_t, int64_t>> co_canThrow()
        override {
      throw detail::test::BiDiMethodException{"Method threw"};
    }
  };

  testConfig({std::make_shared<Handler>()});
  auto client = makeClient<detail::test::TestBiDiService>();
  EXPECT_THROW(
      co_await client->co_canThrow(), detail::test::BiDiMethodException);
}

CO_TEST_F(BiDiServiceE2ETest, SinkThrowsDeclaredException) {
  struct Handler : public ServiceHandler<detail::test::TestBiDiService> {
    folly::coro::Task<StreamTransformation<int64_t, int64_t>> co_canThrow()
        override {
      co_return StreamTransformation<int64_t, int64_t>{
          [](folly::coro::AsyncGenerator<int64_t&&> input)
              -> folly::coro::AsyncGenerator<int64_t&&> {
            try {
              for (auto&& item = co_await input.next(); item.has_value();
                   item = co_await input.next()) {
                co_yield std::move(*item);
              }
            } catch (const detail::test::BiDiSinkException&) {
              co_return;
            }
            CO_FAIL() << "Expected BiDiSinkException but was never received";
          }};
    }
  };

  testConfig({std::make_shared<Handler>()});
  auto client = makeClient<detail::test::TestBiDiService>();
  BidirectionalStream<int64_t, int64_t> stream = co_await client->co_canThrow();

  constexpr int64_t kTestLimit = 1000;
  auto sinkGen =
      folly::coro::co_invoke([]() -> folly::coro::AsyncGenerator<int64_t&&> {
        for (int64_t i = 0; i < kTestLimit; ++i) {
          co_yield int64_t(i);
        }
        throw detail::test::BiDiSinkException{"For test"};
      });
  auto sinkTask = folly::coro::co_invoke(
      [clientSink = std::move(stream.sink),
       sinkGen = std::move(sinkGen)]() mutable -> folly::coro::Task<void> {
        EXPECT_THROW(
            co_await std::move(clientSink).sink(std::move(sinkGen)),
            apache::thrift::SinkThrew);
      });

  auto streamTask = folly::coro::co_invoke(
      [&, streamGen = std::move(stream.stream).toAsyncGenerator()]() mutable
      -> folly::coro::Task<void> {
        for (int64_t i = 0; i < kTestLimit; ++i) {
          auto next = co_await streamGen.next();
          if (!next) {
            CO_FAIL() << fmt::format(
                "Did not receive all stream elements, expected {} but got {}",
                kTestLimit,
                i);
          }
        }
        EXPECT_FALSE(co_await streamGen.next());
      });
  co_await folly::coro::collectAll(std::move(sinkTask), std::move(streamTask));
}

CO_TEST_F(BiDiServiceE2ETest, SinkThrowsUndeclaredException) {
  struct Handler : public ServiceHandler<detail::test::TestBiDiService> {
    folly::coro::Task<StreamTransformation<int64_t, int64_t>> co_canThrow()
        override {
      co_return StreamTransformation<int64_t, int64_t>{
          [](folly::coro::AsyncGenerator<int64_t&&> input)
              -> folly::coro::AsyncGenerator<int64_t&&> {
            try {
              for (auto&& item = co_await input.next(); item.has_value();
                   item = co_await input.next()) {
                co_yield std::move(*item);
              }
            } catch (const apache::thrift::TApplicationException& e) {
              EXPECT_EQ(e.getMessage(), "std::runtime_error: For test");
              EXPECT_EQ(e.getType(), TApplicationException::UNKNOWN);
              co_return;
            }
            CO_FAIL()
                << "Expected TApplicationException but was never received";
          }};
    }
  };

  testConfig({std::make_shared<Handler>()});
  auto client = makeClient<detail::test::TestBiDiService>();
  BidirectionalStream<int64_t, int64_t> stream = co_await client->co_canThrow();

  constexpr int64_t kTestLimit = 1000;
  auto sinkGen =
      folly::coro::co_invoke([]() -> folly::coro::AsyncGenerator<int64_t&&> {
        for (int64_t i = 0; i < kTestLimit; ++i) {
          co_yield int64_t(i);
        }
        throw std::runtime_error{"For test"};
      });
  auto sinkTask = folly::coro::co_invoke(
      [clientSink = std::move(stream.sink),
       sinkGen = std::move(sinkGen)]() mutable -> folly::coro::Task<void> {
        EXPECT_THROW(
            co_await std::move(clientSink).sink(std::move(sinkGen)),
            apache::thrift::SinkThrew);
      });

  auto streamTask = folly::coro::co_invoke(
      [&, streamGen = std::move(stream.stream).toAsyncGenerator()]() mutable
      -> folly::coro::Task<void> {
        for (int64_t i = 0; i < kTestLimit; ++i) {
          auto next = co_await streamGen.next();
          if (!next) {
            CO_FAIL() << fmt::format(
                "Did not receive all stream elements, expected {} but got {}",
                kTestLimit,
                i);
          }
        }
        EXPECT_FALSE(co_await streamGen.next());
      });
  co_await folly::coro::collectAll(std::move(sinkTask), std::move(streamTask));
}

CO_TEST_F(BiDiServiceE2ETest, IgnoreInputProduceOutput) {
  constexpr int64_t kTestLimit = 100;

  struct Handler : public ServiceHandler<detail::test::TestBiDiService> {
    folly::coro::Task<StreamTransformation<int64_t, int64_t>> co_intStream()
        override {
      co_return StreamTransformation<int64_t, int64_t>{
          [this](folly::coro::AsyncGenerator<int64_t&&>)
              -> folly::coro::AsyncGenerator<int64_t&&> {
            for (int64_t i = 0; i < kTestLimit; ++i) {
              co_yield int64_t(i);
            }
          }};
    }
  };

  testConfig({std::make_shared<Handler>()});

  auto client = makeClient<detail::test::TestBiDiService>();
  BidirectionalStream<int64_t, int64_t> stream =
      co_await client->co_intStream();

  auto sinkGen =
      folly::coro::co_invoke([]() -> folly::coro::AsyncGenerator<int64_t&&> {
        int64_t counter = 0;
        while (true) {
          co_yield counter++;
        }
      });

  auto streamTask = folly::coro::co_invoke(
      [&, streamGen = std::move(stream.stream).toAsyncGenerator()]() mutable
      -> folly::coro::Task<void> {
        for (int64_t i = 0; i < kTestLimit; ++i) {
          auto next = co_await streamGen.next();
          if (!next) {
            CO_FAIL() << fmt::format(
                "Did not receive all stream elements, expected 100 but got {}",
                i);
          }
        }

        EXPECT_FALSE(co_await streamGen.next());
      });

  co_await folly::coro::collectAll(
      stream.sink.sink(std::move(sinkGen)), std::move(streamTask));
}

} // namespace apache::thrift
