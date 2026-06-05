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

#include <memory>

#include <folly/ExceptionWrapper.h>
#include <folly/Function.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/lang/Assume.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Messages.h>

namespace apache::thrift::fast_thrift::rocket::server {

/**
 * RocketServerAppAdapter — standalone rocket server endpoint adapter.
 *
 * Acts as the tail endpoint of a rocket-only server pipeline. Decouples the
 * rocket transport layer from any upper-layer protocol (Thrift, SR, etc.).
 *
 * Satisfies TailEndpointHandler (onRead/onException) and
 * RocketServerAppOutboundHandler (write).
 *
 * Inbound path (requests): The pipeline delivers messages to this adapter
 * via onRead(). The adapter forwards them to the registered onRequest
 * callback.
 *
 * Outbound path (responses): The upper layer calls write() to push
 * RocketResponseMessages into the pipeline.
 *
 * Usage:
 *   // 1. Create adapter
 *   RocketServerAppAdapter adapter;
 *   adapter.setOnRequest([&](TypeErasedBox&& msg) noexcept { ... });
 *   adapter.setOnError([&](exception_wrapper&& e) noexcept { ... });
 *
 *   // 2. Build rocket pipeline with adapter as tail
 *   auto pipeline = PipelineBuilder<
 *       TransportHandler, RocketServerAppAdapter, Allocator>()
 *       .setHead(transportHandler)
 *       .setTail(&adapter)
 *       ...
 *       .build();
 *
 *   // 3. Wire pipeline
 *   adapter.setPipeline(pipeline.get());
 *
 *   // 4. Send responses
 *   adapter.write(RocketResponseMessage{...});
 */
class RocketServerAppAdapter : public folly::DelayedDestruction {
 public:
  using Ptr = std::unique_ptr<RocketServerAppAdapter, Destructor>;

  using OnRequestFn = folly::Function<channel_pipeline::Result(
      channel_pipeline::TypeErasedBox&&) noexcept>;
  using OnErrorFn = folly::Function<void(folly::exception_wrapper&&) noexcept>;

  // Lifecycle callbacks — relay channel_pipeline lifecycle transitions to
  // the owner using rocket-domain terminology (connect/disconnect map to
  // the underlying onPipelineActive/onPipelineInactive). Distinct from
  // OnError: lifecycle events carry no exception, they just signal "the
  // connection became connected / disconnected".
  using OnConnectFn = folly::Function<void() noexcept>;
  using OnDisconnectFn = folly::Function<void() noexcept>;

  // Cross-pipeline write-ready relay: invoked when the rocket pipeline's
  // tail receives onWriteReady() from the transport. An upper pipeline
  // (e.g. thrift) installs this to be told its own pipeline can fire
  // onWriteReady().
  using OnWriteReadyFn = folly::Function<void() noexcept>;

  RocketServerAppAdapter() = default;

  RocketServerAppAdapter(const RocketServerAppAdapter&) = delete;
  RocketServerAppAdapter& operator=(const RocketServerAppAdapter&) = delete;
  RocketServerAppAdapter(RocketServerAppAdapter&&) = delete;
  RocketServerAppAdapter& operator=(RocketServerAppAdapter&&) = delete;

  void setPipeline(channel_pipeline::PipelineImpl* pipeline) noexcept {
    DCHECK(pipeline);
    if (pipeline_) {
      XLOG(FATAL) << "must reset pipeline before setting a new one";
    }
    pipeline_ = pipeline;
    pipelineGuard_ =
        std::make_unique<folly::DelayedDestruction::DestructorGuard>(pipeline);
  }

  /**
   * Release this adapter's hold on the pipeline. After this returns,
   * the pipeline may be destroyed (modulo other guards) and write() must
   * not be called.
   */
  void resetPipeline() noexcept {
    pipeline_ = nullptr;
    pipelineGuard_.reset();
  }

  // Test-only observer: returns the adapter's current pipeline pointer
  // (nullptr after resetPipeline). Used to verify teardown ordering
  // invariants.
  channel_pipeline::PipelineImpl* getPipeline() const noexcept {
    return pipeline_;
  }

