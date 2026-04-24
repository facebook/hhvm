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
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>

#include <functional>

namespace apache::thrift::fast_thrift::channel_pipeline::test {

/**
 * MockHandler is a configurable handler for testing.
 *
 * It records all method calls and allows customization of behavior
 * via callbacks. By default, it passes through to the next handler.
 *
 * Includes writeReadyHook_ and readReadyHook_ for explicit ready signaling.
 * Hooks are automatically detected by makeHandlerNode.
 */
class MockHandler {
 public:
  // Hooks for explicit ready signaling (detected by makeHandlerNode)
  WriteReadyHook writeReadyHook_;
  ReadReadyHook readReadyHook_;

  // Recorded call types
  struct ReadCall {
    TypeErasedBox msg;
  };
  struct WriteCall {
    TypeErasedBox msg;
  };
  struct ExceptionCall {
    folly::exception_wrapper error;
  };

  // Callbacks for customizing behavior
  using OnReadCallback =
      std::function<Result(detail::ContextImpl&, TypeErasedBox&&)>;
  using OnWriteCallback =
      std::function<Result(detail::ContextImpl&, TypeErasedBox&&)>;
  using OnExceptionCallback =
      std::function<void(detail::ContextImpl&, folly::exception_wrapper&&)>;
  using LifecycleCallback = std::function<void(detail::ContextImpl&)>;

  MockHandler() = default;

  // === InboundHandler methods ===

  void onPipelineActivated(detail::ContextImpl& ctx) noexcept {
    pipelineActivatedCount_++;
    if (onPipelineActivatedCallback_) {
      onPipelineActivatedCallback_(ctx);
    }
  }

  Result onRead(detail::ContextImpl& ctx, TypeErasedBox&& msg) noexcept {
    readCount_++;
    if (onReadCallback_) {
      return onReadCallback_(ctx, std::move(msg));
    }
    return ctx.fireRead(std::move(msg));
  }

  void onException(
      detail::ContextImpl& ctx, folly::exception_wrapper&& e) noexcept {
    exceptionCount_++;
    if (onExceptionCallback_) {
      onExceptionCallback_(ctx, std::move(e));
    } else {
      ctx.fireException(std::move(e));
    }
  }

  // === OutboundHandler methods ===

  Result onWrite(detail::ContextImpl& ctx, TypeErasedBox&& msg) noexcept {
    writeCount_++;
    if (onWriteCallback_) {
      return onWriteCallback_(ctx, std::move(msg));
    }
    return ctx.fireWrite(std::move(msg));
  }

  void onWriteReady(detail::ContextImpl& ctx) noexcept {
    writeReadyCount_++;
    if (onWriteReadyCallback_) {
      onWriteReadyCallback_(ctx);
    }
  }

  void onReadReady(detail::ContextImpl& ctx) noexcept {
    readReadyCount_++;
    if (onReadReadyCallback_) {
      onReadReadyCallback_(ctx);
    }
  }

  void onPipelineDeactivated(detail::ContextImpl& ctx) noexcept {
    pipelineDeactivatedCount_++;
    if (onPipelineDeactivatedCallback_) {
      onPipelineDeactivatedCallback_(ctx);
    }
  }

  // === HandlerLifecycle methods ===

  void handlerAdded(detail::ContextImpl& ctx) noexcept {
    handlerAddedCount_++;
    handlerAddedOrder_ = nextHandlerOrder_++;
    if (handlerAddedCallback_) {
      handlerAddedCallback_(ctx);
    }
  }

  void handlerRemoved(detail::ContextImpl& ctx) noexcept {
    handlerRemovedCount_++;
    handlerRemovedOrder_ = nextHandlerOrder_++;
    if (handlerRemovedCallback_) {
      handlerRemovedCallback_(ctx);
    }
  }

  // === Configuration ===

  void setOnRead(OnReadCallback cb) { onReadCallback_ = std::move(cb); }
  void setOnPipelineActivated(LifecycleCallback cb) {
    onPipelineActivatedCallback_ = std::move(cb);
  }
  void setOnWrite(OnWriteCallback cb) { onWriteCallback_ = std::move(cb); }
  void setOnException(OnExceptionCallback cb) {
    onExceptionCallback_ = std::move(cb);
  }
  void setOnWriteReady(LifecycleCallback cb) {
    onWriteReadyCallback_ = std::move(cb);
  }
  void setOnReadReady(LifecycleCallback cb) {
    onReadReadyCallback_ = std::move(cb);
  }
  void setOnPipelineDeactivated(LifecycleCallback cb) {
    onPipelineDeactivatedCallback_ = std::move(cb);
  }
  void setHandlerAdded(LifecycleCallback cb) {
    handlerAddedCallback_ = std::move(cb);
  }
  void setHandlerRemoved(LifecycleCallback cb) {
    handlerRemovedCallback_ = std::move(cb);
  }

