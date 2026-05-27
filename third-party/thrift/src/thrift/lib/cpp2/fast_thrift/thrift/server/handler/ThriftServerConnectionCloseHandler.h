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

#include <chrono>
#include <cstdint>
#include <memory>
#include <utility>

#include <folly/ExceptionWrapper.h>
#include <folly/Portability.h>
#include <folly/io/async/AsyncTimeout.h>
#include <folly/io/async/EventBase.h>
#include <folly/logging/xlog.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Event.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponsePayloads.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * ThriftServerConnectionCloseHandler — duplex pipeline handler that owns the
 * in-flight request counter for a thrift server connection and
 * sequences the connection's terminal phase entirely internally.
 *
 * Sits immediately upstream of the tail adapter in the thrift pipeline.
 *
 *   ThriftServerTransportAdapter (head)
 *     → [thrift handlers] → ThriftServerConnectionCloseHandler → tail adapter
 *
 * Owns two deadlines. Owners (ConnectionManager etc.) need only
 * trigger drain and wait for the closeCallback; they do not need to
 * know about either deadline.
 *
 *   drainTimeout — how long to wait for in-flight responses to flush
 *     to the wire after a CloseConnection event. Expiry forces pipeline
 *     deactivation (the socket goes away regardless of stragglers).
 *
 *   reapTimeout — how long to wait for in-flight handler callbacks to
 *     return after the pipeline has been deactivated. Expiry is a
 *     LOG(FATAL) — a handler that hasn't completed within this window
 *     after the socket is gone is stuck and would otherwise leak the
 *     adapter memory silently.
 *
 * State machine:
 *
 *                       CloseConnection event
 *               Open ────────────────► Draining
 *                │                       │  ├── drain timer fires ──┐
 *                │                       │  │                       │
 *                │                       │  └── in-flight == 0      │
 *                │                       ▼      (deactivate)        │
 *                │                     Closed                       │
 *                │                                                  ▼
 *                │   pipeline deactivated externally     ClosedReaping
 *                │   (peer FIN, transport error, drain        │  │
 *                │   timer expiry above)                      │  │
 *                ├──────────────────────────────────────────► │  │
 *                │           in-flight > 0                    │  │
 *                │                                            │  │
 *                │   pipeline deactivated, in-flight == 0     │  │
 *                └──────────────────────────────────────────► │  │
 *                                                             │  │
 *                                              in-flight == 0 │  │ reap
 *                                                             ▼  ▼ timer
 *                                                          Closed   fires
 *                                                                    │
 *                                                                    ▼
 *                                                              LOG(FATAL)
 *
 * Closed transitions broadcast a ConnectionClosed pipeline event; the
 * tail adapter handles it by firing the user closeCallback. From the
 * owner's perspective the only observable edges are: close triggered
 * (via the broadcast CloseConnection event from the adapter) and
 * closeCallback fires (signaling "safe to drop the connection").
 */
