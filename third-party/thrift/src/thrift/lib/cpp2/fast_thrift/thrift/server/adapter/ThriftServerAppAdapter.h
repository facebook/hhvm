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

#include <functional>
#include <optional>

#include <folly/ExceptionWrapper.h>
#include <folly/Synchronized.h>
#include <folly/container/F14Map.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/logging/xlog.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/RequestMetadata.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponseError.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponseMetadata.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponseSerializer.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * ThriftServerAppAdapter — base class for generated fast server handlers.
 *
 * The generated FastHandler extends this class, inheriting pipeline
 * integration, metadata deserialization, and response writing.
 *
 * Satisfies TailEndpointHandler concept (onRead / onException).
 */
class ThriftServerAppAdapter : public folly::DelayedDestruction {
 public:
  using ProcessFn = channel_pipeline::Result (*)(
      ThriftServerAppAdapter*,
      ThriftServerRequestMessage&&,
      apache::thrift::ProtocolId) noexcept;

  ThriftServerAppAdapter() = default;
  ThriftServerAppAdapter(ThriftServerAppAdapter&&) = delete;
  ThriftServerAppAdapter& operator=(ThriftServerAppAdapter&&) = delete;

  void setCloseCallback(std::function<void()> cb) {
    closeCallback_.withWLock([&](auto& fn) { fn = std::move(cb); });
  }

  void setPipeline(channel_pipeline::PipelineImpl* pipeline) noexcept {
    pipeline_ = pipeline;
    evb_ = pipeline_->eventBase();
  }

  channel_pipeline::PipelineImpl* pipeline() const noexcept {
    return pipeline_;
  }

  // === TailEndpointHandler interface ===

  channel_pipeline::Result onRead(
      channel_pipeline::TypeErasedBox&& msg) noexcept {
    auto request = msg.take<ThriftServerRequestMessage>();
    DCHECK(request.streamId != 0) << "Invalid stream ID";

    if (FOLLY_UNLIKELY(
            request.frame.type() != frame::FrameType::REQUEST_RESPONSE)) {
      XLOG(ERR) << "Unsupported frame type: " << request.frame.typeName();
      writeThriftError(
          request.streamId,
          apache::thrift::ResponseRpcErrorCode::WRONG_RPC_KIND,
          std::string("Unsupported frame type: ") + request.frame.typeName());
      return channel_pipeline::Result::Success;
    }

    return handleRequestResponse(std::move(request));
  }

  void onException(folly::exception_wrapper&& e) noexcept {
    XLOG(ERR) << "Pipeline exception: " << e.what();
    auto cb = closeCallback_.withWLock([](auto& fn) { return std::move(fn); });
    if (cb && pipeline_) {
      pipeline_->eventBase()->runInEventBaseThread(std::move(cb));
    }
  }

  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}
  void onWriteReady() noexcept {}

  void writeResponse(ThriftServerResponseMessage&& response) noexcept {
    if (FOLLY_UNLIKELY(!pipeline_)) {
      XLOG(ERR) << "Pipeline not set, cannot send response";
      return;
    }
    auto result = pipeline_->fireWrite(
        channel_pipeline::erase_and_box(std::move(response)));
    if (FOLLY_UNLIKELY(result != channel_pipeline::Result::Success)) {
      XLOG(WARN) << "Pipeline write failed!";
    }
  }

  void writeThriftError(
      uint32_t streamId,
      apache::thrift::ResponseRpcErrorCode errorCode,
      std::string errorMessage) noexcept {
    auto error = serializeResponseRpcError(errorCode, std::move(errorMessage));
    writeResponse(
        ThriftServerResponseMessage{
            .payload =
                ThriftServerResponsePayload{
                    .data = std::move(error.data),
                    .metadata = nullptr,
                    .complete = true},
            .streamId = streamId,
            .errorCode = static_cast<uint32_t>(error.rocketErrorCode)});
  }

  void writeAppError(
      uint32_t streamId,
      std::string exName,
      std::string errorMessage,
      apache::thrift::ErrorBlame blame) noexcept {
    writeResponse(
        ThriftServerResponseMessage{
            .payload =
                ThriftServerResponsePayload{
                    .data = nullptr,
                    .metadata = makeAppErrorResponseMetadata(
                        std::move(exName), std::move(errorMessage), blame),
                    .complete = true},
            .streamId = streamId,
            .errorCode = 0});
  }

  // Send a declared exception (IDL `throws`) as a PAYLOAD frame.
  // The exception struct is carried in `exceptionData` (caller-serialized),
  // analogous to how a successful response carries the result struct.
  void writeResponseError(
      uint32_t streamId,
      std::unique_ptr<folly::IOBuf> exceptionData,
      std::string exName,
      std::string exWhat,
      std::optional<apache::thrift::ErrorClassification>
          classification) noexcept {
    writeResponse(
        ThriftServerResponseMessage{
            .payload =
                ThriftServerResponsePayload{
                    .data = std::move(exceptionData),
                    .metadata = makeDeclaredExceptionMetadata(
                        std::move(exName), std::move(exWhat), classification),
                    .complete = true},
            .streamId = streamId,
            .errorCode = 0});
  }

 protected:
  void addMethodHandler(std::string_view name, ProcessFn handler) {
    dispatch_[std::string(name)] = handler;
  }

  channel_pipeline::Result handleRequestResponse(
      ThriftServerRequestMessage&& request) noexcept {
    apache::thrift::RequestRpcMetadata metadata;
    auto error = deserializeRequestMetadata(request.frame, metadata);
    if (FOLLY_UNLIKELY(error)) {
      XLOG(ERR) << "Request metadata deserialization failed: " << error.what();
      writeThriftError(
          request.streamId,
          apache::thrift::ResponseRpcErrorCode::REQUEST_PARSING_FAILURE,
          std::string("Request metadata deserialization failed: ") +
              folly::exceptionStr(error).toStdString());
      return channel_pipeline::Result::Success;
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
      writeThriftError(
          request.streamId,
          apache::thrift::ResponseRpcErrorCode::WRONG_RPC_KIND,
          std::string("Unsupported RPC kind: ") +
              std::to_string(
                  kindRef.has_value() ? static_cast<int>(*kindRef) : -1));
      return channel_pipeline::Result::Success;
    }

    auto it = dispatch_.find(methodName);
    if (FOLLY_LIKELY(it != dispatch_.end())) {
      return it->second(
          this, std::move(request), metadata.protocol().value_or(0));
    }

    writeThriftError(
        request.streamId,
        apache::thrift::ResponseRpcErrorCode::UNKNOWN_METHOD,
        std::string("Unknown method: ") + std::string(methodName));
    return channel_pipeline::Result::Success;
  }

  channel_pipeline::PipelineImpl* pipeline_{nullptr};

  ~ThriftServerAppAdapter() override {
    auto cb = closeCallback_.withWLock([](auto& fn) { return std::move(fn); });
    if (cb) {
      cb();
    }
  }

  folly::EventBase* getEventBase() const { return evb_; }

 private:
  folly::EventBase* evb_{nullptr};
  folly::F14FastMap<std::string, ProcessFn> dispatch_;
  folly::Synchronized<std::function<void()>> closeCallback_;
};

} // namespace apache::thrift::fast_thrift::thrift
