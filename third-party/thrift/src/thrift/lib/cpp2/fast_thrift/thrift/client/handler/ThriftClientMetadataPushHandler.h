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
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Handler.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/Messages.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>

namespace apache::thrift::fast_thrift::thrift::client::handler {

/**
 * ThriftClientMetadataPushHandler - Inbound handler for processing
 * METADATA_PUSH frames from the server.
 *
 * This handler intercepts and processes METADATA_PUSH frames which carry
 * ServerPushMetadata including:
 * - setupResponse: Server's response to SETUP (version, zstd support)
 * - streamHeadersPush: Headers for a specific stream
 * - drainCompletePush: Server is draining connections
 *
 * Message flow:
 *   Inbound: ThriftResponseMessage with METADATA_PUSH → consumed (not
 * forwarded) Inbound: Other ThriftResponseMessage → passed through unchanged
 *
 * This is an inbound-only handler - outbound messages pass through unchanged.
 */
class ThriftClientMetadataPushHandler {
 public:
  ThriftClientMetadataPushHandler() = default;

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
   * Process inbound frames, intercepting connection frames.
   *
   * Connection frames are handled here and NOT forwarded:
   * - METADATA_PUSH: Deserialized and processed (setupResponse, drain, etc.)
   * - KEEPALIVE: Logged and consumed
   * - Other connection frames: Logged and consumed
   *
   * Stream-level frames pass through unchanged.
   */
  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onRead(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto& response = msg.get<ThriftResponseMessage>();
    auto& frame = response.frame;

    // Pass through non-METADATA_PUSH frames
    if (frame.type() !=
        apache::thrift::fast_thrift::frame::FrameType::METADATA_PUSH) {
      return ctx.fireRead(std::move(msg));
    }

    return handleMetadataPush(ctx, frame);
  }

  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }

  // === Accessors for testing ===

  int32_t serverVersion() const noexcept { return serverVersion_; }

  bool isSetupComplete() const noexcept { return setupComplete_; }

  bool serverSupportsZstd() const noexcept { return serverSupportsZstd_; }

 private:
  /**
   * Handle METADATA_PUSH frames from the server.
   * Deserializes ServerPushMetadata and dispatches based on type.
   */
  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result handleMetadataPush(
      Context& ctx,
      apache::thrift::fast_thrift::frame::read::ParsedFrame& frame) noexcept {
    // Deserialize ServerPushMetadata from the frame
    // METADATA_PUSH frames have the entire payload as metadata
    apache::thrift::ServerPushMetadata serverMeta;
    try {
      apache::thrift::CompactProtocolReader reader;
      auto cursor = frame.payloadCursor();
      reader.setInput(cursor);
      serverMeta.read(&reader);
    } catch (const std::exception& ex) {
      XLOG(ERR) << "Failed to deserialize METADATA_PUSH frame: " << ex.what();
      ctx.fireException(
          folly::make_exception_wrapper<
              apache::thrift::transport::TTransportException>(
              apache::thrift::transport::TTransportException::CORRUPTED_DATA,
              fmt::format(
                  "Failed to deserialize metadata push frame: {}", ex.what())));
      return apache::thrift::fast_thrift::channel_pipeline::Result::Error;
    }

    // Dispatch based on ServerPushMetadata type
    // NOLINTNEXTLINE(clang-diagnostic-switch-enum)
    switch (serverMeta.getType()) {
      case apache::thrift::ServerPushMetadata::Type::setupResponse:
        return handleSetupResponse(serverMeta.get_setupResponse());

      case apache::thrift::ServerPushMetadata::Type::drainCompletePush:
        return handleDrainComplete(ctx, serverMeta.get_drainCompletePush());

      case apache::thrift::ServerPushMetadata::Type::streamHeadersPush:
        XLOG(DBG2) << "Received streamHeadersPush, ignoring (not supported)";
        break;

      default:
        break;
    }

    return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
  }

  /**
   * Handle setupResponse from the server.
   * Stores server version and marks setup as complete.
   */
  apache::thrift::fast_thrift::channel_pipeline::Result handleSetupResponse(
      const apache::thrift::SetupResponse& setupResponse) noexcept {
    serverVersion_ = setupResponse.version().value_or(0);
    serverSupportsZstd_ = setupResponse.zstdSupported().value_or(false);
    setupComplete_ = true;
    XLOG(DBG2) << "Received setupResponse: version=" << serverVersion_
               << ", zstdSupported=" << serverSupportsZstd_;
    return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
  }

  /**
   * Handle drainCompletePush from the server.
   *
   * Generic drain is informational — the actual close is driven separately
   * (CONNECTION_CLOSE error frame). EXCEEDED_INGRESS_MEM_LIMIT is the
   * exception: the server is loadshedding and pending requests should be
   * failed with LOADSHEDDING semantics so callers can back off / retry.
   */
  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result handleDrainComplete(
      Context& ctx,
      const apache::thrift::DrainCompletePush& drainPush) noexcept {
    auto drainCode = drainPush.drainCompleteCode();
    if (drainCode &&
        *drainCode ==
            apache::thrift::DrainCompleteCode::EXCEEDED_INGRESS_MEM_LIMIT) {
      ctx.fireException(
          folly::make_exception_wrapper<apache::thrift::TApplicationException>(
              apache::thrift::TApplicationException::LOADSHEDDING,
              "Server exceeded ingress memory limit"));
    }
    return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
  }

  int32_t serverVersion_{0};
  bool setupComplete_{false};
  bool serverSupportsZstd_{false};
};

static_assert(
    apache::thrift::fast_thrift::channel_pipeline::InboundHandler<
        ThriftClientMetadataPushHandler,
        apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl>,
    "ThriftClientMetadataPushHandler must satisfy InboundHandler concept");

} // namespace apache::thrift::fast_thrift::thrift::client::handler
