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
#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/common/RocketStreamContext.h>
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
 * Per-stream state lives in the pipeline-level `RocketStreamContexts` state,
 * registered once per connection via
 * `PipelineBuilder::addState<RocketStreamContexts>()` and reached here through
 * `ctx.state<RocketStreamContexts>()`. This handler owns the entry lifecycle
 * (insert on stream-open, erase on terminal) so per-pattern handlers can share
 * the same map without their own bookkeeping. The pipeline owns the map's
 * lifetime; this handler does not clear it on removal.
 *
 * Pipeline position:
 *   App <-> StreamHandler <-> FrameHandler <-> Transport
 *
 * Message flow:
 *   Inbound:  ParsedFrame{streamId, ...} -> RocketRequestMessage{frame,
 *     streamId, streamType} (streamType stamped from the shared map so
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

  // The shared stream map is owned by the pipeline and destroyed with it; this
  // handler does not clear it on removal.
  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {}

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
    auto& contexts = ctx.template state<RocketStreamContexts>();
    auto& request = msg.get<RocketRequestMessage>();
    auto& frame = request.frame;
    uint32_t streamId = frame.streamId();

    if (frame.isConnectionFrame()) {
      // Connection-level frames (streamId == 0) carry no stream state, so
      // there is nothing for this handler to track. Forward rather than drop:
      // SETUP is answered by the layer above, which needs to see it. Frames
      // consumed below this point (KEEPALIVE) never reach here anyway.
      return ctx.fireRead(std::move(msg));
    }

    auto frameType = frame.type();
    const auto& desc =
        apache::thrift::fast_thrift::frame::getDescriptor(frameType);

    if (frame.isTerminalFrame()) {
      return onTerminalEvent(ctx, contexts, streamId, desc, std::move(msg));
    }

    if (desc.isRequestFrame) {
      return onNewStream(ctx, contexts, streamId, frameType, std::move(msg));
    }

    return onStreamFrame(ctx, contexts, streamId, desc, std::move(msg));
  }

  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.template state<RocketStreamContexts>().streams.clear();
    ctx.fireException(std::move(e));
  }

  // === OutboundHandler ===

  /**
   * Handle outbound responses from the application.
   *
   * - Passes connection-level frames straight through
   * - Validates streamId is an active stream
   * - If complete, removes the stream
   * - Forwards RocketResponseMessage to FrameHandler
   */
  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onWrite(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto& contexts = ctx.template state<RocketStreamContexts>();
    auto& response = msg.get<RocketResponseMessage>();

    // streamId is a direct field read; isComplete() encapsulates the
    // per-frame-type terminal check (ERROR/CANCEL terminal by frame type,
    // PAYLOAD/CHANNEL forward the `complete` flag). The handler only owns
    // the erase policy.
    uint32_t streamId = response.frame.streamId;
    bool complete = response.frame.isComplete();

    // Connection-level frames — the SETUP answer and its refusal — belong to
    // the connection, not to a stream, so there is no context to look up. They
    // are only reachable from above this handler, so an unknown streamId here
    // still means a stream frame that outlived its stream.
    if (FOLLY_UNLIKELY(
            streamId ==
            apache::thrift::fast_thrift::frame::kConnectionStreamId)) {
      return ctx.fireWrite(std::move(msg));
    }

    auto it = contexts.streams.find(streamId);
    if (it == contexts.streams.end()) {
      return apache::thrift::fast_thrift::channel_pipeline::Result::Error;
    }

    if (complete) {
      contexts.streams.erase(it);
    }

    return ctx.fireWrite(std::move(msg));
  }

  template <typename Context>
  void onPipelineInactive(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onWriteReady(Context& /*ctx*/) noexcept {}

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
      RocketStreamContexts& contexts,
      uint32_t streamId,
      const apache::thrift::fast_thrift::frame::FrameDescriptor& desc,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto it = contexts.streams.find(streamId);
    if (it == contexts.streams.end()) {
      logUnknownStreamId(desc, streamId);
      return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
    }
    auto streamType = it->second.streamType;
    contexts.streams.erase(it);

    auto& request = msg.get<RocketRequestMessage>();
    request.streamId = streamId;
    request.streamType = streamType;
    return ctx.fireRead(std::move(msg));
  }

  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onNewStream(
      Context& ctx,
      RocketStreamContexts& contexts,
      uint32_t streamId,
      apache::thrift::fast_thrift::frame::FrameType streamType,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    if (!contexts.streams
             .emplace(streamId, RocketStreamContext{.streamType = streamType})
             .second) {
      return apache::thrift::fast_thrift::channel_pipeline::Result::Error;
    }

    auto& request = msg.get<RocketRequestMessage>();
    request.streamId = streamId;
    request.streamType = streamType;
    auto result = ctx.fireRead(std::move(msg));

    // Only rollback on error; backpressure means the request was accepted.
    if (result ==
        apache::thrift::fast_thrift::channel_pipeline::Result::Error) {
      contexts.streams.erase(streamId);
    }

    return result;
  }

  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onStreamFrame(
      Context& ctx,
      RocketStreamContexts& contexts,
      uint32_t streamId,
      const apache::thrift::fast_thrift::frame::FrameDescriptor& desc,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto it = contexts.streams.find(streamId);
    if (it == contexts.streams.end()) {
      logUnknownStreamId(desc, streamId);
      return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
    }
    auto streamType = it->second.streamType;

    auto& request = msg.get<RocketRequestMessage>();
    request.streamId = streamId;
    request.streamType = streamType;
    return ctx.fireRead(std::move(msg));
  }
};

} // namespace apache::thrift::fast_thrift::rocket::server::handler
