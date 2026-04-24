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
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>
#include <thrift/lib/cpp2/fast_thrift/frame/ErrorCode.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameViews.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>

namespace apache::thrift::fast_thrift::rocket::client::handler {

// Import ErrorCode from framing namespace for use in this handler
using apache::thrift::fast_thrift::frame::ErrorCode;
using apache::thrift::fast_thrift::frame::toString;

/**
 * RocketClientErrorFrameHandler - Pipeline handler for client-side ERROR frame
 * processing.
 *
 * This handler intercepts connection-level ERROR frames (streamId == 0) from
 * the server and converts them to appropriate exceptions. These errors indicate
 * fatal connection problems like invalid setup, rejected setup, or connection
 * close.
 *
 * Similar to how RocketClient::handleError() processes ERROR frames in the
 * existing Rocket client implementation.
 *
 * The handler consumes connection-level ERROR frames and fires exceptions.
 * Stream-level ERROR frames (streamId > 0) pass through unchanged.
 */
class RocketClientErrorFrameHandler {
 public:
  RocketClientErrorFrameHandler() = default;

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
   * Process inbound frames, intercepting connection-level ERROR frames.
   *
   * Connection-level ERROR frames (streamId == 0) are handled here:
   * - Extract error code from frame
   * - Convert to appropriate TTransportException
   * - Fire exception to notify connection failure
   *
   * Stream-level ERROR frames and all other frames pass through unchanged.
   */
  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onRead(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto& response = msg.get<RocketResponseMessage>();

    // Only handle connection-level ERROR frames (streamId == 0)
    if (response.frame.type() !=
            apache::thrift::fast_thrift::frame::FrameType::ERROR ||
        !response.frame.isConnectionFrame()) {
      return ctx.fireRead(std::move(msg));
    }

    // Extract error code and message from ERROR frame
    return handleError(ctx, response.frame);
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
      case ErrorCode::CONNECTION_CLOSE:
        ew = folly::make_exception_wrapper<TTransportException>(
            TTransportException::NOT_OPEN,
            makeExceptionMessage("Connection closed by server"));
        break;

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

      case ErrorCode::RESERVED:
      case ErrorCode::APPLICATION_ERROR:
      case ErrorCode::REJECTED:
      case ErrorCode::CANCELED:
      case ErrorCode::INVALID:
      case ErrorCode::RESERVED_EXT:
      default:
        ew = folly::make_exception_wrapper<TTransportException>(
            TTransportException::END_OF_FILE,
            makeExceptionMessage(
                fmt::format(
                    "Unhandled error frame on control stream [{}]",
                    toString(errorCode))));
        break;
    }

    ctx.fireException(std::move(ew));
    return apache::thrift::fast_thrift::channel_pipeline::Result::Error;
  }
};

static_assert(
    apache::thrift::fast_thrift::channel_pipeline::InboundHandler<
        RocketClientErrorFrameHandler,
        apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl>,
    "RocketClientErrorFrameHandler must satisfy InboundHandler concept");

} // namespace apache::thrift::fast_thrift::rocket::client::handler
