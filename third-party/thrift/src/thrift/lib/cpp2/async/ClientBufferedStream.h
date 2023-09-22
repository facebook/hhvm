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

#include <algorithm>
#include <array>
#include <variant>

#include <folly/GLog.h>
#include <folly/Portability.h>
#include <folly/experimental/coro/AsyncGenerator.h>
#include <folly/experimental/coro/Baton.h>
#include <folly/futures/Future.h>
#include <folly/io/async/fdsock/SocketFds.h>
#include <folly/synchronization/Baton.h>
#include <thrift/lib/cpp2/async/ClientStreamBridge.h>

namespace yarpl {
namespace flowable {
class ThriftStreamShim;
}
} // namespace yarpl
namespace apache {
namespace thrift {

template <typename T>
class ClientBufferedStream {
 public:
  ClientBufferedStream() {}
  ClientBufferedStream(
      apache::thrift::detail::ClientStreamBridge::ClientPtr streamBridge,
      folly::Try<T> (*decode)(folly::Try<StreamPayload>&&),
      const BufferOptions& bufferOptions)
      : streamBridge_(std::move(streamBridge)),
        decode_(decode),
        bufferOptions_(bufferOptions) {}

  // onNextTry may return bool or void; false cancels the subscription.
  template <typename OnNextTry>
  void subscribeInline(OnNextTry&& onNextTry) && {
    CHECK_EQ(bufferOptions_.memSize, 0)
        << "MemoryBufferSize only supported by AsyncGenerator subscription";
    auto streamBridge = std::move(streamBridge_);

    if (bufferOptions_.chunkSize == 0) {
      streamBridge->requestN(1);
      ++bufferOptions_.chunkSize;
    }

    int32_t outstanding = bufferOptions_.chunkSize;
    size_t payloadDataSize = 0;

    apache::thrift::detail::ClientStreamBridge::ClientQueue queue;
    class ReadyCallback : public apache::thrift::detail::ClientStreamConsumer {
     public:
      void consume() override { baton.post(); }

      void canceled() override { std::terminate(); }

      void wait() { baton.wait(); }

     private:
      folly::Baton<> baton;
    };

    while (true) {
      if (queue.empty()) {
        ReadyCallback callback;
        if (streamBridge->wait(&callback)) {
          callback.wait();
        }
        queue = streamBridge->getMessages();
      }

      {
        auto& payload = queue.front();
        if (payload.hasValue()) {
          if (!payload->payload) {
            FB_LOG_EVERY_MS(WARNING, 1000)
                << "Dropping unhandled stream header frame";
            queue.pop();
            continue;
          }
          payloadDataSize += payload->payload->computeChainDataLength();
        }
        auto value = decode_(std::move(payload));
        queue.pop();
        bool done = !value.hasValue();
        using Res = std::invoke_result_t<OnNextTry, folly::Try<T>>;
        if constexpr (std::is_same_v<Res, bool>) {
          done |= !onNextTry(std::move(value));
        } else {
          static_assert(
              std::is_void_v<Res>, "onNextTry must return bool or void");
          onNextTry(std::move(value));
        }
        if (done) {
          break;
        }
      }

      if ((--outstanding <= bufferOptions_.chunkSize / 2) ||
          (payloadDataSize >= kRequestCreditPayloadSize)) {
        streamBridge->requestN(bufferOptions_.chunkSize - outstanding);
        outstanding = bufferOptions_.chunkSize;
        payloadDataSize = 0;
      }
    }
  }

#if FOLLY_HAS_COROUTINES
  FOLLY_PUSH_WARNING
  FOLLY_GCC_DISABLE_WARNING("-Wattributes")
  FOLLY_MSVC_DISABLE_WARNING(5030)
  [[clang::annotate("not_coroutine")]] folly::coro::AsyncGenerator<T&&>
  toAsyncGenerator() && {
    FOLLY_POP_WARNING
    return bufferOptions_.memSize
        ? toAsyncGeneratorWithSizeTarget(
              std::move(streamBridge_),
              bufferOptions_.chunkSize,
              decode_,
              bufferOptions_.memSize,
              bufferOptions_.maxChunkSize)
        : toAsyncGeneratorImpl<false>(
              std::move(streamBridge_), bufferOptions_.chunkSize, decode_);
  }

