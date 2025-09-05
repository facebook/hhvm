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
      std::size_t afterNoop = 0;

      std::size_t beforeSum = 0;
      std::vector<int64_t> sumValues = {};
      std::size_t afterSum = 0;
      int64_t sumResult = 0;

      std::size_t beforeEcho = 0;
      std::string echoText;
      std::size_t afterEcho = 0;
      std::string echoResult;
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

    void after_noop(AfterParams) override { state_.afterNoop++; }

    void before_sum(BeforeParams, const std::vector<int64_t>& values) override {
      state_.beforeSum++;
      state_.sumValues = values;
    }

    void after_sum(AfterParams, int64_t sum) override {
      state_.afterSum++;
      state_.sumResult = sum;
    }

    void before_echo(BeforeParams, const EchoRequest& request) override {
      state_.beforeEcho++;
      state_.echoText = *request.text();
    }

    void after_echo(AfterParams, const EchoResponse& response) override {
      state_.afterEcho++;
      state_.echoResult = *response.text();
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
  EXPECT_EQ(state.decorator.afterNoop, 1);

  auto sum = co_await client->co_sum({1, 2, 3});
  EXPECT_EQ(sum, 6);
  EXPECT_EQ(state.decorator.beforeSum, 1);
  EXPECT_THAT(state.decorator.sumValues, ElementsAre(1, 2, 3));
  EXPECT_EQ(state.interceptor.onRequest, 2);
  EXPECT_EQ(state.handler.coSum, 1);
  EXPECT_EQ(state.decorator.afterSum, 1);
  EXPECT_EQ(state.decorator.sumResult, 6);

  EchoRequest request;
  request.text() = "hello";
  auto response = co_await client->co_echo(std::move(request));
  EXPECT_EQ(*response.text(), "hello");
  EXPECT_EQ(state.decorator.beforeEcho, 1);
  EXPECT_EQ(state.decorator.echoText, "hello");
  EXPECT_EQ(state.interceptor.onRequest, 3);
  EXPECT_EQ(state.handler.coEcho, 1);
  EXPECT_EQ(state.decorator.afterEcho, 1);
  EXPECT_EQ(state.decorator.echoResult, "hello");
}

