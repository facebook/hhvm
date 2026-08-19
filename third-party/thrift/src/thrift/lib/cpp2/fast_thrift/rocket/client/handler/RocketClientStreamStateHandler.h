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

#include <folly/ExceptionWrapper.h>
#include <folly/lang/Assume.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Handler.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/DirectStreamMap.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameViews.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Event.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/common/RocketClientStreamContext.h>

#include <cstdint>
#include <vector>

namespace apache::thrift::fast_thrift::rocket::client::handler {

/**
 * RocketClientStreamStateHandler - Pipeline handler for client-side RSocket
 * stream state management.
 *
 * This handler sits between the application layer and the FrameHandler,
 * managing stream IDs, storing request callbacks, and correlating responses.
 *
 * Per-stream state lives in the pipeline-level `RocketClientStreamContexts`
 * state, registered once per connection via
 * `PipelineBuilder::addState<RocketClientStreamContexts>()` and reached here
 * through `ctx.state<RocketClientStreamContexts>()`. This handler owns the
 * entry lifecycle (insert on stream-open, erase on terminal) so the other
 * client rocket handlers can share the same map without their own
 * bookkeeping. The pipeline owns the map's lifetime; this handler does not
 * clear it on removal.
 *
 * Pipeline position:
 *   App <-> StreamHandler <-> FrameHandler <-> Transport
 *
 * Key features:
 * - Generates odd stream IDs (1, 3, 5, ...) per RSocket protocol for clients
 * - Stores streamId -> StreamState mapping for active streams
 * - Terminal frames (ERROR, CANCEL, complete PAYLOAD) trigger stream state
 * cleanup
 *
 * Message flow:
 *   Outbound: RocketRequestMessage{streamType, payload, requestContext} ->
 *             RocketRequestMessage{streamId, streamType, payload}
 *             (streamId assigned; requestContext ownership is moved into the
 *             per-stream slot only after the downstream write succeeds —
 *             on failure the message is destroyed and the deleter frees
 *             the heap ctx without ever double-owning)
 *   Inbound:  RocketResponseMessage{frame} ->
 *             RocketResponseMessage{streamType, requestContext, frame}
 *             (streamType stamped on every stream-scoped frame; requestContext
 *             ownership moved out of the slot on terminal frames so the
 *             AppAdapter can recover the heap ctx)
 */
class RocketClientStreamStateHandler {
 public:
  RocketClientStreamStateHandler() = default;

  using EventId = client::RocketClientEventId;
  static constexpr apache::thrift::fast_thrift::channel_pipeline::Subscriptions<
      EventId::FrameWriteComplete>
      kSubscribedEvents{};

  // === HandlerLifecycle ===

  // The state pointer is cached rather than re-derived per callback so the
  // stream-id generator and the test accessors, which take no context, can
  // still reach the map. build() propagates pipeline state before it calls
  // handlerAdded, so the state is live by the time this runs.
  template <typename Context>
  void handlerAdded(Context& ctx) noexcept {
    contexts_ = &ctx.template state<RocketClientStreamContexts>();
  }

  // The shared stream map is owned by the pipeline and destroyed with it; this
  // handler does not clear it on removal.
  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {
    contexts_ = nullptr;
  }

  // === InboundHandler ===

  template <typename Context>
  void onPipelineActive(Context& /*ctx*/) noexcept {
    // RSocket clients start every connection at streamId 1. On a
    // reactivate (disconnect → reconnect) the slot map should already be
    // empty thanks to onPipelineInactive's drain.
    DCHECK(streams().empty())
        << "onPipelineActive called with " << streams().size()
        << " leftover streams; previous disconnect did not drain";
    nextStreamId_ = 1;
  }

  template <typename Context>
  void onReadReady(Context& /*ctx*/) noexcept {}

