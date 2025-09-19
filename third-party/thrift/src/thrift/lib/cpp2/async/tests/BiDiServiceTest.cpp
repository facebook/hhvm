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

struct LowLevelBiDiServiceTest
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

TEST_F(LowLevelBiDiServiceTest, Basic) {
  test(
      [](auto done) {
        auto client = new FiniteClient(5, 100, done);
        client->setSinkLimitAction(FiniteClient::SinkLimitAction::COMPLETE);
        return client;
      },
      [] {
        auto server = new EchoServer();
        return server;
      });
}

TEST_F(LowLevelBiDiServiceTest, SinkCompletesButStreamContinues) {
  test(
      [](auto done) {
        auto client = new FiniteClient(3, 100, done);
        client->setSinkLimitAction(FiniteClient::SinkLimitAction::COMPLETE);
        return client;
      },
      [] {
        auto server = new FiniteServer(5, 100);
        server->setStreamLimitAction(FiniteServer::StreamLimitAction::COMPLETE);
        return server;
      });
}

TEST_F(LowLevelBiDiServiceTest, StreamCompletesButSinkContinues) {
  test(
      [](auto done) {
        auto client = new FiniteClient(5, 100, done);
        client->setSinkLimitAction(FiniteClient::SinkLimitAction::COMPLETE);
        return client;
      },
      [] {
        auto server = new FiniteServer(3, 100);
        server->setStreamLimitAction(FiniteServer::StreamLimitAction::COMPLETE);
        return server;
      });
}

TEST_F(LowLevelBiDiServiceTest, SinkErrorsButStreamCompletes) {
  test(
      [](auto done) {
        auto client = new FiniteClient(3, 100, done);
        client->setSinkLimitAction(FiniteClient::SinkLimitAction::ERROR);
        return client;
      },
      [] {
        auto server = new ConfigurableServer();
        server->setSinkErrorAction(
            ConfigurableServer::SinkErrorAction::CONTINUE_STREAM);
        return server;
      });
}

TEST_F(LowLevelBiDiServiceTest, StreamErrorsButSinkCompletes) {
  test(
      [](auto done) {
        auto client = new FiniteClient(5, 100, done);
        client->setSinkLimitAction(FiniteClient::SinkLimitAction::COMPLETE);
        return client;
      },
      [] {
        auto server = new FiniteServer(3, 100);
        server->setStreamLimitAction(FiniteServer::StreamLimitAction::ERROR);
        return server;
      });
}

TEST_F(LowLevelBiDiServiceTest, SinkGetsCancelledButStreamCompletes) {
  test(
      [](auto done) {
        auto client = new FiniteClient(5, 100, done);
        client->setSinkLimitAction(FiniteClient::SinkLimitAction::COMPLETE);
        return client;
      },
      [] {
        auto server = new FiniteServer(5, 3);
        server->setSinkLimitAction(FiniteServer::SinkLimitAction::CANCEL_SINK);
        server->setStreamLimitAction(FiniteServer::StreamLimitAction::COMPLETE);
        return server;
      });
}

TEST_F(LowLevelBiDiServiceTest, StreamGetsCancelledButSinkCompletes) {
  test(
      [](auto done) {
        auto client = new FiniteClient(5, 3, done);
        client->setSinkLimitAction(FiniteClient::SinkLimitAction::COMPLETE);
        client->setStreamLimitAction(
            FiniteClient::StreamLimitAction::CANCEL_STREAM);
        return client;
      },
      [] {
        auto server = new FiniteServer(10, 10);
        server->setStreamLimitAction(FiniteServer::StreamLimitAction::COMPLETE);
        return server;
      });
}

TEST_F(LowLevelBiDiServiceTest, SinkErrorsThenStreamErrors) {
  test(
      [](auto done) {
        auto client = new FiniteClient(3, 100, done);
        client->setSinkLimitAction(FiniteClient::SinkLimitAction::ERROR);
        return client;
      },
      [] {
        auto server = new ConfigurableServer();
        server->setSinkErrorAction(
            ConfigurableServer::SinkErrorAction::ERROR_STREAM);
        return server;
      });
}

TEST_F(LowLevelBiDiServiceTest, StreamGetsCancelledThenSinkGetsCancelled) {
  test(
      [](auto done) {
        auto client = new FiniteClient(3, 100, done);
        client->setSinkLimitAction(FiniteClient::SinkLimitAction::COMPLETE);
        return client;
      },
      [] {
        auto server = new ConfigurableServer();
        server->setSinkCompleteAction(
            ConfigurableServer::SinkCompleteAction::COMPLETE_STREAM);
        server->setStreamCancelAction(
            ConfigurableServer::StreamCancelAction::CANCEL_SINK);
        return server;
      });
}

TEST_F(LowLevelBiDiServiceTest, BothSinkAndStreamGetCancelledOnRequestN) {
  class CustomClient : public BiDiSimplifiedEchoClient {
   public:
    explicit CustomClient(std::shared_ptr<CompletionSignal> done)
        : BiDiSimplifiedEchoClient(3, std::move(done)) {}

    bool onSinkRequestN(uint64_t n) override {
      DestructionGuard dg(this);
      LOG(INFO) << "Client received sink requestN " << n
                << " and will cancel stream";
      if (isStreamOpen()) {
        closeStream();
        std::ignore = serverCallback_->onStreamCancel();
      }
      return isAlive();
    }
  };

  class CustomServer : public BiDiSimplifiedEchoServer {
    bool onStreamRequestN(uint64_t n) override {
      DestructionGuard dg(this);
      LOG(INFO) << "Server received stream requestN " << n
                << " and will cancel sink";
      if (isSinkOpen()) {
        std::ignore = clientCallback_->onSinkRequestN(n);
        closeSink();
        std::ignore = clientCallback_->onSinkCancel();
      }
      return isAlive();
    }
  };

  test(
      [](auto done) { return new CustomClient(std::move(done)); },
      [] { return new CustomServer(); });
}

