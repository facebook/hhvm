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

#include <cstdint>
#include <memory>

#include <folly/CPortability.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Event.h>

namespace apache::thrift::fast_thrift::channel_pipeline {

// Forward declarations
class PipelineImpl;

namespace detail {

/**
 * ContextImpl is the concrete implementation of the Context API.
 *
 * Each handler in the pipeline gets its own ContextImpl instance.
 * The context provides:
 * - Fire methods to propagate messages to next handlers
 * - Access to the owning pipeline
 * - Buffer allocation
 * - EventBase access
 *
 * ContextImpl satisfies the ContextApi concept defined in context.h.
 *
 * Contexts are strictly owned by PipelineImpl and share its lifetime.
 * They do not need independent delayed destruction - the pipeline's
 * DestructorGuard protects all contexts during callback execution.
 */
class ContextImpl {
 public:
  /**
   * Constructs a context for a handler at the given index.
   *
   * @param pipeline The owning pipeline
   * @param event_base The EventBase this pipeline runs on
   * @param allocator The buffer allocator to use
   * @param handler_index The index of this handler in the pipeline
   * @param handler_id The unique ID for this handler
   */
  ContextImpl(
      PipelineImpl* pipeline,
      folly::EventBase* eventBase,
      void* allocator,
      size_t handlerIndex,
      HandlerId handlerId) noexcept;

  // Non-copyable, move-only (required for vector<ContextImpl> storage)
  ContextImpl(const ContextImpl&) = delete;
  ContextImpl& operator=(const ContextImpl&) = delete;
  ~ContextImpl() = default;
  ContextImpl(ContextImpl&&) noexcept = default;
  ContextImpl& operator=(ContextImpl&&) noexcept = default;

  // === Identity ===

  /**
   * Returns the handler ID for this context.
   */
  HandlerId handlerId() const noexcept { return handlerId_; }

  // === Fire methods (propagate to next handler) ===

  /**
   * Propagate pipeline activation to the next inbound handler.
   * Direction: tail → head (increasing index)
   */
  void activate() noexcept;

  /**
   * Fire a read event to the next inbound handler.
   * Direction: tail → head (increasing index)
   *
   * Uses cached next-handler pointer — no PipelineImpl round-trip.
   *
   * @param msg The message to propagate
   * @return Result from the next handler (or head endpoint at head)
   */
  FOLLY_ALWAYS_INLINE Result fireRead(TypeErasedBox&& msg) noexcept {
    return nextReadFn_(nextHandler_, *nextCtx_, std::move(msg));
  }

  /**
   * Fire a write event to the next outbound handler.
   * Direction: head → tail (decreasing index)
   *
   * Uses cached prev-handler pointer — no PipelineImpl round-trip.
   *
   * @param msg The message to propagate
   * @return Result from the next handler (or tail endpoint at tail)
   */
  FOLLY_ALWAYS_INLINE Result fireWrite(TypeErasedBox&& msg) noexcept {
    return prevWriteFn_(prevHandler_, *prevCtx_, std::move(msg));
  }

  /**
   * Fire an exception event to the next inbound handler.
   * Direction: tail → head (increasing index)
   *
   * Uses cached next-handler pointer — no PipelineImpl round-trip.
   *
   * @param e The exception to propagate
   */
  FOLLY_ALWAYS_INLINE void fireException(
      folly::exception_wrapper&& e) noexcept {
    nextExceptionFn_(nextHandler_, *nextCtx_, std::move(e));
  }

  /**
   * Fire a user event of type `ev` carrying `eventMessage` to every handler
   * and endpoint subscribed to that event type — and only those. Subscribers
   * register the event types they care about (see EventSubscriber); a handler
   * subscribed to several types receives each via its own list. The firer is
   * itself invoked only if it subscribed to `ev`.
   *
   * The event box is passed to subscribers as a const ref; they inspect it via
   * `get<T>()`. Direction is not part of the event identity.
   *
   * Type-safe entry point: handlers pass the pipeline's event enum; the
   * non-templated forward (private) hands off to the owning pipeline.
   */
  template <EventEnum E>
  void fireEvent(E ev, TypeErasedBox&& eventMessage) noexcept {
    fireEvent(static_cast<std::uint32_t>(ev), std::move(eventMessage));
  }

  /**
   * Propagate pipeline deactivation to the next outbound handler.
   * Direction: head → tail (decreasing index)
   */
  void deactivate() noexcept;

  // === Pipeline access ===

