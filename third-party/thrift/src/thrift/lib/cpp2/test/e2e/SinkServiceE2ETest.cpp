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
#include <thrift/lib/cpp2/test/e2e/E2ETestFixture.h>
#include <thrift/lib/cpp2/test/e2e/gen-cpp2/TestSinkE2EService.h>

using namespace ::testing;

namespace apache::thrift {

namespace {

class SinkServiceE2ETest : public test::E2ETestFixture {};

} // namespace

CO_TEST_F(SinkServiceE2ETest, BasicSinkWithFinalResponse) {
  struct Handler : public ServiceHandler<detail::test::TestSinkE2EService> {
    SinkConsumer<int32_t, std::string> echo(int32_t count) override {
      return SinkConsumer<int32_t, std::string>{
          [count](folly::coro::AsyncGenerator<int32_t&&> gen)
              -> folly::coro::Task<std::string> {
            int32_t received = 0;
            while (auto item = co_await gen.next()) {
              EXPECT_EQ(*item, received);
              ++received;
            }
            EXPECT_EQ(received, count);
            co_return fmt::format("received {}", received);
          },
          10 /* buffer size */};
    }
  };

  testConfig({std::make_shared<Handler>()});
  auto client = makeClient<detail::test::TestSinkE2EService>();
  auto sink = co_await client->co_echo(5);
  auto finalResponse =
      co_await sink.sink([]() -> folly::coro::AsyncGenerator<int32_t&&> {
        for (int32_t i = 0; i < 5; ++i) {
          co_yield int32_t(i);
        }
      }());
  EXPECT_EQ(finalResponse, "received 5");
}

CO_TEST_F(SinkServiceE2ETest, BasicSinkWithInitialResponse) {
  struct Handler : public ServiceHandler<detail::test::TestSinkE2EService> {
    ResponseAndSinkConsumer<std::string, int32_t, std::string> echoWithResponse(
        int32_t count) override {
      return {
          fmt::format("expecting {}", count),
          SinkConsumer<int32_t, std::string>{
              [count](folly::coro::AsyncGenerator<int32_t&&> gen)
                  -> folly::coro::Task<std::string> {
                int32_t received = 0;
                while (auto item = co_await gen.next()) {
                  ++received;
                }
                EXPECT_EQ(received, count);
                co_return fmt::format("received {}", received);
              },
              10 /* buffer size */}};
    }
  };

  testConfig({std::make_shared<Handler>()});
  auto client = makeClient<detail::test::TestSinkE2EService>();
  auto result = co_await client->co_echoWithResponse(3);
  EXPECT_EQ(result.response, "expecting 3");

  auto finalResponse =
      co_await result.sink.sink([]() -> folly::coro::AsyncGenerator<int32_t&&> {
        for (int32_t i = 0; i < 3; ++i) {
          co_yield int32_t(i);
        }
      }());
  EXPECT_EQ(finalResponse, "received 3");
}

CO_TEST_F(SinkServiceE2ETest, MethodThrowsDeclaredException) {
  struct Handler : public ServiceHandler<detail::test::TestSinkE2EService> {
    SinkConsumer<int32_t, std::string> canThrow() override {
      throw detail::test::SinkMethodException{"method error"};
    }
  };

  testConfig({std::make_shared<Handler>()});
  auto client = makeClient<detail::test::TestSinkE2EService>();
  EXPECT_THROW(
      co_await client->co_canThrow(), detail::test::SinkMethodException);
}

CO_TEST_F(SinkServiceE2ETest, MethodThrowsUndeclaredException) {
  struct Handler : public ServiceHandler<detail::test::TestSinkE2EService> {
    SinkConsumer<int32_t, std::string> canThrow() override {
      throw std::runtime_error{"undeclared method error"};
    }
  };

  testConfig({std::make_shared<Handler>()});
  auto client = makeClient<detail::test::TestSinkE2EService>();
  EXPECT_THROW(
      co_await client->co_canThrow(), apache::thrift::TApplicationException);
}

CO_TEST_F(SinkServiceE2ETest, SinkItemThrowsDeclaredException) {
  struct Handler : public ServiceHandler<detail::test::TestSinkE2EService> {
    SinkConsumer<int32_t, std::string> canThrow() override {
      return SinkConsumer<int32_t, std::string>{
          [](folly::coro::AsyncGenerator<int32_t&&> gen)
              -> folly::coro::Task<std::string> {
            try {
              while (auto item = co_await gen.next()) {
              }
            } catch (const detail::test::SinkItemException&) {
              co_return "caught SinkItemException";
            }
            co_return "no exception";
          },
          10 /* buffer size */};
    }
  };

  testConfig({std::make_shared<Handler>()});
  auto client = makeClient<detail::test::TestSinkE2EService>();
  auto sink = co_await client->co_canThrow();
  EXPECT_THROW(
      co_await sink.sink([]() -> folly::coro::AsyncGenerator<int32_t&&> {
        co_yield int32_t(1);
        throw detail::test::SinkItemException{"item error"};
      }()),
      apache::thrift::SinkThrew);
}

CO_TEST_F(SinkServiceE2ETest, FinalResponseThrowsDeclaredException) {
  struct Handler : public ServiceHandler<detail::test::TestSinkE2EService> {
    SinkConsumer<int32_t, std::string> canThrow() override {
      return SinkConsumer<int32_t, std::string>{
          [](folly::coro::AsyncGenerator<int32_t&&> gen)
              -> folly::coro::Task<std::string> {
            while (auto item = co_await gen.next()) {
            }
            throw detail::test::SinkFinalException{"final error"};
          },
          10 /* buffer size */};
    }
  };

  testConfig({std::make_shared<Handler>()});
  auto client = makeClient<detail::test::TestSinkE2EService>();
  auto sink = co_await client->co_canThrow();
  EXPECT_THROW(
      co_await sink.sink([]() -> folly::coro::AsyncGenerator<int32_t&&> {
        co_yield int32_t(1);
      }()),
      detail::test::SinkFinalException);
}

CO_TEST_F(SinkServiceE2ETest, EmptySink) {
  struct Handler : public ServiceHandler<detail::test::TestSinkE2EService> {
    SinkConsumer<int32_t, std::string> echo(int32_t /*count*/) override {
      return SinkConsumer<int32_t, std::string>{
          [](folly::coro::AsyncGenerator<int32_t&&> gen)
              -> folly::coro::Task<std::string> {
            int32_t received = 0;
            while (auto item = co_await gen.next()) {
              ++received;
            }
            EXPECT_EQ(received, 0);
            co_return "empty";
          },
          10 /* buffer size */};
    }
  };

  testConfig({std::make_shared<Handler>()});
  auto client = makeClient<detail::test::TestSinkE2EService>();
  auto sink = co_await client->co_echo(0);
  auto finalResponse = co_await sink.sink(
      []() -> folly::coro::AsyncGenerator<int32_t&&> { co_return; }());
  EXPECT_EQ(finalResponse, "empty");
}

} // namespace apache::thrift
