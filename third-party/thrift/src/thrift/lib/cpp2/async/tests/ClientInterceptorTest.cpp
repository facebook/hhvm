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
#include <folly/Demangle.h>

#include <folly/coro/GtestHelpers.h>

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

constexpr auto kDummyHeader = "dummy_header";

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
  folly::coro::Task<void> co_noop(
      RequestParams requestParams, bool shouldThrow) override {
    auto* header = requestParams.getRequestContext()->getHeader();
    auto& readHeaders = header->getHeaders();
    if (auto dummyHeader = readHeaders.find(kDummyHeader);
        dummyHeader != readHeaders.end()) {
      header->setHeader(kDummyHeader, dummyHeader->second);
    }

    if (shouldThrow) {
      apache::thrift::test::TestException ex;
      ex.message() = "Exception from noop";
      throw ex;
    }
    co_return;
  }

  folly::coro::Task<std::unique_ptr<std::string>> co_echo(
      std::unique_ptr<std::string> str) override {
    if (*str == "exception") {
      apache::thrift::test::TestException ex;
      ex.message() = "Test exception from server";
      throw ex;
    }
    co_return std::move(str);
  }

  folly::coro::Task<apache::thrift::TileAndResponse<SampleInteractionIf, void>>
  co_createInteraction(bool shouldThrow) override {
    if (shouldThrow) {
      apache::thrift::test::TestException ex;
      ex.message() = "Exception creating interaction";
      throw ex;
    }

    class SampleInteractionImpl : public SampleInteractionIf {
      folly::coro::Task<std::unique_ptr<std::string>> co_echo(
          std::unique_ptr<std::string> str) override {
        if (*str == "exception") {
          apache::thrift::test::TestException ex;
          ex.message() = "Exception from interaction";
          throw ex;
        }
        co_return std::move(str);
      }
    };
    co_return {std::make_unique<SampleInteractionImpl>()};
  }

  folly::coro::Task<apache::thrift::TileAndResponse<
      SampleInteractionIf,
      std::unique_ptr<std::string>>>
  co_createInteractionAndEcho(
      std::unique_ptr<::std::string> str, bool shouldThrow) override {
    if (shouldThrow) {
      apache::thrift::test::TestException ex;
      ex.message() = "Exception creating interaction with echo";
      throw ex;
    }

    class SampleInteractionImpl : public SampleInteractionIf {
      folly::coro::Task<std::unique_ptr<std::string>> co_echo(
          std::unique_ptr<std::string> str) override {
        if (*str == "exception") {
          apache::thrift::test::TestException ex;
          ex.message() = "Exception from interaction";
          throw ex;
        }
        co_return std::move(str);
      }
    };
    co_return {std::make_unique<SampleInteractionImpl>(), std::move(str)};
  }

  folly::coro::Task<std::unique_ptr<std::string>> co_requestArgs(
      std::int32_t,
      std::unique_ptr<std::string>,
      std::unique_ptr<test::RequestArgsStruct>) override {
    co_return std::make_unique<std::string>("return value");
  }

  apache::thrift::ServerStream<std::int32_t> sync_iota(
      std::int32_t start, bool shouldThrow) override {
    if (shouldThrow) {
      apache::thrift::test::TestException ex;
      ex.message() = "Exception from iota";
      throw ex;
    }
    return folly::coro::co_invoke(
        [current =
             start]() mutable -> folly::coro::AsyncGenerator<std::int32_t&&> {
          while (true) {
            co_yield current++;
          }
        });
  }

  apache::thrift::ResponseAndServerStream<std::int32_t, std::int32_t>
  sync_iotaWithResponse(std::int32_t start, bool shouldThrow) override {
    if (shouldThrow) {
      apache::thrift::test::TestException ex;
      ex.message() = "Exception from iotaWithResponse";
      throw ex;
    }
    auto stream = folly::coro::co_invoke(
        [current =
             start]() mutable -> folly::coro::AsyncGenerator<std::int32_t&&> {
          while (true) {
            co_yield current++;
          }
        });
    return {start, std::move(stream)};
  }

  apache::thrift::SinkConsumer<std::int32_t, std::int32_t> dump() override {
    return apache::thrift::SinkConsumer<std::int32_t, std::int32_t>{
        [](folly::coro::AsyncGenerator<std::int32_t&&> gen)
            -> folly::coro::Task<std::int32_t> {
          std::int32_t count = 0;
          while (auto chunk = co_await gen.next()) {
            count++;
          }
          co_return count;
        }};
  }

  folly::coro::Task<std::unique_ptr<std::string>> co_headerClientMethod(
      std::unique_ptr<std::string> p_str) override {
    co_return std::move(p_str);
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
  virtual folly::coro::Task<void> noop(bool shouldThrow = false) = 0;
  virtual folly::coro::Task<void> noop(
      RpcOptions&, bool shouldThrow = false) = 0;
  virtual folly::coro::Task<
      apache::thrift::Client<test::ClientInterceptorTest>::SampleInteraction>
  createInteraction(bool shouldThrow = false) = 0;
  virtual folly::coro::Task<std::pair<
      apache::thrift::Client<test::ClientInterceptorTest>::SampleInteraction,
      std::string>>
  createInteractionAndEcho(std::string str, bool shouldThrow = false) = 0;

  virtual folly::coro::Task<std::string> requestArgs(
      std::int32_t arg1, std::string arg2, test::RequestArgsStruct arg3) = 0;

  virtual folly::coro::Task<folly::coro::AsyncGenerator<std::int32_t&&>> iota(
      std::int32_t start, bool shouldThrow = false) = 0;
  virtual folly::coro::Task<
      std::pair<std::int32_t, folly::coro::AsyncGenerator<std::int32_t&&>>>
  iotaWithResponse(std::int32_t start, bool shouldThrow = false) = 0;

  virtual folly::coro::Task<
      apache::thrift::ClientSink<std::int32_t, std::int32_t>>
  dump() = 0;

  virtual folly::coro::Task<void> fireAndForget(std::int32_t value) = 0;

  virtual folly::coro::Task<std::pair<
      std::string,
      std::unique_ptr<apache::thrift::transport::THeader>>>
  headerClientMethod(std::string str) = 0;

 protected:
  std::unique_ptr<apache::thrift::Client<test::ClientInterceptorTest>> client_;
};

