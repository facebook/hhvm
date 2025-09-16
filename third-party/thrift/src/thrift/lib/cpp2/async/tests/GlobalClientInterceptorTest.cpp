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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <folly/coro/GtestHelpers.h>

#include <folly/init/Init.h>

#include <thrift/lib/cpp2/async/ClientInterceptor.h>
#include <thrift/lib/cpp2/async/tests/gen-cpp2/ClientInterceptor_clients.h>
#include <thrift/lib/cpp2/async/tests/gen-cpp2/ClientInterceptor_handlers.h>
#include <thrift/lib/cpp2/runtime/Init.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

#include <utility>
#include <vector>

using namespace ::testing;

namespace {

class GlobalTracingClientInterceptor
    : public apache::thrift::ClientInterceptor<folly::Unit> {
 public:
  GlobalTracingClientInterceptor() {}

  std::string getName() const override {
    return "GlobalTracingClientInterceptor";
  }

  using Trace = std::pair<std::string, std::string>;
  const std::vector<Trace>& requests() const { return requests_; }

  std::optional<folly::Unit> onRequest(RequestInfo requestInfo) override {
    requests_.emplace_back(
        std::string(requestInfo.serviceName),
        std::string(requestInfo.methodName));
    return folly::unit;
  }

 private:
  std::vector<Trace> requests_;
};

std::shared_ptr<GlobalTracingClientInterceptor> globalTracingInterceptor =
    std::make_shared<GlobalTracingClientInterceptor>();

struct TestHandler : apache::thrift::ServiceHandler<
                         apache::thrift::test::ClientInterceptorTest> {
  folly::coro::Task<void> co_noop(bool) override { co_return; }
};

} // namespace

CO_TEST(GlobalClientInterceptorTest, GlobalClientInterceptor) {
  apache::thrift::ScopedServerInterfaceThread server(
      std::make_shared<TestHandler>());
  auto client = server.newClient<
      apache::thrift::Client<apache::thrift::test::ClientInterceptorTest>>();
  EXPECT_THAT(globalTracingInterceptor->requests(), IsEmpty());

  using Trace = GlobalTracingClientInterceptor::Trace;
  co_await client->co_noop(false);
  const std::vector<Trace> expectedTrace{
      Trace{"ClientInterceptorTest", "noop"},
  };
  EXPECT_THAT(
      globalTracingInterceptor->requests(), ElementsAreArray(expectedTrace));
}

int main(int argc, char** argv) {
  InitGoogleTest(&argc, argv);
  apache::thrift::runtime::InitOptions options;
  options.clientInterceptors.push_back(globalTracingInterceptor);
  apache::thrift::runtime::init(std::move(options));
  folly::Init init(&argc, &argv);
  return RUN_ALL_TESTS();
}