  struct RichPayloadReceived { // sent as `RichPayloadToSend` from server
    T payload;
    StreamPayloadMetadata metadata;
    folly::SocketFds fds;
  };
  struct UnorderedHeader {
    StreamPayloadMetadata metadata;
  };
  struct OrderedHeader {
    StreamPayloadMetadata metadata;
  };
  using MessageVariant =
      std::variant<T, RichPayloadReceived, UnorderedHeader, OrderedHeader>;
  folly::coro::AsyncGenerator<MessageVariant&&>
  toAsyncGeneratorWithHeader() && {
    CHECK_EQ(bufferOptions_.memSize, 0)
        << "MemoryBufferSize not supported by toAsyncGeneratorWithHeader()";
    return toAsyncGeneratorImpl<true>(
        std::move(streamBridge_), bufferOptions_.chunkSize, decode_);
  }
#endif // FOLLY_HAS_COROUTINES

  template <typename Callback>
  auto subscribeExTry(folly::Executor::KeepAlive<> e, Callback&& onNextTry) && {
    CHECK_EQ(bufferOptions_.memSize, 0)
        << "MemoryBufferSize only supported by AsyncGenerator subscription";
    if (bufferOptions_.chunkSize == 0) {
      streamBridge_->requestN(1);
      ++bufferOptions_.chunkSize;
    }

    auto c = new Continuation<std::decay_t<Callback>>(
        e,
        std::forward<Callback>(onNextTry),
        std::move(streamBridge_),
        decode_,
        bufferOptions_.chunkSize);
    Subscription sub(c->state_);
    e->add([c]() { (*c)(); });
    return sub;
  }

 private:
#if FOLLY_HAS_COROUTINES
  template <bool WithHeader>
  static folly::coro::AsyncGenerator<
      std::conditional_t<WithHeader, MessageVariant, T>&&>
  toAsyncGeneratorImpl(
      apache::thrift::detail::ClientStreamBridge::ClientPtr streamBridge,
      int32_t chunkBufferSize,
      folly::Try<T> (*decode)(folly::Try<StreamPayload>&&)) {
    if (chunkBufferSize == 0) {
      streamBridge->requestN(1);
      ++chunkBufferSize;
    }

    int32_t outstanding = chunkBufferSize;
    size_t payloadDataSize = 0;

    auto updateCredits = [&] {
      if ((--outstanding <= chunkBufferSize / 2) ||
          (payloadDataSize >= kRequestCreditPayloadSize)) {
        streamBridge->requestN(chunkBufferSize - outstanding);
        outstanding = chunkBufferSize;
        payloadDataSize = 0;
      }
    };

    apache::thrift::detail::ClientStreamBridge::ClientQueue queue;
    class ReadyCallback : public apache::thrift::detail::ClientStreamConsumer {
     public:
      void consume() override { baton.post(); }

      void canceled() override { baton.post(); }

      folly::coro::Baton baton;
    };

    while (true) {
      co_await folly::coro::co_safe_point;
      if (queue.empty()) {
        ReadyCallback callback;
        if (streamBridge->wait(&callback)) {
          folly::CancellationCallback cb{
              co_await folly::coro::co_current_cancellation_token,
              [&] { streamBridge->cancel(); }};
          co_await callback.baton;
        }
        queue = streamBridge->getMessages();
        if (queue.empty()) {
          // we've been cancelled
          apache::thrift::detail::ClientStreamBridge::Ptr(
              streamBridge.release());
          co_yield folly::coro::co_cancelled;
        }
      }

      {
        auto& payload = queue.front();
        if (!payload.hasValue() && !payload.hasException()) {
          break;
        }
        const size_t payloadSize = payload.hasValue() && payload->payload
            ? payload->payload->computeChainDataLength()
            : 0;
        if (payload.hasValue()) {
          if (!payloadSize) {
            if constexpr (!WithHeader) {
              FB_LOG_EVERY_MS(WARNING, 1000)
                  << "Dropping unhandled stream header frame";
              if (payload->isOrderedHeader) {
                updateCredits();
              }
              queue.pop();
              continue;
            }
          } else {
            payloadDataSize += payloadSize;
          }
        }
        if constexpr (WithHeader) {
          if (payload.hasValue()) {
            if (payloadSize) {
              // The "else" is the normal, lightweight path -- no user
              // metadata, no FDs.  In that case the user code gets plain
              // `T` in the variant.  Otherwise -- if the payload had either
              // custom metadata, or FDs, we pass back `RichPayloadReceived`.
              if (payload->metadata.otherMetadata().has_value() ||
                  !payload->fds.empty()) {
                RichPayloadReceived ret;
                ret.metadata = std::move(payload->metadata);
                ret.fds = std::move(payload->fds.dcheckReceivedOrEmpty());
                ret.payload = *decode(std::move(payload));
                queue.pop();
                co_yield std::move(ret);
              } else {
                T ret = *decode(std::move(payload));
                queue.pop();
                co_yield std::move(ret);
              }
            } else if (payload->isOrderedHeader) {
              OrderedHeader ret{std::move(payload->metadata)};
              queue.pop();
              co_yield std::move(ret);
            } else {
              UnorderedHeader ret{std::move(payload->metadata)};
              queue.pop();
              co_yield std::move(ret);
              continue;
            }
          } else {
            co_yield folly::coro::co_error(
                decode(std::move(payload)).exception());
          }
        } else {
          auto value = decode(std::move(payload));
          queue.pop();
          co_yield folly::coro::co_result(std::move(value));
        }
        updateCredits();
      }
    }
  }

