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
  // connection became connected / disconnected / was closed".
  using OnConnectFn = folly::Function<void() noexcept>;
  using OnDisconnectFn = folly::Function<void() noexcept>;
  using OnCloseFn = folly::Function<void() noexcept>;

  RocketServerAppAdapter() = default;

  RocketServerAppAdapter(const RocketServerAppAdapter&) = delete;
  RocketServerAppAdapter& operator=(const RocketServerAppAdapter&) = delete;
  RocketServerAppAdapter(RocketServerAppAdapter&&) = delete;
  RocketServerAppAdapter& operator=(RocketServerAppAdapter&&) = delete;

  void setPipeline(channel_pipeline::PipelineImpl* pipeline) noexcept {
    pipeline_ = pipeline;
  }

  /**
   * Release this adapter's pointer to the pipeline. After this returns,
   * write() must not be called.
   *
   * Unlike the client-side adapter, this adapter does NOT hold a
   * DelayedDestruction guard on the pipeline. The server's existing
   * teardown paths (RocketServerConnection::close → destroy) explicitly
   * call pipeline->close() while both endpoints are still alive; test
   * fixtures rely on dropping the pipeline before the endpoints. Adding
   * a guard here would defer pipeline destruction past the transport
   * handler's death, producing UAFs when the deferred handlerRemoved
   * fan-out reached the freed transport.
   */
  void resetPipeline() noexcept { pipeline_ = nullptr; }

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
      OnConnectFn connectHandler,
      OnDisconnectFn disconnectHandler,
      OnCloseFn closeHandler) noexcept {
    onConnect_ = std::move(connectHandler);
    onDisconnect_ = std::move(disconnectHandler);
    onClose_ = std::move(closeHandler);
  }

  // === RocketServerAppOutboundHandler interface ===

  /**
   * Send a rocket response message into the pipeline (outbound path).
   */
  channel_pipeline::Result write(RocketResponseMessage&& msg) noexcept {
    if (FOLLY_UNLIKELY(!pipeline_)) {
      return channel_pipeline::Result::Error;
    }
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
    // Defensive: pipeline removed us without a prior deactivate. Fan out
    // onDisconnect_ so the owner observes the disconnect before the close.
    if (!disconnected_) {
      disconnected_ = true;
      if (onDisconnect_) {
        onDisconnect_();
      }
    }
    if (onClose_) {
      onClose_();
    }
    // Drop callbacks so any post-detach use-after-detach (e.g. a queued
    // EventBase callback that still holds a stale captured `this`) sees
    // empty Functions and no-ops rather than firing into stale state.
    onRequest_ = {};
    onError_ = {};
    onConnect_ = {};
    onDisconnect_ = {};
    onClose_ = {};
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

  void onWriteReady() noexcept {}

 protected:
  // Release the pipeline guard if a caller dropped the connection
  // without first running RocketServerConnection::destroy() (which would
  // have called resetPipeline). On the server, the ownership of the
  // connection is split between ConnectionHandler::connections_ and the
  // benchmark/test fixtures that emplace the connection directly, so the
  // strict client-side invariant of "pipeline_ is null at dtor" cannot
  // be enforced here.
  ~RocketServerAppAdapter() override { resetPipeline(); }

 private:
  channel_pipeline::PipelineImpl* pipeline_{nullptr};
  OnRequestFn onRequest_;
  OnErrorFn onError_;
  OnConnectFn onConnect_;
  OnDisconnectFn onDisconnect_;
  OnCloseFn onClose_;
  bool disconnected_{true};
};

} // namespace apache::thrift::fast_thrift::rocket::server
