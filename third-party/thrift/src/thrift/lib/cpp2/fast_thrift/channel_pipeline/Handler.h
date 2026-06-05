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

#include <cstddef>

#include <folly/ExceptionWrapper.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Event.h>

namespace apache::thrift::fast_thrift::channel_pipeline {

/**
 * HandlerLifecycle concept - defines lifecycle callbacks for handlers.
 *
 * Handlers implement these callbacks to:
 * - Initialize and cache references to other handlers (handlerAdded)
 * - Clean up cached pointers before destruction (handlerRemoved)
 *
 * Guarantees:
 * - handlerAdded() is called in order handlers are added
 * - handlerRemoved() is called in REVERSE order (LIFO)
 * - Between these calls, ctx.pipeline() is guaranteed non-null
 */
template <typename H, typename Ctx>
concept HandlerLifecycle = requires(H h, Ctx& ctx) {
  // Called after handler is added to pipeline - safe to cache other contexts
  { h.handlerAdded(ctx) } noexcept -> std::same_as<void>;

  // Called before handler is removed (reverse order) - clear cached pointers
  { h.handlerRemoved(ctx) } noexcept -> std::same_as<void>;
};

/**
 * InboundHandler concept - handles data flowing from head to tail.
 *
 * Inbound handlers process:
 * - onRead: Incoming data/messages
 * - onReadReady: Notification that reads can resume
 * - onException: Notification of an exception
 * - onPipelineActive: Notification that pipeline is activated
 *
 * Handlers return Result to signal backpressure:
 * - Result::Success — continue processing
 * - Result::Backpressure — slow down, upstream should pause
 * - Result::Error — something went wrong
 *
 * Note: Read backpressure is handled at the transport level via TCP flow
 * control. When a handler returns Result::Backpressure from onRead, the
 * transport adapter should call pauseRead() on the socket. There is no
 * handler-to-handler read backpressure signaling.
 *
 * onException returns void because exceptions must always be
 * handled/propagated.
 */
template <typename H, typename Ctx>
concept InboundHandler =
    HandlerLifecycle<H, Ctx> &&
    requires(
        H h, Ctx& ctx, TypeErasedBox&& message, folly::exception_wrapper&& e) {
      // Returns Result to propagate backpressure
      { h.onRead(ctx, std::move(message)) } noexcept -> std::same_as<Result>;
      // Notifications — must always be handled
      { h.onReadReady(ctx) } noexcept -> std::same_as<void>;
      { h.onException(ctx, std::move(e)) } noexcept -> std::same_as<void>;
      { h.onPipelineActive(ctx) } noexcept -> std::same_as<void>;
    };

/**
 * OutboundHandler concept - handles data flowing from tail to head.
 *
 * Outbound handlers process:
 * - onWrite: Outgoing data/messages
 * - onWriteReady: Notification that more data can be written
 * - onPipelineInactive: Notification that pipeline is deactivated
 *
 * Handlers return Result to signal backpressure:
 * - Result::Success — continue processing
 * - Result::Backpressure — slow down, upstream should pause
 * - Result::Error — something went wrong
 */
template <typename H, typename Ctx>
concept OutboundHandler = HandlerLifecycle<H, Ctx> &&
    requires(H h, Ctx& ctx, TypeErasedBox&& message) {
      // Returns Result to propagate backpressure
      { h.onWrite(ctx, std::move(message)) } noexcept -> std::same_as<Result>;
      // Notifications — must always be handled
      { h.onWriteReady(ctx) } noexcept -> std::same_as<void>;
      { h.onPipelineInactive(ctx) } noexcept -> std::same_as<void>;
    };

/**
 * DuplexHandler concept - handles both inbound and outbound data.
 *
 * A handler that satisfies both InboundHandler and OutboundHandler.
 * handlerAdded() and handlerRemoved() are called once per handler instance.
 */
template <typename H, typename Ctx>
concept DuplexHandler = InboundHandler<H, Ctx> && OutboundHandler<H, Ctx>;

/**
 * EventSubscriber concept - an internal handler that registers for specific
 * user-event types.
 *
 * The handler declares which event types it cares about via a static
 * `kSubscribedEvents` (an array-like constexpr range of `E` values) and
 * implements `onEvent(ctx, E, const TypeErasedBox&)`. The framework links one
 * hook per subscribed event, so the handler is invoked only for those events.
 */
template <typename H, typename E, typename Ctx>
concept EventSubscriber =
    requires(H h, Ctx& ctx, E ev, const TypeErasedBox& evt) {
      { H::kSubscribedEvents.data() } -> std::convertible_to<const E*>;
      { H::kSubscribedEvents.size() } -> std::convertible_to<std::size_t>;
      { h.onEvent(ctx, ev, evt) } noexcept -> std::same_as<void>;
    };

/**
 * EndpointEventSubscriber concept - a head/tail endpoint that registers for
 * specific user-event types.
 *
 * Identical to EventSubscriber except endpoints receive no Context: they
 * implement `onEvent(E, const TypeErasedBox&)`.
 */
template <typename H, typename E>
concept EndpointEventSubscriber =
    requires(H h, E ev, const TypeErasedBox& evt) {
      { H::kSubscribedEvents.data() } -> std::convertible_to<const E*>;
      { H::kSubscribedEvents.size() } -> std::convertible_to<std::size_t>;
      { h.onEvent(ev, evt) } noexcept -> std::same_as<void>;
    };

} // namespace apache::thrift::fast_thrift::channel_pipeline
