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

#include <stdexcept>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <folly/experimental/coro/GtestHelpers.h>
#include <thrift/lib/cpp2/server/ServerModule.h>
#include <thrift/lib/cpp2/server/ServiceInterceptor.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/server/test/gen-cpp2/ServiceInterceptor_clients.h>
#include <thrift/lib/cpp2/server/test/gen-cpp2/ServiceInterceptor_handlers.h>
#include <thrift/lib/cpp2/test/util/TrackingTProcessorEventHandler.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace apache::thrift;
using apache::thrift::test::TrackingTProcessorEventHandler;
using namespace ::testing;

namespace {

struct TestHandler
    : apache::thrift::ServiceHandler<test::ServiceInterceptorTest> {
  folly::coro::Task<void> co_noop() override { co_return; }

  folly::coro::Task<std::unique_ptr<std::string>> co_echo(
      std::unique_ptr<std::string> str) override {
    co_return std::move(str);
  }

  void async_eb_echo_eb(
      apache::thrift::HandlerCallbackPtr<std::unique_ptr<std::string>> callback,
      std::unique_ptr<std::string> str) override {
    callback->result(*str);
  }

  folly::coro::Task<apache::thrift::TileAndResponse<SampleInteractionIf, void>>
  co_createInteraction() override {
    class SampleInteractionImpl : public SampleInteractionIf {
      folly::coro::Task<std::unique_ptr<std::string>> co_echo(
          std::unique_ptr<std::string> str) override {
        co_return std::move(str);
      }
    };
    co_return {std::make_unique<SampleInteractionImpl>()};
  }

  apache::thrift::ServerStream<std::int32_t> sync_iota(
      std::int32_t start) override {
    return folly::coro::co_invoke(
        [current =
             start]() mutable -> folly::coro::AsyncGenerator<std::int32_t&&> {
          while (true) {
            co_yield current++;
          }
        });
  }
};

class TestModule : public apache::thrift::ServerModule {
 public:
  explicit TestModule(std::shared_ptr<ServiceInterceptorBase> interceptor)
      : interceptor_(std::move(interceptor)) {}

  std::string getName() const override { return "TestModule"; }

  std::vector<std::shared_ptr<ServiceInterceptorBase>> getServiceInterceptors()
      override {
    return {interceptor_};
  }

 private:
  std::shared_ptr<ServiceInterceptorBase> interceptor_;
};

struct ServiceInterceptorCountWithRequestState
    : public ServiceInterceptor<int> {
 public:
  using RequestState = int;

  folly::coro::Task<std::optional<RequestState>> onRequest(
      RequestInfo) override {
    onRequestCount++;
    co_return 1;
  }

  folly::coro::Task<void> onResponse(
      RequestState* requestState, ResponseInfo) override {
    onResponseCount += *requestState;
    co_return;
  }

  int onRequestCount = 0;
  int onResponseCount = 0;
};

struct ServiceInterceptorThrowOnRequest
    : public ServiceInterceptor<folly::Unit> {
 public:
  folly::coro::Task<std::optional<folly::Unit>> onRequest(
      RequestInfo) override {
    onRequestCount++;
    throw std::runtime_error(
        "Exception from ServiceInterceptorThrowOnRequest::onRequest");
    co_return std::nullopt;
  }

  folly::coro::Task<void> onResponse(folly::Unit*, ResponseInfo) override {
    onResponseCount++;
    co_return;
  }

  int onRequestCount = 0;
  int onResponseCount = 0;
};

struct ServiceInterceptorThrowOnResponse
    : public ServiceInterceptor<folly::Unit> {
 public:
  folly::coro::Task<std::optional<folly::Unit>> onRequest(
      RequestInfo) override {
    onRequestCount++;
    co_return std::nullopt;
  }

  folly::coro::Task<void> onResponse(folly::Unit*, ResponseInfo) override {
    onResponseCount++;
    throw std::runtime_error(
        "Exception from ServiceInterceptorThrowOnResponse::onResponse");
    co_return;
  }

  int onRequestCount = 0;
  int onResponseCount = 0;
};

} // namespace

CO_TEST(ServiceInterceptorTest, BasicTM) {
  auto interceptor =
      std::make_shared<ServiceInterceptorCountWithRequestState>();
  ScopedServerInterfaceThread runner(
      std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(interceptor));
      });

  auto client =
      runner.newClient<apache::thrift::Client<test::ServiceInterceptorTest>>();
  co_await client->co_echo("");
  EXPECT_EQ(interceptor->onRequestCount, 1);
  EXPECT_EQ(interceptor->onResponseCount, 1);

  co_await client->co_echo("");
  EXPECT_EQ(interceptor->onRequestCount, 2);
  EXPECT_EQ(interceptor->onResponseCount, 2);
}

