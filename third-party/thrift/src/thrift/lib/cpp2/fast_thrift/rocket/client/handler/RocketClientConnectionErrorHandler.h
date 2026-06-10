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

#include <glog/logging.h>
#include <folly/ExceptionWrapper.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>
#include <thrift/lib/cpp2/fast_thrift/frame/ErrorCode.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameViews.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Event.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>

namespace apache::thrift::fast_thrift::rocket::client::handler {

// Import ErrorCode from framing namespace for use in this handler
using apache::thrift::fast_thrift::frame::ErrorCode;
using apache::thrift::fast_thrift::frame::toString;

/**
 * RocketClientConnectionErrorHandler - Pipeline handler for client-side ERROR
 * frame processing.
 *
 * This handler intercepts fatal connection-level ERROR frames (streamId == 0)
 * from the server — invalid/rejected setup, abrupt connection error, and
 * protocol violations — and converts them to the appropriate exception.
 *
 * CONNECTION_CLOSE is the exception: it is a graceful-drain signal, not a
 * fault, so instead of an exception it fires a RocketClientEvent with
 * Kind::ConnectionClose up the pipeline (the app adapter relays it to the
 * thrift drain handler). In-flight
 * work is untouched. Stream-level ERROR frames (streamId > 0) also pass
 * through unchanged.
 */
class RocketClientConnectionErrorHandler {
 public:
  RocketClientConnectionErrorHandler() = default;

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

  /**
   * Process inbound frames, intercepting fatal connection-level ERROR frames
   * (streamId == 0) and converting them to a TTransportException.
   *
   * CONNECTION_CLOSE fires a RocketClientEvent (Kind::ConnectionClose) up the
   * pipeline (graceful drain, not a fault). Stream-level ERROR frames and all
   * other frames pass
   * through unchanged.
   */
  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onRead(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto& response = msg.get<RocketResponseMessage>();

    // In-process per-request errors are not connection-level ERRORs.
    if (FOLLY_UNLIKELY(
            !response.payload.is<
                apache::thrift::fast_thrift::frame::read::ParsedFrame>())) {
      return ctx.fireRead(std::move(msg));
    }
    auto& parsed =
        response.payload
            .get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();

    // Only handle connection-level ERROR frames (streamId == 0)
    if (parsed.type() != apache::thrift::fast_thrift::frame::FrameType::ERROR ||
        !parsed.isConnectionFrame()) {
      return ctx.fireRead(std::move(msg));
    }

    // CONNECTION_CLOSE is a graceful-drain signal, not a fault. Emit a
    // connection-close lifecycle event up the pipeline (the app adapter
    // relays it to the thrift drain handler) and consume the frame — it
    // carries no per-request payload. Every other connection-level error is
    // fatal and is converted to an exception.
    if (extractError(parsed).first == ErrorCode::CONNECTION_CLOSE) {
      ctx.fireEvent(
          RocketClientEventId::ConnectionClose,
          apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
              RocketClientEvent{
                  .kind = RocketClientEvent::Kind::ConnectionClose,
                  .status = {},
                  .frameCount = 0,
                  .bytes = 0}));
      return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
    }

    return handleError(ctx, parsed);
  }

  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }

 private:
  /**
   * Extract error code and msg from ERROR frame.
   */
  std::pair<ErrorCode, std::string_view> extractError(
      const apache::thrift::fast_thrift::frame::read::ParsedFrame&
          frame) noexcept {
    apache::thrift::fast_thrift::frame::read::ErrorView view(frame);
    auto cursor = view.payloadCursor();
    auto bytes = cursor.peekBytes();
    std::string_view msg(
        reinterpret_cast<const char*>(bytes.data()), view.payloadSize());
    return {ErrorCode(view.errorCode()), msg};
  }

  /**
   * Handle connection-level ERROR frame based on error code.
   * Similar to RocketClient::handleError().
   */
  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result handleError(
      Context& ctx,
      apache::thrift::fast_thrift::frame::read::ParsedFrame& frame) noexcept {
    using TTransportException = apache::thrift::transport::TTransportException;

    auto [errorCode, errorMessage] = extractError(frame);

    auto makeExceptionMessage = [&errorMessage](std::string_view prefix) {
      if (errorMessage.empty()) {
        return std::string(prefix);
      }
      return fmt::format("{}: {}", prefix, errorMessage);
    };

    folly::exception_wrapper ew;
    switch (errorCode) {
      case ErrorCode::INVALID_SETUP:
        ew = folly::make_exception_wrapper<TTransportException>(
            TTransportException::INVALID_SETUP,
            makeExceptionMessage("Connection setup failed: invalid setup"));
        break;

      case ErrorCode::UNSUPPORTED_SETUP:
        ew = folly::make_exception_wrapper<TTransportException>(
            TTransportException::NOT_SUPPORTED,
            makeExceptionMessage(
                "Connection setup failed: unsupported setup parameters"));
        break;

      case ErrorCode::REJECTED_SETUP:
        ew = folly::make_exception_wrapper<TTransportException>(
            TTransportException::NOT_OPEN,
            makeExceptionMessage(
                "Connection setup failed: setup rejected by server"));
        break;

      case ErrorCode::REJECTED_RESUME:
        ew = folly::make_exception_wrapper<TTransportException>(
            TTransportException::NOT_OPEN,
            makeExceptionMessage(
                "Connection resume failed: resume rejected by server"));
        break;

      case ErrorCode::CONNECTION_ERROR:
        ew = folly::make_exception_wrapper<TTransportException>(
            TTransportException::END_OF_FILE,
            makeExceptionMessage("Connection error from server"));
        break;

      // Stream-level error codes are illegal on the connection stream
      // (streamId 0): per the RSocket spec they MUST carry streamId > 0.
      // Receiving one here is a peer protocol violation.
      case ErrorCode::APPLICATION_ERROR:
      case ErrorCode::REJECTED:
      case ErrorCode::CANCELED:
      case ErrorCode::INVALID:
        ew = folly::make_exception_wrapper<TTransportException>(
            TTransportException::END_OF_FILE,
            makeExceptionMessage(
                fmt::format(
                    "Protocol violation: stream-level error code {} on connection stream",
                    toString(errorCode))));
        break;

      // Reserved codes must never appear on the wire.
      case ErrorCode::RESERVED:
      case ErrorCode::RESERVED_EXT:
        ew = folly::make_exception_wrapper<TTransportException>(
            TTransportException::END_OF_FILE,
            makeExceptionMessage(
                fmt::format(
                    "Protocol violation: reserved error code {} on connection stream",
                    toString(errorCode))));
        break;

      // onRead intercepts CONNECTION_CLOSE as a graceful-drain event; reaching
      // here is a programming error.
      case ErrorCode::CONNECTION_CLOSE:
        DCHECK(false) << "CONNECTION_CLOSE reached handleError";
        [[fallthrough]];

      default:
        ew = folly::make_exception_wrapper<TTransportException>(
            TTransportException::END_OF_FILE,
            makeExceptionMessage(
                fmt::format(
                    "Unknown error code {} on connection stream",
                    static_cast<uint32_t>(errorCode))));
        break;
    }

    ctx.fireException(std::move(ew));
    return apache::thrift::fast_thrift::channel_pipeline::Result::Error;
  }
};

static_assert(
    apache::thrift::fast_thrift::channel_pipeline::InboundHandler<
        RocketClientConnectionErrorHandler,
        apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl>,
    "RocketClientConnectionErrorHandler must satisfy InboundHandler concept");

} // namespace apache::thrift::fast_thrift::rocket::client::handler
