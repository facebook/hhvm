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

#pragma once

#include <folly/Portability.h>
#include <folly/Try.h>
#include <folly/coro/AsyncGenerator.h>
#include <folly/coro/Baton.h>
#include <folly/coro/Invoke.h>
#include <folly/coro/Task.h>
#include <thrift/lib/cpp2/async/ServerStreamDetail.h>
#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/async/TwoWayBridge.h>
#include <thrift/lib/cpp2/async/TwoWayBridgeUtil.h>

namespace apache::thrift::detail {
namespace test {
class TestProducerCallback;
}

struct StreamControl {
  enum Code : int32_t { CANCEL = -1, PAUSE = -2, RESUME = -3 };
};

class ServerGeneratorStreamBridge;

// This template explicitly instantiated in ServerGeneratorStreamBridge.cpp
extern template class TwoWayBridge<
    ServerGeneratorStreamBridge,
    folly::Try<StreamPayload>,
    QueueConsumer,
    int64_t,
    ServerGeneratorStreamBridge>;

class ServerGeneratorStreamBridge : public TwoWayBridge<
                                        ServerGeneratorStreamBridge,
                                        folly::Try<StreamPayload>,
                                        QueueConsumer,
                                        int64_t,
                                        ServerGeneratorStreamBridge>,
                                    private StreamServerCallback {
 public:
  class ProducerCallback {
   public:
    // Producer can call stream->publish() to send serialized stream chunks.
    // Producer needs to wait for messages from client (eg.
    // credits/cancellation) by calling stream->wait() and then using
    // stream->getMessages() to get the messages once they are ready.
    // Producer needs to call stream->serverClose() and destroy the stream
    // `Ptr` when it is done.
    virtual void provideStream(Ptr stream) = 0;
    virtual ~ProducerCallback() = default;
  };
  static ServerStreamFactory fromProducerCallback(ProducerCallback* cb);

  ~ServerGeneratorStreamBridge() override;

#if FOLLY_HAS_COROUTINES
 private:
  template <bool WithHeader, typename T>
  static folly::coro::Task<> fromAsyncGeneratorImpl(
      std::unique_ptr<ServerGeneratorStreamBridge, Deleter> stream,
      StreamElementEncoder<T>* encode,
      folly::coro::AsyncGenerator<
          std::conditional_t<WithHeader, MessageVariant<T>, T>&&> gen,
      TileStreamGuard interaction);

 public:
  template <bool WithHeader, typename T>
  static ServerStreamFn<T> fromAsyncGenerator(
      folly::coro::AsyncGenerator<
          std::conditional_t<WithHeader, MessageVariant<T>, T>&&>&& gen);
#endif // FOLLY_HAS_COROUTINES

  void consume();

  void canceled();

  void close();

  ServerQueue getMessages();

  bool wait(QueueConsumer* consumer);

  void publish(folly::Try<StreamPayload>&& payload);

 private:
  ServerGeneratorStreamBridge(
      StreamClientCallback* clientCallback,
      folly::EventBase* clientEb,
      std::shared_ptr<ContextStack> contextStack = nullptr);

  bool onStreamRequestN(uint64_t credits) override;

  void onStreamCancel() override;

  void resetClientCallback(StreamClientCallback& clientCallback) override;

  void pauseStream() override;

  void resumeStream() override;

  void processPayloads();

  // Helper methods to encapsulate ContextStack usage
  void notifyStreamSubscribe(const TileStreamGuard& interaction);
  void notifyStreamFinally(
      folly::CancellationSource& cancelSource,
      const folly::exception_wrapper& exception = {});
  void notifyStreamPause(details::STREAM_PAUSE_REASON reason);
  void notifyStreamResumeReceive();
  void notifyStreamCredit(int64_t credits);
  void notifyStreamNext();
  void notifyStreamError(const folly::exception_wrapper& exception);

  StreamClientCallback* streamClientCallback_;
  folly::EventBase* clientEventBase_;
  std::shared_ptr<ContextStack> contextStack_;
#if FOLLY_HAS_COROUTINES
  folly::CancellationSource cancelSource_;
#endif

  friend class test::TestProducerCallback;
};

} // namespace apache::thrift::detail

#include <thrift/lib/cpp2/async/ServerGeneratorStreamBridge-inl.h>
