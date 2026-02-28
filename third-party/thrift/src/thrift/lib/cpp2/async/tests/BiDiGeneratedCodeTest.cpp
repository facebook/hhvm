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

#include <gtest/gtest.h>

#include <folly/coro/Sleep.h>

#include <thrift/lib/cpp2/GeneratedCodeHelper.h>
#include <thrift/lib/cpp2/async/tests/util/BiDiConfigurableServer.h>
#include <thrift/lib/cpp2/async/tests/util/BiDiEchoServer.h>
#include <thrift/lib/cpp2/async/tests/util/BiDiFiniteClient.h>
#include <thrift/lib/cpp2/async/tests/util/BiDiFiniteServer.h>
#include <thrift/lib/cpp2/async/tests/util/BiDiSimplifiedEchoClient.h>
#include <thrift/lib/cpp2/async/tests/util/BiDiSimplifiedEchoServer.h>
#include <thrift/lib/cpp2/async/tests/util/BiDiTestUtil.h>
#include <thrift/lib/cpp2/async/tests/util/Util.h>
#include <thrift/lib/cpp2/server/ServerFlags.h>

namespace apache::thrift {

using namespace apache::thrift::detail::test;

using EchoServer = BiDiEchoServer;
using ConfigurableServer = BiDiConfigurableServer;
using FiniteServer = BiDiFiniteServer;
using FiniteClient = BiDiFiniteClient;
using SimplifiedEchoClient = BiDiSimplifiedEchoClient;
using SimplifiedEchoServer = BiDiSimplifiedEchoServer;

struct TestHandler : public AsyncProcessorFactory {
  using ServerCallbackFactory = std::function<BiDiServerCallbackPtr()>;

  struct TestAsyncProcessor : public AsyncProcessor {
    explicit TestAsyncProcessor(ServerCallbackFactory serverCallbackFactory)
        : serverCallbackFactory_(std::move(serverCallbackFactory)) {}

    void processSerializedCompressedRequestWithMetadata(
        ResponseChannelRequest::UniquePtr,
        SerializedCompressedRequest&&,
        const MethodMetadata&,
        protocol::PROTOCOL_TYPES,
        Cpp2RequestContext*,
        folly::EventBase*,
        concurrency::ThreadManager*) override {
      LOG(FATAL)
          << "This method shouldn't be called with ResourcePools enabled";
    }

    void executeRequest(
        ServerRequest&& request,
        const AsyncProcessorFactory::MethodMetadata& /* methodMetadata */)
        override {
      auto req = std::move(request.request());
      auto* eb =
          apache::thrift::detail::ServerRequestHelper::eventBase(request);
      std::ignore = ResponseChannelRequest::sendBiDiReply(
          std::move(req),
          eb,
          ResponsePayload{makeResponse("hello")},
          serverCallbackFactory_());
    }

    void processInteraction(ServerRequest&&) override { std::terminate(); }

    ServerCallbackFactory serverCallbackFactory_;
  };

  std::unique_ptr<AsyncProcessor> getProcessor() override {
    return std::make_unique<TestAsyncProcessor>(serverCallbackFactory_);
  }

  std::vector<ServiceHandlerBase*> getServiceHandlers() override { return {}; }

  CreateMethodMetadataResult createMethodMetadata() override {
    WildcardMethodMetadataMap wildcardMap;
    wildcardMap.wildcardMetadata = std::make_shared<WildcardMethodMetadata>(
        AsyncProcessorFactory::MethodMetadata::ExecutorType::EVB);
    wildcardMap.knownMethods = {};

    return wildcardMap;
  }

  void useServerCallbackFactory(ServerCallbackFactory serverCallbackFactory) {
    serverCallbackFactory_ = std::move(serverCallbackFactory);
  }