CO_TEST(ServiceInterceptorTest, BasicEB) {
  auto interceptor =
      std::make_shared<ServiceInterceptorCountWithRequestState>();
  ScopedServerInterfaceThread runner(
      std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(interceptor));
      });

  auto client =
      runner.newClient<apache::thrift::Client<test::ServiceInterceptorTest>>();
  co_await client->co_echo_eb("");
  EXPECT_EQ(interceptor->onRequestCount, 1);
  EXPECT_EQ(interceptor->onResponseCount, 1);

  co_await client->co_echo_eb("");
  EXPECT_EQ(interceptor->onRequestCount, 2);
  EXPECT_EQ(interceptor->onResponseCount, 2);
}

// void return calls HandlerCallback::done() instead of
// HandlerCallback::result()
CO_TEST(ServiceInterceptorTest, BasicVoidReturn) {
  auto interceptor =
      std::make_shared<ServiceInterceptorCountWithRequestState>();
  ScopedServerInterfaceThread runner(
      std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(interceptor));
      });

  auto client =
      runner.newClient<apache::thrift::Client<test::ServiceInterceptorTest>>();
  co_await client->co_noop();
  EXPECT_EQ(interceptor->onRequestCount, 1);
  EXPECT_EQ(interceptor->onResponseCount, 1);

  co_await client->co_noop();
  EXPECT_EQ(interceptor->onRequestCount, 2);
  EXPECT_EQ(interceptor->onResponseCount, 2);
}

CO_TEST(ServiceInterceptorTest, OnRequestException) {
  auto interceptor = std::make_shared<ServiceInterceptorThrowOnRequest>();
  ScopedServerInterfaceThread runner(
      std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(interceptor));
      });

  auto client =
      runner.newClient<apache::thrift::Client<test::ServiceInterceptorTest>>();
  EXPECT_THROW(
      {
        try {
          co_await client->co_echo("");
        } catch (const apache::thrift::TApplicationException& ex) {
          EXPECT_THAT(
              std::string(ex.what()),
              HasSubstr("ServiceInterceptor::onRequest threw exceptions"));
          EXPECT_THAT(
              std::string(ex.what()),
              HasSubstr(
                  "Exception from ServiceInterceptorThrowOnRequest::onRequest"));
          throw;
        }
      },
      apache::thrift::TApplicationException);
  EXPECT_EQ(interceptor->onRequestCount, 1);
  EXPECT_EQ(interceptor->onResponseCount, 0);
}

CO_TEST(ServiceInterceptorTest, OnRequestExceptionEB) {
  auto interceptor = std::make_shared<ServiceInterceptorThrowOnRequest>();
  ScopedServerInterfaceThread runner(
      std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(interceptor));
      });

  auto client =
      runner.newClient<apache::thrift::Client<test::ServiceInterceptorTest>>();
  EXPECT_THROW(
      {
        try {
          co_await client->co_echo_eb("");
        } catch (const apache::thrift::TApplicationException& ex) {
          EXPECT_THAT(
              std::string(ex.what()),
              HasSubstr("ServiceInterceptor::onRequest threw exceptions"));
          EXPECT_THAT(
              std::string(ex.what()),
              HasSubstr(
                  "Exception from ServiceInterceptorThrowOnRequest::onRequest"));
          throw;
        }
      },
      apache::thrift::TApplicationException);
  EXPECT_EQ(interceptor->onRequestCount, 1);
  EXPECT_EQ(interceptor->onResponseCount, 0);
}

CO_TEST(ServiceInterceptorTest, OnResponseException) {
  auto interceptor = std::make_shared<ServiceInterceptorThrowOnResponse>();
  ScopedServerInterfaceThread runner(
      std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(interceptor));
      });

  auto client =
      runner.newClient<apache::thrift::Client<test::ServiceInterceptorTest>>();
  EXPECT_THROW(
      {
        try {
          co_await client->co_echo("");
        } catch (const apache::thrift::TApplicationException& ex) {
          EXPECT_THAT(
              std::string(ex.what()),
              HasSubstr("ServiceInterceptor::onResponse threw exceptions"));
          EXPECT_THAT(
              std::string(ex.what()),
              HasSubstr(
                  "Exception from ServiceInterceptorThrowOnResponse::onResponse"));
          throw;
        }
      },
      apache::thrift::TApplicationException);
  EXPECT_EQ(interceptor->onRequestCount, 1);
  EXPECT_EQ(interceptor->onResponseCount, 1);
}

