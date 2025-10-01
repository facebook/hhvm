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
#include <folly/coro/Task.h>

#include <thrift/lib/cpp2/GeneratedCodeHelper.h>
#include <thrift/lib/cpp2/async/ServerBiDiStreamFactory.h>
#include <thrift/lib/cpp2/async/ServerCallbackStapler.h>
#include <thrift/lib/cpp2/async/Sink.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/async/tests/util/BiDiFiniteClient.h>
#include <thrift/lib/cpp2/async/tests/util/BiDiTestUtil.h>
#include <thrift/lib/cpp2/async/tests/util/Util.h>
#include <thrift/lib/cpp2/server/ServerFlags.h>

namespace apache::thrift {
using namespace ::testing;
using namespace apache::thrift::detail;
using namespace apache::thrift::detail::test;

class IdentityDecoder : public SinkElementDecoder<StreamPayload> {
 public:
  virtual ~IdentityDecoder() = default;
  folly::Try<StreamPayload> operator()(
      folly::Try<StreamPayload>&& payload) override {
    if (payload.hasException()) {
      return folly::Try<StreamPayload>(std::move(payload.exception()));
    };
    return folly::Try<StreamPayload>(std::move(payload.value()));
  }
};

class IdentityEncoder : public StreamElementEncoder<StreamPayload> {
 public:
  virtual ~IdentityEncoder() = default;
  folly::Try<StreamPayload> operator()(StreamPayload&& payload) override {
    return folly::Try<StreamPayload>(std::move(payload));
  }

  folly::Try<StreamPayload> operator()(folly::exception_wrapper&& ew) override {
    return folly::Try<StreamPayload>(ew);
  }
};

struct TestHandler : public AsyncProcessorFactory {
  using StreamTransformationFactory =
      std::function<StreamTransformation<StreamPayload, StreamPayload>()>;

  struct TestAsyncProcessor : public AsyncProcessor {
    explicit TestAsyncProcessor(
        StreamTransformationFactory streamTransformationFactory,
        IdentityDecoder* decoder,
        IdentityEncoder* encoder)
        : streamTransformationFactory_(std::move(streamTransformationFactory)),
          decoder_(decoder),
          encoder_(encoder) {}

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
      auto executor = ServerRequestHelper::executor(request);

      ServerBiDiStreamFactory factory(
          streamTransformationFactory_(), *decoder_, *encoder_, executor);

      req->sendBiDiReply(
          ResponsePayload{makeResponse("hello")}, std::move(factory));
    }

    void processInteraction(ServerRequest&&) override { std::terminate(); }

    StreamTransformationFactory streamTransformationFactory_;
    IdentityDecoder* decoder_;
    IdentityEncoder* encoder_;
  };

  std::unique_ptr<AsyncProcessor> getProcessor() override {
    return std::make_unique<TestAsyncProcessor>(
        streamTransformationFactory_, decoder_.get(), encoder_.get());
  }

  std::vector<ServiceHandlerBase*> getServiceHandlers() override { return {}; }

  CreateMethodMetadataResult createMethodMetadata() override {
    WildcardMethodMetadataMap wildcardMap;
    wildcardMap.wildcardMetadata = std::make_shared<WildcardMethodMetadata>(
        AsyncProcessorFactory::MethodMetadata::ExecutorType::EVB);
    wildcardMap.knownMethods = {};

    return wildcardMap;
  }

  void useStreamTransformationFactory(
      StreamTransformationFactory streamTransformationFactory) {
    streamTransformationFactory_ = std::move(streamTransformationFactory);
  }

 private:
  StreamTransformationFactory streamTransformationFactory_;
  std::unique_ptr<IdentityDecoder> decoder_ =
      std::make_unique<IdentityDecoder>();
  std::unique_ptr<IdentityEncoder> encoder_ =
      std::make_unique<IdentityEncoder>();
};