  static folly::coro::AsyncGenerator<T&&> toAsyncGeneratorWithSizeTarget(
      apache::thrift::detail::ClientStreamBridge::ClientPtr streamBridge,
      int32_t chunkBufferSize,
      folly::Try<T> (*decode)(folly::Try<StreamPayload>&&),
      size_t memBufferTarget,
      int32_t maxChunkBufferSize) {
    if (chunkBufferSize == 0) {
      streamBridge->requestN(1);
      ++chunkBufferSize;
    }

    int32_t outstanding = chunkBufferSize;
    size_t bufferMemSize = 0;

    // Use a circular buffer with the latest payload sizes to estimate the
    // recent average payload size.
    constexpr size_t kEstimationWindowSize = 128;
    std::array<size_t, kEstimationWindowSize> payloadSizesWindow;
    std::fill(payloadSizesWindow.begin(), payloadSizesWindow.end(), 0);

    size_t windowSum = 0;
    size_t numReceivedPayloads = 0;
    size_t maxPayloadSize = 0;

    auto updateCredits = [&] {
      DCHECK_LT(0, outstanding);
      --outstanding;
      // If enough payloads have been received estimate the recent average
      // payload size, otherwise conservatively use the largest received
      // size.
      size_t estPayloadSize = std::max<size_t>(
          numReceivedPayloads < kEstimationWindowSize
              ? maxPayloadSize
              : (windowSum + kEstimationWindowSize - 1) / kEstimationWindowSize,
          1);
      size_t outstandingSize = bufferMemSize + outstanding * estPayloadSize;
      size_t spaceAvailable =
          std::max<ssize_t>(memBufferTarget - outstandingSize, 0);

      // Issue more credits when available space is at least 16kB (to
      // amortize the request over 16kB worth of received payloads) or half
      // of the buffer (to ensure there are enough outstanding credits in
      // the small buffer / small payloads regime).
      if (spaceAvailable >= kRequestCreditPayloadSize ||
          spaceAvailable >= memBufferTarget / 2) {
        // Convert to credits using the size estimate, but cap credits and
        // outstanding requests to maxChunkBufferSize if this was requested
        // in BufferOptions.
        DCHECK_LE(outstanding, maxChunkBufferSize);
        const int32_t remainingCredits = maxChunkBufferSize - outstanding;
        int32_t request =
            (spaceAvailable + estPayloadSize - 1) / estPayloadSize;
        request = std::min(remainingCredits, request);
        streamBridge->requestN(request);
        outstanding += request;
      }
    };

    apache::thrift::detail::ClientStreamBridge::ClientQueueWithTailPtr queue;
    class ReadyCallback : public apache::thrift::detail::ClientStreamConsumer {
     public:
      void consume() override { baton.post(); }

      void canceled() override { baton.post(); }

      folly::coro::Baton baton;
    };

    while (true) {
      co_await folly::coro::co_safe_point;

      {
        // Always check for new buffered messages to update queue size
        apache::thrift::detail::ClientStreamBridge::ClientQueue incoming;
        if (queue.empty()) {
          ReadyCallback callback;
          if (streamBridge->wait(&callback)) {
            folly::CancellationCallback cb{
                co_await folly::coro::co_current_cancellation_token,
                [&] { streamBridge->cancel(); }};
            co_await callback.baton;
          }
          incoming = streamBridge->getMessages();
          if (incoming.empty()) {
            // we've been cancelled
            apache::thrift::detail::ClientStreamBridge::Ptr(
                streamBridge.release());
            co_yield folly::coro::co_cancelled;
          }
        } else {
          incoming = streamBridge->getMessages();
        }

        // Sum sizes of new buffered messages and append to queue
        queue.append(
            apache::thrift::detail::ClientStreamBridge::ClientQueueWithTailPtr(
                std::move(incoming), [&](auto& payload) {
                  if (payload.hasValue() && payload->payload) {
                    bufferMemSize += payload->payload->computeChainDataLength();
                  }
                }));
      }

      {
        auto& payload = queue.front();
        if (!payload.hasValue() && !payload.hasException()) {
          break;
        }
        const size_t payloadSize = payload.hasValue() && payload->payload
            ? payload->payload->computeChainDataLength()
            : 0;
        if (payload.hasValue()) {
          if (!payloadSize) {
            FB_LOG_EVERY_MS(WARNING, 1000)
                << "Dropping unhandled stream header frame";
            if (payload->isOrderedHeader) {
              updateCredits();
            }
            queue.pop();
            continue;
          }
          windowSum -= std::exchange(
              payloadSizesWindow[numReceivedPayloads % kEstimationWindowSize],
              payloadSize);
          windowSum += payloadSize;
          numReceivedPayloads += 1;
          maxPayloadSize = std::max(maxPayloadSize, payloadSize);
          bufferMemSize -= payloadSize;
        }
        auto value = decode(std::move(payload));
        queue.pop();
        co_yield folly::coro::co_result(std::move(value));
        updateCredits();
      }
    }
  }
#endif // FOLLY_HAS_COROUTINES

