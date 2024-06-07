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
#include <thrift/lib/cpp2/async/HTTPClientChannel.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/server/ServerModule.h>
#include <thrift/lib/cpp2/server/ServiceInterceptor.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/server/test/gen-cpp2/ServiceInterceptor_clients.h>
#include <thrift/lib/cpp2/server/test/gen-cpp2/ServiceInterceptor_handlers.h>
#include <thrift/lib/cpp2/test/util/TrackingTProcessorEventHandler.h>
#include <thrift/lib/cpp2/transport/http2/common/HTTP2RoutingHandler.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace apache::thrift;
using apache::thrift::test::TrackingTProcessorEventHandler;
using namespace ::testing;

namespace {

using TransportType = Cpp2ConnContext::TransportType;

std::unique_ptr<HTTP2RoutingHandler> createHTTP2RoutingHandler(
    ThriftServer& server) {
  auto h2Options = std::make_unique<proxygen::HTTPServerOptions>();
  h2Options->threads = static_cast<size_t>(server.getNumIOWorkerThreads());
  h2Options->idleTimeout = server.getIdleTimeout();
  h2Options->shutdownOn = {SIGINT, SIGTERM};
  return std::make_unique<HTTP2RoutingHandler>(
      std::move(h2Options), server.getThriftProcessor(), server);
}

class ServiceInterceptorTestP : public ::testing::TestWithParam<TransportType> {
 public:
  TransportType transportType() const { return GetParam(); }

  std::unique_ptr<ScopedServerInterfaceThread> makeServer(
      std::shared_ptr<AsyncProcessorFactory> service,
      ScopedServerInterfaceThread::ServerConfigCb configureServer = {}) {
    auto runner = std::make_unique<ScopedServerInterfaceThread>(
        std::move(service), std::move(configureServer));
    if (transportType() == TransportType::HTTP2) {
      auto& thriftServer = runner->getThriftServer();
      thriftServer.addRoutingHandler(createHTTP2RoutingHandler(thriftServer));
    }
    return runner;
  }

  ScopedServerInterfaceThread::MakeChannelFunc channelFor(
      TransportType transportType) {
    return [transportType](
               folly::AsyncSocket::UniquePtr socket) -> RequestChannel::Ptr {
      switch (transportType) {
        case TransportType::HEADER:
          return HeaderClientChannel::newChannel(std::move(socket));
        case TransportType::ROCKET:
          return RocketClientChannel::newChannel(std::move(socket));
        case TransportType::HTTP2: {
          auto channel = HTTPClientChannel::newHTTP2Channel(std::move(socket));
          channel->setProtocolId(protocol::T_COMPACT_PROTOCOL);
          return channel;
        }
        default:
          throw std::logic_error{"Unreachable!"};
      }
    };
  }

  template <typename ClientT>
  std::unique_ptr<ClientT> makeClient(ScopedServerInterfaceThread& runner) {
    return runner.newClient<ClientT>(nullptr, channelFor(transportType()));
  }
};

struct TestHandler
    : apache::thrift::ServiceHandler<test::ServiceInterceptorTest> {
  folly::coro::Task<void> co_noop() override { co_return; }

  folly::coro::Task<std::unique_ptr<std::string>> co_echo(
      std::unique_ptr<std::string> str) override {
    if (*str == "throw") {
      throw std::runtime_error("You asked for it!");
    }
    co_return std::move(str);
  }

  folly::coro::Task<std::unique_ptr<std::string>> co_requestArgs(
      std::int32_t,
      std::unique_ptr<std::string>,
      std::unique_ptr<test::RequestArgsStruct>) override {
    co_return std::make_unique<std::string>("return value");
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

using InterceptorList = std::vector<std::shared_ptr<ServiceInterceptorBase>>;

class TestModule : public apache::thrift::ServerModule {
 public:
  explicit TestModule(std::shared_ptr<ServiceInterceptorBase> interceptor) {
    interceptors_.emplace_back(std::move(interceptor));
  }

  explicit TestModule(InterceptorList interceptors)
      : interceptors_(std::move(interceptors)) {}

  std::string getName() const override { return "TestModule"; }

  InterceptorList getServiceInterceptors() override { return interceptors_; }

 private:
  InterceptorList interceptors_;
};

struct ServiceInterceptorCountWithRequestState
    : public ServiceInterceptor<int, int> {
 public:
  using ConnectionState = int;
  using RequestState = int;

  std::optional<ConnectionState> onConnection(
      ConnectionInfo) noexcept override {
    onConnectionCount++;
    return 1;
  }
  void onConnectionClosed(
      ConnectionState* connectionState, ConnectionInfo) noexcept override {
    onConnectionClosedCount += *connectionState;
  }

  folly::coro::Task<std::optional<RequestState>> onRequest(
      ConnectionState*, RequestInfo) override {
    onRequestCount++;
    co_return 1;
  }

  folly::coro::Task<void> onResponse(
      RequestState* requestState, ConnectionState*, ResponseInfo) override {
    onResponseCount += *requestState;
    co_return;
  }

  int onConnectionCount = 0;
  int onConnectionClosedCount = 0;
  int onRequestCount = 0;
  int onResponseCount = 0;
};

struct ServiceInterceptorThrowOnRequest
    : public ServiceInterceptor<folly::Unit> {
 public:
  folly::coro::Task<std::optional<folly::Unit>> onRequest(
      folly::Unit*, RequestInfo) override {
    onRequestCount++;
    throw std::runtime_error(
        "Exception from ServiceInterceptorThrowOnRequest::onRequest");
    co_return std::nullopt;
  }

  folly::coro::Task<void> onResponse(
      folly::Unit*, folly::Unit*, ResponseInfo) override {
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
      folly::Unit*, RequestInfo) override {
    onRequestCount++;
    co_return std::nullopt;
  }

  folly::coro::Task<void> onResponse(
      folly::Unit*, folly::Unit*, ResponseInfo) override {
    onResponseCount++;
    throw std::runtime_error(
        "Exception from ServiceInterceptorThrowOnResponse::onResponse");
    co_return;
  }

  int onRequestCount = 0;
  int onResponseCount = 0;
};

} // namespace

CO_TEST_P(ServiceInterceptorTestP, BasicTM) {
  auto interceptor =
      std::make_shared<ServiceInterceptorCountWithRequestState>();
  auto runner =
      makeServer(std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(interceptor));
      });