struct BiDiBridgesTest
    : public AsyncTestSetup<TestHandler, Client<TestSinkService>> {
  using StreamTransformationFactory =
      std::function<StreamTransformation<StreamPayload, StreamPayload>()>;
  using ClientCallbackFactory = std::function<BiDiClientCallback*(
      std::shared_ptr<CompletionSignal> done)>;

  void SetUp() override {
    FLAGS_thrift_allow_resource_pools_for_wildcards = true;
    AsyncTestSetup::SetUp();
  }

  void test(
      ClientCallbackFactory clientCallbackFactory,
      const StreamTransformationFactory& streamTransformationFactory) {
    DCHECK(clientCallbackFactory);
    DCHECK(streamTransformationFactory);

    handler_->useStreamTransformationFactory(streamTransformationFactory);
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

TEST_F(BiDiBridgesTest, Basic) {
  test(
      [](auto done) {
        auto client = new BiDiFiniteClient(5, 100, done);
        client->setSinkLimitAction(BiDiFiniteClient::SinkLimitAction::COMPLETE);
        return client;
      },
      []() -> StreamTransformation<StreamPayload, StreamPayload> {
        return StreamTransformation<StreamPayload, StreamPayload>(
            [](folly::coro::AsyncGenerator<StreamPayload&&> gen)
                -> folly::coro::AsyncGenerator<StreamPayload&&> {
              while (auto item = co_await gen.next()) {
                co_yield std::move(*item);
              }
            });
      });
}

TEST_F(BiDiBridgesTest, IgnoreInputProduceOutput) {
  test(
      [](auto done) {
        auto client = new BiDiFiniteClient(5, 100, done);
        client->setSinkLimitAction(BiDiFiniteClient::SinkLimitAction::COMPLETE);
        return client;
      },
      []() -> StreamTransformation<StreamPayload, StreamPayload> {
        return StreamTransformation<StreamPayload, StreamPayload>(
            [](folly::coro::AsyncGenerator<StreamPayload&&> gen)
                -> folly::coro::AsyncGenerator<StreamPayload&&> {
              for (int i = 0; i < 5; i++) {
                co_yield StreamPayload(makeResponse(std::to_string(i)), {});
              }
            });
      });
}

TEST_F(BiDiBridgesTest, ConsumeInputIgnoreOutput) {
  test(
      [](auto done) {
        auto client = new BiDiFiniteClient(5, 100, done);
        client->setSinkLimitAction(BiDiFiniteClient::SinkLimitAction::COMPLETE);
        return client;
      },
      []() -> StreamTransformation<StreamPayload, StreamPayload> {
        return StreamTransformation<StreamPayload, StreamPayload>(
            [](folly::coro::AsyncGenerator<StreamPayload&&> gen)
                -> folly::coro::AsyncGenerator<StreamPayload&&> {
              while (auto item = co_await gen.next()) {
                LOG(INFO) << "INPUT";
              }
            });
      });
}

TEST_F(BiDiBridgesTest, ClientErrorsTheSink) {
  test(
      [](auto done) {
        auto client = new BiDiFiniteClient(5, 100, done);
        client->setSinkLimitAction(BiDiFiniteClient::SinkLimitAction::ERROR);
        return client;
      },
      []() -> StreamTransformation<StreamPayload, StreamPayload> {
        return StreamTransformation<StreamPayload, StreamPayload>(
            [](folly::coro::AsyncGenerator<StreamPayload&&> gen)
                -> folly::coro::AsyncGenerator<StreamPayload&&> {
              try {
                while (auto item = co_await gen.next()) {
                  co_yield std::move(*item);
                }
              } catch (...) {
                LOG(INFO) << "Exception got caught by handler!";
                throw;
              }
            });
      });
}

TEST_F(BiDiBridgesTest, ClientCancelsTheStream) {
  test(
      [](auto done) {
        auto client = new BiDiFiniteClient(5, 2, done);
        client->setSinkLimitAction(BiDiFiniteClient::SinkLimitAction::COMPLETE);
        client->setStreamLimitAction(
            BiDiFiniteClient::StreamLimitAction::CANCEL_STREAM);
        return client;
      },
      []() -> StreamTransformation<StreamPayload, StreamPayload> {
        return StreamTransformation<StreamPayload, StreamPayload>(
            [](folly::coro::AsyncGenerator<StreamPayload&&> gen)
                -> folly::coro::AsyncGenerator<StreamPayload&&> {
              while (auto item = co_await gen.next()) {
                co_yield std::move(*item);
              }
            });
      });
}

TEST_F(BiDiBridgesTest, HandoffInput) {
  folly::coro::AsyncScope backgroundScope;
  test(
      [](auto done) {
        auto client = new BiDiFiniteClient(5, 100, done);
        client->setSinkLimitAction(BiDiFiniteClient::SinkLimitAction::COMPLETE);
        return client;
      },
      [&backgroundScope]()
          -> StreamTransformation<StreamPayload, StreamPayload> {
        return StreamTransformation<StreamPayload, StreamPayload>(
            [&backgroundScope](folly::coro::AsyncGenerator<StreamPayload&&> gen)
                -> folly::coro::AsyncGenerator<StreamPayload&&> {
              auto consumeFunc =
                  [](folly::coro::AsyncGenerator<StreamPayload&&> gen)
                  -> folly::coro::Task<void> {
                while (auto item = co_await gen.next()) {
                  LOG(INFO) << "INPUT";
                }
              };

              backgroundScope.add(folly::coro::co_withExecutor(
                  folly::getGlobalCPUExecutor(),
                  folly::coro::co_invoke(
                      std::move(consumeFunc), std::move(gen))));
              co_return;
            });
      });
  backgroundScope.cleanup().wait();
}

} // namespace apache::thrift
