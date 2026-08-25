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
#include <optional>
#include <stdexcept>
#include <thread>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <folly/SocketAddress.h>
#include <folly/coro/BlockingWait.h>
#include <folly/coro/Task.h>
#include <folly/init/Init.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBaseManager.h>
#include <thrift/lib/cpp/server/TServerEventHandler.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/ClientBufferedStream.h>
#include <thrift/lib/cpp2/async/ClientInterceptor.h>
#include <thrift/lib/cpp2/async/InterceptorFlags.h>
#include <thrift/lib/cpp2/async/PooledRequestChannel.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/runtime/Init.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/python/client/OmniClient.h> // @manual=//thrift/lib/python/client:omni_client__cython-lib
#include <thrift/lib/python/client/test/event_handler_helper.h>
#include <thrift/lib/python/client/test/gen-cpp2/CompressionTestService.h>
#include <thrift/lib/python/client/test/gen-cpp2/TestService.h>
#include <thrift/lib/python/client/test/gen-cpp2/test_types.h>

using namespace apache::thrift;
using namespace apache::thrift::python::client;
using namespace apache::thrift::python::test;

const std::string kTestHeaderKey = "headerKey";
const std::string kTestHeaderValue = "headerValue";
constexpr size_t kLargePayloadSize = 8192;
constexpr char kStreamValueCharacter = 'a';
constexpr char kStreamExceptionCharacter = 'E';
constexpr char kSinkFinalCharacter = 'S';
constexpr char kBiDiValueCharacter = 'B';

/**
 * A simple interceptor that tracks requests for testing global interceptor
 * registration in Python/OmniClient.
 */
class TracingClientInterceptor
    : public apache::thrift::ClientInterceptor<std::int64_t> {
 public:
  std::string getName() const override { return "TracingClientInterceptor"; }

  using Trace = std::
      tuple<std::string, std::string, std::int64_t, THeader::StringToStringMap>;
  const std::vector<Trace>& requests() const { return requests_; }
  const std::vector<Trace>& responses() const { return responses_; }
  const std::vector<std::string>& responseExceptions() const {
    return responseExceptions_;
  }
  void clear() {
    requests_.clear();
    responses_.clear();
    responseExceptions_.clear();
    requestCount_ = 0;
    responseCount_ = 0;
  }

  std::optional<std::int64_t> onRequest(RequestInfo requestInfo) override {
    auto requestData = requestCount_++;
    requests_.emplace_back(
        std::string(requestInfo.serviceName),
        std::string(requestInfo.methodName),
        requestData,
        requestInfo.headers->getWriteHeaders());
    return requestData;
  }

  std::optional<folly::coro::Task<void>> onResponse(
      std::int64_t* requestData, ResponseInfo responseInfo) override {
    if (responseInfo.result.hasException()) {
      responseExceptions_.emplace_back(
          responseInfo.result.exception().what().toStdString());
    }

    responses_.emplace_back(
        std::string(responseInfo.serviceName),
        std::string(responseInfo.methodName),
        *requestData,
        responseInfo.headers->getHeaders());
    return std::nullopt;
  }

 private:
  std::vector<Trace> requests_;
  std::vector<Trace> responses_;
  std::vector<std::string> responseExceptions_;
  std::atomic_uint64_t requestCount_{0};
  std::atomic_uint64_t responseCount_{0};
};

// Global interceptor instance - registered in main() before tests run
std::shared_ptr<TracingClientInterceptor> gTracingInterceptor =
    std::make_shared<TracingClientInterceptor>();

/**
 * A simple Scaffold service that will be used to test the Thrift OmniClient.
 */
