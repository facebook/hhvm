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
#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Event.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/common/RocketClientStreamContext.h>

#include <algorithm>
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
 * correlating the outbound write with the response that answers it, so they
 * are parked on the stream's entry in the shared RocketClientStreamContexts
 * map from the write until the terminal frame.
 *
 * The handler is stateless. It never inserts or erases an entry — it only
 * fills in the stats fields of entries RocketClientStreamStateHandler already
 * owns, which is what keeps a measured request from outliving the stream it
 * describes on any completion, error, or teardown path.
 *
 * Installing this handler is the opt-in for measurement: a pipeline that
 * leaves it out does no correlation and reads no clock, and every stats field
 * stays at its zero default. Nothing else in the rocket pipeline measures on
 * its behalf.
 *
 * Position: head-side of RocketClientStreamStateHandler. That is the only
 * placement that works in both directions, since the outbound walk runs
 * tail-to-head and the inbound walk head-to-tail:
 *   - outbound, StreamStateHandler has already assigned `frame.streamId` and
 *     opened the stream's entry, so there is somewhere to record the write.
 *   - inbound, wire frames arrive here before StreamStateHandler consumes the
 *     terminal one and erases that entry.
 *
 * In-process per-request errors and connection-level frames carry no wire
 * payload, so stats are left at their zero defaults.
 */
class RocketClientStatsHandler {
 public:
  RocketClientStatsHandler() = default;

  using EventId = client::RocketClientEventId;
  static constexpr apache::thrift::fast_thrift::channel_pipeline::
      Subscriptions<EventId::FrameWriteComplete, EventId::FirstResponseFrame>
          kSubscribedEvents{};

  // Steady, not system: these marks only ever produce durations, and a wall
  // clock stepping backwards would turn one into a negative latency.
  using Clock = PendingRequestStats::Clock;

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
    if (FOLLY_LIKELY(
            response.payload
                .is<apache::thrift::fast_thrift::frame::read::ParsedFrame>())) {
      const auto& parsed =
          response.payload
              .get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();
      response.stats.responseWireSizeBytes = parsed.dataSize();
      response.stats.responseMetadataAndPayloadSizeBytes = parsed.payloadSize();
      if (!parsed.isConnectionFrame() && parsed.isTerminalFrame()) {
        stamp(ctx, parsed.streamId(), response.stats);
      }
    }
    // The remaining alternative is a per-request error raised below us (e.g. a
    // codec serialize failure). It carries no wire payload and the upper
    // layers discard stats on the error path, so there is nothing to stamp;
    // StreamStateHandler erases the entry when it sees the same message.
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
    auto& request = msg.get<RocketRequestMessage>();
    auto& streams = ctx.template state<RocketClientStreamContexts>().streams;
    auto it = streams.find(request.frame.streamId);
    if (FOLLY_LIKELY(it != streams.end())) {
      // Sizes have to be taken here, head-side of the marshal and codec
      // handlers that consume these buffers, and they deliberately exclude the
      // RSocket frame header so they mean the same thing as the legacy
      // channel's.
      const uint32_t dataSize = chainBytes(request.frame.data);
      const uint32_t metadataSize = chainBytes(request.frame.metadata);
      it->second.stats = PendingRequestStats{
          .writeStart = Clock::now(),
          .writeComplete = {},
          .firstResponseFrame = {},
          .requestWireSizeBytes = dataSize,
          .requestMetadataAndPayloadSizeBytes = dataSize + metadataSize,
      };
    }
    return ctx.fireWrite(std::move(msg));
  }

  template <typename Context>
  void onPipelineInactive(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onWriteReady(Context& /*ctx*/) noexcept {}

  // === EventSubscriber ===

  template <typename Context>
  void onEvent(
      Context& ctx,
      EventId ev,
      const apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&
          box) noexcept {
    if (ev == EventId::FirstResponseFrame) {
      const auto& event = box.get<client::FirstResponseFrameEvent>();
      auto& streams = ctx.template state<RocketClientStreamContexts>().streams;
      if (auto it = streams.find(event.streamId); it != streams.end()) {
        it->second.stats.firstResponseFrame = event.arrivalTime;
      }
      return;
    }

    const auto& event = box.get<client::FrameWriteCompleteEvent>();
    auto& streams = ctx.template state<RocketClientStreamContexts>().streams;
    auto it = streams.find(event.streamId);
    if (it == streams.end()) {
      return;
    }
    // The request's bytes are gone as of now. Stamped even on a failed write:
    // a request that died on the socket still spent the write time it spent.
    it->second.stats.writeComplete = Clock::now();
  }

 private:
  static uint32_t chainBytes(
      const std::unique_ptr<folly::IOBuf>& buf) noexcept {
    return buf ? static_cast<uint32_t>(buf->computeChainDataLength()) : 0;
  }

  /**
   * Fill the request-side half of a response's stats from its stream's entry.
   *
   * Read-only: the entry belongs to StreamStateHandler, which erases it when
   * the same terminal frame reaches it a moment later.
   *
   * Every latency is measured from the write-completion mark, so a pipeline
   * whose event factory never fires write completions leaves them all at zero
   * — the documented "not measured" value — rather than reporting a duration
   * measured from the clock's epoch. This is what makes
   * `responseRoundTripLatency` the discriminator for the clamped zero below:
   * it is filled on exactly the responses that were measured at all.
   */
  template <typename Context>
  static void stamp(
      Context& ctx, uint32_t streamId, RocketStats& stats) noexcept {
    auto& streams = ctx.template state<RocketClientStreamContexts>().streams;
    auto it = streams.find(streamId);
    if (it == streams.end()) {
      return;
    }
    const auto& pending = it->second.stats;
    stats.requestWireSizeBytes = pending.requestWireSizeBytes;
    stats.requestMetadataAndPayloadSizeBytes =
        pending.requestMetadataAndPayloadSizeBytes;
    if (pending.writeComplete.time_since_epoch().count() != 0) {
      const auto now = Clock::now();
      stats.requestWriteLatency = pending.writeComplete - pending.writeStart;
      stats.responseRoundTripLatency = now - pending.writeComplete;
      // A whole response has no reported first-fragment time, and the frame in
      // hand is its first and only one — so `now` is the first-frame instant,
      // and the field equals the round trip. A fragmented response uses the
      // earlier stamp, making the difference between the two the reassembly
      // tail.
      const auto firstFrame =
          pending.firstResponseFrame.time_since_epoch().count() != 0
          ? pending.firstResponseFrame
          : now;
      // Clamp rather than report a negative interval: a response that beats
      // the write-completion event leaves firstFrame behind writeComplete.
      // Written unconditionally so the clamp yields a *measured* zero, which
      // `responseRoundTripLatency` being nonzero separates from the unmeasured
      // zero this whole branch is skipped for.
      stats.firstResponsePayloadFrameLatency =
          std::max<std::chrono::nanoseconds>(
              firstFrame - pending.writeComplete,
              std::chrono::nanoseconds::zero());
    }
  }
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
