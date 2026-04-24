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
#include <folly/Function.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/lang/Assume.h>
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

  RocketServerAppAdapter() = default;

  RocketServerAppAdapter(const RocketServerAppAdapter&) = delete;
  RocketServerAppAdapter& operator=(const RocketServerAppAdapter&) = delete;
  RocketServerAppAdapter(RocketServerAppAdapter&&) = delete;
  RocketServerAppAdapter& operator=(RocketServerAppAdapter&&) = delete;

  void setPipeline(channel_pipeline::PipelineImpl* pipeline) noexcept {
    pipeline_ = pipeline;
  }

  void setRequestHandlers(
      OnRequestFn reqHandler, OnErrorFn errHandler) noexcept {
    onRequest_ = std::move(reqHandler);
    onError_ = std::move(errHandler);
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
  void handlerRemoved() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}
  void onWriteReady() noexcept {}

 protected:
  ~RocketServerAppAdapter() override = default;

 private:
  channel_pipeline::PipelineImpl* pipeline_{nullptr};
  OnRequestFn onRequest_;
  OnErrorFn onError_;
};

} // namespace apache::thrift::fast_thrift::rocket::server
