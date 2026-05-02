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
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Handler.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>
#include <thrift/lib/cpp2/fast_thrift/frame/ErrorCode.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Messages.h>

#include <variant>

namespace apache::thrift::fast_thrift::rocket::server::handler {

/**
 * RocketServerRequestResponseHandler - Per-pattern handler for the
 * REQUEST_RESPONSE RPC pattern on the server side.
 *
 * Frame-level concerns (parsing/serializing wire bytes) live in
 * RocketServerFrameCodecHandler; per-stream lifecycle (registration,
 * streamType stamping, terminal-frame cleanup, duplicate-response
 * rejection) lives in RocketServerStreamStateHandler. This handler owns
 * RPC-pattern semantics specific to RR:
 *
 * Inbound (after RocketServerStreamStateHandler stamps streamType):
 *   - Pattern filter: only act on streams established as REQUEST_RESPONSE;
 *     frames belonging to other RPC patterns flow through unchanged so
 *     their respective per-pattern handlers can process them.
 *   - REQUEST_RESPONSE: the initial request from the client.
 *   - CANCEL: the only other valid client-initiated frame on an RR stream.
 *     Forwarded to the application so it can abort in-flight work.
 *   - EXT: per RSocket spec, silently dropped when the `ignore` flag is
 *     set; otherwise treated as a protocol violation.
 *   - Anything else (REQUEST_N, REQUEST_STREAM/CHANNEL/FNF on an active
 *     RR streamId, non-ignorable EXT, ...) is a peer protocol violation
 *     per RSocket spec. The handler synthesizes a stream-level
 *     ERROR(INVALID) response on the same streamId via ctx.fireWrite,
 *     terminating the stream while keeping the connection alive.
 *     Result::Error is reserved for connection-fatal events.
 *
 * Outbound:
 *   - Pattern filter: only act on RR stream responses.
 *   - ComposedPayloadFrame: stamps `complete = true, next = true` (RR is
 *     single-shot terminal).
 *   - ComposedErrorFrame: ERROR is implicitly terminal — no stamping
 *     needed. Pass through.
 *   - Other patterns: pass through unchanged.
 *
 * Stateless. Stream lifecycle (insert on request, erase on completion,
 * duplicate-response rejection) is enforced upstream by
 * RocketServerStreamStateHandler.
 *
 * Pipeline position:
 *   App -> RocketServerRequestResponseHandler -> StreamStateHandler -> ...
 *   (i.e., closest to the application on both directions)
 */
class RocketServerRequestResponseHandler {
 public:
  RocketServerRequestResponseHandler() = default;

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

    // Pattern filter: only act on streams established as REQUEST_RESPONSE.
    // Frames belonging to other RPC patterns flow through unchanged so their
    // respective pattern handlers can process them.
    if (request.streamType !=
        apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE) {
      return ctx.fireRead(std::move(msg));
    }

    // In-process per-request errors are already terminal failures; no
    // pattern validation applies. Pass through to the App callback.
    if (FOLLY_UNLIKELY(
            !request.payload.is<
                apache::thrift::fast_thrift::frame::read::ParsedFrame>())) {
      return ctx.fireRead(std::move(msg));
    }
    auto& parsed =
        request.payload
            .get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();

    auto frameType = parsed.type();

    // Hot path: the initial REQUEST_RESPONSE frame from the client.
    if (FOLLY_LIKELY(
            frameType ==
            apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE)) {
      return ctx.fireRead(std::move(msg));
    }

    // Cold path: CANCEL is the only other valid client-initiated frame on an
    // RR stream. Forward to the application so it can abort in-flight work.
    if (frameType == apache::thrift::fast_thrift::frame::FrameType::CANCEL) {
      return ctx.fireRead(std::move(msg));
    }

    // EXT frames carry an `ignore` flag: per RSocket spec, an unrecognized
    // EXT MUST be dropped silently when ignore=true and rejected with
    // ERROR(INVALID) otherwise.
    if (frameType == apache::thrift::fast_thrift::frame::FrameType::EXT &&
        parsed.metadata.shouldIgnore()) {
      return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
    }

    // Anything else (REQUEST_N, REQUEST_STREAM/CHANNEL/FNF on an active RR
    // streamId, non-ignorable EXT, etc.) is a peer protocol violation per
    // RSocket spec, which warrants closing the stream.
    XLOG(ERR) << "Unexpected frame type for REQUEST_RESPONSE stream: streamId="
              << request.streamId << ", frameType=" << parsed.typeName()
              << "; sending ERROR(INVALID)";

    auto description = fmt::format(
        "unexpected frame type {} on REQUEST_RESPONSE stream",
        parsed.typeName());
    RocketResponseMessage errorResponse{
        .frame =
            apache::thrift::fast_thrift::frame::ComposedErrorFrame{
                .data = folly::IOBuf::fromString(std::move(description)),
                .metadata = nullptr,
                .header =
                    {.streamId = request.streamId,
                     .errorCode =
                         static_cast<uint32_t>(apache::thrift::fast_thrift::
                                                   frame::ErrorCode::INVALID)},
            },
        .streamType =
            apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE,
    };
    return ctx.fireWrite(
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(errorResponse)));
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

    // Pattern filter: only act on RR stream responses.
    if (FOLLY_UNLIKELY(
            response.streamType !=
            apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE)) {
      return ctx.fireWrite(std::move(msg));
    }

    // Stamp RR terminal flags on ComposedPayloadFrame — RR is single-shot.
    // ComposedErrorFrame is implicitly terminal; no stamping needed.
    if (response.frame
            .is<apache::thrift::fast_thrift::frame::ComposedPayloadFrame>()) {
      auto& payload =
          response.frame
              .get<apache::thrift::fast_thrift::frame::ComposedPayloadFrame>();
      payload.header.complete = true;
      payload.header.next = true;
    }

    return ctx.fireWrite(std::move(msg));
  }

  template <typename Context>
  void onPipelineInactive(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onWriteReady(Context& /*ctx*/) noexcept {}
};

static_assert(
    apache::thrift::fast_thrift::channel_pipeline::DuplexHandler<
        RocketServerRequestResponseHandler,
        apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl>,
    "RocketServerRequestResponseHandler must satisfy DuplexHandler concept");

} // namespace apache::thrift::fast_thrift::rocket::server::handler