  auto client =
      makeClient<apache::thrift::Client<test::ServiceInterceptorTest>>(*runner);
  co_await client->co_echo("");
  EXPECT_EQ(interceptor->onRequestCount, 1);
  EXPECT_EQ(interceptor->onResponseCount, 1);

  co_await client->co_echo("");
  EXPECT_EQ(interceptor->onRequestCount, 2);
  EXPECT_EQ(interceptor->onResponseCount, 2);
}

CO_TEST_P(ServiceInterceptorTestP, BasicEB) {
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
CO_TEST_P(ServiceInterceptorTestP, BasicVoidReturn) {
  auto interceptor1 =
      std::make_shared<ServiceInterceptorCountWithRequestState>();
  auto interceptor2 =
      std::make_shared<ServiceInterceptorCountWithRequestState>();
  auto runner =
      makeServer(std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(
            InterceptorList{interceptor1, interceptor2}));
      });

  // HTTP2 does not support onConnection and onConnectionClosed because
  // ThriftProcessor creates & disposes the Cpp2ConnContext every request, not
  // connection.
  const auto valueIfNotHttp2 = [&](int value) -> int {
    return transportType() == TransportType::HTTP2 ? 0 : value;
  };

  {
    auto client = runner->newStickyClient<
        apache::thrift::Client<test::ServiceInterceptorTest>>(
        nullptr /* callbackExecutor */, channelFor(transportType()));
    co_await client->co_noop();
    for (auto& interceptor : {interceptor1, interceptor2}) {
      EXPECT_EQ(interceptor->onRequestCount, 1);
      EXPECT_EQ(interceptor->onResponseCount, 1);
      EXPECT_EQ(interceptor->onConnectionCount, valueIfNotHttp2(1));
      EXPECT_EQ(interceptor->onConnectionClosedCount, valueIfNotHttp2(0));
    }

    co_await client->co_noop();
    for (auto& interceptor : {interceptor1, interceptor2}) {
      EXPECT_EQ(interceptor->onRequestCount, 2);
      EXPECT_EQ(interceptor->onResponseCount, 2);
      EXPECT_EQ(interceptor->onConnectionCount, valueIfNotHttp2(1));
      EXPECT_EQ(interceptor->onConnectionClosedCount, valueIfNotHttp2(0));
    }
  }

  runner.reset();
  for (auto& interceptor : {interceptor1, interceptor2}) {
    EXPECT_EQ(interceptor->onConnectionCount, valueIfNotHttp2(1));
    EXPECT_EQ(interceptor->onConnectionClosedCount, valueIfNotHttp2(1));
  }
}

