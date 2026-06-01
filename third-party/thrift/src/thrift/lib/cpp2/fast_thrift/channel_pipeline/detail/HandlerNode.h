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
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Backpressure.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Handler.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>

#include <memory>

namespace apache::thrift::fast_thrift::channel_pipeline::detail {

/**
 * HandlerNode stores a type-erased handler with function pointers for dispatch.
 *
 * This implements the External Polymorphism pattern:
 * - Handlers are plain classes (no virtual methods required)
 * - Function pointers are generated at compile-time when the handler is added
 * - Dispatch is a single function pointer call (no vtable lookup)
 *
 * Storage uses two pointers:
 * - `handler_ptr_`: Raw pointer for fast function calls
 * - `owner_`: Owning unique_ptr with type-erased deleter for destruction
 */
struct HandlerNode {
  // Handler identification
  HandlerId handlerId{0};

  // Raw pointer for fast dispatch (no indirection through unique_ptr)
  void* handlerPtr{nullptr};

  // Owning pointer with type-erased deleter
  std::unique_ptr<void, void (*)(void*)> owner{nullptr, nullptr};

  // Function pointer types
  using OnReadFn = Result (*)(void*, ContextImpl&, TypeErasedBox&&) noexcept;
  using OnWriteFn = Result (*)(void*, ContextImpl&, TypeErasedBox&&) noexcept;
  using OnExceptionFn =
      void (*)(void*, ContextImpl&, folly::exception_wrapper&&) noexcept;
  using OnWriteReadyFn = void (*)(void*, ContextImpl&) noexcept;
  using OnPipelineDeactivatedFn = void (*)(void*, ContextImpl&) noexcept;
  using OnPipelineActivatedFn = void (*)(void*, ContextImpl&) noexcept;
  using HandlerAddedFn = void (*)(void*, ContextImpl&) noexcept;
  using HandlerRemovedFn = void (*)(void*, ContextImpl&) noexcept;

  // Inbound handler methods
  OnPipelineActivatedFn onPipelineActivatedFn{nullptr};
  OnReadFn onReadFn{nullptr};
  OnExceptionFn onExceptionFn{nullptr};

  // Outbound handler methods
  OnWriteFn onWriteFn{nullptr};
  OnWriteReadyFn onWriteReadyFn{nullptr};
  OnPipelineDeactivatedFn onPipelineDeactivatedFn{nullptr};

  // Lifecycle methods
  HandlerAddedFn handlerAddedFn{nullptr};
  HandlerRemovedFn handlerRemovedFn{nullptr};

  // Hook pointer for write backpressure registration (nullptr if handler
  // doesn't have hook). Detected at compile time by makeHandlerNode. Note: Read
  // backpressure is handled at transport level via TCP flow control.
  WriteReadyHook* writeReadyHook_{nullptr};

  // Move-only with explicit move constructor to nullify moved-from object
  HandlerNode() = default;
  ~HandlerNode() = default;

  HandlerNode(HandlerNode&& other) noexcept
      : handlerId(other.handlerId),
        handlerPtr(other.handlerPtr),
        owner(std::move(other.owner)),
        onPipelineActivatedFn(other.onPipelineActivatedFn),
        onReadFn(other.onReadFn),
        onExceptionFn(other.onExceptionFn),
        onWriteFn(other.onWriteFn),
        onWriteReadyFn(other.onWriteReadyFn),
        onPipelineDeactivatedFn(other.onPipelineDeactivatedFn),
        handlerAddedFn(other.handlerAddedFn),
        handlerRemovedFn(other.handlerRemovedFn),
        writeReadyHook_(other.writeReadyHook_) {
    other.handlerId = 0;
    other.handlerPtr = nullptr;
    other.onPipelineActivatedFn = nullptr;
    other.onReadFn = nullptr;
    other.onExceptionFn = nullptr;
    other.onWriteFn = nullptr;
    other.onWriteReadyFn = nullptr;
    other.onPipelineDeactivatedFn = nullptr;
    other.handlerAddedFn = nullptr;
    other.handlerRemovedFn = nullptr;
    other.writeReadyHook_ = nullptr;
  }

  HandlerNode& operator=(HandlerNode&& other) noexcept {
    if (this != &other) {
      handlerId = other.handlerId;
      handlerPtr = other.handlerPtr;
      owner = std::move(other.owner);
      onPipelineActivatedFn = other.onPipelineActivatedFn;
      onReadFn = other.onReadFn;
      onExceptionFn = other.onExceptionFn;
      onWriteFn = other.onWriteFn;
      onWriteReadyFn = other.onWriteReadyFn;
      onPipelineDeactivatedFn = other.onPipelineDeactivatedFn;
      handlerAddedFn = other.handlerAddedFn;
      handlerRemovedFn = other.handlerRemovedFn;
      writeReadyHook_ = other.writeReadyHook_;

      other.handlerId = 0;
      other.handlerPtr = nullptr;
      other.onPipelineActivatedFn = nullptr;
      other.onReadFn = nullptr;
      other.onExceptionFn = nullptr;
      other.onWriteFn = nullptr;
      other.onWriteReadyFn = nullptr;
      other.onPipelineDeactivatedFn = nullptr;
      other.handlerAddedFn = nullptr;
      other.handlerRemovedFn = nullptr;
      other.writeReadyHook_ = nullptr;
    }
    return *this;
  }

