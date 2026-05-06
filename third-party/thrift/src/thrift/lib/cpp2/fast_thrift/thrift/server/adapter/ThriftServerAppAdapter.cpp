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

#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerAppAdapter.h>

#include <string>
#include <utility>

#include <folly/logging/xlog.h>

#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/RequestMetadata.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponseError.h>

namespace apache::thrift::fast_thrift::thrift {

ThriftServerAppAdapter::~ThriftServerAppAdapter() {
  auto cb = closeCallback_.withWLock([](auto& fn) { return std::move(fn); });
  if (cb) {
    cb();
  }
}

void ThriftServerAppAdapter::setCloseCallback(std::function<void()> cb) {
  closeCallback_.withWLock([&](auto& fn) { fn = std::move(cb); });
}

void ThriftServerAppAdapter::setPipeline(
    channel_pipeline::PipelineImpl* pipeline) noexcept {
  pipeline_ = pipeline;
  evb_ = pipeline_->eventBase();
}

channel_pipeline::Result ThriftServerAppAdapter::onRead(
    channel_pipeline::TypeErasedBox&& msg) noexcept {
  auto request = msg.take<ThriftServerRequestMessage>();
  DCHECK(request.streamId != 0) << "Invalid stream ID";

  if (FOLLY_UNLIKELY(
          request.frame.type() != frame::FrameType::REQUEST_RESPONSE)) {
    XLOG(ERR) << "Unsupported frame type: " << request.frame.typeName();
    return writeFrameworkError(
        request.streamId,
        apache::thrift::ResponseRpcErrorCode::WRONG_RPC_KIND,
        std::string("Unsupported frame type: ") + request.frame.typeName());
  }

  return handleRequestResponse(std::move(request));
}

void ThriftServerAppAdapter::onException(
    folly::exception_wrapper&& e) noexcept {
  XLOG(ERR) << "Pipeline exception: " << e.what();
  auto cb = closeCallback_.withWLock([](auto& fn) { return std::move(fn); });
  if (cb && pipeline_) {
    pipeline_->eventBase()->runInEventBaseThread(std::move(cb));
  }
}

channel_pipeline::Result ThriftServerAppAdapter::writeResponse(
    uint32_t streamId,
    std::unique_ptr<folly::IOBuf> data,
    std::unique_ptr<folly::IOBuf> metadata,
    bool complete) noexcept {
  return fireResponse(
      ThriftServerResponseMessage{
          .payload = ThriftResponsePayload{
              .data = std::move(data),
              .metadata = std::move(metadata),
              .streamId = streamId,
              .complete = complete,
              .next = true}});
}

channel_pipeline::Result ThriftServerAppAdapter::writeError(
    uint32_t streamId,
    std::unique_ptr<folly::IOBuf> errorData,
    apache::thrift::fast_thrift::frame::ErrorCode errorCode) noexcept {
  return fireResponse(
      ThriftServerResponseMessage{
          .payload = ThriftErrorPayload{
              .data = std::move(errorData),
              .metadata = nullptr,
              .streamId = streamId,
              .errorCode = static_cast<uint32_t>(errorCode)}});
}

void ThriftServerAppAdapter::addMethodHandler(
    std::string_view name, RequestResponseProcessFn handler) {
  dispatch_[std::string(name)] = handler;
}

channel_pipeline::Result ThriftServerAppAdapter::handleRequestResponse(
    ThriftServerRequestMessage&& request) noexcept {
  apache::thrift::RequestRpcMetadata metadata;
  auto error = deserializeRequestMetadata(request.frame, metadata);
  if (FOLLY_UNLIKELY(!!error)) {
    XLOG(ERR) << "Request metadata deserialization failed: " << error.what();
    return writeFrameworkError(
        request.streamId,
        apache::thrift::ResponseRpcErrorCode::REQUEST_PARSING_FAILURE,
        std::string("Request metadata deserialization failed: ") +
            folly::exceptionStr(error).toStdString());
  }

  std::string_view methodName;
  if (FOLLY_LIKELY(metadata.name().has_value())) {
    methodName = metadata.name()->view();
  }

  // Reject unsupported RPC kinds (streaming, sink, etc.)
  auto kindRef = metadata.kind();
  if (FOLLY_UNLIKELY(
          !kindRef.has_value() ||
          *kindRef !=
              apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE)) {
    XLOG(ERR) << "Unsupported RPC kind: "
              << (kindRef.has_value() ? static_cast<int>(*kindRef) : -1);
    return writeFrameworkError(
        request.streamId,
        apache::thrift::ResponseRpcErrorCode::WRONG_RPC_KIND,
        std::string("Unsupported RPC kind: ") +
            std::to_string(
                kindRef.has_value() ? static_cast<int>(*kindRef) : -1));
  }

  auto it = dispatch_.find(methodName);
  if (FOLLY_LIKELY(it != dispatch_.end())) {
    auto streamId = request.streamId;
    apache::thrift::ProtocolId protocol = metadata.protocol().value_or(0);
    auto data = std::move(request.frame).extractData();
    return it->second(this, streamId, std::move(data), protocol);
  }

  return writeFrameworkError(
      request.streamId,
      apache::thrift::ResponseRpcErrorCode::UNKNOWN_METHOD,
      std::string("Unknown method: ") + std::string(methodName));
}

channel_pipeline::Result ThriftServerAppAdapter::writeUnknownException(
    uint32_t streamId,
    const folly::exception_wrapper& ew,
    apache::thrift::ErrorBlame blame) noexcept {
  auto md = makeAppErrorResponseMetadata(
      ew.class_name().toStdString(), ew.what().toStdString(), blame);
  return writeResponse(
      streamId, /*data=*/nullptr, std::move(md), /*complete=*/true);
}

channel_pipeline::Result ThriftServerAppAdapter::writeFrameworkError(
    uint32_t streamId,
    apache::thrift::ResponseRpcErrorCode code,
    std::string message) noexcept {
  auto err = serializeResponseRpcError(code, std::move(message));
  return writeError(streamId, std::move(err.data), err.errorCode);
}

channel_pipeline::Result ThriftServerAppAdapter::fireResponse(
    ThriftServerResponseMessage&& response) noexcept {
  if (FOLLY_UNLIKELY(!pipeline_)) {
    XLOG(ERR) << "Pipeline not set, cannot send response";
    return channel_pipeline::Result::Error;
  }
  auto result = pipeline_->fireWrite(
      channel_pipeline::erase_and_box(std::move(response)));
  if (FOLLY_UNLIKELY(result == channel_pipeline::Result::Error)) {
    XLOG(ERR) << "Pipeline write failed; closing pipeline";
    pipeline_->close();
  }
  return result;
}

} // namespace apache::thrift::fast_thrift::thrift
