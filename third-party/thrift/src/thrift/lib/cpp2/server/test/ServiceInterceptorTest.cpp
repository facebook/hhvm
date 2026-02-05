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

#include <functional>
#include <stdexcept>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <folly/coro/Baton.h>
#include <folly/coro/GtestHelpers.h>
#include <folly/coro/Sleep.h>
#include <thrift/lib/cpp2/async/HTTPClientChannel.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include <thrift/lib/cpp2/async/MultiplexAsyncProcessor.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/server/ServerFlags.h>
#include <thrift/lib/cpp2/server/ServerModule.h>
#include <thrift/lib/cpp2/server/ServiceInterceptor.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/server/ThriftServerInternals.h>
#include <thrift/lib/cpp2/server/test/gen-cpp2/ServiceInterceptor_clients.h>
#include <thrift/lib/cpp2/server/test/gen-cpp2/ServiceInterceptor_handlers.h>
#include <thrift/lib/cpp2/test/gen-cpp2/MultiplexAsyncProcessor2_clients.h>
#include <thrift/lib/cpp2/test/gen-cpp2/MultiplexAsyncProcessor2_handlers.h>
#include <thrift/lib/cpp2/test/gen-cpp2/MultiplexAsyncProcessor_clients.h>
#include <thrift/lib/cpp2/test/gen-cpp2/MultiplexAsyncProcessor_handlers.h>
#include <thrift/lib/cpp2/transport/http2/common/HTTP2RoutingHandler.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace apache::thrift;
using namespace apache::thrift::test;
using namespace apache::thrift::test2;
using namespace ::testing;

