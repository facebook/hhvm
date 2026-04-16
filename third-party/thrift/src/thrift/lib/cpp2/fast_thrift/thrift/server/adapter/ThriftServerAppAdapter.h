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

#include <folly/ExceptionWrapper.h>
#include <folly/Synchronized.h>
#include <folly/container/F14Map.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/logging/xlog.h>

#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/RequestMetadata.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponseSerializer.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * ThriftServerAppAdapter — base class for generated fast server handlers.
 *
 * The generated FastHandler extends this class, inheriting pipeline
 * integration, metadata deserialization, and response writing.
 *
 * Satisfies ServerInboundAppAdapter concept (onMessage / onException).
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

  // === ServerInboundAppAdapter interface ===

  channel_pipeline::Result onMessage(
      channel_pipeline::TypeErasedBox&& msg) noexcept {
    auto request = msg.take<ThriftServerRequestMessage>();
    DCHECK(request.streamId != 0) << "Invalid stream ID";

    if (FOLLY_UNLIKELY(
            request.frame.type() != frame::FrameType::REQUEST_RESPONSE)) {
      XLOG(ERR) << "Unsupported frame type: " << request.frame.typeName();
      writeResponse(buildErrorResponse(
          request.streamId,
          std::string("Unsupported frame type: ") + request.frame.typeName()));
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

  template <typename ProtocolWriter>
  void writeErr(uint32_t streamId, folly::exception_wrapper&& ew) noexcept {
    if (FOLLY_UNLIKELY(!pipeline_)) {
      XLOG(ERR) << "Pipeline not set, cannot send error";
      return;
    }

    std::string errMsg = ew.what().toStdString();
    apache::thrift::TApplicationException tae(
        apache::thrift::TApplicationException::INTERNAL_ERROR, errMsg);

    auto metadata = makeErrorResponseMetadata(std::move(errMsg));
    auto data = serializeErrorStruct<ProtocolWriter>(tae);
    writeResponse(
        ThriftServerResponseMessage{
            .payload =
                ThriftServerResponsePayload{
                    .data = std::move(data),
                    .metadata = std::move(metadata),
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
      writeResponse(buildErrorResponse(
          request.streamId,
          std::string("Request metadata deserialization failed: ") +
              folly::exceptionStr(error).toStdString()));
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
      writeResponse(buildErrorResponse(
          request.streamId,
          std::string("Unsupported RPC kind: ") +
              std::to_string(
                  kindRef.has_value() ? static_cast<int>(*kindRef) : -1)));
      return channel_pipeline::Result::Success;
    }

    auto it = dispatch_.find(methodName);
    if (FOLLY_LIKELY(it != dispatch_.end())) {
      return it->second(
          this, std::move(request), metadata.protocol().value_or(0));
    }

    writeResponse(buildErrorResponse(
        request.streamId,
        std::string("Unknown method: ") + std::string(methodName)));
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
  template <typename ProtocolWriter>
  std::unique_ptr<folly::IOBuf> serializeErrorStruct(
      const TApplicationException& tae) {
    ProtocolWriter prot;
    size_t bufSize = tae.serializedSizeZC(&prot);
    folly::IOBufQueue queue;
    prot.setOutput(&queue, bufSize);
    tae.write(&prot);
    return queue.move();
  }

  folly::EventBase* evb_{nullptr};
  folly::F14FastMap<std::string, ProcessFn> dispatch_;
  folly::Synchronized<std::function<void()>> closeCallback_;
};

} // namespace apache::thrift::fast_thrift::thrift