class CompressionTestServiceHandler
    : virtual public apache::thrift::ServiceHandler<CompressionTestService> {
 public:
  CompressionTestServiceHandler() {}
  ~CompressionTestServiceHandler() override {}
  int add(int num1, int num2) override { return num1 + num2; }
  void oneway() override {}
  void readHeader(
      std::string& value, std::unique_ptr<std::string> key) override {
    // Read the received header and echo it back in the response body and
    // response header.
    const auto& readHeaders = getRequestContext()->getHeader()->getHeaders();
    value = getRequestContext()->getHeader()->getHeaders().at(*key);

    for (const auto& [header, headerValue] : readHeaders) {
      getRequestContext()->getHeader()->setHeader(header, headerValue);
    }
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

  ServerStream<SimpleResponse> compressionStreamValue() override {
    return folly::coro::co_invoke(
        []() -> folly::coro::AsyncGenerator<SimpleResponse&&> {
          SimpleResponse response;
          response.value() =
              std::string(kLargePayloadSize, kStreamValueCharacter);
          co_yield std::move(response);
        });
  }

  ServerStream<SimpleResponse> compressionStreamDeclaredError() override {
    return folly::coro::co_invoke(
        []() -> folly::coro::AsyncGenerator<SimpleResponse&&> {
          SimpleResponse response;
          response.value() = "before error";
          co_yield std::move(response);

          ArithmeticException error;
          error.msg() =
              std::string(kLargePayloadSize, kStreamExceptionCharacter);
          throw error;
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

  folly::coro::Task<StreamTransformation<folly::IOBuf, folly::IOBuf>>
  co_bidiBuffer() override {
    co_return StreamTransformation<folly::IOBuf, folly::IOBuf>{
        [](folly::coro::AsyncGenerator<folly::IOBuf&&> clientInput)
            -> folly::coro::AsyncGenerator<folly::IOBuf&&> {
          // This test covers inbound decode only; the client sends no outbound
          // items. A read from clientInput before co_yield would block the
          // response. Stream cancellation destroys the unused input when the
          // test drops the response.
          (void)clientInput;
          co_yield std::move(*folly::IOBuf::copyBuffer(
              std::string(kLargePayloadSize, kBiDiValueCharacter)));
        }};
  }

  SinkConsumer<folly::IOBuf, folly::IOBuf> countSinkPyBuf(
      int from, int to) override {
    EXPECT_EQ('a', from);
    EXPECT_EQ('d', to);
    SinkConsumer<folly::IOBuf, folly::IOBuf> consumer{
        [from, to](folly::coro::AsyncGenerator<folly::IOBuf&&> gen)
            -> folly::coro::Task<folly::IOBuf> {
          int expected = from;
          std::string uploads;
          while (auto iobufChunk = co_await gen.next()) {
            StreamChunk chunk;
            folly::IOBuf bufferCopy = *iobufChunk;
            CompactSerializer::deserialize<StreamChunk>(&bufferCopy, chunk);
            int value = *chunk.value();
            EXPECT_EQ(expected++, value);
            uploads += std::to_string(value);
          }
          EXPECT_EQ(to, expected);
          co_return std::move(*IOBuf::fromString(uploads));
        }};
    return consumer;
  }

  SinkConsumer<folly::IOBuf, folly::IOBuf> compressionSinkFinalResponse()
      override {
    SinkConsumer<folly::IOBuf, folly::IOBuf> consumer{
        [](folly::coro::AsyncGenerator<folly::IOBuf&&> gen)
            -> folly::coro::Task<folly::IOBuf> {
          size_t chunkCount = 0;
          while (co_await gen.next()) {
            ++chunkCount;
          }
          EXPECT_EQ(size_t{1}, chunkCount);
          co_return std::move(*IOBuf::fromString(
              std::string(kLargePayloadSize, kSinkFinalCharacter)));
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

struct RequestCompressionObservation {
  size_t callCount{0};
  std::optional<std::thread::id> thread;
  std::optional<size_t> requestSize;
  std::optional<transport::THeader::StringToStringMap> rpcOptionsHeaders;
  std::optional<transport::THeader::StringToStringMap> requestHeaders;
  bool calledBeforeDispatch{false};
  bool throwOnCompress{false};
};

class CompressionTrackingChannel final : public RequestChannel {
 public:
  static RequestChannel::Ptr newChannel(
      RequestChannel::Ptr impl,
      std::optional<std::thread::id>& decompressResponseThread,
      std::optional<CompressionAlgorithm>& responseCompressionAlgorithm,
      RequestCompressionObservation& requestCompressionObservation) {
    return {
        new CompressionTrackingChannel(
            std::move(impl),
            decompressResponseThread,
            responseCompressionAlgorithm,
            requestCompressionObservation),
        {}};
  }

  void sendRequestResponse(
      RpcOptions&& rpcOptions,
      MethodMetadata&& methodMetadata,
      SerializedRequest&& request,
      std::shared_ptr<transport::THeader> header,
      RequestClientCallback::Ptr callback,
      std::unique_ptr<folly::IOBuf> frameworkMetadata) override {
    requestCompressionObservation_.calledBeforeDispatch =
        requestCompressionObservation_.callCount > 0;
    impl_->sendRequestResponse(
        std::move(rpcOptions),
        std::move(methodMetadata),
        std::move(request),
        std::move(header),
        std::move(callback),
        std::move(frameworkMetadata));
  }

  void sendRequestNoResponse(
      RpcOptions&& rpcOptions,
      MethodMetadata&& methodMetadata,
      SerializedRequest&& request,
      std::shared_ptr<transport::THeader> header,
      RequestClientCallback::Ptr callback,
      std::unique_ptr<folly::IOBuf> frameworkMetadata) override {
    impl_->sendRequestNoResponse(
        std::move(rpcOptions),
        std::move(methodMetadata),
        std::move(request),
        std::move(header),
        std::move(callback),
        std::move(frameworkMetadata));
  }

  void sendRequestStream(
      RpcOptions&& rpcOptions,
      MethodMetadata&& methodMetadata,
      SerializedRequest&& request,
      std::shared_ptr<transport::THeader> header,
      StreamClientCallback* callback,
      std::unique_ptr<folly::IOBuf> frameworkMetadata) override {
    impl_->sendRequestStream(
        std::move(rpcOptions),
        std::move(methodMetadata),
        std::move(request),
        std::move(header),
        callback,
        std::move(frameworkMetadata));
  }

  void sendRequestSink(
      RpcOptions&& rpcOptions,
      MethodMetadata&& methodMetadata,
      SerializedRequest&& request,
      std::shared_ptr<transport::THeader> header,
      SinkClientCallback* callback,
      std::unique_ptr<folly::IOBuf> frameworkMetadata) override {
    impl_->sendRequestSink(
        std::move(rpcOptions),
        std::move(methodMetadata),
        std::move(request),
        std::move(header),
        callback,
        std::move(frameworkMetadata));
  }

  void sendRequestBiDi(
      RpcOptions&& rpcOptions,
      MethodMetadata&& methodMetadata,
      SerializedRequest&& request,
      std::shared_ptr<transport::THeader> header,
      BiDiClientCallback* callback,
      std::unique_ptr<folly::IOBuf> frameworkMetadata) override {
    impl_->sendRequestBiDi(
        std::move(rpcOptions),
        std::move(methodMetadata),
        std::move(request),
        std::move(header),
        callback,
        std::move(frameworkMetadata));
  }

  void compressRequest(
      SerializedRequest& request,
      const RpcOptions& rpcOptions,
      transport::THeader& header) override {
    ++requestCompressionObservation_.callCount;
    requestCompressionObservation_.thread = std::this_thread::get_id();
    requestCompressionObservation_.requestSize = request.buffer
        ? std::optional<size_t>{request.buffer->computeChainDataLength()}
        : std::nullopt;
    requestCompressionObservation_.rpcOptionsHeaders =
        rpcOptions.getWriteHeaders();
    requestCompressionObservation_.requestHeaders = header.getWriteHeaders();
    if (requestCompressionObservation_.throwOnCompress) {
      throw std::runtime_error("compression hook failure");
    }
    // This wrapper proves that OmniClient invokes the outer channel hook.
    // The pooled delegate has no caller-thread compression policy;
    // ServiceRouter integration tests own real compression behavior.
    impl_->compressRequest(request, rpcOptions, header);
  }

  void decompressResponse(ClientReceiveState& state) override {
    decompressResponseThread_ = std::this_thread::get_id();
    if (auto* header = state.header()) {
      responseCompressionAlgorithm_ = header->getResponseCompressionAlgorithm();
    } else {
      responseCompressionAlgorithm_.reset();
    }
    impl_->decompressResponse(state);
  }

  void terminateInteraction(InteractionId id) override {
    impl_->terminateInteraction(std::move(id));
  }

  InteractionId createInteraction(ManagedStringView&& name) override {
    return impl_->createInteraction(std::move(name));
  }

  InteractionId registerInteraction(
      ManagedStringView&& name, int64_t id) override {
    return impl_->registerInteraction(std::move(name), id);
  }

  void setCloseCallback(CloseCallback* callback) override {
    impl_->setCloseCallback(callback);
  }

  folly::EventBase* getEventBase() const override {
    return impl_->getEventBase();
  }

  uint16_t getProtocolId() override { return impl_->getProtocolId(); }

 private:
  CompressionTrackingChannel(
      RequestChannel::Ptr impl,
      std::optional<std::thread::id>& decompressResponseThread,
      std::optional<CompressionAlgorithm>& responseCompressionAlgorithm,
      RequestCompressionObservation& requestCompressionObservation)
      : impl_(std::move(impl)),
        decompressResponseThread_(decompressResponseThread),
        responseCompressionAlgorithm_(responseCompressionAlgorithm),
        requestCompressionObservation_(requestCompressionObservation) {}

  RequestChannel::Ptr impl_;
  std::optional<std::thread::id>& decompressResponseThread_;
  std::optional<CompressionAlgorithm>& responseCompressionAlgorithm_;
  RequestCompressionObservation& requestCompressionObservation_;
};

class InteractionTrackingClient final : public OmniClient {
 public:
  explicit InteractionTrackingClient(RequestChannelShared channel)
      : OmniClient(std::move(channel)) {}

  size_t setInteractionCallCount() const { return setInteractionCallCount_; }

 protected:
  void setInteraction(RpcOptions&) override { ++setInteractionCallCount_; }

 private:
  size_t setInteractionCallCount_{0};
};

class OmniClientTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Startup the test server.
    server_ = createServer(
        std::make_shared<CompressionTestServiceHandler>(), serverPort_);

    // Verify interceptor is registered
    auto registeredInterceptors =
        apache::thrift::runtime::getGlobalClientInterceptors();
    ASSERT_NE(registeredInterceptors, nullptr);
    ASSERT_EQ(registeredInterceptors->size(), 1);
    ASSERT_EQ(
        (*registeredInterceptors)[0]->getName(), "TracingClientInterceptor");

    // Clean up the global interceptor state.
    gTracingInterceptor->clear();
    EXPECT_TRUE(gTracingInterceptor->requests().empty());
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
      auto pooledChannel = PooledRequestChannel::newChannel(
          executor,
          ioThread_,
          [this](folly::EventBase& evb) {
            auto chan = apache::thrift::RocketClientChannel::newChannel(
                folly::AsyncSocket::UniquePtr(
                    new folly::AsyncSocket(&evb, "::1", serverPort_)));
            if (compressResponses_) {
              CompressionConfig compressionConfig;
              compressionConfig.codecConfig().ensure().set_zstdConfig();
              chan->setDesiredCompressionConfig(compressionConfig);
            }
            chan->setProtocolId(prot);
            chan->setTimeout(500 /* ms */);
            return chan;
          },
          prot);
      auto channel = CompressionTrackingChannel::newChannel(
          std::move(pooledChannel),
          decompressResponseThread_,
          responseCompressionAlgorithm_,
          requestCompressionObservation_);
      OmniClient client(std::move(channel));
      executorThread_ = std::this_thread::get_id();
      decompressResponseThread_.reset();
      responseCompressionAlgorithm_.reset();
      requestCompressionObservation_ = {};
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
          co_await onResponse(
              co_await client.semifuture_send(
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
          auto resp = co_await onResponse(
              co_await client.semifuture_send(
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
  bool compressResponses_{false};
  std::optional<std::thread::id> executorThread_;
  std::optional<std::thread::id> decompressResponseThread_;
  std::optional<CompressionAlgorithm> responseCompressionAlgorithm_;
  RequestCompressionObservation requestCompressionObservation_;
  std::unique_ptr<ThriftServer> server_;
  folly::EventBase* eb_ = folly::EventBaseManager::get()->getEventBase();
  uint16_t serverPort_{0};
  std::shared_ptr<folly::IOExecutor> ioThread_{
      std::make_shared<folly::ScopedEventBaseThread>()};
};

// The whole point of the marker: a legacy event handler shared with generated
// C++ clients can still tell that this request came from thrift-python.
TEST_F(OmniClientTest, RequestsReportThePythonClientRuntime) {
  class RuntimeRecordingHandler : public TProcessorEventHandler {
   public:
    std::optional<ClientRuntime> observed;

   private:
    void* getServiceContext(
        std::string_view,
        std::string_view,
        TConnectionContext* connectionContext) override {
      observed = connectionContext->getClientRuntime();
      return nullptr;
    }
  };

  auto recorder = std::make_shared<RuntimeRecordingHandler>();
  AddRequest request;
  request.num1() = 1;
  request.num2() = 41;

  connectToServer<CompactSerializer>(
      [&](OmniClient& client) -> folly::coro::Task<void> {
        // Per-instance, so the test does not depend on what the global handler
        // list happens to hold.
        client.clearEventHandlers();
        client.addEventHandler(recorder);
        auto resp = co_await client.semifuture_send(
            "TestService",
            "add",
            CompactSerializer::serialize<std::string>(request),
            apache::thrift::MethodMetadata::Data(
                "add", apache::thrift::FunctionQualifier::Unspecified),
            {},
            {},
            co_await folly::coro::co_current_executor);
        testContains<CompactSerializer>(std::move(resp.buf.value()), 42);
      });

  EXPECT_EQ(recorder->observed, ClientRuntime::Python);
}

class OmniClientCompressionTest : public OmniClientTest,
                                  public ::testing::WithParamInterface<bool> {
 protected:
  void SetUp() override {
    OmniClientTest::SetUp();
    compressResponses_ = true;
  }

  void TearDown() override {
    THRIFT_FLAG_UNMOCK(thrift_client_compress_request_on_cpu);
    OmniClientTest::TearDown();
  }
};

class OmniClientStreamPayloadCompressionTest : public OmniClientTest {
 protected:
  void SetUp() override {
    OmniClientTest::SetUp();
    compressResponses_ = true;
    THRIFT_FLAG_SET_MOCK(thrift_client_compress_request_on_cpu, true);
  }

  void TearDown() override {
    THRIFT_FLAG_UNMOCK(thrift_client_compress_request_on_cpu);
    OmniClientTest::TearDown();
  }
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

TEST_F(OmniClientTest, RequestCompressionHookRunsBeforeDispatch) {
  // GIVEN
  ReadHeaderRequest request;
  request.key() = kTestHeaderKey;
  const std::unordered_map<std::string, std::string> requestHeaders{
      {kTestHeaderKey, kTestHeaderValue}};
  const transport::THeader::StringToStringMap expectedHookHeaders{
      {kTestHeaderKey, kTestHeaderValue}};
  const auto expectedRequestSize =
      CompactSerializer::serialize<std::string>(request).size();

  // WHEN
  testSendHeaders(
      "TestService", "readHeader", request, requestHeaders, kTestHeaderValue);

  // THEN
  EXPECT_EQ(
      std::make_tuple(
          size_t{1},
          executorThread_,
          std::optional<size_t>{expectedRequestSize},
          std::optional{expectedHookHeaders},
          std::optional{expectedHookHeaders},
          true),
      std::make_tuple(
          requestCompressionObservation_.callCount,
          requestCompressionObservation_.thread,
          requestCompressionObservation_.requestSize,
          requestCompressionObservation_.rpcOptionsHeaders,
          requestCompressionObservation_.requestHeaders,
          requestCompressionObservation_.calledBeforeDispatch));
}

TEST_F(OmniClientTest, RequestCompressionFailureClearsInteractionFactory) {
  // GIVEN
  ReadHeaderRequest request;
  request.key() = kTestHeaderKey;
  const std::unordered_map<std::string, std::string> requestHeaders{
      {kTestHeaderKey, kTestHeaderValue}};
  const auto expected = std::make_tuple(true, true, size_t{1});

  // WHEN
  // THEN
  connectToServer<CompactSerializer>(
      [this, request, requestHeaders, expected](
          OmniClient& client) -> folly::coro::Task<void> {
        InteractionTrackingClient factoryClient(client.getChannelShared());
        client.set_interaction_factory(&factoryClient);
        requestCompressionObservation_.throwOnCompress = true;

        std::string firstArgs =
            CompactSerializer::serialize<std::string>(request);
        auto firstResponse = co_await client.semifuture_send(
            "TestService",
            "readHeader",
            firstArgs,
            apache::thrift::MethodMetadata::Data(
                "readHeader", apache::thrift::FunctionQualifier::Unspecified),
            requestHeaders,
            {},
            co_await folly::coro::co_current_executor);

        requestCompressionObservation_.throwOnCompress = false;
        std::string secondArgs =
            CompactSerializer::serialize<std::string>(request);
        auto secondResponse = co_await client.semifuture_send(
            "TestService",
            "readHeader",
            secondArgs,
            apache::thrift::MethodMetadata::Data(
                "readHeader", apache::thrift::FunctionQualifier::Unspecified),
            requestHeaders,
            {},
            co_await folly::coro::co_current_executor);

        EXPECT_EQ(
            expected,
            std::make_tuple(
                firstResponse.buf.hasError(),
                secondResponse.buf.hasValue(),
                factoryClient.setInteractionCallCount()));
      });
}

TEST_P(OmniClientCompressionTest, CompressedResponse) {
  // GIVEN
  THRIFT_FLAG_SET_MOCK(thrift_client_compress_request_on_cpu, GetParam());
  const std::string expected(kLargePayloadSize, kStreamValueCharacter);
  const auto expectedCompressionAlgorithm =
      GetParam() ? CompressionAlgorithm::ZSTD : CompressionAlgorithm::NONE;
  ReadHeaderRequest request;
  request.key() = kTestHeaderKey;

  // WHEN
  // THEN
  testSendHeaders(
      "TestService",
      "readHeader",
      request,
      {{kTestHeaderKey, expected}},
      expected);

  EXPECT_EQ(
      std::make_pair(
          executorThread_,
          std::optional<CompressionAlgorithm>{expectedCompressionAlgorithm}),
      std::make_pair(decompressResponseThread_, responseCompressionAlgorithm_));
}

INSTANTIATE_TEST_SUITE_P(
    DecompressionThread,
    OmniClientCompressionTest,
    ::testing::Bool(),
    [](const ::testing::TestParamInfo<bool>& info) {
      return info.param ? "DeferredHook" : "EagerUnpack";
    });

TEST_F(OmniClientTest, SinkRequestTest) {
  EmptyRequest request;
  request.hi() = "hi";
  SimpleResponse response;
  response.value() = "initial";
  testSend("TestService", "dumbSink", request, response, RpcKind::SINK);
}

TEST_F(OmniClientStreamPayloadCompressionTest, StreamValues) {
  // GIVEN
  CompressionStreamRequest request;
  const std::string expected(kLargePayloadSize, kStreamValueCharacter);

  // WHEN
  // THEN
  testSendStream(
      "CompressionTestService",
      "compressionStreamValue",
      request,
      [this, expected](
          OmniClientResponseWithHeaders&& resp) -> folly::coro::Task<void> {
        auto gen = std::move(*resp.stream).toAsyncGenerator();
        auto actual = co_await gen.next();
        if (!actual) {
          ADD_FAILURE() << "Expected a stream value payload";
          co_return;
        }
        testContains<CompactSerializer>(std::move(*actual), expected);
      });
}

TEST_F(OmniClientStreamPayloadCompressionTest, StreamDeclaredException) {
  // GIVEN
  CompressionStreamRequest request;
  const std::string expected(kLargePayloadSize, kStreamExceptionCharacter);

  // WHEN
  // THEN
  testSendStream(
      "CompressionTestService",
      "compressionStreamDeclaredError",
      request,
      [this, expected](
          OmniClientResponseWithHeaders&& resp) -> folly::coro::Task<void> {
        auto gen = std::move(*resp.stream).toAsyncGenerator();
        co_await gen.next();
        auto actual = co_await gen.next();
        if (!actual) {
          ADD_FAILURE() << "Expected a declared stream exception payload";
          co_return;
        }
        testContains<CompactSerializer>(std::move(*actual), expected);
      });
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

TEST_F(OmniClientStreamPayloadCompressionTest, SinkFinalResponse) {
  // GIVEN
  CompressionSinkRequest request;
  NumsRequest sinkInput;
  sinkInput.f() = 0;
  sinkInput.t() = 1;
  const std::string expected(kLargePayloadSize, kSinkFinalCharacter);

  // WHEN
  // THEN
  testSendSink<CompactSerializer, CompressionSinkRequest>(
      "CompressionTestService",
      "compressionSinkFinalResponse",
      request,
      expected,
      [sinkInput](OmniClientResponseWithHeaders&& resp)
          -> folly::coro::Task<std::unique_ptr<folly::IOBuf>> {
        auto sink = std::move(*resp.sink);
        co_return co_await sink.sink(chunkBufGenerator(sinkInput));
      });
}

TEST_F(OmniClientStreamPayloadCompressionTest, BiDiInboundValue) {
  // GIVEN
  EmptyRequest request;
  const std::string expected(kLargePayloadSize, kBiDiValueCharacter);

  // WHEN
  // THEN
  connectToServer<CompactSerializer>(
      [request, expected](OmniClient& client) -> folly::coro::Task<void> {
        std::string args = CompactSerializer::serialize<std::string>(request);
        auto data = apache::thrift::MethodMetadata::Data(
            "bidiBuffer", apache::thrift::FunctionQualifier::Unspecified);
        auto response = co_await client.semifuture_send(
            "TestService",
            "bidiBuffer",
            args,
            std::move(data),
            {},
            {},
            co_await folly::coro::co_current_executor,
            RpcKind::BIDIRECTIONAL_STREAM);
        auto gen = std::move(*response.stream).toAsyncGenerator();
        auto actual = co_await gen.next();
        if (!actual) {
          ADD_FAILURE() << "Expected a bidi inbound value payload";
          co_return;
        }
        EXPECT_EQ(expected, (*actual)->to<std::string>());
      });
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

TEST_F(OmniClientTest, GetChannelProtocolIdReturnsValidProtocol) {
  // Test that getChannelProtocolId returns a valid folly::Try with the correct
  // protocol ID when using CompactSerializer
  connectToServer<CompactSerializer>(
      [](OmniClient& client) -> folly::coro::Task<void> {
        auto protocolTry = client.getChannelProtocolId();
        EXPECT_FALSE(protocolTry.hasException());
        EXPECT_EQ(protocolTry.value(), protocol::T_COMPACT_PROTOCOL);
        co_return;
      });

  // Test that getChannelProtocolId returns a valid folly::Try with the correct
  // protocol ID when using BinarySerializer
  connectToServer<BinarySerializer>(
      [](OmniClient& client) -> folly::coro::Task<void> {
        auto protocolTry = client.getChannelProtocolId();
        EXPECT_FALSE(protocolTry.hasException());
        EXPECT_EQ(protocolTry.value(), protocol::T_BINARY_PROTOCOL);
        co_return;
      });
}

TEST_F(OmniClientTest, GlobalClientInterceptorInvoked) {
  // Verify that the interceptor registered via runtime::init() is actually
  // used by OmniClient when the flag is enabled.

  AddRequest request;
  request.num1() = 1;
  request.num2() = 41;

  testSend<CompactSerializer>("TestService", "add", request, 42);

  // Verify the registered onRequest interceptor was called
  ASSERT_EQ(gTracingInterceptor->requests().size(), 1);
  EXPECT_EQ(std::get<0>(gTracingInterceptor->requests()[0]), "TestService");
  EXPECT_EQ(std::get<1>(gTracingInterceptor->requests()[0]), "add");

  // Verify the registered onResponse interceptor was called
  ASSERT_EQ(gTracingInterceptor->responses().size(), 1);
  EXPECT_EQ(std::get<0>(gTracingInterceptor->responses()[0]), "TestService");
  EXPECT_EQ(std::get<1>(gTracingInterceptor->responses()[0]), "add");

  // And the request data is the same
  EXPECT_EQ(
      std::get<2>(gTracingInterceptor->requests()[0]),
      std::get<2>(gTracingInterceptor->responses()[0]));
}

TEST_F(OmniClientTest, GlobalClientInterceptorNotInvokedWhenFlagDisabled) {
  // Verify that even though an interceptor is registered, it is NOT used
  // when the flag is disabled.

  // Flag defaults to false, but be explicit
  THRIFT_FLAG_SET_MOCK(enable_python_client_interceptors, false);

  AddRequest request;
  request.num1() = 1;
  request.num2() = 41;

  testSend<CompactSerializer>("TestService", "add", request, 42);

  // Verify the interceptor was NOT called despite being registered
  EXPECT_TRUE(gTracingInterceptor->requests().empty());
  EXPECT_TRUE(gTracingInterceptor->responses().empty());

  THRIFT_FLAG_UNMOCK(enable_python_client_interceptors);
}

TEST_F(OmniClientTest, GlobalClientInterceptorOnResponseNotInvokedOneWayRpc) {
  // Verify that one way requests do not invoke the onResponse interceptor

  testOnewaySendHeaders<CompactSerializer>(
      "TestService", "oneway", EmptyRequest{});

  // Verify the registered onRequest interceptor was called
  ASSERT_EQ(gTracingInterceptor->requests().size(), 1);
  EXPECT_EQ(std::get<0>(gTracingInterceptor->requests()[0]), "TestService");
  EXPECT_EQ(std::get<1>(gTracingInterceptor->requests()[0]), "oneway");

  // Verify the onResponse was not
  EXPECT_TRUE(gTracingInterceptor->responses().empty());
}

TEST_F(OmniClientTest, GlobalClientInterceptorTestHeader) {
  // Verify that the interceptor registered via runtime::init() is actually
  // used by OmniClient when the flag is enabled.

  ReadHeaderRequest request;
  request.key() = kTestHeaderKey;

  testSendHeaders(
      "TestService",
      "readHeader",
      request,
      {{kTestHeaderKey, kTestHeaderValue}},
      kTestHeaderValue);

  // Verify the registered onRequest interceptor was called
  ASSERT_EQ(gTracingInterceptor->requests().size(), 1);
  EXPECT_EQ(std::get<0>(gTracingInterceptor->requests()[0]), "TestService");
  EXPECT_EQ(std::get<1>(gTracingInterceptor->requests()[0]), "readHeader");
  EXPECT_EQ(
      std::get<3>(gTracingInterceptor->requests()[0]).at(kTestHeaderKey),
      kTestHeaderValue);

  // Verify the registered onResponse interceptor was called
  ASSERT_EQ(gTracingInterceptor->requests().size(), 1);
  EXPECT_EQ(std::get<0>(gTracingInterceptor->responses()[0]), "TestService");
  EXPECT_EQ(std::get<1>(gTracingInterceptor->responses()[0]), "readHeader");
  EXPECT_EQ(
      std::get<3>(gTracingInterceptor->responses()[0]).at(kTestHeaderKey),
      kTestHeaderValue);

  // And the request data is the same
  EXPECT_EQ(
      std::get<2>(gTracingInterceptor->requests()[0]),
      std::get<2>(gTracingInterceptor->responses()[0]));
}

TEST_F(OmniClientTest, GlobalClientInterceptorOnResponseRunOnException) {
  // Verify that interceptor callbacks are run on exceptions

  // This request will throw an exception - use connectToServer directly
  // since testSend calls .value() which crashes on unset repsonse buffer
  connectToServer<CompactSerializer>(
      [](OmniClient& client) -> folly::coro::Task<void> {
        std::string args =
            CompactSerializer::serialize<std::string>(EmptyRequest{});
        auto data = apache::thrift::MethodMetadata::Data(
            "oops", apache::thrift::FunctionQualifier::Unspecified);
        auto resp = co_await client.semifuture_send(
            "TestService",
            "oops",
            args,
            std::move(data),
            {},
            {},
            co_await folly::coro::co_current_executor,
            RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);
        co_return;
      });

  // Verify the registered onRequest interceptor was called
  ASSERT_EQ(gTracingInterceptor->requests().size(), 1);
  EXPECT_EQ(std::get<0>(gTracingInterceptor->requests()[0]), "TestService");
  EXPECT_EQ(std::get<1>(gTracingInterceptor->requests()[0]), "oops");

  // Verify the onResponse was called with the exception we expected
  ASSERT_EQ(gTracingInterceptor->responses().size(), 1);
  EXPECT_EQ(std::get<0>(gTracingInterceptor->responses()[0]), "TestService");
  EXPECT_EQ(std::get<1>(gTracingInterceptor->responses()[0]), "oops");
  EXPECT_EQ(gTracingInterceptor->responseExceptions().size(), 1);

  // And the request data is the same
  EXPECT_EQ(
      std::get<2>(gTracingInterceptor->requests()[0]),
      std::get<2>(gTracingInterceptor->responses()[0]));
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  // Register our test interceptor with Thrift init.
  apache::thrift::runtime::InitOptions thriftOptions;
  thriftOptions.clientInterceptors.push_back(gTracingInterceptor);
  apache::thrift::runtime::init(std::move(thriftOptions));
  folly::Init follyInit(&argc, &argv);

  // Enable the client interceptor flag.
  THRIFT_FLAG_SET_MOCK(enable_python_client_interceptors, true);

  return RUN_ALL_TESTS();
}
