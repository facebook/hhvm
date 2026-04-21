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

#include <condition_variable>
#include <mutex>

#include <gtest/gtest.h>

#include <folly/coro/Sleep.h>
#include <folly/coro/Task.h>
#include <folly/executors/GlobalExecutor.h>
#include <folly/io/async/ScopedEventBaseThread.h>

#include <thrift/lib/cpp2/GeneratedCodeHelper.h>
#include <thrift/lib/cpp2/async/ServerBiDiStreamBridge.h>
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

// Implementation of ServerBiDiCallback similar to TestProducerCallback
// in TestStreamService.cpp. This callback uses ServerBiDiStreamBridge::getTask
// and ServerBiDiSinkBridge::getInput to handle flow control automatically.
// Works directly with StreamPayload to avoid needing external encoder/decoder.
class TestServerBiDiCallback
    : public ServerBiDiStreamFactory::ServerBiDiCallback {
 public:
  using StreamTransformationFactory =
      std::function<StreamTransformation<StreamPayload, StreamPayload>()>;

  TestServerBiDiCallback(
      StreamTransformation<StreamPayload, StreamPayload> streamTransformation,
      IdentityDecoder* decoder,
      IdentityEncoder* encoder,
      folly::Executor::KeepAlive<> executor)
      : transformFn_(std::move(streamTransformation)),
        executor_(std::move(executor)),
        decoder_(decoder),
        encoder_(encoder) {}

  ~TestServerBiDiCallback() override = default;

  void provideBiDiBridge(
      ServerBiDiStreamBridge::Ptr streamBridge,
      ServerBiDiSinkBridge::Ptr sinkBridge) override {
    // Move executor out so we don't hold the KeepAlive, which would prevent
    // server shutdown
    auto executor = std::move(executor_);
    auto task = ServerBiDiStreamBridge::getTask(
        streamBridge->copy(),
        folly::coro::co_invoke(
            std::move(transformFn_.func),
            ServerBiDiSinkBridge::getInput(sinkBridge->copy(), decoder_)),
        encoder_);
    folly::coro::co_withExecutor(executor, std::move(task)).start();
    delete this;
  }

 private:
  StreamTransformation<StreamPayload, StreamPayload> transformFn_;
  folly::Executor::KeepAlive<> executor_;
  IdentityDecoder* decoder_;
  IdentityEncoder* encoder_;
};

struct TestHandler : public AsyncProcessorFactory {
  using StreamTransformationFactory =
      std::function<StreamTransformation<StreamPayload, StreamPayload>()>;

  struct TestAsyncProcessor : public AsyncProcessor {
    explicit TestAsyncProcessor(
        StreamTransformationFactory streamTransformationFactory,
        IdentityDecoder* decoder,
        IdentityEncoder* encoder,
        bool useServerBiDiCallback = false)
        : streamTransformationFactory_(std::move(streamTransformationFactory)),
          decoder_(decoder),
          encoder_(encoder),
          useServerBiDiCallback_(useServerBiDiCallback) {}

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

      if (useServerBiDiCallback_) {
        auto* callback = new TestServerBiDiCallback(
            streamTransformationFactory_(), decoder_, encoder_, executor);
        ServerBiDiStreamFactory factory(callback, 100);
        req->sendBiDiReply(
            ResponsePayload{makeResponse("hello")}, std::move(factory));
      } else {
        ServerBiDiStreamFactory factory(
            streamTransformationFactory_(), *decoder_, *encoder_, executor);
        req->sendBiDiReply(
            ResponsePayload{makeResponse("hello")}, std::move(factory));
      }
    }

    void processInteraction(ServerRequest&&) override { std::terminate(); }

    StreamTransformationFactory streamTransformationFactory_;
    IdentityDecoder* decoder_;
    IdentityEncoder* encoder_;
    bool useServerBiDiCallback_;
  };

  std::unique_ptr<AsyncProcessor> getProcessor() override {
    return std::make_unique<TestAsyncProcessor>(
        streamTransformationFactory_,
        decoder_.get(),
        encoder_.get(),
        useServerBiDiCallback_);
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

  void useServerBiDiCallback(bool useServerBiDiCallback) {
    useServerBiDiCallback_ = useServerBiDiCallback;
  }

 private:
  bool useServerBiDiCallback_ = false;
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
      const StreamTransformationFactory& streamTransformationFactory,
      bool useServerBiDiCallback = false) {
    DCHECK(clientCallbackFactory);
    DCHECK(streamTransformationFactory);

    handler_->useStreamTransformationFactory(streamTransformationFactory);
    handler_->useServerBiDiCallback(useServerBiDiCallback);
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

              backgroundScope.add(
                  folly::coro::co_withExecutor(
                      folly::getGlobalCPUExecutor(),
                      folly::coro::co_invoke(
                          std::move(consumeFunc), std::move(gen))));
              co_return;
            });
      });
  backgroundScope.cleanup().wait();
}

TEST_F(BiDiBridgesTest, ClientCancelsStreamWhileTransformBlocksOnInput) {
  test(
      [](auto done) {
        auto client = new BiDiFiniteClient(0, 1, done);
        client->setSinkLimitAction(
            BiDiFiniteClient::SinkLimitAction::HOLD_OPEN);
        client->setStreamLimitAction(
            BiDiFiniteClient::StreamLimitAction::CANCEL_STREAM);
        return client;
      },
      []() -> StreamTransformation<StreamPayload, StreamPayload> {
        return StreamTransformation<StreamPayload, StreamPayload>(
            [](folly::coro::AsyncGenerator<StreamPayload&&> gen)
                -> folly::coro::AsyncGenerator<StreamPayload&&> {
              // Yield one item so the client receives it and cancels
              co_yield StreamPayload(makeResponse("trigger-cancel"), {});
              // Block on gen.next() - no sink data will ever arrive.
              // Without cancellation support, this hangs forever.
              while (auto item = co_await gen.next()) {
                co_yield std::move(*item);
              }
            });
      });
}