  /**
   * Handle inbound frames.
   * Manages stream lifecycle and forwards the message to the next inbound
   * handler.
   */
  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onRead(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto& response = msg.get<RocketResponseMessage>();

    // Hot path: parsed wire frame.
    if (FOLLY_LIKELY(
            response.payload
                .is<apache::thrift::fast_thrift::frame::read::ParsedFrame>())) {
      auto& parsed =
          response.payload
              .get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();

      // Connection-level frames pass through unchanged
      if (parsed.isConnectionFrame()) {
        return ctx.fireRead(std::move(msg));
      }

      auto it = streams().find(parsed.streamId());
      if (it == streams().end()) {
        XLOG(ERR) << "Received frame for unknown stream: streamId="
                  << parsed.streamId() << ", frameType=" << parsed.typeName();
        return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
      }

      // Stamp streamType on every stream-scoped frame so downstream
      // per-pattern handlers can dispatch statelessly. On terminal
      // frames also move requestContext ownership out of the slot map
      // so the AppAdapter receives it (or, if the message is dropped
      // anywhere upstream, the deleter on the response frees the
      // heap ctx). Non-terminal frames leave the slot intact and
      // carry no requestContext today — when streaming patterns land,
      // a raw observer field will be added alongside the owning
      // handle for intermediate frame delivery.
      response.streamType = it->second.streamType;
      if (parsed.isTerminalFrame()) {
        response.requestContext = std::move(it->second.requestContext);
        streams().erase(it);
      }

      return ctx.fireRead(std::move(msg));
    }

    // Cold path: in-process per-request error (e.g. from codec serialize
    // failure). Always terminal; clean up the stream and stamp identity.
    auto& err = response.payload.get<RocketResponseError>();
    auto it = streams().find(err.streamId);
    if (it == streams().end()) {
      XLOG(ERR) << "Received per-request error for unknown stream: streamId="
                << err.streamId;
      return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
    }
    response.streamType = it->second.streamType;
    response.requestContext = std::move(it->second.requestContext);
    streams().erase(it);
    return ctx.fireRead(std::move(msg));
  }

  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    // Connection-fatal exception. Fail every in-flight (sent) request
    // with the wrapped reason — matches legacy
    // RocketClient::failAllSentWrites semantics (each request resolves
    // with the actual connection exception, not the generic auto-detach
    // error). Then propagate upward so the AppAdapter flips state to
    // Closed and tears down the pipeline.
    failAllActiveStreams(ctx, e);
    ctx.fireException(std::move(e));
  }

  // === OutboundHandler ===

  /**
   * Handle outbound requests from the application.
   *
   * - Assigns a streamId and stores the stream state
   * - Forwards the message to the next handler
   *
   * This overload conforms to the pipeline handler concept (TypeErasedBox).
   * The handler extracts the request context, assigns a stream ID, and
   * forwards the message unchanged to downstream handlers.
   */
  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onWrite(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto& request = msg.get<RocketRequestMessage>();
    // Today only REQUEST_RESPONSE is stamped here. SETUP frames are
    // injected downstream of this handler by RocketClientSetupFrameHandler
    // and never reach this onWrite. When more RPC patterns are added, this
    // becomes a frameType-dispatched stamp of `request.frame.streamId`.
    DCHECK(
        request.frame.frameType ==
        apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE)
        << "StreamStateHandler::onWrite saw an unexpected frame type";

    DCHECK(request.frame.streamId == kInvalidStreamId);
    if (request.frame.streamId != kInvalidStreamId) {
      return apache::thrift::fast_thrift::channel_pipeline::Result::Error;
    }

    request.frame.streamId = generateStreamId();
    const uint32_t sid = request.frame.streamId;
    DCHECK(!streams().contains(sid))
        << "Stream ID " << sid << " already exists in active streams";

    // Insert into the slot BEFORE fireWrite so the codec's synthesized
    // RocketResponseError path (fired upward from inside fireWrite) can
    // find the slot and route the error correctly through onRead. The
    // slot owns the requestContext now — on Result::Error below we erase
    // the slot, which destructs the TypeErasedPtr and fires the
    // rescue deleter so the caller's handler resolves promptly rather
    // than waiting for connection teardown.
    streams().emplace(
        sid,
        RocketClientStreamContext{
            .requestContext = std::move(request.requestContext),
            .streamType = request.streamType,
            .stats = {},
        });

    auto result = ctx.fireWrite(std::move(msg));
    if (FOLLY_UNLIKELY(
            result ==
            apache::thrift::fast_thrift::channel_pipeline::Result::Error)) {
      // Rollback: this request was queued but never reached the wire.
      // Fail it as an unwritten request — matches legacy
      // RocketClient::failAllScheduledWrites semantics. No-op if the
      // inbound error path already removed the slot (e.g., codec
      // synthesized RocketResponseError → fireRead → our onRead erased
      // it before returning).
      failStreamOnWriteError(ctx, sid);
    }
    return result;
  }

