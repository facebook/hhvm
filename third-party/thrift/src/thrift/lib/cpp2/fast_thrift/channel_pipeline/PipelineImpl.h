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

#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/DelayedDestructionBase.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Backpressure.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/EndpointAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/HandlerNode.h>

#include <folly/sorted_vector_types.h>

#include <memory>
#include <vector>

namespace apache::thrift::fast_thrift::channel_pipeline {

/**
 * PipelineImpl is the concrete implementation of the Pipeline concept.
 *
 * The pipeline owns all handlers and their contexts. It provides:
 * - Fire from head/tail: fireRead/fireWrite to start the chain
 * - Fire to specific handler: sendRead/sendWrite by handler ID
 * - Context lookup: context(id) to get a handler's context
 * - Head/Tail integration: Routes to head and tail endpoint adapters
 * - Backpressure tracking: Tracks which handlers have pending ready callbacks
 *
 * Handlers are stored in a contiguous array for cache friendliness.
 * Contexts are stored in parallel (one per handler).
 *
 * Inherits from DelayedDestruction for safe async cleanup. This ensures the
 * pipeline is not destroyed while handler callbacks are executing. All methods
 * that invoke callbacks use DestructorGuard to defer destruction.
 */
class PipelineImpl : public folly::DelayedDestruction {
 public:
  using Ptr = folly::DelayedDestructionUniquePtr<PipelineImpl>;

  /**
   * Creates a new PipelineImpl.
   * Use PipelineBuilder for construction.
   *
   * Direction is fixed: reads flow head→tail, writes flow tail→head.
   */
  PipelineImpl(
      folly::EventBase* event_base,
      std::vector<detail::HandlerNode> handlers,
      void* head,
      void* tail,
      void* allocator) noexcept;

  // Non-copyable
  PipelineImpl(const PipelineImpl&) = delete;
  PipelineImpl& operator=(const PipelineImpl&) = delete;
  PipelineImpl(PipelineImpl&&) = delete;
  PipelineImpl& operator=(PipelineImpl&&) = delete;

  // === Fire from head/tail ===

  /**
   * Activate the pipeline, notifying all handlers.
   *
   * Called by the endpoint adapter after the pipeline is fully wired up
   * and the connection is established.
   */
  void activate() noexcept;

  /**
   * Deactivate the pipeline, notifying all handlers.
   *
   * Called by the endpoint when the connection is lost.
   * Handlers should reset connection-specific state.
   */
  void deactivate() noexcept;

  /**
   * Fire a read event starting from the read-entry handler.
   */
  Result fireRead(TypeErasedBox&& msg) noexcept;

  /**
   * Fire a write event starting from the write-entry handler.
   */
  Result fireWrite(TypeErasedBox&& msg) noexcept;

  // === Fire to specific handler ===

  /**
   * Fire a read event to a specific handler by ID.
   */
  Result sendRead(HandlerId handlerId, TypeErasedBox&& msg) noexcept;

  /**
   * Fire a write event to a specific handler by ID.
   */
  Result sendWrite(HandlerId handlerId, TypeErasedBox&& msg) noexcept;

  /**
   * Fire an exception to a specific handler by ID.
   */
  void sendException(
      HandlerId handlerId, folly::exception_wrapper&& e) noexcept;

  // === Context lookup ===

  /**
   * Get a context by handler ID.
   * Returns nullptr if handler not found.
   */
  detail::ContextImpl* context(HandlerId handlerId) noexcept;

  // === Lifecycle ===

  /**
   * Close the pipeline.
   * Calls handlerRemoved in reverse order.
   */
  void close() noexcept;

  // === Internal methods (used by ContextImpl) ===

  /**
   * Fire read starting from a specific index.
   * Called by ContextImpl::fireRead().
   */
  Result fireReadFromIndex(size_t index, TypeErasedBox&& msg) noexcept;

  /**
   * Fire write starting from a specific index.
   * Called by ContextImpl::fireWrite().
   */
  Result fireWriteFromIndex(size_t index, TypeErasedBox&& msg) noexcept;

  /**
   * Fire exception starting from a specific index.
   * Called by ContextImpl::fireException().
   */
  void fireExceptionFromIndex(
      size_t index, folly::exception_wrapper&& e) noexcept;

  /**
   * Deactivate handlers starting from a specific index.
   * Called by ContextImpl::deactivate().
   */
  void deactivateFromIndex(size_t index) noexcept;

  /**
   * Fire read to the terminal endpoint.
   * Called when fireRead passes the last handler in the read chain.
   */
  Result fireReadToTerminal(TypeErasedBox&& msg) noexcept;

  /**
   * Fire write to the terminal endpoint.
   * Called when fireWrite passes the last handler in the write chain.
   */
  Result fireWriteToTerminal(TypeErasedBox&& msg) noexcept;

  /**
   * Fire exception to the terminal endpoint.
   * Called when fireException passes the last handler in the read chain.
   */
  void fireExceptionToTerminal(folly::exception_wrapper&& e) noexcept;

  /**
   * Allocate a buffer using the pipeline's allocator.
   */
  BytesPtr allocate(size_t size) noexcept;

  /**
   * Get the EventBase this pipeline runs on.
   */
  folly::EventBase* eventBase() const noexcept { return eventBase_; }

  /**
   * Get the number of handlers in the pipeline.
   */
  size_t handlerCount() const noexcept { return handlers_.size(); }

  // === Backpressure / Ready Notifications ===

  /**
   * Called by tail when it can accept more writes.
   * Propagates onWriteReady() to all handlers with pending write
   * backpressure.
   */
  void onWriteReady() noexcept;

