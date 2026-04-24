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
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Messages.h>

#include <cstdint>
#include <utility>
#include <variant>

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
 *     streamId}
 *   Outbound: RocketResponseMessage{payload, streamId, complete} ->
 *     RocketResponseMessage{payload, streamId}
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
   * - Connection-level frames (streamId == 0): pass through
   * - Request-initiating frames: register new stream, fire to app
   * - CANCEL/ERROR: remove active stream (terminal), fire to app
   * - Non-terminal frames (e.g., REQUEST_N): pass through
   * - Unknown streamId: log warning and drop
   */
  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onRead(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto& frame =
        msg.get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();
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
      return onTerminalEvent(ctx, streamId, desc, std::move(frame));
    }

    if (desc.isRequestFrame) {
      return onNewStream(ctx, streamId, std::move(frame));
    }

    return onStreamFrame(ctx, streamId, desc, std::move(msg));
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

    uint32_t streamId = response.streamId;
    bool complete = response.complete;

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
      apache::thrift::fast_thrift::frame::read::ParsedFrame&& frame) noexcept {
    auto it = activeStreams_.find(streamId);
    if (it == activeStreams_.end()) {
      logUnknownStreamId(desc, streamId);
      return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
    }
    activeStreams_.erase(it);

    RocketRequestMessage request{
        .frame = std::move(frame), .error = {}, .streamId = streamId};
    return ctx.fireRead(
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(request)));
  }

  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onNewStream(
      Context& ctx,
      uint32_t streamId,
      apache::thrift::fast_thrift::frame::read::ParsedFrame&& frame) noexcept {
    if (activeStreams_.find(streamId) != activeStreams_.end()) {
      return apache::thrift::fast_thrift::channel_pipeline::Result::Error;
    }

    activeStreams_.emplace(streamId, std::monostate{});

    RocketRequestMessage request{
        .frame = std::move(frame), .error = {}, .streamId = streamId};
    auto result = ctx.fireRead(
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(request)));

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
    if (activeStreams_.find(streamId) != activeStreams_.end()) {
      return ctx.fireRead(std::move(msg));
    }

    logUnknownStreamId(desc, streamId);
    return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
  }

  apache::thrift::fast_thrift::frame::read::DirectStreamMap<std::monostate>
      activeStreams_;
};

} // namespace apache::thrift::fast_thrift::rocket::server::handler