  template <typename Context>
  void onPipelineInactive(Context& ctx) noexcept {
    // Graceful close. Fail every in-flight (sent) request with
    // END_OF_FILE — matches legacy RocketClient::onConnectionClosed
    // semantics. No-op if onException already ran the fan-out (slot
    // map is empty).
    failAllActiveStreams(
        ctx,
        folly::make_exception_wrapper<
            apache::thrift::transport::TTransportException>(
            apache::thrift::transport::TTransportException::END_OF_FILE,
            "Connection closed"));
  }

  template <typename Context>
  void onWriteReady(Context& /*ctx*/) noexcept {}

  // === EventSubscriber ===

  template <typename Context>
  void onEvent(
      Context& ctx,
      EventId /*ev*/,
      const apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&
          box) noexcept {
    const auto& event = box.get<client::FrameWriteCompleteEvent>();
    auto it = streams().find(event.streamId);
    if (it == streams().end()) {
      return;
    }
    ctx.fireEvent(
        EventId::RocketWriteComplete,
        apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox(
            client::RocketWriteCompleteEvent{
                .requestContext = it->second.requestContext.get(),
                .status = event.status,
            }));
  }

  // === Stream ID Generation ===

  /**
   * Generate the next stream ID.
   *
   * Client generates odd IDs: 1, 3, 5, ... Caps at the RSocket 31-bit
   * limit; wraps back to 1 and latches `hitMaxStreamId_` so subsequent
   * allocations skip IDs still live in the shared stream table. The
   * collision loop is bounded by `streams().size() + 1` — a contiguous run
   * of collisions cannot exceed the number of live streams.
   */
  uint32_t generateStreamId() noexcept {
    uint32_t id;
    size_t iter = 0;
    const size_t maxIter = streams().size() + 1;
    do {
      id = nextStreamId_;
      nextStreamId_ += 2;
      XCHECK_LE(++iter, maxIter)
          << "generateStreamId collision loop exceeded live-stream bound";
    } while (hitMaxStreamId_ && streams().contains(id));
    if (FOLLY_UNLIKELY(id == kMaxStreamId)) {
      nextStreamId_ = 1;
      hitMaxStreamId_ = true;
    }
    return id;
  }

  // === Accessors for testing ===

  uint32_t nextStreamId() const noexcept { return nextStreamId_; }

  size_t activeStreamCount() const noexcept { return streams().size(); }

  bool hasActiveStream(uint32_t streamId) const noexcept {
    return streams().find(streamId) != streams().end();
  }

  // For testing only - allows setting the next stream ID to simulate collisions
  void setNextStreamIdForTest(uint32_t id) noexcept { nextStreamId_ = id; }

 private:
  using StreamMap = apache::thrift::fast_thrift::frame::read::DirectStreamMap<
      RocketClientStreamContext>;

  /// The connection's shared stream table. Valid between handlerAdded and
  /// handlerRemoved, which is every point a handler callback or a test
  /// accessor can run.
  StreamMap& streams() noexcept { return contexts_->streams; }
  const StreamMap& streams() const noexcept { return contexts_->streams; }