namespace apache::thrift::detail {

// This is a helper since the flag exists in the apache::thrift::detail
// namespace
void setDisabledServiceInterceptorsFlag(std::string flagValue) {
  THRIFT_FLAG_SET_MOCK(disabled_service_interceptors, std::move(flagValue));
}

} // namespace apache::thrift::detail

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
 private:
  gflags::FlagSaver flagSaver_;

 public:
  ServiceInterceptorTestP() { FLAGS_thrift_disable_resource_pools = true; }

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

  std::shared_ptr<AsyncProcessorFactory> multiplex(
      std::vector<std::shared_ptr<AsyncProcessorFactory>> services) {
    return std::make_shared<MultiplexAsyncProcessorFactory>(
        std::move(services));
  }

  ScopedServerInterfaceThread::MakeChannelFunc channelFor(
      TransportType transportType) {
    return [transportType](
               folly::AsyncSocket::UniquePtr socket) -> RequestChannel::Ptr {
      switch (transportType) {
        case TransportType::HEADER:
          return HeaderClientChannel::newChannel(
              HeaderClientChannel::WithoutRocketUpgrade{}, std::move(socket));
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

  folly::coro::Task<std::unique_ptr<test::ResponseArgsStruct>> co_echoStruct(
      std::unique_ptr<test::RequestArgsStruct> request) override {
    auto result = std::make_unique<test::ResponseArgsStruct>();
    result->foo() = std::move(*request->foo());
    result->bar() = std::move(*request->bar());
    co_return result;
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

  std::unique_ptr<SampleInteraction2If> createSampleInteraction2() override {
    class SampleInteraction2Impl : public SampleInteraction2If {
      folly::coro::Task<std::unique_ptr<std::string>> co_echo(
          std::unique_ptr<std::string> str) override {
        co_return std::move(str);
      }
    };
    return std::make_unique<SampleInteraction2Impl>();
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

  folly::coro::Task<void> co_fireAndForget(std::int32_t) override { co_return; }
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

template <class RequestState, class ConnectionState = folly::Unit>
struct NamedServiceInterceptor
    : public ServiceInterceptor<RequestState, ConnectionState> {
  explicit NamedServiceInterceptor(std::string name) : name_(std::move(name)) {}

  std::string getName() const override { return name_; }

 private:
  std::string name_;
};

struct ServiceInterceptorCountWithRequestState
    : public NamedServiceInterceptor<int, int> {
 public:
  using ConnectionState = int;
  using RequestState = int;

  using NamedServiceInterceptor::NamedServiceInterceptor;

  std::optional<ConnectionState> onConnectionEstablished(
      ConnectionInfo) override {
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
    : public NamedServiceInterceptor<folly::Unit> {
 public:
  using NamedServiceInterceptor::NamedServiceInterceptor;

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

// Covering the case where ServiceInterceptor should be able to catch exception
// thrown from another interceptor
struct ServiceInterceptorRequestExceptionCatch
    : public NamedServiceInterceptor<folly::Unit> {
 public:
  using NamedServiceInterceptor::NamedServiceInterceptor;

  explicit ServiceInterceptorRequestExceptionCatch(
      const std::string_view interceptorName,
      const bool shouldSelfThrow,
      const std::string_view expectedErrMsg)
      : NamedServiceInterceptor<folly::Unit>(std::string(interceptorName)),
        expectedErrMsg_(expectedErrMsg),
        shouldSelfThrow_(shouldSelfThrow) {}

  folly::coro::Task<std::optional<folly::Unit>> onRequest(
      folly::Unit*, RequestInfo) override {
    onRequestCount++;
    if (shouldSelfThrow_) {
      throw std::logic_error(
          "Exception from ServiceInterceptorRequestExceptionCatch");
    }
    co_return std::nullopt;
  }

  folly::coro::Task<void> onResponse(
      folly::Unit*, folly::Unit*, ResponseInfo responseInfo) override {
    onResponseCount++;
    if (std::holds_alternative<folly::exception_wrapper>(
            responseInfo.resultOrActiveException)) {
      const auto& exception = std::get<folly::exception_wrapper>(
          responseInfo.resultOrActiveException);
      EXPECT_TRUE(exception.get_exception() != nullptr);
      if (shouldSelfThrow_) {
        EXPECT_THAT(
            std::string(exception.get_exception()->what()),
            HasSubstr(
                "Exception from ServiceInterceptorRequestExceptionCatch"));
      } else {
        EXPECT_THAT(
            std::string(exception.get_exception()->what()),
            HasSubstr(expectedErrMsg_));
      }
    }
    co_return;
  }

  int onRequestCount = 0;
  int onResponseCount = 0;
  const std::string expectedErrMsg_;
  const bool shouldSelfThrow_;
};

struct ServiceInterceptorThrowOnResponse
    : public NamedServiceInterceptor<folly::Unit> {
 public:
  using NamedServiceInterceptor::NamedServiceInterceptor;

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

struct ServiceInterceptorRethrowActiveExceptionOnResponse
    : public NamedServiceInterceptor<folly::Unit> {
 public:
  using NamedServiceInterceptor::NamedServiceInterceptor;

  folly::coro::Task<std::optional<folly::Unit>> onRequest(
      folly::Unit*, RequestInfo) override {
    onRequestCount++;
    co_return std::nullopt;
  }

  folly::coro::Task<void> onResponse(
      folly::Unit*, folly::Unit*, ResponseInfo responseInfo) override {
    onResponseCount++;
    if (auto* ex = std::get_if<folly::exception_wrapper>(
            &responseInfo.resultOrActiveException)) {
      ex->throw_exception();
    }
    co_return;
  }

  int onRequestCount = 0;
  int onResponseCount = 0;
};

struct ServiceInterceptorThrowOnConnection
    : public NamedServiceInterceptor<folly::Unit> {
 public:
  using NamedServiceInterceptor::NamedServiceInterceptor;

  [[noreturn]] std::optional<folly::Unit> onConnectionEstablished(
      ConnectionInfo) override {
    onConnectionCount++;
    throw std::runtime_error(
        "Exception from ServiceInterceptorThrowOnConnection::onConnectionEstablished");
  }

  void onConnectionClosed(folly::Unit*, ConnectionInfo) noexcept override {
    onConnectionClosedCount++;
  }

  folly::coro::Task<std::optional<folly::Unit>> onRequest(
      folly::Unit*, RequestInfo) override {
    co_return std::nullopt;
  }

  folly::coro::Task<void> onResponse(
      folly::Unit*, folly::Unit*, ResponseInfo) override {
    co_return;
  }

  int onConnectionCount = 0;
  int onConnectionClosedCount = 0;
};

struct ServiceInterceptorLogResultTypeOnResponse
    : public NamedServiceInterceptor<folly::Unit> {
 public:
  using NamedServiceInterceptor::NamedServiceInterceptor;

  folly::coro::Task<void> onResponse(
      folly::Unit*, folly::Unit*, ResponseInfo responseInfo) override {
    results.emplace_back(
        folly::variant_match(
            responseInfo.resultOrActiveException,
            [](const folly::exception_wrapper& ex) -> Entry {
              return Entry{ResultKind::EXCEPTION, *ex.type()};
            },
            [](const apache::thrift::util::TypeErasedRef& result) -> Entry {
              return Entry{ResultKind::OK, result.type()};
            }));
    co_return;
  }

  enum class ResultKind {
    OK,
    EXCEPTION,
  };
  struct Entry {
    ResultKind kind;
    std::type_index type;

    bool operator==(const Entry& other) const {
      return std::tie(kind, type) == std::tie(other.kind, other.type);
    }
  };
  [[maybe_unused]] friend std::ostream& operator<<(
      std::ostream& os, const Entry& entry) {
    auto kindStr = entry.kind == ResultKind::OK ? "OK" : "EXCEPTION";
    return os << "Entry(kind=" << kindStr
              << ", type=" << folly::demangle(entry.type.name()) << ")";
  }

  std::vector<Entry> results;
};

class MockInterceptorMetricCallback : public InterceptorMetricCallback {
 public:
  MOCK_METHOD(
      void,
      onConnectionAttemptedComplete,
      (const ServiceInterceptorQualifiedName& qualifiedName,
       std::chrono::microseconds onResponseTime),
      (override));
  MOCK_METHOD(
      void,
      onConnectionComplete,
      (const ServiceInterceptorQualifiedName& qualifiedName,
       std::chrono::microseconds onResponseTime),
      (override));
  MOCK_METHOD(
      void,
      onConnectionClosedComplete,
      (const ServiceInterceptorQualifiedName& qualifiedName,
       std::chrono::microseconds onResponseTime),
      (override));
  MOCK_METHOD(
      void,
      onRequestComplete,
      (const ServiceInterceptorQualifiedName& qualifiedName,
       std::chrono::microseconds onResponseTime),
      (override));
  MOCK_METHOD(
      void,
      onResponseComplete,
      (const ServiceInterceptorQualifiedName& qualifiedName,
       std::chrono::microseconds onResponseTime),
      (override));
  MOCK_METHOD(
      void,
      onStreamBeginComplete,
      (const ServiceInterceptorQualifiedName& qualifiedName,
       std::chrono::microseconds duration),
      (override));
  MOCK_METHOD(
      void,
      onStreamPayloadComplete,
      (const ServiceInterceptorQualifiedName& qualifiedName,
       std::chrono::microseconds duration),
      (override));
  MOCK_METHOD(
      void,
      onStreamEndComplete,
      (const ServiceInterceptorQualifiedName& qualifiedName,
       std::chrono::microseconds duration),
      (override));
};

} // namespace

CO_TEST_P(ServiceInterceptorTestP, BasicTM) {
  auto interceptor =
      std::make_shared<ServiceInterceptorCountWithRequestState>("Interceptor1");
  auto interceptorMetricCallback =
      std::make_shared<MockInterceptorMetricCallback>();
  auto runner =
      makeServer(std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(interceptor));
        detail::ThriftServerInternals(server).setInterceptorMetricCallback(
            interceptorMetricCallback);
      });
  const auto callsIfNotHttp2 = [&](int value) -> int {
    return transportType() == TransportType::HTTP2 ? 0 : value;
  };
  EXPECT_CALL(*interceptorMetricCallback, onConnectionAttemptedComplete(_, _))
      .Times(callsIfNotHttp2(2))
      .WillRepeatedly(Return());
  EXPECT_CALL(*interceptorMetricCallback, onConnectionComplete(_, _))
      .Times(callsIfNotHttp2(2))
      .WillRepeatedly(Return());
  EXPECT_CALL(*interceptorMetricCallback, onConnectionClosedComplete(_, _))
      .Times(callsIfNotHttp2(2))
      .WillRepeatedly(Return());
  EXPECT_CALL(*interceptorMetricCallback, onRequestComplete(_, _))
      .Times(2)
      .WillRepeatedly(Return());
  EXPECT_CALL(*interceptorMetricCallback, onResponseComplete(_, _))
      .Times(2)
      .WillRepeatedly(Return());

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
      std::make_shared<ServiceInterceptorCountWithRequestState>("Interceptor1");
  auto runner =
      makeServer(std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(interceptor));
      });

  auto client =
      makeClient<apache::thrift::Client<test::ServiceInterceptorTest>>(*runner);
  co_await client->co_echo_eb("");
  EXPECT_EQ(interceptor->onRequestCount, 1);
  EXPECT_EQ(interceptor->onResponseCount, 1);

  co_await client->co_echo_eb("");
  EXPECT_EQ(interceptor->onRequestCount, 2);
  EXPECT_EQ(interceptor->onResponseCount, 2);
}

CO_TEST_P(ServiceInterceptorTestP, InterceptorControl) {
  struct BrokenInterceptor : public NamedServiceInterceptor<folly::Unit> {
    using NamedServiceInterceptor::NamedServiceInterceptor;

    [[noreturn]] std::optional<folly::Unit> onConnectionEstablished(
        ConnectionInfo) override {
      throw std::runtime_error("Broken");
    }

    // onConnectionClosed is noexcept

    [[noreturn]] folly::coro::Task<std::optional<folly::Unit>> onRequest(
        folly::Unit*, RequestInfo) override {
      throw std::runtime_error("Broken");
    }

    [[noreturn]] folly::coro::Task<void> onResponse(
        folly::Unit*, folly::Unit*, ResponseInfo) override {
      throw std::runtime_error("Broken");
    }
  };

  auto interceptor = std::make_shared<BrokenInterceptor>("BrokenInterceptor");
  auto runner =
      makeServer(std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(interceptor));
      });
  auto client =
      makeClient<apache::thrift::Client<test::ServiceInterceptorTest>>(*runner);

  // Setting the flag disables the interceptor, allowing the call to succeed
  apache::thrift::detail::setDisabledServiceInterceptorsFlag(
      "TestModule.BrokenInterceptor");
  co_await client->co_echo("Should not throw");
}

// void return calls HandlerCallback::done() instead of
// HandlerCallback::result()
CO_TEST_P(ServiceInterceptorTestP, BasicVoidReturn) {
  auto interceptor1 =
      std::make_shared<ServiceInterceptorCountWithRequestState>("Interceptor1");
  auto interceptor2 =
      std::make_shared<ServiceInterceptorCountWithRequestState>("Interceptor2");
  auto runner =
      makeServer(std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(
            std::make_unique<TestModule>(
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

CO_TEST_P(ServiceInterceptorTestP, BasicOneWay) {
  struct OneWayInterceptor : ServiceInterceptorCountWithRequestState {
    using Base = ServiceInterceptorCountWithRequestState;

    explicit OneWayInterceptor(folly::coro::Baton& fireAndForgetBaton)
        : Base("OneWayInterceptor"), fireAndForgetBaton_(fireAndForgetBaton) {}

    folly::coro::Task<void> onResponse(
        RequestState* requestState,
        ConnectionState* connectionState,
        ResponseInfo responseInfo) override {
      co_await Base::onResponse(
          requestState, connectionState, std::move(responseInfo));
      fireAndForgetBaton_.post();
    }

   private:
    folly::coro::Baton& fireAndForgetBaton_;
  };

  folly::coro::Baton fireAndForgetBaton;
  auto interceptor =
      std::make_shared<OneWayInterceptor>(std::ref(fireAndForgetBaton));
  auto runner =
      makeServer(std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(interceptor));
      });

  auto client =
      makeClient<apache::thrift::Client<test::ServiceInterceptorTest>>(*runner);
  co_await client->co_fireAndForget(5);
  co_await fireAndForgetBaton;

  EXPECT_EQ(interceptor->onRequestCount, 1);
  // oneway methods should invoke onResponse even though there is technically no
  // response sent back to the client
  EXPECT_EQ(interceptor->onResponseCount, 1);
}

CO_TEST_P(ServiceInterceptorTestP, NonTrivialRequestState) {
  struct Counts {
    int construct = 0;
    int destruct = 0;
  } counts;

  struct RequestState {
    explicit RequestState(Counts& counts) : counts_(&counts) {
      counts_->construct++;
    }
    RequestState(RequestState&& other) noexcept
        : counts_(std::exchange(other.counts_, nullptr)) {}
    RequestState& operator=(RequestState&& other) noexcept {
      counts_ = std::exchange(other.counts_, nullptr);
      return *this;
    }
    ~RequestState() {
      if (counts_) {
        counts_->destruct++;
      }
    }

   private:
    Counts* counts_;
  };

  struct ServiceInterceptorNonTrivialRequestState
      : public NamedServiceInterceptor<RequestState> {
   public:
    ServiceInterceptorNonTrivialRequestState(std::string name, Counts& counts)
        : NamedServiceInterceptor(std::move(name)), counts_(counts) {}

    folly::coro::Task<std::optional<RequestState>> onRequest(
        folly::Unit*, RequestInfo) override {
      co_return RequestState(counts_);
    }

    folly::coro::Task<void> onResponse(
        RequestState*, folly::Unit*, ResponseInfo) override {
      co_return;
    }

   private:
    Counts& counts_;
  };
  auto interceptor1 =
      std::make_shared<ServiceInterceptorNonTrivialRequestState>(
          "Interceptor1", counts);
  auto interceptor2 =
      std::make_shared<ServiceInterceptorNonTrivialRequestState>(
          "Interceptor2", counts);
  auto runner =
      makeServer(std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(
            std::make_unique<TestModule>(
                InterceptorList{interceptor1, interceptor2}));
      });

  auto client =
      makeClient<apache::thrift::Client<test::ServiceInterceptorTest>>(*runner);
  co_await client->co_noop();

  EXPECT_EQ(counts.construct, 2);
  EXPECT_EQ(counts.destruct, 2);
}

CO_TEST_P(ServiceInterceptorTestP, IterationOrder) {
  int seq = 0;

  class ServiceInterceptorRecordingExecutionSequence
      : public NamedServiceInterceptor<folly::Unit> {
   public:
    using RequestState = folly::Unit;

    explicit ServiceInterceptorRecordingExecutionSequence(
        std::string name, int& seq)
        : NamedServiceInterceptor(std::move(name)), seq_(seq) {}

    folly::coro::Task<std::optional<RequestState>> onRequest(
        folly::Unit*, RequestInfo) override {
      onRequestSeq = ++seq_;
      co_return std::nullopt;
    }

    folly::coro::Task<void> onResponse(
        RequestState*, folly::Unit*, ResponseInfo) override {
      onResponseSeq = ++seq_;
      co_return;
    }

    int onRequestSeq = 0;
    int onResponseSeq = 0;

   private:
    int& seq_;
  };

  auto interceptor1 =
      std::make_shared<ServiceInterceptorRecordingExecutionSequence>(
          "Interceptor1", seq);
  auto interceptor2 =
      std::make_shared<ServiceInterceptorRecordingExecutionSequence>(
          "Interceptor2", seq);
  auto runner =
      makeServer(std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(
            std::make_unique<TestModule>(
                InterceptorList{interceptor1, interceptor2}));
      });

  auto client =
      makeClient<apache::thrift::Client<test::ServiceInterceptorTest>>(*runner);
  co_await client->co_noop();

  EXPECT_EQ(interceptor1->onRequestSeq, 1);
  EXPECT_EQ(interceptor2->onRequestSeq, 2);
  EXPECT_EQ(interceptor2->onResponseSeq, 3);
  EXPECT_EQ(interceptor1->onResponseSeq, 4);
}

TEST_P(ServiceInterceptorTestP, OnStartServing) {
  struct ServiceInterceptorCountOnStartServing
      : public NamedServiceInterceptor<folly::Unit> {
   public:
    using RequestState = folly::Unit;

    using NamedServiceInterceptor::NamedServiceInterceptor;

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
  auto interceptor1 =
      std::make_shared<ServiceInterceptorCountOnStartServing>("Interceptor1");
  auto interceptor2 =
      std::make_shared<ServiceInterceptorCountOnStartServing>("Interceptor2");
  auto runner =
      makeServer(std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(
            std::make_unique<TestModule>(
                InterceptorList{interceptor1, interceptor2}));
      });

  for (auto& interceptor : {interceptor1, interceptor2}) {
    EXPECT_EQ(interceptor->onStartServingCount, 1);
  }
}

TEST_P(ServiceInterceptorTestP, DuplicateNameThrows) {
  auto interceptor1 =
      std::make_shared<ServiceInterceptorCountWithRequestState>("Duplicate");
  auto interceptor2 =
      std::make_shared<ServiceInterceptorCountWithRequestState>("Duplicate");

  EXPECT_THROW(
      {
        try {
          makeServer(
              std::make_shared<TestHandler>(), [&](ThriftServer& server) {
                server.addModule(
                    std::make_unique<TestModule>(
                        InterceptorList{interceptor1, interceptor2}));
              });
        } catch (const std::logic_error& ex) {
          EXPECT_THAT(ex.what(), HasSubstr("TestModule.Duplicate"));
          throw;
        }
      },
      std::logic_error);
}

CO_TEST_P(ServiceInterceptorTestP, OnRequestException) {
  auto interceptor1 =
      std::make_shared<ServiceInterceptorThrowOnRequest>("Interceptor1");
  auto interceptor2 =
      std::make_shared<ServiceInterceptorCountWithRequestState>("Interceptor2");
  auto interceptor3 =
      std::make_shared<ServiceInterceptorThrowOnRequest>("Interceptor3");
  auto runner =
      makeServer(std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(
            std::make_unique<TestModule>(
                InterceptorList{interceptor1, interceptor2, interceptor3}));
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
          EXPECT_THAT(
              std::string(ex.what()), HasSubstr("[TestModule.Interceptor1]"));
          EXPECT_THAT(
              std::string(ex.what()),
              Not(HasSubstr("[TestModule.Interceptor2]")));
          EXPECT_THAT(
              std::string(ex.what()), HasSubstr("[TestModule.Interceptor3]"));
          throw;
        }
      },
      apache::thrift::TApplicationException);
  EXPECT_EQ(interceptor1->onRequestCount, 1);
  EXPECT_EQ(interceptor1->onResponseCount, 1);
  EXPECT_EQ(interceptor2->onRequestCount, 1);
  EXPECT_EQ(interceptor2->onResponseCount, 1);
  EXPECT_EQ(interceptor3->onRequestCount, 1);
  EXPECT_EQ(interceptor3->onResponseCount, 1);
}