class CoroClientInterface : public ClientInterface {
 public:
  using ClientInterface::ClientInterface;

  folly::coro::Task<std::string> echo(std::string str) override {
    co_return co_await client_->co_echo(std::move(str));
  }
  folly::coro::Task<void> noop(bool shouldThrow = false) override {
    co_await client_->co_noop(shouldThrow);
    co_return;
  }
  folly::coro::Task<void> noop(
      RpcOptions& rpcOptions, bool shouldThrow = false) override {
    co_await client_->co_noop(rpcOptions, shouldThrow);
    co_return;
  }
  folly::coro::Task<
      apache::thrift::Client<test::ClientInterceptorTest>::SampleInteraction>
  createInteraction(bool shouldThrow = false) override {
    co_return co_await client_->co_createInteraction(shouldThrow);
  }
  folly::coro::Task<std::pair<
      apache::thrift::Client<test::ClientInterceptorTest>::SampleInteraction,
      std::string>>
  createInteractionAndEcho(std::string str, bool shouldThrow = false) override {
    co_return co_await client_->co_createInteractionAndEcho(str, shouldThrow);
  }

  folly::coro::Task<std::string> requestArgs(
      std::int32_t arg1,
      std::string arg2,
      test::RequestArgsStruct arg3) override {
    co_return co_await client_->co_requestArgs(
        arg1, std::move(arg2), std::move(arg3));
  }

  folly::coro::Task<folly::coro::AsyncGenerator<std::int32_t&&>> iota(
      std::int32_t start, bool shouldThrow = false) override {
    co_return (co_await client_->co_iota(start, shouldThrow))
        .toAsyncGenerator();
  }
  folly::coro::Task<
      std::pair<std::int32_t, folly::coro::AsyncGenerator<std::int32_t&&>>>
  iotaWithResponse(std::int32_t start, bool shouldThrow = false) override {
    auto [initialResponse, stream] =
        co_await client_->co_iotaWithResponse(start, shouldThrow);
    co_return std::make_pair(
        initialResponse, std::move(stream).toAsyncGenerator());
  }

  folly::coro::Task<apache::thrift::ClientSink<std::int32_t, std::int32_t>>
  dump() override {
    co_return co_await client_->co_dump();
  }

  folly::coro::Task<void> fireAndForget(std::int32_t value) override {
    co_await client_->co_fireAndForget(value);
  }

  folly::coro::Task<std::pair<
      std::string,
      std::unique_ptr<apache::thrift::transport::THeader>>>
  headerClientMethod(std::string) override {
    throw std::logic_error("Should not be called from coro client");
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
  folly::coro::Task<void> noop(bool shouldThrow = false) override {
    client_->sync_noop(shouldThrow);
    co_return;
  }
  folly::coro::Task<void> noop(
      RpcOptions& rpcOptions, bool shouldThrow = false) override {
    client_->sync_noop(rpcOptions, shouldThrow);
    co_return;
  }
  folly::coro::Task<
      apache::thrift::Client<test::ClientInterceptorTest>::SampleInteraction>
  createInteraction(bool shouldThrow = false) override {
    co_return client_->sync_createInteraction(shouldThrow);
  }
  folly::coro::Task<std::pair<
      apache::thrift::Client<test::ClientInterceptorTest>::SampleInteraction,
      std::string>>
  createInteractionAndEcho(std::string str, bool shouldThrow = false) override {
    co_return client_->sync_createInteractionAndEcho(str, shouldThrow);
  }

  folly::coro::Task<std::string> requestArgs(
      std::int32_t arg1,
      std::string arg2,
      test::RequestArgsStruct arg3) override {
    std::string ret;
    client_->sync_requestArgs(ret, arg1, std::move(arg2), std::move(arg3));
    co_return ret;
  }

  folly::coro::Task<folly::coro::AsyncGenerator<std::int32_t&&>> iota(
      std::int32_t start, bool shouldThrow = false) override {
    co_return client_->sync_iota(start, shouldThrow).toAsyncGenerator();
  }
  folly::coro::Task<
      std::pair<std::int32_t, folly::coro::AsyncGenerator<std::int32_t&&>>>
  iotaWithResponse(std::int32_t start, bool shouldThrow = false) override {
    auto [initialResponse, stream] =
        client_->sync_iotaWithResponse(start, shouldThrow);
    co_return std::make_pair(
        initialResponse, std::move(stream).toAsyncGenerator());
  }

  folly::coro::Task<apache::thrift::ClientSink<std::int32_t, std::int32_t>>
  dump() override {
    throw std::logic_error("sync_* functions do not support sinks");
  }

  folly::coro::Task<void> fireAndForget(std::int32_t value) override {
    client_->sync_fireAndForget(value);
    co_return;
  }

  folly::coro::Task<std::pair<
      std::string,
      std::unique_ptr<apache::thrift::transport::THeader>>>
  headerClientMethod(std::string) override {
    throw std::logic_error("Should not be called from sync client");
  }
};

class SemiFutureClientInterface : public ClientInterface {
 public:
  using ClientInterface::ClientInterface;

  folly::coro::Task<std::string> echo(std::string str) override {
    co_return co_await client_->semifuture_echo(str);
  }
  folly::coro::Task<void> noop(bool shouldThrow = false) override {
    co_await client_->semifuture_noop(shouldThrow);
  }
  folly::coro::Task<void> noop(
      RpcOptions& rpcOptions, bool shouldThrow = false) override {
    co_await client_->semifuture_noop(rpcOptions, shouldThrow);
  }
  folly::coro::Task<
      apache::thrift::Client<test::ClientInterceptorTest>::SampleInteraction>
  createInteraction(bool shouldThrow = false) override {
    co_return co_await client_->semifuture_createInteraction(shouldThrow);
  }
  folly::coro::Task<std::pair<
      apache::thrift::Client<test::ClientInterceptorTest>::SampleInteraction,
      std::string>>
  createInteractionAndEcho(std::string str, bool shouldThrow = false) override {
    co_return co_await client_->semifuture_createInteractionAndEcho(
        str, shouldThrow);
  }

  folly::coro::Task<std::string> requestArgs(
      std::int32_t arg1,
      std::string arg2,
      test::RequestArgsStruct arg3) override {
    co_return co_await client_->semifuture_requestArgs(
        arg1, std::move(arg2), std::move(arg3));
  }

