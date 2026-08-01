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

#include <cstdint>
#include <limits>

#include <fmt/core.h>
#include <folly/ExceptionWrapper.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Handler.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>
#include <thrift/lib/cpp2/fast_thrift/frame/ErrorCode.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameViews.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/common/RocketStreamContext.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Messages.h>

namespace apache::thrift::fast_thrift::rocket::server::handler {

/**
 * RocketServerStreamHandler - Per-pattern handler for the REQUEST_STREAM
 * RPC pattern on the server side.
 *
 * Peer to RocketServerRequestResponseHandler. Stateless: per-stream state lives
 * in the pipeline-level `RocketStreamContexts` map (reached through
 * `ctx.state<RocketStreamContexts>()`), whose entry lifecycle is owned by
 * `RocketServerStreamStateHandler`. This handler only reads/writes the
 * `credits` field on an existing entry; it never inserts or erases entries.
 *
 * It acts on a frame only when the shared entry exists and its `streamType` is
 * REQUEST_STREAM; every other stream (and every unknown streamId) passes
 * through untouched. It enforces the RSocket credit invariant: the responder
 * MUST NOT send more PAYLOAD frames than the requester has granted via
 * REQUEST_N. On a violation it synthesizes an outbound ERROR; the erase falls
 * out of StreamStateHandler handling that terminal ERROR, so this handler does
 * not clean up itself.
 *
 * Pipeline position (StreamStateHandler is head-ward, so on inbound the shared
 * entry already exists when this handler runs; on outbound this handler runs
 * first, so its credit enforcement precedes StreamStateHandler's terminal
 * erase):
 *   App -> RocketServerStreamHandler -> StreamStateHandler -> ...
 */
class RocketServerStreamHandler {
 public:
  RocketServerStreamHandler() = default;

  // === HandlerLifecycle ===

