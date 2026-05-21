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

#include <folly/io/async/AsyncSocketException.h>
#include <folly/logging/xlog.h>

#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/ServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponseError.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponseMetadata.h>

namespace apache::thrift::fast_thrift::thrift {

static_assert(
    ServerInboundAppAdapter<ThriftServerAppAdapter>,
    "ThriftServerAppAdapter must satisfy ServerInboundAppAdapter");
static_assert(
    ServerOutboundAppAdapter<ThriftServerAppAdapter>,
    "ThriftServerAppAdapter must satisfy ServerOutboundAppAdapter");
static_assert(
    ServerComposableAppAdapter<ThriftServerAppAdapter>,
    "ThriftServerAppAdapter must satisfy ServerComposableAppAdapter");

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
  DCHECK(pipeline);
  DCHECK(state_ == State::Created);
  pipeline_ = pipeline;
  evb_ = pipeline_->eventBase();
  state_ = State::Ready;
}

void ThriftServerAppAdapter::onPipelineActive() noexcept {
  DCHECK(state_ == State::Ready);
  state_ = State::Open;
}

void ThriftServerAppAdapter::onPipelineInactive() noexcept {
  // Pipeline disconnect is the canonical "no longer accepting writes" edge.
  // Whether we got here via graceful close or hard error, we're done.
  state_ = State::Closed;
}

channel_pipeline::Result ThriftServerAppAdapter::onRead(
    channel_pipeline::TypeErasedBox&& msg) noexcept {
  if (FOLLY_UNLIKELY(state_ != State::Open)) {
    return channel_pipeline::Result::Error;
  }
  auto request = msg.take<ThriftServerRequestMessage>();
  DCHECK(request.streamId != 0) << "Invalid stream ID";
  return handleRequestResponse(std::move(request));
}

void ThriftServerAppAdapter::onException(
    folly::exception_wrapper&& e) noexcept {
  // Routine connection close (peer FIN/RST, idle timeout, server-initiated
  // drop) propagates here as an exception. Match legacy thrift's
  // RocketServerConnection: TTransportException(END_OF_FILE/NOT_OPEN/...) is
  // the signal to start a graceful drain — let in-flight handler callbacks
  // try to write final responses; refuse new requests. Real protocol /
  // server faults still log and skip the drain (go straight to Closed).
  using TX = apache::thrift::transport::TTransportException;
  bool benign = false;
  e.with_exception([&](const TX& ex) {
    benign = ex.getType() == TX::END_OF_FILE || ex.getType() == TX::NOT_OPEN ||
        ex.getType() == TX::INTERRUPTED || ex.getType() == TX::TIMED_OUT;
  });
  e.with_exception([&](const folly::AsyncSocketException& ex) {
    benign = benign || ex.getType() == folly::AsyncSocketException::END_OF_FILE;
  });
  if (!benign) {
    XLOG(ERR) << "Pipeline exception: " << e.what();
  }

  if (state_ == State::Open) {
    state_ = benign ? State::Closing : State::Closed;
  }

  auto cb = closeCallback_.withWLock([](auto& fn) { return std::move(fn); });
  if (cb && pipeline_) {
    pipeline_->eventBase()->runInEventBaseThread(std::move(cb));
  }
}

channel_pipeline::Result ThriftServerAppAdapter::writeResponse(
    uint32_t streamId,
    std::unique_ptr<folly::IOBuf> data,
    std::unique_ptr<apache::thrift::ResponseRpcMetadata> metadata) noexcept {
  return fireResponse(
      ThriftServerResponseMessage{
          .payload = ThriftInitialResponsePayload{
              .data = std::move(data),
              .metadata = std::move(metadata),
              .streamId = streamId,
          }});
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

std::vector<std::string_view> ThriftServerAppAdapter::methodNames()
    const noexcept {
  std::vector<std::string_view> names;
  names.reserve(dispatch_.size());
  for (const auto& [name, _] : dispatch_) {
    names.push_back(name);
  }
  return names;
}

channel_pipeline::Result ThriftServerAppAdapter::handleRequestResponse(
    ThriftServerRequestMessage&& request) noexcept {
  auto& inbound = request.payload;
  DCHECK(inbound.is<ThriftRequestResponsePayload>());
  auto& rr = inbound.get<ThriftRequestResponsePayload>();
  DCHECK(rr.metadata != nullptr);
  auto& metadata = *rr.metadata;

  std::string_view methodName;
  if (FOLLY_LIKELY(metadata.name().has_value())) {
    methodName = metadata.name()->view();
  }

  const auto kind =
      metadata.kind().value_or(static_cast<apache::thrift::RpcKind>(-1));
  if (FOLLY_UNLIKELY(
          kind != apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE)) {
    return handleWrongRpcKind(request.streamId, kind);
  }

  auto it = dispatch_.find(methodName);
  if (FOLLY_LIKELY(it != dispatch_.end())) {
    apache::thrift::ProtocolId protocol = metadata.protocol().value_or(0);
    return it->second(
        this,
        request.streamId,
        std::move(rr.data),
        protocol,
        std::move(request.requestContext));
  }

  return handleUnknownMethod(request.streamId, methodName);
}

channel_pipeline::Result ThriftServerAppAdapter::handleWrongRpcKind(
    uint32_t streamId, apache::thrift::RpcKind kind) noexcept {
  XLOG(ERR) << "Unsupported RPC kind: " << static_cast<int>(kind);
  return writeFrameworkError(
      streamId,
      apache::thrift::ResponseRpcErrorCode::WRONG_RPC_KIND,
      std::string("Unsupported RPC kind: ") +
          std::to_string(static_cast<int>(kind)));
}

channel_pipeline::Result ThriftServerAppAdapter::handleUnknownMethod(
    uint32_t streamId, std::string_view methodName) noexcept {
  return writeFrameworkError(
      streamId,
      apache::thrift::ResponseRpcErrorCode::UNKNOWN_METHOD,
      std::string("Unknown method: ") + std::string(methodName));
}

channel_pipeline::Result ThriftServerAppAdapter::writeUnknownException(
    uint32_t streamId,
    const folly::exception_wrapper& ew,
    apache::thrift::ErrorBlame blame) noexcept {
  auto md = std::make_unique<apache::thrift::ResponseRpcMetadata>();
  fillAppErrorResponseMetadata(
      *md, ew.class_name().toStdString(), ew.what().toStdString(), blame);
  return writeResponse(streamId, /*data=*/nullptr, std::move(md));
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
  // Refuse writes once we're Closed. Open and Closing both still try — in
  // Closing the pipeline may swallow the write if the wire is gone, but
  // an in-flight handler callback completing during graceful drain should
  // be allowed to push a final response toward the wire.
  if (FOLLY_UNLIKELY(state_ == State::Closed)) {
    return channel_pipeline::Result::Error;
  }
  if (FOLLY_UNLIKELY(!pipeline_)) {
    return handleMissingPipeline();
  }
  return pipeline_->fireWrite(
      channel_pipeline::erase_and_box(std::move(response)));
}

channel_pipeline::Result
ThriftServerAppAdapter::handleMissingPipeline() noexcept {
  XLOG(ERR) << "Pipeline not set, cannot send response";
  return channel_pipeline::Result::Error;
}

} // namespace apache::thrift::fast_thrift::thrift