  /**
   * Fail one stream whose outbound write returned Result::Error from a
   * downstream handler before reaching the wire. Mirrors legacy
   * RocketClient::failAllScheduledWrites (per-stream).
   *
   * The unit is a stream slot, not a request — in REQUEST_RESPONSE
   * today they coincide, but as streaming patterns land this also
   * covers a continuation/cancel write that failed for an existing
   * stream.
   *
   * Resolves the handler with `TTransportException::NOT_OPEN`, message
   * "Dropping unsent request. Pipeline write failed." — same code as
   * legacy and nearly the same wording, with our local cause in place
   * of legacy's "Connection closed after: <ew>".
   *
   * No-op if the slot is already gone.
   */
  template <typename Context>
  void failStreamOnWriteError(Context& ctx, uint32_t streamId) noexcept {
    failStream(
        ctx,
        streamId,
        folly::make_exception_wrapper<
            apache::thrift::transport::TTransportException>(
            apache::thrift::transport::TTransportException::NOT_OPEN,
            "Dropping unsent request. Pipeline write failed."));
  }

  /**
   * Fail every in-flight (sent) stream with `ew`. Used on connection
   * teardown — `onException` passes the propagated wrapped reason,
   * `onPipelineInactive` synthesizes END_OF_FILE. Mirrors legacy
   * RocketClient::failAllSentWrites.
   *
   * Snapshots stream IDs before iterating because each failStream does
   * a fireRead that may re-enter and mutate the slot map.
   */
  template <typename Context>
  void failAllActiveStreams(
      Context& ctx, const folly::exception_wrapper& ew) noexcept {
    if (streams().empty()) {
      return;
    }
    std::vector<uint32_t> ids;
    ids.reserve(streams().size());
    streams().forEach([&](uint32_t key, RocketClientStreamContext& /*val*/) {
      ids.push_back(key);
    });
    for (uint32_t sid : ids) {
      failStream(ctx, sid, ew);
    }
  }

  /**
   * Mechanism: synthesize a RocketResponseError for one stream and route
   * it inbound via `fireRead`. Skips own onRead via the cached
   * next-handler pointer in ContextImpl, so the bridge / AppAdapter sees
   * the error directly and resolves the handler with `ew` — not the
   * generic auto-detach exception that ~TypeErasedPtr would fire.
   *
   * Moves `requestContext` out of the slot BEFORE erase so the slot
   * destructor sees an empty TypeErasedPtr; the only thing that runs
   * the deleter is our explicit fireRead with `ew` attached.
   *
   * No-op if the slot is already gone (e.g., the inbound terminal-frame
   * path already erased it).
   */
  template <typename Context>
  void failStream(
      Context& ctx, uint32_t streamId, folly::exception_wrapper ew) noexcept {
    auto it = streams().find(streamId);
    if (it == streams().end()) {
      return;
    }
    RocketResponseMessage msg;
    msg.payload = RocketResponseError{
        .ew = std::move(ew),
        .streamId = streamId,
    };
    msg.streamType = it->second.streamType;
    msg.requestContext = std::move(it->second.requestContext);
    streams().erase(it);
    (void)ctx.fireRead(
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(msg)));
  }

  static constexpr uint32_t kMaxStreamId = (1u << 31) - 1;

  uint32_t nextStreamId_{1};
  bool hitMaxStreamId_{false};

  /// The connection's shared stream table, owned by the pipeline. Non-null
  /// between handlerAdded and handlerRemoved.
  RocketClientStreamContexts* contexts_{nullptr};
};

static_assert(
    apache::thrift::fast_thrift::channel_pipeline::DuplexHandler<
        RocketClientStreamStateHandler,
        apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl>,
    "RocketClientStreamStateHandler must satisfy DuplexHandler concept");

static_assert(
    apache::thrift::fast_thrift::channel_pipeline::EventSubscriber<
        RocketClientStreamStateHandler,
        apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl>,
    "RocketClientStreamStateHandler must satisfy EventSubscriber concept");

} // namespace apache::thrift::fast_thrift::rocket::client::handler
