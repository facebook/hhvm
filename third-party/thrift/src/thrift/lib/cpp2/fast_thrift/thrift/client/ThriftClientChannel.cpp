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

#include <thrift/lib/cpp2/fast_thrift/thrift/client/ThriftClientChannel.h>

#include <fmt/core.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/async/RequestCallback.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/common/TypeErasedPtr.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/RequestMetadata.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/ResponseMetadata.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/common/PayloadVariants.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/util/ErrorDecoding.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/util/RocketFrameDecoder.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/transport/core/RpcMetadataUtil.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

ThriftClientChannel::UniquePtr ThriftClientChannel::newChannel(
    std::unique_ptr<rocket::client::RocketClientConnection> connection,
    uint16_t protocolId) {
  return UniquePtr(new ThriftClientChannel(std::move(connection), protocolId));
}

ThriftClientChannel::ThriftClientChannel(
    std::unique_ptr<rocket::client::RocketClientConnection> connection,
    uint16_t protocolId)
    : connection_(std::move(connection)),
      evb_(connection_->pipeline->eventBase()),
      protocolId_(protocolId) {
  // Register as the rocket connection's response + lifecycle sink. The
  // connection owns the rocket pipeline; the channel drives it via the
  // app adapter.
  connection_->appAdapter->setResponseHandlers(
      [this](channel_pipeline::TypeErasedBox&& msg) noexcept {
        return onResponse(std::move(msg));
      },
      [this](folly::exception_wrapper&& e) noexcept { onError(std::move(e)); });
  // The channel is always handed an already-connected connection, so it does
  // not observe connect; it only needs the disconnect edge to stop accepting
  // writes.
  connection_->setLifecycleHandlers(
      []() noexcept {}, [this]() noexcept { onDisconnect(); });
}

ThriftClientChannel::~ThriftClientChannel() {
  if (connection_) {
    connection_->destroy();
  }
}

void ThriftClientChannel::onDisconnect() noexcept {
  // Connection went down — the canonical "no longer accepting writes" edge.
  state_ = State::Closed;
}

void ThriftClientChannel::sendRequestInternal(
    const apache::thrift::RpcOptions& options,
    apache::thrift::MethodMetadata&& methodMetadata,
    apache::thrift::SerializedRequest&& request,
    std::shared_ptr<apache::thrift::transport::THeader> /*header*/,
    apache::thrift::RequestClientCallback::Ptr callbackPtr,
    std::unique_ptr<folly::IOBuf> /*frameworkMetadata*/) {
  evb_->dcheckIsInEventBaseThread();

  if (FOLLY_UNLIKELY(state_ != State::Open)) {
    handleNotOpen(std::move(callbackPtr));
    return;
  }

  auto metadata = makeRequestMetadata(
      options,
      methodMetadata.name_managed(),
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      static_cast<apache::thrift::ProtocolId>(protocolId_));

  // Serialize the request inline. toRocketFrame() serializes the request
  // metadata and can throw on serializer/allocator failure — fail just this
  // request without tearing down the channel.
  apache::thrift::fast_thrift::frame::ComposedFrame frame;
  try {
    // Request metadata is currently always Binary on the wire — SETUP-time
    // negotiation is not yet plumbed through the client.
    frame =
        ThriftRequestResponsePayload{
            .data = std::move(request.buffer),
            .metadata = std::move(metadata),
        }
            .toRocketFrame(rocket::server::MetadataProtocol::BINARY);
  } catch (...) {
    handleMetadataError(
        std::move(callbackPtr),
        folly::exception_wrapper(std::current_exception()));
    return;
  }

  // RequestClientCallback::Ptr has an auto-detach deleter that fires
  // onResponseError if dropped without firing — so the context's default
  // delete is sufficient to handle the rescue path on write failure.
  rocket::RocketRequestMessage rocketMsg{
      .frame = std::move(frame),
      .requestContext = apache::thrift::fast_thrift::rocket::from_unique_ptr(
          std::make_unique<ChannelCallbackContext>(std::move(callbackPtr))),
      .streamType =
          apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE,
  };

  auto result = connection_->appAdapter->write(std::move(rocketMsg));
  if (FOLLY_UNLIKELY(
          result ==
          apache::thrift::fast_thrift::channel_pipeline::Result::Error)) {
    handleWriteError();
  }
}

