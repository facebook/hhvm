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

  RocketClientAppAdapter() = default;

  RocketClientAppAdapter(const RocketClientAppAdapter&) = delete;
  RocketClientAppAdapter& operator=(const RocketClientAppAdapter&) = delete;
  RocketClientAppAdapter(RocketClientAppAdapter&&) = delete;
  RocketClientAppAdapter& operator=(RocketClientAppAdapter&&) = delete;

  void setPipeline(channel_pipeline::PipelineImpl* pipeline) noexcept {
    pipeline_ = pipeline;
  }

  void setResponseHandlers(
      OnResponseFn respHandler, OnErrorFn errHandler) noexcept {
    onResponse_ = std::move(respHandler);
    onError_ = std::move(errHandler);
  }

  // === RocketClientAppOutboundHandler interface ===

  /**
   * Write a rocket request message into the pipeline (outbound path).
   */
  channel_pipeline::Result write(RocketRequestMessage&& msg) noexcept {
    if (FOLLY_UNLIKELY(!pipeline_)) {
      return channel_pipeline::Result::Error;
    }
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
  void handlerRemoved() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}

 protected:
  ~RocketClientAppAdapter() override = default;

 private:
  channel_pipeline::PipelineImpl* pipeline_{nullptr};
  OnResponseFn onResponse_;
  OnErrorFn onError_;
};

} // namespace apache::thrift::fast_thrift::rocket::client
