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

namespace apache::thrift::detail {

#if FOLLY_HAS_COROUTINES
template <bool WithHeader, typename T>
folly::coro::Task<> ServerGeneratorStreamBridge::fromAsyncGeneratorImpl(
    std::unique_ptr<ServerGeneratorStreamBridge, Deleter> stream,
    StreamElementEncoder<T>* encode,
    folly::coro::AsyncGenerator<
        std::conditional_t<WithHeader, MessageVariant<T>, T>&&> gen,
    TileStreamGuard,
    ContextStack::UniquePtr contextStack) {
  using ReadyCallback = ServerStreamConsumerBaton<folly::coro::Baton>;
  bool pauseStream = false;
  int64_t credits = 0;
  SCOPE_EXIT {
    stream->serverClose();
  };

  if (contextStack) {
    contextStack->onStreamSubscribe();
  }

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
            if (contextStack) {
              contextStack->onStreamFinally(
                  details::STREAM_ENDING_TYPES::CANCEL);
            }
            co_return;
          case detail::StreamControl::PAUSE:
            if (contextStack) {
              contextStack->onStreamPauseReceive();
            }
            pauseStream = true;
            break;
          case detail::StreamControl::RESUME:
            if (contextStack) {
              contextStack->onStreamResumeReceive();
            }
            pauseStream = false;
            break;
          default:
            if (contextStack) {
              contextStack->onStreamCredit(next);
            }
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
      if (contextStack) {
        if (stream->cancelSource_.isCancellationRequested()) {
          contextStack->onStreamFinally(details::STREAM_ENDING_TYPES::CANCEL);
        } else {
          contextStack->handleStreamErrorWrapped(next.exception());
          contextStack->onStreamFinally(details::STREAM_ENDING_TYPES::ERROR);
        }
      }
      stream->publish((*encode)(std::move(next.exception())));
      co_return;
    }
    if (!next->has_value()) {
      stream->publish({});
      if (contextStack) {
        contextStack->onStreamFinally(details::STREAM_ENDING_TYPES::COMPLETE);
      }
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

    if (contextStack) {
      contextStack->onStreamNext();
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
                                   ContextStack::UniquePtr
                                       contextStack) mutable {
      DCHECK(clientEb->isInEventBaseThread());
      auto stream = new ServerGeneratorStreamBridge(callback, clientEb);
      auto streamPtr = stream->copy();
      fromAsyncGeneratorImpl<WithHeader>(
          std::move(streamPtr),
          encode,
          std::move(gen),
          TileStreamGuard::transferFrom(std::move(interaction)),
          std::move(contextStack))
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
