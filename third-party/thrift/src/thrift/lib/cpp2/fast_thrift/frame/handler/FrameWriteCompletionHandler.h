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

#include <array>
#include <cstdint>
#include <deque>
#include <utility>

#include <folly/ExceptionWrapper.h>
#include <folly/container/F14Map.h>
#include <folly/logging/xlog.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/transport/WriteCompletion.h>

namespace apache::thrift::fast_thrift::frame::handler {

/**
 * FrameWriteCompletionHandlerT - resolves batch-level write completions into
 * per-frame write completions, keyed by streamId.
 *
 * A frame-layer duplex handler: outbound it sees each frame as a ComposedFrame,
 * inbound it sees responses as a ParsedFrame. It records the streamId of every
 * outbound frame in a FIFO and fires exactly one FrameWriteComplete{streamId}
 * per frame, which an upstream stream-state handler resolves to the request's
 * context.
 *
 * Delivery is exactly-once via two terminal paths:
 *  - onEvent(BatchWriteComplete{frameCount=N}): pop N streamIds in write order;
 *    fire FrameWriteComplete per still-live entry.
 *  - onRead: a terminal stream response proves the request reached the wire, so
 *    fire FrameWriteComplete{streamId, Success} early — before the stream-state
 *    handler erases the stream, so its streamId lookup still resolves — and
 *    tombstone the FIFO entry so the later batch completion skips it. This
 *    covers the race where the response beats the transport write-completion.
 *
 * onPipelineInactive drains every still-live entry as
 * FrameWriteComplete{Error}.
 *
 * Storage is a streamId FIFO (`order_`) plus a per-streamId tombstone map
 * (`live_`) for O(1) early-completion lookup on the inbound hot path. One
 * in-flight frame per streamId is assumed — true for REQUEST_RESPONSE.
 *
 * Templated on EventFactory, which keeps the pipeline's event enum and message
 * types out of this handler. It must provide:
 *   - using EventId = ...;
 *   - static constexpr EventId kBatchWriteCompleteEvent;
 *   - using BatchWriteCompleteEventType = ...;  // fields: status, frameCount
 *   - static std::pair<EventId, TypeErasedBox> makeFrameWriteComplete(
 *         transport::WriteCompletionStatus status, uint32_t streamId) noexcept;
 *
 * EB-thread only — no synchronization.
 */
template <typename EventFactory>
class FrameWriteCompletionHandlerT {
 public:
  FrameWriteCompletionHandlerT() = default;

  using EventId = typename EventFactory::EventId;
  static constexpr std::array<EventId, 1> kSubscribedEvents{
      EventFactory::kBatchWriteCompleteEvent};

  // === HandlerLifecycle ===

  template <typename Context>
  void handlerAdded(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {
    DCHECK(order_.empty()) << "handlerRemoved called with " << order_.size()
                           << " un-drained write-completion entries";
    order_.clear();
    live_.clear();
  }

  // === InboundHandler ===

  template <typename Context>
  void onPipelineActive(Context& /*ctx*/) noexcept {
    DCHECK(order_.empty()) << "onPipelineActive with " << order_.size()
                           << " leftover entries; previous close did not drain";
    // Defensively drop any leftover entries so a violated invariant cannot
    // fire stale completions against the new connection's streams in release.
    order_.clear();
    live_.clear();
  }

  template <typename Context>
  void onReadReady(Context& /*ctx*/) noexcept {}

  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onRead(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    const auto& parsed =
        msg.get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();
    // A terminal stream response means the request reached the wire. Complete
    // it now (before the stream-state handler erases the stream) and tombstone
    // the entry, so the later batch completion for the same frame is a no-op.
    if (!parsed.isConnectionFrame() && parsed.isTerminalFrame()) {
      completeEarly(ctx, parsed.streamId());
    }
    return ctx.fireRead(std::move(msg));
  }

  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }

  // === OutboundHandler ===

  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onWrite(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    // Read the streamId before fireWrite (the message is moved downstream), and
    // record it only on a non-Error result, so a synchronous downstream failure
    // creates no phantom entry.
    const uint32_t streamId =
        msg.get<apache::thrift::fast_thrift::frame::ComposedFrame>().streamId;
    auto result = ctx.fireWrite(std::move(msg));
    if (result !=
        apache::thrift::fast_thrift::channel_pipeline::Result::Error) {
      DCHECK(live_.find(streamId) == live_.end())
          << "streamId " << streamId
          << " already has a live entry; one in-flight frame per streamId "
             "assumed";
      order_.push_back(streamId);
      live_[streamId] = false;
    }
    return result;
  }

  template <typename Context>
  void onPipelineInactive(Context& ctx) noexcept {
    for (uint32_t streamId : order_) {
      auto it = live_.find(streamId);
      if (it != live_.end() && !it->second) {
        fireFrameWriteComplete(
            ctx,
            streamId,
            apache::thrift::fast_thrift::transport::WriteCompletionStatus::
                Error);
      }
    }
    order_.clear();
    live_.clear();
  }

  template <typename Context>
  void onWriteReady(Context& /*ctx*/) noexcept {}

  // === EventSubscriber ===

  template <typename Context>
  void onEvent(
      Context& ctx,
      EventId /*ev*/,
      const apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&
          box) noexcept {
    const auto& batch =
        box.get<typename EventFactory::BatchWriteCompleteEventType>();
    for (size_t i = 0; i < batch.frameCount; ++i) {
      if (FOLLY_UNLIKELY(order_.empty())) {
        XLOG(DFATAL) << "FrameWriteCompletionHandler FIFO exhausted before "
                        "frameCount";
        return;
      }
      const uint32_t streamId = order_.front();
      order_.pop_front();
      auto it = live_.find(streamId);
      const bool tombstoned = it != live_.end() && it->second;
      if (it != live_.end()) {
        live_.erase(it);
      }
      if (!tombstoned) {
        fireFrameWriteComplete(ctx, streamId, batch.status);
      }
    }
  }

  // === Accessors for testing ===

  size_t pendingCount() const noexcept { return order_.size(); }

 private:
  // Fire a stream's write-completion early and tombstone its FIFO entry. No-op
  // if the stream has no live entry (its batch completion already fired it).
  template <typename Context>
  void completeEarly(Context& ctx, uint32_t streamId) noexcept {
    auto it = live_.find(streamId);
    if (it == live_.end() || it->second) {
      return;
    }
    it->second = true;
    fireFrameWriteComplete(
        ctx,
        streamId,
        apache::thrift::fast_thrift::transport::WriteCompletionStatus::Success);
  }

  template <typename Context>
  void fireFrameWriteComplete(
      Context& ctx,
      uint32_t streamId,
      apache::thrift::fast_thrift::transport::WriteCompletionStatus
          status) noexcept {
    auto [eventId, eventMsg] =
        EventFactory::makeFrameWriteComplete(status, streamId);
    ctx.fireEvent(eventId, std::move(eventMsg));
  }

  // Outbound frames in write order. The companion map tracks, per still-live
  // streamId, whether it was already completed early via onRead.
  std::deque<uint32_t> order_;
  folly::F14FastMap<uint32_t, bool> live_;
};

} // namespace apache::thrift::fast_thrift::frame::handler
