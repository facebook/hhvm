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
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/common/RocketClientConnection.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/Messages.h>

namespace apache::thrift::fast_thrift::thrift::client {

/**
 * ThriftClientTransportAdapter — tail endpoint of the thrift client pipeline.
 *
 * Bridges the thrift pipeline to the rocket pipeline by converting between
 * thrift and rocket message types. Owns a RocketClientConnection
 * which holds the rocket pipeline resources.
 *
 * Outbound path (requests):
 *   ThriftClientAppAdapter.write() → [thrift handlers] → this.onWrite()
 *   → converts ThriftRequestMessage to RocketRequestMessage
 *   → RocketClientAppAdapter.write()
 *
 * Inbound path (responses):
 *   RocketClientAppAdapter delivers response → this.onTransportResponse()
 *   → converts RocketResponseMessage to ThriftResponseMessage
 *   → pipeline_->fireRead() → [thrift handlers] → ThriftClientAppAdapter
 *
 * Usage:
 *   // 1. Create connection (heap-allocated for pointer stability)
 *   auto connection = std::make_unique<RocketClientConnection>();
 *   // ... set up rocket pipeline using connection->appAdapter as head ...
 *
 *   // 2. Create adapter with connection (takes ownership)
 *   ThriftClientTransportAdapter transportAdapter(std::move(connection));
 *
 *   // 3. Build thrift pipeline with this adapter as head
 *   auto thriftPipeline = PipelineBuilder<
 *       ThriftClientTransportAdapter, ThriftClientAppAdapter, Allocator>()
 *       .setHead(&transportAdapter)
 *       .setTail(&appAdapter)
 *       ...
 *       .build();
 *
 *   // 4. Wire up
 *   transportAdapter.setPipeline(thriftPipeline.get());
 */
class ThriftClientTransportAdapter {
 public:
  explicit ThriftClientTransportAdapter(
      std::unique_ptr<rocket::client::RocketClientConnection> connection)
      : connection_(std::move(connection)) {
    connection_->appAdapter->setResponseHandlers(
        [this](channel_pipeline::TypeErasedBox&& msg) noexcept {
          return onTransportResponse(std::move(msg));
        },
        [this](folly::exception_wrapper&& e) noexcept {
          onTransportError(std::move(e));
        });
  }

  ~ThriftClientTransportAdapter() = default;

  ThriftClientTransportAdapter(const ThriftClientTransportAdapter&) = delete;
  ThriftClientTransportAdapter& operator=(const ThriftClientTransportAdapter&) =
      delete;
  ThriftClientTransportAdapter(ThriftClientTransportAdapter&&) = delete;
  ThriftClientTransportAdapter& operator=(ThriftClientTransportAdapter&&) =
      delete;

  void setPipeline(channel_pipeline::PipelineImpl* pipeline) noexcept {
    pipeline_ = pipeline;
  }

  // === Transport-facing interface (inbound from rocket pipeline) ===

  /**
   * Called when the rocket pipeline delivers a response.
   *
   * Converts RocketResponseMessage to ThriftResponseMessage and pushes
   * it into the thrift pipeline via fireRead.
   */
  channel_pipeline::Result onTransportResponse(
      channel_pipeline::TypeErasedBox&& msg) noexcept {
    auto response = msg.take<rocket::RocketResponseMessage>();

    // TODO: Fire request based errors via fireRead. onException is only for
    // connection level errors.
    if (FOLLY_UNLIKELY(response.payload.is<rocket::RocketResponseError>())) {
      pipeline_->fireException(
          std::move(response.payload.get<rocket::RocketResponseError>().ew));
      return channel_pipeline::Result::Success;
    }

    ThriftResponseMessage thriftMsg{
        .frame = std::move(response.payload.get<frame::read::ParsedFrame>()),
        .requestHandle = response.requestHandle,
        .streamType = response.streamType,
    };

    return pipeline_->fireRead(
        channel_pipeline::erase_and_box(std::move(thriftMsg)));
  }

  /**
   * Called when the rocket pipeline delivers an error response.
   * Propagates the error up the thrift pipeline and then closes the transport
   * connection.
   */
  void onTransportError(folly::exception_wrapper&& ew) noexcept {
    if (FOLLY_UNLIKELY(!pipeline_)) {
      return;
    }

    pipeline_->fireException(std::move(ew));
    connection_->close(
        folly::make_exception_wrapper<std::runtime_error>("Transport error"));
  }

  // === HeadEndpointHandler interface ===

  /**
   * Called when an outbound write reaches the tail of the thrift pipeline.
   *
   * Converts ThriftRequestMessage to RocketRequestMessage and writes it
   * into the rocket pipeline.
   *
   * Today only the request-response RPC pattern is wired through the
   * client; `ThriftRequestMessage::payload` is a single-alternative
   * variant of `ThriftRequestResponsePayload`. As other RpcKinds are
   * added to the variant, this method gains dispatch over the held
   * alternative — by then the rocket layer's variant is also expected to
   * widen so that `payload.toRocketFrame()` can plug in directly.
   */
  channel_pipeline::Result onWrite(
      channel_pipeline::TypeErasedBox&& msg) noexcept {
    auto request = msg.take<ThriftRequestMessage>();
    rocket::RocketRequestMessage rocketMsg{
        .frame = std::move(request.payload.get<ThriftRequestResponsePayload>())
                     .toRocketFrame(),
        .requestHandle = request.requestHandle,
        .streamType = frame::FrameType::REQUEST_RESPONSE,
    };
    return connection_->appAdapter->write(std::move(rocketMsg));
  }

  void onReadReady() noexcept {}

  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}

 private:
  channel_pipeline::PipelineImpl* pipeline_{nullptr};
  std::unique_ptr<rocket::client::RocketClientConnection> connection_;
};

} // namespace apache::thrift::fast_thrift::thrift::client
