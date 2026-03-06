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
#include <thrift/lib/cpp2/test/e2e/gen-cpp2/TestRequestResponseService.h>

using namespace ::testing;

namespace apache::thrift {

namespace {

class RequestResponseServiceE2ETest : public test::E2ETestFixture {};

} // namespace

CO_TEST_F(RequestResponseServiceE2ETest, BasicEcho) {
  struct Handler
      : public ServiceHandler<detail::test::TestRequestResponseService> {
    folly::coro::Task<std::unique_ptr<std::string>> co_echo(
        std::unique_ptr<std::string> message) override {
      co_return std::move(message);
    }
  };

  testConfig({std::make_shared<Handler>()});
  auto client = makeClient<detail::test::TestRequestResponseService>();
  auto result = co_await client->co_echo("Hello");
  EXPECT_EQ(result, "Hello");
}

CO_TEST_F(RequestResponseServiceE2ETest, VoidReturn) {
  struct Handler
      : public ServiceHandler<detail::test::TestRequestResponseService> {
    folly::coro::Task<void> co_voidMethod() override { co_return; }
  };

  testConfig({std::make_shared<Handler>()});
  auto client = makeClient<detail::test::TestRequestResponseService>();
  co_await client->co_voidMethod();
}

CO_TEST_F(RequestResponseServiceE2ETest, MethodThrowsDeclaredException) {
  struct Handler
      : public ServiceHandler<detail::test::TestRequestResponseService> {
    folly::coro::Task<void> co_canThrow() override {
      throw detail::test::RRDeclaredException{"declared error"};
    }
  };

  testConfig({std::make_shared<Handler>()});
  auto client = makeClient<detail::test::TestRequestResponseService>();
  EXPECT_THROW(
      co_await client->co_canThrow(), detail::test::RRDeclaredException);
}

CO_TEST_F(RequestResponseServiceE2ETest, MethodThrowsUndeclaredException) {
  struct Handler
      : public ServiceHandler<detail::test::TestRequestResponseService> {
    folly::coro::Task<void> co_canThrow() override {
      throw std::runtime_error{"undeclared error"};
    }
  };

  testConfig({std::make_shared<Handler>()});
  auto client = makeClient<detail::test::TestRequestResponseService>();
  EXPECT_THROW(
      co_await client->co_canThrow(), apache::thrift::TApplicationException);
}

CO_TEST_F(RequestResponseServiceE2ETest, MultipleSequentialRequests) {
  struct Handler
      : public ServiceHandler<detail::test::TestRequestResponseService> {
    folly::coro::Task<std::unique_ptr<std::string>> co_echo(
        std::unique_ptr<std::string> message) override {
      co_return std::move(message);
    }
  };

  testConfig({std::make_shared<Handler>()});
  auto client = makeClient<detail::test::TestRequestResponseService>();

  constexpr int kNumRequests = 100;
  for (int i = 0; i < kNumRequests; ++i) {
    auto msg = fmt::format("request-{}", i);
    auto result = co_await client->co_echo(msg);
    EXPECT_EQ(result, msg);
  }
}

} // namespace apache::thrift
