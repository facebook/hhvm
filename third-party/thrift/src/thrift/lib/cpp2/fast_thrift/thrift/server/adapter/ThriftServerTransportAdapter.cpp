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

#include <string_view>

#include <thrift/lib/cpp2/fast_thrift/thrift/server/SetupResponseBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/ConnectionPayloads.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponseError.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/SetupMessages.h>
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

channel_pipeline::Result ThriftServerTransportAdapter::onConnectionFrame(
    rocket::server::RocketRequestMessage&& request) noexcept {
  if (FOLLY_LIKELY(
          request.frame.type() ==
          apache::thrift::fast_thrift::frame::FrameType::SETUP)) {
    return onSetupFrame(std::move(request));
  }
  XLOG_EVERY_MS(WARN, 60000)
      << "dropping unhandled connection frame " << request.frame.typeName();
  return channel_pipeline::Result::Success;
}

channel_pipeline::Result ThriftServerTransportAdapter::onSetupFrame(
    rocket::server::RocketRequestMessage&& request) noexcept {
  using ErrorCode = apache::thrift::fast_thrift::frame::ErrorCode;

  // Latched for the connection: every later request's metadata is decoded with
  // what this SETUP negotiated.
  metadataProtocol_ = request.metadataProtocol;

  auto decoded = fromRocketFrame(std::move(request.frame), metadataProtocol_);
  if (FOLLY_UNLIKELY(!decoded.hasValue())) {
    return rejectSetup(
        ErrorCode::INVALID_SETUP, "Could not read the client's SETUP metadata");
  }

  ThriftServerRequestMessage message;
  message.payload = std::move(decoded).value();
  // The traversal is synchronous, so by the time it returns the answer — the
  // SETUP response, or a refusal from a handler that declined to forward — is
  // already on the write path.
  return pipeline_->fireRead(
      channel_pipeline::erase_and_box(std::move(message)));
}

channel_pipeline::Result ThriftServerTransportAdapter::rejectSetup(
    apache::thrift::fast_thrift::frame::ErrorCode code,
    std::string_view reason) noexcept {
  const auto writeResult = writeToRocket(
      channel_pipeline::erase_and_box(makeSetupRejectionMessage(code, reason)));
  if (FOLLY_UNLIKELY(writeResult != channel_pipeline::Result::Success)) {
    XLOG(WARN) << "Failed to write setup rejection "
               << apache::thrift::fast_thrift::frame::toString(code);
  }
  // Non-Success whatever the write did: the connection is still awaiting a
  // setup, and reporting success would leave the rocket setup handler treating
  // the next frame as post-setup traffic.
  return channel_pipeline::Result::Error;
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
