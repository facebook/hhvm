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

#include <cstring>

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
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameViews.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Messages.h>

namespace apache::thrift::fast_thrift::rocket::server::handler {

/**
 * RocketServerKeepAliveHandler - Pipeline handler for the RSocket
 * connection-level KEEPALIVE frame.
 *
 * Positioned on the transport side of RocketServerStreamStateHandler so its
 * echo, emitted via ctx.fireWrite, travels toward the transport without
 * re-entering StreamStateHandler::onWrite (which rejects streamId 0 as an
 * unknown stream).
 *
 * Pipeline position:
 *   Transport <-> ... <-> SetupFrameHandler <-> KeepAliveHandler <->
 *   StreamStateHandler <-> App
 *
 * Inbound:
 *   - KEEPALIVE with respond flag: echo a KEEPALIVE back with the respond
 *     flag cleared, mirroring the payload; lastReceivedPosition is 0 since
 *     stream resumption is unsupported. The frame is consumed.
 *   - KEEPALIVE without respond flag: the peer's reply to a keepalive this
 *     server never sends. Consumed silently.
 *   - KEEPALIVE with a non-zero stream ID: a protocol violation. Emit a
 *     connection-level ERROR(CONNECTION_ERROR) and close the connection.
 *   - Any other frame: forwarded downstream unchanged.
 *
 * Outbound: passthrough.
 */
class RocketServerKeepAliveHandler {
 public:
  RocketServerKeepAliveHandler() = default;

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
    auto& frame = request.frame;

    if (FOLLY_LIKELY(
            frame.type() !=
            apache::thrift::fast_thrift::frame::FrameType::KEEPALIVE)) {
      return ctx.fireRead(std::move(msg));
    }

    // KEEPALIVE is a connection-level frame; a non-zero stream ID is a peer
    // protocol violation that terminates the connection (stock parity).
    if (FOLLY_UNLIKELY(!frame.isConnectionFrame())) {
      XLOG(ERR) << "Received KEEPALIVE with non-zero streamId="
                << frame.streamId() << "; closing connection";
      return sendConnectionError(ctx, "KEEPALIVE must use stream ID 0");
    }

    // Only reply when the peer sets the respond flag. A KEEPALIVE without it
    // is the peer's echo of a keepalive this server never initiates, so it is
    // consumed without a reply.
    apache::thrift::fast_thrift::frame::read::KeepAliveView view(frame);
    if (view.shouldRespond()) {
      RocketResponseMessage echo{
          .frame =
              apache::thrift::fast_thrift::frame::ComposedFrame{
                  .frameType =
                      apache::thrift::fast_thrift::frame::FrameType::KEEPALIVE,
                  .streamId =
                      apache::thrift::fast_thrift::frame::kConnectionStreamId,
                  .data = std::move(frame).extractData(),
                  .respond = false,
                  .lastReceivedPosition = 0,
              },
      };
      if (ctx.fireWrite(
              apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
                  std::move(echo))) ==
          apache::thrift::fast_thrift::channel_pipeline::Result::Error) {
        XLOG(WARN) << "Failed to write KEEPALIVE echo";
      }
    }

    // Connection-level protocol frame fully handled here; never forwarded to
    // downstream stream handlers or the application.
    return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
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
    return ctx.fireWrite(std::move(msg));
  }

  template <typename Context>
  void onPipelineInactive(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onWriteReady(Context& /*ctx*/) noexcept {}

 private:
  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result sendConnectionError(
      Context& ctx, const char* message) noexcept {
    try {
      auto errorData = ctx.copyBuffer(message, std::strlen(message));
      RocketResponseMessage response{
          .frame =
              apache::thrift::fast_thrift::frame::ComposedFrame{
                  .frameType =
                      apache::thrift::fast_thrift::frame::FrameType::ERROR,
                  .streamId =
                      apache::thrift::fast_thrift::frame::kConnectionStreamId,
                  .data = std::move(errorData),
                  .errorCode =
                      static_cast<uint32_t>(apache::thrift::fast_thrift::frame::
                                                ErrorCode::CONNECTION_ERROR),
              },
      };
      if (ctx.fireWrite(
              apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
                  std::move(response))) !=
          apache::thrift::fast_thrift::channel_pipeline::Result::Success) {
        XLOG(WARN) << "Failed to deliver KEEPALIVE CONNECTION_ERROR frame";
      }
    } catch (...) {
      XLOG(ERR)
          << "Exception while delivering KEEPALIVE CONNECTION_ERROR frame";
    }

    ctx.close();

    return apache::thrift::fast_thrift::channel_pipeline::Result::Error;
  }
};

static_assert(
    apache::thrift::fast_thrift::channel_pipeline::DuplexHandler<
        RocketServerKeepAliveHandler,
        apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl>,
    "RocketServerKeepAliveHandler must satisfy DuplexHandler concept");

} // namespace apache::thrift::fast_thrift::rocket::server::handler
