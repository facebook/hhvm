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
    rocket::server::RocketServerAppAdapter& appAdapter,
    OnRocketDisconnectFn onRocketDisconnect,
    OnRocketDestroyFn onRocketDestroy)
    : appAdapter_(appAdapter),
      onRocketDisconnect_(std::move(onRocketDisconnect)),
      onRocketDestroy_(std::move(onRocketDestroy)) {
  appAdapter_.setRequestHandlers(
      [this](channel_pipeline::TypeErasedBox&& msg) noexcept {
        return onTransportRequest(std::move(msg));
      },
      [this](folly::exception_wrapper&& e) noexcept {
        onTransportError(std::move(e));
      });
  // Subscribe to the rocket adapter's lifecycle. The bridge mirrors
  // those events into the thrift pipeline's lifecycle. onClose lets
  // the bridge observe that the rocket side has fully torn down so it
  // can drop the action callbacks — they'd UAF if invoked later from
  // the bridge's own teardown path.
  appAdapter_.setLifecycleHandlers(
      [this]() noexcept { onConnect(); },
      [this]() noexcept { onDisconnect(); },
      [this]() noexcept { onRocketClosed(); });
}

ThriftServerTransportAdapter::~ThriftServerTransportAdapter() {
  // Graceful-stop tears the bridge down before the rocket adapter. The
  // ctor installed `this`-capturing lambdas on appAdapter_ via
  // setRequestHandlers/setLifecycleHandlers; clear them so a later
  // rocket-side close/disconnect (e.g. from ConnectionManager
  // closeConnections after our drain) can't fire them and dereference
  // freed memory. Skip when rocket has already torn down (onRocketClosed
  // cleared rocketAlive_) — the reference would itself be dangling.
  if (rocketAlive_) {
    appAdapter_.setRequestHandlers({}, {});
    appAdapter_.setLifecycleHandlers({}, {}, {});
  }
  // Defensive: if the bridge is destroyed without going through the
  // thrift pipeline's handlerRemoved (which would have invoked the
  // destroy callback already), tear the rocket connection down
  // explicitly. The callback is idempotent so the normal path stays a
  // no-op.
  resetPipeline();
  if (onRocketDestroy_) {
    onRocketDestroy_();
  }
}

void ThriftServerTransportAdapter::handlerRemoved() noexcept {
  // Contract: the thrift pipeline always deactivates before removing
  // handlers, so connected_ must already be false here.
  DCHECK(!connected_);
  if (onRocketDestroy_) {
    auto fn = std::move(onRocketDestroy_);
    fn();
  }
}

void ThriftServerTransportAdapter::onPipelineInactive() noexcept {
  if (!connected_) {
    return;
  }
  connected_ = false;
  if (onRocketDisconnect_) {
    onRocketDisconnect_();
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

void ThriftServerTransportAdapter::onRocketClosed() noexcept {
  rocketAlive_ = false;
  onRocketDisconnect_ = {};
  onRocketDestroy_ = {};
}

channel_pipeline::Result ThriftServerTransportAdapter::handleDecodeFailure(
    uint32_t streamId,
    apache::thrift::fast_thrift::frame::FrameType streamType,
    const folly::exception_wrapper& error) noexcept {
  // Per-request decode failure: synthesize a REQUEST_PARSING_FAILURE
  // ERROR frame and write it outbound through the rocket pipeline.
  // Don't propagate as a connection-level exception or push a valueless
  // payload downstream — the connection itself is healthy. The error
  // body is Compact-serialized regardless of negotiated metadata
  // protocol (matches legacy: ResponseRpcError is a control-frame body
  // with its own wire contract, not application metadata).
  auto serialized = serializeResponseRpcError(
      apache::thrift::ResponseRpcErrorCode::REQUEST_PARSING_FAILURE,
      error.what().toStdString());
  ThriftErrorPayload errorPayload{
      .data = std::move(serialized.data),
      .metadata = nullptr,
      .streamId = streamId,
      .errorCode = static_cast<uint32_t>(serialized.errorCode),
  };
  return appAdapter_.write(
      rocket::server::RocketResponseMessage{
          .frame = std::move(errorPayload).toRocketFrame(metadataProtocol_),
          .streamType = streamType,
      });
}

} // namespace apache::thrift::fast_thrift::thrift::server