TEST_P(ServiceInterceptorTestP, OnStartServing) {
  struct ServiceInterceptorCountOnStartServing
      : public ServiceInterceptor<folly::Unit> {
   public:
    using RequestState = folly::Unit;

    folly::coro::Task<void> co_onStartServing(InitParams) override {
      onStartServingCount++;
      co_return;
    }

    folly::coro::Task<std::optional<RequestState>> onRequest(
        folly::Unit*, RequestInfo) override {
      co_return folly::unit;
    }

    folly::coro::Task<void> onResponse(
        RequestState*, folly::Unit*, ResponseInfo) override {
      co_return;
    }

    int onStartServingCount = 0;
  };
  auto interceptor1 = std::make_shared<ServiceInterceptorCountOnStartServing>();
  auto interceptor2 = std::make_shared<ServiceInterceptorCountOnStartServing>();
  ScopedServerInterfaceThread runner(
      std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(
            InterceptorList{interceptor1, interceptor2}));
      });

  for (auto& interceptor : {interceptor1, interceptor2}) {
    EXPECT_EQ(interceptor->onStartServingCount, 1);
  }
}

CO_TEST_P(ServiceInterceptorTestP, OnRequestException) {
  auto interceptor1 = std::make_shared<ServiceInterceptorThrowOnRequest>();
  auto interceptor2 =
      std::make_shared<ServiceInterceptorCountWithRequestState>();
  auto runner =
      makeServer(std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(
            InterceptorList{interceptor1, interceptor2}));
      });

  auto client =
      makeClient<apache::thrift::Client<test::ServiceInterceptorTest>>(*runner);
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
  EXPECT_EQ(interceptor1->onRequestCount, 1);
  EXPECT_EQ(interceptor1->onResponseCount, 1);
  EXPECT_EQ(interceptor2->onRequestCount, 1);
  EXPECT_EQ(interceptor2->onResponseCount, 1);
}

CO_TEST_P(ServiceInterceptorTestP, OnRequestExceptionEB) {
  auto interceptor = std::make_shared<ServiceInterceptorThrowOnRequest>();
  auto runner =
      makeServer(std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(interceptor));
      });

  auto client =
      makeClient<apache::thrift::Client<test::ServiceInterceptorTest>>(*runner);
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
  EXPECT_EQ(interceptor->onResponseCount, 1);
}

CO_TEST_P(ServiceInterceptorTestP, OnResponseException) {
  auto interceptor = std::make_shared<ServiceInterceptorThrowOnResponse>();
  auto runner =
      makeServer(std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(interceptor));
      });

  auto client =
      makeClient<apache::thrift::Client<test::ServiceInterceptorTest>>(*runner);
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
  auto interceptor1 = std::make_shared<ServiceInterceptorThrowOnResponse>();
  auto interceptor2 =
      std::make_shared<ServiceInterceptorCountWithRequestState>();
  ScopedServerInterfaceThread runner(
      std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(
            InterceptorList{interceptor1, interceptor2}));
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
  EXPECT_EQ(interceptor1->onRequestCount, 1);
  EXPECT_EQ(interceptor1->onResponseCount, 1);
  EXPECT_EQ(interceptor2->onRequestCount, 1);
  EXPECT_EQ(interceptor2->onResponseCount, 1);
}

CO_TEST_P(
    ServiceInterceptorTestP, OnResponseBypassedForUnsafeReleasedCallback) {
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

  auto runner = makeServer(
      std::make_shared<TestHandlerUnsafeReleaseCallback>(),
      [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(interceptor));
      });

  auto client =
      makeClient<apache::thrift::Client<test::ServiceInterceptorTest>>(*runner);
  co_await client->co_echo("");
  EXPECT_EQ(interceptor->onRequestCount, 1);
  EXPECT_EQ(interceptor->onResponseCount, 0);

  co_await client->co_echo("");
  EXPECT_EQ(interceptor->onRequestCount, 2);
  EXPECT_EQ(interceptor->onResponseCount, 0);
}

CO_TEST_P(
    ServiceInterceptorTestP, OnResponseExceptionPreservesApplicationException) {
  auto interceptor = std::make_shared<ServiceInterceptorThrowOnResponse>();
  auto runner =
      makeServer(std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(interceptor));
      });

  auto client =
      makeClient<apache::thrift::Client<test::ServiceInterceptorTest>>(*runner);
  EXPECT_THROW(
      {
        try {
          co_await client->co_echo("throw");
        } catch (const apache::thrift::TApplicationException& ex) {
          EXPECT_THAT(std::string(ex.what()), HasSubstr("You asked for it!"));
          throw;
        }
      },
      apache::thrift::TApplicationException);
  EXPECT_EQ(interceptor->onRequestCount, 1);
  EXPECT_EQ(interceptor->onResponseCount, 1);
}

