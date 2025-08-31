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

class MockMethodDecorator
    : public ServiceMethodDecorator<ServiceMethodDecoratorTest> {
 public:
  MOCK_METHOD(std::string_view, getName, (), (const, override));
  MOCK_METHOD(
      void,
      onBeforeStartServing,
      (ServiceMethodDecoratorBase::BeforeStartServingParams),
      (override));
};

class Handler : public ServiceHandler<ServiceMethodDecoratorTest> {
 public:
  Handler(std::size_t& counter) : counter_{counter} {}

  folly::coro::Task<void> co_onBeforeStartServing(
      BeforeStartServingParams) override {
    counter_++;
    co_return;
  }

  folly::coro::Task<void> co_noop() override { co_return; }

 private:
  std::size_t& counter_;
};

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

CO_TEST_P(ServiceMethodDecoratorTestP, DecoratorDataIsSetup) {
  struct CountUseDecoratorData
      : ServiceMethodDecorator<ServiceMethodDecoratorTest> {
    CountUseDecoratorData(std::size_t& counter) : counter_{counter} {}

    std::string_view getName() const override {
      return "CountUseDecoratorData";
    }

    void onBeforeStartServing(BeforeStartServingParams) override { counter_++; }

    std::size_t& counter_;
  };

  struct CountInterceptor
      : public ServiceInterceptor<folly::Unit, folly::Unit> {
    CountInterceptor(std::size_t& counter) : counter_{counter} {}

    std::string getName() const override { return "CountInterceptor"; }

    folly::coro::Task<void> co_onStartServing(InitParams) override {
      counter_++;
      co_return;
    }

    std::size_t& counter_;
  };

  struct Module : public ServerModule {
    Module(std::size_t& counter) : counter_{counter} {}

    std::string getName() const override { return "TestModule"; }

    std::vector<std::shared_ptr<ServiceInterceptorBase>>
    getServiceInterceptors() override {
      return {std::make_shared<CountInterceptor>(counter_)};
    }

    std::size_t& counter_;
  };

  std::size_t counter{0};
  std::size_t interceptorCounter{0};
  std::size_t handlerCounter{0};
  auto handler = std::make_shared<Handler>(handlerCounter);
  {
    auto decorator = std::make_shared<CountUseDecoratorData>(counter);
    ServiceMethodDecoratorList<ServiceMethodDecoratorTest> decorators;
    decorators.push_back(std::move(decorator));
    apache::thrift::decorate(*handler, std::move(decorators));
  }

  auto runner = makeServer(handler, [&](ThriftServer& server) {
    server.addModule(std::make_unique<Module>(interceptorCounter));
  });
  auto client = makeClient<Client<ServiceMethodDecoratorTest>>(*runner);

  co_await client->co_noop();
  EXPECT_EQ(counter, 1);
  EXPECT_EQ(interceptorCounter, 1);
  EXPECT_EQ(handlerCounter, 1);
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
