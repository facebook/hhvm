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

#include <chrono>
#include <stdexcept>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <folly/SocketAddress.h>
#include <folly/coro/BlockingWait.h>
#include <folly/coro/Task.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBaseManager.h>
#include <thrift/lib/cpp/server/TServerEventHandler.h>
#include <thrift/lib/cpp2/async/ClientBufferedStream.h>
#include <thrift/lib/cpp2/async/PooledRequestChannel.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/python/client/OmniClient.h> // @manual=//thrift/lib/python/client:omni_client__cython-lib
#include <thrift/lib/python/client/test/event_handler_helper.h>
#include <thrift/lib/python/client/test/gen-cpp2/TestService.h>
#include <thrift/lib/python/client/test/gen-cpp2/test_types.h>

using namespace apache::thrift;
using namespace apache::thrift::python::client;
using namespace apache::thrift::python::test;

const std::string kTestHeaderKey = "headerKey";
const std::string kTestHeaderValue = "headerValue";

/**
 * A simple Scaffold service that will be used to test the Thrift OmniClient.
 */
class TestServiceHandler
    : virtual public apache::thrift::ServiceHandler<TestService> {
 public:
  TestServiceHandler() {}
  ~TestServiceHandler() override {}
  int add(int num1, int num2) override { return num1 + num2; }
  void oneway() override {}
  void readHeader(
      std::string& value, std::unique_ptr<std::string> key) override {
    value = getRequestContext()->getHeader()->getHeaders().at(*key);
  }
  ServerStream<SimpleResponse> nums(int f, int t) override {
    if (t < f) {
      ArithmeticException e;
      e.msg() = "my_magic_arithmetic_exception";
      throw e;
    }
    return folly::coro::co_invoke(
        [f, t]() -> folly::coro::AsyncGenerator<SimpleResponse&&> {
          for (int i = f; i <= t; ++i) {
            SimpleResponse r;
            r.value() = std::to_string(i);
            co_yield std::move(r);
          }
          if (f < 0) {
            throw std::logic_error("negative_number_detected");
          }
          ArithmeticException e;
          e.msg() = "throw_from_inside_stream";
          throw e;
        });
  }

  ResponseAndServerStream<std::int64_t, SimpleResponse> sumAndNums(
      int f, int t) override {
    if (t < f) {
      ArithmeticException e;
      e.msg() = "my_magic_arithmetic_exception";
      throw e;
    }
    return {
        (f + t) * (t - f + 1) / 2,
        folly::coro::co_invoke(
            [f, t]() -> folly::coro::AsyncGenerator<SimpleResponse&&> {
              for (int i = f; i <= t; ++i) {
                SimpleResponse r;
                r.value() = std::to_string(i);
                co_yield std::move(r);
              }
            }),
    };
  }

  ResponseAndSinkConsumer<SimpleResponse, EmptyChunk, SimpleResponse> dumbSink(
      std::unique_ptr<std::string> hi) override {
    EXPECT_EQ(*hi, "hi");
    SinkConsumer<EmptyChunk, SimpleResponse> consumer{
        [&](folly::coro::AsyncGenerator<EmptyChunk&&> gen)
            -> folly::coro::Task<SimpleResponse> {
          SimpleResponse response;
          response.value() = "final";
          co_return response;
        },
        1};
    SimpleResponse response;
    response.value() = "initial";
    return {std::move(response), std::move(consumer)};
  }

  SinkConsumer<folly::IOBuf, folly::IOBuf> countSinkPyBuf(
      int from, int to) override {
    EXPECT_EQ(from, 'a');
    EXPECT_EQ(to, 'd');
    SinkConsumer<folly::IOBuf, folly::IOBuf> consumer{
        [from, to](folly::coro::AsyncGenerator<folly::IOBuf&&> gen)
            -> folly::coro::Task<folly::IOBuf> {
          int expected = from;
          std::string uploads;
          while (auto iobuf_chunk = co_await gen.next()) {
            StreamChunk chunk;
            folly::IOBuf buf_copy = *iobuf_chunk;
            CompactSerializer::deserialize<StreamChunk>(&buf_copy, chunk);
            int c = *chunk.value();
            EXPECT_EQ(c, expected++);
            uploads += std::to_string(*chunk.value());
          }
          EXPECT_EQ(expected, to);
          co_return std::move(*IOBuf::fromString(uploads));
        }};
    return consumer;
  }
};

