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

#include <folly/container/F14Set.h>
#include <folly/io/IOBuf.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Handler.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>

namespace apache::thrift::fast_thrift::rocket::client::handler {

/**
 * RocketClientRequestResponseFrameHandler - Duplex pipeline handler for
 * REQUEST_RESPONSE interactions.
 *
 * This handler manages both outbound request serialization and inbound
 * response handling for REQUEST_RESPONSE streams. It tracks which stream IDs
 * belong to request-response interactions to properly route inbound frames.
 *
 * Pipeline position:
 *   App <-> RocketClientSetupFrameHandler <-> RocketClientStreamStateHandler
 * <-> RocketClientRequestResponseFrameHandler <-> Transport
 *
 * Message flow:
 *   Outbound: RocketRequestMessage{streamId, frameType, payload} ->
 *             std::unique_ptr<folly::IOBuf> (serialized frame)
 *   Inbound:  ParsedFrame -> (if request-response stream) handle locally,
 *             otherwise forward to next handler
 */
class RocketClientRequestResponseFrameHandler {
 public:
  RocketClientRequestResponseFrameHandler() = default;

  // === HandlerLifecycle ===

  template <typename Context>
  void handlerAdded(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {
    requestResponseStreams_.clear();
  }

  // === InboundHandler ===

  /**
   * Handle inbound frames.
   *
   * Checks if the frame belongs to a request-response stream. If so,
   * validates the frame and forwards it. Otherwise, passes through to
   * the next handler.
   */
  template <typename Context>
  [[nodiscard]] apache::thrift::fast_thrift::channel_pipeline::Result onRead(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto& response = msg.get<RocketResponseMessage>();

    // Check if this is a request-response stream
    auto it = requestResponseStreams_.find(response.frame.streamId());
    if (it == requestResponseStreams_.end()) {
      // Not a request-response stream, forward to next handler
      return ctx.fireRead(std::move(msg));
    } else {
      // Erase from tracking since we received the response
      requestResponseStreams_.erase(it);
    }

    if (FOLLY_UNLIKELY(!isResponseValid(response))) {
      return apache::thrift::fast_thrift::channel_pipeline::Result::Error;
    } else {
      return ctx.fireRead(std::move(msg));
    }
  }

  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    requestResponseStreams_.clear();
    ctx.fireException(std::move(e));
  }

  // === OutboundHandler ===

  /**
   * Handle outbound RocketRequestMessage.
   *
   * For REQUEST_RESPONSE frames: tracks the stream ID and serializes
   * the payload into a wire-format frame.
   * For other frame types: forwards unchanged to the next handler.
   */
  template <typename Context>
  void onPipelineActivated(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onPipelineDeactivated(Context& /*ctx*/) noexcept {
    requestResponseStreams_.clear();
  }

  template <typename Context>
  void onWriteReady(Context& /*ctx*/) noexcept {}

  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onWrite(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto& request = msg.get<RocketRequestMessage>();

    // We only handle REQUEST_RESPONSE frames
    if (request.frameType !=
        apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE) {
      return ctx.fireWrite(std::move(msg));
    }

    // Get the payload from the variant
    auto& payload = request.frame.get<RocketFramePayload>();

    // Track this stream ID as a request-response stream
    // Cache the stream ID for the case where we fail to send the frame
    uint32_t streamId = payload.streamId;
    requestResponseStreams_.insert(streamId);

    // Serialize the request into a frame
    auto serializedFrame = apache::thrift::fast_thrift::frame::write::serialize(
        apache::thrift::fast_thrift::frame::write::RequestResponseHeader{
            .streamId = streamId,
            .follows = payload.follows,
        },
        std::move(payload.metadata),
        std::move(payload.data));

    // Replace payload with serialized frame in the variant
    request.frame = std::move(serializedFrame);

    auto result = ctx.fireWrite(std::move(msg));
    if (result ==
        apache::thrift::fast_thrift::channel_pipeline::Result::Error) {
      // If we failed to send the frame, we should remove from tracking
      requestResponseStreams_.erase(streamId);
    }
    return result;
  }

  // === Accessors for testing ===

  size_t pendingRequestResponseCount() const noexcept {
    return requestResponseStreams_.size();
  }

  bool hasPendingRequestResponse(uint32_t streamId) const noexcept {
    return requestResponseStreams_.find(streamId) !=
        requestResponseStreams_.end();
  }

 private:
  /**
   * Validates frame type and flags and returns false if the response is
   * invalid.
   */
  bool isResponseValid(RocketResponseMessage& response) noexcept {
    // NOLINTNEXTLINE(clang-diagnostic-switch-enum)
    switch (response.frame.type()) {
      case apache::thrift::fast_thrift::frame::FrameType::PAYLOAD: {
        // For request-response, payload must have next or complete flag
        if (!response.frame.hasNext() && !response.frame.isComplete()) {
          XLOG(ERR)
              << "Client received single response payload without next or "
                 "complete flag: streamId="
              << response.frame.streamId();
          return false;
        }
        return true;
      }
      case apache::thrift::fast_thrift::frame::FrameType::ERROR: {
        return true;
      }
      default:
        XLOG(ERR) << "Unexpected frame type for request-response stream: "
                  << "streamId=" << response.frame.streamId() << ", frameType="
                  << static_cast<uint8_t>(response.frame.type());
        return false;
    }
  }

  // Set of stream IDs that are request-response interactions
  folly::F14FastSet<uint32_t> requestResponseStreams_;
};

static_assert(
    apache::thrift::fast_thrift::channel_pipeline::DuplexHandler<
        RocketClientRequestResponseFrameHandler,
        apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl>,
    "RocketClientRequestResponseFrameHandler must satisfy DuplexHandler concept");

} // namespace apache::thrift::fast_thrift::rocket::client::handler