  folly::coro::Task<folly::coro::AsyncGenerator<std::int32_t&&>> iota(
      std::int32_t start, bool shouldThrow = false) override {
    co_return (co_await client_->semifuture_iota(start, shouldThrow))
        .toAsyncGenerator();
  }
  folly::coro::Task<
      std::pair<std::int32_t, folly::coro::AsyncGenerator<std::int32_t&&>>>
  iotaWithResponse(std::int32_t start, bool shouldThrow = false) override {
    auto [initialResponse, stream] =
        co_await client_->semifuture_iotaWithResponse(start, shouldThrow);
    co_return std::make_pair(
        initialResponse, std::move(stream).toAsyncGenerator());
  }

  folly::coro::Task<apache::thrift::ClientSink<std::int32_t, std::int32_t>>
  dump() override {
    throw std::logic_error("semifuture_* functions do not support sinks");
  }

  folly::coro::Task<void> fireAndForget(std::int32_t value) override {
    co_await client_->semifuture_fireAndForget(value);
  }

  folly::coro::Task<std::pair<
      std::string,
      std::unique_ptr<apache::thrift::transport::THeader>>>
  headerClientMethod(std::string str) override {
    RpcOptions rpcOptions;
    co_return co_await client_->header_semifuture_headerClientMethod(
        rpcOptions, std::move(str));
  }
};

class FutureClientInterface : public ClientInterface {
 public:
  using ClientInterface::ClientInterface;

  folly::coro::Task<std::string> echo(std::string str) override {
    co_return co_await client_->future_echo(str);
  }
  folly::coro::Task<void> noop(bool shouldThrow = false) override {
    co_await client_->future_noop(shouldThrow);
  }
  folly::coro::Task<void> noop(
      RpcOptions& rpcOptions, bool shouldThrow = false) override {
    co_await client_->future_noop(rpcOptions, shouldThrow);
  }
  folly::coro::Task<
      apache::thrift::Client<test::ClientInterceptorTest>::SampleInteraction>
  createInteraction(bool = false) override {
    throw std::logic_error("future_* functions do not support interactions");
  }
  folly::coro::Task<std::pair<
      apache::thrift::Client<test::ClientInterceptorTest>::SampleInteraction,
      std::string>>
  createInteractionAndEcho(std::string = "", bool = false) override {
    throw std::logic_error("future_* functions do not support interactions");
  }

  folly::coro::Task<std::string> requestArgs(
      std::int32_t arg1,
      std::string arg2,
      test::RequestArgsStruct arg3) override {
    co_return co_await client_->future_requestArgs(
        arg1, std::move(arg2), std::move(arg3));
  }

  folly::coro::Task<folly::coro::AsyncGenerator<std::int32_t&&>> iota(
      std::int32_t, bool = false) override {
    throw std::logic_error("future_* functions do not support streaming");
  }
  folly::coro::Task<
      std::pair<int32_t, folly::coro::AsyncGenerator<std::int32_t&&>>>
  iotaWithResponse(std::int32_t, bool = false) override {
    throw std::logic_error("future_* functions do not support streaming");
  }

  folly::coro::Task<apache::thrift::ClientSink<std::int32_t, std::int32_t>>
  dump() override {
    throw std::logic_error("future_* functions do not support sinks");
  }

  folly::coro::Task<void> fireAndForget(std::int32_t value) override {
    co_await client_->future_fireAndForget(value);
  }

