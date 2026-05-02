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
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>

#include <string>

namespace apache::thrift::fast_thrift::rocket::client::handler {

/**
 * RocketClientRequestResponseHandler - Per-pattern handler for the
 * REQUEST_RESPONSE RPC pattern on the client side.
 *
 * Frame-level concerns (parsing/serializing wire bytes) live in
 * RocketClientFrameCodecHandler; per-stream lifecycle (streamId allocation,
 * streamType stamping, terminal-frame cleanup) lives in
 * RocketClientStreamStateHandler. This handler owns RPC-pattern semantics
 * specific to RR:
 *
 * Inbound (after RocketClientStreamStateHandler stamps streamType):
 *   - Pattern filter: only act on responses whose stream originated from a
 *     REQUEST_RESPONSE request; everything else passes through.
 *   - PAYLOAD: the only legal data-bearing response. Must carry NEXT;
 *     a COMPLETE-only PAYLOAD on RR is a peer protocol violation.
 *   - ERROR: terminal failure response, forwarded to App.
 *   - EXT: per RSocket spec, silently dropped when the `ignore` flag is set;
 *     otherwise treated as a protocol violation.
 *   - Anything else (REQUEST_N, server-initiated CANCEL, ...) is a peer
 *     protocol violation. The handler synthesizes a stream-level
 *     ERROR(INVALID) response in place of the offending frame and forwards
 *     it upstream so the App's pending callback resolves with a failure.
 *     Result::Error is reserved for connection-fatal events.
 *
 * Inbound-only by design: REQUEST_RESPONSE has no follow-up frames from the
 * requester (no REQUEST_N, no client-side wire CANCEL — RR cancellation is
 * handled by per-request timeout / connection close, never on the wire). All
 * outbound concerns belong upstream (request shaping in App, streamId
 * assignment in StreamStateHandler, serialization in FrameCodecHandler).
 *
 * Pipeline position:
 *   App <- RocketClientRequestResponseHandler <- StreamStateHandler <- ...
 *   (i.e., closest to the application on the inbound direction)
 */
class RocketClientRequestResponseHandler {
 public:
  RocketClientRequestResponseHandler() = default;

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
  apache::thrift::fast_thrift::channel_pipeline::Result onRead(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto& response = msg.get<RocketResponseMessage>();

    // Pattern filter: only act on RR streams; let other patterns pass through.
    if (response.streamType !=
        apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE) {
      return ctx.fireRead(std::move(msg));
    }

    auto frameType = response.frame.type();

    // Hot path: PAYLOAD response. Per spec, RR responses MUST carry NEXT
    // (the response data); a COMPLETE-only PAYLOAD is a peer protocol
    // violation, surfaced to the App as a synthetic ERROR.
    if (FOLLY_LIKELY(
            frameType ==
            apache::thrift::fast_thrift::frame::FrameType::PAYLOAD)) {
      if (FOLLY_UNLIKELY(!response.frame.hasNext())) {
        XLOG(ERR) << "RR PAYLOAD response missing NEXT flag: streamId="
                  << response.frame.streamId();
        synthesizeStreamError(
            response, "RR PAYLOAD response missing NEXT flag");
      }
      return ctx.fireRead(std::move(msg));
    }

    // Cold path: ERROR response.
    if (frameType == apache::thrift::fast_thrift::frame::FrameType::ERROR) {
      return ctx.fireRead(std::move(msg));
    }

    // EXT with ignore=true: per RSocket spec, drop silently and keep the
    // stream alive (the server may follow up with a real response).
    if (frameType == apache::thrift::fast_thrift::frame::FrameType::EXT &&
        response.frame.metadata.shouldIgnore()) {
      return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
    }

    // Any other frame type on an RR stream (REQUEST_N, server-initiated
    // CANCEL, non-ignorable EXT, ...) is a peer protocol violation per
    // RSocket spec. Surface it to the App as a synthetic ERROR(INVALID) so
    // the pending request callback resolves rather than hangs; the
    // connection stays up.
    XLOG(ERR) << "Unexpected frame type for REQUEST_RESPONSE stream: streamId="
              << response.frame.streamId()
              << ", frameType=" << response.frame.typeName();
    synthesizeStreamError(
        response,
        fmt::format(
            "unexpected frame type {} on REQUEST_RESPONSE stream",
            response.frame.typeName()));
    return ctx.fireRead(std::move(msg));
  }

  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }

 private:
  // Replace `response.frame` with a freshly serialized stream-level
  // ERROR(INVALID) frame on the same streamId. Used to convert peer
  // protocol violations into a per-stream failure delivered to the App.
  static void synthesizeStreamError(
      RocketResponseMessage& response, std::string description) noexcept {
    auto buf =
        apache::thrift::fast_thrift::frame::ComposedErrorFrame{
            .data = folly::IOBuf::fromString(std::move(description)),
            .header =
                {.streamId = response.frame.streamId(),
                 .errorCode = static_cast<uint32_t>(
                     apache::thrift::fast_thrift::frame::ErrorCode::INVALID)},
        }
            .serialize();
    response.frame =
        apache::thrift::fast_thrift::frame::read::parseFrame(std::move(buf));
  }
};

static_assert(
    apache::thrift::fast_thrift::channel_pipeline::InboundHandler<
        RocketClientRequestResponseHandler,
        apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl>,
    "RocketClientRequestResponseHandler must satisfy InboundHandler concept");

} // namespace apache::thrift::fast_thrift::rocket::client::handler
