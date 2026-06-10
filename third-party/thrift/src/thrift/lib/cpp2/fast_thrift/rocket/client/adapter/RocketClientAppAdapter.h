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
#include <memory>

#include <folly/ExceptionWrapper.h>
#include <folly/Function.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/lang/Assume.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Event.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>

namespace apache::thrift::fast_thrift::rocket::client {

/**
 * RocketClientAppAdapter — standalone rocket client endpoint adapter.
 *
 * Acts as the head endpoint of a rocket-only client pipeline. Decouples the
 * rocket transport layer from any upper-layer protocol (Thrift, SR, etc.).
 *
 * Satisfies TailEndpointHandler (onRead/onException) and
 * RocketClientAppOutboundHandler (write).
 *
 * Inbound path (responses): The pipeline delivers messages to this adapter
 * via onRead(). The adapter forwards them to the registered onResponse
 * callback.
 *
 * Outbound path (requests): The upper layer calls write() to push
 * RocketRequestMessages into the pipeline.
 *
 * Usage:
 *   // 1. Create adapter
 *   RocketClientAppAdapter adapter;
 *   adapter.setOnResponse([&](TypeErasedBox&& msg) noexcept { ... });
 *   adapter.setOnError([&](exception_wrapper&& e) noexcept { ... });
 *
 *   // 2. Build rocket pipeline with adapter as tail
 *   auto pipeline = PipelineBuilder<
 *       TransportHandler, RocketClientAppAdapter, Allocator>()
 *       .setHead(transportHandler)
 *       .setTail(&adapter)
 *       ...
 *       .build();
 *
 *   // 3. Wire pipeline
 *   adapter.setPipeline(pipeline.get());
 *
 *   // 4. Send requests
 *   adapter.write(RocketRequestMessage{...});
 */
class RocketClientAppAdapter : public folly::DelayedDestruction {
 public:
  using Ptr = std::unique_ptr<RocketClientAppAdapter, Destructor>;

  using OnResponseFn = folly::Function<channel_pipeline::Result(
      channel_pipeline::TypeErasedBox&&) noexcept>;
  using OnErrorFn = folly::Function<void(folly::exception_wrapper&&) noexcept>;

  // Lifecycle callbacks — relay channel_pipeline lifecycle transitions to
  // the owner using rocket-domain terminology (connect/disconnect map to
  // the underlying onPipelineActive/onPipelineInactive). Distinct from
  // OnError: lifecycle events carry no exception, they just signal "the
  // connection became connected / disconnected".
  using OnConnectFn = folly::Function<void() noexcept>;
  using OnDisconnectFn = folly::Function<void() noexcept>;

  // Notification that the transport is about to close: the server sent
  // ERROR(CONNECTION_CLOSE) (graceful drain). NOT a lifecycle/destruction
  // edge — the connection is still live and in-flight work keeps flowing.
  // An upper pipeline (thrift) installs this to begin draining.
  using OnCloseFn = folly::Function<void() noexcept>;

  // Cross-pipeline write-ready relay: invoked when the rocket pipeline's
  // tail receives onWriteReady() from the transport. An upper pipeline
  // (e.g. thrift) installs this to be told its own pipeline can fire
  // onWriteReady().
  using OnWriteReadyFn = folly::Function<void() noexcept>;

  RocketClientAppAdapter() = default;

  RocketClientAppAdapter(const RocketClientAppAdapter&) = delete;
  RocketClientAppAdapter& operator=(const RocketClientAppAdapter&) = delete;
  RocketClientAppAdapter(RocketClientAppAdapter&&) = delete;
  RocketClientAppAdapter& operator=(RocketClientAppAdapter&&) = delete;

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

  void setResponseHandlers(
      OnResponseFn respHandler, OnErrorFn errHandler) noexcept {
    onResponse_ = std::move(respHandler);
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

  // Install the "transport about to close" (graceful-drain) notification.
  void setOnClose(OnCloseFn fn) noexcept { onClose_ = std::move(fn); }

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

  // === RocketClientAppOutboundHandler interface ===

  /**
   * Write a rocket request message into the pipeline (outbound path).
   */
  channel_pipeline::Result write(RocketRequestMessage&& msg) noexcept {
    DCHECK(pipeline_);
    return pipeline_->fireWrite(
        channel_pipeline::erase_and_box(std::move(msg)));
  }

  // === TailEndpointHandler interface ===

  /**
   * Called by the pipeline when a response message arrives (inbound path).
   * Forwards to the registered onResponse callback.
   */
  channel_pipeline::Result onRead(
      channel_pipeline::TypeErasedBox&& msg) noexcept {
    if (FOLLY_UNLIKELY(!onResponse_)) {
      return channel_pipeline::Result::Error;
    }
    return onResponse_(std::move(msg));
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
    DCHECK(disconnected_);
    // Defensive: pipeline removed us without a prior deactivate. Fan out
    // onDisconnect_ so the owner observes the disconnect.
    if (!disconnected_) {
      disconnected_ = true;
      if (onDisconnect_) {
        onDisconnect_();
      }
    }
    // Drop callbacks so any post-destroy use-after-detach (e.g. a queued
    // EventBase callback that still holds a stale captured `this`) sees
    // empty Functions and no-ops rather than firing into stale state.
    onResponse_ = {};
    onError_ = {};
    onConnect_ = {};
    onDisconnect_ = {};
    onClose_ = {};
    onWriteReady_ = {};
  }

  void onPipelineActive() noexcept {
    disconnected_ = false;
    if (onConnect_) {
      onConnect_();
    }
  }

  void onPipelineInactive() noexcept {
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

  // Subscribe only to the connection-close event; write-completion events on
  // the same pipeline go to other subscribers.
  static constexpr std::array<RocketClientEventId, 1> kSubscribedEvents{
      RocketClientEventId::ConnectionClose};

  // Relay a ConnectionClose event (server graceful drain) to the upper
  // pipeline via onClose — the transport is about to close.
  void onEvent(
      RocketClientEventId /*ev*/,
      const channel_pipeline::TypeErasedBox& evt) noexcept {
    if (evt.get<RocketClientEvent>().kind ==
            RocketClientEvent::Kind::ConnectionClose &&
        onClose_) {
      onClose_();
    }
  }

 protected:
  ~RocketClientAppAdapter() override {
    DCHECK(!pipeline_);
    resetPipeline();
  }

 private:
  channel_pipeline::PipelineImpl* pipeline_{nullptr};
  std::unique_ptr<folly::DelayedDestruction::DestructorGuard> pipelineGuard_;
  OnResponseFn onResponse_;
  OnErrorFn onError_;
  OnConnectFn onConnect_;
  OnDisconnectFn onDisconnect_;
  OnCloseFn onClose_;
  OnWriteReadyFn onWriteReady_;
  bool disconnected_{true};
};

} // namespace apache::thrift::fast_thrift::rocket::client
