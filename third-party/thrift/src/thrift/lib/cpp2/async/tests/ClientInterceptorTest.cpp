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

#include <folly/experimental/coro/GtestHelpers.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fmt/core.h>

#include <thrift/lib/cpp2/async/ClientInterceptor.h>
#include <thrift/lib/cpp2/async/HTTPClientChannel.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/async/tests/gen-cpp2/ClientInterceptor_clients.h>
#include <thrift/lib/cpp2/async/tests/gen-cpp2/ClientInterceptor_handlers.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/transport/http2/common/HTTP2RoutingHandler.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace apache::thrift;
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

enum class ClientCallbackKind { CORO, SYNC };

struct TestHandler
    : apache::thrift::ServiceHandler<test::ClientInterceptorTest> {
  folly::coro::Task<void> co_noop() override { co_return; }

  folly::coro::Task<std::unique_ptr<std::string>> co_echo(
      std::unique_ptr<std::string> str) override {
    if (*str == "throw") {
      throw std::runtime_error("You asked for it!");
    }
    co_return std::move(str);
  }
};

class ClientInterface {
 public:
  explicit ClientInterface(
      std::unique_ptr<apache::thrift::Client<test::ClientInterceptorTest>>
          client)
      : client_(std::move(client)) {}

  virtual ~ClientInterface() = default;

  virtual folly::coro::Task<std::string> echo(std::string str) = 0;
  virtual folly::coro::Task<void> noop() = 0;

 protected:
  std::unique_ptr<apache::thrift::Client<test::ClientInterceptorTest>> client_;
};

class CoroClientInterface : public ClientInterface {
 public:
  using ClientInterface::ClientInterface;

  folly::coro::Task<std::string> echo(std::string str) override {
    co_return co_await client_->co_echo(std::move(str));
  }
  folly::coro::Task<void> noop() override {
    co_await client_->co_noop();
    co_return;
  }
};

class SyncClientInterface : public ClientInterface {
 public:
  using ClientInterface::ClientInterface;

  folly::coro::Task<std::string> echo(std::string str) override {
    std::string ret;
    client_->sync_echo(ret, std::move(str));
    co_return ret;
  }
  folly::coro::Task<void> noop() override {
    client_->sync_noop();
    co_return;
  }
};

class ClientInterceptorTestP
    : public ::testing::TestWithParam<
          std::tuple<TransportType, ClientCallbackKind>> {
 public:
  TransportType transportType() const { return std::get<0>(GetParam()); }
  ClientCallbackKind clientCallbackType() const {
    return std::get<1>(GetParam());
  }

 private:
  void SetUp() override {
    runner = std::make_unique<ScopedServerInterfaceThread>(
        std::make_shared<TestHandler>());
    if (transportType() == TransportType::HTTP2) {
      auto& thriftServer = runner->getThriftServer();
      thriftServer.addRoutingHandler(createHTTP2RoutingHandler(thriftServer));
    }
  }
  std::unique_ptr<ScopedServerInterfaceThread> runner;

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

  std::shared_ptr<RequestChannel> makeChannel() {
    return runner
        ->newClient<apache::thrift::Client<test::ClientInterceptorTest>>(
            nullptr, channelFor(transportType()))
        ->getChannelShared();
  }

 public:
  std::unique_ptr<ClientInterface> makeClient(
      std::shared_ptr<std::vector<std::shared_ptr<ClientInterceptorBase>>>
          interceptors) {
    auto client =
        std::make_unique<apache::thrift::Client<test::ClientInterceptorTest>>(
            makeChannel(), std::move(interceptors));
    switch (clientCallbackType()) {
      case ClientCallbackKind::CORO:
        return std::make_unique<CoroClientInterface>(std::move(client));
      case ClientCallbackKind::SYNC:
        return std::make_unique<SyncClientInterface>(std::move(client));
      default:
        throw std::logic_error{"Unknown client callback type!"};
    }
  }
};

template <class... InterceptorPtrs>
std::shared_ptr<std::vector<std::shared_ptr<ClientInterceptorBase>>>
makeInterceptorsList(InterceptorPtrs&&... interceptors) {
  auto list =
      std::make_shared<std::vector<std::shared_ptr<ClientInterceptorBase>>>();
  (list->emplace_back(std::forward<InterceptorPtrs>(interceptors)), ...);
  return list;
}

template <class RequestState>
struct NamedClientInterceptor : public ClientInterceptor<RequestState> {
  explicit NamedClientInterceptor(std::string name) : name_(std::move(name)) {}

  std::string getName() const override { return name_; }

 private:
  std::string name_;
};

class ClientInterceptorCountWithRequestState
    : public NamedClientInterceptor<int> {
 public:
  using RequestState = int;

  using NamedClientInterceptor::NamedClientInterceptor;

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

class ClientInterceptorThatThrowsOnRequest
    : public ClientInterceptorCountWithRequestState {
 public:
  using ClientInterceptorCountWithRequestState::
      ClientInterceptorCountWithRequestState;

  folly::coro::Task<std::optional<RequestState>> onRequest(
      RequestInfo requestInfo) override {
    co_await ClientInterceptorCountWithRequestState::onRequest(
        std::move(requestInfo));
    throw std::runtime_error("Oh no!");
  }
};

class ClientInterceptorThatThrowsOnResponse
    : public ClientInterceptorCountWithRequestState {
 public:
  using ClientInterceptorCountWithRequestState::
      ClientInterceptorCountWithRequestState;

  folly::coro::Task<void> onResponse(
      RequestState* requestState, ResponseInfo responseInfo) override {
    co_await ClientInterceptorCountWithRequestState::onResponse(
        requestState, std::move(responseInfo));
    throw std::runtime_error("Oh no!");
  }
};

} // namespace

