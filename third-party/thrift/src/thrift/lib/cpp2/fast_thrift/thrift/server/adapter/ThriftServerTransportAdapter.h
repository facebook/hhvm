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
#include <folly/Portability.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/MetadataProtocol.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/adapter/RocketServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/common/RocketServerConnection.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Event.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/PayloadVariants.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/RocketFrameDecoder.h>

namespace apache::thrift::fast_thrift::thrift::server {

/**
 * ThriftServerTransportAdapter — head endpoint of the thrift server pipeline.
 *
 * Owns a RocketServerConnection (transport handler + rocket pipeline + rocket
 * app adapter) and bridges the thrift pipeline to it. Converts between thrift
 * and rocket message types on the per-request path and propagates lifecycle
 * events between the two pipelines.
 *
 * Inbound path (requests):
 *   rocket app adapter delivers request → onTransportRequest()
 *   → converts RocketRequestMessage to ThriftServerRequestMessage
 *   → pipeline_->fireRead() → [thrift handlers] → tail adapter
 *
 * Outbound path (responses):
 *   tail adapter → [thrift handlers] → onWrite()
 *   → converts ThriftServerResponseMessage to RocketResponseMessage
 *   → rocketConn_->appAdapter->write()
 *
 * Lifecycle propagation:
 *   rocket active   → onConnect()         → thrift pipeline activate
 *   rocket inactive → onDisconnect()      → thrift pipeline deactivate
 *   thrift inactive → onPipelineInactive() → rocketConn_->disconnect()
 *   thrift removed  → handlerRemoved()    → rocketConn_->destroy()
 *
 * The connected_ flag absorbs the reentry that arises when one direction
 * triggers the other (rocket inactive deactivates thrift, which would
 * otherwise re-enter onPipelineInactive and disconnect again).
 *
 * Hot/cold split: per-request methods (onTransportRequest, onWrite,
 * onTransportError) are inlined here so callers can fuse them with the
 * surrounding pipeline traversal. Per-connection lifecycle methods (ctor,
 * dtor, handlerRemoved, onPipelineInactive, onConnect, onDisconnect) and
 * the per-request error fallback (handleDecodeFailure) are declared
 * FOLLY_NOINLINE and defined out-of-line in the .cpp — they fire at most
 * once per connection (or per decode failure), so keeping them off the
 * inline path shrinks the hot path's icache footprint.
 */
class ThriftServerTransportAdapter {
 public:
  // Takes ownership of the rocket connection. The rocket connection is
  // torn down (disconnect + destroy) when the thrift pipeline tears down
  // this bridge, via handlerRemoved / onPipelineInactive.
  FOLLY_NOINLINE explicit ThriftServerTransportAdapter(
      std::unique_ptr<rocket::server::RocketServerConnection> rocketConn);

  FOLLY_NOINLINE ~ThriftServerTransportAdapter();

  ThriftServerTransportAdapter(const ThriftServerTransportAdapter&) = delete;
  ThriftServerTransportAdapter& operator=(const ThriftServerTransportAdapter&) =
      delete;
  ThriftServerTransportAdapter(ThriftServerTransportAdapter&&) = delete;
  ThriftServerTransportAdapter& operator=(ThriftServerTransportAdapter&&) =
      delete;

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
   * Release this bridge's hold on the thrift pipeline. Called from
   * handlerRemoved on the normal teardown path; also from the dtor as a
   * defensive fallback.
   */
  void resetPipeline() noexcept {
    pipeline_ = nullptr;
    pipelineGuard_.reset();
  }

  // Set the metadata protocol (Binary or Compact) used to deserialize
  // inbound request metadata.
  void setMetadataProtocol(rocket::server::MetadataProtocol p) noexcept {
    metadataProtocol_ = p;
  }

  rocket::server::RocketServerConnection& rocketConnection() noexcept {
    return *rocketConn_;
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

    auto decoded = fromRocketFrame(std::move(request.frame), metadataProtocol_);

    if (FOLLY_UNLIKELY(!decoded.hasValue())) {
      return handleDecodeFailure(
          request.streamId, request.streamType, decoded.error());
    }

    ThriftServerRequestMessage thriftMsg;
    thriftMsg.streamId = request.streamId;
    thriftMsg.payload = std::move(decoded.value());
    return pipeline_->fireRead(
        channel_pipeline::erase_and_box(std::move(thriftMsg)));
  }