CO_TEST_P(
    ServiceInterceptorTestP, InterceptorOnRequestExceptionCatchSelfThrowTest) {
  auto interceptor1 = std::make_shared<ServiceInterceptorRequestExceptionCatch>(
      "Interceptor1",
      true /* self throw */,
      "Exception from ServiceInterceptorRequestExceptionCatch");
  auto runner =
      makeServer(std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(
            std::make_unique<TestModule>(InterceptorList{interceptor1}));
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
                  "Exception from ServiceInterceptorRequestExceptionCatch"));
          EXPECT_THAT(
              std::string(ex.what()), HasSubstr("[TestModule.Interceptor1]"));
          throw;
        }
      },
      apache::thrift::TApplicationException);
  EXPECT_EQ(interceptor1->onRequestCount, 1);
  EXPECT_EQ(interceptor1->onResponseCount, 1);
}

CO_TEST_P(
    ServiceInterceptorTestP, InterceptorOnRequestExceptionCatchOtherThrowTest) {
  auto interceptor1 =
      std::make_shared<ServiceInterceptorThrowOnRequest>("Interceptor1");
  auto interceptor2 = std::make_shared<ServiceInterceptorRequestExceptionCatch>(
      "Interceptor2",
      false /* self throw */,
      "Exception from ServiceInterceptorThrowOnRequest::onRequest");
  auto runner =
      makeServer(std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(
            std::make_unique<TestModule>(
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
          EXPECT_THAT(
              std::string(ex.what()), HasSubstr("[TestModule.Interceptor1]"));
          EXPECT_THAT(
              std::string(ex.what()),
              Not(HasSubstr("[TestModule.Interceptor2]")));
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
  auto interceptor =
      std::make_shared<ServiceInterceptorThrowOnRequest>("Interceptor1");
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
  auto interceptor1 =
      std::make_shared<ServiceInterceptorThrowOnResponse>("Interceptor1");
  auto interceptor2 =
      std::make_shared<ServiceInterceptorCountWithRequestState>("Interceptor2");
  auto interceptor3 =
      std::make_shared<ServiceInterceptorThrowOnResponse>("Interceptor3");
  auto runner =
      makeServer(std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(
            std::make_unique<TestModule>(
                InterceptorList{interceptor1, interceptor2, interceptor3}));
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
          EXPECT_THAT(
              std::string(ex.what()), HasSubstr("[TestModule.Interceptor1]"));
          EXPECT_THAT(
              std::string(ex.what()),
              Not(HasSubstr("[TestModule.Interceptor2]")));
          EXPECT_THAT(
              std::string(ex.what()), HasSubstr("[TestModule.Interceptor3]"));
          throw;
        }
      },
      apache::thrift::TApplicationException);
  EXPECT_EQ(interceptor1->onRequestCount, 1);
  EXPECT_EQ(interceptor1->onResponseCount, 1);
  EXPECT_EQ(interceptor2->onRequestCount, 1);
  EXPECT_EQ(interceptor2->onResponseCount, 1);
  EXPECT_EQ(interceptor3->onRequestCount, 1);
  EXPECT_EQ(interceptor3->onResponseCount, 1);
}

CO_TEST_P(ServiceInterceptorTestP, OnResponseExceptionEB) {
  auto interceptor1 =
      std::make_shared<ServiceInterceptorThrowOnResponse>("Interceptor1");
  auto interceptor2 =
      std::make_shared<ServiceInterceptorCountWithRequestState>("Interceptor2");
  auto runner =
      makeServer(std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(
            std::make_unique<TestModule>(
                InterceptorList{interceptor1, interceptor2}));
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
      std::make_shared<ServiceInterceptorCountWithRequestState>("Interceptor1");

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
    ServiceInterceptorTestP, OnResponseExceptionSwallowsApplicationException) {
  auto interceptor =
      std::make_shared<ServiceInterceptorRethrowActiveExceptionOnResponse>(
          "Interceptor1");
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
          EXPECT_THAT(
              std::string(ex.what()),
              HasSubstr("ServiceInterceptor::onResponse threw exceptions"));
          EXPECT_THAT(
              std::string(ex.what()), HasSubstr("[TestModule.Interceptor1]"));
          EXPECT_THAT(std::string(ex.what()), HasSubstr("You asked for it!"));
          throw;
        }
      },
      apache::thrift::TApplicationException);
  EXPECT_EQ(interceptor->onRequestCount, 1);
  EXPECT_EQ(interceptor->onResponseCount, 1);
}

