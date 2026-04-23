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

#include <folly/coro/Collect.h>
#include <folly/coro/GtestHelpers.h>
#include <folly/coro/Sleep.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/test/e2e/E2ETestFixture.h>
#include <thrift/lib/cpp2/test/e2e/gen-cpp2/TestBiDiService.h>

using namespace ::testing;

namespace apache::thrift {

namespace {

constexpr std::chrono::hours kEffectivelyWaitForever{24};

class BiDiServiceE2ETest : public test::E2ETestFixture {};

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

CO_TEST_F(BiDiServiceE2ETest, CustomSinkBufferSize) {
  constexpr int32_t kBufferSize = 5;
  // Send enough items to trigger credit replenishment (creditsUsed > bufferSize
  // / 2).
  constexpr int64_t kNumItems = 20;

  struct Handler : public ServiceHandler<detail::test::TestBiDiService> {
    folly::coro::Task<StreamTransformation<int64_t, int64_t>> co_intStream()
        override {
      co_return StreamTransformation<int64_t, int64_t>{
          .func = [](folly::coro::AsyncGenerator<int64_t&&> input)
              -> folly::coro::AsyncGenerator<int64_t&&> {
            while (auto item = co_await input.next()) {
              co_yield std::move(*item);
            }
          },
          .bufferSize = kBufferSize};
    }
  };

  testConfig({std::make_shared<Handler>()});

  auto client = makeClient<detail::test::TestBiDiService>();
  BidirectionalStream<int64_t, int64_t> stream =
      co_await client->co_intStream();

  auto sinkGen =
      folly::coro::co_invoke([]() -> folly::coro::AsyncGenerator<int64_t&&> {
        for (int64_t i = 0; i < kNumItems; ++i) {
          co_yield int64_t(i);
        }
      });

  auto sinkTask = folly::coro::co_invoke(
      [clientSink = std::move(stream.sink),
       sinkGen = std::move(sinkGen)]() mutable -> folly::coro::Task<void> {
        co_await std::move(clientSink).sink(std::move(sinkGen));
      });

  auto streamTask = folly::coro::co_invoke(
      [streamGen = std::move(stream.stream).toAsyncGenerator()]() mutable
          -> folly::coro::Task<void> {
        for (int64_t i = 0; i < kNumItems; ++i) {
          auto next = co_await streamGen.next();
          EXPECT_TRUE(next.has_value());
          EXPECT_EQ(*next, i);
        }
        EXPECT_FALSE(co_await streamGen.next());
      });

  co_await folly::coro::collectAll(std::move(sinkTask), std::move(streamTask));
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

CO_TEST_F(BiDiServiceE2ETest, MethodThrowsUndeclaredException) {
  struct Handler : public ServiceHandler<detail::test::TestBiDiService> {
    folly::coro::Task<StreamTransformation<int64_t, int64_t>> co_canThrow()
        override {
      throw std::runtime_error{"Method threw"};
    }
  };

  testConfig({std::make_shared<Handler>()});
  auto client = makeClient<detail::test::TestBiDiService>();
  EXPECT_THROW(
      co_await client->co_canThrow(), apache::thrift::TApplicationException);
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

CO_TEST_F(BiDiServiceE2ETest, StreamThrowsDeclaredException) {
  constexpr int64_t kStreamItemsUntilThrow = 500;

  struct Handler : public ServiceHandler<detail::test::TestBiDiService> {
    folly::coro::Task<StreamTransformation<int64_t, int64_t>> co_canThrow()
        override {
      co_return StreamTransformation<int64_t, int64_t>{
          [](folly::coro::AsyncGenerator<int64_t&&>)
              -> folly::coro::AsyncGenerator<int64_t&&> {
            for (std::size_t i = 0; i < kStreamItemsUntilThrow; ++i) {
              co_yield int64_t(i);
            }
            throw detail::test::BiDiStreamException{"For test"};
          }};
    }
  };

  testConfig({std::make_shared<Handler>()});
  auto client = makeClient<detail::test::TestBiDiService>();
  BidirectionalStream<int64_t, int64_t> stream = co_await client->co_canThrow();

  auto sinkGen =
      folly::coro::co_invoke([]() -> folly::coro::AsyncGenerator<int64_t&&> {
        for (int64_t i = 0; i < 1000; ++i) {
          co_yield int64_t(i);
        }
      });
  auto sinkTask = folly::coro::co_invoke(
      [clientSink = std::move(stream.sink),
       sinkGen = std::move(sinkGen)]() mutable -> folly::coro::Task<void> {
        co_await std::move(clientSink).sink(std::move(sinkGen));
      });

  auto streamTask = folly::coro::co_invoke(
      [&, streamGen = std::move(stream.stream).toAsyncGenerator()]() mutable
          -> folly::coro::Task<void> {
        for (int64_t i = 0; i < kStreamItemsUntilThrow; ++i) {
          auto next = co_await streamGen.next();
          if (!next) {
            CO_FAIL() << fmt::format(
                "Did not receive all stream elements, expected {} but got {}",
                kStreamItemsUntilThrow,
                i);
          }
        }
        EXPECT_THROW(
            co_await streamGen.next(), detail::test::BiDiStreamException);
      });
  co_await folly::coro::collectAll(std::move(sinkTask), std::move(streamTask));
}

CO_TEST_F(BiDiServiceE2ETest, StreamThrowsUndeclaredException) {
  constexpr int64_t kStreamItemsUntilThrow = 500;

  struct Handler : public ServiceHandler<detail::test::TestBiDiService> {
    folly::coro::Task<StreamTransformation<int64_t, int64_t>> co_canThrow()
        override {
      co_return StreamTransformation<int64_t, int64_t>{
          [](folly::coro::AsyncGenerator<int64_t&&>)
              -> folly::coro::AsyncGenerator<int64_t&&> {
            for (std::size_t i = 0; i < kStreamItemsUntilThrow; ++i) {
              co_yield int64_t(i);
            }
            throw std::runtime_error{"For test"};
          }};
    }
  };

  testConfig({std::make_shared<Handler>()});
  auto client = makeClient<detail::test::TestBiDiService>();
  BidirectionalStream<int64_t, int64_t> stream = co_await client->co_canThrow();

  auto sinkGen =
      folly::coro::co_invoke([]() -> folly::coro::AsyncGenerator<int64_t&&> {
        for (int64_t i = 0; i < 1000; ++i) {
          co_yield int64_t(i);
        }
      });
  auto sinkTask = folly::coro::co_invoke(
      [clientSink = std::move(stream.sink),
       sinkGen = std::move(sinkGen)]() mutable -> folly::coro::Task<void> {
        co_await std::move(clientSink).sink(std::move(sinkGen));
      });

  auto streamTask = folly::coro::co_invoke(
      [&, streamGen = std::move(stream.stream).toAsyncGenerator()]() mutable
          -> folly::coro::Task<void> {
        for (int64_t i = 0; i < kStreamItemsUntilThrow; ++i) {
          auto next = co_await streamGen.next();
          if (!next) {
            CO_FAIL() << fmt::format(
                "Did not receive all stream elements, expected {} but got {}",
                kStreamItemsUntilThrow,
                i);
          }
        }
        EXPECT_THROW(
            co_await streamGen.next(), apache::thrift::TApplicationException);
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

CO_TEST_F(BiDiServiceE2ETest, ConsumeInputNoOutput) {
  constexpr int64_t kTestLimit = 10000;

  struct Handler : public ServiceHandler<detail::test::TestBiDiService> {
    folly::coro::Task<StreamTransformation<int64_t, int64_t>> co_intStream()
        override {
      co_return StreamTransformation<int64_t, int64_t>{
          [this](folly::coro::AsyncGenerator<int64_t&&> input)
              -> folly::coro::AsyncGenerator<int64_t&&> {
            while (auto item = co_await input.next()) {
              ++counter;
            }
            co_return;
          }};
    }

    int64_t counter = 0;
  };

  auto handler = std::make_shared<Handler>();
  testConfig({handler});

  auto client = makeClient<detail::test::TestBiDiService>();
  BidirectionalStream<int64_t, int64_t> stream =
      co_await client->co_intStream();

  auto sinkGen =
      folly::coro::co_invoke([]() -> folly::coro::AsyncGenerator<int64_t&&> {
        for (int64_t i = 0; i < kTestLimit; ++i) {
          co_yield int64_t(i);
        }
      });

  auto streamTask = folly::coro::co_invoke(
      [&, streamGen = std::move(stream.stream).toAsyncGenerator()]() mutable
          -> folly::coro::Task<void> {
        while (auto item = co_await streamGen.next()) {
          CO_FAIL() << "Expected stream to be empty";
        }
      });

  co_await folly::coro::collectAll(
      stream.sink.sink(std::move(sinkGen)), std::move(streamTask));
  EXPECT_EQ(handler->counter, kTestLimit);
}

CO_TEST_F(BiDiServiceE2ETest, ClientDropsStreamWhileServerBlocksOnInput) {
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
  auto client = makeClient<detail::test::TestBiDiService>();
  auto bidi = co_await client->co_echo();

  // Sink: send one item, then hold open without sending more
  auto sinkGen = folly::coro::co_invoke(
      []() -> folly::coro::AsyncGenerator<std::string&&> {
        co_yield std::string("hello");
        co_await folly::coro::sleep(kEffectivelyWaitForever);
      });

  auto sinkTask = folly::coro::co_invoke(
      [clientSink = std::move(bidi.sink),
       sinkGen = std::move(sinkGen)]() mutable -> folly::coro::Task<void> {
        try {
          co_await std::move(clientSink).sink(std::move(sinkGen));
        } catch (...) {
          // Expected when server cancels the sink
        }
      });

  auto streamTask = folly::coro::co_invoke(
      [streamGen = std::move(bidi.stream).toAsyncGenerator()]() mutable
          -> folly::coro::Task<void> {
        // Read the echoed item
        auto first = co_await streamGen.next();
        EXPECT_TRUE(first.has_value());
        EXPECT_EQ(*first, "hello");
        // streamGen destroyed on return -> cancels the stream
        // Server is blocked on input.next() waiting for more sink data
        // With cancellation support, this unblocks the server
      });

  co_await folly::coro::collectAll(std::move(sinkTask), std::move(streamTask));
}

CO_TEST_F(BiDiServiceE2ETest, ServerStopsOutputWhileClientBlocksOnStream) {
  struct Handler : public ServiceHandler<detail::test::TestBiDiService> {
    folly::coro::Task<StreamTransformation<std::string, std::string>> co_echo()
        override {
      co_return StreamTransformation<std::string, std::string>{
          [](folly::coro::AsyncGenerator<std::string&&> input)
              -> folly::coro::AsyncGenerator<std::string&&> {
            // Read one item, echo it, then stop producing output
            if (auto item = co_await input.next()) {
              co_yield std::move(*item);
            }
            // Returning here destroys the input generator via RAII,
            // which should cancel the client's sink
          }};
    }
  };

  testConfig({std::make_shared<Handler>()});
  auto client = makeClient<detail::test::TestBiDiService>();
  auto bidi = co_await client->co_echo();

  // Sink: send one item, then hold open without sending more
  auto sinkGen = folly::coro::co_invoke(
      []() -> folly::coro::AsyncGenerator<std::string&&> {
        co_yield std::string("hello");
        co_await folly::coro::sleep(kEffectivelyWaitForever);
      });

  auto sinkTask = folly::coro::co_invoke(
      [clientSink = std::move(bidi.sink),
       sinkGen = std::move(sinkGen)]() mutable -> folly::coro::Task<void> {
        try {
          co_await std::move(clientSink).sink(std::move(sinkGen));
        } catch (...) {
          // Expected when server cancels the sink
        }
      });

  auto streamTask = folly::coro::co_invoke(
      [streamGen = std::move(bidi.stream).toAsyncGenerator()]() mutable
          -> folly::coro::Task<void> {
        // Read the echoed item
        auto first = co_await streamGen.next();
        EXPECT_TRUE(first.has_value());
        EXPECT_EQ(*first, "hello");
        // Server stopped producing, so the stream should complete
        auto end = co_await streamGen.next();
        EXPECT_FALSE(end.has_value());
      });

  co_await folly::coro::collectAll(std::move(sinkTask), std::move(streamTask));
}

CO_TEST_F(
    BiDiServiceE2ETest, ClientDropsStreamWhileServerProducesIndefinitely) {
  struct Handler : public ServiceHandler<detail::test::TestBiDiService> {
    folly::coro::Task<StreamTransformation<std::string, std::string>> co_echo()
        override {
      co_return StreamTransformation<std::string, std::string>{
          [](folly::coro::AsyncGenerator<std::string&&> /*input*/)
              -> folly::coro::AsyncGenerator<std::string&&> {
            // Ignore input, produce output forever until cancelled
            int i = 0;
            while (true) {
              co_yield std::to_string(i++);
              co_await folly::coro::co_safe_point;
            }
          }};
    }
  };

  testConfig({std::make_shared<Handler>()});
  auto client = makeClient<detail::test::TestBiDiService>();
  auto bidi = co_await client->co_echo();

  // Sink: hold open without sending anything
  auto sinkTask = folly::coro::co_invoke(
      [clientSink = std::move(bidi.sink)]() mutable -> folly::coro::Task<void> {
        try {
          auto emptyGen = folly::coro::co_invoke(
              []() -> folly::coro::AsyncGenerator<std::string&&> {
                co_await folly::coro::sleep(kEffectivelyWaitForever);
              });
          co_await std::move(clientSink).sink(std::move(emptyGen));
        } catch (...) {
          // Expected when server cancels the sink
        }
      });

  auto streamTask = folly::coro::co_invoke(
      [streamGen = std::move(bidi.stream).toAsyncGenerator()]() mutable
          -> folly::coro::Task<void> {
        // Read a few items to confirm the server is producing
        auto first = co_await streamGen.next();
        EXPECT_TRUE(first.has_value());
        EXPECT_EQ(*first, "0");
        auto second = co_await streamGen.next();
        EXPECT_TRUE(second.has_value());
        EXPECT_EQ(*second, "1");
        // streamGen destroyed on return -> cancels the stream
        // Server's co_safe_point should observe cancellation and stop
      });

  co_await folly::coro::collectAll(std::move(sinkTask), std::move(streamTask));
}

CO_TEST_F(BiDiServiceE2ETest, CreditStarvationTimeout) {
  // Server produces items indefinitely. Client reads a few items then stops
  // consuming. The server exhausts stream credits and the credit starvation
  // timeout fires, cancelling both stream and sink halves.
  struct Handler : public ServiceHandler<detail::test::TestBiDiService> {
    folly::coro::Task<StreamTransformation<std::string, std::string>> co_echo()
        override {
      co_return StreamTransformation<std::string, std::string>{
          [](folly::coro::AsyncGenerator<std::string&&> /*input*/)
              -> folly::coro::AsyncGenerator<std::string&&> {
            int i = 0;
            while (true) {
              co_yield std::to_string(i++);
              co_await folly::coro::co_safe_point;
            }
          }};
    }
  };

  testConfig({
      .handler = std::make_shared<Handler>(),
      .serverConfigCb =
          [](ThriftServer& server) {
            server.setStreamExpireTime(std::chrono::milliseconds(500));
          },
  });
  auto client = makeClient<detail::test::TestBiDiService>();
  auto bidi = co_await client->co_echo();

  // Hold sink open so the BiDi connection stays alive.
  auto sinkTask = folly::coro::co_invoke(
      [clientSink = std::move(bidi.sink)]() mutable -> folly::coro::Task<void> {
        try {
          co_await std::move(clientSink)
              .sink(
                  folly::coro::co_invoke(
                      []() -> folly::coro::AsyncGenerator<std::string&&> {
                        co_await folly::coro::sleep(kEffectivelyWaitForever);
                      }));
        } catch (...) {
          // Expected: server cancels sink on credit timeout.
        }
      });

  auto streamTask = folly::coro::co_invoke(
      [streamGen = std::move(bidi.stream).toAsyncGenerator()]() mutable
          -> folly::coro::Task<void> {
        // Read one item to verify the stream is working.
        auto first = co_await streamGen.next();
        EXPECT_TRUE(first.has_value());

        // Stop consuming so the server exhausts credits and the timeout fires.
        co_await folly::coro::sleep(std::chrono::milliseconds(2000));

        // Drain any buffered items, then expect the credit timeout error.
        bool gotTimeout = false;
        try {
          while (co_await streamGen.next()) {
          }
        } catch (const TApplicationException& ex) {
          EXPECT_EQ(ex.getType(), TApplicationException::TIMEOUT);
          gotTimeout = true;
        }
        EXPECT_TRUE(gotTimeout);
      });

  co_await folly::coro::collectAll(std::move(sinkTask), std::move(streamTask));
}

CO_TEST_F(BiDiServiceE2ETest, CreditStarvationTimeoutClientStopsConsuming) {
  // Server produces items indefinitely. Client stops consuming after one
  // item, so the server eventually exhausts credits. The credit starvation
  // timeout fires, tearing down both halves of the bidi interaction.
  struct Handler : public ServiceHandler<detail::test::TestBiDiService> {
    folly::coro::Task<StreamTransformation<int64_t, int64_t>> co_intStream()
        override {
      co_return StreamTransformation<int64_t, int64_t>{
          [](folly::coro::AsyncGenerator<int64_t&&> /*input*/)
              -> folly::coro::AsyncGenerator<int64_t&&> {
            for (int64_t i = 0;; ++i) {
              co_yield int64_t(i);
            }
          }};
    }
  };

  testConfig({
      .handler = std::make_shared<Handler>(),
      .serverConfigCb =
          [](ThriftServer& server) {
            server.setStreamExpireTime(std::chrono::milliseconds(500));
          },
  });
  auto client = makeClient<detail::test::TestBiDiService>();
  auto bidi = co_await client->co_intStream();

  auto sinkTask = folly::coro::co_invoke(
      [clientSink = std::move(bidi.sink)]() mutable -> folly::coro::Task<void> {
        try {
          co_await std::move(clientSink)
              .sink(
                  folly::coro::co_invoke(
                      []() -> folly::coro::AsyncGenerator<int64_t&&> {
                        co_await folly::coro::sleep(kEffectivelyWaitForever);
                      }));
        } catch (...) {
        }
      });

  auto streamTask = folly::coro::co_invoke(
      [streamGen = std::move(bidi.stream).toAsyncGenerator()]() mutable
          -> folly::coro::Task<void> {
        // Read one item, then stop consuming.
        auto first = co_await streamGen.next();
        EXPECT_TRUE(first.has_value());

        co_await folly::coro::sleep(std::chrono::milliseconds(2000));

        // The credit starvation timeout should fire.
        bool gotTimeout = false;
        try {
          while (co_await streamGen.next()) {
          }
        } catch (const TApplicationException& ex) {
          EXPECT_EQ(ex.getType(), TApplicationException::TIMEOUT);
          gotTimeout = true;
        }
        EXPECT_TRUE(gotTimeout);
      });

  co_await folly::coro::collectAll(std::move(sinkTask), std::move(streamTask));
}

CO_TEST_F(BiDiServiceE2ETest, SinkChunkTimeout) {
  // Server sets a chunk timeout on the sink direction. Client sends one item
  // then pauses longer than the timeout. The server-side timeout fires,
  // cancelling both halves of the bidi interaction.
  struct Handler : public ServiceHandler<detail::test::TestBiDiService> {
    folly::coro::Task<StreamTransformation<std::string, std::string>> co_echo()
        override {
      co_return StreamTransformation<std::string, std::string>{
          .func = [](folly::coro::AsyncGenerator<std::string&&> input)
              -> folly::coro::AsyncGenerator<std::string&&> {
            while (auto item = co_await input.next()) {
              co_yield std::move(*item);
            }
          },
          .chunkTimeout = std::chrono::milliseconds{200}};
    }
  };

  testConfig({std::make_shared<Handler>()});
  auto client = makeClient<detail::test::TestBiDiService>();
  auto bidi = co_await client->co_echo();

  // Sink: send one item, then pause longer than the chunk timeout.
  auto sinkTask = folly::coro::co_invoke(
      [clientSink = std::move(bidi.sink)]() mutable -> folly::coro::Task<void> {
        try {
          co_await std::move(clientSink)
              .sink(
                  folly::coro::co_invoke(
                      []() -> folly::coro::AsyncGenerator<std::string&&> {
                        co_yield std::string("hello");
                        // Pause longer than the 200ms chunk timeout.
                        co_await folly::coro::sleep(
                            std::chrono::milliseconds(2000));
                        co_yield std::string("should not arrive");
                      }));
        } catch (...) {
          // Expected: server cancels the sink on chunk timeout.
        }
      });

  auto streamTask = folly::coro::co_invoke(
      [streamGen = std::move(bidi.stream).toAsyncGenerator()]() mutable
          -> folly::coro::Task<void> {
        // Read the first echoed item.
        auto first = co_await streamGen.next();
        EXPECT_TRUE(first.has_value());
        EXPECT_EQ(*first, "hello");

        // The chunk timeout fires while we wait. Drain any buffered items,
        // then expect the timeout error.
        bool gotTimeout = false;
        try {
          while (co_await streamGen.next()) {
          }
        } catch (const TApplicationException& ex) {
          EXPECT_EQ(ex.getType(), TApplicationException::TIMEOUT);
          gotTimeout = true;
        }
        EXPECT_TRUE(gotTimeout);
      });

  co_await folly::coro::collectAll(std::move(sinkTask), std::move(streamTask));
}

CO_TEST_F(BiDiServiceE2ETest, NoSinkChunkTimeoutWhenClientSendsPromptly) {
  // Server sets a chunk timeout, but the client sends items fast enough that
  // the timeout never fires. The interaction completes normally.
  constexpr int64_t kNumItems = 20;

  struct Handler : public ServiceHandler<detail::test::TestBiDiService> {
    folly::coro::Task<StreamTransformation<int64_t, int64_t>> co_intStream()
        override {
      co_return StreamTransformation<int64_t, int64_t>{
          .func = [](folly::coro::AsyncGenerator<int64_t&&> input)
              -> folly::coro::AsyncGenerator<int64_t&&> {
            while (auto item = co_await input.next()) {
              co_yield std::move(*item);
            }
          },
          .chunkTimeout = std::chrono::milliseconds{500}};
    }
  };

  testConfig({std::make_shared<Handler>()});
  auto client = makeClient<detail::test::TestBiDiService>();
  auto bidi = co_await client->co_intStream();

  auto sinkTask = folly::coro::co_invoke(
      [clientSink = std::move(bidi.sink)]() mutable -> folly::coro::Task<void> {
        co_await std::move(clientSink)
            .sink(
                folly::coro::co_invoke(
                    []() -> folly::coro::AsyncGenerator<int64_t&&> {
                      for (int64_t i = 0; i < kNumItems; ++i) {
                        co_yield int64_t(i);
                      }
                    }));
      });

  auto streamTask = folly::coro::co_invoke(
      [streamGen = std::move(bidi.stream).toAsyncGenerator()]() mutable
          -> folly::coro::Task<void> {
        for (int64_t i = 0; i < kNumItems; ++i) {
          auto next = co_await streamGen.next();
          EXPECT_TRUE(next.has_value());
          EXPECT_EQ(*next, i);
        }
        // Stream should complete normally, no timeout.
        EXPECT_FALSE(co_await streamGen.next());
      });

  co_await folly::coro::collectAll(std::move(sinkTask), std::move(streamTask));
}

CO_TEST_F(BiDiServiceE2ETest, NoCreditStarvationWhenClientConsumes) {
  // Server produces a fixed number of items. Client consumes all of them
  // promptly. The credit starvation timeout should NOT fire because the
  // client keeps sending REQUEST_N credits.
  constexpr int64_t kStreamItems = 50;

  struct Handler : public ServiceHandler<detail::test::TestBiDiService> {
    folly::coro::Task<StreamTransformation<int64_t, int64_t>> co_intStream()
        override {
      co_return StreamTransformation<int64_t, int64_t>{
          [](folly::coro::AsyncGenerator<int64_t&&> /*input*/)
              -> folly::coro::AsyncGenerator<int64_t&&> {
            for (int64_t i = 0; i < kStreamItems; ++i) {
              co_yield int64_t(i);
            }
          }};
    }
  };

  testConfig({
      .handler = std::make_shared<Handler>(),
      .serverConfigCb =
          [](ThriftServer& server) {
            server.setStreamExpireTime(std::chrono::milliseconds(500));
          },
  });
  auto client = makeClient<detail::test::TestBiDiService>();
  auto bidi = co_await client->co_intStream();

  auto sinkTask = folly::coro::co_invoke(
      [clientSink = std::move(bidi.sink)]() mutable -> folly::coro::Task<void> {
        co_await std::move(clientSink)
            .sink(
                folly::coro::co_invoke(
                    []() -> folly::coro::AsyncGenerator<int64_t&&> {
                      co_return;
                    }));
      });

  auto streamTask = folly::coro::co_invoke(
      [streamGen = std::move(bidi.stream).toAsyncGenerator()]() mutable
          -> folly::coro::Task<void> {
        for (int64_t i = 0; i < kStreamItems; ++i) {
          auto next = co_await streamGen.next();
          EXPECT_TRUE(next.has_value());
          EXPECT_EQ(*next, i);
        }
        // Stream should complete normally, no timeout.
        EXPECT_FALSE(co_await streamGen.next());
      });

  co_await folly::coro::collectAll(std::move(sinkTask), std::move(streamTask));
}

CO_TEST_F(BiDiServiceE2ETest, CustomBufferReplenishThreshold) {
  constexpr int32_t kBufferSize = 10;
  constexpr int64_t kNumItems = 20;

  struct Handler : public ServiceHandler<detail::test::TestBiDiService> {
    folly::coro::Task<StreamTransformation<int64_t, int64_t>> co_intStream()
        override {
      co_return StreamTransformation<int64_t, int64_t>{
          .func = [](folly::coro::AsyncGenerator<int64_t&&> input)
              -> folly::coro::AsyncGenerator<int64_t&&> {
            while (auto item = co_await input.next()) {
              co_yield std::move(*item);
            }
          },
          .bufferSize = kBufferSize,
          .bufferReplenishThreshold = 3};
    }
  };

  testConfig({std::make_shared<Handler>()});

  auto client = makeClient<detail::test::TestBiDiService>();
  BidirectionalStream<int64_t, int64_t> stream =
      co_await client->co_intStream();

  auto sinkGen =
      folly::coro::co_invoke([&]() -> folly::coro::AsyncGenerator<int64_t&&> {
        for (int64_t i = 0; i < kNumItems; ++i) {
          co_yield int64_t(i);
        }
      });

  auto sinkTask = folly::coro::co_invoke(
      [clientSink = std::move(stream.sink),
       sinkGen = std::move(sinkGen)]() mutable -> folly::coro::Task<void> {
        co_await std::move(clientSink).sink(std::move(sinkGen));
      });

  auto streamTask = folly::coro::co_invoke(
      [streamGen = std::move(stream.stream).toAsyncGenerator()]() mutable
          -> folly::coro::Task<void> {
        for (int64_t i = 0; i < kNumItems; ++i) {
          auto next = co_await streamGen.next();
          EXPECT_TRUE(next.has_value());
          EXPECT_EQ(*next, i);
        }
        EXPECT_FALSE(co_await streamGen.next());
      });

  co_await folly::coro::collectAll(std::move(sinkTask), std::move(streamTask));
}

/**
 * Verify that RpcOptions::setBufferReplenishThreshold flows through to the
 * client-side stream in a BiDi method.
 */
CO_TEST_F(BiDiServiceE2ETest, ClientStreamCustomBufferReplenishThreshold) {
  constexpr int64_t kNumItems = 50;

  struct Handler : public ServiceHandler<detail::test::TestBiDiService> {
    folly::coro::Task<StreamTransformation<int64_t, int64_t>> co_intStream()
        override {
      co_return StreamTransformation<int64_t, int64_t>{
          .func = [](folly::coro::AsyncGenerator<int64_t&&> input)
              -> folly::coro::AsyncGenerator<int64_t&&> {
            while (auto item = co_await input.next()) {
              co_yield std::move(*item);
            }
          },
          .bufferSize = 100};
    }
  };

  testConfig({std::make_shared<Handler>()});

  auto client = makeClient<detail::test::TestBiDiService>();

  apache::thrift::RpcOptions rpcOptions;
  rpcOptions.setChunkBufferSize(10);
  rpcOptions.setBufferReplenishThreshold(3);

  BidirectionalStream<int64_t, int64_t> stream =
      co_await client->co_intStream(rpcOptions);

  auto sinkGen =
      folly::coro::co_invoke([&]() -> folly::coro::AsyncGenerator<int64_t&&> {
        for (int64_t i = 0; i < kNumItems; ++i) {
          co_yield int64_t(i);
        }
      });

  auto sinkTask = folly::coro::co_invoke(
      [clientSink = std::move(stream.sink),
       sinkGen = std::move(sinkGen)]() mutable -> folly::coro::Task<void> {
        co_await std::move(clientSink).sink(std::move(sinkGen));
      });

  auto streamTask = folly::coro::co_invoke(
      [streamGen = std::move(stream.stream).toAsyncGenerator()]() mutable
          -> folly::coro::Task<void> {
        for (int64_t i = 0; i < kNumItems; ++i) {
          auto next = co_await streamGen.next();
          EXPECT_TRUE(next.has_value());
          EXPECT_EQ(*next, i);
        }
        EXPECT_FALSE(co_await streamGen.next());
      });

  co_await folly::coro::collectAll(std::move(sinkTask), std::move(streamTask));
}

} // namespace apache::thrift