  template <typename Context>
  void handlerAdded(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {}

  // === InboundHandler ===

  template <typename Context>
  void onPipelineActive(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onReadReady(Context& /*ctx*/) noexcept {}

  template <typename Context>
  [[nodiscard]] apache::thrift::fast_thrift::channel_pipeline::Result onRead(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto& request = msg.get<RocketRequestMessage>();
    auto& streams = ctx.template state<RocketStreamContexts>().streams;
    uint32_t streamId = request.frame.streamId();

    auto it = streams.find(streamId);
    if (it == streams.end() ||
        it->second.streamType !=
            apache::thrift::fast_thrift::frame::FrameType::REQUEST_STREAM) {
      return ctx.fireRead(std::move(msg));
    }

    auto frameType = request.frame.type();

    if (frameType ==
        apache::thrift::fast_thrift::frame::FrameType::REQUEST_STREAM) {
      return onInboundRequestStream(ctx, it->second, request, std::move(msg));
    }

    if (frameType == apache::thrift::fast_thrift::frame::FrameType::REQUEST_N) {
      return onInboundRequestN(ctx, it->second, request, std::move(msg));
    }

    if (frameType == apache::thrift::fast_thrift::frame::FrameType::EXT) {
      return onInboundExt(ctx, request, std::move(msg));
    }

    // CANCEL and other continuation frames carry no credit action; the shared
    // entry's lifecycle is StreamStateHandler's responsibility.
    return ctx.fireRead(std::move(msg));
  }

  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }

  // === OutboundHandler ===

  template <typename Context>
  [[nodiscard]] apache::thrift::fast_thrift::channel_pipeline::Result onWrite(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto& response = msg.get<RocketResponseMessage>();
    auto& streams = ctx.template state<RocketStreamContexts>().streams;
    uint32_t streamId = response.frame.streamId;

    auto it = streams.find(streamId);
    if (it == streams.end() ||
        it->second.streamType !=
            apache::thrift::fast_thrift::frame::FrameType::REQUEST_STREAM) {
      return ctx.fireWrite(std::move(msg));
    }

    if (response.frame.frameType ==
        apache::thrift::fast_thrift::frame::FrameType::PAYLOAD) {
      return onOutboundPayload(ctx, it->second, response, std::move(msg));
    }

    // ERROR is terminal; StreamStateHandler erases the entry. Forward as-is.
    if (response.frame.frameType ==
        apache::thrift::fast_thrift::frame::FrameType::ERROR) {
      return ctx.fireWrite(std::move(msg));
    }

    return synthesizeError(
        ctx,
        streamId,
        fmt::format(
            "invalid outbound frame type {} on streaming stream",
            static_cast<int>(response.frame.frameType)));
  }

  template <typename Context>
  void onPipelineInactive(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onWriteReady(Context& /*ctx*/) noexcept {}

 private:
  template <typename Context>
  [[nodiscard]] apache::thrift::fast_thrift::channel_pipeline::Result
  onInboundRequestStream(
      Context& ctx,
      RocketStreamContext& entry,
      RocketRequestMessage& request,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    uint32_t streamId = request.frame.streamId();

    auto view = apache::thrift::fast_thrift::frame::read::asView<
        apache::thrift::fast_thrift::frame::read::RequestStreamView>(
        request.frame);
    auto rawN = view.initialRequestN();
    auto n = static_cast<int32_t>(rawN);

    if (n <= 0) {
      return synthesizeError(
          ctx, streamId, "REQUEST_STREAM with invalid initialRequestN");
    }

    // Seed credits on the entry StreamStateHandler already created for this
    // stream open (credits default to 0 there).
    entry.credits = rawN;

    return ctx.fireRead(std::move(msg));
  }

  template <typename Context>
  [[nodiscard]] apache::thrift::fast_thrift::channel_pipeline::Result
  onInboundRequestN(
      Context& ctx,
      RocketStreamContext& entry,
      RocketRequestMessage& request,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    uint32_t streamId = request.frame.streamId();

    auto view = apache::thrift::fast_thrift::frame::read::asView<
        apache::thrift::fast_thrift::frame::read::RequestNView>(request.frame);
    auto rawN = view.requestN();
    auto n = static_cast<int32_t>(rawN);

    if (n <= 0) {
      return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
    }

    auto increment = static_cast<uint64_t>(rawN);
    if (entry.credits > std::numeric_limits<uint64_t>::max() - increment) {
      return synthesizeError(
          ctx, streamId, "REQUEST_N would overflow credit counter");
    }
    entry.credits += increment;

    return ctx.fireRead(std::move(msg));
  }

  template <typename Context>
  [[nodiscard]] apache::thrift::fast_thrift::channel_pipeline::Result
  onInboundExt(
      Context& ctx,
      RocketRequestMessage& request,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
      /*msg*/) noexcept {
    // EXT is consumed either way (dropped when ignorable, replaced by a
    // synthesized ERROR otherwise), so the inbound frame is never forwarded.
    if (request.frame.metadata.shouldIgnore()) {
      return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
    }

    return synthesizeError(
        ctx, request.frame.streamId(), "non-ignorable EXT on streaming stream");
  }

  template <typename Context>
  [[nodiscard]] apache::thrift::fast_thrift::channel_pipeline::Result
  onOutboundPayload(
      Context& ctx,
      RocketStreamContext& entry,
      RocketResponseMessage& response,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    uint32_t streamId = response.frame.streamId;
    bool hasNext = response.frame.next;
    bool isComplete = response.frame.complete;

    if (hasNext) {
      if (entry.credits == 0) {
        return synthesizeError(
            ctx, streamId, "outbound PAYLOAD exceeds granted credits");
      }
      --entry.credits;
    }

    auto result = ctx.fireWrite(std::move(msg));

    // Write failure rollback: non-terminal PAYLOAD re-increments credits to
    // stay consistent with the peer (frame was never sent). Terminal writes are
    // not rolled back — StreamStateHandler has already erased the entry, so the
    // re-find below misses and is a no-op.
    if (result ==
            apache::thrift::fast_thrift::channel_pipeline::Result::Error &&
        hasNext && !isComplete) {
      auto& streams = ctx.template state<RocketStreamContexts>().streams;
      auto rollbackIt = streams.find(streamId);
      if (rollbackIt != streams.end()) {
        ++rollbackIt->second.credits;
      }
    }

    return result;
  }

  template <typename Context>
  [[nodiscard]] apache::thrift::fast_thrift::channel_pipeline::Result
  synthesizeError(
      Context& ctx, uint32_t streamId, std::string description) noexcept {
    RocketResponseMessage errorResponse{
        .frame =
            apache::thrift::fast_thrift::frame::ComposedFrame{
                .frameType =
                    apache::thrift::fast_thrift::frame::FrameType::ERROR,
                .streamId = streamId,
                .data = folly::IOBuf::fromString(std::move(description)),
                .errorCode = static_cast<uint32_t>(
                    apache::thrift::fast_thrift::frame::ErrorCode::INVALID),
            },
        .streamType =
            apache::thrift::fast_thrift::frame::FrameType::REQUEST_STREAM,
    };
    return ctx.fireWrite(
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(errorResponse)));
  }
};

// Note: unlike stateless handlers, this handler reaches pipeline-level state
// via `ctx.state<RocketStreamContexts>()`, which lives on the TypedContext view
// (not the bare ContextImpl). Handlers never name TypedContext, so the
// DuplexHandler concept is verified when the handler is added to a
// state-registering pipeline (see PipelineBuilder::addNextDuplex), matching
// RocketServerStreamStateHandler.

} // namespace apache::thrift::fast_thrift::rocket::server::handler