/**
 * Small event-handler to know when a server is ready.
 */
class ServerReadyEventHandler : public server::TServerEventHandler {
 public:
  void preServe(const folly::SocketAddress* address) override {
    port_ = address->getPort();
    baton_.post();
  }

  int32_t waitForPortAssignment() {
    baton_.wait();
    return port_;
  }

 private:
  folly::Baton<> baton_;
  int32_t port_;
};

std::unique_ptr<ThriftServer> createServer(
    std::shared_ptr<AsyncProcessorFactory> processorFactory, uint16_t& port) {
  auto server = std::make_unique<ThriftServer>();
  server->setPort(0);
  server->setInterface(std::move(processorFactory));
  server->setNumIOWorkerThreads(1);
  server->setNumCPUWorkerThreads(1);
  server->setQueueTimeout(std::chrono::milliseconds(0));
  server->setIdleTimeout(std::chrono::milliseconds(0));
  server->setTaskExpireTime(std::chrono::milliseconds(0));
  server->setStreamExpireTime(std::chrono::milliseconds(0));
  auto eventHandler = std::make_shared<ServerReadyEventHandler>();
  server->setServerEventHandler(eventHandler);
  server->setup();

  // Get the port that the server has bound to
  port = eventHandler->waitForPortAssignment();
  return server;
}

class OmniClientTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Startup the test server.
    server_ = createServer(std::make_shared<TestServiceHandler>(), serverPort_);
  }

  void TearDown() override {
    // Stop the server and wait for it to complete.
    if (server_) {
      server_->cleanUp();
      server_.reset();
    }
  }

  template <class S>
  void connectToServer(
      folly::Function<folly::coro::Task<void>(OmniClient&)> callMe) {
    constexpr protocol::PROTOCOL_TYPES prot =
        std::is_same_v<S, apache::thrift::BinarySerializer>
        ? protocol::T_BINARY_PROTOCOL
        : protocol::T_COMPACT_PROTOCOL;
    folly::coro::blockingWait([this, &callMe]() -> folly::coro::Task<void> {
      CHECK_GT(serverPort_, 0) << "Check if the server has started already";
      folly::Executor* executor = co_await folly::coro::co_current_executor;
      auto channel = PooledRequestChannel::newChannel(
          executor,
          ioThread_,
          [this](folly::EventBase& evb) {
            auto chan = apache::thrift::RocketClientChannel::newChannel(
                folly::AsyncSocket::UniquePtr(
                    new folly::AsyncSocket(&evb, "::1", serverPort_)));
            chan->setProtocolId(prot);
            chan->setTimeout(500 /* ms */);
            return chan;
          },
          prot);
      OmniClient client(std::move(channel));
      co_await callMe(client);
    }());
  }

  // Send a request and compare the results to the expected value.
  template <class S = CompactSerializer, class Request, class Result>
  void testSendHeaders(
      const std::string& service,
      const std::string& function,
      const Request& req,
      const std::unordered_map<std::string, std::string>& headers,
      const Result& expected,
      const RpcKind rpcKind = RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      const bool clearEventHandlers = false) {
    connectToServer<S>(
        [=, this](OmniClient& client) -> folly::coro::Task<void> {
          if (clearEventHandlers) {
            client.clearEventHandlers();
          }
          std::string args = S::template serialize<std::string>(req);
          auto data = apache::thrift::MethodMetadata::Data(
              function, apache::thrift::FunctionQualifier::Unspecified);
          auto resp = co_await client.semifuture_send(
              service,
              function,
              args,
              std::move(data),
              headers,
              {},
              co_await folly::coro::co_current_executor,
              rpcKind);
          testContains<S>(std::move(resp.buf.value()), expected);
        });
  }

  template <class S = CompactSerializer, class Request, class Result>
  void testSend(
      const std::string& service,
      const std::string& function,
      const Request& req,
      const Result& expected,
      const RpcKind rpcKind = RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      const bool clearEventHandlers = false) {
    testSendHeaders<S>(
        service, function, req, {}, expected, rpcKind, clearEventHandlers);
  }

  // Send a request and compare the results to the expected value.
  template <class S, class Request>
  void testOnewaySendHeaders(
      const std::string& service,
      const std::string& function,
      const Request& req,
      const std::unordered_map<std::string, std::string>& headers = {}) {
    connectToServer<S>([=](OmniClient& client) -> folly::coro::Task<void> {
      std::string args = S::template serialize<std::string>(req);
      auto data = apache::thrift::MethodMetadata::Data(
          function, apache::thrift::FunctionQualifier::Unspecified);
      client.oneway_send(service, function, args, std::move(data), headers, {});
      co_return;
    });
  }

  template <class S, typename T>
  void testContains(std::unique_ptr<folly::IOBuf> buf, const T& expected) {
    std::string expectedStr = S::template serialize<std::string>(expected);
    std::string result = buf->to<std::string>();
    // Contains instead of equals because of the envelope around the response.
    EXPECT_THAT(result, testing::HasSubstr(expectedStr));
  }

  template <class S = CompactSerializer, class Request>
  void testSendStream(
      const std::string& service,
      const std::string& function,
      const Request& req,
      folly::Function<folly::coro::Task<void>(OmniClientResponseWithHeaders&&)>
          onResponse) {
    connectToServer<S>(
        [&](OmniClient& client) mutable -> folly::coro::Task<void> {
          std::string args = S::template serialize<std::string>(req);
          auto data = apache::thrift::MethodMetadata::Data(
              function, apache::thrift::FunctionQualifier::Unspecified);
          co_await onResponse(co_await client.semifuture_send(
              service,
              function,
              args,
              std::move(data),
              {},
              {},
              co_await folly::coro::co_current_executor,
              RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE));
        });
  }

  template <class S = CompactSerializer, class Request>
  void testSendSink(
      const std::string& service,
      const std::string& function,
      const Request& req,
      const std::string& expected,
      folly::Function<folly::coro::Task<std::unique_ptr<folly::IOBuf>>(
          OmniClientResponseWithHeaders&&)> onResponse) {
    connectToServer<S>(
        [&](OmniClient& client) mutable -> folly::coro::Task<void> {
          std::string args = S::template serialize<std::string>(req);
          auto data = apache::thrift::MethodMetadata::Data(
              function, apache::thrift::FunctionQualifier::Unspecified);
          auto resp = co_await onResponse(co_await client.semifuture_send(
              service,
              function,
              args,
              std::move(data),
              {},
              {},
              co_await folly::coro::co_current_executor,
              RpcKind::SINK));
          EXPECT_EQ(resp->template to<std::string>(), expected);
        });
  }

 protected:
  std::unique_ptr<ThriftServer> server_;
  folly::EventBase* eb_ = folly::EventBaseManager::get()->getEventBase();
  uint16_t serverPort_{0};
  std::shared_ptr<folly::IOExecutor> ioThread_{
      std::make_shared<folly::ScopedEventBaseThread>()};
};