template <typename Context>
class ThriftServerConnectionCloseHandler {
 public:
  // Default deadlines. Both are connection-scoped, intended to be
  // configurable per-service by the pipeline builder; the values here
  // are reasonable fallbacks for unit tests and unconfigured paths.
  static constexpr std::chrono::milliseconds kDefaultDrainTimeout{30'000};
  static constexpr std::chrono::milliseconds kDefaultReapTimeout{60'000};

  ThriftServerConnectionCloseHandler() = default;

  ThriftServerConnectionCloseHandler(
      std::chrono::milliseconds drainTimeout,
      std::chrono::milliseconds reapTimeout) noexcept
      : drainTimeout_(drainTimeout), reapTimeout_(reapTimeout) {}

  // HandlerLifecycle
  void handlerAdded(Context& ctx) noexcept {
    auto* evb = ctx.eventBase();
    drainTimer_ = folly::AsyncTimeout::make(*evb, [this, &ctx]() noexcept {
      if (state_ != State::Draining) {
        return;
      }
      // Force-deactivate. The synchronous onPipelineInactive cascade
      // re-enters us, observes Draining→inFlight>0, and arms the reap
      // timer. We don't reschedule the drain timer; leaving it
      // un-armed is correct.
      ctx.deactivate();
    });
    reapTimer_ = folly::AsyncTimeout::make(*evb, [this]() noexcept {
      if (state_ != State::ClosedReaping) {
        return;
      }
      XLOGF(
          FATAL,
          "ThriftServerConnectionCloseHandler reap timed out after {}ms with {} in-flight "
          "callback(s) outstanding; a thrift handler completion is stuck. "
          "Stack should point at the offending site.",
          reapTimeout_.count(),
          inFlight_);
    });
  }

  void handlerRemoved(Context& /*ctx*/) noexcept {
    drainTimer_.reset();
    reapTimer_.reset();
  }

  void onPipelineActive(Context& /*ctx*/) noexcept {}

  // Pipeline deactivation arrives via two paths: (a) external — the
  // transport tore the socket down (peer FIN, transport error) — or
  // (b) internal — our own drain timer fired and called deactivate().
  // Both look identical from here.
  void onPipelineInactive(Context& ctx) noexcept {
    if (state_ == State::Closed) {
      return;
    }
    // If we were Draining, cancel the drain timer — we're past it now.
    // cancelTimeout is safe even if not scheduled.
    drainTimer_->cancelTimeout();
    if (inFlight_ == 0) {
      transitionToClosed(ctx);
      return;
    }
    state_ = State::ClosedReaping;
    reapTimer_->scheduleTimeout(reapTimeout_);
  }

  void onReadReady(Context& /*ctx*/) noexcept {}
  void onWriteReady(Context& /*ctx*/) noexcept {}

  // Inbound: reject if not Open, else count and forward. Both Draining
  // and ClosedReaping reject new work — the former because we've sent
  // CONNECTION_CLOSE, the latter because the socket is already gone.
  channel_pipeline::Result onRead(
      Context& ctx, channel_pipeline::TypeErasedBox&& msg) noexcept {
    if (FOLLY_UNLIKELY(state_ != State::Open)) {
      return channel_pipeline::Result::Error;
    }
    ++inFlight_;
    return ctx.fireRead(std::move(msg));
  }

  // Outbound: every response is paired with a prior inbound request
  // (CONNECTION_CLOSE we emit during drain enters via ctx.fireWrite,
  // which skips our own onWrite). Decrement first, then decide whether
  // to forward. ClosedReaping bails out: the socket is gone, no wire
  // write is possible — but the decrement must still happen so
  // maybeFinalize can drive the transition to Closed. Closed must
  // never receive a write (inFlight_ is 0 and the owner is dropping
  // the connection)
  channel_pipeline::Result onWrite(
      Context& ctx, channel_pipeline::TypeErasedBox&& msg) noexcept {
    DCHECK(state_ != State::Closed) << "no writes expected after Closed";
    DCHECK_GT(inFlight_, 0u);
    --inFlight_;
    if (FOLLY_UNLIKELY(state_ == State::ClosedReaping)) {
      maybeFinalize(ctx);
      return channel_pipeline::Result::Error;
    }
    auto result = ctx.fireWrite(std::move(msg));
    maybeFinalize(ctx);
    return result;
  }

  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }

  // Broadcast pipeline event. We act only on CloseConnection — every
  // other event type (including our own emitted ConnectionClosed,
  // delivered back via self-broadcast) is ignored.
  void onEvent(
      Context& ctx, const channel_pipeline::TypeErasedBox& evt) noexcept {
    const auto& event = evt.template get<ThriftServerEvent>();
    if (event.type != ThriftServerEventType::CloseConnection) {
      return;
    }
    handleCloseConnectionEvent(ctx);
  }

  // === Test accessors ===
  bool isDraining() const noexcept { return state_ == State::Draining; }
  bool isReaping() const noexcept { return state_ == State::ClosedReaping; }
  bool isClosed() const noexcept { return state_ == State::Closed; }
  uint32_t inFlight() const noexcept { return inFlight_; }
  std::chrono::milliseconds drainTimeout() const noexcept {
    return drainTimeout_;
  }
  std::chrono::milliseconds reapTimeout() const noexcept {
    return reapTimeout_;
  }

 private:
  enum class State : uint8_t {
    Open,
    Draining,
    ClosedReaping,
    Closed,
  };

  void handleCloseConnectionEvent(Context& ctx) noexcept {
    if (state_ != State::Open) {
      return;
    }
    state_ = State::Draining;
    drainTimer_->scheduleTimeout(drainTimeout_);
    (void)ctx.fireWrite(
        channel_pipeline::erase_and_box(makeConnectionCloseMessage()));
    maybeFinalize(ctx);
  }

  void maybeFinalize(Context& ctx) noexcept {
    if (inFlight_ != 0) {
      return;
    }
    if (state_ == State::Draining) {
      transitionToClosed(ctx);
      // Draining still owns the socket — deactivate now so the
      // transport tears the connection down. ClosedReaping was
      // triggered BY deactivation, so it skips this step.
      ctx.deactivate();
    } else if (state_ == State::ClosedReaping) {
      transitionToClosed(ctx);
    }
  }

  void transitionToClosed(Context& ctx) noexcept {
    state_ = State::Closed;
    drainTimer_->cancelTimeout();
    reapTimer_->cancelTimeout();
    ctx.fireEvent(
        channel_pipeline::erase_and_box(
            ThriftServerEvent{ThriftServerEventType::ConnectionClosed}));
  }

  uint32_t inFlight_{0};
  State state_{State::Open};
  std::chrono::milliseconds drainTimeout_{kDefaultDrainTimeout};
  std::chrono::milliseconds reapTimeout_{kDefaultReapTimeout};
  std::unique_ptr<folly::AsyncTimeout> drainTimer_;
  std::unique_ptr<folly::AsyncTimeout> reapTimer_;
};

} // namespace apache::thrift::fast_thrift::thrift