  /**
   * Check if any handlers have pending write backpressure.
   */
  bool hasPendingWriteReady() const noexcept {
    return !writeReadyList_.empty();
  }

  /**
   * Called by head when it can accept more reads.
   * Propagates onReadReady() to all handlers with pending read
   * backpressure.
   */
  void onReadReady() noexcept;

  /**
   * Check if any handlers have pending read ready notifications.
   */
  bool hasPendingReadReady() const noexcept { return !readReadyList_.empty(); }

  // === Internal: Hook accessors for ContextImpl ===

  /**
   * Get the write ready hook for a handler by index.
   * Returns nullptr if handler doesn't have a hook.
   */
  WriteReadyHook* handlerWriteReadyHook(size_t index) noexcept {
    return handlers_[index].writeReadyHook_;
  }

  /**
   * Get the write ready list for registration.
   */
  WriteReadyList& writeReadyList() noexcept { return writeReadyList_; }

  /**
   * Get the read ready hook for a handler by index.
   * Returns nullptr if handler doesn't have a hook.
   */
  ReadReadyHook* handlerReadReadyHook(size_t index) noexcept {
    return handlers_[index].readReadyHook_;
  }

  /**
   * Get the read ready list for registration.
   */
  ReadReadyList& readReadyList() noexcept { return readReadyList_; }

 protected:
  // Protected destructor - must use destroy() or Ptr for cleanup.
  // Called via DelayedDestruction when guardCount reaches zero.
  ~PipelineImpl() override;

  // Override to ensure handlers are cleaned up before destruction.
  // Called by DelayedDestruction::destroy() when safe to destruct.
  // Note: We do NOT call callHandlerRemoved() here because it uses
  // DestructorGuard which would trigger infinite recursion.
  // Instead, close() is responsible for calling handlerRemoved.
  void onDelayedDestroy(bool delayed) override {
    if (delayed && !getDestroyPending()) {
      return;
    }
    // Call handlerRemoved directly without DestructorGuard to avoid recursion
    if (!closed_) {
      closed_ = true;
      callHandlerRemovedImpl();
    }
    delete this;
  }

 private:
  // O(log N) handler lookup using sorted_vector_map - cache-friendly for small
  // N
  size_t lookupHandler(HandlerId handlerId) const noexcept;

  folly::sorted_vector_map<HandlerId, size_t> handlerMap_;

  // Initialize contexts after construction
  void initializeContexts() noexcept;
  // Call handlerAdded for all handlers
  void callHandlerAdded() noexcept;
  // Call handlerRemoved for all handlers (reverse order)
  void callHandlerRemoved() noexcept;
  // Helper to call handlerRemoved without DestructorGuard.
  // Used by both callHandlerRemoved() and onDelayedDestroy().
  void callHandlerRemovedImpl() noexcept;

  folly::EventBase* eventBase_;
  std::vector<detail::HandlerNode> handlers_;
  std::vector<detail::ContextImpl> contexts_;

  // Type-erased endpoint adapters
  void* head_; // HeadHandler*
  void* tail_; // TailHandler*
  void* allocator_; // BufferAllocator*

  // Read-exit terminal

  // Read-exit terminal (whichever endpoint reads exit at)
  Result (*readTerminalOnMessageFn_)(void*, TypeErasedBox&&) noexcept {nullptr};
  void (*readTerminalOnExceptionFn_)(
      void*, folly::exception_wrapper&&) noexcept {nullptr};
  void* readTerminal_{nullptr};

  // Write-exit terminal (whichever endpoint writes exit at)
  Result (*writeTerminalOnMessageFn_)(void*, TypeErasedBox&&) noexcept {
      nullptr};
  void* writeTerminal_{nullptr};

  BytesPtr (*allocateFn_)(void*, size_t) noexcept {nullptr};

  // Endpoint lifecycle function pointers (new-style only)
  void (*headOnPipelineActiveFn_)(void*) noexcept {nullptr};
  void (*headOnPipelineInactiveFn_)(void*) noexcept {nullptr};
  void (*headHandlerRemovedFn_)(void*) noexcept {nullptr};
  void (*tailOnPipelineActiveFn_)(void*) noexcept {nullptr};
  void (*tailOnPipelineInactiveFn_)(void*) noexcept {nullptr};
  void (*tailHandlerRemovedFn_)(void*) noexcept {nullptr};

  // Cached entry-point dispatch for fireRead/fireWrite hot paths.
  // Points directly to the read-entry/write-entry handler's function pointer,
  // eliminating the vector index on the entry path.
  detail::HandlerNode::OnReadFn firstReadFn_{nullptr};
  detail::HandlerNode::OnExceptionFn firstExceptionFn_{nullptr};
  void* firstHandler_{nullptr};
  detail::ContextImpl* firstCtx_{nullptr};

  detail::HandlerNode::OnWriteFn lastWriteFn_{nullptr};
  void* lastHandler_{nullptr};
  detail::ContextImpl* lastCtx_{nullptr};

  bool closed_{false};

  // Intrusive lists tracking handlers awaiting write/read ready notifications.
  // Handlers self-register via ctx.awaitWriteReady()/ctx.awaitReadReady().
  WriteReadyList writeReadyList_;
  ReadReadyList readReadyList_;

  // PipelineBuilder needs access to set up the pipeline
  template <typename HeadHandler, typename TailHandler, typename Allocator>
  friend class PipelineBuilder;

  // ContextImpl needs access to ready lists for registration
  friend class detail::ContextImpl;
};

} // namespace apache::thrift::fast_thrift::channel_pipeline