TEST_F(OmniClientTest, AddTestFailsWithBadEventHandler) {
  AddRequest request;
  request.num1() = 1;
  request.num2() = 41;
  addHandler();
  EXPECT_THROW(
      {
        testSend<CompactSerializer>("TestService", "add", request, 42);
        testSend<BinarySerializer>("TestService", "add", request, 42);
      },
      folly::BadExpectedAccess<folly::exception_wrapper>);
}

TEST_F(OmniClientTest, AddTestPassesWhenBadEventHandlerIsCleared) {
  AddRequest request;
  request.num1() = 1;
  request.num2() = 41;
  addHandler();
  testSend<CompactSerializer>(
      "TestService",
      "add",
      request,
      42,
      RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      true);
  testSend<BinarySerializer>(
      "TestService",
      "add",
      request,
      42,
      RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      true);
}

TEST_F(OmniClientTest, AddTest) {
  AddRequest request;
  request.num1() = 1;
  request.num2() = 41;

  testSend<CompactSerializer>("TestService", "add", request, 42);
  testSend<BinarySerializer>("TestService", "add", request, 42);
}

TEST_F(OmniClientTest, OnewayTest) {
  EmptyRequest request;
  testOnewaySendHeaders<CompactSerializer>("TestService", "oneway", request);
  testOnewaySendHeaders<BinarySerializer>("TestService", "oneway", request);
}

TEST_F(OmniClientTest, ReadHeaderTest) {
  ReadHeaderRequest request;
  request.key() = kTestHeaderKey;

  testSendHeaders(
      "TestService",
      "readHeader",
      request,
      {{kTestHeaderKey, kTestHeaderValue}},
      kTestHeaderValue);
}