  folly::coro::Task<std::pair<
      std::string,
      std::unique_ptr<apache::thrift::transport::THeader>>>
  headerClientMethod(std::string str) override {
    RpcOptions rpcOptions;
    co_return co_await client_->header_future_headerClientMethod(
        rpcOptions, std::move(str));
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

 public:
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

class TracingClientInterceptor : public NamedClientInterceptor<folly::Unit> {
 public:
  using NamedClientInterceptor::NamedClientInterceptor;

  using Trace = std::pair<std::string, std::string>;
  const std::vector<Trace>& requests() const { return requests_; }
  const std::vector<Trace>& responses() const { return responses_; }

  std::optional<folly::Unit> onRequest(RequestInfo requestInfo) override {
    requests_.emplace_back(
        std::string(requestInfo.serviceName),
        std::string(requestInfo.methodName));
    return folly::unit;
  }

  void onResponse(folly::Unit*, ResponseInfo responseInfo) override {
    responses_.emplace_back(
        std::string(responseInfo.serviceName),
        std::string(responseInfo.methodName));
  }

 private:
  std::vector<Trace> requests_;
  std::vector<Trace> responses_;
};

/**
 * A client interceptor that captures onResponse calls and makes a copy of the
 * data from responseInfo.result.
 *
 * This interceptor can be used to verify that the result field in responseInfo
 * contains the correct type and value for both successful calls and RPC
 * exceptions.
 */
class ClientInterceptorWithResponseValue
    : public NamedClientInterceptor<folly::Unit> {
 public:
  using RequestState = folly::Unit;
  using ResultVariant =
      std::variant<std::monostate, std::string, int32_t, int64_t>;

  explicit ClientInterceptorWithResponseValue(std::string name)
      : NamedClientInterceptor(std::move(name)) {}

  std::optional<RequestState> onRequest(RequestInfo) override {
    return folly::unit;
  }

  void onResponse(RequestState*, ResponseInfo responseInfo) override {
    // Store the method name
    methodName = responseInfo.methodName;

    // Store whether there was an exception
    hadException = responseInfo.result.hasException();

    // Store the result if available
    if (hadException) {
      // Get the exception type
      auto* typeInfo = responseInfo.result.exception().type();

      // Store the type name
      resultType = folly::demangle(*typeInfo);

      // Store the exception value
      exceptionMessageSeen =
          responseInfo.result.exception().what().toStdString();
    } else if (responseInfo.result.hasValue()) {
      // Get the type info from TypeErasedRef
      const std::type_info& typeInfo = responseInfo.result->type();

      // Store the type name
      resultType = folly::demangle(typeInfo);

      // Check the type and extract the value accordingly
      if (std::holds_alternative<std::string>(resultValue)) {
        resultValue = responseInfo.result->value<std::string>();
        hadResult = true;
      } else if (std::holds_alternative<int32_t>(resultValue)) {
        resultValue = responseInfo.result->value<int32_t>();
        hadResult = true;
      } else if (std::holds_alternative<int64_t>(resultValue)) {
        resultValue = responseInfo.result->value<int64_t>();
        hadResult = true;
      } else {
        // If the type is not one of the supported types, ignore it
        resultValue = std::monostate{};
        hadResult = false;
      }
    } else {
      // No result
      resultValue = std::monostate{};
      resultType = "none";
      hadResult = false;
    }
  }

  // Stored information about the response
  std::string methodName;
  bool hadException = false;
  bool hadResult = false;
  std::string resultType;
  std::string exceptionMessageSeen;
  ResultVariant resultValue = std::monostate{};
};

/**
 * A client interceptor that calls a provided callback function when onResponse
 * is invoked.
 *
 * This allows tests to implement custom onResponse behavior inline using
 * lambdas without having to create a new interceptor class for each test case.
 */
class ClientInterceptorOnResponse : public NamedClientInterceptor<folly::Unit> {
 public:
  using RequestState = folly::Unit;
  using OnResponseCallback = std::function<void(ResponseInfo)>;

  /**
   * Create a new ClientInterceptorOnResponse with the given name and callback.
   *
   * @param name The name of the interceptor
   * @param callback The function to call when onResponse is invoked
   */
  ClientInterceptorOnResponse(std::string name, OnResponseCallback callback)
      : NamedClientInterceptor(std::move(name)),
        callback_(std::move(callback)) {}

  std::optional<RequestState> onRequest(RequestInfo) override {
    return folly::unit;
  }

  void onResponse(RequestState*, ResponseInfo responseInfo) override {
    // Call the provided callback with the response info
    if (callback_) {
      callback_(std::move(responseInfo));
    }
  }

 private:
  OnResponseCallback callback_;
};

using ResponseInfo = apache::thrift::ClientInterceptorBase::ResponseInfo;
auto makeExceptionCaptureInterceptor(std::string& message) {
  return std::make_shared<ClientInterceptorOnResponse>(
      "ExceptionCaptureInterceptor", [&message](ResponseInfo responseInfo) {
        if (responseInfo.result.hasException()) {
          const auto& ex = responseInfo.result.exception();
          if (ex.is_compatible_with<apache::thrift::test::TestException>()) {
            message = *ex.get_exception<apache::thrift::test::TestException>()
                           ->message();
          } else {
            ADD_FAILURE() << "Unexpected exception type: " << ex.class_name();
          }
        } else {
          ADD_FAILURE() << "Expected exception, but got none";
        }
      });
}

/**
 * Creates an interceptor that throws an exception in its onResponse method.
 * This is useful for testing that interceptor exceptions override RPC results.
 *
 * @param name The name of the interceptor (used in exception messages)
 * @return A shared pointer to a ClientInterceptorOnResponse that throws an
 * exception
 */
auto makeExceptionGeneratorInterceptor(
    const std::string& name = "ExceptionGeneratorInterceptor") {
  static const std::string kStandardExceptionMessage =
      "Interceptor exception from onResponse";
  return std::make_shared<ClientInterceptorOnResponse>(
      name, [name](const ResponseInfo&) {
        throw std::runtime_error(
            fmt::format("{}: {}", name, kStandardExceptionMessage));
      });
}

/**
 * Creates an interceptor that captures the result of a response.
 * This is useful for verifying that the result field in responseInfo contains
 * the correct type and value for successful calls.
 *
 * @param value A reference to store the captured result value
 * @return A shared pointer to a ClientInterceptorOnResponse that captures the
 * result value
 */
template <typename T>
auto makeResultCaptureInterceptor(T& value) {
  return std::make_shared<ClientInterceptorOnResponse>(
      "ResultCaptureInterceptor", [&value](ResponseInfo responseInfo) {
        if (responseInfo.result.hasValue()) {
          const auto& result = responseInfo.result.value();
          if (result.type() == typeid(T)) {
            value = result.value<T>();
          } else {
            ADD_FAILURE() << "Unexpected result type: "
                          << folly::demangle(result.type());
          }
        } else {
          ADD_FAILURE() << "Expected result, but got none";
        }
      });
}

/**
 * Creates an interceptor that captures the result type of a response and
 * applies a provided function to it.
 * This allows tests to implement custom behavior inline using lambdas.
 *
 * @param func The function to apply to the captured result value
 * @return A shared pointer to a ClientInterceptorOnResponse that captures the
 * result type and applies the function
 */
template <typename T, typename F>
auto makeResultTypeCaptureInterceptor(F&& func) {
  return std::make_shared<ClientInterceptorOnResponse>(
      "ResultCaptureInterceptor",
      [func = std::forward<F>(func)](ResponseInfo responseInfo) {
        if (responseInfo.result.hasValue()) {
          const auto& result = responseInfo.result.value();
          if (result.type() == typeid(T)) {
            func(result.value<T>());
          } else {
            ADD_FAILURE() << "Unexpected result type: "
                          << folly::demangle(result.type());
          }
        } else {
          ADD_FAILURE() << "Expected result, but got none";
        }
      });
}
} // namespace

CO_TEST_P(ClientInterceptorTestP, Basic) {
  int onResponseCount = 0;
  auto interceptor = std::make_shared<ClientInterceptorOnResponse>(
      "Interceptor1", [&](const ResponseInfo&) { onResponseCount++; });
  auto tracer = std::make_shared<TracingClientInterceptor>("Tracer");
  auto client = makeClient(makeInterceptorsList(interceptor, tracer));

  co_await client->echo("foo");

  co_await client->noop();

  using Trace = TracingClientInterceptor::Trace;
  const std::vector<Trace> expectedTrace{
      Trace{"ClientInterceptorTest", "echo"},
      Trace{"ClientInterceptorTest", "noop"},
  };
  EXPECT_THAT(tracer->requests(), ElementsAreArray(expectedTrace));
  EXPECT_THAT(tracer->responses(), ElementsAreArray(expectedTrace));
}

CO_TEST_P(ClientInterceptorTestP, BasicOneWay) {
  auto interceptor =
      std::make_shared<ClientInterceptorCountWithRequestState>("Interceptor1");
  auto tracer = std::make_shared<TracingClientInterceptor>("Tracer");
  auto client = makeClient(makeInterceptorsList(interceptor, tracer));

  co_await client->fireAndForget(0);
  EXPECT_EQ(interceptor->onRequestCount, 1);
  EXPECT_EQ(interceptor->onResponseCount, 0);

  using Trace = TracingClientInterceptor::Trace;
  const std::vector<Trace> expectedTrace{
      Trace{"ClientInterceptorTest", "fireAndForget"},
  };
  EXPECT_THAT(tracer->requests(), ElementsAreArray(expectedTrace));
  EXPECT_THAT(tracer->responses(), IsEmpty());

  // Make sure to wait until requests are processed.
  co_await client->noop();
}

CO_TEST_P(ClientInterceptorTestP, OnRequestException) {
  auto interceptor1 =
      std::make_shared<ClientInterceptorThatThrowsOnRequest>("Interceptor1");
  auto interceptor2 =
      std::make_shared<ClientInterceptorCountWithRequestState>("Interceptor2");
  auto interceptor3 =
      std::make_shared<ClientInterceptorThatThrowsOnRequest>("Interceptor3");
  auto tracer = std::make_shared<TracingClientInterceptor>("Tracer");
  auto client = makeClient(
      makeInterceptorsList(interceptor1, interceptor2, interceptor3, tracer));

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

  using Trace = TracingClientInterceptor::Trace;
  EXPECT_THAT(
      tracer->requests(), ElementsAre(Trace{"ClientInterceptorTest", "noop"}));
  EXPECT_THAT(tracer->responses(), IsEmpty());
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
      co_await client->echo("throw"),
      apache::thrift::ClientInterceptorException);
  EXPECT_EQ(interceptor->onResponseCount, 1);
}

// Test that the result field in responseInfo contains the correct type for
// successful calls
CO_TEST_P(ClientInterceptorTestP, ResponseInfoResultTypeWithSuccess) {
  std::string testValue = "test_value";
  std::string capturedResult;
  auto interceptor = makeResultCaptureInterceptor(capturedResult);
  auto client = makeClient(makeInterceptorsList(interceptor));

  auto result = co_await client->echo(testValue);

  EXPECT_EQ(result, testValue);
  EXPECT_EQ(capturedResult, testValue);
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
    auto tracer = std::make_shared<TracingClientInterceptor>("Tracer");
    auto client =
        makeClient(makeInterceptorsList(interceptor1, interceptor2, tracer));

    {
      auto interaction = co_await client->createInteraction(false);
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

    using Trace = TracingClientInterceptor::Trace;
    const std::vector<Trace> expectedTrace{
        Trace{"ClientInterceptorTest", "createInteraction"},
        Trace{"ClientInterceptorTest", "SampleInteraction.echo"},
        Trace{"ClientInterceptorTest", "echo"},
    };
    EXPECT_THAT(tracer->requests(), ElementsAreArray(expectedTrace));
    EXPECT_THAT(tracer->responses(), ElementsAreArray(expectedTrace));
  }

  // Test interaction creation with initial response
  {
    auto interceptor1 =
        std::make_shared<ClientInterceptorCountWithRequestState>(
            "Interceptor1");
    auto interceptor2 =
        std::make_shared<ClientInterceptorCountWithRequestState>(
            "Interceptor2");
    auto tracer = std::make_shared<TracingClientInterceptor>("Tracer");
    auto client =
        makeClient(makeInterceptorsList(interceptor1, interceptor2, tracer));
    {
      auto [interaction, response] =
          co_await client->createInteractionAndEcho("hello", false);
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

    using Trace = TracingClientInterceptor::Trace;
    const std::vector<Trace> expectedTrace{
        Trace{"ClientInterceptorTest", "createInteractionAndEcho"},
        Trace{"ClientInterceptorTest", "SampleInteraction.echo"},
        Trace{"ClientInterceptorTest", "echo"},
    };
    EXPECT_THAT(tracer->requests(), ElementsAreArray(expectedTrace));
    EXPECT_THAT(tracer->responses(), ElementsAreArray(expectedTrace));
  }
}

CO_TEST_P(ClientInterceptorTestP, RequestArguments) {
  class ClientInterceptorWithRequestArguments
      : public NamedClientInterceptor<folly::Unit> {
   public:
    using RequestState = folly::Unit;

    using NamedClientInterceptor::NamedClientInterceptor;

    std::optional<RequestState> onRequest(RequestInfo requestInfo) override {
      argsCount = requestInfo.arguments.count();
      arg1 = requestInfo.arguments.get(0)->value<std::int32_t>();
      arg2 = requestInfo.arguments.get(1)->value<std::string>();
      arg3 = requestInfo.arguments.get(2)->value<test::RequestArgsStruct>();
      EXPECT_THROW(
          requestInfo.arguments.get(2)->value<std::int32_t>(), std::bad_cast);
      EXPECT_FALSE(requestInfo.arguments.get(3).has_value());
      return std::nullopt;
    }

    std::size_t argsCount = 0;
    std::int32_t arg1 = 0;
    std::string arg2;
    test::RequestArgsStruct arg3;
  };
  auto interceptor = std::make_shared<ClientInterceptorWithRequestArguments>(
      "WithRequestArguments");
  auto client = makeClient(makeInterceptorsList(interceptor));

  test::RequestArgsStruct requestArgs;
  requestArgs.foo() = 1;
  requestArgs.bar() = "hello";
  auto result = co_await client->requestArgs(1, "hello", requestArgs);
  EXPECT_EQ(interceptor->argsCount, 3);
  EXPECT_EQ(interceptor->arg1, 1);
  EXPECT_EQ(interceptor->arg2, "hello");
  EXPECT_EQ(interceptor->arg3, requestArgs);
}

CO_TEST_P(ClientInterceptorTestP, Headers) {
  class ClientInterceptorWithHeaders
      : public NamedClientInterceptor<folly::Unit> {
   public:
    using RequestState = folly::Unit;

    using NamedClientInterceptor::NamedClientInterceptor;

    std::optional<RequestState> onRequest(RequestInfo requestInfo) override {
      onRequestHeader = requestInfo.headers->getWriteHeaders().at(kDummyHeader);
      return std::nullopt;
    }
    void onResponse(RequestState*, ResponseInfo responseInfo) override {
      onResponseHeader = responseInfo.headers->getHeaders().at(kDummyHeader);
    }

    std::string onRequestHeader;
    std::string onResponseHeader;
  };
  auto interceptor =
      std::make_shared<ClientInterceptorWithHeaders>("WithHeaders");
  auto client = makeClient(makeInterceptorsList(interceptor));

  RpcOptions rpcOptions;
  rpcOptions.setWriteHeader(kDummyHeader, "dummy value");
  co_await client->noop(rpcOptions);
  EXPECT_EQ(interceptor->onRequestHeader, "dummy value");
  EXPECT_EQ(interceptor->onResponseHeader, "dummy value");
}

CO_TEST_P(ClientInterceptorTestP, RpcOptions) {
  class ClientInterceptorWithRpcOptions
      : public NamedClientInterceptor<folly::Unit> {
   public:
    using RequestState = folly::Unit;

    using NamedClientInterceptor::NamedClientInterceptor;

    std::optional<RequestState> onRequest(RequestInfo requestInfo) override {
      readContextPropMask = requestInfo.rpcOptions->getContextPropMask();
      return std::nullopt;
    }

    uint8_t readContextPropMask = 0;
  };
  auto interceptor =
      std::make_shared<ClientInterceptorWithRpcOptions>("WithRpcOptions");
  auto client = makeClient(makeInterceptorsList(interceptor));

  RpcOptions rpcOptions;
  rpcOptions.setContextPropMask(5);
  co_await client->noop(rpcOptions);
  EXPECT_EQ(interceptor->readContextPropMask, 5);
}

CO_TEST_P(ClientInterceptorTestP, BasicStream) {
  if (transportType() != TransportType::ROCKET) {
    // only rocket supports streaming
    co_return;
  }
  if (clientCallbackType() == ClientCallbackKind::FUTURE) {
    // future_* functions do not support streaming
    co_return;
  }
  auto interceptor1 =
      std::make_shared<ClientInterceptorCountWithRequestState>("Interceptor1");
  auto interceptor2 =
      std::make_shared<ClientInterceptorCountWithRequestState>("Interceptor2");
  auto tracer = std::make_shared<TracingClientInterceptor>("Tracer");
  auto client =
      makeClient(makeInterceptorsList(interceptor1, interceptor2, tracer));
  {
    auto stream = co_await client->iota(1);
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
  using Trace = TracingClientInterceptor::Trace;
  const std::vector<Trace> expectedTrace{
      Trace{"ClientInterceptorTest", "iota"},
  };
  EXPECT_THAT(tracer->requests(), ElementsAreArray(expectedTrace));
  EXPECT_THAT(tracer->responses(), ElementsAreArray(expectedTrace));
}

CO_TEST_P(ClientInterceptorTestP, BasicSink) {
  if (transportType() != TransportType::ROCKET) {
    // only rocket supports sinks
    co_return;
  }
  if (clientCallbackType() != ClientCallbackKind::CORO) {
    // only co_* functions support sinks
    co_return;
  }
  auto interceptor1 =
      std::make_shared<ClientInterceptorCountWithRequestState>("Interceptor1");
  auto interceptor2 =
      std::make_shared<ClientInterceptorCountWithRequestState>("Interceptor2");
  auto tracer = std::make_shared<TracingClientInterceptor>("Tracer");
  auto client =
      makeClient(makeInterceptorsList(interceptor1, interceptor2, tracer));
  {
    auto sink = co_await client->dump();
    std::size_t response =
        co_await sink.sink([]() -> folly::coro::AsyncGenerator<std::int32_t&&> {
          co_yield 1;
          co_yield 2;
          co_yield 3;
        }());
    EXPECT_EQ(response, 3);
    for (auto& interceptor : {interceptor1, interceptor2}) {
      EXPECT_EQ(interceptor->onRequestCount, 1);
      EXPECT_EQ(interceptor->onResponseCount, 1);
    }
  }
  for (auto& interceptor : {interceptor1, interceptor2}) {
    EXPECT_EQ(interceptor->onRequestCount, 1);
    EXPECT_EQ(interceptor->onResponseCount, 1);
  }
  using Trace = TracingClientInterceptor::Trace;
  const std::vector<Trace> expectedTrace{
      Trace{"ClientInterceptorTest", "dump"},
  };
  EXPECT_THAT(tracer->requests(), ElementsAreArray(expectedTrace));
  EXPECT_THAT(tracer->responses(), ElementsAreArray(expectedTrace));
}

CO_TEST_P(ClientInterceptorTestP, DepreceatedHeaderClientMethods) {
  if (clientCallbackType() != ClientCallbackKind::SEMIFUTURE &&
      clientCallbackType() != ClientCallbackKind::FUTURE) {
    // this test case only testts deprecated header client methods, which are
    // future / semifuture
    co_return;
  }

  auto interceptor =
      std::make_shared<ClientInterceptorCountWithRequestState>("Interceptor1");
  auto client = makeClient(makeInterceptorsList(interceptor));

  co_await client->headerClientMethod("foo");
  EXPECT_EQ(interceptor->onRequestCount, 1);
  EXPECT_EQ(interceptor->onResponseCount, 1);
}

CO_TEST_P(ClientInterceptorTestP, ResponseValueSeen) {
  std::string expectedResponse = "response value";
  std::string actualResponse;
  auto interceptor = makeResultCaptureInterceptor(actualResponse);
  auto client = makeClient(makeInterceptorsList(interceptor));

  auto response = co_await client->echo(expectedResponse);
  EXPECT_EQ(actualResponse, expectedResponse);
  EXPECT_EQ(response, expectedResponse);
}

// Test that the result field in responseInfo contains the correct type for
// the noop RPC in both success and exception cases
CO_TEST_P(ClientInterceptorTestP, NoopWithResponseInfoResult) {
  // Test success case
  {
    auto interceptor = std::make_shared<ClientInterceptorWithResponseValue>(
        "NoopSuccessInterceptor");
    auto client = makeClient(makeInterceptorsList(interceptor));

    co_await client->noop(false);

    // Verify the interceptor received the correct response info
    EXPECT_EQ(interceptor->methodName, "noop");
    EXPECT_FALSE(interceptor->hadException);
    EXPECT_FALSE(interceptor->hadResult);
    EXPECT_EQ(interceptor->resultType, "folly::Unit");
  }

  // Test exception case
  {
    std::string capturedExceptionMessage;
    auto interceptor =
        makeExceptionCaptureInterceptor(capturedExceptionMessage);
    auto client = makeClient(makeInterceptorsList(interceptor));

    EXPECT_THROW(
        co_await client->noop(true), apache::thrift::test::TestException);

    // Verify the interceptor received the correct response info
    EXPECT_EQ(capturedExceptionMessage, "Exception from noop");
  }
}

// Test that the result field in responseInfo contains the correct type for
// RPC exceptions
CO_TEST_P(ClientInterceptorTestP, ResponseInfoResultTypeWithRpcException) {
  std::string capturedExceptionMessage;
  auto interceptor = makeExceptionCaptureInterceptor(capturedExceptionMessage);
  auto client = makeClient(makeInterceptorsList(interceptor));

  EXPECT_THROW(
      auto _ = co_await client->echo("exception"),
      apache::thrift::test::TestException);

  EXPECT_EQ(capturedExceptionMessage, "Test exception from server");
}

// An exception thrown in the interceptor should take priority over the
// exception thrown by the server in being presented to the client first.
CO_TEST_P(ClientInterceptorTestP, InterceptorExceptionPriority) {
  auto interceptor = makeExceptionGeneratorInterceptor("PriorityInterceptor");

  auto client = makeClient(makeInterceptorsList(interceptor));

  EXPECT_THROW(
      auto _ = co_await client->echo("throw"),
      apache::thrift::ClientInterceptorException);
}
// Test for interactions with exceptions
CO_TEST_P(ClientInterceptorTestP, Interaction) {
  if (transportType() != TransportType::ROCKET) {
    // only rocket supports interactions
    co_return;
  }
  if (clientCallbackType() == ClientCallbackKind::FUTURE) {
    // future_* functions do not support interactions
    co_return;
  }

  std::string capturedExceptionMessage;
  auto interceptor = makeExceptionCaptureInterceptor(capturedExceptionMessage);
  auto client = makeClient(makeInterceptorsList(interceptor));

  // Test interaction without initial response
  // Test for interaction methods with exceptions
  EXPECT_THROW(
      {
        try {
          co_await client->createInteraction(true);
        } catch (const apache::thrift::test::TestException& ex) {
          EXPECT_EQ(*ex.message(), "Exception creating interaction");
          throw;
        }
      },
      apache::thrift::test::TestException);

  EXPECT_EQ(capturedExceptionMessage, "Exception creating interaction");

  // Test interaction with initial response
  capturedExceptionMessage.clear();
  EXPECT_THROW(
      {
        try {
          co_await client->createInteractionAndEcho("exception", true);
        } catch (const apache::thrift::test::TestException& ex) {
          EXPECT_EQ(ex.message(), "Exception creating interaction with echo");
          throw;
        }
      },
      apache::thrift::test::TestException);

  EXPECT_EQ(
      capturedExceptionMessage, "Exception creating interaction with echo");
}

// Test for streams with exceptions
CO_TEST_P(ClientInterceptorTestP, Stream) {
  if (transportType() != TransportType::ROCKET) {
    // only rocket supports streaming
    co_return;
  }
  if (clientCallbackType() == ClientCallbackKind::FUTURE) {
    // future_* functions do not support streaming
    co_return;
  }

  std::string capturedExceptionMessage;
  auto interceptor = makeExceptionCaptureInterceptor(capturedExceptionMessage);
  auto client = makeClient(makeInterceptorsList(interceptor));

  // Test stream without initial response
  EXPECT_THROW(
      {
        try {
          auto stream = co_await client->iota(1, true);
          // Trying to get a value from the stream should throw
          co_await stream.next();
        } catch (const apache::thrift::test::TestException& ex) {
          EXPECT_EQ(ex.message(), "Exception from iota");
          throw;
        }
      },
      apache::thrift::test::TestException);

  EXPECT_EQ(capturedExceptionMessage, "Exception from iota");

  // Test stream with initial response
  EXPECT_THROW(
      auto _ = co_await client->iotaWithResponse(1, true),
      apache::thrift::test::TestException);

  EXPECT_EQ(capturedExceptionMessage, "Exception from iotaWithResponse");
}

// Test for streams with successful return values
CO_TEST_P(ClientInterceptorTestP, StreamWithReturnValue) {
  if (transportType() != TransportType::ROCKET) {
    // only rocket supports streaming
    co_return;
  }
  if (clientCallbackType() == ClientCallbackKind::FUTURE) {
    // future_* functions do not support streaming
    co_return;
  }

  // Test stream without initial response
  {
    int count = 0;
    auto interceptor = makeResultTypeCaptureInterceptor<
        apache::thrift::ClientBufferedStream<int>>(
        [&count](const auto&) { ++count; });
    auto client = makeClient(makeInterceptorsList(interceptor));

    auto stream = co_await client->iota(1);

    // Verify the stream works correctly
    EXPECT_EQ((co_await stream.next()).value(), 1);
    EXPECT_EQ((co_await stream.next()).value(), 2);

    // Verify the interceptor received the correct response type
    EXPECT_EQ(count, 1);

    // Verify the stream can still be consumed after the interceptor has seen it
    EXPECT_EQ((co_await stream.next()).value(), 3);
  }

  // Test stream with initial response
  {
    int initialValue = 0;
    int count = 0;
    auto interceptor = makeResultTypeCaptureInterceptor<
        apache::thrift::ResponseAndClientBufferedStream<int, int>>(
        [&](const auto& responseAndStream) {
          ++count;
          initialValue = responseAndStream.response;
        });

    auto client = makeClient(makeInterceptorsList(interceptor));

    auto [initialResponse, stream] = co_await client->iotaWithResponse(5);

    // Verify the initial response is correct
    EXPECT_EQ(initialResponse, 5);

    // Verify the stream works correctly
    EXPECT_EQ((co_await stream.next()).value(), 5);
    EXPECT_EQ((co_await stream.next()).value(), 6);

    // Verify the interceptor received the correct response type
    EXPECT_EQ(initialValue, 5);
    EXPECT_EQ(count, 1);

    // Verify the stream can still be consumed after the interceptor has seen it
    EXPECT_EQ((co_await stream.next()).value(), 7);
  }
}

// Test that interceptor exceptions override noop method results
CO_TEST_P(ClientInterceptorTestP, NoopWithInterceptorException) {
  auto interceptor =
      makeExceptionGeneratorInterceptor("NoopExceptionInterceptor");
  auto client = makeClient(makeInterceptorsList(interceptor));

  // Test that interceptor exception takes precedence over normal result
  EXPECT_THROW(
      {
        try {
          co_await client->noop(false);
        } catch (const apache::thrift::ClientInterceptorException& ex) {
          EXPECT_THAT(
              ex.what(), HasSubstr("Interceptor exception from onResponse"));
          EXPECT_THAT(ex.what(), HasSubstr("NoopExceptionInterceptor"));
          throw;
        }
      },
      apache::thrift::ClientInterceptorException);

  // Test that interceptor exception takes precedence over server exception
  EXPECT_THROW(
      {
        try {
          co_await client->noop(
              true); // This would normally throw TestException
        } catch (const apache::thrift::ClientInterceptorException& ex) {
          EXPECT_THAT(
              ex.what(), HasSubstr("Interceptor exception from onResponse"));
          EXPECT_THAT(ex.what(), HasSubstr("NoopExceptionInterceptor"));
          throw;
        }
      },
      apache::thrift::ClientInterceptorException);
}

// Test that interceptor exceptions override header client method results
CO_TEST_P(ClientInterceptorTestP, HeaderClientMethodWithInterceptorException) {
  if (clientCallbackType() != ClientCallbackKind::SEMIFUTURE &&
      clientCallbackType() != ClientCallbackKind::FUTURE) {
    // This test case only tests deprecated header client methods, which are
    // future / semifuture
    co_return;
  }

  auto interceptor =
      makeExceptionGeneratorInterceptor("HeaderClientMethodInterceptor");
  auto client = makeClient(makeInterceptorsList(interceptor));

  EXPECT_THROW(
      ({
        try {
          auto [response, header] = co_await client->headerClientMethod("test");
        } catch (const apache::thrift::ClientInterceptorException& ex) {
          EXPECT_THAT(
              ex.what(), HasSubstr("Interceptor exception from onResponse"));
          EXPECT_THAT(ex.what(), HasSubstr("HeaderClientMethodInterceptor"));
          throw;
        }
      }),
      apache::thrift::ClientInterceptorException);
}

// Test that interceptor exceptions override stream results
CO_TEST_P(ClientInterceptorTestP, StreamWithInterceptorException) {
  if (transportType() != TransportType::ROCKET) {
    // only rocket supports streaming
    co_return;
  }
  if (clientCallbackType() == ClientCallbackKind::FUTURE) {
    // future_* functions do not support streaming
    co_return;
  }

  // Test stream without initial response
  {
    auto interceptor =
        makeExceptionGeneratorInterceptor("StreamExceptionInterceptor");
    auto client = makeClient(makeInterceptorsList(interceptor));

    EXPECT_THROW(
        {
          try {
            auto stream = co_await client->iota(1);
          } catch (const apache::thrift::ClientInterceptorException& ex) {
            EXPECT_THAT(
                ex.what(), HasSubstr("Interceptor exception from onResponse"));
            EXPECT_THAT(ex.what(), HasSubstr("StreamExceptionInterceptor"));
            throw;
          }
        },
        apache::thrift::ClientInterceptorException);
  }

  // Test stream with initial response
  {
    auto interceptor = makeExceptionGeneratorInterceptor(
        "StreamWithResponseExceptionInterceptor");
    auto client = makeClient(makeInterceptorsList(interceptor));
    EXPECT_THROW(
        {
          try {
            auto _ = co_await client->iotaWithResponse(5);
          } catch (const apache::thrift::ClientInterceptorException& ex) {
            EXPECT_THAT(
                ex.what(), HasSubstr("Interceptor exception from onResponse"));
            EXPECT_THAT(
                ex.what(), HasSubstr("StreamWithResponseExceptionInterceptor"));
            throw;
          }
        },
        apache::thrift::ClientInterceptorException);
  }

  // Test that interceptor exception overrides server exception
  {
    auto interceptor =
        makeExceptionGeneratorInterceptor("StreamExceptionOverrideInterceptor");
    auto client = makeClient(makeInterceptorsList(interceptor));

    EXPECT_THROW(
        {
          try {
            auto stream = co_await client->iota(1, true);
          } catch (const apache::thrift::ClientInterceptorException& ex) {
            // The interceptor exception should override the server exception
            EXPECT_THAT(
                ex.what(), HasSubstr("Interceptor exception from onResponse"));
            EXPECT_THAT(
                ex.what(), HasSubstr("StreamExceptionOverrideInterceptor"));
            throw;
          }
        },
        apache::thrift::ClientInterceptorException);
  }
}

// Test that interceptor exceptions override interaction results
CO_TEST_P(ClientInterceptorTestP, InteractionWithInterceptorException) {
  if (transportType() != TransportType::ROCKET) {
    // only rocket supports interactions
    co_return;
  }
  if (clientCallbackType() == ClientCallbackKind::FUTURE) {
    // future_* functions do not support interactions
    co_return;
  }

  // Test interaction creation: exception thrown from
  // interceptor takes precedence over exception thrown from server.
  // Test interaction creation
  {
    auto interceptor = makeExceptionGeneratorInterceptor(
        "InteractionCreationExceptionInterceptor");
    auto client = makeClient(makeInterceptorsList(interceptor));

    EXPECT_THROW(
        {
          try {
            auto interaction = co_await client->createInteraction(true);
          } catch (const apache::thrift::ClientInterceptorException& ex) {
            EXPECT_THAT(
                ex.what(), HasSubstr("Interceptor exception from onResponse"));
            EXPECT_THAT(
                ex.what(),
                HasSubstr("InteractionCreationExceptionInterceptor"));
            throw;
          }
        },
        apache::thrift::ClientInterceptorException);
  }

  // Test interaction creation with initial response: exception thrown from
  // interceptor takes precedence over exception thrown from server.
  // Test interaction with initial response
  {
    auto interceptor = makeExceptionGeneratorInterceptor(
        "InteractionWithResponseExceptionInterceptor");
    auto client = makeClient(makeInterceptorsList(interceptor));

    EXPECT_THROW(
        ({
          try {
            auto [interaction, response] =
                co_await client->createInteractionAndEcho("test", false);
          } catch (const apache::thrift::ClientInterceptorException& ex) {
            EXPECT_THAT(
                ex.what(), HasSubstr("Interceptor exception from onResponse"));
            EXPECT_THAT(
                ex.what(),
                HasSubstr("InteractionWithResponseExceptionInterceptor"));
            throw;
          }
        }),
        apache::thrift::ClientInterceptorException);
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
