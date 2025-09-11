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

#include <thrift/lib/cpp/StreamEventHandler.h>

namespace apache::thrift::detail {

#if FOLLY_HAS_COROUTINES
template <bool WithHeader, typename T>
folly::coro::Task<> ServerGeneratorStreamBridge::fromAsyncGeneratorImpl(
    std::unique_ptr<ServerGeneratorStreamBridge, Deleter> stream,
    StreamElementEncoder<T>* encode,
    folly::coro::AsyncGenerator<
        std::conditional_t<WithHeader, MessageVariant<T>, T>&&> gen,
    TileStreamGuard interaction) {
  class ReadyCallback final : public QueueConsumer {
   public:
    void consume() override { baton.post(); }
    void canceled() override { LOG(WARNING) << "Server stream is cancelled"; }

    folly::coro::Task<void> wait() { co_await baton; }

    folly::coro::Baton baton;
  };

  bool pauseStream = false;
  int64_t credits = 0;
  SCOPE_EXIT {
    stream->serverClose();
  };

  stream->notifyStreamSubscribe(interaction);

  // ensure the generator is destroyed before the interaction TimeStreamGuard
  auto gen_ = std::move(gen);

  while (true) {
    if (credits == 0 || pauseStream) {
      ReadyCallback ready;
      if (stream->wait(&ready)) {
        co_await ready.baton;
      }
    }

    {
      auto queue = stream->getMessages();
      while (!queue.empty()) {
        auto next = queue.front();
        queue.pop();
        switch (next) {
          case detail::StreamControl::CANCEL:
            stream->notifyStreamFinally(stream->cancelSource_);
            co_return;
          case detail::StreamControl::PAUSE:
            stream->notifyStreamPause(
                details::STREAM_PAUSE_REASON::EXPLICIT_PAUSE);
            pauseStream = true;
            break;
          case detail::StreamControl::RESUME:
            stream->notifyStreamResumeReceive();
            pauseStream = false;
            break;
          default:
            stream->notifyStreamCredit(next);
            credits += next;
            break;
        }
      }
    }

    if (UNLIKELY(pauseStream || credits == 0)) {
      continue;
    }

    auto next =
        co_await folly::coro::co_awaitTry(folly::coro::co_withCancellation(
            stream->cancelSource_.getToken(), gen_.next()));
    if (next.hasException()) {
      stream->notifyStreamFinally(stream->cancelSource_, next.exception());
      stream->publish((*encode)(std::move(next.exception())));
      co_return;
    }
    if (!next->has_value()) {
      stream->publish({});
      stream->notifyStreamFinally(stream->cancelSource_);
      co_return;
    }

    auto&& item = **next;
    if constexpr (WithHeader) {
      folly::Try<StreamPayload> sp =
          encodeMessageVariant(encode, std::move(item));
      bool hasPayload = sp->payload || sp->isOrderedHeader;
      stream->publish(std::move(sp));
      if (hasPayload) {
        --credits;
      }
    } else {
      stream->publish((*encode)(std::move(item)));
      --credits;
    }

    stream->notifyStreamNext();
    if (credits == 0) {
      stream->notifyStreamPause(details::STREAM_PAUSE_REASON::NO_CREDITS);
    }
  }
}

template <bool WithHeader, typename T>
ServerStreamFn<T> ServerGeneratorStreamBridge::fromAsyncGenerator(
    folly::coro::AsyncGenerator<
        std::conditional_t<WithHeader, MessageVariant<T>, T>&&>&& gen) {
  return [gen = std::move(gen)](
             folly::Executor::KeepAlive<> serverExecutor,
             StreamElementEncoder<T>* encode) mutable {
    return ServerStreamFactory([gen = std::move(gen),
                                serverExecutor = std::move(serverExecutor),
                                encode](
                                   FirstResponsePayload&& payload,
                                   StreamClientCallback* callback,
                                   folly::EventBase* clientEb,
                                   TilePtr&& interaction,
                                   std::shared_ptr<ContextStack>
                                       contextStack) mutable {
      DCHECK(clientEb->isInEventBaseThread());

      // For Stream, ContextStack is co-owned by Several Steam components
      if (contextStack && callback) {
        callback->setContextStack(contextStack);
      }

      auto stream = new ServerGeneratorStreamBridge(
          callback, clientEb, std::move(contextStack));
      auto streamPtr = stream->copy();
      fromAsyncGeneratorImpl<WithHeader>(
          std::move(streamPtr),
          encode,
          std::move(gen),
          TileStreamGuard::transferFrom(std::move(interaction)))
          .scheduleOn(std::move(serverExecutor))
          .start([sp = stream->copy()](folly::Try<folly::Unit> t) {
            if (t.hasException()) {
              LOG(ERROR)
                  << "Closing Stream because it received an exception before it started: "
                  << std::endl
                  << t.exception();
              sp->close();
            }
          });
      std::ignore =
          callback->onFirstResponse(std::move(payload), clientEb, stream);
      stream->processPayloads();
    });
  };
}
#endif // FOLLY_HAS_COROUTINES

} // namespace apache::thrift::detail
