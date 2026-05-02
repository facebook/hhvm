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

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Messages.h>

#include <folly/container/F14Set.h>

namespace apache::thrift::fast_thrift::rocket::server::handler {

/**
 * RocketServerRequestResponseFrameHandler - Duplex pipeline handler for
 * REQUEST_RESPONSE interactions on the server side.
 *
 * Responsibilities (post-codec-serialization migration):
 *   - Inbound: track REQUEST_RESPONSE streamIds so outbound responses can be
 *     identified as RR-pattern. Roll back tracking on downstream error.
 *   - Outbound: for tracked RR streams, stamp `complete = true, next = true`
 *     on the held `ComposedPayloadFrame` header (RR is single-shot terminal).
 *     Does NOT serialize — RocketServerFrameCodecHandler does.
 *
 * Pipeline position:
 *   App <-> RocketServerStreamStateHandler <->
 *           RocketServerRequestResponseFrameHandler <-> Codec <-> Transport
 */
class RocketServerRequestResponseFrameHandler {
 public:
  RocketServerRequestResponseFrameHandler() = default;

  // === HandlerLifecycle ===

  template <typename Context>
  void handlerAdded(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {
    requestResponseStreams_.clear();
  }

  // === InboundHandler ===

  template <typename Context>
  void onPipelineActive(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onReadReady(Context& /*ctx*/) noexcept {}

  /**
   * Handle inbound frames.
   *
   * For REQUEST_RESPONSE frames: tracks the stream ID and forwards
   * the frame to the next handler. Rolls back tracking if downstream
   * returns an error.
   * For terminal frames (CANCEL, ERROR): removes tracking for the stream
   * if it was a request-response stream.
   * For all other frames: forwards unchanged.
   */
  template <typename Context>
  [[nodiscard]] apache::thrift::fast_thrift::channel_pipeline::Result onRead(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto& frame =
        msg.get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();
    uint32_t streamId = frame.streamId();

    if (frame.type() ==
        apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE) {
      auto [iter, inserted] = requestResponseStreams_.insert(streamId);
      auto result = ctx.fireRead(std::move(msg));
      // Only rollback on error; backpressure means the request was accepted.
      if (result ==
              apache::thrift::fast_thrift::channel_pipeline::Result::Error &&
          inserted) {
        requestResponseStreams_.erase(streamId);
      }
      return result;
    }

    if (frame.isTerminalFrame()) {
      requestResponseStreams_.erase(streamId);
    }

    return ctx.fireRead(std::move(msg));
  }

  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    requestResponseStreams_.clear();
    ctx.fireException(std::move(e));
  }

  // === OutboundHandler ===

  /**
   * Handle outbound RocketResponseMessage.
   *
   * For tracked RR streams: stamps `complete = true, next = true` on the
   * held ComposedPayloadFrame header (RR is single-shot terminal); ERROR
   * payloads are terminal by definition and need no stamping.
   * For other stream types: forwards unchanged. Codec serializes downstream.
   */
  template <typename Context>
  [[nodiscard]] apache::thrift::fast_thrift::channel_pipeline::Result onWrite(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto& response = msg.get<RocketResponseMessage>();

    uint32_t streamId = response.frame.streamId();

    auto it = requestResponseStreams_.find(streamId);
    if (it == requestResponseStreams_.end()) {
      return ctx.fireWrite(std::move(msg));
    }

    requestResponseStreams_.erase(it);

    // Stamp RR terminal flags on ComposedPayloadFrame (ERROR is implicitly
    // terminal — no stamping needed).
    if (response.frame
            .is<apache::thrift::fast_thrift::frame::ComposedPayloadFrame>()) {
      auto& payload =
          response.frame
              .get<apache::thrift::fast_thrift::frame::ComposedPayloadFrame>();
      payload.header.complete = true;
      payload.header.next = true;
    }

    auto result = ctx.fireWrite(std::move(msg));
    // Only re-add on error; backpressure means the write was accepted.
    if (result ==
        apache::thrift::fast_thrift::channel_pipeline::Result::Error) {
      requestResponseStreams_.insert(streamId);
    }
    return result;
  }

  template <typename Context>
  void onPipelineInactive(Context& /*ctx*/) noexcept {
    requestResponseStreams_.clear();
  }

  template <typename Context>
  void onWriteReady(Context& /*ctx*/) noexcept {}

  // === Accessors for testing ===

  size_t pendingRequestResponseCount() const noexcept {
    return requestResponseStreams_.size();
  }

  bool hasPendingRequestResponse(uint32_t streamId) const noexcept {
    return requestResponseStreams_.find(streamId) !=
        requestResponseStreams_.end();
  }

 private:
  // Set of stream IDs that are request-response interactions
  folly::F14FastSet<uint32_t> requestResponseStreams_;
};

} // namespace apache::thrift::fast_thrift::rocket::server::handler
