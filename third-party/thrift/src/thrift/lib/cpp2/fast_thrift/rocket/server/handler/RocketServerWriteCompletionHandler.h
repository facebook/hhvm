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

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Event.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Event.h>

namespace apache::thrift::fast_thrift::rocket::server::handler {

/**
 * RocketServerWriteCompletionHandler — the frame/rocket seam on the write
 * completion path.
 *
 * The frame layer reports that the bytes for a streamId reached the socket;
 * this handler restates that as the rocket layer's own completion, so nothing
 * above the rocket boundary has to subscribe to a frame-layer event. It is the
 * server counterpart of the hop RocketClientStreamStateHandler performs on the
 * client, split out into its own handler because the server has no per-stream
 * request context to resolve against — the streamId is the whole identity.
 *
 * Carries no state: the FrameWriteComplete it consumes was already reassembled
 * from the batch by FragmentCompletionTracker, one event per original frame.
 *
 * The message path is pure pass-through in both directions; this handler exists
 * for the event path alone.
 *
 * EB-thread only — no synchronization.
 */
class RocketServerWriteCompletionHandler {
 public:
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
  [[nodiscard]] channel_pipeline::Result onRead(
      Context& ctx, channel_pipeline::TypeErasedBox&& msg) noexcept {
    return ctx.fireRead(std::move(msg));
  }

  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }

  // === OutboundHandler ===

  template <typename Context>
  [[nodiscard]] channel_pipeline::Result onWrite(
      Context& ctx, channel_pipeline::TypeErasedBox&& msg) noexcept {
    return ctx.fireWrite(std::move(msg));
  }

  template <typename Context>
  void onPipelineInactive(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onWriteReady(Context& /*ctx*/) noexcept {}

  // === EventSubscriber ===

  // Only the frame-layer event. The RocketWriteComplete this handler fires is a
  // different id, so its own output is never routed back to it.
  static constexpr channel_pipeline::Subscriptions<
      RocketServerEventId::FrameWriteComplete>
      kSubscribedEvents{};

  template <typename Context>
  void onEvent(
      Context& ctx,
      RocketServerEventId /*ev*/,
      const channel_pipeline::TypeErasedBox& box) noexcept {
    const auto& event = box.template get<FrameWriteCompleteEvent>();
    ctx.fireEvent(
        RocketServerEventId::RocketWriteComplete,
        channel_pipeline::TypeErasedBox(
            RocketWriteCompleteEvent{
                .streamId = event.streamId,
                .status = event.status,
                .quiesced = event.quiesced,
            }));
  }
};

} // namespace apache::thrift::fast_thrift::rocket::server::handler