void ThriftClientChannel::sendRequestResponse(
    const apache::thrift::RpcOptions& options,
    apache::thrift::MethodMetadata&& methodMetadata,
    apache::thrift::SerializedRequest&& request,
    std::shared_ptr<apache::thrift::transport::THeader> header,
    apache::thrift::RequestClientCallback::Ptr callbackPtr,
    std::unique_ptr<folly::IOBuf> frameworkMetadata) {
  sendRequestInternal(
      options,
      std::move(methodMetadata),
      std::move(request),
      std::move(header),
      std::move(callbackPtr),
      std::move(frameworkMetadata));
}

// Rvalue overload delegates to const& version
void ThriftClientChannel::sendRequestResponse(
    apache::thrift::RpcOptions&& options,
    apache::thrift::MethodMetadata&& methodMetadata,
    apache::thrift::SerializedRequest&& request,
    std::shared_ptr<apache::thrift::transport::THeader> header,
    apache::thrift::RequestClientCallback::Ptr callbackPtr,
    std::unique_ptr<folly::IOBuf> frameworkMetadata) {
  sendRequestResponse(
      static_cast<const apache::thrift::RpcOptions&>(options),
      std::move(methodMetadata),
      std::move(request),
      std::move(header),
      std::move(callbackPtr),
      std::move(frameworkMetadata));
}

apache::thrift::fast_thrift::channel_pipeline::Result
ThriftClientChannel::onResponse(
    apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
        msg) noexcept {
  auto response = msg.take<rocket::RocketResponseMessage>();
  auto ctx = response.requestContext.release_as<ChannelCallbackContext>();
  if (FOLLY_UNLIKELY(!ctx)) {
    handleNullContext();
    return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
  }
  auto callback = std::move(ctx->cb);
  ctx.reset();

  // Per-request error from below (e.g., rocket in-process serialize failure
  // or wire-decode failure). Fail just this callback; channel stays Open.
  if (FOLLY_UNLIKELY(response.payload.is<rocket::RocketResponseError>())) {
    handleResponseError(
        std::move(callback),
        std::move(response.payload.get<rocket::RocketResponseError>().ew));
    return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
  }

  DCHECK(
      response.streamType ==
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE)
      << "Unsupported frame type: " << static_cast<int>(response.streamType);

  handleRequestResponse(
      std::move(response.payload.get<frame::read::ParsedFrame>()),
      std::move(callback));
  return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
}

