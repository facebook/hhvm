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
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/DirectStreamMap.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameViews.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>

#include <cstdint>

namespace apache::thrift::fast_thrift::rocket::client::handler {

/**
 * RocketClientStreamStateHandler - Pipeline handler for client-side RSocket
 * stream state management.
 *
 * This handler sits between the application layer and the FrameHandler,
 * managing stream IDs, storing request callbacks, and correlating responses.
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

  // === HandlerLifecycle ===

  template <typename Context>
  void handlerAdded(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {
    activeStreams_.clear();
    nextStreamId_ = 1;
  }

  // === InboundHandler ===

  template <typename Context>
  void onPipelineActive(Context& /*ctx*/) noexcept {}

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

      auto it = activeStreams_.find(parsed.streamId());
      if (it == activeStreams_.end()) {
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
        activeStreams_.erase(it);
      }

      return ctx.fireRead(std::move(msg));
    }

    // Cold path: in-process per-request error (e.g. from codec serialize
    // failure). Always terminal; clean up the stream and stamp identity.
    auto& err = response.payload.get<RocketResponseError>();
    auto it = activeStreams_.find(err.streamId);
    if (it == activeStreams_.end()) {
      XLOG(ERR) << "Received per-request error for unknown stream: streamId="
                << err.streamId;
      return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
    }
    response.streamType = it->second.streamType;
    response.requestContext = std::move(it->second.requestContext);
    activeStreams_.erase(it);
    return ctx.fireRead(std::move(msg));
  }

  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    activeStreams_.clear();
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
    // Today only ComposedRequestResponseFrame is stamped here. SETUP frames
    // are injected downstream of this handler by RocketClientSetupFrameHandler
    // and never reach this onWrite. When more RPC patterns are added, this
    // becomes a `request.frame.visit(...)` that stamps
    // `payload.header.streamId` uniformly across pattern arms.
    DCHECK(request.frame.is<
           apache::thrift::fast_thrift::frame::ComposedRequestResponseFrame>())
        << "StreamStateHandler::onWrite saw an unexpected payload arm";
    auto& payload = request.frame.get<
        apache::thrift::fast_thrift::frame::ComposedRequestResponseFrame>();

    DCHECK(payload.streamId() == kInvalidStreamId);
    if (payload.streamId() != kInvalidStreamId) {
      return apache::thrift::fast_thrift::channel_pipeline::Result::Error;
    }

    payload.header.streamId = generateStreamId();
    const uint32_t sid = payload.streamId();
    DCHECK(!activeStreams_.contains(sid))
        << "Stream ID " << sid << " already exists in active streams";

    // Insert into the slot BEFORE fireWrite so the codec's synthesized
    // RocketResponseError path (fired upward from inside fireWrite) can
    // find the slot and route the error correctly through onRead. The
    // slot owns the requestContext now — on Result::Error below we erase
    // the slot, which destructs the TypeErasedPtr and fires the
    // rescue deleter so the caller's handler resolves promptly rather
    // than waiting for connection teardown.
    activeStreams_.emplace(
        sid,
        ClientStreamContext{
            .requestContext = std::move(request.requestContext),
            .streamType = request.streamType,
        });

    auto result = ctx.fireWrite(std::move(msg));
    if (FOLLY_UNLIKELY(
            result ==
            apache::thrift::fast_thrift::channel_pipeline::Result::Error)) {
      // erase is a no-op if the inbound error path already removed the
      // slot (e.g., codec synthesized RocketResponseError → fireRead →
      // our onRead erased it before returning).
      activeStreams_.erase(sid);
    }
    return result;
  }

  template <typename Context>
  void onPipelineInactive(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onWriteReady(Context& /*ctx*/) noexcept {}

  // === Stream ID Generation ===

  /**
   * Generate the next stream ID.
   *
   * Client generates odd IDs: 1, 3, 5, ...
   */
  uint32_t generateStreamId() noexcept {
    uint32_t id = nextStreamId_;
    nextStreamId_ += 2;
    return id;
  }

  // === Accessors for testing ===

  uint32_t nextStreamId() const noexcept { return nextStreamId_; }

  size_t activeStreamCount() const noexcept { return activeStreams_.size(); }

  bool hasActiveStream(uint32_t streamId) const noexcept {
    return activeStreams_.find(streamId) != activeStreams_.end();
  }

  // For testing only - allows setting the next stream ID to simulate collisions
  void setNextStreamIdForTest(uint32_t id) noexcept { nextStreamId_ = id; }

 private:
  /**
   * ClientStreamContext - State stored for each pending request stream.
   *
   * `requestContext` is a type-erased owning handle to the AppAdapter's
   * heap-allocated per-request context. The rocket layer never
   * dereferences the pointer; it only routes the handle and runs the
   * deleter the AppAdapter supplied on any cleanup path (terminal frame,
   * activeStreams_.clear() on connection teardown, ~DirectStreamMap on
   * pipeline destruction). The slot map never holds a stale pointer.
   */
  struct ClientStreamContext {
    TypeErasedPtr requestContext;
    apache::thrift::fast_thrift::frame::FrameType streamType;
  };

  uint32_t nextStreamId_{1};
  apache::thrift::fast_thrift::frame::read::DirectStreamMap<ClientStreamContext>
      activeStreams_;
};

static_assert(
    apache::thrift::fast_thrift::channel_pipeline::DuplexHandler<
        RocketClientStreamStateHandler,
        apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl>,
    "RocketClientStreamStateHandler must satisfy DuplexHandler concept");

} // namespace apache::thrift::fast_thrift::rocket::client::handler