TEST_F(LowLevelBiDiServiceTest, BothSinkAndStreamGetCancelledAfterOnePayload) {
  class CustomClient : public BiDiSimplifiedEchoClient {
   public:
    explicit CustomClient(std::shared_ptr<CompletionSignal> done)
        : BiDiSimplifiedEchoClient(3, std::move(done)) {}

    bool onStreamNext(StreamPayload&&) override {
      DestructionGuard dg(this);
      LOG(INFO) << "Client received stream chunk #" << chunksReceived_++;

      if (isSinkOpen()) {
        std::ignore = serverCallback_->onSinkNext(makeSinkPayload());
      }

      closeStream();
      std::ignore = serverCallback_->onStreamCancel();

      return isAlive();
    }
  };

  class CustomServer : public BiDiSimplifiedEchoServer {
    bool onSinkNext(StreamPayload&&) override {
      DestructionGuard dg(this);
      LOG(INFO) << "Server received sink chunk #" << chunksReceived_++;

      if (isStreamOpen()) {
        std::ignore = clientCallback_->onStreamNext(makeStreamPayload());
      }

      closeSink();
      std::ignore = clientCallback_->onSinkCancel();

      return isAlive();
    }
  };

  test(
      [](auto done) { return new CustomClient(std::move(done)); },
      [] { return new CustomServer(); });
}

TEST_F(LowLevelBiDiServiceTest, BothSidesError) {
  class CustomClient : public BiDiSimplifiedEchoClient {
   public:
    explicit CustomClient(std::shared_ptr<CompletionSignal> done)
        : BiDiSimplifiedEchoClient(3, std::move(done)) {}
    bool onFirstResponse(
        FirstResponsePayload&&,
        folly::EventBase*,
        BiDiServerCallback* serverCallback) override {
      DestructionGuard dg(this);
      firstResponseReceived();
      serverCallback_ = serverCallback;
      LOG(INFO) << "Client received initial response";

      closeSink();
      std::ignore = serverCallback_->onSinkError(
          folly::make_exception_wrapper<std::runtime_error>("error"));
      return isAlive();
    }
  };

  test(
      [](auto done) { return new CustomClient(std::move(done)); },
      [] { return new BiDiSimplifiedEchoServer(); });
}

//
// First response error tests
//
struct TestHandlerThatReturnsError : public AsyncProcessorFactory {
  struct TestAsyncProcessorThatReturnsError : public AsyncProcessor {
    void processSerializedCompressedRequestWithMetadata(
        ResponseChannelRequest::UniquePtr,
        SerializedCompressedRequest&&,
        const MethodMetadata&,
        protocol::PROTOCOL_TYPES,
        Cpp2RequestContext*,
        folly::EventBase*,
        concurrency::ThreadManager*) override {
      LOG(FATAL)
          << "processSerializedCompressedRequestWithMetadata shouldn't be called in the test";
    }

    void executeRequest(
        ServerRequest&& request,
        const AsyncProcessorFactory::MethodMetadata& /* methodMetadata */)
        override {
      auto req = std::move(request.request());
      LOG(INFO) << "Server sends first response error";
      req->sendErrorWrapped(
          folly::make_exception_wrapper<TApplicationException>(
              "first response error"),
          "");
    }

    void processInteraction(ServerRequest&&) override { std::terminate(); }
  };

  std::unique_ptr<AsyncProcessor> getProcessor() override {
    return std::make_unique<TestAsyncProcessorThatReturnsError>();
  }

  std::vector<ServiceHandlerBase*> getServiceHandlers() override { return {}; }

  CreateMethodMetadataResult createMethodMetadata() override {
    WildcardMethodMetadataMap wildcardMap;
    wildcardMap.wildcardMetadata = std::make_shared<WildcardMethodMetadata>(
        AsyncProcessorFactory::MethodMetadata::ExecutorType::EVB);
    wildcardMap.knownMethods = {};
    return wildcardMap;
  }
};

struct FirstResponseErrorBiDiTest : public AsyncTestSetup<
                                        TestHandlerThatReturnsError,
                                        Client<TestBiDiService>> {
  using ClientCallbackFactory = std::function<BiDiClientCallback*(
      std::shared_ptr<CompletionSignal> done)>;

  void SetUp() override {
    FLAGS_thrift_allow_resource_pools_for_wildcards = true;
    AsyncTestSetup::SetUp();
  }

  void test(ClientCallbackFactory clientCallbackFactory) {
    DCHECK(clientCallbackFactory);
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

TEST_F(FirstResponseErrorBiDiTest, FirstResponseError) {
  test([](auto completion) { return new SimplifiedEchoClient(3, completion); });
}

//
// Tests for the generated code
//

struct GeneratedCodeBiDiServiceTest : LowLevelBiDiServiceTest {};

TEST_F(GeneratedCodeBiDiServiceTest, StreamCompletesThenSinkCompletes) {
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

TEST_F(GeneratedCodeBiDiServiceTest, SinkCompletesThenStreamCompletes) {
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

TEST_F(GeneratedCodeBiDiServiceTest, StreamErrorsButSinkCompletes) {
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

TEST_F(GeneratedCodeBiDiServiceTest, SinkErrorsButStreamCompletes) {
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

TEST_F(GeneratedCodeBiDiServiceTest, StreamErrorsAndSinkCompletes) {
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