CO_TEST_P(ClientInterceptorTestP, Basic) {
  auto interceptor =
      std::make_shared<ClientInterceptorCountWithRequestState>("Interceptor1");
  auto client = makeClient(makeInterceptorsList(interceptor));

  co_await client->echo("foo");
  EXPECT_EQ(interceptor->onRequestCount, 1);
  EXPECT_EQ(interceptor->onResponseCount, 1);

  co_await client->noop();
  EXPECT_EQ(interceptor->onRequestCount, 2);
  EXPECT_EQ(interceptor->onResponseCount, 2);
}

CO_TEST_P(ClientInterceptorTestP, OnRequestException) {
  auto interceptor1 =
      std::make_shared<ClientInterceptorThatThrowsOnRequest>("Interceptor1");
  auto interceptor2 =
      std::make_shared<ClientInterceptorCountWithRequestState>("Interceptor2");
  auto interceptor3 =
      std::make_shared<ClientInterceptorThatThrowsOnRequest>("Interceptor3");
  auto client = makeClient(
      makeInterceptorsList(interceptor1, interceptor2, interceptor3));

  EXPECT_THROW(
      {
        try {
          co_await client->noop();
        } catch (const ClientInterceptorException& ex) {
          EXPECT_EQ(ex.causes().size(), 2);
          EXPECT_EQ(ex.causes()[0].sourceInterceptorName, "Interceptor1");
          EXPECT_EQ(ex.causes()[1].sourceInterceptorName, "Interceptor3");
          EXPECT_THAT(ex.what(), HasSubstr("[Interceptor1]"));
          EXPECT_THAT(ex.what(), Not(HasSubstr("Interceptor2")));
          EXPECT_THAT(ex.what(), HasSubstr("[Interceptor3]"));
          EXPECT_THAT(ex.what(), HasSubstr("ClientInterceptor::onRequest"));
          throw;
        }
      },
      ClientInterceptorException);
  EXPECT_EQ(interceptor1->onRequestCount, 1);
  EXPECT_EQ(interceptor2->onRequestCount, 1);
  EXPECT_EQ(interceptor3->onRequestCount, 1);
  EXPECT_EQ(interceptor1->onResponseCount, 0);
  EXPECT_EQ(interceptor2->onResponseCount, 0);
  EXPECT_EQ(interceptor3->onResponseCount, 0);
}

CO_TEST_P(ClientInterceptorTestP, OnResponseException) {
  auto interceptor1 =
      std::make_shared<ClientInterceptorThatThrowsOnResponse>("Interceptor1");
  auto interceptor2 =
      std::make_shared<ClientInterceptorCountWithRequestState>("Interceptor2");
  auto interceptor3 =
      std::make_shared<ClientInterceptorThatThrowsOnResponse>("Interceptor3");
  auto client = makeClient(
      makeInterceptorsList(interceptor1, interceptor2, interceptor3));

  EXPECT_THROW(
      {
        try {
          co_await client->noop();
        } catch (const ClientInterceptorException& ex) {
          EXPECT_EQ(ex.causes().size(), 2);
          EXPECT_EQ(ex.causes()[0].sourceInterceptorName, "Interceptor1");
          EXPECT_EQ(ex.causes()[1].sourceInterceptorName, "Interceptor3");
          EXPECT_THAT(ex.what(), HasSubstr("[Interceptor1]"));
          EXPECT_THAT(ex.what(), Not(HasSubstr("Interceptor2")));
          EXPECT_THAT(ex.what(), HasSubstr("[Interceptor3]"));
          EXPECT_THAT(ex.what(), HasSubstr("ClientInterceptor::onResponse"));
          throw;
        }
      },
      ClientInterceptorException);
  EXPECT_EQ(interceptor1->onRequestCount, 1);
  EXPECT_EQ(interceptor2->onRequestCount, 1);
  EXPECT_EQ(interceptor3->onRequestCount, 1);
  EXPECT_EQ(interceptor1->onResponseCount, 1);
  EXPECT_EQ(interceptor2->onResponseCount, 1);
  EXPECT_EQ(interceptor3->onResponseCount, 1);
}

INSTANTIATE_TEST_SUITE_P(
    ClientInterceptorTestP,
    ClientInterceptorTestP,
    Combine(
        Values(
            TransportType::HEADER, TransportType::ROCKET, TransportType::HTTP2),
        Values(ClientCallbackKind::CORO, ClientCallbackKind::SYNC)),
    [](const TestParamInfo<ClientInterceptorTestP::ParamType>& info) {
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
      const auto clientCallbackType =
          [](ClientCallbackKind value) -> std::string_view {
        switch (value) {
          case ClientCallbackKind::CORO:
            return "CORO";
          case ClientCallbackKind::SYNC:
            return "SYNC";
          default:
            throw std::logic_error{"Unreachable!"};
        }
      };
      return fmt::format(
          "{}___{}",
          transportType(std::get<0>(info.param)),
          clientCallbackType(std::get<1>(info.param)));
    });
