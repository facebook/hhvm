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

enum class ClientCallbackKind { CORO, SYNC, SEMIFUTURE, FUTURE };

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

  folly::coro::Task<apache::thrift::TileAndResponse<
      SampleInteractionIf,
      std::unique_ptr<std::string>>>
  co_createInteractionAndEcho(std::unique_ptr<::std::string> str) override {
    class SampleInteractionImpl : public SampleInteractionIf {
      folly::coro::Task<std::unique_ptr<std::string>> co_echo(
          std::unique_ptr<std::string> str) override {
        co_return std::move(str);
      }
    };
    co_return {std::make_unique<SampleInteractionImpl>(), std::move(str)};
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
  virtual folly::coro::Task<
      apache::thrift::Client<test::ClientInterceptorTest>::SampleInteraction>
  createInteraction() = 0;
  virtual folly::coro::Task<std::pair<
      apache::thrift::Client<test::ClientInterceptorTest>::SampleInteraction,
      std::string>>
  createInteractionAndEcho(std::string str) = 0;

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
  folly::coro::Task<
      apache::thrift::Client<test::ClientInterceptorTest>::SampleInteraction>
  createInteraction() override {
    co_return co_await client_->co_createInteraction();
  }
  folly::coro::Task<std::pair<
      apache::thrift::Client<test::ClientInterceptorTest>::SampleInteraction,
      std::string>>
  createInteractionAndEcho(std::string str) override {
    co_return co_await client_->co_createInteractionAndEcho(std::move(str));
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
  folly::coro::Task<
      apache::thrift::Client<test::ClientInterceptorTest>::SampleInteraction>
  createInteraction() override {
    co_return client_->sync_createInteraction();
  }
  folly::coro::Task<std::pair<
      apache::thrift::Client<test::ClientInterceptorTest>::SampleInteraction,
      std::string>>
  createInteractionAndEcho(std::string str) override {
    co_return client_->sync_createInteractionAndEcho(std::move(str));
  }
};

class SemiFutureClientInterface : public ClientInterface {
 public:
  using ClientInterface::ClientInterface;

  folly::coro::Task<std::string> echo(std::string str) override {
    co_return co_await client_->semifuture_echo(str);
  }
  folly::coro::Task<void> noop() override {
    co_await client_->semifuture_noop();
  }
  folly::coro::Task<
      apache::thrift::Client<test::ClientInterceptorTest>::SampleInteraction>
  createInteraction() override {
    co_return co_await client_->semifuture_createInteraction();
  }
  folly::coro::Task<std::pair<
      apache::thrift::Client<test::ClientInterceptorTest>::SampleInteraction,
      std::string>>
  createInteractionAndEcho(std::string str) override {
    co_return co_await client_->semifuture_createInteractionAndEcho(
        std::move(str));
  }
};

class FutureClientInterface : public ClientInterface {
 public:
  using ClientInterface::ClientInterface;

  folly::coro::Task<std::string> echo(std::string str) override {
    co_return co_await client_->future_echo(str);
  }
  folly::coro::Task<void> noop() override { co_await client_->future_noop(); }
  folly::coro::Task<
      apache::thrift::Client<test::ClientInterceptorTest>::SampleInteraction>
  createInteraction() override {
    throw std::logic_error("future_* functions do not support interactions");
  }
  folly::coro::Task<std::pair<
      apache::thrift::Client<test::ClientInterceptorTest>::SampleInteraction,
      std::string>>
  createInteractionAndEcho(std::string) override {
    throw std::logic_error("future_* functions do not support interactions");
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
            folly::getGlobalIOExecutor().get(), // for future_ prefix-ed methods
            channelFor(transportType()))
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
      case ClientCallbackKind::SEMIFUTURE:
        return std::make_unique<SemiFutureClientInterface>(std::move(client));
      case ClientCallbackKind::FUTURE:
        return std::make_unique<FutureClientInterface>(std::move(client));
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

  std::optional<RequestState> onRequest(RequestInfo) override {
    onRequestCount++;
    return 1;
  }

  void onResponse(RequestState* requestState, ResponseInfo) override {
    onResponseCount += *requestState;
  }

  int onRequestCount = 0;
  int onResponseCount = 0;
};

class ClientInterceptorThatThrowsOnRequest
    : public ClientInterceptorCountWithRequestState {
 public:
  using ClientInterceptorCountWithRequestState::
      ClientInterceptorCountWithRequestState;

  std::optional<RequestState> onRequest(RequestInfo requestInfo) override {
    ClientInterceptorCountWithRequestState::onRequest(std::move(requestInfo));
    throw std::runtime_error("Oh no!");
  }
};

class ClientInterceptorThatThrowsOnResponse
    : public ClientInterceptorCountWithRequestState {
 public:
  using ClientInterceptorCountWithRequestState::
      ClientInterceptorCountWithRequestState;

  void onResponse(
      RequestState* requestState, ResponseInfo responseInfo) override {
    ClientInterceptorCountWithRequestState::onResponse(
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

CO_TEST_P(ClientInterceptorTestP, IterationOrder) {
  int seq = 1;

  class ClientInterceptorRecordingExecutionSequence
      : public NamedClientInterceptor<folly::Unit> {
   public:
    using RequestState = folly::Unit;

    ClientInterceptorRecordingExecutionSequence(std::string name, int& seq)
        : NamedClientInterceptor(std::move(name)), seq_(seq) {}

    std::optional<RequestState> onRequest(RequestInfo) override {
      onRequestSeq = seq_++;
      return std::nullopt;
    }

    void onResponse(RequestState*, ResponseInfo) override {
      onResponseSeq = seq_++;
    }

    int onRequestSeq = 0;
    int onResponseSeq = 0;

   private:
    int& seq_;
  };

  auto interceptor1 =
      std::make_shared<ClientInterceptorRecordingExecutionSequence>(
          "Interceptor1", seq);
  auto interceptor2 =
      std::make_shared<ClientInterceptorRecordingExecutionSequence>(
          "Interceptor2", seq);
  auto client = makeClient(makeInterceptorsList(interceptor1, interceptor2));

  co_await client->noop();

  EXPECT_EQ(interceptor1->onRequestSeq, 1);
  EXPECT_EQ(interceptor2->onRequestSeq, 2);
  EXPECT_EQ(interceptor2->onResponseSeq, 3);
  EXPECT_EQ(interceptor1->onResponseSeq, 4);
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
          EXPECT_EQ(ex.causes()[0].sourceInterceptorName, "Interceptor3");
          EXPECT_EQ(ex.causes()[1].sourceInterceptorName, "Interceptor1");
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

CO_TEST_P(
    ClientInterceptorTestP, OnResponseExceptionSwallowsApplicationException) {
  auto interceptor =
      std::make_shared<ClientInterceptorThatThrowsOnResponse>("Interceptor1");
  auto client = makeClient(makeInterceptorsList(interceptor));

  EXPECT_THROW(
      {
        try {
          co_await client->echo("throw");
        } catch (const apache::thrift::ClientInterceptorException& ex) {
          EXPECT_THAT(std::string(ex.what()), HasSubstr("[Interceptor1]"));
          throw;
        }
      },
      apache::thrift::ClientInterceptorException);
  EXPECT_EQ(interceptor->onRequestCount, 1);
  EXPECT_EQ(interceptor->onResponseCount, 1);
}

CO_TEST_P(ClientInterceptorTestP, NonTrivialRequestState) {
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

  struct ClientInterceptorNonTrivialRequestState
      : public NamedClientInterceptor<RequestState> {
   public:
    ClientInterceptorNonTrivialRequestState(std::string name, Counts& counts)
        : NamedClientInterceptor(std::move(name)), counts_(counts) {}

    std::optional<RequestState> onRequest(RequestInfo) override {
      return RequestState(counts_);
    }
    void onResponse(RequestState*, ResponseInfo) override {}

   private:
    Counts& counts_;
  };

  auto interceptor1 = std::make_shared<ClientInterceptorNonTrivialRequestState>(
      "Interceptor1", counts);
  auto interceptor2 = std::make_shared<ClientInterceptorNonTrivialRequestState>(
      "Interceptor2", counts);
  auto client = makeClient(makeInterceptorsList(interceptor1, interceptor2));

  co_await client->noop();

  EXPECT_EQ(counts.construct, 2);
  EXPECT_EQ(counts.destruct, 2);
}

CO_TEST_P(ClientInterceptorTestP, BasicInteraction) {
  if (transportType() != TransportType::ROCKET) {
    // only rocket supports interactions
    co_return;
  }
  if (clientCallbackType() == ClientCallbackKind::FUTURE) {
    // future_* functions do not support interactions
    co_return;
  }

  {
    auto interceptor1 =
        std::make_shared<ClientInterceptorCountWithRequestState>(
            "Interceptor1");
    auto interceptor2 =
        std::make_shared<ClientInterceptorCountWithRequestState>(
            "Interceptor2");
    auto client = makeClient(makeInterceptorsList(interceptor1, interceptor2));

    {
      auto interaction = co_await client->createInteraction();
      for (auto& interceptor : {interceptor1, interceptor2}) {
        EXPECT_EQ(interceptor->onRequestCount, 1);
        EXPECT_EQ(interceptor->onResponseCount, 1);
      }

      co_await interaction.co_echo("");
      for (auto& interceptor : {interceptor1, interceptor2}) {
        EXPECT_EQ(interceptor->onRequestCount, 2);
        EXPECT_EQ(interceptor->onResponseCount, 2);
      }

      co_await client->echo("");
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

  // With initial response
  {
    auto interceptor1 =
        std::make_shared<ClientInterceptorCountWithRequestState>(
            "Interceptor1");
    auto interceptor2 =
        std::make_shared<ClientInterceptorCountWithRequestState>(
            "Interceptor2");
    auto client = makeClient(makeInterceptorsList(interceptor1, interceptor2));
    {
      auto [interaction, response] =
          co_await client->createInteractionAndEcho("hello");
      for (auto& interceptor : {interceptor1, interceptor2}) {
        EXPECT_EQ(interceptor->onRequestCount, 1);
        EXPECT_EQ(interceptor->onResponseCount, 1);
      }

      co_await interaction.co_echo("");
      for (auto& interceptor : {interceptor1, interceptor2}) {
        EXPECT_EQ(interceptor->onRequestCount, 2);
        EXPECT_EQ(interceptor->onResponseCount, 2);
      }

      co_await client->echo("");
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
}

INSTANTIATE_TEST_SUITE_P(
    ClientInterceptorTestP,
    ClientInterceptorTestP,
    Combine(
        Values(
            TransportType::HEADER, TransportType::ROCKET, TransportType::HTTP2),
        Values(
            ClientCallbackKind::CORO,
            ClientCallbackKind::SYNC,
            ClientCallbackKind::SEMIFUTURE,
            ClientCallbackKind::FUTURE)),
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
          case ClientCallbackKind::SEMIFUTURE:
            return "SEMIFUTURE";
          case ClientCallbackKind::FUTURE:
            return "FUTURE";
          default:
            throw std::logic_error{"Unreachable!"};
        }
      };
      return fmt::format(
          "{}___{}",
          transportType(std::get<0>(info.param)),
          clientCallbackType(std::get<1>(info.param)));
    });