CO_TEST_P(
    ServiceInterceptorTestP, OnResponseExceptionSwallowsOnRequestException) {
  auto interceptor1 =
      std::make_shared<ServiceInterceptorThrowOnRequest>("Interceptor1");
  auto interceptor2 =
      std::make_shared<ServiceInterceptorRethrowActiveExceptionOnResponse>(
          "Interceptor2");
  auto runner =
      makeServer(std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(
            std::make_unique<TestModule>(
                InterceptorList{interceptor1, interceptor2}));
      });

  auto client =
      makeClient<apache::thrift::Client<test::ServiceInterceptorTest>>(*runner);
  EXPECT_THROW(
      {
        try {
          co_await client->co_noop();
        } catch (const apache::thrift::TApplicationException& ex) {
          EXPECT_THAT(
              std::string(ex.what()),
              HasSubstr("ServiceInterceptor::onRequest threw exceptions"));
          EXPECT_THAT(
              std::string(ex.what()), HasSubstr("[TestModule.Interceptor1]"));
          EXPECT_THAT(
              std::string(ex.what()),
              HasSubstr("ServiceInterceptor::onResponse threw exceptions"));
          EXPECT_THAT(
              std::string(ex.what()), HasSubstr("[TestModule.Interceptor2]"));
          throw;
        }
      },
      apache::thrift::TApplicationException);
  EXPECT_EQ(interceptor1->onRequestCount, 1);
  EXPECT_EQ(interceptor1->onResponseCount, 1);
  EXPECT_EQ(interceptor2->onRequestCount, 1);
  EXPECT_EQ(interceptor2->onResponseCount, 1);
}

CO_TEST_P(ServiceInterceptorTestP, OnConnectionException) {
  auto interceptor1 =
      std::make_shared<ServiceInterceptorThrowOnConnection>("Interceptor1");
  auto interceptor2 =
      std::make_shared<ServiceInterceptorCountWithRequestState>("Interceptor2");
  auto interceptor3 =
      std::make_shared<ServiceInterceptorThrowOnConnection>("Interceptor3");
  auto runner =
      makeServer(std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(
            std::make_unique<TestModule>(
                InterceptorList{interceptor1, interceptor2, interceptor3}));
      });

  if (transportType() == TransportType::HTTP2) {
    // HTTP2 does not support onConnection
    auto client =
        makeClient<apache::thrift::Client<test::ServiceInterceptorTest>>(
            *runner);
    EXPECT_NO_THROW({ co_await client->co_echo(""); });
    co_return;
  }

  EXPECT_THROW(
      {
        try {
          auto client =
              makeClient<apache::thrift::Client<test::ServiceInterceptorTest>>(
                  *runner);
          co_await client->co_echo("");
        } catch (const apache::thrift::transport::TTransportException& ex) {
          if (transportType() == TransportType::HEADER) {
            // Header transport does not support any mechanism of communicating
            // errors on connection creation back to the client.
            throw;
          }
          EXPECT_THAT(
              std::string(ex.what()),
              HasSubstr(
                  "ServiceInterceptor::onConnectionEstablished threw exceptions"));
          EXPECT_THAT(
              std::string(ex.what()),
              HasSubstr(
                  "Exception from ServiceInterceptorThrowOnConnection::onConnectionEstablished"));
          EXPECT_THAT(
              std::string(ex.what()), HasSubstr("[TestModule.Interceptor1]"));
          EXPECT_THAT(
              std::string(ex.what()),
              Not(HasSubstr("[TestModule.Interceptor2]")));
          EXPECT_THAT(
              std::string(ex.what()), HasSubstr("[TestModule.Interceptor3]"));
          throw;
        }
      },
      apache::thrift::transport::TTransportException);

  runner.reset();
  EXPECT_EQ(interceptor1->onConnectionCount, 1);
  EXPECT_EQ(interceptor1->onConnectionClosedCount, 1);
  // If onConnection throws, then requests should not be processed.
  EXPECT_EQ(interceptor2->onRequestCount, 0);
  EXPECT_EQ(interceptor2->onResponseCount, 0);
  EXPECT_EQ(interceptor3->onConnectionCount, 1);
  EXPECT_EQ(interceptor3->onConnectionClosedCount, 1);
}

