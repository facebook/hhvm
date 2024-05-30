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

#include <folly/portability/GTest.h>

#include <folly/experimental/coro/GtestHelpers.h>
#include <thrift/lib/cpp2/server/ServerModule.h>
#include <thrift/lib/cpp2/server/ServiceInterceptorBase.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/server/test/gen-cpp2/ServiceInterceptor_clients.h>
#include <thrift/lib/cpp2/server/test/gen-cpp2/ServiceInterceptor_handlers.h>
#include <thrift/lib/cpp2/test/util/TrackingTProcessorEventHandler.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace apache::thrift;
using apache::thrift::test::TrackingTProcessorEventHandler;

namespace {

struct TestHandler
    : apache::thrift::ServiceHandler<test::ServiceInterceptorTest> {
  folly::coro::Task<std::unique_ptr<std::string>> co_echo(
      std::unique_ptr<std::string> str) override {
    co_return std::move(str);
  }
};

} // namespace

CO_TEST(ServiceInterceptorTest, BasicOnRequest) {
  struct ServiceInterceptorImpl : public ServiceInterceptorBase {
   public:
    void internal_onConnection(ConnectionInfo) override {}

    folly::coro::Task<void> internal_onRequest(
        ConnectionInfo, RequestInfo) override {
      onRequestCount++;
      co_return;
    }

    folly::coro::Task<void> internal_onResponse(
        ConnectionInfo, ResponseInfo) override {
      onResponseCount++;
      co_return;
    }

    int onRequestCount = 0;
    int onResponseCount = 0;
  };
  class TestModule : public apache::thrift::ServerModule {
   public:
    explicit TestModule(std::shared_ptr<ServiceInterceptorBase> interceptor)
        : interceptor_(std::move(interceptor)) {}

    std::string getName() const override { return "TestModule"; }

    std::vector<std::shared_ptr<ServiceInterceptorBase>>
    getServiceInterceptors() override {
      return {interceptor_};
    }

   private:
    std::shared_ptr<ServiceInterceptorBase> interceptor_;
  };

  auto interceptor = std::make_shared<ServiceInterceptorImpl>();
  ScopedServerInterfaceThread runner(
      std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(interceptor));
      });

  auto client =
      runner.newClient<apache::thrift::Client<test::ServiceInterceptorTest>>();
  co_await client->co_echo("");
  EXPECT_EQ(interceptor->onRequestCount, 1);
  EXPECT_EQ(interceptor->onResponseCount, 1);
}