  struct SharedState {
    explicit SharedState(
        apache::thrift::detail::ClientStreamBridge::ClientPtr sb)
        : streamBridge(sb.release()) {}
    folly::Promise<folly::Unit> promise;
    apache::thrift::detail::ClientStreamBridge::Ptr streamBridge;
  };

 public:
  class Subscription {
    explicit Subscription(std::shared_ptr<SharedState> state)
        : state_(std::move(state)) {}

   public:
    Subscription(Subscription&& s) = default;
    Subscription& operator=(Subscription&& s) {
      if (std::exchange(state_, std::move(s.state_))) {
        LOG(FATAL) << "Subscription has to be joined/detached";
      }
      return *this;
    }
    ~Subscription() {
      if (state_) {
        LOG(FATAL) << "Subscription has to be joined/detached";
      }
    }

    void cancel() { state_->streamBridge->cancel(); }

    void detach() && { state_.reset(); }

    void join() && { std::move(*this).futureJoin().wait(); }

    folly::SemiFuture<folly::Unit> futureJoin() && {
      return std::exchange(state_, nullptr)->promise.getSemiFuture();
    }

   private:
    std::shared_ptr<SharedState> state_;
    friend class ClientBufferedStream;
  };

 private:
  template <typename OnNextTry>
  // Ownership model: caller owns until wait returns true.
  // Argument is released ("leaked") when wait() succeeds. It is transferred
  // to the new execution frame in consume(). If wait() returns false its
  // argument is not released and the caller frees it as normal. If wait() is
  // interrupted by cancel() the memory is freed in canceled()
  class Continuation : public apache::thrift::detail::ClientStreamConsumer {
   public:
    Continuation(
        folly::Executor::KeepAlive<> e,
        OnNextTry onNextTry,
        apache::thrift::detail::ClientStreamBridge::ClientPtr streamBridge,
        folly::Try<T> (*decode)(folly::Try<StreamPayload>&&),
        int32_t chunkBufferSize)
        : e_(e),
          onNextTry_(std::move(onNextTry)),
          decode_(decode),
          chunkBufferSize_(chunkBufferSize),
          state_(std::make_shared<SharedState>(std::move(streamBridge))) {
      outstanding_ = chunkBufferSize_;
    }

