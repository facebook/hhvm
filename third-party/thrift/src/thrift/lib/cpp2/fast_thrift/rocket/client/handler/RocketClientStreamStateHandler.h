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
 *   Outbound: RocketRequestMessage{frameType, payload, callback} ->
 *             RocketRequestMessage{streamId, frameType, payload} (with streamId
 * assigned) Inbound:  ParsedFrame{streamId, ...} ->
 *             RocketResponseMessage{requestFrameType, callback, payload, ...}
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
  void onPipelineActivated(Context& /*ctx*/) noexcept {}

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

    // Connection-level frames pass through unchanged
    if (response.frame.isConnectionFrame()) {
      return ctx.fireRead(std::move(msg));
    }

    // Make sure we have an active stream for this frame
    auto it = activeStreams_.find(response.frame.streamId());
    if (it == activeStreams_.end()) {
      XLOG(ERR) << "Received frame for unknown stream: streamId="
                << response.frame.streamId()
                << ", frameType=" << response.frame.typeName();
      return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
    }

    // For non-terminal frames, just pass through (e.g., REQUEST_N)
    // Don't remove from active streams yet
    if (!response.frame.isTerminalFrame()) {
      return ctx.fireRead(std::move(msg));
    }

    // Extract stream state and erase from active streams
    response.requestHandle = it->second.requestHandle;
    response.requestFrameType = it->second.requestFrameType;
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
    auto& payload = request.frame.get<RocketFramePayload>();

    DCHECK(payload.streamId == kInvalidStreamId);
    if (payload.streamId != kInvalidStreamId) {
      return apache::thrift::fast_thrift::channel_pipeline::Result::Error;
    }

    payload.streamId = generateStreamId();
    DCHECK(!activeStreams_.contains(payload.streamId))
        << "Stream ID " << payload.streamId
        << " already exists in active streams";
    activeStreams_.emplace(
        payload.streamId,
        ClientStreamContext{
            .requestFrameType = request.frameType,
            .requestHandle = request.requestHandle,
        });

    return ctx.fireWrite(std::move(msg));
  }

  template <typename Context>
  void onPipelineDeactivated(Context& /*ctx*/) noexcept {}

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
   * Used to track active streams and store per-stream context needed to
   * correlate responses with their original requests.
   */
  struct ClientStreamContext {
    apache::thrift::fast_thrift::frame::FrameType requestFrameType;
    uint32_t requestHandle;
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
