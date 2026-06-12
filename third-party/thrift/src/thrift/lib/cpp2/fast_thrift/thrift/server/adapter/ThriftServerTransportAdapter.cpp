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

#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerTransportAdapter.h>

#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponseError.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift::server {

ThriftServerTransportAdapter::ThriftServerTransportAdapter(
    std::unique_ptr<rocket::server::RocketServerConnection> rocketConn)
    : rocketConn_(std::move(rocketConn)) {
  DCHECK(rocketConn_);
  rocketConn_->appAdapter->setRequestHandlers(
      [this](channel_pipeline::TypeErasedBox&& msg) noexcept {
        return onTransportRequest(std::move(msg));
      },
      [this](folly::exception_wrapper&& e) noexcept {
        onTransportError(std::move(e));
      });
  // Mirror rocket lifecycle into the thrift pipeline.
  rocketConn_->setLifecycleHandlers(
      [this]() noexcept { onConnect(); },
      [this]() noexcept { onDisconnect(); });
  // Bridge rocket-pipeline egress-drain notifications into the thrift
  // pipeline. Fired when the rocket transport's write buffer drains;
  // walks the thrift pipeline's writeReadyList and notifies its tail.
  rocketConn_->appAdapter->setOnWriteReady([this]() noexcept {
    if (pipeline_) {
      pipeline_->onWriteReady();
    }
  });
  // Bridge rocket-pipeline write-completion notifications into the thrift
  // pipeline.
  rocketConn_->appAdapter->setOnWriteComplete(
      [this](const rocket::server::RocketWriteCompleteEvent& e) noexcept {
        onWriteComplete(e);
      });
}

ThriftServerTransportAdapter::~ThriftServerTransportAdapter() {
  // Defensive: if the bridge is destroyed without going through the
  // thrift pipeline's handlerRemoved, tear the rocket connection down
  // explicitly. RocketServerConnection::destroy() is idempotent.
  resetPipeline();
  if (rocketConn_) {
    rocketConn_->destroy();
  }
}

void ThriftServerTransportAdapter::handlerRemoved() noexcept {
  // Pipeline destruction can reach handlerRemoved without a preceding
  // deactivate (owner-initiated destruction on the IO thread during stop
  // is one such path). Proactively disconnect the rocket connection so
  // its lifecycle handlers see the inactive transition before destroy.
  if (connected_) {
    connected_ = false;
    if (rocketConn_) {
      rocketConn_->disconnect();
    }
  }
  if (rocketConn_) {
    rocketConn_->destroy();
  }
}

void ThriftServerTransportAdapter::onPipelineInactive() noexcept {
  if (!connected_) {
    return;
  }
  connected_ = false;
  if (rocketConn_) {
    rocketConn_->disconnect();
  }
}

void ThriftServerTransportAdapter::onConnect() noexcept {
  if (connected_) {
    return;
  }
  connected_ = true;
  if (pipeline_) {
    pipeline_->activate();
  }
}

void ThriftServerTransportAdapter::onDisconnect() noexcept {
  if (!connected_) {
    return;
  }
  connected_ = false;
  if (pipeline_) {
    pipeline_->deactivate();
  }
}

channel_pipeline::Result ThriftServerTransportAdapter::handleDecodeFailure(
    uint32_t streamId,
    apache::thrift::fast_thrift::frame::FrameType streamType,
    const folly::exception_wrapper& error) noexcept {
  // Per-request decode failure: synthesize a REQUEST_PARSING_FAILURE
  // ERROR frame and write it outbound through the rocket pipeline. The
  // error body is Compact-serialized regardless of negotiated metadata
  // protocol (ResponseRpcError is a control-frame body with its own wire
  // contract, not application metadata).
  auto serialized = serializeResponseRpcError(
      apache::thrift::ResponseRpcErrorCode::REQUEST_PARSING_FAILURE,
      error.what().toStdString());
  ThriftErrorPayload errorPayload{
      .data = std::move(serialized.data),
      .metadata = nullptr,
      .streamId = streamId,
      .errorCode = static_cast<uint32_t>(serialized.errorCode),
  };
  return rocketConn_->appAdapter->write(
      rocket::server::RocketResponseMessage{
          .frame = std::move(errorPayload).toRocketFrame(metadataProtocol_),
          .streamType = streamType,
      });
}

} // namespace apache::thrift::fast_thrift::thrift::server
