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
#include <thrift/lib/cpp2/test/e2e/gen-cpp2/TestStreamE2EService.h>

using namespace ::testing;

namespace apache::thrift {

namespace {

class StreamServiceE2ETest : public test::E2ETestFixture {};

} // namespace

CO_TEST_F(StreamServiceE2ETest, BasicStream) {
  struct Handler : public ServiceHandler<detail::test::TestStreamE2EService> {
    ServerStream<int32_t> range(int32_t from, int32_t to) override {
      for (int32_t i = from; i <= to; ++i) {
        co_yield int32_t(i);
      }
    }
  };

  testConfig({std::make_shared<Handler>()});
  auto client = makeClient<detail::test::TestStreamE2EService>();
  auto gen = (co_await client->co_range(0, 4)).toAsyncGenerator();

  for (int32_t expected = 0; expected <= 4; ++expected) {
    auto item = co_await gen.next();
    EXPECT_TRUE(item.has_value());
    EXPECT_EQ(*item, expected);
  }
  EXPECT_FALSE((co_await gen.next()).has_value());
}

CO_TEST_F(StreamServiceE2ETest, BasicStreamWithResponse) {
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

  testConfig({std::make_shared<Handler>()});
  auto client = makeClient<detail::test::TestStreamE2EService>();
  auto result = co_await client->co_rangeWithResponse(0, 4);
  EXPECT_EQ(result.response, 5);

  auto gen = std::move(result.stream).toAsyncGenerator();
  for (int32_t expected = 0; expected <= 4; ++expected) {
    auto item = co_await gen.next();
    EXPECT_TRUE(item.has_value());
    EXPECT_EQ(*item, expected);
  }
  EXPECT_FALSE((co_await gen.next()).has_value());
}

CO_TEST_F(StreamServiceE2ETest, EmptyStream) {
  struct Handler : public ServiceHandler<detail::test::TestStreamE2EService> {
    ServerStream<int32_t> range(int32_t /*from*/, int32_t /*to*/) override {
      co_return;
    }
  };

  testConfig({std::make_shared<Handler>()});
  auto client = makeClient<detail::test::TestStreamE2EService>();
  auto gen = (co_await client->co_range(0, 0)).toAsyncGenerator();
  EXPECT_FALSE((co_await gen.next()).has_value());
}

CO_TEST_F(StreamServiceE2ETest, StreamThrowsDeclaredException) {
  struct Handler : public ServiceHandler<detail::test::TestStreamE2EService> {
    ServerStream<int32_t> canThrow(int32_t from, int32_t to) override {
      for (int32_t i = from; i < to; ++i) {
        co_yield int32_t(i);
      }
      throw detail::test::StreamDeclaredException{"stream error"};
    }
  };

  testConfig({std::make_shared<Handler>()});
  auto client = makeClient<detail::test::TestStreamE2EService>();
  auto gen = (co_await client->co_canThrow(0, 3)).toAsyncGenerator();

  for (int32_t expected = 0; expected < 3; ++expected) {
    auto item = co_await gen.next();
    EXPECT_TRUE(item.has_value());
    EXPECT_EQ(*item, expected);
  }
  EXPECT_THROW(co_await gen.next(), detail::test::StreamDeclaredException);
}

CO_TEST_F(StreamServiceE2ETest, StreamThrowsUndeclaredException) {
  struct Handler : public ServiceHandler<detail::test::TestStreamE2EService> {
    ServerStream<int32_t> canThrow(int32_t from, int32_t to) override {
      for (int32_t i = from; i < to; ++i) {
        co_yield int32_t(i);
      }
      throw std::runtime_error{"undeclared stream error"};
    }
  };

  testConfig({std::make_shared<Handler>()});
  auto client = makeClient<detail::test::TestStreamE2EService>();
  auto gen = (co_await client->co_canThrow(0, 3)).toAsyncGenerator();

  for (int32_t expected = 0; expected < 3; ++expected) {
    auto item = co_await gen.next();
    EXPECT_TRUE(item.has_value());
    EXPECT_EQ(*item, expected);
  }
  EXPECT_THROW(co_await gen.next(), apache::thrift::TApplicationException);
}

CO_TEST_F(StreamServiceE2ETest, ClientCancelsStream) {
  struct Handler : public ServiceHandler<detail::test::TestStreamE2EService> {
    ServerStream<int32_t> range(int32_t from, int32_t /*to*/) override {
      int32_t i = from;
      while (true) {
        co_yield int32_t(i++);
        co_await folly::coro::co_safe_point;
      }
    }
  };

  testConfig({std::make_shared<Handler>()});
  auto client = makeClient<detail::test::TestStreamE2EService>();

  {
    auto gen = (co_await client->co_range(0, 0)).toAsyncGenerator();
    auto first = co_await gen.next();
    EXPECT_TRUE(first.has_value());
    EXPECT_EQ(*first, 0);
    // gen destroyed here — cancels the stream
  }

  // Verify the client connection is still healthy after cancellation
  auto gen2 = (co_await client->co_range(100, 100)).toAsyncGenerator();
  auto item = co_await gen2.next();
  EXPECT_TRUE(item.has_value());
  EXPECT_EQ(*item, 100);
}

} // namespace apache::thrift
