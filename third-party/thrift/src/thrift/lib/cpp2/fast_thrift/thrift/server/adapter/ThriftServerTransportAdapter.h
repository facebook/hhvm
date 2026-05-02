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
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/adapter/RocketServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>

namespace apache::thrift::fast_thrift::thrift::server {

/**
 * ThriftServerTransportAdapter — head endpoint of the thrift server pipeline.
 *
 * Bridges the thrift pipeline to the rocket pipeline by converting between
 * thrift and rocket message types. Holds a reference to the
 * RocketServerAppAdapter that sits at the tail of the rocket pipeline.
 *
 * Inbound path (requests):
 *   RocketServerAppAdapter delivers request → this.onTransportRequest()
 *   → converts RocketRequestMessage to ThriftServerRequestMessage
 *   → pipeline_->fireRead() → [thrift handlers] → ThriftServerChannel
 *
 * Outbound path (responses):
 *   ThriftServerChannel → [thrift handlers] → this.onWrite()
 *   → converts ThriftServerResponseMessage to RocketResponseMessage
 *   → RocketServerAppAdapter.write()
 *
 * Usage:
 *   // 1. Create rocket pipeline with app adapter as tail
 *   RocketServerAppAdapter appAdapter;
 *   auto rocketPipeline = PipelineBuilder<
 *       TransportHandler, RocketServerAppAdapter, Allocator>()
 *       .setHead(transportHandler)
 *       .setTail(&appAdapter)
 *       ...
 *       .build();
 *
 *   // 2. Create transport adapter referencing the app adapter
 *   ThriftServerTransportAdapter transportAdapter(appAdapter);
 *
 *   // 3. Build thrift pipeline with this adapter as head
 *   auto thriftPipeline = PipelineBuilder<
 *       ThriftServerTransportAdapter, ThriftServerChannel, Allocator>()
 *       .setHead(&transportAdapter)
 *       .setTail(serverChannel)
 *       ...
 *       .build();
 *
 *   // 4. Wire up
 *   transportAdapter.setPipeline(thriftPipeline.get());
 */
class ThriftServerTransportAdapter {
 public:
  explicit ThriftServerTransportAdapter(
      rocket::server::RocketServerAppAdapter& appAdapter)
      : appAdapter_(appAdapter) {
    appAdapter_.setRequestHandlers(
        [this](channel_pipeline::TypeErasedBox&& msg) noexcept {
          return onTransportRequest(std::move(msg));
        },
        [this](folly::exception_wrapper&& e) noexcept {
          onTransportError(std::move(e));
        });
  }

  ~ThriftServerTransportAdapter() = default;

  ThriftServerTransportAdapter(const ThriftServerTransportAdapter&) = delete;
  ThriftServerTransportAdapter& operator=(const ThriftServerTransportAdapter&) =
      delete;
  ThriftServerTransportAdapter(ThriftServerTransportAdapter&&) = delete;
  ThriftServerTransportAdapter& operator=(ThriftServerTransportAdapter&&) =
      delete;

  void setPipeline(channel_pipeline::PipelineImpl* pipeline) noexcept {
    pipeline_ = pipeline;
  }

  // === Transport-facing interface (inbound from rocket pipeline) ===

  /**
   * Called when the rocket pipeline delivers a request.
   *
   * Converts RocketRequestMessage to ThriftServerRequestMessage and pushes
   * it into the thrift pipeline via fireRead.
   */
  channel_pipeline::Result onTransportRequest(
      channel_pipeline::TypeErasedBox&& msg) noexcept {
    auto request = msg.take<rocket::server::RocketRequestMessage>();

    ThriftServerRequestMessage thriftMsg{
        .frame = std::move(request.frame),
        .streamId = request.streamId,
    };

    return pipeline_->fireRead(
        channel_pipeline::erase_and_box(std::move(thriftMsg)));
  }

  /**
   * Called when the rocket pipeline delivers an error.
   *
   * Propagates the error through the thrift pipeline via
   * fireExceptionFromIndex. Connection lifecycle is managed externally
   * (e.g., by ConnectionHandler or the test fixture).
   */
  void onTransportError(folly::exception_wrapper&& e) noexcept {
    if (pipeline_) {
      pipeline_->fireException(std::move(e));
    }
  }

  // === HeadEndpointHandler interface ===

  /**
   * Called when an outbound write reaches the head of the thrift pipeline.
   *
   * Converts ThriftServerResponseMessage to RocketResponseMessage and writes
   * it into the rocket pipeline.
   */
  channel_pipeline::Result onWrite(
      channel_pipeline::TypeErasedBox&& msg) noexcept {
    auto response = msg.take<ThriftServerResponseMessage>();

    rocket::server::RocketResponseMessage rocketMsg;
    // Only REQUEST_RESPONSE is wired today; when STREAM / CHANNEL / FNF land,
    // streamType must be plumbed through ThriftServerResponseMessage from the
    // inbound ThriftServerRequestMessage.
    rocketMsg.streamType =
        apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE;
    if (response.errorCode != 0) {
      rocketMsg.frame = apache::thrift::fast_thrift::frame::ComposedErrorFrame{
          .data = std::move(response.payload.data),
          .metadata = std::move(response.payload.metadata),
          .header =
              {.streamId = response.streamId, .errorCode = response.errorCode},
      };
    } else {
      rocketMsg.frame =
          apache::thrift::fast_thrift::frame::ComposedPayloadFrame{
              .data = std::move(response.payload.data),
              .metadata = std::move(response.payload.metadata),
              .header =
                  {.streamId = response.streamId,
                   .complete = response.payload.complete,
                   .next = true},
          };
    }

    return appAdapter_.write(std::move(rocketMsg));
  }

  /**
   * Called when an exception propagates through the thrift pipeline.
   *
   * Per-request thrift errors do NOT close the transport connection.
   */
  void onException(folly::exception_wrapper&& /*e*/) noexcept {}

  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}
  void onReadReady() noexcept {}

 private:
  channel_pipeline::PipelineImpl* pipeline_{nullptr};
  rocket::server::RocketServerAppAdapter& appAdapter_;
};

} // namespace apache::thrift::fast_thrift::thrift::server