  HandlerNode(const HandlerNode&) = delete;
  HandlerNode& operator=(const HandlerNode&) = delete;
};

/**
 * Creates a HandlerNode from a concrete handler type.
 *
 * This function template captures the concrete handler type at compile time
 * and generates appropriate function pointers. For handlers that don't
 * satisfy InboundHandler or OutboundHandler concepts, the corresponding
 * methods become passthroughs.
 *
 * If the handler has read_ready_hook_ or write_ready_hook_ members, their
 * addresses are captured for use by the await_read_ready() /
 * await_write_ready() context methods.
 *
 * @tparam H The concrete handler type
 * @param handler_id The unique ID for this handler
 * @param handler Unique pointer to the handler instance
 * @return A fully initialized HandlerNode
 */
template <typename H>
HandlerNode makeHandlerNode(HandlerId handlerId, std::unique_ptr<H> handler) {
  HandlerNode node;
  node.handlerId = handlerId;
  node.handlerPtr = handler.get();
  node.owner = std::unique_ptr<void, void (*)(void*)>(
      handler.release(), [](void* p) { delete static_cast<H*>(p); });

  // Capture write hook pointer at compile time if handler has it
  if constexpr (requires(H& h) { h.writeReadyHook_; }) {
    node.writeReadyHook_ = &static_cast<H*>(node.handlerPtr)->writeReadyHook_;
  }

  // Lifecycle methods - always present (required by HandlerLifecycle concept)
  node.handlerAddedFn = [](void* h, ContextImpl& ctx) noexcept {
    static_cast<H*>(h)->handlerAdded(ctx);
  };

  node.handlerRemovedFn = [](void* h, ContextImpl& ctx) noexcept {
    static_cast<H*>(h)->handlerRemoved(ctx);
  };

  // Inbound methods - check if handler satisfies InboundHandler
  if constexpr (InboundHandler<H, ContextImpl>) {
    node.onPipelineActivatedFn = [](void* h, ContextImpl& ctx) noexcept {
      static_cast<H*>(h)->onPipelineActivated(ctx);
    };

    node.onReadFn =
        [](void* h, ContextImpl& ctx, TypeErasedBox&& msg) noexcept -> Result {
      return static_cast<H*>(h)->onRead(ctx, std::move(msg));
    };

    node.onExceptionFn =
        [](void* h, ContextImpl& ctx, folly::exception_wrapper&& e) noexcept {
          static_cast<H*>(h)->onException(ctx, std::move(e));
        };
  } else {
    // Passthrough for non-inbound handlers
    node.onPipelineActivatedFn = [](void*, ContextImpl&) noexcept {
      // No-op for non-inbound handlers (activation already propagates to all)
    };

    node.onReadFn =
        [](void*, ContextImpl& ctx, TypeErasedBox&& msg) noexcept -> Result {
      return ctx.fireRead(std::move(msg));
    };

    node.onExceptionFn =
        [](void*, ContextImpl& ctx, folly::exception_wrapper&& e) noexcept {
          ctx.fireException(std::move(e));
        };
  }

  // Outbound methods - check if handler satisfies OutboundHandler
  if constexpr (OutboundHandler<H, ContextImpl>) {
    node.onWriteFn =
        [](void* h, ContextImpl& ctx, TypeErasedBox&& msg) noexcept -> Result {
      return static_cast<H*>(h)->onWrite(ctx, std::move(msg));
    };

    node.onWriteReadyFn = [](void* h, ContextImpl& ctx) noexcept {
      static_cast<H*>(h)->onWriteReady(ctx);
    };

    node.onPipelineDeactivatedFn = [](void* h, ContextImpl& ctx) noexcept {
      static_cast<H*>(h)->onPipelineDeactivated(ctx);
    };
  } else {
    // Passthrough for non-outbound handlers
    node.onWriteFn =
        [](void*, ContextImpl& ctx, TypeErasedBox&& msg) noexcept -> Result {
      return ctx.fireWrite(std::move(msg));
    };

    node.onWriteReadyFn = [](void*, ContextImpl&) noexcept {
      // No-op for non-outbound handlers
    };

    node.onPipelineDeactivatedFn = [](void*, ContextImpl&) noexcept {
      // No-op for non-outbound handlers
    };
  }

  return node;
}

} // namespace apache::thrift::fast_thrift::channel_pipeline::detail
