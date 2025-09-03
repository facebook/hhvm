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
#include <thrift/lib/cpp2/async/HTTPClientChannel.h>
#include <thrift/lib/cpp2/server/ServerModule.h>
#include <thrift/lib/cpp2/server/ServiceInterceptor.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/server/test/gen-cpp2/ServiceMethodDecorator_clients.h>
#include <thrift/lib/cpp2/server/test/gen-cpp2/ServiceMethodDecorator_handlers.h>
#include <thrift/lib/cpp2/transport/http2/common/HTTP2RoutingHandler.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace ::testing;

namespace apache::thrift::test {

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

class ServiceMethodDecoratorTestP
    : public testing::TestWithParam<TransportType> {
 public:
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

  TransportType transportType() const { return GetParam(); }

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

} // namespace

CO_TEST_P(ServiceMethodDecoratorTestP, DecoratorMethodsAreRun) {
  struct TestState {
    struct Decorator {
      std::size_t useDecoratorData = 0;

      std::size_t beforeNoop = 0;

      std::size_t beforeSum = 0;
      std::vector<int64_t> sumValues = {};

      std::size_t beforeEcho = 0;
      std::string echoText;
    } decorator;

    struct Interceptor {
      std::size_t useDecoratorData = 0;
      std::size_t onRequest = 0;
    } interceptor;

    struct Handler {
      std::size_t useDecoratorData = 0;
      std::size_t coNoop = 0;
      std::size_t coSum = 0;
      std::size_t coEcho = 0;
    } handler;
  };

  struct CountingDecorator
      : ServiceMethodDecorator<ServiceMethodDecoratorTest> {
    CountingDecorator(TestState::Decorator& state) : state_{state} {}

    std::string_view getName() const override { return "CountingDecorator"; }

    void onBeforeStartServing(BeforeStartServingParams) override {
      state_.useDecoratorData++;
    }

    void before_noop(BeforeParams) override { state_.beforeNoop++; }

    void before_sum(BeforeParams, const std::vector<int64_t>& values) override {
      state_.beforeSum++;
      state_.sumValues = values;
    }

    void before_echo(BeforeParams, const EchoRequest& request) override {
      state_.beforeEcho++;
      state_.echoText = *request.text();
    }

    TestState::Decorator& state_;
  };

  struct Interceptor : public ServiceInterceptor<folly::Unit, folly::Unit> {
    explicit Interceptor(TestState::Interceptor& state) : state_{state} {}

    std::string getName() const override { return "Interceptor"; }

    folly::coro::Task<void> co_onStartServing(InitParams) override {
      state_.useDecoratorData++;
      co_return;
    }

    folly::coro::Task<std::optional<folly::Unit>> onRequest(
        folly::Unit*, RequestInfo) override {
      state_.onRequest++;
      co_return std::nullopt;
    }

    TestState::Interceptor& state_;
  };

  struct Module : public ServerModule {
    explicit Module(TestState::Interceptor& state) {
      interceptors_ = {std::make_shared<Interceptor>(state)};
    }

    std::string getName() const override { return "Module"; }

    std::vector<std::shared_ptr<ServiceInterceptorBase>>
    getServiceInterceptors() override {
      return interceptors_;
    }

    std::vector<std::shared_ptr<ServiceInterceptorBase>> interceptors_;
  };

  struct Handler : public ServiceHandler<ServiceMethodDecoratorTest> {
    explicit Handler(TestState::Handler& state) : state_{state} {}

    folly::coro::Task<void> co_onBeforeStartServing(
        BeforeStartServingParams) override {
      state_.useDecoratorData++;
      co_return;
    }

    folly::coro::Task<void> co_noop() override {
      state_.coNoop++;
      co_return;
    }

    folly::coro::Task<int64_t> co_sum(
        std::unique_ptr<std::vector<int64_t>> values) override {
      state_.coSum++;
      co_return std::accumulate(values->begin(), values->end(), 0);
    }

    folly::coro::Task<std::unique_ptr<EchoResponse>> co_echo(
        std::unique_ptr<EchoRequest> request) override {
      state_.coEcho++;
      EchoResponse response;
      response.text() = *request->text();
      co_return std::make_unique<EchoResponse>(std::move(response));
    }

    TestState::Handler& state_;
  };

  TestState state;
  auto handler = std::make_shared<Handler>(state.handler);
  {
    auto decorator = std::make_shared<CountingDecorator>(state.decorator);
    ServiceMethodDecoratorList<ServiceMethodDecoratorTest> decorators;
    decorators.push_back(std::move(decorator));
    apache::thrift::decorate(*handler, std::move(decorators));
  }

  auto runner = makeServer(handler, [&](ThriftServer& server) {
    server.addModule(std::make_unique<Module>(state.interceptor));
  });
  auto client = makeClient<Client<ServiceMethodDecoratorTest>>(*runner);

  EXPECT_EQ(state.decorator.useDecoratorData, 1);
  EXPECT_EQ(state.interceptor.useDecoratorData, 1);
  EXPECT_EQ(state.handler.useDecoratorData, 1);

  co_await client->co_noop();
  EXPECT_EQ(state.decorator.beforeNoop, 1);
  EXPECT_EQ(state.interceptor.onRequest, 1);
  EXPECT_EQ(state.handler.coNoop, 1);

  auto sum = co_await client->co_sum({1, 2, 3});
  EXPECT_EQ(sum, 6);
  EXPECT_EQ(state.decorator.beforeSum, 1);
  EXPECT_THAT(state.decorator.sumValues, ElementsAre(1, 2, 3));
  EXPECT_EQ(state.interceptor.onRequest, 2);
  EXPECT_EQ(state.handler.coSum, 1);

  EchoRequest request;
  request.text() = "hello";
  auto response = co_await client->co_echo(std::move(request));
  EXPECT_EQ(*response.text(), "hello");
  EXPECT_EQ(state.decorator.beforeEcho, 1);
  EXPECT_EQ(state.decorator.echoText, "hello");
  EXPECT_EQ(state.interceptor.onRequest, 3);
  EXPECT_EQ(state.handler.coEcho, 1);
}

CO_TEST_P(ServiceMethodDecoratorTestP, MultipleDecorators) {
  constexpr std::size_t kNumDecorators = 10;

  struct TestState {
    TestState() : beforeNoop(kNumDecorators, 0) {}
    std::vector<std::size_t> beforeNoop;
  };

  struct CountingDecorator
      : ServiceMethodDecorator<ServiceMethodDecoratorTest> {
    CountingDecorator(TestState& state, std::size_t idx)
        : beforeCounter_{state.beforeNoop[idx]} {}

    std::string_view getName() const override { return "CountingDecorator"; }

    void before_noop(BeforeParams) override { beforeCounter_++; }

    std::size_t& beforeCounter_;
  };

  struct Handler : public ServiceHandler<ServiceMethodDecoratorTest> {
    folly::coro::Task<void> co_noop() override { co_return; }
  };

  TestState state;
  auto handler = std::make_shared<Handler>();
  {
    ServiceMethodDecoratorList<ServiceMethodDecoratorTest> decorators;
    for (std::size_t i = 0; i < kNumDecorators; ++i) {
      auto decorator = std::make_shared<CountingDecorator>(state, i);
      decorators.push_back(std::move(decorator));
    }
    apache::thrift::decorate(*handler, std::move(decorators));
  }

  auto runner = makeServer(handler);
  auto client = makeClient<Client<ServiceMethodDecoratorTest>>(*runner);
  co_await client->co_noop();
  std::for_each(
      state.beforeNoop.begin(), state.beforeNoop.end(), [](std::size_t count) {
        EXPECT_EQ(count, 1);
      });
}

namespace {
using AssetData = std::vector<std::string>;
constexpr server::DecoratorDataKey<AssetData> kAssets;
} // namespace

CO_TEST_P(ServiceMethodDecoratorTestP, DecoratorDataPassed) {
  struct TestState {
    std::vector<std::string> observedByInterceptorOnRequest = {};
    std::vector<std::string> observedByHandler = {};
  };

  class AssetExtractionDecorator
      : public ServiceMethodDecorator<ServiceMethodDecoratorTest> {
   public:
    std::string_view getName() const override {
      return "AssetExtractionDecorator";
    }

    void onBeforeStartServing(BeforeStartServingParams params) override {
      assetData_ = params.decoratorDataHandleFactory->makeHandleForKey(kAssets);
    }

    void before_echo(
        BeforeParams beforeParams, const EchoRequest& request) override {
      std::vector<std::string> assets = {*request.text()};
      beforeParams.decoratorData.put(assetData_, std::move(assets));
    }

   private:
    server::DecoratorDataHandle<AssetData> assetData_ =
        server::DecoratorDataHandle<AssetData>::uninitialized();
  };

  struct AssetExtractionInterceptor
      : public ServiceInterceptor<folly::Unit, folly::Unit> {
    explicit AssetExtractionInterceptor(TestState& state) : state_{state} {}

    std::string getName() const override {
      return "AssetExtractionInterceptor";
    }

    folly::coro::Task<void> co_onStartServing(InitParams params) override {
      assetData_ = params.decoratorDataHandleFactory->makeHandleForKey(kAssets);
      co_return;
    }

    folly::coro::Task<std::optional<folly::Unit>> onRequest(
        folly::Unit*, RequestInfo info) override {
      if (const auto* extractedAssets = info.decoratorData->get(assetData_)) {
        state_.observedByInterceptorOnRequest = *extractedAssets;
      }
      co_return std::nullopt;
    }

    TestState& state_;
    server::DecoratorDataHandle<AssetData> assetData_ =
        server::DecoratorDataHandle<AssetData>::uninitialized();
  };

  struct Module : public ServerModule {
    explicit Module(TestState& state) {
      interceptors_ = {std::make_shared<AssetExtractionInterceptor>(state)};
    }

    std::string getName() const override { return "Module"; }

    std::vector<std::shared_ptr<ServiceInterceptorBase>>
    getServiceInterceptors() override {
      return interceptors_;
    }

    std::vector<std::shared_ptr<ServiceInterceptorBase>> interceptors_;
  };

  struct Handler : public ServiceHandler<ServiceMethodDecoratorTest> {
    explicit Handler(TestState& state) : state_{state} {}

    folly::coro::Task<void> co_onBeforeStartServing(
        BeforeStartServingParams params) override {
      assetData_ = params.decoratorDataHandleFactory->makeHandleForKey(kAssets);
      co_return;
    }

    folly::coro::Task<std::unique_ptr<EchoResponse>> co_echo(
        RequestParams params, std::unique_ptr<EchoRequest> request) override {
      if (const auto* extractedAssets =
              params.getRequestContext()->getDecoratorData().get(assetData_)) {
        state_.observedByHandler = *extractedAssets;
      }
      EchoResponse response;
      response.text() = *request->text();
      co_return std::make_unique<EchoResponse>(std::move(response));
    }

    TestState& state_;
    server::DecoratorDataHandle<AssetData> assetData_ =
        server::DecoratorDataHandle<AssetData>::uninitialized();
  };

  TestState state;
  auto handler = std::make_shared<Handler>(state);
  {
    auto decorator = std::make_shared<AssetExtractionDecorator>();
    ServiceMethodDecoratorList<ServiceMethodDecoratorTest> decorators;
    decorators.push_back(std::move(decorator));
    apache::thrift::decorate(*handler, std::move(decorators));
  }

  auto runner = makeServer(handler, [&](ThriftServer& server) {
    server.addModule(std::make_unique<Module>(state));
  });
  auto client = makeClient<Client<ServiceMethodDecoratorTest>>(*runner);

  EchoRequest request;
  request.text() = "asset";
  auto response = co_await client->co_echo(std::move(request));
  EXPECT_EQ(*response.text(), "asset");
  EXPECT_THAT(state.observedByInterceptorOnRequest, ElementsAre("asset"));
  EXPECT_THAT(state.observedByHandler, ElementsAre("asset"));
}

CO_TEST_P(ServiceMethodDecoratorTestP, AdaptedData) {
  class AdaptedDecorator : public ServiceMethodDecorator<TestAdapter> {
   public:
    std::string_view getName() const override { return "AdaptedDecorator"; }

    void onBeforeStartServing(BeforeStartServingParams params) override {
      (void)params;
      assetData_ = params.decoratorDataHandleFactory->makeHandleForKey(kAssets);
    }

    void before_echo(
        BeforeParams beforeParams, const AdaptedEcho& request) override {
      std::vector<std::string> assets = {request.adaptedText};
      beforeParams.decoratorData.put(assetData_, std::move(assets));
    }

   private:
    server::DecoratorDataHandle<AssetData> assetData_ =
        server::DecoratorDataHandle<AssetData>::uninitialized();
  };

  struct TestState {
    std::vector<std::string> observedByHandler = {};
  };

  struct Handler : public ServiceHandler<TestAdapter> {
    explicit Handler(TestState& state) : state_{state} {}

    folly::coro::Task<void> co_onBeforeStartServing(
        BeforeStartServingParams params) override {
      assetData_ = params.decoratorDataHandleFactory->makeHandleForKey(kAssets);
      co_return;
    }

    folly::coro::Task<std::unique_ptr<AdaptedEchoResponse>> co_echo(
        RequestParams params,
        std::unique_ptr<EchoRequestAdapted> request) override {
      if (const auto* extractedAssets =
              params.getRequestContext()->getDecoratorData().get(assetData_)) {
        state_.observedByHandler = *extractedAssets;
      }
      AdaptedEchoResponse response;
      response.adaptedResponseText = request->adaptedText;
      co_return std::make_unique<AdaptedEchoResponse>(std::move(response));
    }

    TestState& state_;
    server::DecoratorDataHandle<AssetData> assetData_ =
        server::DecoratorDataHandle<AssetData>::uninitialized();
  };

  TestState state;
  auto handler = std::make_shared<Handler>(state);
  {
    auto decorator = std::make_shared<AdaptedDecorator>();
    ServiceMethodDecoratorList<TestAdapter> decorators;
    decorators.push_back(std::move(decorator));
    apache::thrift::decorate(*handler, std::move(decorators));
  }

  auto runner = makeServer(handler);
  auto client = makeClient<Client<TestAdapter>>(*runner);

  EchoRequestAdapted request;
  request.adaptedText = "asset";
  auto response = co_await client->co_echo(std::move(request));
  EXPECT_EQ(response.adaptedResponseText, "asset");
  EXPECT_THAT(state.observedByHandler, ElementsAre("asset"));
}

INSTANTIATE_TEST_SUITE_P(
    ServiceMethodDecoratorTestP,
    ServiceMethodDecoratorTestP,
    testing::Values(
        TransportType::HEADER, TransportType::ROCKET, TransportType::HTTP2),
    [](const TestParamInfo<ServiceMethodDecoratorTestP::ParamType>& info) {
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
} // namespace apache::thrift::test
