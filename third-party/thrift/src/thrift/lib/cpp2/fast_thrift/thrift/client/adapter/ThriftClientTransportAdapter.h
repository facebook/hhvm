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
#include <folly/io/async/DelayedDestruction.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/common/RocketClientConnection.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/common/Event.h>

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
    // Subscribe to the connection's domain lifecycle. The bridge mirrors
    // those events into the thrift pipeline's lifecycle.
    connection_->setLifecycleHandlers(
        [this]() noexcept { onConnect(); },
        [this]() noexcept { onDisconnect(); });
    // Bridge rocket-pipeline egress-drain notifications into the thrift
    // pipeline. Fired when the rocket transport's write buffer drains;
    // walks the thrift pipeline's writeReadyList and notifies its tail.
    connection_->appAdapter->setOnWriteReady([this]() noexcept {
      if (pipeline_) {
        pipeline_->onWriteReady();
      }
    });
    // Bridge the rocket transport's graceful-close notification (server sent
    // CONNECTION_CLOSE) into a thrift CloseConnection event. The
    // pipeline-resident drain handler reacts to it; the bridge only
    // translates the rocket-native signal into thrift vocabulary. The event
    // carries no payload — the type alone is the signal.
    connection_->appAdapter->setOnClose([this]() noexcept {
      if (pipeline_) {
        pipeline_->fireEvent(
            ThriftClientEventType::CloseConnection,
            channel_pipeline::TypeErasedBox{});
      }
    });
    // Bridge per-request write-completion notifications from the rocket
    // pipeline into the thrift pipeline.
    connection_->appAdapter->setOnWriteComplete(
        [this](const rocket::client::RocketWriteCompleteEvent& e) noexcept {
          onWriteComplete(e);
        });
  }

  ~ThriftClientTransportAdapter() {
    // Defensive: if the bridge is destroyed without going through the
    // thrift pipeline's handlerRemoved (which would have called destroy
    // already), tear the rocket connection down explicitly. destroy() is
    // idempotent so the normal path stays a no-op.
    resetPipeline();
    if (connection_) {
      connection_->destroy();
    }
  }

  ThriftClientTransportAdapter(const ThriftClientTransportAdapter&) = delete;
  ThriftClientTransportAdapter& operator=(const ThriftClientTransportAdapter&) =
      delete;
  ThriftClientTransportAdapter(ThriftClientTransportAdapter&&) = delete;
  ThriftClientTransportAdapter& operator=(ThriftClientTransportAdapter&&) =
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

  // === Transport-facing interface (inbound from the connection) ===

  /**
   * Called when the connection delivers a response.
   *
   * Converts the rocket response message to a thrift response message and
   * pushes it into the thrift pipeline via fireRead. Per-request errors
   * are translated into a `ThriftClientResponseError` payload alternative —
   * they travel inbound through the same fireRead path, so the tail can
   * fail just the affected pending callback without tearing down the
   * channel.
   */
  channel_pipeline::Result onTransportResponse(
      channel_pipeline::TypeErasedBox&& msg) noexcept {
    auto response = msg.take<rocket::RocketResponseMessage>();

    ThriftResponseMessage thriftMsg{
        .payload = {},
        .requestContext = std::move(response.requestContext),
        .streamType = response.streamType,
        .rpcTransportStats = toRpcTransportStats(response.stats),
    };

    if (FOLLY_UNLIKELY(response.payload.is<rocket::RocketResponseError>())) {
      thriftMsg.payload = ThriftClientResponseError{
          .ew = std::move(
              response.payload.get<rocket::RocketResponseError>().ew)};
    } else {
      // Decode the parsed wire frame into the typed thrift inbound payload
      // variant. From here on, the thrift pipeline never sees `ParsedFrame`.
      //
      // Today only REQUEST_RESPONSE is wired end-to-end on the client;
      // hard-code the corresponding `RpcKind`. METADATA_PUSH frames ignore
      // `kind` in fromRocketFrame. As FNF / Stream / Sink / Bidi land,
      // replace this with a `streamType → RpcKind` mapping.
      auto decoded = fromRocketFrame(
          std::move(response.payload.get<frame::read::ParsedFrame>()),
          apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);
      if (FOLLY_UNLIKELY(!decoded.hasValue())) {
        // Wire-decode failure: surface as a per-request
        // ThriftClientResponseError so the channel fails just this
        // callback. Connection stays Open.
        thriftMsg.payload =
            ThriftClientResponseError{.ew = std::move(decoded.error())};
      } else {
        thriftMsg.payload = std::move(decoded.value());
      }
    }

    return pipeline_->fireRead(
        channel_pipeline::erase_and_box(std::move(thriftMsg)));
  }

  /**
   * Called when the connection delivers an error. Propagates the error
   * up the thrift pipeline. The disconnect happens via the lifecycle
   * path (onDisconnect callback / onPipelineInactive), not from here.
   */
  void onTransportError(folly::exception_wrapper&& ew) noexcept {
    if (FOLLY_UNLIKELY(!pipeline_)) {
      return;
    }
    pipeline_->fireException(std::move(ew));
  }

  // Called when the rocket pipeline reports a per-request write completion.
  // Relays it up the thrift pipeline as ThriftClientEventType::WriteComplete.
  // Precondition: pipeline is wired (the rocket connection that fires this is
  // torn down before pipeline_ is cleared).
  void onWriteComplete(
      const rocket::client::RocketWriteCompleteEvent& event) noexcept {
    pipeline_->fireEvent(
        ThriftClientEventType::WriteComplete,
        channel_pipeline::TypeErasedBox(
            ThriftClientWriteCompleteEvent{
                .requestContext = event.requestContext,
                .status = event.status,
            }));
  }

  // === HeadEndpointHandler interface ===

  /**
   * Called when an outbound write reaches the head of the thrift
   * pipeline. Converts the thrift request message to a rocket request
   * message and submits it to the connection.
   *
   * Today only the request-response RPC pattern is wired through the
   * client; `ThriftRequestMessage::payload` is a single-alternative
   * variant of `ThriftRequestResponsePayload`. As other RpcKinds are
   * added to the variant, this method gains dispatch over the held
   * alternative.
   */
  channel_pipeline::Result onWrite(
      channel_pipeline::TypeErasedBox&& msg) noexcept {
    auto request = msg.take<ThriftRequestMessage>();
    // toRocketFrame() serializes the request metadata and can throw on
    // serializer/allocator failure. Catch and deliver inbound as a
    // per-request ThriftClientResponseError so the AppAdapter fails just this
    // request without tearing the channel down.
    frame::ComposedFrame frame;
    try {
      // Client outbound: request metadata is currently always Binary on the
      // wire — SETUP-time negotiation is not yet plumbed through the client
      // transport adapter. The arg is uniform with the server-side variant
      // dispatch but unused for request metadata today.
      frame = std::move(request.payload.get<ThriftRequestResponsePayload>())
                  .toRocketFrame(rocket::server::MetadataProtocol::BINARY);
    } catch (...) {
      ThriftResponseMessage errMsg{
          .payload =
              ThriftClientResponseError{
                  .ew = folly::exception_wrapper(std::current_exception())},
          .requestContext = std::move(request.requestContext),
          .streamType = frame::FrameType::REQUEST_RESPONSE,
          .rpcTransportStats = {},
      };
      return pipeline_->fireRead(
          channel_pipeline::erase_and_box(std::move(errMsg)));
    }
    rocket::RocketRequestMessage rocketMsg{
        .frame = std::move(frame),
        .requestContext = std::move(request.requestContext),
        .streamType = frame::FrameType::REQUEST_RESPONSE,
    };
    return connection_->appAdapter->write(std::move(rocketMsg));
  }

  // Thrift pipeline signaled it can accept more inbound reads — relay
  // down to the rocket pipeline so its head TransportHandler can resume
  // the socket read callback.
  void onReadReady() noexcept {
    if (connection_ && connection_->appAdapter) {
      connection_->appAdapter->notifyReadReady();
    }
  }

  void handlerAdded() noexcept {}

  // Thrift pipeline destroyed → tear down the rocket connection.
  void handlerRemoved() noexcept {
    DCHECK(!connected_);
    // Defensive: removed without prior deactivate. destroy() runs
    // disconnect first, so the rocket side still tears down cleanly.
    connected_ = false;
    if (connection_) {
      connection_->destroy();
    }
  }

  // Thrift pipeline went active. Just stamp our local view; the
  // connection drives activation via its onConnect callback.
  void onPipelineActive() noexcept { connected_ = true; }

  // Thrift pipeline went inactive — disconnect the connection. The
  // connection's onDisconnect callback re-enters us; the connected_
  // flag absorbs the loop.
  void onPipelineInactive() noexcept {
    if (!connected_) {
      return;
    }
    connected_ = false;
    if (connection_) {
      connection_->disconnect();
    }
  }

 private:
  // Translate the rocket layer's transport stats into the thrift-layer
  // RpcTransportStats. The uncompressed *SerializedSizeBytes fields are a
  // thrift-layer concept the rocket layer cannot observe; they stay zero.
  static apache::thrift::RpcTransportStats toRpcTransportStats(
      const rocket::RocketStats& s) noexcept {
    apache::thrift::RpcTransportStats out;
    out.requestWireSizeBytes = s.requestWireSizeBytes;
    out.requestMetadataAndPayloadSizeBytes =
        s.requestMetadataAndPayloadSizeBytes;
    out.responseWireSizeBytes = s.responseWireSizeBytes;
    out.responseMetadataAndPayloadSizeBytes =
        s.responseMetadataAndPayloadSizeBytes;
    out.requestWriteLatency = s.requestWriteLatency;
    out.responseRoundTripLatency = s.responseRoundTripLatency;
    return out;
  }

  // Connection came up — activate the thrift pipeline so handlers can
  // react. Idempotent in connected_; reactivate after disconnect works.
  void onConnect() noexcept {
    if (connected_) {
      return;
    }
    connected_ = true;
    if (pipeline_) {
      pipeline_->activate();
    }
  }

  // Connection went down — deactivate the thrift pipeline so handlers
  // drain in-flight state. Idempotent in connected_.
  void onDisconnect() noexcept {
    if (!connected_) {
      return;
    }
    connected_ = false;
    if (pipeline_) {
      pipeline_->deactivate();
    }
  }

  channel_pipeline::PipelineImpl* pipeline_{nullptr};
  std::unique_ptr<folly::DelayedDestruction::DestructorGuard> pipelineGuard_;
  std::unique_ptr<rocket::client::RocketClientConnection> connection_;
  bool connected_{false};
};

} // namespace apache::thrift::fast_thrift::thrift::client