  // === Inspection ===

  int readCount() const { return readCount_; }
  int pipelineActivatedCount() const { return pipelineActivatedCount_; }
  int writeCount() const { return writeCount_; }
  int exceptionCount() const { return exceptionCount_; }
  int writeReadyCount() const { return writeReadyCount_; }
  int readReadyCount() const { return readReadyCount_; }
  int pipelineDeactivatedCount() const { return pipelineDeactivatedCount_; }
  int handlerAddedCount() const { return handlerAddedCount_; }
  int handlerRemovedCount() const { return handlerRemovedCount_; }
  int handlerAddedOrder() const { return handlerAddedOrder_; }
  int handlerRemovedOrder() const { return handlerRemovedOrder_; }

  // Reset global order counter (call between tests)
  static void resetOrderCounter() { nextHandlerOrder_ = 0; }

 private:
  int readCount_{0};
  int pipelineActivatedCount_{0};
  int writeCount_{0};
  int exceptionCount_{0};
  int writeReadyCount_{0};
  int readReadyCount_{0};
  int pipelineDeactivatedCount_{0};
  int handlerAddedCount_{0};
  int handlerRemovedCount_{0};
  int handlerAddedOrder_{-1};
  int handlerRemovedOrder_{-1};

  OnReadCallback onReadCallback_;
  OnWriteCallback onWriteCallback_;
  OnExceptionCallback onExceptionCallback_;
  LifecycleCallback onPipelineActivatedCallback_;
  LifecycleCallback onWriteReadyCallback_;
  LifecycleCallback onReadReadyCallback_;
  LifecycleCallback onPipelineDeactivatedCallback_;
  LifecycleCallback handlerAddedCallback_;
  LifecycleCallback handlerRemovedCallback_;

  static inline int nextHandlerOrder_{0};
};

/**
 * InboundOnlyHandler - handler that only implements inbound methods.
 * Used to test passthrough behavior for outbound.
 */
class InboundOnlyHandler {
 public:
  void onPipelineActivated(detail::ContextImpl& /*ctx*/) noexcept {
    pipelineActivatedCount_++;
  }

  Result onRead(detail::ContextImpl& ctx, TypeErasedBox&& msg) noexcept {
    readCount_++;
    return ctx.fireRead(std::move(msg));
  }

  void onReadReady(detail::ContextImpl&) noexcept {}

  void onException(
      detail::ContextImpl& ctx, folly::exception_wrapper&& e) noexcept {
    exceptionCount_++;
    ctx.fireException(std::move(e));
  }

  void handlerAdded(detail::ContextImpl&) noexcept { handlerAddedCount_++; }

  void handlerRemoved(detail::ContextImpl&) noexcept { handlerRemovedCount_++; }

  int readCount() const { return readCount_; }
  int pipelineActivatedCount() const { return pipelineActivatedCount_; }
  int exceptionCount() const { return exceptionCount_; }
  int handlerAddedCount() const { return handlerAddedCount_; }
  int handlerRemovedCount() const { return handlerRemovedCount_; }

 private:
  int readCount_{0};
  int pipelineActivatedCount_{0};
  int exceptionCount_{0};
  int handlerAddedCount_{0};
  int handlerRemovedCount_{0};
};

/**
 * OutboundOnlyHandler - handler that only implements outbound methods.
 * Used to test passthrough behavior for inbound.
 */
class OutboundOnlyHandler {
 public:
  Result onWrite(detail::ContextImpl& ctx, TypeErasedBox&& msg) noexcept {
    writeCount_++;
    return ctx.fireWrite(std::move(msg));
  }

  void onWriteReady(detail::ContextImpl&) noexcept { writeReadyCount_++; }

  void onPipelineDeactivated(detail::ContextImpl&) noexcept {
    pipelineDeactivatedCount_++;
  }

  void handlerAdded(detail::ContextImpl&) noexcept { handlerAddedCount_++; }

  void handlerRemoved(detail::ContextImpl&) noexcept { handlerRemovedCount_++; }

  int writeCount() const { return writeCount_; }
  int handlerAddedCount() const { return handlerAddedCount_; }
  int handlerRemovedCount() const { return handlerRemovedCount_; }

 private:
  int writeCount_{0};
  int writeReadyCount_{0};
  int pipelineDeactivatedCount_{0};
  int handlerAddedCount_{0};
  int handlerRemovedCount_{0};
};

} // namespace apache::thrift::fast_thrift::channel_pipeline::test
