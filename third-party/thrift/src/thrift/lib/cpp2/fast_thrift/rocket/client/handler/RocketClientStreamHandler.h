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

#include <fmt/core.h>
#include <folly/ExceptionWrapper.h>
#include <folly/io/IOBuf.h>
#include <folly/lang/Hint.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Handler.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>
#include <thrift/lib/cpp2/fast_thrift/frame/ErrorCode.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/DirectStreamMap.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>

#include <string>

namespace apache::thrift::fast_thrift::rocket::client::handler {

/**
 * RocketClientStreamHandler - Per-pattern handler for the REQUEST_STREAM
 * RPC pattern on the client side.
 *
 * Peer to RocketClientRequestResponseHandler. Duplex: the outbound path
 * observes REQUEST_STREAM frames to register streamIds in its own
 * DirectStreamSet; the inbound path filters responses by that registry.
 * Validates inbound frame ordering. No credit enforcement (client is the
 * grantor, not the enforcer).
 *
 * Identifies frames via its own registry, not via StreamStateHandler's
 * streamType stamp — per-pattern handlers are order-independent.
 *
 * Pipeline position:
 *   App -> RocketClientStreamHandler -> RR Handler -> StreamState -> ...
 *
 * Lifecycle: streams_ is populated on outbound REQUEST_STREAM and cleaned
 * up on terminal inbound events (ERROR, COMPLETE, RocketResponseError).
 * onException and onPipelineInactive are stateless pass-throughs; the
 * destructor cleans up streams_.
 */
class RocketClientStreamHandler {
 public:
  RocketClientStreamHandler() = default;

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
    auto& response = msg.get<RocketResponseMessage>();

    // In-process errors are always terminal; clean up and pass through.
    if (FOLLY_UNLIKELY(
            !response.payload.is<
                apache::thrift::fast_thrift::frame::read::ParsedFrame>())) {
      auto& err = response.payload.get<RocketResponseError>();
      streams_.erase(err.streamId);
      return ctx.fireRead(std::move(msg));
    }

    auto& parsed =
        response.payload
            .get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();
    uint32_t streamId = parsed.streamId();

    if (streams_.find(streamId) == streams_.end()) {
      return ctx.fireRead(std::move(msg));
    }

    auto frameType = parsed.type();

    if (frameType == apache::thrift::fast_thrift::frame::FrameType::PAYLOAD) {
      return onInboundPayload(ctx, parsed, streamId, std::move(msg));
    }

    if (frameType == apache::thrift::fast_thrift::frame::FrameType::ERROR) {
      streams_.erase(streamId);
      return ctx.fireRead(std::move(msg));
    }

    if (frameType == apache::thrift::fast_thrift::frame::FrameType::EXT) {
      if (parsed.metadata.shouldIgnore()) {
        return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
      }
      streams_.erase(streamId);
      synthesizeStreamError(response, "non-ignorable EXT on streaming stream");
      return ctx.fireRead(std::move(msg));
    }

    streams_.erase(streamId);
    synthesizeStreamError(
        response,
        fmt::format(
            "unexpected frame type {} on streaming stream", parsed.typeName()));
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
    auto& request = msg.get<RocketRequestMessage>();

    if (request.frame.frameType ==
        apache::thrift::fast_thrift::frame::FrameType::REQUEST_STREAM) {
      streams_.emplace(request.frame.streamId);
    }

    return ctx.fireWrite(std::move(msg));
  }

  template <typename Context>
  void onPipelineInactive(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onWriteReady(Context& /*ctx*/) noexcept {}

 private:
  template <typename Context>
  [[nodiscard]] apache::thrift::fast_thrift::channel_pipeline::Result
  onInboundPayload(
      Context& ctx,
      const apache::thrift::fast_thrift::frame::read::ParsedFrame& parsed,
      uint32_t streamId,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    bool hasNext = parsed.hasNext();
    bool isComplete = parsed.isComplete();

    if (!hasNext && !isComplete) {
      auto& response = msg.get<RocketResponseMessage>();
      streams_.erase(streamId);
      synthesizeStreamError(
          response, "malformed PAYLOAD: neither NEXT nor COMPLETE set");
      return ctx.fireRead(std::move(msg));
    }

    if (isComplete) {
      streams_.erase(streamId);
    }

    return ctx.fireRead(std::move(msg));
  }

  /// Replace response.payload with a synthesized ERROR(INVALID) frame on
  /// the same streamId. Caller must have verified the variant holds a
  /// ParsedFrame.
  static void synthesizeStreamError(
      RocketResponseMessage& response, std::string description) noexcept {
    const uint32_t streamId =
        response.payload
            .get<apache::thrift::fast_thrift::frame::read::ParsedFrame>()
            .streamId();
    auto buf =
        apache::thrift::fast_thrift::frame::ComposedFrame{
            .frameType = apache::thrift::fast_thrift::frame::FrameType::ERROR,
            .streamId = streamId,
            .data = folly::IOBuf::fromString(std::move(description)),
            .errorCode = static_cast<uint32_t>(
                apache::thrift::fast_thrift::frame::ErrorCode::INVALID),
        }
            .serialize();
    response.payload =
        apache::thrift::fast_thrift::frame::read::parseFrame(std::move(buf));
  }

  apache::thrift::fast_thrift::frame::read::DirectStreamSet<> streams_;
};

static_assert(
    apache::thrift::fast_thrift::channel_pipeline::DuplexHandler<
        RocketClientStreamHandler,
        apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl>,
    "RocketClientStreamHandler must satisfy DuplexHandler concept");

} // namespace apache::thrift::fast_thrift::rocket::client::handler