  /**
   * Called when the rocket pipeline delivers an error. Propagates the
   * error up the thrift pipeline. The disconnect happens via the
   * lifecycle path (onDisconnect callback / onPipelineInactive), not from
   * here. Precondition: pipeline is wired; the DestructorGuard taken by
   * setPipeline keeps it alive.
   */
  void onTransportError(folly::exception_wrapper&& e) noexcept {
    pipeline_->fireException(std::move(e));
  }

  // Called when the rocket pipeline reports a completed write batch. Relays it
  // up the thrift pipeline as a ThriftServerEventType::WriteComplete event
  // carrying the batch's status / frame count / bytes. Precondition: pipeline
  // is wired (the rocket connection that fires this is torn down before
  // pipeline_ is cleared).
  void onWriteComplete(
      const rocket::server::RocketWriteCompleteEvent& event) noexcept {
    pipeline_->fireEvent(
        ThriftServerEventType::WriteComplete,
        channel_pipeline::TypeErasedBox(
            ThriftServerWriteCompleteEvent{
                .status = event.status,
                .frameCount = event.frameCount,
                .bytes = event.bytes,
            }));
  }

  // === HeadEndpointHandler interface ===

  /**
   * Called when an outbound write reaches the head of the thrift pipeline.
   *
   * Converts ThriftServerResponseMessage to RocketResponseMessage and writes
   * it into the rocket pipeline. The variant's `toRocketFrame()` does the
   * fold-expression dispatch; no runtime switch needed here.
   *
   * Today only REQUEST_RESPONSE is wired through the server; `streamType`
   * is hardcoded. When STREAM / CHANNEL / FNF handlers come online, the
   * App will set `rpcKind` on the variant and we'll map it to streamType
   * here (e.g., via RpcKindMapping::toFrameType).
   */
  channel_pipeline::Result onWrite(
      channel_pipeline::TypeErasedBox&& msg) noexcept {
    auto response = msg.take<ThriftServerResponseMessage>();
    rocket::server::RocketResponseMessage rocketMsg{
        .frame = std::move(response.payload).toRocketFrame(metadataProtocol_),
        .streamType =
            apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE,
    };
    return rocketConn_->appAdapter->write(std::move(rocketMsg));
  }

  void handlerAdded() noexcept {}

  // Thrift pipeline destroyed → tear down the rocket connection.
  FOLLY_NOINLINE void handlerRemoved() noexcept;

  // Thrift pipeline went active. Just stamp our local view; the rocket
  // side drives activation via its onConnect callback.
  void onPipelineActive() noexcept { connected_ = true; }

  // Thrift pipeline went inactive — disconnect the rocket connection.
  // The rocket-side onDisconnect callback re-enters us; the connected_
  // flag absorbs the loop.
  FOLLY_NOINLINE void onPipelineInactive() noexcept;

  // Thrift pipeline signaled it can accept more inbound reads — relay
  // down to the rocket pipeline so its head TransportHandler can resume
  // the socket read callback.
  void onReadReady() noexcept {
    if (rocketConn_ && rocketConn_->appAdapter) {
      rocketConn_->appAdapter->notifyReadReady();
    }
  }

 private:
  // Rocket pipeline became active — activate the thrift pipeline so
  // handlers can react. Idempotent in connected_; reactivate after
  // disconnect works.
  FOLLY_NOINLINE void onConnect() noexcept;

  // Rocket pipeline went inactive — deactivate the thrift pipeline so
  // handlers drain in-flight state. Idempotent in connected_.
  FOLLY_NOINLINE void onDisconnect() noexcept;

  FOLLY_NOINLINE channel_pipeline::Result handleDecodeFailure(
      uint32_t streamId,
      apache::thrift::fast_thrift::frame::FrameType streamType,
      const folly::exception_wrapper& error) noexcept;

  channel_pipeline::PipelineImpl* pipeline_{nullptr};
  std::unique_ptr<folly::DelayedDestruction::DestructorGuard> pipelineGuard_;
  // Owned rocket connection — torn down by handlerRemoved /
  // onPipelineInactive when the thrift pipeline goes away.
  std::unique_ptr<rocket::server::RocketServerConnection> rocketConn_;
  rocket::server::MetadataProtocol metadataProtocol_{
      rocket::server::MetadataProtocol::BINARY};
  bool connected_{false};
};

} // namespace apache::thrift::fast_thrift::thrift::server
