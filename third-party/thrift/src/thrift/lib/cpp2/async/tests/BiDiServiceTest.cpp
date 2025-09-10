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
#include <thrift/lib/cpp2/async/tests/util/BiDiTestUtil.h>
#include <thrift/lib/cpp2/async/tests/util/Util.h>
#include <thrift/lib/cpp2/server/ServerFlags.h>

namespace apache::thrift {

using namespace apache::thrift::detail::test;

using EchoServer = BiDiEchoServer;
using ConfigurableServer = BiDiConfigurableServer;
using FiniteServer = BiDiFiniteServer;
using FiniteClient = BiDiFiniteClient;

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
    : public AsyncTestSetup<TestHandler, Client<TestSinkService>> {
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
            Client<TestSinkService>& client) -> folly::coro::Task<void> {
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

} // namespace apache::thrift
