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
#include <folly/io/IOBuf.h>
#include <folly/lang/Hint.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Handler.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/DirectStreamMap.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Event.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>

#include <chrono>
#include <cstdint>
#include <memory>

namespace apache::thrift::fast_thrift::rocket::client::handler {

/**
 * RocketClientStatsHandler - Measures the transport-level per-request stats
 * observable at the rocket layer and stamps them onto the inbound response.
 *
 * Response-side wire sizes are read straight off the parsed frame. The
 * request-side sizes and both latencies cannot be: they are only knowable by
 * correlating the outbound write with the response that answers it, so the
 * handler keeps a per-stream slot from the write until the terminal frame.
 *
 * Installing this handler is the whole opt-in — a pipeline that leaves it out
 * does no correlation, keeps no map, and reads no clock. Nothing else in the
 * rocket pipeline measures on its behalf.
 *
 * Position: head-side of RocketClientStreamStateHandler. That is the only
 * placement that works in both directions, since the outbound walk runs
 * tail-to-head and the inbound walk head-to-tail:
 *   - outbound, StreamStateHandler has already assigned `frame.streamId`, so
 *     there is a key to file the request under.
 *   - inbound, wire frames arrive here before StreamStateHandler consumes the
 *     terminal one and drops its slot.
 *
 * In-process per-request errors and connection-level frames carry no wire
 * payload, so stats are left at their zero defaults.
 */
class RocketClientStatsHandler {
 public:
  RocketClientStatsHandler() = default;

  using EventId = client::RocketClientEventId;
  static constexpr apache::thrift::fast_thrift::channel_pipeline::Subscriptions<
      EventId::FrameWriteComplete>
      kSubscribedEvents{};

  // Steady, not system: these marks only ever produce durations, and a wall
  // clock stepping backwards would turn one into a negative latency.
  using Clock = std::chrono::steady_clock;

  // === HandlerLifecycle ===

  template <typename Context>
  void handlerAdded(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {
    pending_.clear();
  }

  // === InboundHandler ===

  template <typename Context>
  void onPipelineActive(Context& /*ctx*/) noexcept {
    pending_.clear();
  }

  template <typename Context>
  void onReadReady(Context& /*ctx*/) noexcept {}

  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onRead(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto& response = msg.get<RocketResponseMessage>();
    if (FOLLY_LIKELY(
            response.payload
                .is<apache::thrift::fast_thrift::frame::read::ParsedFrame>())) {
      const auto& parsed =
          response.payload
              .get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();
      response.stats.responseWireSizeBytes = parsed.dataSize();
      response.stats.responseMetadataAndPayloadSizeBytes = parsed.payloadSize();
      if (!parsed.isConnectionFrame() && parsed.isTerminalFrame()) {
        stampAndRelease(parsed.streamId(), response.stats);
      }
      return ctx.fireRead(std::move(msg));
    }

    // Per-request error raised below us (e.g. a codec serialize failure).
    // Always terminal for the stream, and the upper layers discard stats on
    // the error path, so drop the slot without stamping.
    pending_.erase(response.payload.get<RocketResponseError>().streamId);
    return ctx.fireRead(std::move(msg));
  }

  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    // Connection-fatal: StreamStateHandler fans its in-flight streams out as
    // synthesized errors fired toward the tail, which never reach this
    // handler. Dropping every slot here is what keeps the map from
    // outliving the requests it describes.
    pending_.clear();
    ctx.fireException(std::move(e));
  }