void ThriftClientChannel::handleRequestResponse(
    apache::thrift::fast_thrift::frame::read::ParsedFrame&& frame,
    apache::thrift::RequestClientCallback::Ptr callback) {
  // Decode the wire frame into the typed thrift inbound payload variant.
  auto decoded = fromRocketFrame(
      std::move(frame),
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE);
  if (FOLLY_UNLIKELY(!decoded.hasValue())) {
    handleResponseError(std::move(callback), std::move(decoded.error()));
    return;
  }

  auto& inbound = decoded.value();

  if (FOLLY_LIKELY(inbound.is<ThriftInitialResponsePayload>())) {
    auto& payload = inbound.get<ThriftInitialResponsePayload>();
    DCHECK(payload.metadata != nullptr);
    auto& metadata = *payload.metadata;

    if (auto error = processPayloadMetadata(metadata);
        FOLLY_UNLIKELY(!!error)) {
      handleMetadataError(std::move(callback), std::move(error));
      return;
    }

    auto tHeader = std::make_unique<apache::thrift::transport::THeader>();
    tHeader->setClientType(THRIFT_ROCKET_CLIENT_TYPE);
    apache::thrift::detail::fillTHeaderFromResponseRpcMetadata(
        metadata, *tHeader);
    callback.release()->onResponse(
        apache::thrift::ClientReceiveState(
            protocolId_,
            apache::thrift::MessageType::T_REPLY,
            apache::thrift::SerializedResponse(std::move(payload.data)),
            std::move(tHeader),
            nullptr,
            apache::thrift::RpcTransportStats{}));
    return;
  }

  if (inbound.is<ThriftErrorPayload>()) {
    auto& payload = inbound.get<ThriftErrorPayload>();
    auto decodedError =
        decodeErrorAsResponse(payload.errorCode, payload.data.get());
    if (decodedError.hasValue()) {
      auto& resp = decodedError.value();
      apache::thrift::TApplicationException tae(resp.exType, resp.what);
      auto serialized = apache::thrift::serializeErrorStruct(
          static_cast<apache::thrift::protocol::PROTOCOL_TYPES>(protocolId_),
          tae);

      apache::thrift::ResponseRpcMetadata errorMetadata;
      if (resp.exCode) {
        errorMetadata.otherMetadata().emplace();
        (*errorMetadata.otherMetadata())[std::string(
            apache::thrift::detail::kHeaderEx)] = *resp.exCode;
      }
      if (resp.load) {
        errorMetadata.load() = *resp.load;
      }

      auto tHeader = std::make_unique<apache::thrift::transport::THeader>();
      tHeader->setClientType(THRIFT_ROCKET_CLIENT_TYPE);
      apache::thrift::detail::fillTHeaderFromResponseRpcMetadata(
          errorMetadata, *tHeader);
      callback.release()->onResponse(
          apache::thrift::ClientReceiveState(
              protocolId_,
              apache::thrift::MessageType::T_EXCEPTION,
              apache::thrift::SerializedResponse(std::move(serialized)),
              std::move(tHeader),
              nullptr,
              apache::thrift::RpcTransportStats{}));
    } else {
      callback.release()->onResponseError(std::move(decodedError.error()));
    }
    return;
  }

  callback.release()->onResponseError(
      folly::make_exception_wrapper<apache::thrift::TApplicationException>(
          apache::thrift::TApplicationException::PROTOCOL_ERROR,
          "Unexpected payload alternative on RR response"));
}

void ThriftClientChannel::onError(folly::exception_wrapper&& e) noexcept {
  XLOG(ERR) << "Rocket connection exception: " << e.what();
  if (state_ == State::Closed) {
    return;
  }

  if (state_ == State::Open) {
    lastError_ = e;
  }

  auto* tex = e.get_exception<apache::thrift::transport::TTransportException>();
  if (tex &&
      tex->getType() ==
          apache::thrift::transport::TTransportException::NOT_OPEN) {
    // CONNECTION_CLOSE: graceful drain — reject new writes, let inflight
    // responses complete.
    state_ = State::Closing;
    return;
  }

  state_ = State::Closed;
}

void ThriftClientChannel::handleNotOpen(
    apache::thrift::RequestClientCallback::Ptr callbackPtr) noexcept {
  callbackPtr.release()->onResponseError(
      folly::make_exception_wrapper<
          apache::thrift::transport::TTransportException>(
          apache::thrift::transport::TTransportException::NOT_OPEN,
          lastError_
              ? fmt::format(
                    "Connection not open: {}", lastError_.what().toStdString())
              : "Connection not open"));
}

void ThriftClientChannel::handleWriteError() noexcept {
  // Close the channel. Subsequent sends will be rejected.
  onError(
      folly::make_exception_wrapper<
          apache::thrift::transport::TTransportException>(
          apache::thrift::transport::TTransportException::UNKNOWN,
          "Failed to write request to rocket connection"));
}

void ThriftClientChannel::handleMetadataError(
    apache::thrift::RequestClientCallback::Ptr callback,
    folly::exception_wrapper error) noexcept {
  callback.release()->onResponseError(std::move(error));
}

void ThriftClientChannel::handleNullContext() noexcept {
  XLOG(WARN) << "Response with null requestContext";
}

void ThriftClientChannel::handleResponseError(
    apache::thrift::RequestClientCallback::Ptr callback,
    folly::exception_wrapper ew) noexcept {
  callback.release()->onResponseError(std::move(ew));
}

} // namespace apache::thrift::fast_thrift::thrift