CO_TEST_P(ServiceInterceptorTestP, BasicInteraction) {
  if (transportType() != TransportType::ROCKET) {
    // only rocket supports interactions
    co_return;
  }
  auto interceptor1 =
      std::make_shared<ServiceInterceptorCountWithRequestState>("Interceptor1");
  auto interceptor2 =
      std::make_shared<ServiceInterceptorCountWithRequestState>("Interceptor2");
  auto runner =
      makeServer(std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(
            std::make_unique<TestModule>(
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
      std::make_shared<ServiceInterceptorCountWithRequestState>("Interceptor1");
  auto interceptor2 =
      std::make_shared<ServiceInterceptorCountWithRequestState>("Interceptor2");
  auto runner =
      makeServer(std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(
            std::make_unique<TestModule>(
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

// ============ Streaming Interceptor Tests ============

CO_TEST_P(ServiceInterceptorTestP, StreamingInterceptorLifecycle) {
  if (transportType() != TransportType::ROCKET) {
    // only rocket supports streaming
    co_return;
  }

  // Baton to synchronize with onStreamEnd callback
  folly::coro::Baton streamEndBaton;

  // Interceptor that tracks the full streaming lifecycle
  // Uses RequestState to store state that persists through streaming
  struct StreamingLifecycleInterceptor
      : public ServiceInterceptor<std::string, folly::Unit> {
    using ConnectionState = folly::Unit;
    using RequestState = std::string;

    explicit StreamingLifecycleInterceptor(
        std::string name, folly::coro::Baton& baton)
        : name_(std::move(name)), streamEndBaton_(baton) {}

    std::string getName() const override { return name_; }

    folly::coro::Task<std::optional<RequestState>> onRequest(
        ConnectionState*, RequestInfo) override {
      // Initialize state that will be accessible during streaming
      co_return std::make_optional<RequestState>("stream-state-initialized");
    }

    folly::coro::Task<void> onStreamBegin(
        RequestState*, ConnectionState*, StreamInfo info) override {
      onStreamBeginCalled = true;
      streamId = info.streamId;
      serviceName = std::string(info.serviceName);
      methodName = std::string(info.methodName);
      co_return;
    }

    folly::coro::Task<void> onStreamPayload(
        RequestState* requestState,
        ConnectionState*,
        StreamPayloadInfo info) override {
      ++payloadCount;
      sequenceNumbers.push_back(info.sequenceNumber);
      // Verify we can access the typed payload
      auto value = info.payload.value<std::int32_t>();
      payloadValues.push_back(value);
      // Verify request state is accessible during streaming
      if (requestState) {
        EXPECT_EQ(*requestState, "stream-state-initialized");
      }
      co_return;
    }

    folly::coro::Task<void> onStreamEnd(
        RequestState* requestState,
        ConnectionState*,
        StreamEndInfo info) override {
      onStreamEndCalled = true;
      endReason = info.reason;
      totalPayloadsReported = info.totalPayloads;
      // Verify request state persists to stream end
      if (requestState) {
        EXPECT_EQ(*requestState, "stream-state-initialized");
      }
      streamEndBaton_.post();
      co_return;
    }

    std::string name_;
    folly::coro::Baton& streamEndBaton_;
    bool onStreamBeginCalled = false;
    bool onStreamEndCalled = false;
    detail::StreamId streamId = 0;
    std::string serviceName;
    std::string methodName;
    int payloadCount = 0;
    std::vector<uint64_t> sequenceNumbers;
    std::vector<std::int32_t> payloadValues;
    details::STREAM_ENDING_TYPES endReason =
        details::STREAM_ENDING_TYPES::COMPLETE;
    uint64_t totalPayloadsReported = 0;
  };

  auto interceptor = std::make_shared<StreamingLifecycleInterceptor>(
      "Test", std::ref(streamEndBaton));
  auto runner =
      makeServer(std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(interceptor));
      });

  auto client =
      makeClient<apache::thrift::Client<test::ServiceInterceptorTest>>(*runner);
  {
    auto stream = (co_await client->co_iota(100)).toAsyncGenerator();
    // Consume a few payloads
    EXPECT_EQ((co_await stream.next()).value(), 100);
    EXPECT_EQ((co_await stream.next()).value(), 101);
    EXPECT_EQ((co_await stream.next()).value(), 102);
  }
  // Wait for onStreamEnd to be called on the server
  co_await streamEndBaton;

  EXPECT_TRUE(interceptor->onStreamBeginCalled);
  EXPECT_GT(interceptor->streamId, 0u);
  EXPECT_EQ(interceptor->serviceName, "ServiceInterceptorTest");
  EXPECT_EQ(interceptor->methodName, "iota");

  // Verify payloads were intercepted with correct sequence numbers
  EXPECT_GE(interceptor->payloadCount, 3);
  for (size_t i = 0; i < interceptor->sequenceNumbers.size(); ++i) {
    EXPECT_EQ(interceptor->sequenceNumbers[i], i);
  }
  // Verify payload values were accessible
  const std::vector<std::int32_t> expectedPayloadValues{100, 101, 102};
  EXPECT_EQ(
      std::vector<std::int32_t>(
          interceptor->payloadValues.begin(),
          interceptor->payloadValues.begin() + 3),
      expectedPayloadValues);

  // Verify stream end was called
  EXPECT_TRUE(interceptor->onStreamEndCalled);
  EXPECT_EQ(interceptor->endReason, details::STREAM_ENDING_TYPES::CANCEL);
  EXPECT_EQ(interceptor->totalPayloadsReported, interceptor->payloadCount);
}

CO_TEST_P(
    ServiceInterceptorTestP, StreamingInterceptorMultipleInterceptorsLIFO) {
  if (transportType() != TransportType::ROCKET) {
    co_return;
  }

  // Baton to synchronize - posted by the last interceptor in LIFO order
  folly::coro::Baton streamEndBaton;

  // Track callback order across multiple interceptors
  static std::vector<std::string> callOrder;
  callOrder.clear();

  struct OrderTrackingInterceptor
      : public ServiceInterceptor<folly::Unit, folly::Unit> {
    using ConnectionState = folly::Unit;
    using RequestState = folly::Unit;

    explicit OrderTrackingInterceptor(
        std::string name, folly::coro::Baton* baton = nullptr)
        : name_(std::move(name)), streamEndBaton_(baton) {}

    std::string getName() const override { return name_; }

    folly::coro::Task<void> onStreamBegin(
        RequestState*, ConnectionState*, StreamInfo) override {
      callOrder.push_back(name_ + ":begin");
      co_return;
    }

    folly::coro::Task<void> onStreamPayload(
        RequestState*, ConnectionState*, StreamPayloadInfo) override {
      // Only track first payload to keep test simple
      if (callOrder.empty() ||
          callOrder.back().find(":payload") == std::string::npos ||
          callOrder.back().find(name_) == std::string::npos) {
        callOrder.push_back(name_ + ":payload");
      }
      co_return;
    }

    folly::coro::Task<void> onStreamEnd(
        RequestState*, ConnectionState*, StreamEndInfo) override {
      callOrder.push_back(name_ + ":end");
      if (streamEndBaton_) {
        streamEndBaton_->post();
      }
      co_return;
    }

    std::string name_;
    folly::coro::Baton* streamEndBaton_;
  };

  // First interceptor is called last in LIFO order, so it posts the baton
  auto interceptor1 =
      std::make_shared<OrderTrackingInterceptor>("First", &streamEndBaton);
  auto interceptor2 = std::make_shared<OrderTrackingInterceptor>("Second");
  auto interceptor3 = std::make_shared<OrderTrackingInterceptor>("Third");

  auto runner =
      makeServer(std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(
            std::make_unique<TestModule>(
                InterceptorList{interceptor1, interceptor2, interceptor3}));
      });

  auto client =
      makeClient<apache::thrift::Client<test::ServiceInterceptorTest>>(*runner);
  {
    auto stream = (co_await client->co_iota(1)).toAsyncGenerator();
    EXPECT_EQ((co_await stream.next()).value(), 1);
  }

  // Wait for all onStreamEnd callbacks to complete
  co_await streamEndBaton;

  // Verify onStreamBegin called in forward order (FIFO)
  EXPECT_GE(callOrder.size(), 6u);
  if (callOrder.size() < 6u) {
    co_return;
  }
  const std::vector<std::string> expectedBegin{
      "First:begin", "Second:begin", "Third:begin"};
  EXPECT_EQ(
      std::vector<std::string>(callOrder.begin(), callOrder.begin() + 3),
      expectedBegin);

  // Verify onStreamEnd called in reverse order (LIFO)
  auto endIt = std::find_if(
      callOrder.begin(), callOrder.end(), [](const std::string& s) {
        return s.find(":end") != std::string::npos;
      });
  EXPECT_NE(endIt, callOrder.end());
  if (endIt == callOrder.end()) {
    co_return;
  }
  std::vector<std::string> endCalls(endIt, callOrder.end());
  const std::vector<std::string> expectedEnd{
      "Third:end", "Second:end", "First:end"};
  EXPECT_EQ(endCalls, expectedEnd);
}

CO_TEST_P(ServiceInterceptorTestP, StreamingInterceptorMetrics) {
  if (transportType() != TransportType::ROCKET) {
    co_return;
  }

  // Simple interceptor that does minimal work
  struct SimpleStreamInterceptor
      : public ServiceInterceptor<folly::Unit, folly::Unit> {
    std::string getName() const override { return "SimpleStream"; }
  };

  auto interceptor = std::make_shared<SimpleStreamInterceptor>();
  auto interceptorMetricCallback =
      std::make_shared<MockInterceptorMetricCallback>();

  auto runner =
      makeServer(std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(interceptor));
        detail::ThriftServerInternals(server).setInterceptorMetricCallback(
            interceptorMetricCallback);
      });

  // Expect stream metrics to be called
  EXPECT_CALL(*interceptorMetricCallback, onStreamBeginComplete(_, _))
      .Times(testing::AtLeast(1));
  EXPECT_CALL(*interceptorMetricCallback, onStreamPayloadComplete(_, _))
      .Times(testing::AtLeast(1));
  EXPECT_CALL(*interceptorMetricCallback, onStreamEndComplete(_, _))
      .Times(testing::AtLeast(1));

  // Also expect request/response metrics
  EXPECT_CALL(*interceptorMetricCallback, onRequestComplete(_, _))
      .Times(testing::AtLeast(1));
  EXPECT_CALL(*interceptorMetricCallback, onResponseComplete(_, _))
      .Times(testing::AtLeast(1));

  // Allow connection metrics (may or may not be called depending on transport)
  EXPECT_CALL(*interceptorMetricCallback, onConnectionAttemptedComplete(_, _))
      .Times(testing::AnyNumber());
  EXPECT_CALL(*interceptorMetricCallback, onConnectionComplete(_, _))
      .Times(testing::AnyNumber());
  EXPECT_CALL(*interceptorMetricCallback, onConnectionClosedComplete(_, _))
      .Times(testing::AnyNumber());

  auto client =
      makeClient<apache::thrift::Client<test::ServiceInterceptorTest>>(*runner);
  {
    auto stream = (co_await client->co_iota(1)).toAsyncGenerator();
    // Consume at least one payload
    EXPECT_EQ((co_await stream.next()).value(), 1);
  }

  // Give the server time to process stream end
  co_await folly::coro::sleep(std::chrono::milliseconds(100));
}

CO_TEST_P(ServiceInterceptorTestP, RequestArguments) {
  struct ServiceInterceptorWithRequestArguments
      : public NamedServiceInterceptor<folly::Unit> {
   public:
    using ConnectionState = folly::Unit;
    using RequestState = folly::Unit;

    using NamedServiceInterceptor::NamedServiceInterceptor;

    folly::coro::Task<std::optional<RequestState>> onRequest(
        ConnectionState*, RequestInfo requestInfo) override {
      argsCount = requestInfo.arguments.count();
      arg1 = requestInfo.arguments.get(0)->value<std::int32_t>();
      arg2 = requestInfo.arguments.get(1)->value<std::string>();
      arg3 = requestInfo.arguments.get(2)->value<test::RequestArgsStruct>();
      EXPECT_THROW(
          requestInfo.arguments.get(2)->value<std::int32_t>(), std::bad_cast);
      EXPECT_FALSE(requestInfo.arguments.get(3).has_value());

      // Also verify dynamicArguments API
      if (requestInfo.dynamicArguments) {
        dynamicArgsAvailable = true;
        dynamicArgsCount = requestInfo.dynamicArguments->count();

        // Access by index
        dynamicArg1 = requestInfo.dynamicArguments->get(0).asI32();
        dynamicArg2 =
            std::string(requestInfo.dynamicArguments->get(1).asString());
        {
          const auto& s = requestInfo.dynamicArguments->get(2).asStruct();
          if (auto fooField = s.getField("foo")) {
            dynamicArg3Foo = fooField->asI32();
          }
          if (auto barField = s.getField("bar")) {
            dynamicArg3Bar = std::string(barField->asString());
          }
        }

        // Access by name
        dynamicArg1ByName =
            requestInfo.dynamicArguments->get("primitive").asI32();
        EXPECT_EQ(requestInfo.dynamicArguments->name(0), "primitive");

        // Out of bounds should throw
        dynamicArg3OutOfBoundsThrows = true;
        try {
          requestInfo.dynamicArguments->get(3);
          dynamicArg3OutOfBoundsThrows = false;
        } catch (const std::out_of_range&) {
          // Expected
        }
      }

      co_return std::nullopt;
    }

    std::size_t argsCount = 0;
    std::int32_t arg1 = 0;
    std::string arg2;
    test::RequestArgsStruct arg3;

    bool dynamicArgsAvailable = false;
    std::size_t dynamicArgsCount = 0;
    std::int32_t dynamicArg1 = 0;
    std::string dynamicArg2;
    std::int32_t dynamicArg3Foo = 0;
    std::string dynamicArg3Bar;
    std::int32_t dynamicArg1ByName = 0;
    bool dynamicArg3OutOfBoundsThrows = false;
  };
  auto interceptor =
      std::make_shared<ServiceInterceptorWithRequestArguments>("Interceptor1");
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

  EXPECT_TRUE(interceptor->dynamicArgsAvailable);
  EXPECT_EQ(interceptor->dynamicArgsCount, 3);
  EXPECT_EQ(interceptor->dynamicArg1, 1);
  EXPECT_EQ(interceptor->dynamicArg2, "hello");
  EXPECT_EQ(interceptor->dynamicArg3Foo, 1);
  EXPECT_EQ(interceptor->dynamicArg3Bar, "hello");
  EXPECT_EQ(interceptor->dynamicArg1ByName, 1);
  EXPECT_TRUE(interceptor->dynamicArg3OutOfBoundsThrows);
}

namespace {
struct ServiceNameInfo {
  std::string serviceName;
  std::string definingServiceName;
  std::string methodName;
  std::string qualifiedMethodName;

  [[maybe_unused]] friend bool operator==(
      const ServiceNameInfo& lhs, const ServiceNameInfo& rhs) {
    return std::tie(
               lhs.serviceName,
               lhs.definingServiceName,
               lhs.methodName,
               lhs.qualifiedMethodName) ==
        std::tie(
               rhs.serviceName,
               rhs.definingServiceName,
               rhs.methodName,
               rhs.qualifiedMethodName);
  }

  [[maybe_unused]] friend std::ostream& operator<<(
      std::ostream& out, const ServiceNameInfo& info) {
    return out << fmt::format(
               "[{}, {}, {}, {}]",
               info.serviceName,
               info.definingServiceName,
               info.methodName,
               info.qualifiedMethodName);
  }
};

struct ServiceInterceptorCheckingServiceAndMethodNames
    : public NamedServiceInterceptor<folly::Unit> {
 public:
  using ConnectionState = folly::Unit;
  using RequestState = folly::Unit;

  ServiceInterceptorCheckingServiceAndMethodNames()
      : NamedServiceInterceptor("SomeName") {}

  folly::coro::Task<std::optional<RequestState>> onRequest(
      ConnectionState*, RequestInfo requestInfo) override {
    namesOnRequest.emplace_back(
        std::string(requestInfo.serviceName),
        std::string(requestInfo.definingServiceName),
        std::string(requestInfo.methodName),
        std::string(requestInfo.qualifiedMethodName));
    co_return std::nullopt;
  }

  folly::coro::Task<void> onResponse(
      RequestState*, ConnectionState*, ResponseInfo requestInfo) override {
    namesOnResponse.emplace_back(
        std::string(requestInfo.serviceName),
        std::string(requestInfo.definingServiceName),
        std::string(requestInfo.methodName),
        std::string(requestInfo.qualifiedMethodName));
    co_return;
  }

  std::vector<ServiceNameInfo> namesOnRequest;
  std::vector<ServiceNameInfo> namesOnResponse;
};
} // namespace

CO_TEST_P(ServiceInterceptorTestP, ServiceAndMethodNamesBasic) {
  auto interceptor =
      std::make_shared<ServiceInterceptorCheckingServiceAndMethodNames>();
  auto runner =
      makeServer(std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(interceptor));
      });

  auto client =
      makeClient<apache::thrift::Client<test::ServiceInterceptorTest>>(*runner);
  co_await client->co_echo("");
  co_await client->co_noop();

  std::vector<ServiceNameInfo> expectedNames = {
      {"ServiceInterceptorTest",
       "ServiceInterceptorTest",
       "echo",
       "ServiceInterceptorTest.echo"},
      {"ServiceInterceptorTest",
       "ServiceInterceptorTestBase",
       "noop",
       "ServiceInterceptorTestBase.noop"},
  };

  if (transportType() == TransportType::ROCKET) {
    // only rocket supports interactions
    auto interaction = co_await client->co_createInteraction();
    co_await interaction.co_echo("");
    expectedNames.emplace_back(
        "ServiceInterceptorTest",
        "ServiceInterceptorTest",
        "createInteraction",
        "ServiceInterceptorTest.createInteraction");
    expectedNames.emplace_back(
        "ServiceInterceptorTest",
        "ServiceInterceptorTest",
        "SampleInteraction.echo",
        "ServiceInterceptorTest.SampleInteraction.echo");
  }

  EXPECT_THAT(interceptor->namesOnRequest, ElementsAreArray(expectedNames));
  EXPECT_THAT(interceptor->namesOnResponse, ElementsAreArray(expectedNames));
}
/* Service Authorization Platform depends on ServiceInterceptor multiplex
 * conflict resolution. Breakage in these tests could result in ACL failures.*/
