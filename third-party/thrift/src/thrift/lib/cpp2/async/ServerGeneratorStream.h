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

namespace apache {
namespace thrift {
namespace detail {
namespace test {
class TestProducerCallback;
}

class ServerStreamConsumer {
 public:
  virtual ~ServerStreamConsumer() = default;
  virtual void consume() = 0;
  virtual void canceled() = 0;
};

template <typename Baton>
class ServerStreamConsumerBaton final : public ServerStreamConsumer {
 public:
  void consume() override { baton.post(); }
  void canceled() override { std::terminate(); }
  Baton baton;
};

struct StreamControl {
  enum Code : int32_t { CANCEL = -1, PAUSE = -2, RESUME = -3 };
};

class ServerGeneratorStream;

// This template explicitly instantiated in ServerGeneratorStream.cpp
extern template class TwoWayBridge<
    ServerGeneratorStream,
    folly::Try<StreamPayload>,
    ServerStreamConsumer,
    int64_t,
    ServerGeneratorStream>;

class ServerGeneratorStream : public TwoWayBridge<
                                  ServerGeneratorStream,
                                  folly::Try<StreamPayload>,
                                  ServerStreamConsumer,
                                  int64_t,
                                  ServerGeneratorStream>,
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

  ~ServerGeneratorStream() override;

#if FOLLY_HAS_COROUTINES
 private:
  template <bool WithHeader, typename T>
  static folly::coro::Task<> fromAsyncGeneratorImpl(
      std::unique_ptr<ServerGeneratorStream, Deleter> stream,
      StreamElementEncoder<T>* encode,
      folly::coro::AsyncGenerator<
          std::conditional_t<WithHeader, MessageVariant<T>, T>&&> gen,
      TileStreamGuard);

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

  bool wait(ServerStreamConsumer* consumer);

  void publish(folly::Try<StreamPayload>&& payload);

 private:
  ServerGeneratorStream(
      StreamClientCallback* clientCallback, folly::EventBase* clientEb);

  bool onStreamRequestN(uint64_t credits) override;

  void onStreamCancel() override;

  void resetClientCallback(StreamClientCallback& clientCallback) override;

  void pauseStream() override;

  void resumeStream() override;

  void processPayloads();

  StreamClientCallback* streamClientCallback_;
  folly::EventBase* clientEventBase_;
#if FOLLY_HAS_COROUTINES
  folly::CancellationSource cancelSource_;
#endif

  friend class test::TestProducerCallback;
};

} // namespace detail
} // namespace thrift
} // namespace apache

#include <thrift/lib/cpp2/async/ServerGeneratorStream-inl.h>
