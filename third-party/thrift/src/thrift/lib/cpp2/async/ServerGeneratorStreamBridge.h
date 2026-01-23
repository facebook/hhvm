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

#if FOLLY_HAS_COROUTINES
  template <bool WithHeader, typename T>
  static ServerStreamFn<T> fromAsyncGenerator(
      folly::coro::AsyncGenerator<
          std::conditional_t<WithHeader, MessageVariant<T>, T>&&>&& gen) {
    return [gen = std::move(gen)](
               folly::Executor::KeepAlive<> serverExecutor,
               StreamElementEncoder<T>* encode) mutable {
      return ServerStreamFactory([gen = std::move(gen),
                                  serverExecutor = std::move(serverExecutor),
                                  encode](
                                     FirstResponsePayload&&
                                         firstResponsePayload,
                                     StreamClientCallback* clientCallback,
                                     folly::EventBase* evb,
                                     TilePtr&& interaction,
                                     std::shared_ptr<ContextStack>
                                         contextStack) mutable {
        DCHECK(evb->isInEventBaseThread());

        auto stream =
            new ServerGeneratorStreamBridge(clientCallback, evb, contextStack);
        auto streamPtr = stream->copy();
        fromAsyncGeneratorImpl<WithHeader>(
            std::move(streamPtr),
            encode,
            std::move(gen),
            TileStreamGuard::transferFrom(std::move(interaction)),
            contextStack)
            .scheduleOn(std::move(serverExecutor))
            .start([stream = stream->copy()](folly::Try<folly::Unit> t) {
              if (t.hasException()) {
                LOG(ERROR)
                    << "Closing Stream because it received an exception before it started: "
                    << std::endl
                    << t.exception();
                stream->serverClose();
              }
            });
        std::ignore = clientCallback->onFirstResponse(
            std::move(firstResponsePayload), evb, stream);
        stream->processClientMessages();
      });
    };
  }
#endif // FOLLY_HAS_COROUTINES

  //
  // TwoWayBridge methods
  //
  void consume();
  void canceled();
  //
  // end of TwoWayBridge methods
  //

  using TwoWayBridge::serverClose;
  using TwoWayBridge::serverGetMessages;
  using TwoWayBridge::serverPush;
  using TwoWayBridge::serverWait;

 private:
#if FOLLY_HAS_COROUTINES
  template <bool WithHeader, typename T>
  static folly::coro::Task<> fromAsyncGeneratorImpl(
      std::unique_ptr<ServerGeneratorStreamBridge, Deleter> stream,
      StreamElementEncoder<T>* encode,
      folly::coro::AsyncGenerator<
          std::conditional_t<WithHeader, MessageVariant<T>, T>&&> gen,
      TileStreamGuard interaction,
      std::shared_ptr<ContextStack> contextStack) {
    class ReadyCallback final : public QueueConsumer {
     public:
      void consume() override { baton_.post(); }
      void canceled() override { LOG(WARNING) << "Server stream is cancelled"; }
      folly::coro::Task<void> wait() { co_await baton_; }

     private:
      folly::coro::Baton baton_;
    };

    bool pauseStream = false;
    int64_t credits = 0;
    SCOPE_EXIT {
      stream->serverClose();
    };

    notifyStreamSubscribe(contextStack.get(), interaction);

    // ensure the generator is destroyed before the interaction TimeStreamGuard
    auto gen_ = std::move(gen);

    while (true) {
      if (credits == 0 || pauseStream) {
        ReadyCallback ready;
        if (stream->serverWait(&ready)) {
          co_await ready.wait();
        }
      }

      {
        auto queue = stream->serverGetMessages();
        while (!queue.empty()) {
          auto next = queue.front();
          queue.pop();
          switch (next) {
            case detail::StreamControl::CANCEL:
              co_return;
            case detail::StreamControl::PAUSE:
              notifyStreamPause(
                  contextStack.get(),
                  details::STREAM_PAUSE_REASON::EXPLICIT_PAUSE);
              pauseStream = true;
              break;
            case detail::StreamControl::RESUME:
              notifyStreamResumeReceive(contextStack.get());
              pauseStream = false;
              break;
            default:
              notifyStreamCredit(contextStack.get(), next);
              credits += next;
              break;
          }
        }
      }

      if (UNLIKELY(pauseStream || credits == 0)) {
        continue;
      }

      auto next = co_await folly::coro::co_awaitTry(
          folly::coro::co_withCancellation(
              stream->cancelSource_.getToken(), gen_.next()));
      if (next.hasException()) {
        stream->serverPush((*encode)(std::move(next.exception())));
        co_return;
      }
      if (!next->has_value()) {
        stream->serverPush({});
        co_return;
      }

      auto&& item = **next;
      if constexpr (WithHeader) {
        folly::Try<StreamPayload> sp =
            encodeMessageVariant(encode, std::move(item));
        bool hasPayload = sp->payload || sp->isOrderedHeader;
        stream->serverPush(std::move(sp));
        if (hasPayload) {
          --credits;
        }
      } else {
        stream->serverPush((*encode)(std::move(item)));
        --credits;
      }

      notifyStreamNext(contextStack.get());
      if (credits == 0) {
        notifyStreamPause(
            contextStack.get(), details::STREAM_PAUSE_REASON::NO_CREDITS);
      }
    }
  }
#endif // FOLLY_HAS_COROUTINES

  ServerGeneratorStreamBridge(
      StreamClientCallback* clientCallback,
      folly::EventBase* evb,
      std::shared_ptr<ContextStack> contextStack = nullptr);

  //
  // StreamServerCallback methods
  //
  bool onStreamRequestN(int32_t credits) override;

  void onStreamCancel() override;

  void resetClientCallback(StreamClientCallback& clientCallback) override;

  void pauseStream() override;

  void resumeStream() override;
  //
  // end of StreamServerCallback methods
  //

  void processClientMessages();

  StreamClientCallback* clientCallback_;
  folly::EventBase* evb_;
  std::shared_ptr<ContextStack> contextStack_;

#if FOLLY_HAS_COROUTINES
  folly::CancellationSource cancelSource_;
#endif

  //
  // Helper methods to encapsulate ContextStack usage
  //
  static void notifyStreamSubscribe(
      ContextStack* contextStack, const TileStreamGuard& interaction);
  static void notifyStreamPause(
      ContextStack* contextStack, details::STREAM_PAUSE_REASON reason);
  static void notifyStreamResumeReceive(ContextStack* contextStack);
  static void notifyStreamCredit(ContextStack* contextStack, int64_t credits);
  static void notifyStreamNext(ContextStack* contextStack);
  //
  // end of Helper methods to encapsulate ContextStack usage
  //

  friend class test::TestProducerCallback;
};

} // namespace apache::thrift::detail