CO_TEST_P(
    ServiceInterceptorTestP, ServiceAndMethodNamesMultiplexBasicConflicts) {
  class SecondHandler : public apache::thrift::ServiceHandler<Second> {
    int three() override { return 3; }
    int four() override { return 4; }
  };

  class ThirdHandler : public apache::thrift::ServiceHandler<Third> {
    int five() override { return 5; }
    int six() override { return 6; }
  };

  class ConflictsHandler : public apache::thrift::ServiceHandler<Conflicts> {
    int four() override { return 444; }
    int five() override { return 555; }
  };
  std::vector<std::shared_ptr<AsyncProcessorFactory>> services = {
      /* order matters when resolving conflicts */
      std::make_shared<SecondHandler>(),
      std::make_shared<ConflictsHandler>(),
      std::make_shared<ThirdHandler>(),
  };
  auto interceptor =
      std::make_shared<ServiceInterceptorCheckingServiceAndMethodNames>();
  auto runner = makeServer(multiplex(services), [&](ThriftServer& server) {
    server.addModule(std::make_unique<TestModule>(interceptor));
  });

  auto client2 = makeClient<apache::thrift::Client<Second>>(*runner);
  auto client3 = makeClient<apache::thrift::Client<Third>>(*runner);

  EXPECT_EQ(co_await client2->co_three(), 3);
  // Second takes precedence
  EXPECT_EQ(co_await client2->co_four(), 4);
  // Conflicts takes precedence
  EXPECT_EQ(co_await client3->co_five(), 555);
  EXPECT_EQ(co_await client3->co_six(), 6);

  std::vector<ServiceNameInfo> expectedNames = {
      {"Second", "Second", "three", "Second.three"},
      {"Second", "Second", "four", "Second.four"},
      {"Conflicts", "Conflicts", "five", "Conflicts.five"},
      {"Third", "Third", "six", "Third.six"},
  };

  EXPECT_THAT(interceptor->namesOnRequest, ElementsAreArray(expectedNames));
  EXPECT_THAT(interceptor->namesOnResponse, ElementsAreArray(expectedNames));
}