TEST_F(BiDiBridgesTest, BasicWithBiDiCallback) {
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
      },
      true);
}

// ==========================================================================
// Direct bridge-level tests for Pause/Resume support
// ==========================================================================

namespace {

/// Minimal StreamClientCallback that counts received items and supports
/// condition-variable-based waiting for a target count.
class CountingStreamClientCallback : public StreamClientCallback {
 public:
  bool onFirstResponse(
      FirstResponsePayload&&,
      folly::EventBase*,
      StreamServerCallback*) override {
    return true;
  }
  void onFirstResponseError(folly::exception_wrapper) override {}
  bool onStreamNext(StreamPayload&&) override {
    received.fetch_add(1, std::memory_order_relaxed);
    cv_.notify_one();
    return true;
  }
  void onStreamError(folly::exception_wrapper) override {}
  void onStreamComplete() override {}
  void resetServerCallback(StreamServerCallback&) override {}

  /// Block until received count reaches \p target, or fail after 5s.
  void waitForCount(int32_t target) {
    std::unique_lock lock(mu_);
    ASSERT_TRUE(cv_.wait_for(
        lock,
        std::chrono::seconds(5),
        [&] { return received.load(std::memory_order_relaxed) >= target; }))
        << "Timed out waiting for count " << target;
  }

  std::atomic<int32_t> received{0};

 private:
  std::mutex mu_;
  std::condition_variable cv_;
};

} // namespace

TEST(ServerBiDiStreamBridgePauseTest, PauseResumeSuspendsAndResumesStream) {
  folly::ScopedEventBaseThread evbThread;
  auto* evb = evbThread.getEventBase();

  CountingStreamClientCallback clientCb;
  auto* bridge =
      new apache::thrift::detail::ServerBiDiStreamBridge(&clientCb, evb);

  IdentityEncoder encoder;

  auto gen = []() -> folly::coro::AsyncGenerator<StreamPayload&&> {
    for (int i = 0;; ++i) {
      co_yield StreamPayload(
          apache::thrift::detail::test::makeResponse(std::to_string(i)), {});
    }
  }();

  auto taskSF = folly::coro::co_withExecutor(
                    folly::getGlobalCPUExecutor(),
                    apache::thrift::detail::ServerBiDiStreamBridge::getTask(
                        bridge->copy(), std::move(gen), &encoder))
                    .start();

  // Register the client-side consumer on the evb.
  evb->runInEventBaseThreadAndWait([&] { bridge->processClientMessages(); });

  // Grant credits to start the stream.  All bridge calls are dispatched on the
  // evb to mirror the transport-layer threading model and avoid data races with
  // processClientMessages().
  evb->runInEventBaseThreadAndWait([&] { bridge->onStreamRequestN(1000); });

  // Wait for some items to flow.
  clientCb.waitForCount(10);

  // Pause the stream.
  evb->runInEventBaseThreadAndWait([&] { bridge->pauseStream(); });

  // Allow time for the coroutine to observe the Pause message.
  std::this_thread::sleep_for(std::chrono::milliseconds(100)); // sleep override
  int32_t countAfterPause = clientCb.received.load();

  // Verify no more items arrive while paused.
  std::this_thread::sleep_for(std::chrono::milliseconds(200)); // sleep override
  EXPECT_EQ(clientCb.received.load(), countAfterPause)
      << "Stream must not produce items while paused";

  // Resume the stream.
  evb->runInEventBaseThreadAndWait([&] { bridge->resumeStream(); });

  // Wait for more items to arrive.
  clientCb.waitForCount(countAfterPause + 5);
  EXPECT_GT(clientCb.received.load(), countAfterPause)
      << "Stream should resume producing items after Resume";

  // Tear down.
  evb->runInEventBaseThreadAndWait([&] { bridge->onStreamCancel(); });
  std::move(taskSF).get();
}

TEST(ServerBiDiStreamBridgePauseTest, PauseThenCancelTerminatesCleanly) {
  folly::ScopedEventBaseThread evbThread;
  auto* evb = evbThread.getEventBase();

  CountingStreamClientCallback clientCb;
  auto* bridge =
      new apache::thrift::detail::ServerBiDiStreamBridge(&clientCb, evb);

  IdentityEncoder encoder;

  auto gen = []() -> folly::coro::AsyncGenerator<StreamPayload&&> {
    for (int i = 0;; ++i) {
      co_yield StreamPayload(
          apache::thrift::detail::test::makeResponse(std::to_string(i)), {});
    }
  }();

  auto taskSF = folly::coro::co_withExecutor(
                    folly::getGlobalCPUExecutor(),
                    apache::thrift::detail::ServerBiDiStreamBridge::getTask(
                        bridge->copy(), std::move(gen), &encoder))
                    .start();

  evb->runInEventBaseThreadAndWait([&] { bridge->processClientMessages(); });

  // All bridge calls are dispatched on the evb to mirror the transport-layer
  // threading model and avoid data races with processClientMessages().
  evb->runInEventBaseThreadAndWait([&] { bridge->onStreamRequestN(100); });

  // Wait for some items.
  clientCb.waitForCount(5);

  // Pause, then immediately cancel.
  evb->runInEventBaseThreadAndWait([&] {
    bridge->pauseStream();
    bridge->onStreamCancel();
  });

  // The coroutine must exit cleanly.
  ASSERT_NO_THROW(std::move(taskSF).get());
}

} // namespace apache::thrift