 private:
  ServerCallbackFactory serverCallbackFactory_;
};

struct BiDiGeneratedCodeTest
    : public AsyncTestSetup<TestHandler, Client<TestBiDiService>> {
  using ServerCallbackFactory = std::function<BiDiServerCallback*()>;
  using ClientCallbackFactory = std::function<BiDiClientCallback*(
      std::shared_ptr<CompletionSignal> done)>;

  void SetUp() override {
    FLAGS_thrift_allow_resource_pools_for_wildcards = true;
    AsyncTestSetup::SetUp();
  }

  void test(
      ClientCallbackFactory clientCallbackFactory,
      ServerCallbackFactory serverCallbackFactory) {
    DCHECK(clientCallbackFactory);
    DCHECK(serverCallbackFactory);

    handler_->useServerCallbackFactory(
        [serverCallbackFactory = std::move(serverCallbackFactory)] {
          DCHECK(serverCallbackFactory);
          auto callback = serverCallbackFactory();
          DCHECK(callback);
          return BiDiServerCallbackPtr(callback);
        });
    connectToServer(
        [clientCallbackFactory = std::move(clientCallbackFactory)](
            Client<TestBiDiService>& client) -> folly::coro::Task<void> {
          DCHECK(clientCallbackFactory);
          auto completion = std::make_shared<CompletionSignal>();
          auto clientCallback = clientCallbackFactory(completion);
          DCHECK(clientCallback);

          auto* channel = client.getChannel();
          channel->sendRequestBiDi(
              RpcOptions{}
                  .setTimeout(std::chrono::milliseconds(1000))
                  .setChunkBufferSize(7),
              "test",
              SerializedRequest{makeRequest("test")},
              std::make_shared<transport::THeader>(),
              clientCallback,
              nullptr);
          co_await completion->waitForDone();
        });
  }
};

TEST_F(BiDiGeneratedCodeTest, StreamCompletesThenSinkCompletes) {
  handler_->useServerCallbackFactory([] {
    auto server = new FiniteServer(1, 10);
    server->setStreamLimitAction(FiniteServer::StreamLimitAction::COMPLETE);
    return BiDiServerCallbackPtr(server);
  });

  connectToServer(
      [](Client<TestBiDiService>& client) -> folly::coro::Task<void> {
        auto [sink, stream] = co_await client.co_echo();
        auto gen = std::move(stream).toAsyncGenerator();
        EXPECT_EQ("stream-payload", *co_await gen.next());
        EXPECT_FALSE(co_await gen.next());
        co_await sink.sink([&]() -> folly::coro::AsyncGenerator<std::string&&> {
          co_yield "sink-payload";
        }());
      });
}

TEST_F(BiDiGeneratedCodeTest, SinkCompletesThenStreamCompletes) {
  handler_->useServerCallbackFactory([] {
    auto server = new FiniteServer(1, 10);
    server->setStreamLimitAction(FiniteServer::StreamLimitAction::COMPLETE);
    return BiDiServerCallbackPtr(server);
  });

  connectToServer(
      [](Client<TestBiDiService>& client) -> folly::coro::Task<void> {
        auto [sink, stream] = co_await client.co_echo();
        co_await sink.sink([&]() -> folly::coro::AsyncGenerator<std::string&&> {
          co_yield "sink-payload";
        }());
        auto gen = std::move(stream).toAsyncGenerator();
        EXPECT_EQ("stream-payload", *co_await gen.next());
        EXPECT_FALSE(co_await gen.next());
      });
}

TEST_F(BiDiGeneratedCodeTest, StreamErrorsButSinkCompletes) {
  handler_->useServerCallbackFactory([] {
    auto server = new FiniteServer(3, 100);
    server->setStreamLimitAction(FiniteServer::StreamLimitAction::ERROR);
    return BiDiServerCallbackPtr(server);
  });

  connectToServer(
      [](Client<TestBiDiService>& client) -> folly::coro::Task<void> {
        auto [sink, stream] = co_await client.co_echo();
        auto gen = std::move(stream).toAsyncGenerator();

        // Stream should error after limited messages
        bool streamErrored = false;
        try {
          while (true) {
            auto response = co_await gen.next();
            if (!response.has_value()) {
              break;
            }
          }
        } catch (const std::exception&) {
          streamErrored = true;
        }
        EXPECT_TRUE(streamErrored);

        // Sink should still complete successfully
        co_await sink.sink([&]() -> folly::coro::AsyncGenerator<std::string&&> {
          for (int i = 0; i < 5; ++i) {
            co_yield folly::to<std::string>("sink-", i);
          }
        }());
      });
}

TEST_F(BiDiGeneratedCodeTest, SinkErrorsButStreamCompletes) {
  handler_->useServerCallbackFactory([] {
    auto server = new ConfigurableServer();
    server->setSinkErrorAction(
        ConfigurableServer::SinkErrorAction::CONTINUE_STREAM);
    return BiDiServerCallbackPtr(server);
  });

  connectToServer(
      [](Client<TestBiDiService>& client) -> folly::coro::Task<void> {
        auto [sink, stream] = co_await client.co_echo();
        auto gen = std::move(stream).toAsyncGenerator();

        // Start sink that will error after a few messages
        try {
          co_await sink.sink(
              [&]() -> folly::coro::AsyncGenerator<std::string&&> {
                for (int i = 0; i < 3; ++i) {
                  co_yield folly::to<std::string>("sink-", i);
                }
                throw std::runtime_error("sink error");
              }());
        } catch (const std::exception&) {
        }

        // Stream echoes sink payloads then sends 2 more
        for (int i = 0; i < 3; ++i) {
          EXPECT_EQ(folly::to<std::string>("sink-", i), *co_await gen.next());
        }
        EXPECT_EQ("stream-payload", *co_await gen.next());
        EXPECT_EQ("stream-payload", *co_await gen.next());
        EXPECT_FALSE(co_await gen.next());
      });
}

TEST_F(BiDiGeneratedCodeTest, StreamErrorsAndSinkCompletes) {
  handler_->useServerCallbackFactory([] {
    auto server = new FiniteServer(2, 100);
    server->setStreamLimitAction(FiniteServer::StreamLimitAction::ERROR);
    return BiDiServerCallbackPtr(server);
  });

  connectToServer(
      [](Client<TestBiDiService>& client) -> folly::coro::Task<void> {
        auto [sink, stream] = co_await client.co_echo();
        auto gen = std::move(stream).toAsyncGenerator();

        // First drain the stream until it errors
        bool streamErrored = false;
        try {
          while (true) {
            EXPECT_TRUE(co_await gen.next());
          }
        } catch (const std::exception&) {
          streamErrored = true;
        }
        EXPECT_TRUE(streamErrored);

        // Now that the stream has errored, sink should still complete
        // successfully
        co_await sink.sink([&]() -> folly::coro::AsyncGenerator<std::string&&> {
          for (int i = 0; i < 3; ++i) {
            co_yield folly::to<std::string>("sink-", i);
          }
        }());
      });
}

} // namespace apache::thrift