  /**
   * Returns a pointer to the owning pipeline.
   * Guaranteed non-null between handler_added() and handler_removed().
   */
  PipelineImpl* pipeline() const noexcept { return pipeline_; }

  // === Buffer allocation ===

  /**
   * Allocate a buffer using the pipeline's allocator.
   *
   * @param size The size of the buffer to allocate
   * @return A unique pointer to the allocated IOBuf
   */
  BytesPtr allocate(size_t size) noexcept;

  /**
   * Copy data into a new buffer using the pipeline's allocator.
   */
  BytesPtr copyBuffer(const void* data, size_t size) noexcept;

  // === Event loop access ===

  /**
   * Returns the EventBase this pipeline runs on.
   */
  folly::EventBase* eventBase() const noexcept { return eventBase_; }

  // === Lifecycle ===

  /**
   * Close the pipeline.
   * Delegates to pipeline->close().
   */
  void close() noexcept;

  // === Backpressure / Ready Registration ===

  /**
   * Register this handler to receive onWriteReady() callback.
   *
   * Call this when returning Result::Backpressure from onWrite() to be
   * notified when the pipeline can accept writes again. The handler must
   * have a write_ready_hook_ member (detected at compile time).
   *
   * No-op if the handler doesn't have a write_ready_hook_ or is already
   * registered.
   */
  void awaitWriteReady() noexcept;

  /**
   * Cancel pending await for write ready.
   *
   * Call this after successfully processing buffered messages to unregister
   * from the write ready list.
   *
   * No-op if not currently awaiting.
   */
  void cancelAwaitWriteReady() noexcept;

  /**
   * Check if this handler is currently awaiting write ready.
   */
  bool isAwaitingWriteReady() const noexcept;

  /**
   * Register this handler to receive onReadReady() callback.
   *
   * Use this when an inbound handler needs to be notified to resume processing
   * after it returned Result::Backpressure from onRead().
   */
  void awaitReadReady() noexcept;

  /**
   * Cancel pending await for read ready.
   */
  void cancelAwaitReadReady() noexcept;

  /**
   * Check if this handler is currently awaiting read ready.
   */
  bool isAwaitingReadReady() const noexcept;

  // === Internal (used by PipelineImpl) ===

  /**
   * Returns the handler index for this context.
   */
  size_t handlerIndex() const noexcept { return handlerIndex_; }

 private:
  // Non-templated, out-of-line forward for fireEvent: the public typed overload
  // casts the event enum to its id and calls this, which hands off to the
  // owning pipeline. Out-of-line (defined in the .cpp) so this header need not
  // see PipelineImpl's definition — breaks the ContextImpl/PipelineImpl include
  // cycle. Private so callers use the type-safe enum API.
  void fireEvent(std::uint32_t ev, TypeErasedBox&& eventMessage) noexcept;

  PipelineImpl* pipeline_;
  folly::EventBase* eventBase_;
  void* allocator_; // Type-erased BufferAllocator
  size_t handlerIndex_;
  HandlerId handlerId_;

  // Cached next-handler dispatch for hot path.
  // Set once during PipelineImpl::initializeContexts().
  // Eliminates per-hop round-trip through PipelineImpl.

  // Read/exception direction: head→tail. Each context's "next" is the
  // handler closer to the tail.
  Result (*nextReadFn_)(void*, ContextImpl&, TypeErasedBox&&) noexcept {
      nullptr};
  void (*nextExceptionFn_)(
      void*, ContextImpl&, folly::exception_wrapper&&) noexcept {nullptr};
  void* nextHandler_{nullptr};
  ContextImpl* nextCtx_{nullptr};

  // Write direction: tail→head (prev handler or head terminal)
  Result (*prevWriteFn_)(void*, ContextImpl&, TypeErasedBox&&) noexcept {
      nullptr};
  void* prevHandler_{nullptr};
  ContextImpl* prevCtx_{nullptr};

  // Per-event subscription hooks — one per event type this handler subscribed
  // to, each linked into the matching per-event list in PipelineImpl. Kept
  // out-of-line (heap array) so the hot read/write dispatch fields above stay
  // compact; touched only on the cold event path. Empty when the handler
  // subscribes to nothing. Allocated and linked by PipelineImpl during build.
  std::unique_ptr<EventHook[]> eventHooks_;
  std::uint32_t eventHookCount_{0};

  friend class ::apache::thrift::fast_thrift::channel_pipeline::PipelineImpl;
};

} // namespace detail
} // namespace apache::thrift::fast_thrift::channel_pipeline
