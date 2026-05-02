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
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameDescriptor.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/DirectStreamMap.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Messages.h>

#include <cstdint>
#include <utility>

namespace apache::thrift::fast_thrift::rocket::server::handler {

/**
 * RocketServerStreamStateHandler - Pipeline handler for server-side RSocket
 * stream management.
 *
 * This handler sits between the application layer and the FrameHandler,
 * tracking active streams initiated by clients and routing responses.
 *
 * Pipeline position:
 *   App <-> StreamHandler <-> FrameHandler <-> Transport
 *
 * Message flow:
 *   Inbound:  ParsedFrame{streamId, ...} -> RocketRequestMessage{frame,
 *     streamId, streamType} (streamType stamped from per-stream map so
 *     downstream per-pattern handlers can dispatch statelessly)
 *   Outbound: RocketResponseMessage{frame, streamType (set by App)} ->
 *     RocketResponseMessage forwarded; lifecycle managed by streamId.
 */
class RocketServerStreamStateHandler {
 public:
  RocketServerStreamStateHandler() = default;

  // === HandlerLifecycle ===

  template <typename Context>
  void handlerAdded(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {
    activeStreams_.clear();
  }

  // === InboundHandler ===

  template <typename Context>
  void onPipelineActive(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onReadReady(Context& /*ctx*/) noexcept {}

  /**
   * Handle inbound frames from FrameHandler.
   *
   * Hot path: parsed wire frame.
   * - Connection-level frames (streamId == 0): pass through
   * - Request-initiating frames: register new stream, fire to app
   * - CANCEL/ERROR: remove active stream (terminal), fire to app
   * - Non-terminal frames (e.g., REQUEST_N): pass through
   * - Unknown streamId: log warning and drop
   *
   * Cold path: RocketRequestMessage carrying RocketRequestError. This is
   * an in-process per-request failure routed back inbound by the codec
   * (e.g., outbound serialize threw). Always terminal; clean up the
   * stream and stamp identity, then forward to App.
   */
  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onRead(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto& request = msg.get<RocketRequestMessage>();

    // Hot path: parsed wire frame from the codec.
    if (FOLLY_LIKELY(
            request.payload
                .is<apache::thrift::fast_thrift::frame::read::ParsedFrame>())) {
      auto& frame =
          request.payload
              .get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();
      uint32_t streamId = frame.streamId();

      if (frame.isConnectionFrame()) {
        // Connection-level frames (streamId == 0) like KEEPALIVE are
        // protocol-level frames handled below this layer. Do not forward
        // to the application layer.
        return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
      }

      auto frameType = frame.type();
      const auto& desc =
          apache::thrift::fast_thrift::frame::getDescriptor(frameType);

      if (frame.isTerminalFrame()) {
        return onTerminalEvent(ctx, streamId, desc, std::move(msg));
      }

      if (desc.isRequestFrame) {
        return onNewStream(ctx, streamId, frameType, std::move(msg));
      }

      return onStreamFrame(ctx, streamId, desc, std::move(msg));
    }

    // Cold path: in-process per-request error (e.g. from codec serialize
    // failure). Always terminal — App must be notified so it can clean up
    // its per-stream state. The codec has already stamped streamId and
    // streamType on the message, so no lookup is required to forward.
    // If the stream is still tracked here (e.g. non-terminal response
    // failed), erase it; for terminal responses the outbound onWrite path
    // has already erased.
    if (auto it = activeStreams_.find(request.streamId);
        it != activeStreams_.end()) {
      activeStreams_.erase(it);
    }
    return ctx.fireRead(std::move(msg));
  }

  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    activeStreams_.clear();
    ctx.fireException(std::move(e));
  }

  // === OutboundHandler ===

  /**
   * Handle outbound responses from the application.
   *
   * - Validates streamId is an active stream
   * - If complete, removes the stream
   * - Forwards RocketResponseMessage to FrameHandler
   */
  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onWrite(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto& response = msg.get<RocketResponseMessage>();

    // streamId() is a free read on ComposedFrameVariant. "complete" depends
    // on the held alternative: ERROR is implicitly terminal; PAYLOAD uses
    // its header.complete flag.
    uint32_t streamId = response.frame.streamId();
    bool complete =
        response.frame
            .is<apache::thrift::fast_thrift::frame::ComposedErrorFrame>() ||
        response.frame
            .get<apache::thrift::fast_thrift::frame::ComposedPayloadFrame>()
            .header.complete;

    auto it = activeStreams_.find(streamId);
    if (it == activeStreams_.end()) {
      return apache::thrift::fast_thrift::channel_pipeline::Result::Error;
    }

    if (complete) {
      activeStreams_.erase(it);
    }

    return ctx.fireWrite(std::move(msg));
  }

  template <typename Context>
  void onPipelineInactive(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onWriteReady(Context& /*ctx*/) noexcept {}

  // === Accessors for testing ===

  size_t activeStreamCount() const noexcept { return activeStreams_.size(); }

  bool hasActiveStream(uint32_t streamId) const noexcept {
    return activeStreams_.find(streamId) != activeStreams_.end();
  }

 private:
  void logUnknownStreamId(
      const apache::thrift::fast_thrift::frame::FrameDescriptor& desc,
      uint32_t streamId) noexcept {
    XLOG(WARN) << "Received " << desc.name
               << " for unknown streamId=" << streamId << ", dropping";
  }

  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onTerminalEvent(
      Context& ctx,
      uint32_t streamId,
      const apache::thrift::fast_thrift::frame::FrameDescriptor& desc,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto it = activeStreams_.find(streamId);
    if (it == activeStreams_.end()) {
      logUnknownStreamId(desc, streamId);
      return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
    }
    auto streamType = it->second.streamType;
    activeStreams_.erase(it);

    auto& request = msg.get<RocketRequestMessage>();
    request.streamId = streamId;
    request.streamType = streamType;
    return ctx.fireRead(std::move(msg));
  }

  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onNewStream(
      Context& ctx,
      uint32_t streamId,
      apache::thrift::fast_thrift::frame::FrameType streamType,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    if (activeStreams_.find(streamId) != activeStreams_.end()) {
      return apache::thrift::fast_thrift::channel_pipeline::Result::Error;
    }

    activeStreams_.emplace(
        streamId, ServerStreamContext{.streamType = streamType});

    auto& request = msg.get<RocketRequestMessage>();
    request.streamId = streamId;
    request.streamType = streamType;
    auto result = ctx.fireRead(std::move(msg));

    // Only rollback on error; backpressure means the request was accepted.
    if (result ==
        apache::thrift::fast_thrift::channel_pipeline::Result::Error) {
      activeStreams_.erase(streamId);
    }

    return result;
  }

  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onStreamFrame(
      Context& ctx,
      uint32_t streamId,
      const apache::thrift::fast_thrift::frame::FrameDescriptor& desc,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto it = activeStreams_.find(streamId);
    if (it == activeStreams_.end()) {
      logUnknownStreamId(desc, streamId);
      return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
    }
    auto streamType = it->second.streamType;

    auto& request = msg.get<RocketRequestMessage>();
    request.streamId = streamId;
    request.streamType = streamType;
    return ctx.fireRead(std::move(msg));
  }

  // ServerStreamContext - per-stream state held in activeStreams_.
  // Holds the originating REQUEST_* frame type so per-pattern handlers
  // downstream can dispatch statelessly via the stamped streamType.
  struct ServerStreamContext {
    apache::thrift::fast_thrift::frame::FrameType streamType{
        apache::thrift::fast_thrift::frame::FrameType::RESERVED};
  };

  apache::thrift::fast_thrift::frame::read::DirectStreamMap<ServerStreamContext>
      activeStreams_;
};

} // namespace apache::thrift::fast_thrift::rocket::server::handler