CO_TEST_P(ServiceMethodDecoratorTestP, MultipleDecorators) {
  constexpr std::size_t kNumDecorators = 10;

  struct TestState {
    TestState() : beforeNoop(kNumDecorators, 0), afterNoop(kNumDecorators, 0) {}
    std::vector<std::size_t> beforeNoop;
    std::vector<std::size_t> afterNoop;
  };

  struct CountingDecorator
      : ServiceMethodDecorator<ServiceMethodDecoratorTest> {
    CountingDecorator(TestState& state, std::size_t idx)
        : beforeCounter_{state.beforeNoop[idx]},
          afterCounter_{state.afterNoop[idx]} {}

    std::string_view getName() const override { return "CountingDecorator"; }

    void before_noop(BeforeParams) override { beforeCounter_++; }

    void after_noop(AfterParams) override { afterCounter_++; }

    std::size_t& beforeCounter_;
    std::size_t& afterCounter_;
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
  std::for_each(
      state.afterNoop.begin(), state.afterNoop.end(), [](std::size_t count) {
        EXPECT_EQ(count, 1);
      });
}

namespace {
using AssetData = std::vector<std::string>;
constexpr server::DecoratorDataKey<AssetData> kAssetsOnRequest;
constexpr server::DecoratorDataKey<AssetData> kAssetsOnResponse;
} // namespace

CO_TEST_P(ServiceMethodDecoratorTestP, DecoratorDataPassed) {
  struct TestState {
    std::vector<std::string> observedByInterceptorOnRequest = {};
    std::vector<std::string> observedByHandler = {};
    std::vector<std::string> observedByInterceptorOnResponse = {};
  };

  class AssetExtractionDecorator
      : public ServiceMethodDecorator<ServiceMethodDecoratorTest> {
   public:
    std::string_view getName() const override {
      return "AssetExtractionDecorator";
    }

    void onBeforeStartServing(BeforeStartServingParams params) override {
      assetDataOnRequest_ =
          params.decoratorDataHandleFactory->makeHandleForKey(kAssetsOnRequest);
      assetDataOnResponse_ =
          params.decoratorDataHandleFactory->makeHandleForKey(
              kAssetsOnResponse);
    }

    void before_echo(
        BeforeParams beforeParams, const EchoRequest& request) override {
      std::vector<std::string> assets = {*request.text()};
      beforeParams.decoratorData.put(assetDataOnRequest_, std::move(assets));
    }

    void after_echo(
        AfterParams afterParams, const EchoResponse& response) override {
      std::vector<std::string> assets;
      if (const auto* extractedAssets =
              afterParams.decoratorData.get(assetDataOnRequest_)) {
        assets = *extractedAssets;
      }
      assets.push_back(*response.text());
      assets.push_back("after_echo");
      afterParams.decoratorData.put(assetDataOnResponse_, std::move(assets));
    }

   private:
    server::DecoratorDataHandle<AssetData> assetDataOnRequest_ =
        server::DecoratorDataHandle<AssetData>::uninitialized();
    server::DecoratorDataHandle<AssetData> assetDataOnResponse_ =
        server::DecoratorDataHandle<AssetData>::uninitialized();
  };

  struct AssetExtractionInterceptor
      : public ServiceInterceptor<folly::Unit, folly::Unit> {
    explicit AssetExtractionInterceptor(TestState& state) : state_{state} {}

    std::string getName() const override {
      return "AssetExtractionInterceptor";
    }

    folly::coro::Task<void> co_onStartServing(InitParams params) override {
      assetDataOnRequest_ =
          params.decoratorDataHandleFactory->makeHandleForKey(kAssetsOnRequest);
      assetDataOnResponse_ =
          params.decoratorDataHandleFactory->makeHandleForKey(
              kAssetsOnResponse);
      co_return;
    }

    folly::coro::Task<std::optional<folly::Unit>> onRequest(
        folly::Unit*, RequestInfo info) override {
      if (const auto* extractedAssets =
              info.decoratorData->get(assetDataOnRequest_)) {
        state_.observedByInterceptorOnRequest = *extractedAssets;
      }
      co_return std::nullopt;
    }

    folly::coro::Task<void> onResponse(
        folly::Unit*, folly::Unit*, ResponseInfo info) override {
      if (const auto* extractedAssets =
              info.decoratorData->get(assetDataOnResponse_)) {
        state_.observedByInterceptorOnResponse = *extractedAssets;
      }
      co_return;
    }

    TestState& state_;
    server::DecoratorDataHandle<AssetData> assetDataOnRequest_ =
        server::DecoratorDataHandle<AssetData>::uninitialized();
    server::DecoratorDataHandle<AssetData> assetDataOnResponse_ =
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
      assetData_ =
          params.decoratorDataHandleFactory->makeHandleForKey(kAssetsOnRequest);
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
  EXPECT_THAT(
      state.observedByInterceptorOnResponse,
      ElementsAre("asset", "asset", "after_echo"));
}

CO_TEST_P(ServiceMethodDecoratorTestP, AdaptedData) {
  struct TestState {
    std::vector<std::string> observedByHandler = {};
    std::string responseObservedByDecoratorAfter = "";
    std::vector<std::string> assetsObservedByDecoratorAfter = {};
  };

  class AdaptedDecorator : public ServiceMethodDecorator<TestAdapter> {
   public:
    explicit AdaptedDecorator(TestState& state) : state_{state} {}

    std::string_view getName() const override { return "AdaptedDecorator"; }

    void onBeforeStartServing(BeforeStartServingParams params) override {
      (void)params;
      assetDataOnRequest_ =
          params.decoratorDataHandleFactory->makeHandleForKey(kAssetsOnRequest);
    }

    void before_echo(
        BeforeParams beforeParams, const AdaptedEcho& request) override {
      std::vector<std::string> assets = {request.adaptedText};
      beforeParams.decoratorData.put(assetDataOnRequest_, std::move(assets));
    }

    void after_echo(
        AfterParams afterParams, const AdaptedEchoResponse& response) override {
      state_.responseObservedByDecoratorAfter = response.adaptedResponseText;
      std::vector<std::string> assets;
      if (const auto* extractedAssets =
              afterParams.decoratorData.get(assetDataOnRequest_)) {
        assets = *extractedAssets;
      }
      state_.assetsObservedByDecoratorAfter = std::move(assets);
    }

   private:
    TestState& state_;
    server::DecoratorDataHandle<AssetData> assetDataOnRequest_ =
        server::DecoratorDataHandle<AssetData>::uninitialized();
  };

  struct Handler : public ServiceHandler<TestAdapter> {
    explicit Handler(TestState& state) : state_{state} {}

    folly::coro::Task<void> co_onBeforeStartServing(
        BeforeStartServingParams params) override {
      assetData_ =
          params.decoratorDataHandleFactory->makeHandleForKey(kAssetsOnRequest);
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
    auto decorator = std::make_shared<AdaptedDecorator>(state);
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
  EXPECT_EQ(state.responseObservedByDecoratorAfter, "asset");
  EXPECT_THAT(state.assetsObservedByDecoratorAfter, ElementsAre("asset"));
}

namespace {
constexpr server::DecoratorDataKey<bool> kOnRequestData;
constexpr server::DecoratorDataKey<bool> kOnResponseData;
} // namespace

CO_TEST_P(ServiceMethodDecoratorTestP, PerformsInteractions) {
  if (transportType() != TransportType::ROCKET) {
    // only rocket supports interactions
    co_return;
  }
  struct Decorator : public ServiceMethodDecorator<PerformInteractionTest> {
    std::string_view getName() const override { return "Decorator"; }

    void onBeforeStartServing(BeforeStartServingParams params) override {
      onRequestData_ =
          params.decoratorDataHandleFactory->makeHandleForKey(kOnRequestData);
      onResponseData_ =
          params.decoratorDataHandleFactory->makeHandleForKey(kOnResponseData);
    }

    void before_createPerformedInteraction(BeforeParams beforeParams) override {
      EXPECT_EQ(beforeParams.decoratorData.get(onRequestData_), nullptr);
      beforeParams.decoratorData.put(onRequestData_, true);
    }

    void after_createPerformedInteraction(AfterParams afterParams) override {
      const auto* onRequestData = afterParams.decoratorData.get(onRequestData_);
      EXPECT_NE(onRequestData, nullptr);
      EXPECT_TRUE(*onRequestData);
      afterParams.decoratorData.put(onResponseData_, true);
    }

    void before_PerformedInteraction_perform(
        BeforeParams beforeParams) override {
      EXPECT_EQ(beforeParams.decoratorData.get(onRequestData_), nullptr);
      beforeParams.decoratorData.put(onRequestData_, true);
    }

    void after_PerformedInteraction_perform(AfterParams afterParams) override {
      const auto* onRequestData = afterParams.decoratorData.get(onRequestData_);
      EXPECT_NE(onRequestData, nullptr);
      EXPECT_TRUE(*onRequestData);
      afterParams.decoratorData.put(onResponseData_, true);
    }

    server::DecoratorDataHandle<bool> onRequestData_ =
        server::DecoratorDataHandle<bool>::uninitialized();
    server::DecoratorDataHandle<bool> onResponseData_ =
        server::DecoratorDataHandle<bool>::uninitialized();
  };

  struct Interceptor : public ServiceInterceptor<folly::Unit, folly::Unit> {
    std::string getName() const override { return "Interceptor"; }

    folly::coro::Task<void> co_onStartServing(InitParams params) override {
      onRequestData_ =
          params.decoratorDataHandleFactory->makeHandleForKey(kOnRequestData);
      onResponseData_ =
          params.decoratorDataHandleFactory->makeHandleForKey(kOnResponseData);
      co_return;
    }

    folly::coro::Task<std::optional<folly::Unit>> onRequest(
        folly::Unit*, RequestInfo info) override {
      const auto* onRequestData = info.decoratorData->get(onRequestData_);
      EXPECT_NE(onRequestData, nullptr);
      EXPECT_TRUE(*onRequestData);
      co_return std::nullopt;
    }

    folly::coro::Task<void> onResponse(
        folly::Unit*, folly::Unit*, ResponseInfo info) override {
      const auto* onResponseData = info.decoratorData->get(onResponseData_);
      EXPECT_NE(onResponseData, nullptr);
      EXPECT_TRUE(*onResponseData);
      co_return;
    }

    server::DecoratorDataHandle<bool> onRequestData_ =
        server::DecoratorDataHandle<bool>::uninitialized();
    server::DecoratorDataHandle<bool> onResponseData_ =
        server::DecoratorDataHandle<bool>::uninitialized();
  };

  struct Module : public ServerModule {
    explicit Module() { interceptors_ = {std::make_shared<Interceptor>()}; }

    std::string getName() const override { return "Module"; }

    std::vector<std::shared_ptr<ServiceInterceptorBase>>
    getServiceInterceptors() override {
      return interceptors_;
    }

    std::vector<std::shared_ptr<ServiceInterceptorBase>> interceptors_;
  };

  class Handler : public ServiceHandler<PerformInteractionTest> {
   public:
    class PerformedInteraction : public PerformedInteractionIf {
     public:
      PerformedInteraction(
          const server::DecoratorDataHandle<bool>& onRequestData)
          : onRequestData_{onRequestData} {}

      folly::coro::Task<void> co_perform(RequestParams params) override {
        const auto* onRequestData =
            params.getRequestContext()->getDecoratorData().get(onRequestData_);
        EXPECT_NE(onRequestData, nullptr);
        EXPECT_TRUE(*onRequestData);
        co_return;
      }

      const server::DecoratorDataHandle<bool>& onRequestData_;
    };

    folly::coro::Task<void> co_onBeforeStartServing(
        BeforeStartServingParams params) override {
      onRequestData_ =
          params.decoratorDataHandleFactory->makeHandleForKey(kOnRequestData);
      co_return;
    }

   private:
    std::unique_ptr<PerformedInteractionIf> createPerformedInteraction()
        override {
      return std::make_unique<PerformedInteraction>(onRequestData_);
    }

    server::DecoratorDataHandle<bool> onRequestData_ =
        server::DecoratorDataHandle<bool>::uninitialized();
  };

  auto handler = std::make_shared<Handler>();
  {
    auto decorator = std::make_shared<Decorator>();
    ServiceMethodDecoratorList<PerformInteractionTest> decorators;
    decorators.push_back(std::move(decorator));
    apache::thrift::decorate(*handler, std::move(decorators));
  }
  auto runner = makeServer(handler, [&](ThriftServer& server) {
    server.addModule(std::make_unique<Module>());
  });

  auto client = makeClient<Client<PerformInteractionTest>>(*runner);
  auto performsInteraction = client->createPerformedInteraction();
  co_await performsInteraction.co_perform();
}

CO_TEST_P(ServiceMethodDecoratorTestP, ReturnsInteractions) {
  if (transportType() != TransportType::ROCKET) {
    // only rocket supports interactions
    co_return;
  }

  struct Decorator : public ServiceMethodDecorator<ReturnsInteractionTest> {
    std::string_view getName() const override { return "Decorator"; }

    void onBeforeStartServing(BeforeStartServingParams params) override {
      onRequestData_ =
          params.decoratorDataHandleFactory->makeHandleForKey(kOnRequestData);
      onResponseData_ =
          params.decoratorDataHandleFactory->makeHandleForKey(kOnResponseData);
    }

    void before_startAdding(
        BeforeParams beforeParams, int64_t initValue) override {
      EXPECT_EQ(beforeParams.decoratorData.get(onRequestData_), nullptr);
      beforeParams.decoratorData.put(onRequestData_, true);
      before_startAddingCount++;
      beforeVals.push_back(initValue);
    }

    void after_startAdding(AfterParams afterParams, int64_t result) override {
      const auto* onRequestData = afterParams.decoratorData.get(onRequestData_);
      EXPECT_NE(onRequestData, nullptr);
      EXPECT_TRUE(*onRequestData);
      afterParams.decoratorData.put(onResponseData_, true);
      after_startAddingCount++;
      afterVals.push_back(result);
    }

    void before_RunningSum_add(
        BeforeParams beforeParams, int64_t val) override {
      EXPECT_EQ(beforeParams.decoratorData.get(onRequestData_), nullptr);
      beforeParams.decoratorData.put(onRequestData_, true);
      before_addCount++;
      beforeVals.push_back(val);
    }

    void after_RunningSum_add(AfterParams afterParams, int64_t val) override {
      const auto* onRequestData = afterParams.decoratorData.get(onRequestData_);
      EXPECT_NE(onRequestData, nullptr);
      EXPECT_TRUE(*onRequestData);
      afterParams.decoratorData.put(onResponseData_, true);
      after_addCount++;
      afterVals.push_back(val);
    }

    std::vector<int64_t> beforeVals;
    std::size_t before_startAddingCount = 0;
    std::size_t before_addCount = 0;

    std::vector<int64_t> afterVals;
    std::size_t after_startAddingCount = 0;
    std::size_t after_addCount = 0;

    server::DecoratorDataHandle<bool> onRequestData_ =
        server::DecoratorDataHandle<bool>::uninitialized();
    server::DecoratorDataHandle<bool> onResponseData_ =
        server::DecoratorDataHandle<bool>::uninitialized();
  };

  // This interceptor just checks if the onRequest / onResopnse data is set.
  struct Interceptor : public ServiceInterceptor<folly::Unit, folly::Unit> {
    std::string getName() const override { return "Interceptor"; }

    folly::coro::Task<void> co_onStartServing(InitParams params) override {
      onRequestData_ =
          params.decoratorDataHandleFactory->makeHandleForKey(kOnRequestData);
      onResponseData_ =
          params.decoratorDataHandleFactory->makeHandleForKey(kOnResponseData);
      co_return;
    }

    folly::coro::Task<std::optional<folly::Unit>> onRequest(
        folly::Unit*, RequestInfo info) override {
      const auto* onRequestData = info.decoratorData->get(onRequestData_);
      EXPECT_NE(onRequestData, nullptr);
      EXPECT_TRUE(*onRequestData);
      co_return std::nullopt;
    }

    folly::coro::Task<void> onResponse(
        folly::Unit*, folly::Unit*, ResponseInfo info) override {
      const auto* onResponseData = info.decoratorData->get(onResponseData_);
      EXPECT_NE(onResponseData, nullptr);
      EXPECT_TRUE(*onResponseData);
      co_return;
    }

    server::DecoratorDataHandle<bool> onRequestData_ =
        server::DecoratorDataHandle<bool>::uninitialized();
    server::DecoratorDataHandle<bool> onResponseData_ =
        server::DecoratorDataHandle<bool>::uninitialized();
  };

  struct Module : public ServerModule {
    explicit Module() { interceptors_ = {std::make_shared<Interceptor>()}; }

    std::string getName() const override { return "Module"; }

    std::vector<std::shared_ptr<ServiceInterceptorBase>>
    getServiceInterceptors() override {
      return interceptors_;
    }

    std::vector<std::shared_ptr<ServiceInterceptorBase>> interceptors_;
  };

  class Handler : public ServiceHandler<ReturnsInteractionTest> {
   public:
    class RunningSum : public RunningSumIf {
     public:
      RunningSum(
          int64_t initValue,
          const server::DecoratorDataHandle<bool>& onRequestData)
          : sum_{initValue}, onRequestData_{onRequestData} {}

      folly::coro::Task<int64_t> co_add(
          RequestParams params, int64_t val) override {
        const auto* onRequestData =
            params.getRequestContext()->getDecoratorData().get(onRequestData_);
        EXPECT_NE(onRequestData, nullptr);
        EXPECT_TRUE(*onRequestData);
        sum_ += val;
        co_return sum_;
      }

     private:
      int64_t sum_;
      const server::DecoratorDataHandle<bool>& onRequestData_;
    };

    folly::coro::Task<void> co_onBeforeStartServing(
        BeforeStartServingParams params) override {
      onRequestData_ =
          params.decoratorDataHandleFactory->makeHandleForKey(kOnRequestData);
      co_return;
    }

    folly::coro::Task<TileAndResponse<RunningSumIf, int64_t>> co_startAdding(
        int64_t initValue) override {
      co_return TileAndResponse<RunningSumIf, int64_t>{
          std::make_unique<RunningSum>(initValue, onRequestData_), initValue};
    }

   private:
    server::DecoratorDataHandle<bool> onRequestData_ =
        server::DecoratorDataHandle<bool>::uninitialized();
  };

  auto handler = std::make_shared<Handler>();
  auto decorator = std::make_shared<Decorator>();
  {
    ServiceMethodDecoratorList<ReturnsInteractionTest> decorators;
    // Copy so that we can reference decorator later
    decorators.push_back(decorator);
    apache::thrift::decorate(*handler, std::move(decorators));
  }
  auto runner = makeServer(handler, [&](ThriftServer& server) {
    server.addModule(std::make_unique<Module>());
  });

  auto client = makeClient<Client<ReturnsInteractionTest>>(*runner);

  auto [runningSum, _] = co_await client->co_startAdding(0);
  EXPECT_EQ(decorator->before_startAddingCount, 1);
  EXPECT_EQ(decorator->after_startAddingCount, 1);
  EXPECT_THAT(decorator->beforeVals, ElementsAre(0));
  EXPECT_THAT(decorator->afterVals, ElementsAre(0));
  EXPECT_EQ(decorator->before_addCount, 0);
  EXPECT_EQ(decorator->after_addCount, 0);

  co_await runningSum.co_add(5);
  EXPECT_EQ(decorator->before_startAddingCount, 1);
  EXPECT_EQ(decorator->after_startAddingCount, 1);
  EXPECT_THAT(decorator->beforeVals, ElementsAre(0, 5));
  EXPECT_THAT(decorator->afterVals, ElementsAre(0, 5));
  EXPECT_EQ(decorator->before_addCount, 1);
  EXPECT_EQ(decorator->after_addCount, 1);

  co_await runningSum.co_add(10);
  EXPECT_EQ(decorator->before_startAddingCount, 1);
  EXPECT_EQ(decorator->after_startAddingCount, 1);
  EXPECT_THAT(decorator->beforeVals, ElementsAre(0, 5, 10));
  EXPECT_THAT(decorator->afterVals, ElementsAre(0, 5, 15));
  EXPECT_EQ(decorator->before_addCount, 2);
  EXPECT_EQ(decorator->after_addCount, 2);
}

CO_TEST_P(ServiceMethodDecoratorTestP, InheritedInteractions) {
  if (transportType() != TransportType::ROCKET) {
    // only rocket supports interactions
    co_return;
  }
  struct Decorator : public ServiceMethodDecorator<InheritsInteractionTest> {
    std::string_view getName() const override { return "Decorator"; }

    void onBeforeStartServing(BeforeStartServingParams params) override {
      onRequestData_ =
          params.decoratorDataHandleFactory->makeHandleForKey(kOnRequestData);
      onResponseData_ =
          params.decoratorDataHandleFactory->makeHandleForKey(kOnResponseData);
    }

    void before_createPerformedInteraction(BeforeParams beforeParams) override {
      EXPECT_EQ(beforeParams.decoratorData.get(onRequestData_), nullptr);
      beforeParams.decoratorData.put(onRequestData_, true);
    }

    void after_createPerformedInteraction(AfterParams afterParams) override {
      const auto* onRequestData = afterParams.decoratorData.get(onRequestData_);
      EXPECT_NE(onRequestData, nullptr);
      EXPECT_TRUE(*onRequestData);
      afterParams.decoratorData.put(onResponseData_, true);
    }

    void before_PerformedInteraction_perform(
        BeforeParams beforeParams) override {
      EXPECT_EQ(beforeParams.decoratorData.get(onRequestData_), nullptr);
      beforeParams.decoratorData.put(onRequestData_, true);
    }

    void after_PerformedInteraction_perform(AfterParams afterParams) override {
      const auto* onRequestData = afterParams.decoratorData.get(onRequestData_);
      EXPECT_NE(onRequestData, nullptr);
      EXPECT_TRUE(*onRequestData);
      afterParams.decoratorData.put(onResponseData_, true);
    }

    server::DecoratorDataHandle<bool> onRequestData_ =
        server::DecoratorDataHandle<bool>::uninitialized();
    server::DecoratorDataHandle<bool> onResponseData_ =
        server::DecoratorDataHandle<bool>::uninitialized();
  };

  struct Interceptor : public ServiceInterceptor<folly::Unit, folly::Unit> {
    std::string getName() const override { return "Interceptor"; }

    folly::coro::Task<void> co_onStartServing(InitParams params) override {
      onRequestData_ =
          params.decoratorDataHandleFactory->makeHandleForKey(kOnRequestData);
      onResponseData_ =
          params.decoratorDataHandleFactory->makeHandleForKey(kOnResponseData);
      co_return;
    }

    folly::coro::Task<std::optional<folly::Unit>> onRequest(
        folly::Unit*, RequestInfo info) override {
      const auto* onRequestData = info.decoratorData->get(onRequestData_);
      EXPECT_NE(onRequestData, nullptr);
      EXPECT_TRUE(*onRequestData);
      co_return std::nullopt;
    }

    folly::coro::Task<void> onResponse(
        folly::Unit*, folly::Unit*, ResponseInfo info) override {
      const auto* onResponseData = info.decoratorData->get(onResponseData_);
      EXPECT_NE(onResponseData, nullptr);
      EXPECT_TRUE(*onResponseData);
      co_return;
    }

    server::DecoratorDataHandle<bool> onRequestData_ =
        server::DecoratorDataHandle<bool>::uninitialized();
    server::DecoratorDataHandle<bool> onResponseData_ =
        server::DecoratorDataHandle<bool>::uninitialized();
  };

  struct Module : public ServerModule {
    explicit Module() { interceptors_ = {std::make_shared<Interceptor>()}; }

    std::string getName() const override { return "Module"; }

    std::vector<std::shared_ptr<ServiceInterceptorBase>>
    getServiceInterceptors() override {
      return interceptors_;
    }

    std::vector<std::shared_ptr<ServiceInterceptorBase>> interceptors_;
  };

  class Handler : public ServiceHandler<InheritsInteractionTest> {
   public:
    class PerformedInteraction : public PerformedInteractionIf {
     public:
      PerformedInteraction(
          const server::DecoratorDataHandle<bool>& onRequestData)
          : onRequestData_{onRequestData} {}

      folly::coro::Task<void> co_perform(RequestParams params) override {
        const auto* onRequestData =
            params.getRequestContext()->getDecoratorData().get(onRequestData_);
        EXPECT_NE(onRequestData, nullptr);
        EXPECT_TRUE(*onRequestData);
        co_return;
      }

      const server::DecoratorDataHandle<bool>& onRequestData_;
    };

    folly::coro::Task<void> co_onBeforeStartServing(
        BeforeStartServingParams params) override {
      onRequestData_ =
          params.decoratorDataHandleFactory->makeHandleForKey(kOnRequestData);
      co_return;
    }

   private:
    std::unique_ptr<PerformedInteractionIf> createPerformedInteraction()
        override {
      return std::make_unique<PerformedInteraction>(onRequestData_);
    }

    server::DecoratorDataHandle<bool> onRequestData_ =
        server::DecoratorDataHandle<bool>::uninitialized();
  };

  auto handler = std::make_shared<Handler>();
  {
    auto decorator = std::make_shared<Decorator>();
    ServiceMethodDecoratorList<InheritsInteractionTest> decorators;
    decorators.push_back(std::move(decorator));
    apache::thrift::decorate(*handler, std::move(decorators));
  }
  auto runner = makeServer(handler, [&](ThriftServer& server) {
    server.addModule(std::make_unique<Module>());
  });

  auto client = makeClient<Client<PerformInteractionTest>>(*runner);
  auto performsInteraction = client->createPerformedInteraction();
  co_await performsInteraction.co_perform();
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