CO_TEST(ServiceInterceptorTest, OnResponseExceptionEB) {
  auto interceptor = std::make_shared<ServiceInterceptorThrowOnResponse>();
  ScopedServerInterfaceThread runner(
      std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(interceptor));
      });

  auto client =
      runner.newClient<apache::thrift::Client<test::ServiceInterceptorTest>>();
  EXPECT_THROW(
      {
        try {
          co_await client->co_echo_eb("");
        } catch (const apache::thrift::TApplicationException& ex) {
          EXPECT_THAT(
              std::string(ex.what()),
              HasSubstr("ServiceInterceptor::onResponse threw exceptions"));
          EXPECT_THAT(
              std::string(ex.what()),
              HasSubstr(
                  "Exception from ServiceInterceptorThrowOnResponse::onResponse"));
          throw;
        }
      },
      apache::thrift::TApplicationException);
  EXPECT_EQ(interceptor->onRequestCount, 1);
  EXPECT_EQ(interceptor->onResponseCount, 1);
}

CO_TEST(ServiceInterceptorTest, OnResponseBypassedForUnsafeReleasedCallback) {
  auto interceptor =
      std::make_shared<ServiceInterceptorCountWithRequestState>();

  struct TestHandlerUnsafeReleaseCallback
      : apache::thrift::ServiceHandler<test::ServiceInterceptorTest> {
    void async_tm_echo(
        apache::thrift::HandlerCallbackPtr<std::unique_ptr<std::string>>
            callback,
        std::unique_ptr<std::string> str) override {
      std::unique_ptr<
          apache::thrift::HandlerCallback<std::unique_ptr<std::string>>>
          releasedCallback{callback.unsafeRelease()};
      releasedCallback->result(*str);
    }
  };

  ScopedServerInterfaceThread runner(
      std::make_shared<TestHandlerUnsafeReleaseCallback>(),
      [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(interceptor));
      });

  auto client =
      runner.newClient<apache::thrift::Client<test::ServiceInterceptorTest>>();
  co_await client->co_echo("");
  EXPECT_EQ(interceptor->onRequestCount, 1);
  EXPECT_EQ(interceptor->onResponseCount, 0);

  co_await client->co_echo("");
  EXPECT_EQ(interceptor->onRequestCount, 2);
  EXPECT_EQ(interceptor->onResponseCount, 0);
}

CO_TEST(ServiceInterceptorTest, BasicInteraction) {
  auto interceptor =
      std::make_shared<ServiceInterceptorCountWithRequestState>();
  ScopedServerInterfaceThread runner(
      std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(interceptor));
      });

  auto client =
      runner.newClient<apache::thrift::Client<test::ServiceInterceptorTest>>();
  {
    auto interaction = co_await client->co_createInteraction();
    EXPECT_EQ(interceptor->onRequestCount, 1);
    EXPECT_EQ(interceptor->onResponseCount, 1);

    co_await interaction.co_echo("");
    EXPECT_EQ(interceptor->onRequestCount, 2);
    EXPECT_EQ(interceptor->onResponseCount, 2);

    co_await client->co_echo("");
    EXPECT_EQ(interceptor->onRequestCount, 3);
    EXPECT_EQ(interceptor->onResponseCount, 3);
  }

  EXPECT_EQ(interceptor->onRequestCount, 3);
  EXPECT_EQ(interceptor->onResponseCount, 3);
}

CO_TEST(ServiceInterceptorTest, BasicStream) {
  auto interceptor =
      std::make_shared<ServiceInterceptorCountWithRequestState>();
  ScopedServerInterfaceThread runner(
      std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(interceptor));
      });

  auto client =
      runner.newClient<apache::thrift::Client<test::ServiceInterceptorTest>>();
  {
    auto stream = (co_await client->co_iota(1)).toAsyncGenerator();
    EXPECT_EQ((co_await stream.next()).value(), 1);
    EXPECT_EQ((co_await stream.next()).value(), 2);
    EXPECT_EQ(interceptor->onRequestCount, 1);
    EXPECT_EQ(interceptor->onResponseCount, 1);
    // close stream
  }

  EXPECT_EQ(interceptor->onRequestCount, 1);
  EXPECT_EQ(interceptor->onResponseCount, 1);
}