  void setRequestHandlers(
      OnRequestFn reqHandler, OnErrorFn errHandler) noexcept {
    onRequest_ = std::move(reqHandler);
    onError_ = std::move(errHandler);
  }

  void setLifecycleHandlers(
      OnConnectFn connectHandler, OnDisconnectFn disconnectHandler) noexcept {
    onConnect_ = std::move(connectHandler);
    onDisconnect_ = std::move(disconnectHandler);
  }

  void setOnWriteReady(OnWriteReadyFn fn) noexcept {
    onWriteReady_ = std::move(fn);
  }

  /**
   * Fire onReadReady() on the rocket pipeline. The pipeline walks any
   * handlers awaiting read-ready notification and then signals the head
   * (TransportHandler::onReadReady → resumeRead). Used by an upper
   * pipeline (e.g. thrift) to release inbound backpressure that it had
   * earlier applied at its own tail.
   */
  void notifyReadReady() noexcept {
    if (pipeline_) {
      pipeline_->onReadReady();
    }
  }

  // === RocketServerAppOutboundHandler interface ===

  /**
   * Send a rocket response message into the pipeline (outbound path).
   * Precondition: setPipeline() has been called and resetPipeline() has
   * not. The DestructorGuard taken by setPipeline keeps the pipeline
   * alive across calls to write().
   */
  channel_pipeline::Result write(RocketResponseMessage&& msg) noexcept {
    return pipeline_->fireWrite(
        channel_pipeline::erase_and_box(std::move(msg)));
  }

  // === TailEndpointHandler interface ===

  /**
   * Called by the pipeline when a request message arrives (inbound path).
   * Forwards to the registered onRequest callback.
   */
  channel_pipeline::Result onRead(
      channel_pipeline::TypeErasedBox&& msg) noexcept {
    if (FOLLY_UNLIKELY(!onRequest_)) {
      return channel_pipeline::Result::Error;
    }
    return onRequest_(std::move(msg));
  }

  /**
   * Called when an exception propagates through the pipeline.
   * Forwards to the registered onError callback.
   */
  void onException(folly::exception_wrapper&& e) noexcept {
    if (FOLLY_UNLIKELY(!onError_)) {
      return;
    }
    onError_(std::move(e));
  }

  void handlerAdded() noexcept {}

  void handlerRemoved() noexcept {
    // Contract: the pipeline must deactivate before it closes, so by the
    // time we are removed the owner has already observed onDisconnect.
    DCHECK(disconnected_) << "pipeline closed without prior deactivate";
    // Drop callbacks so any post-detach use-after-detach (e.g. a queued
    // EventBase callback that still holds a stale captured `this`) sees
    // empty Functions and no-ops rather than firing into stale state.
    onRequest_ = {};
    onError_ = {};
    onConnect_ = {};
    onDisconnect_ = {};
    onWriteReady_ = {};
  }

  void onPipelineActive() noexcept {
    disconnected_ = false;
    if (onConnect_) {
      onConnect_();
    }
  }

  void onPipelineInactive() noexcept {
    // Idempotent: the transport's beginClose deactivates the pipeline, and
    // RocketServerConnection::destroy deactivates again to satisfy the
    // handlerRemoved contract when destroy is invoked without a prior
    // transport close. Either path may fire first; collapse the second.
    if (disconnected_) {
      return;
    }
    disconnected_ = true;
    if (onDisconnect_) {
      onDisconnect_();
    }
  }

  void onWriteReady() noexcept {
    if (onWriteReady_) {
      onWriteReady_();
    }
  }

 protected:
  ~RocketServerAppAdapter() override {
    DCHECK(!pipeline_);
    resetPipeline();
  }

 private:
  channel_pipeline::PipelineImpl* pipeline_{nullptr};
  std::unique_ptr<folly::DelayedDestruction::DestructorGuard> pipelineGuard_;
  OnRequestFn onRequest_;
  OnErrorFn onError_;
  OnConnectFn onConnect_;
  OnDisconnectFn onDisconnect_;
  OnWriteReadyFn onWriteReady_;
  bool disconnected_{true};
};

} // namespace apache::thrift::fast_thrift::rocket::server