CO_TEST_P(
    ServiceInterceptorTestP,
    ServiceAndMethodNamesMultiplexInteractionConflicts) {
  if (transportType() == TransportType::ROCKET) {
    class Interaction1Handler
        : public apache::thrift::ServiceHandler<Interaction1> {
     public:
      std::unique_ptr<Thing1If> createThing1() override {
        class Thing1 : public Thing1If {
         public:
          void foo() override {}
        };
        return std::make_unique<Thing1>();
      }
    };

    class ConflictsInteraction1Handler
        : public apache::thrift::ServiceHandler<
              apache::thrift::test2::ConflictsInteraction1> {
     public:
      std::unique_ptr<Thing1If> createThing1() override {
        class Thing1 : public Thing1If {
         public:
          void foo() override { ADD_FAILURE() << "Should never be called"; }
          void bar() override { ADD_FAILURE() << "Should never be called"; }
        };
        return std::make_unique<Thing1>();
      }
    };

    std::vector<std::shared_ptr<AsyncProcessorFactory>> services = {
        /* order matters when resolving conflicts */
        std::make_shared<Interaction1Handler>(),
        std::make_shared<ConflictsInteraction1Handler>(),
    };
    auto interceptor =
        std::make_shared<ServiceInterceptorCheckingServiceAndMethodNames>();
    auto runner = makeServer(multiplex(services), [&](ThriftServer& server) {
      server.addModule(std::make_unique<TestModule>(interceptor));
    });

    auto client1 = makeClient<apache::thrift::Client<Interaction1>>(*runner);
    auto client2 =
        makeClient<apache::thrift::Client<ConflictsInteraction1>>(*runner);

    auto thing = client1->createThing1();
    EXPECT_NO_THROW(co_await thing.co_foo());

    auto thing2 = client2->createThing1();

    // Thing1.bar from ConflictsInteraction1 should not be in MethodMetadataMap
    // because Interaction1 already added Thing1.foo.
    EXPECT_THROW(co_await thing2.co_bar(), TApplicationException);

    std::vector<ServiceNameInfo> expectedNames = {
        {"Interaction1",
         "Interaction1",
         "Thing1.foo",
         "Interaction1.Thing1.foo"},
    };

    EXPECT_THAT(interceptor->namesOnRequest, ElementsAreArray(expectedNames));
    EXPECT_THAT(interceptor->namesOnResponse, ElementsAreArray(expectedNames));
  }
}

