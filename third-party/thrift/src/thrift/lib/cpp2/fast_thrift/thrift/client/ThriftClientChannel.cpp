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
#include <thrift/lib/cpp2/fast_thrift/thrift/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/RequestMetadata.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/ResponseMetadata.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/util/ErrorDecoding.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ThriftPayload.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/transport/core/RpcMetadataUtil.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

ThriftClientChannel::UniquePtr ThriftClientChannel::newChannel(
    folly::EventBase* evb, uint16_t protocolId) {
  return UniquePtr(new ThriftClientChannel(evb, protocolId));
}

ThriftClientChannel::ThriftClientChannel(
    folly::EventBase* evb, uint16_t protocolId)
    : evb_(evb), protocolId_(protocolId) {}

ThriftClientChannel::~ThriftClientChannel() {
  resetPipeline();
}

void ThriftClientChannel::setPipeline(
    apache::thrift::fast_thrift::channel_pipeline::PipelineImpl* pipeline) {
  DCHECK(pipeline);
  if (pipeline_) {
    XLOG(FATAL) << "must reset pipeline before setting a new one";
  }
  pipeline_ = pipeline;
  pipelineGuard_ =
      std::make_unique<folly::DelayedDestruction::DestructorGuard>(pipeline);
}

void ThriftClientChannel::resetPipeline() noexcept {
  pipeline_ = nullptr;
  pipelineGuard_.reset();
}

void ThriftClientChannel::onPipelineActive() noexcept {
  state_ = State::Open;
  lastError_ = {};
}

void ThriftClientChannel::onPipelineInactive() noexcept {
  // Pipeline disconnect is the canonical "no longer accepting writes"
  // edge. Whether we got here via a graceful close or a hard error, the
  // state machine ends up Closed.
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

  if (FOLLY_UNLIKELY(!pipeline_)) {
    callbackPtr.release()->onResponseError(
        folly::make_exception_wrapper<apache::thrift::TApplicationException>(
            apache::thrift::TApplicationException::INTERNAL_ERROR,
            "Pipeline not set"));
    return;
  }

  if (FOLLY_UNLIKELY(state_ != State::Open)) {
    callbackPtr.release()->onResponseError(
        folly::make_exception_wrapper<
            apache::thrift::transport::TTransportException>(
            apache::thrift::transport::TTransportException::NOT_OPEN,
            lastError_ ? fmt::format(
                             "Connection not open: {}",
                             lastError_.what().toStdString())
                       : "Connection not open"));
    return;
  }

  // Build RequestRpcMetadata; serialization happens in
  // ThriftRequestResponsePayload::toRocketFrame() at the transport adapter.
  auto metadata = makeRequestMetadata(
      options,
      methodMetadata.name_managed(),
      apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
      static_cast<apache::thrift::ProtocolId>(protocolId_));

  // RequestClientCallback::Ptr has an auto-detach deleter that fires
  // onResponseError if dropped without firing — so default delete here
  // is sufficient to handle the rescue path.
  ThriftRequestMessage msg{
      .payload =
          ThriftRequestResponsePayload{
              .data = std::move(request.buffer),
              .metadata = std::move(metadata),
          },
      .requestContext = apache::thrift::fast_thrift::rocket::from_unique_ptr(
          std::make_unique<ChannelCallbackContext>(std::move(callbackPtr))),
  };

  auto result = pipeline_->fireWrite(
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(msg)));

  if (FOLLY_UNLIKELY(
          result ==
          apache::thrift::fast_thrift::channel_pipeline::Result::Error)) {
    // Close the channel. Subsequent sends will be rejected; pending
    // streams will be failed.
    onException(
        folly::make_exception_wrapper<
            apache::thrift::transport::TTransportException>(
            apache::thrift::transport::TTransportException::UNKNOWN,
            "Failed to write request to pipeline"));
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

void ThriftClientChannel::handleRequestResponse(
    ThriftResponseMessage&& response,
    apache::thrift::RequestClientCallback::Ptr callback) {
  auto& inbound = response.payload.get<ThriftClientInboundPayloadVariant>();

  if (FOLLY_LIKELY(inbound.is<ThriftFirstResponsePayload>())) {
    auto& payload = inbound.get<ThriftFirstResponsePayload>();
    DCHECK(payload.metadata != nullptr);
    auto& metadata = *payload.metadata;

    if (auto error = processPayloadMetadata(metadata);
        FOLLY_UNLIKELY(!!error)) {
      callback.release()->onResponseError(std::move(error));
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
    auto decoded = decodeErrorAsResponse(payload.errorCode, payload.data.get());
    if (decoded.hasValue()) {
      auto& resp = decoded.value();
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
      callback.release()->onResponseError(std::move(decoded.error()));
    }
    return;
  }

  callback.release()->onResponseError(
      folly::make_exception_wrapper<apache::thrift::TApplicationException>(
          apache::thrift::TApplicationException::PROTOCOL_ERROR,
          "Unexpected payload alternative on RR response"));
}

apache::thrift::fast_thrift::channel_pipeline::Result
ThriftClientChannel::onRead(
    apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
        msg) noexcept {
  auto response = msg.take<ThriftResponseMessage>();
  auto ctx = response.requestContext.release_as<ChannelCallbackContext>();
  if (FOLLY_UNLIKELY(!ctx)) {
    XLOG(WARN) << "Response with null requestContext";
    return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
  }
  auto callback = std::move(ctx->cb);
  ctx.reset();

  // Per-request error from below the thrift pipeline (e.g., rocket in-process
  // serialize failure). Fail just this callback; channel stays Open for
  // subsequent requests.
  if (FOLLY_UNLIKELY(response.payload.is<ThriftClientResponseError>())) {
    callback.release()->onResponseError(
        std::move(response.payload.get<ThriftClientResponseError>().ew));
    return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
  }

  DCHECK(
      response.streamType ==
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE)
      << "Unsupported frame type: " << static_cast<int>(response.streamType);

  handleRequestResponse(std::move(response), std::move(callback));
  return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
}

void ThriftClientChannel::onException(folly::exception_wrapper&& e) noexcept {
  XLOG(ERR) << "Pipeline exception: " << e.what();
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
    // responses complete
    state_ = State::Closing;
    return;
  }

  state_ = State::Closed;
}

} // namespace apache::thrift::fast_thrift::thrift