TEST_F(OmniClientTest, SinkRequestTest) {
  EmptyRequest request;
  request.hi() = "hi";
  SimpleResponse response;
  response.value() = "initial";
  testSend("TestService", "dumbSink", request, response, RpcKind::SINK);
}

TEST_F(OmniClientTest, StreamNumsTest) {
  NumsRequest request;
  request.f() = 2;
  request.t() = 4;
  testSendStream(
      "TestService",
      "nums",
      request,
      [this](OmniClientResponseWithHeaders&& resp) -> folly::coro::Task<void> {
        auto gen = std::move(*resp.stream).toAsyncGenerator();
        for (int i = 2; i <= 4; ++i) {
          auto val = co_await gen.next();
          EXPECT_TRUE(val);
          testContains<CompactSerializer>(std::move(*val), std::to_string(i));
        }
        auto val = co_await gen.next();
        testContains<CompactSerializer>(
            std::move(*val), std::string{"throw_from_inside_stream"});
      });
}

TEST_F(OmniClientTest, StreamNumsUndeclaredExceptionTest) {
  NumsRequest request;
  request.f() = -1;
  request.t() = 4;
  testSendStream(
      "TestService",
      "nums",
      request,
      [this](OmniClientResponseWithHeaders&& resp) -> folly::coro::Task<void> {
        auto gen = std::move(*resp.stream).toAsyncGenerator();
        for (int i = -1; i <= 4; ++i) {
          auto val = co_await gen.next();
          EXPECT_TRUE(val);
          testContains<CompactSerializer>(std::move(*val), std::to_string(i));
        }
        EXPECT_THROW(co_await gen.next(), TApplicationException);
      });
}

TEST_F(OmniClientTest, StreamSumAndNumsTest) {
  NumsRequest request;
  request.f() = 2;
  request.t() = 4;
  testSendStream(
      "TestService",
      "sumAndNums",
      request,
      [this](OmniClientResponseWithHeaders&& resp) -> folly::coro::Task<void> {
        testContains<CompactSerializer, int64_t>(
            std::move(resp.buf.value()), 9);
        auto gen = std::move(*resp.stream).toAsyncGenerator();
        for (int i = 2; i <= 4; ++i) {
          auto val = co_await gen.next();
          EXPECT_TRUE(val);
          testContains<CompactSerializer>(std::move(*val), std::to_string(i));
        }
        EXPECT_FALSE(co_await gen.next());
      });
}

TEST_F(OmniClientTest, StreamSumAndNumsExceptionTest) {
  NumsRequest request;
  request.f() = 4;
  request.t() = 2;
  testSendStream(
      "TestService",
      "sumAndNums",
      request,
      [this](OmniClientResponseWithHeaders&& resp) -> folly::coro::Task<void> {
        testContains<CompactSerializer>(
            std::move(resp.buf.value()),
            std::string{"my_magic_arithmetic_exception"});
        auto gen = std::move(*resp.stream).toAsyncGenerator();
        EXPECT_FALSE(co_await gen.next());
      });
}

folly::coro::AsyncGenerator<StreamChunk&&> chunkGenerator(int from, int to) {
  int i = from;
  while (i < to) {
    StreamChunk chunk;
    chunk.value() = i++;
    co_yield std::move(chunk);
  }
}

template <typename S = CompactSerializer>
folly::coro::AsyncGenerator<std::unique_ptr<folly::IOBuf>&&> chunkBufGenerator(
    NumsRequest request) {
  auto gen = chunkGenerator(*request.f(), *request.t());
  while (auto chunk = co_await gen.next()) {
    folly::IOBufQueue queue;
    S::serialize(*chunk, &queue);
    co_yield queue.move();
  }
}

TEST_F(OmniClientTest, CountSinkTest) {
  NumsRequest request;
  request.f() = 'a';
  request.t() = 'd';
  std::string expected_final = "979899";
  testSendSink<CompactSerializer, NumsRequest>(
      "TestService",
      "countSinkPyBuf",
      request,
      expected_final,
      [request](OmniClientResponseWithHeaders&& resp)
          -> folly::coro::Task<std::unique_ptr<folly::IOBuf>> {
        auto sink = std::move(*resp.sink);
        co_return co_await sink.sink(chunkBufGenerator(request));
      });
}
