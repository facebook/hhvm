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
#include <folly/coro/GtestHelpers.h>
#include <thrift/lib/cpp2/async/tests/util/gen-cpp2/TestBiDiService.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace ::testing;

namespace apache::thrift {

namespace {

StreamTransformation<std::string, std::string>::Func makeTransformFunc() {
  return [](folly::coro::AsyncGenerator<std::string&&> input)
             -> folly::coro::AsyncGenerator<std::string&&> {
    while (auto item = co_await input.next()) {
      co_yield std::move(*item);
    }
  };
}

class TestBiDiServiceHandler
    : public ServiceHandler<detail::test::TestBiDiService> {
 public:
  folly::coro::Task<StreamTransformation<std::string, std::string>> co_echo()
      override {
    co_return StreamTransformation<std::string, std::string>{
        makeTransformFunc()};
  }

  folly::coro::Task<
      ResponseAndStreamTransformation<std::string, std::string, std::string>>
  co_echoWithResponse(std::unique_ptr<std::string> initial) override {
    co_return ResponseAndStreamTransformation<
        std::string,
        std::string,
        std::string>{std::move(*initial), {makeTransformFunc()}};
  }
};

class BiDiServiceTest : public Test {
 public:
  BiDiServiceTest()
      : server_{std::make_unique<ScopedServerInterfaceThread>(
            std::make_shared<TestBiDiServiceHandler>())} {}

  std::unique_ptr<Client<detail::test::TestBiDiService>> makeClient() {
    return server_->newClient<Client<detail::test::TestBiDiService>>(
        nullptr,
        [](folly::AsyncSocket::UniquePtr socket) -> RequestChannel::Ptr {
          return RocketClientChannel::newChannel(std::move(socket));
        });
  }

 private:
  std::unique_ptr<ScopedServerInterfaceThread> server_;
};

} // namespace

CO_TEST_F(BiDiServiceTest, BiDiNoResponse) {
  auto client = makeClient();
  BidirectionalStream<std::string, std::string> stream =
      co_await client->co_echo();
  auto sinkGen = folly::coro::co_invoke(
      []() -> folly::coro::AsyncGenerator<std::string&&> {
        co_yield "Hello";
        co_yield "World";
      });
  co_await stream.sink.sink(std::move(sinkGen));

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

CO_TEST_F(BiDiServiceTest, BiDiWithResponse) {
  auto client = makeClient();
  ResponseAndBidirectionalStream<std::string, std::string, std::string> result =
      co_await client->co_echoWithResponse("Test");

  EXPECT_EQ(result.response, "Test");

  auto sinkGen = folly::coro::co_invoke(
      []() -> folly::coro::AsyncGenerator<std::string&&> {
        co_yield "Hello";
        co_yield "World";
      });
  co_await result.sink.sink(std::move(sinkGen));

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

} // namespace apache::thrift
