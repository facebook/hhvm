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
#include <thrift/lib/cpp2/async/RequestCallback.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/RequestMetadata.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/ResponseMetadata.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/common/ThriftClientResponseProcessor.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

ThriftClientChannel::UniquePtr ThriftClientChannel::newChannel(
    folly::EventBase* evb, uint16_t protocolId) {
  return UniquePtr(new ThriftClientChannel(evb, protocolId));
}

ThriftClientChannel::ThriftClientChannel(
    folly::EventBase* evb, uint16_t protocolId)
    : evb_(evb), protocolId_(protocolId) {}

ThriftClientChannel::~ThriftClientChannel() {}

void ThriftClientChannel::setPipeline(
    apache::thrift::fast_thrift::channel_pipeline::PipelineImpl* pipeline) {
  pipeline_ = std::move(pipeline);
}

void ThriftClientChannel::sendRequestInternal(
    const apache::thrift::RpcOptions& options,
    apache::thrift::MethodMetadata&& methodMetadata,
    apache::thrift::SerializedRequest&& request,
    std::shared_ptr<apache::thrift::transport::THeader> /*header*/,
    apache::thrift::RequestClientCallback::Ptr callbackPtr,
    std::unique_ptr<folly::IOBuf> /*frameworkMetadata*/) {
  // Check pipeline is set
  if (!pipeline_) {
    callbackPtr.release()->onResponseError(
        folly::make_exception_wrapper<apache::thrift::TApplicationException>(
            apache::thrift::TApplicationException::INTERNAL_ERROR,
            "Pipeline not set"));
    return;
  }

  // Store callback in pending map keyed by request ID.
  // This ensures we maintain ownership until the request completes,
  // preventing use-after-move bugs on pipeline errors.
  auto requestId = nextRequestId_++;
  auto it = pendingCallbacks_.emplace(requestId, std::move(callbackPtr)).first;

  // Build and serialize RequestRpcMetadata
  std::unique_ptr<folly::IOBuf> metadata;
  try {
    metadata = makeSerializedRequestMetadata(
        options,
        methodMetadata.name_managed(),
        apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
        static_cast<apache::thrift::ProtocolId>(protocolId_));
  } catch (...) {
    auto callback = std::move(it->second);
    pendingCallbacks_.erase(it);
    callback.release()->onResponseError(
        folly::make_exception_wrapper<apache::thrift::TApplicationException>(
            apache::thrift::TApplicationException::INTERNAL_ERROR,
            "Metadata serialization failed"));
    return;
  }

  // Create ThriftRequestMessage for the pipeline
  ThriftRequestMessage msg{
      .payload =
          ThriftRequestPayload{
              .metadata = std::move(metadata),
              .data = std::move(request.buffer),
              .initialRequestN = 0,
              .rpcKind =
                  apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
              .complete = false,
          },
      .requestHandle = static_cast<uint32_t>(requestId),
  };

  // Write to the pipeline
  auto result = pipeline_->fireWrite(
      apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
          std::move(msg)));

  if (result == apache::thrift::fast_thrift::channel_pipeline::Result::Error) {
    // Pipeline failed - retrieve callback from pending map and invoke error
    auto cbIt = pendingCallbacks_.find(requestId);
    if (cbIt != pendingCallbacks_.end()) {
      auto callback = std::move(cbIt->second);
      pendingCallbacks_.erase(cbIt);
      callback.release()->onResponseError(
          folly::make_exception_wrapper<apache::thrift::TApplicationException>(
              apache::thrift::TApplicationException::INTERNAL_ERROR,
              "Pipeline write failed"));
    }
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
  auto& frame = response.frame;

  // Deserialize response metadata
  apache::thrift::ResponseRpcMetadata metadata;
  if (auto error = deserializeResponseMetadata(frame, metadata);
      FOLLY_UNLIKELY(!!error)) {
    callback.release()->onResponseError(std::move(error));
    return;
  }

  // Process payload metadata
  if (auto error = processPayloadMetadata(metadata); FOLLY_UNLIKELY(!!error)) {
    callback.release()->onResponseError(std::move(error));
    return;
  }

  if (auto result = processRequestResponseFrame(frame);
      FOLLY_UNLIKELY(result.hasError())) {
    callback.release()->onResponseError(std::move(result.error()));
  } else {
    auto tHeader = std::make_unique<apache::thrift::transport::THeader>();
    tHeader->setClientType(THRIFT_ROCKET_CLIENT_TYPE);
    apache::thrift::detail::fillTHeaderFromResponseRpcMetadata(
        metadata, *tHeader);
    callback.release()->onResponse(
        apache::thrift::ClientReceiveState(
            protocolId_,
            apache::thrift::MessageType::T_REPLY,
            apache::thrift::SerializedResponse(std::move(result.value())),
            std::move(tHeader),
            nullptr,
            apache::thrift::RpcTransportStats{}));
  }
}

apache::thrift::fast_thrift::channel_pipeline::Result
ThriftClientChannel::onMessage(
    apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
        msg) noexcept {
  auto response = msg.take<ThriftResponseMessage>();
  auto it = pendingCallbacks_.find(response.requestHandle);
  if (it == pendingCallbacks_.end()) {
    return apache::thrift::fast_thrift::channel_pipeline::Result::Error;
  }
  auto callback = std::move(it->second);
  pendingCallbacks_.erase(it);

  // NOLINTNEXTLINE(clang-diagnostic-switch-enum)
  switch (response.requestFrameType) {
    case apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE:
      handleRequestResponse(std::move(response), std::move(callback));
      break;
    default:
      XLOG(ERR) << "Unsupported frame type: "
                << static_cast<int>(response.requestFrameType);
      callback.release()->onResponseError(
          folly::make_exception_wrapper<apache::thrift::TApplicationException>(
              apache::thrift::TApplicationException::INTERNAL_ERROR,
              fmt::format(
                  "Unsupported frame type: {}",
                  static_cast<int>(response.requestFrameType))));
      if (pipeline_) {
        pipeline_->close();
      }
      break;
  }

  return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
}

void ThriftClientChannel::onException(folly::exception_wrapper&& e) noexcept {
  XLOG(ERR) << "Pipeline exception: " << e.what();

  // Fail all pending callbacks with the exception
  pendingCallbacks_.forEach(
      [&](uint64_t, apache::thrift::RequestClientCallback::Ptr& callback) {
        callback.release()->onResponseError(e);
      });
  pendingCallbacks_.clear();

  // Close the pipeline
  if (pipeline_) {
    pipeline_->close();
  }
}

} // namespace apache::thrift::fast_thrift::thrift