  // === OutboundHandler ===

  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onWrite(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto& request = msg.get<RocketRequestMessage>();
    const uint32_t streamId = request.frame.streamId;
    if (FOLLY_LIKELY(streamId != kInvalidStreamId)) {
      // Sizes have to be taken here, head-side of the marshal and codec
      // handlers that consume these buffers, and they deliberately exclude the
      // RSocket frame header so they mean the same thing as the legacy
      // channel's.
      const uint32_t dataSize = chainBytes(request.frame.data);
      const uint32_t metadataSize = chainBytes(request.frame.metadata);
      pending_.emplace(
          streamId,
          PendingRequestStats{
              .writeStart = Clock::now(),
              .writeComplete = {},
              .requestWireSizeBytes = dataSize,
              .requestMetadataAndPayloadSizeBytes = dataSize + metadataSize,
          });
    }

    auto result = ctx.fireWrite(std::move(msg));
    if (FOLLY_UNLIKELY(
            result ==
            apache::thrift::fast_thrift::channel_pipeline::Result::Error)) {
      // The request never reached the wire and StreamStateHandler is about to
      // roll its own slot back, so this one has nothing left to answer it.
      pending_.erase(streamId);
    }
    return result;
  }

  template <typename Context>
  void onPipelineInactive(Context& /*ctx*/) noexcept {
    pending_.clear();
  }

  template <typename Context>
  void onWriteReady(Context& /*ctx*/) noexcept {}

  // === EventSubscriber ===

  template <typename Context>
  void onEvent(
      Context& /*ctx*/,
      EventId /*ev*/,
      const apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&
          box) noexcept {
    const auto& event = box.get<client::FrameWriteCompleteEvent>();
    auto it = pending_.find(event.streamId);
    if (it == pending_.end()) {
      return;
    }
    // The request's bytes are gone as of now. Stamped even on a failed write:
    // a request that died on the socket still spent the write time it spent.
    it->second.writeComplete = Clock::now();
  }

  // === Accessors for testing ===

  size_t pendingCount() const noexcept { return pending_.size(); }

 private:
  struct PendingRequestStats {
    /// Set when the request passes through on its way out.
    Clock::time_point writeStart;
    /// Set when the transport reports the request's bytes are gone. Stays at
    /// the clock epoch on pipelines that never report write completions —
    /// that is the signal to report neither latency.
    Clock::time_point writeComplete;
    uint32_t requestWireSizeBytes{0};
    uint32_t requestMetadataAndPayloadSizeBytes{0};
  };

  static uint32_t chainBytes(
      const std::unique_ptr<folly::IOBuf>& buf) noexcept {
    return buf ? static_cast<uint32_t>(buf->computeChainDataLength()) : 0;
  }

  /**
   * Fill the request-side half of a response's stats from its slot and drop
   * the slot.
   *
   * Both latencies are measured from the write-completion mark, so a pipeline
   * whose event factory never fires write completions leaves them at zero —
   * the documented "not measured" value — rather than reporting a duration
   * measured from the clock's epoch.
   */
  void stampAndRelease(uint32_t streamId, RocketStats& stats) noexcept {
    auto it = pending_.find(streamId);
    if (it == pending_.end()) {
      return;
    }
    const auto& slot = it->second;
    stats.requestWireSizeBytes = slot.requestWireSizeBytes;
    stats.requestMetadataAndPayloadSizeBytes =
        slot.requestMetadataAndPayloadSizeBytes;
    if (slot.writeComplete.time_since_epoch().count() != 0) {
      stats.requestWriteLatency = slot.writeComplete - slot.writeStart;
      stats.responseRoundTripLatency = Clock::now() - slot.writeComplete;
    }
    pending_.erase(it);
  }

  apache::thrift::fast_thrift::frame::read::DirectStreamMap<PendingRequestStats>
      pending_;
};

static_assert(
    apache::thrift::fast_thrift::channel_pipeline::DuplexHandler<
        RocketClientStatsHandler,
        apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl>,
    "RocketClientStatsHandler must satisfy DuplexHandler concept");

static_assert(
    apache::thrift::fast_thrift::channel_pipeline::EventSubscriber<
        RocketClientStatsHandler,
        apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl>,
    "RocketClientStatsHandler must satisfy EventSubscriber concept");

} // namespace apache::thrift::fast_thrift::rocket::client::handler