    ~Continuation() { state_->promise.setValue(); }

    // takes ownership of pointer on success
    static bool wait(std::unique_ptr<Continuation>& cb) {
      bool ret = cb->state_->streamBridge->wait(cb.get());
      if (ret) {
        cb.release();
      }
      return ret;
    }

    void consume() override {
      e_->add([this]() { (*this)(); });
    }

    void canceled() override { delete this; }

    void operator()() noexcept {
      std::unique_ptr<Continuation> cb(this);
      apache::thrift::detail::ClientStreamBridge::ClientQueue queue;

      while (!state_->streamBridge->isCanceled()) {
        if (queue.empty()) {
          if (Continuation::wait(cb)) {
            // The filler will schedule us back on the executor once the queue
            // is refilled so we return here
            return;
          }
          // Otherwise messages are now available (or we've been cancelled)
          queue = state_->streamBridge->getMessages();
          if (queue.empty()) {
            // we've been cancelled
            return;
          }
        }

        {
          auto& payload = queue.front();
          if (!payload.hasValue() && !payload.hasException()) {
            onNextTry_(folly::Try<T>());
            return;
          }
          if (payload.hasValue()) {
            if (!payload->payload) {
              FB_LOG_EVERY_MS(WARNING, 1000)
                  << "Dropping unhandled stream header frame";
              queue.pop();
              continue;
            }
            payloadDataSize_ += payload->payload->computeChainDataLength();
          }
          auto value = decode_(std::move(payload));
          queue.pop();
          const auto hasException = value.hasException();
          onNextTry_(std::move(value));
          if (hasException) {
            return;
          }
        }

        if ((--outstanding_ <= chunkBufferSize_ / 2) ||
            (payloadDataSize_ >= kRequestCreditPayloadSize)) {
          state_->streamBridge->requestN(chunkBufferSize_ - outstanding_);
          outstanding_ = chunkBufferSize_;
          payloadDataSize_ = 0;
        }
      }
    }

   private:
    folly::Executor::KeepAlive<> e_;
    OnNextTry onNextTry_;
    folly::Try<T> (*decode_)(folly::Try<StreamPayload>&&);
    int32_t chunkBufferSize_;
    int32_t outstanding_;
    size_t payloadDataSize_{0};
    std::shared_ptr<SharedState> state_;
    friend class ClientBufferedStream;
  };

  apache::thrift::detail::ClientStreamBridge::ClientPtr streamBridge_;
  folly::Try<T> (*decode_)(folly::Try<StreamPayload>&&) = nullptr;
  BufferOptions bufferOptions_;
  static constexpr size_t kRequestCreditPayloadSize = 16384;

  friend class yarpl::flowable::ThriftStreamShim;
};

template <typename Response, typename StreamElement>
struct ResponseAndClientBufferedStream {
  using ResponseType = Response;
  using StreamElementType = StreamElement;

  Response response;
  ClientBufferedStream<StreamElement> stream;
};
} // namespace thrift
} // namespace apache