CO_TEST_P(ServiceInterceptorTestP, BasicInteraction) {
  if (transportType() != TransportType::ROCKET) {
    // only rocket supports interactions
    co_return;
  }
  auto interceptor1 =
      std::make_shared<ServiceInterceptorCountWithRequestState>();
  auto interceptor2 =
      std::make_shared<ServiceInterceptorCountWithRequestState>();
  auto runner =
      makeServer(std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(
            InterceptorList{interceptor1, interceptor2}));
      });

  auto client =
      makeClient<apache::thrift::Client<test::ServiceInterceptorTest>>(*runner);
  {
    auto interaction = co_await client->co_createInteraction();
    for (auto& interceptor : {interceptor1, interceptor2}) {
      EXPECT_EQ(interceptor->onRequestCount, 1);
      EXPECT_EQ(interceptor->onResponseCount, 1);
    }

    co_await interaction.co_echo("");
    for (auto& interceptor : {interceptor1, interceptor2}) {
      EXPECT_EQ(interceptor->onRequestCount, 2);
      EXPECT_EQ(interceptor->onResponseCount, 2);
    }

    co_await client->co_echo("");
    for (auto& interceptor : {interceptor1, interceptor2}) {
      EXPECT_EQ(interceptor->onRequestCount, 3);
      EXPECT_EQ(interceptor->onResponseCount, 3);
    }
  }

  for (auto& interceptor : {interceptor1, interceptor2}) {
    EXPECT_EQ(interceptor->onRequestCount, 3);
    EXPECT_EQ(interceptor->onResponseCount, 3);
  }
}

CO_TEST_P(ServiceInterceptorTestP, BasicStream) {
  if (transportType() != TransportType::ROCKET) {
    // only rocket supports streaming
    co_return;
  }
  auto interceptor1 =
      std::make_shared<ServiceInterceptorCountWithRequestState>();
  auto interceptor2 =
      std::make_shared<ServiceInterceptorCountWithRequestState>();
  auto runner =
      makeServer(std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(
            InterceptorList{interceptor1, interceptor2}));
      });

  auto client =
      makeClient<apache::thrift::Client<test::ServiceInterceptorTest>>(*runner);
  {
    auto stream = (co_await client->co_iota(1)).toAsyncGenerator();
    EXPECT_EQ((co_await stream.next()).value(), 1);
    EXPECT_EQ((co_await stream.next()).value(), 2);
    for (auto& interceptor : {interceptor1, interceptor2}) {
      EXPECT_EQ(interceptor->onRequestCount, 1);
      EXPECT_EQ(interceptor->onResponseCount, 1);
    }
    // close stream
  }
  for (auto& interceptor : {interceptor1, interceptor2}) {
    EXPECT_EQ(interceptor->onRequestCount, 1);
    EXPECT_EQ(interceptor->onResponseCount, 1);
  }
}

CO_TEST_P(ServiceInterceptorTestP, RequestArguments) {
  struct ServiceInterceptorCountWithRequestState
      : public ServiceInterceptor<folly::Unit> {
   public:
    using ConnectionState = folly::Unit;
    using RequestState = folly::Unit;

    folly::coro::Task<std::optional<RequestState>> onRequest(
        ConnectionState*, RequestInfo requestInfo) override {
      argsCount = requestInfo.arguments.count();
      arg1 = requestInfo.arguments.get(0)->value<std::int32_t>();
      arg2 = requestInfo.arguments.get(1)->value<std::string>();
      arg3 = requestInfo.arguments.get(2)->value<test::RequestArgsStruct>();
      EXPECT_THROW(
          requestInfo.arguments.get(2)->value<std::int32_t>(), std::bad_cast);
      EXPECT_FALSE(requestInfo.arguments.get(3).has_value());
      co_return std::nullopt;
    }

    std::size_t argsCount = 0;
    std::int32_t arg1 = 0;
    std::string arg2;
    test::RequestArgsStruct arg3;
  };
  auto interceptor =
      std::make_shared<ServiceInterceptorCountWithRequestState>();
  auto runner =
      makeServer(std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(interceptor));
      });

  auto client =
      makeClient<apache::thrift::Client<test::ServiceInterceptorTest>>(*runner);
  test::RequestArgsStruct requestArgs;
  requestArgs.foo() = 1;
  requestArgs.bar() = "hello";
  auto result = co_await client->co_requestArgs(1, "hello", requestArgs);
  EXPECT_EQ(interceptor->argsCount, 3);
  EXPECT_EQ(interceptor->arg1, 1);
  EXPECT_EQ(interceptor->arg2, "hello");
  EXPECT_EQ(interceptor->arg3, requestArgs);
}

INSTANTIATE_TEST_SUITE_P(
    ServiceInterceptorTestP,
    ServiceInterceptorTestP,
    ::testing::Values(
        TransportType::HEADER, TransportType::ROCKET, TransportType::HTTP2));