CO_TEST_P(
    ServiceInterceptorTestP, ResultOrActiveExceptionTypesAreCorrectBasic) {
  auto interceptor =
      std::make_shared<ServiceInterceptorLogResultTypeOnResponse>(
          "Interceptor1");
  auto runner =
      makeServer(std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(interceptor));
      });

  auto client =
      makeClient<apache::thrift::Client<test::ServiceInterceptorTest>>(*runner);
  co_await client->co_echo("");
  co_await client->co_noop();

  EXPECT_THROW(
      co_await client->co_echo("throw"), apache::thrift::TApplicationException);

  co_await client->co_echo_eb("");

  {
    test::RequestArgsStruct requestArgs;
    requestArgs.foo() = 1;
    requestArgs.bar() = "hello";
    co_await client->co_echoStruct(requestArgs);
  }

  using ResultKind = ServiceInterceptorLogResultTypeOnResponse::ResultKind;
  const std::vector<ServiceInterceptorLogResultTypeOnResponse::Entry>
      expectedResults = {
          // echo
          {ResultKind::OK, typeid(std::string)},
          // noop
          {ResultKind::OK, typeid(folly::Unit)},
          // echo("throw")
          {ResultKind::EXCEPTION, typeid(std::runtime_error)},
          // echo_eb
          {ResultKind::OK, typeid(std::string)},
          // echoStruct
          {ResultKind::OK, typeid(test::ResponseArgsStruct)},
      };

  EXPECT_THAT(interceptor->results, ElementsAreArray(expectedResults));
}

CO_TEST_P(
    ServiceInterceptorTestP, ResultOrActiveExceptionTypesAreCorrectAdvanced) {
  if (transportType() != TransportType::ROCKET) {
    // only rocket supports all transport features being tested here
    co_return;
  }

  auto interceptor =
      std::make_shared<ServiceInterceptorLogResultTypeOnResponse>(
          "Interceptor1");
  auto runner =
      makeServer(std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(interceptor));
      });

  auto client =
      makeClient<apache::thrift::Client<test::ServiceInterceptorTest>>(*runner);

  {
    auto stream = (co_await client->co_iota(1)).toAsyncGenerator();
    EXPECT_EQ((co_await stream.next()).value(), 1);
    EXPECT_EQ((co_await stream.next()).value(), 2);
    // close stream
  }

  {
    auto interaction = co_await client->co_createInteraction();
    co_await interaction.co_echo("");
    // terminate interaction
  }

  {
    auto interaction = client->createSampleInteraction2();
    co_await interaction.co_echo("");
    // terminate interaction
  }

  using ResultKind = ServiceInterceptorLogResultTypeOnResponse::ResultKind;
  const std::vector<ServiceInterceptorLogResultTypeOnResponse::Entry>
      expectedResults = {
          // iota
          {ResultKind::OK, typeid(apache::thrift::ServerStream<std::int32_t>)},
          // createInteraction
          {ResultKind::OK, typeid(folly::Unit)},
          // SampleInteraction.echo
          {ResultKind::OK, typeid(std::string)},
          // SampleInteraction2.echo
          {ResultKind::OK, typeid(std::string)},
      };

  EXPECT_THAT(interceptor->results, ElementsAreArray(expectedResults));
}

CO_TEST_P(ServiceInterceptorTestP, OnConnectionAttempt) {
  struct ServiceInterceptorTrackingConnectionAttempt
      : public NamedServiceInterceptor<folly::Unit> {
   public:
    using ConnectionState = folly::Unit;
    using RequestState = folly::Unit;

    using NamedServiceInterceptor::NamedServiceInterceptor;

    void onConnectionAttempted(ConnectionInfo) override {
      onConnectionAttemptedCount++;
    }

    std::optional<ConnectionState> onConnectionEstablished(
        ConnectionInfo) override {
      onConnectionEstablishedCount++;
      return std::nullopt;
    }

    void onConnectionClosed(
        ConnectionState*, ConnectionInfo) noexcept override {
      onConnectionClosedCount++;
    }

    folly::coro::Task<std::optional<RequestState>> onRequest(
        ConnectionState*, RequestInfo) override {
      onRequestCount++;
      co_return std::nullopt;
    }

    folly::coro::Task<void> onResponse(
        RequestState*, ConnectionState*, ResponseInfo) override {
      onResponseCount++;
      co_return;
    }

    int onConnectionAttemptedCount = 0;
    int onConnectionEstablishedCount = 0;
    int onConnectionClosedCount = 0;
    int onRequestCount = 0;
    int onResponseCount = 0;
  };

  auto interceptor =
      std::make_shared<ServiceInterceptorTrackingConnectionAttempt>(
          "ConnectionAttemptInterceptor");
  auto runner =
      makeServer(std::make_shared<TestHandler>(), [&](ThriftServer& server) {
        server.addModule(std::make_unique<TestModule>(interceptor));
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

    co_await client->co_echo("");

    // Both onConnectionAttempted and onConnectionEstablished should be called
    // after the first request (which establishes the connection)
    EXPECT_EQ(interceptor->onConnectionAttemptedCount, valueIfNotHttp2(1));
    EXPECT_EQ(interceptor->onConnectionEstablishedCount, valueIfNotHttp2(1));
    EXPECT_EQ(interceptor->onConnectionClosedCount, valueIfNotHttp2(0));
    EXPECT_EQ(interceptor->onRequestCount, 1);
    EXPECT_EQ(interceptor->onResponseCount, 1);

    co_await client->co_echo("");
    EXPECT_EQ(interceptor->onRequestCount, 2);
    EXPECT_EQ(interceptor->onResponseCount, 2);

    // Connection counts should remain the same after additional requests
    EXPECT_EQ(interceptor->onConnectionAttemptedCount, valueIfNotHttp2(1));
    EXPECT_EQ(interceptor->onConnectionEstablishedCount, valueIfNotHttp2(1));
    EXPECT_EQ(interceptor->onConnectionClosedCount, valueIfNotHttp2(0));
  }

  // After client is destroyed, onConnectionClosed should be called
  runner.reset();
  EXPECT_EQ(interceptor->onConnectionAttemptedCount, valueIfNotHttp2(1));
  EXPECT_EQ(interceptor->onConnectionEstablishedCount, valueIfNotHttp2(1));
  EXPECT_EQ(interceptor->onConnectionClosedCount, valueIfNotHttp2(1));
}

INSTANTIATE_TEST_SUITE_P(
    ServiceInterceptorTestP,
    ServiceInterceptorTestP,
    ::testing::Values(
        TransportType::HEADER, TransportType::ROCKET, TransportType::HTTP2),
    [](const TestParamInfo<ServiceInterceptorTestP::ParamType>& info) {
      const auto transportType = [](TransportType value) -> std::string_view {
        switch (value) {
          case TransportType::HEADER:
            return "HEADER";
          case TransportType::ROCKET:
            return "ROCKET";
          case TransportType::HTTP2:
            return "HTTP2";
          default:
            throw std::logic_error{"Unreachable!"};
        }
      };
      return fmt::format("{}", transportType(info.param));
    });
