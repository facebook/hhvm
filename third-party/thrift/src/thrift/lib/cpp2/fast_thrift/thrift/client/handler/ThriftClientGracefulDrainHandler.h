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

#include <array>
#include <cstdint>

#include <folly/ExceptionWrapper.h>
#include <folly/lang/Hint.h>
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Handler.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/common/Event.h>

namespace apache::thrift::fast_thrift::thrift::client::handler {

/**
 * ThriftClientGracefulDrainHandler - owns the connection's graceful-drain
 * state, transport-agnostic, on the thrift side.
 *
 * The transport adapter translates the transport's graceful-close
 * notification (rocket CONNECTION_CLOSE) into a payload-less
 * CloseConnection event. This handler reacts to that event
 * and begins draining. While draining:
 *   - new outbound requests are rejected with a retryable NOT_OPEN so the
 *     caller redispatches to another connection
 *   - in-flight requests are untouched — their responses keep flowing to
 *     completion, and once the last one drains the handler deactivates the
 *     pipeline so the transport tears the connection down
 *
 * Drain is a lifecycle event, not a fault: in-flight work is never failed.
 *
 * State machine:
 *
 *                  CloseConnection event
 *           Open ───────────────────────► Closing
 *            │                              │  in-flight == 0
 *            │                              ▼  (deactivate)
 *            │                            Closed
 *            │   pipeline deactivated externally
 *            │   (peer FIN, transport error)
 *            └────────────────────────────► Closed
 *
 * The handler tracks in-flight requests itself: it counts outbound
 * requests it forwards and decrements as their responses return. This
 * mirrors ThriftServerConnectionCloseHandler (which counts the opposite
 * direction), minus the reaping phase — the client has no handler
 * callbacks to wait on, and the server owns the drain deadline, so no
 * client-side drain timer is needed: if in-flight never completes, the
 * server closes the socket and the pipeline deactivates externally.
 *
 * The 1:1 request↔response counting holds for request-response, the only
 * pattern wired today; streaming will need to revisit it.
 *
 * The handler is transport-agnostic: it reacts to a thrift
 * CloseConnection event, not to any rocket type, so a different transport
 * adapter that emits the same event works unchanged. It sits app-side,
 * just below the channel tail, so it rejects new writes before they
 * descend and sees every response before the tail.
 *
 * State is plain (non-atomic): the pipeline runs single-threaded on its
 * EventBase, so no locks are needed.
 */
class ThriftClientGracefulDrainHandler {
 public:
  ThriftClientGracefulDrainHandler() = default;

  // === HandlerLifecycle ===

  template <typename Context>
  void handlerAdded(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {}

  // === InboundHandler ===

  template <typename Context>
  void onPipelineActive(Context& /*ctx*/) noexcept {
    // A reactivated connection (disconnect → reconnect) starts fresh.
    state_ = State::Open;
    inFlight_ = 0;
  }

  template <typename Context>
  void onReadReady(Context& /*ctx*/) noexcept {}

  /**
   * Every inbound message resolves one in-flight request (a wire response
   * or a per-request error). Decrement, forward, and — if we are draining
   * and the last in-flight request just completed — deactivate.
   */
  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onRead(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    if (FOLLY_LIKELY(inFlight_ > 0)) {
      --inFlight_;
    }
    auto result = ctx.fireRead(std::move(msg));
    if (FOLLY_UNLIKELY(state_ == State::Closing) && inFlight_ == 0) {
      finalize(ctx);
    }
    return result;
  }

  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }

  // The single pipeline event this handler subscribes to: the
  // CloseConnection emitted by the transport adapter on graceful close.
  static constexpr std::array<ThriftClientEventType, 1> kSubscribedEvents{
      ThriftClientEventType::CloseConnection};

  // Begin draining on CloseConnection. The subscription delivers only the
  // event types in kSubscribedEvents, so CloseConnection is the only one
  // that reaches us; the event type arrives as the discriminator argument.
  template <typename Context>
  void onEvent(
      Context& ctx,
      ThriftClientEventType ev,
      const apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&
      /*evt*/) noexcept {
    DCHECK(ev == ThriftClientEventType::CloseConnection);
    if (state_ != State::Open) {
      return;
    }
    state_ = State::Closing;
    // Nothing in flight — close immediately rather than waiting for a
    // response that will never come.
    if (inFlight_ == 0) {
      finalize(ctx);
    }
  }

  // === OutboundHandler ===

  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onWrite(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    if (FOLLY_LIKELY(state_ == State::Open)) {
      ++inFlight_;
      return ctx.fireWrite(std::move(msg));
    }
    return rejectWhileDraining(ctx, std::move(msg));
  }

  template <typename Context>
  void onWriteReady(Context& /*ctx*/) noexcept {}

  // The single place we settle into Closed: the pipeline is now truly
  // inactive. Reached either externally (peer FIN / transport error) or as
  // the synchronous result of our own finalize() → deactivate(). So Closed
  // always means "pipeline inactive", never just "drain decided".
  template <typename Context>
  void onPipelineInactive(Context& /*ctx*/) noexcept {
    state_ = State::Closed;
  }

  // === Accessors for testing ===

  bool isDraining() const noexcept { return state_ == State::Closing; }
  bool isClosed() const noexcept { return state_ == State::Closed; }
  uint32_t inFlight() const noexcept { return inFlight_; }

 private:
  enum class State : uint8_t {
    Open,
    Closing,
    Closed,
  };

  // Drain complete: initiate teardown. deactivate() synchronously cascades
  // onPipelineInactive back through us, which is where we actually settle
  // into Closed — we don't set it here.
  template <typename Context>
  void finalize(Context& ctx) noexcept {
    ctx.deactivate();
  }

  /**
   * Reject a new outbound request while draining. Resolves the request's
   * callback inbound (via fireRead) with a retryable NOT_OPEN so the caller
   * redispatches — the request never reaches the transport. The connection
   * itself is unaffected; in-flight requests continue to drain.
   *
   * Returns Success: the request was handled (its callback resolved), so there
   * is no connection-level error to propagate.
   */
  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result rejectWhileDraining(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto& request = msg.get<ThriftRequestMessage>();
    ThriftResponseMessage resp;
    resp.payload = ThriftClientResponseError{
        .ew = folly::make_exception_wrapper<
            apache::thrift::transport::TTransportException>(
            apache::thrift::transport::TTransportException::NOT_OPEN,
            "Connection draining: request rejected, retry on another connection")};
    resp.requestContext = std::move(request.requestContext);
    resp.streamType =
        apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE;
    (void)ctx.fireRead(
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(resp)));
    return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
  }

  State state_{State::Open};
  uint32_t inFlight_{0};
};

static_assert(
    apache::thrift::fast_thrift::channel_pipeline::DuplexHandler<
        ThriftClientGracefulDrainHandler,
        apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl>,
    "ThriftClientGracefulDrainHandler must satisfy DuplexHandler concept");

static_assert(
    apache::thrift::fast_thrift::channel_pipeline::EventSubscriber<
        ThriftClientGracefulDrainHandler,
        ThriftClientEventType,
        apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl>,
    "ThriftClientGracefulDrainHandler must satisfy EventSubscriber concept "
    "so the pipeline links its CloseConnection event hook");

} // namespace apache::thrift::fast_thrift::thrift::client::handler
