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

#include <atomic>
#include <functional>
#include <memory>
#include <folly/ExceptionWrapper.h>
#include <folly/Synchronized.h>
#include <thrift/lib/cpp/protocol/TProtocolTypes.h>
#include <thrift/lib/cpp2/async/AsyncProcessor.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * ThriftServerChannel - Server-side app adapter backed by fast_thrift
 * pipeline.
 *
 * This adapter translates the fast_thrift channel pipeline to Thrift's
 * AsyncProcessor API. Supports request-response and oneway RPCs.
 *
 * This class implements the ServerInboundAppAdapter concept to receive
 * requests from the pipeline via onMessage().
 *
 * Usage:
 *   auto channel = std::make_unique<ThriftServerChannel>(processorFactory);
 *   // Build pipeline with channel as app handler
 *   channel->setPipeline(std::move(pipeline));
 */
class ThriftServerChannel {
 public:
  explicit ThriftServerChannel(
      std::shared_ptr<apache::thrift::AsyncProcessorFactory> processorFactory);
  ~ThriftServerChannel();

  ThriftServerChannel(const ThriftServerChannel&) = delete;
  ThriftServerChannel& operator=(const ThriftServerChannel&) = delete;
  ThriftServerChannel(ThriftServerChannel&&) = delete;
  ThriftServerChannel& operator=(ThriftServerChannel&&) = delete;

  // Set a callback invoked when the channel closes (connection error via
  // onException) or is destroyed. Used by FastThriftServer to remove the
  // channel from its tracking set.
  void setCloseCallback(std::function<void()> cb);

  // Set the pipeline (takes ownership) - must be called before processing
  // requests
  void setPipeline(
      apache::thrift::fast_thrift::channel_pipeline::PipelineImpl::Ptr
          pipeline);

  // Set the pipeline (non-owning) - for use when the pipeline is owned
  // externally (e.g., by ConnectionHandler)
  void setPipelineRef(
      apache::thrift::fast_thrift::channel_pipeline::PipelineImpl& pipeline);

  // Set the worker context for Cpp2ConnContext initialization.
  // Must be called before processing requests to provide IOWorkerContext.
  void setWorker(std::shared_ptr<apache::thrift::Cpp2Worker> worker);

  // === TailEndpointHandler lifecycle ===
  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}

  // === TailEndpointHandler interface ===
  // Called by the pipeline when a request message arrives
  apache::thrift::fast_thrift::channel_pipeline::Result onRead(
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept;

  // Called when an exception propagates through the pipeline
  void onException(folly::exception_wrapper&& e) noexcept;

 private:
  // Send an error response for a given stream
  void sendErrorResponse(
      uint32_t streamId,
      apache::thrift::protocol::PROTOCOL_TYPES protocolId,
      const std::string& errorMessage);

  std::shared_ptr<apache::thrift::AsyncProcessorFactory> processorFactory_;
  std::unique_ptr<apache::thrift::AsyncProcessor> processor_;
  apache::thrift::fast_thrift::channel_pipeline::PipelineImpl* pipeline_{
      nullptr};
  apache::thrift::fast_thrift::channel_pipeline::PipelineImpl::Ptr
      ownedPipeline_;
  std::shared_ptr<apache::thrift::Cpp2Worker> worker_;
  apache::thrift::Cpp2ConnContext connContext_;

  // Guard flag shared with in-flight PipelineResponseChannelRequests to
  // detect when the pipeline has been destroyed.
  std::shared_ptr<std::atomic<bool>> pipelineAlive_ =
      std::make_shared<std::atomic<bool>>(false);
  folly::Synchronized<std::function<void()>> closeCallback_;
};

} // namespace apache::thrift::fast_thrift::thrift
