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

#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/transport/rocket/core/RingBuffer.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/IncomingFrameHandler.h>

namespace apache::thrift::rocket {

namespace detail::incoming_frame_batcher {
template <
    typename IncomingFrameHandler,
    typename ConnectionT,
    template <typename>
    class ConnectionAdapter>
struct alignas(8) IncomingFrameEvent {
  using Connection = ConnectionAdapter<ConnectionT>;

  IncomingFrameEvent() = default;

  std::unique_ptr<folly::IOBuf> buf;
  Connection* state;
  IncomingFrameHandler* handler;
  std::optional<folly::DelayedDestruction::DestructorGuard> guard;

  IncomingFrameEvent(
      std::unique_ptr<folly::IOBuf>&& buf,
      Connection* state,
      IncomingFrameHandler* handler,
      folly::DelayedDestruction::DestructorGuard&& guard) noexcept
      : buf(std::move(buf)),
        state(state),
        handler(handler),
        guard(std::move(guard)) {}

  IncomingFrameEvent(IncomingFrameEvent&& other) = delete;
  IncomingFrameEvent& operator=(IncomingFrameEvent&& other) = delete;
  IncomingFrameEvent(const IncomingFrameEvent& other) = delete;
  IncomingFrameEvent& operator=(const IncomingFrameEvent& other) = delete;
  ~IncomingFrameEvent() noexcept {}

  FOLLY_ALWAYS_INLINE void handle() { handler->handle(std::move(buf)); }
};
} // namespace detail::incoming_frame_batcher

/**
 * Batches incoming frames per EventBase. This is to amortize the cost of
 * parsing incoming frames. This class is per EventBase- *not* per connection.
 *
 * The template parameter IncomingFrameHandler is the handler for to make
 * testing easier. In practice, IncomingFrameHandler is always the
 * apache::thrift::rocket::IncomingFrameHandler class.
 */
template <typename IncomingFrameHandler, typename Connection>
class IncomingFrameBatcher : public folly::EventBase::LoopCallback {
  using IncomingFrameEvent = detail::incoming_frame_batcher::IncomingFrameEvent<
      IncomingFrameHandler,
      typename Connection::AdaptedConnection,
      apache::thrift::rocket::ConnectionAdapter>;

 public:
  using IngressMemoryFailExceptionFactory =
      folly::Function<folly::exception_wrapper()>;

  IncomingFrameBatcher(
      folly::EventBase& evb, size_t batchLogSize = 4 /* 1<<4 == 16 */)
      : batchSize_(1 << batchLogSize), evb_(evb), queue_(batchLogSize) {}

  FOLLY_ALWAYS_INLINE void handle(
      std::unique_ptr<folly::IOBuf> buf,
      Connection& state,
      IncomingFrameHandler& handler) {
    evb_.dcheckIsInEventBaseThread();
    bool ret = queue_.emplace_back(
        std::move(buf),
        &state,
        &handler,
        folly::DelayedDestruction::DestructorGuard(state.getDestructorGuard()));
    FOLLY_SAFE_DCHECK(ret, "queue is full");
    tryForceDrainOrSchedule();
  }

  void runLoopCallback() noexcept { drain<false>(); }

  FOLLY_ALWAYS_INLINE size_t pending() const noexcept { return queue_.size(); }

 private:
  const size_t batchSize_;
  folly::EventBase& evb_;

  RingBuffer<IncomingFrameEvent> queue_;

  template <bool Forced>
  void drain() {
    queue_.consume(
        [this](IncomingFrameEvent& event) {
          dcheckIncomingFrameEvent(event);
          event.handle();
        },
        batchSize_);

    auto p = pending();
    if (Forced && p == 0 && isLoopCallbackScheduled()) {
      cancelLoopCallback();
    } else if (p > 0) {
      tryScheduleDrain();
    }
  }

  FOLLY_ALWAYS_INLINE void tryScheduleDrain() {
    if (!isLoopCallbackScheduled()) {
      evb_.runInLoop(this, true);
    }
  }

  FOLLY_ALWAYS_INLINE void tryForceDrainOrSchedule() {
    evb_.dcheckIsInEventBaseThread();
    if (pending() >= batchSize_) {
      drain<true>();
    } else {
      tryScheduleDrain();
    }
  }

  FOLLY_ERASE void dcheckIncomingFrameEvent(IncomingFrameEvent& event) {
    if constexpr (folly::kIsDebug) {
      FOLLY_SAFE_DCHECK(
          event.guard.has_value(),
          "missing connection state destruction guard");
      FOLLY_SAFE_DCHECK(event.state != nullptr, "connection state is null");
      FOLLY_SAFE_DCHECK(
          event.handler != nullptr, "incoming frame handler is null");
      FOLLY_SAFE_DCHECK(event.buf != nullptr, "iobuf pointer is null");
      FOLLY_SAFE_DCHECK(event.buf->computeChainCapacity(), "iobuf is empty");
    }
  }
};

} // namespace apache::thrift::rocket
