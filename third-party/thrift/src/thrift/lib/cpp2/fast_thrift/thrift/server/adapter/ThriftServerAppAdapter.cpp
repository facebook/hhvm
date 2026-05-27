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

#include <utility>

#include <folly/io/async/AsyncSocketException.h>
#include <folly/logging/xlog.h>

#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Event.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/ServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponsePayloads.h>

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
  pipeline_ = pipeline;
  evb_ = pipeline_->eventBase();
}

void ThriftServerAppAdapter::onEvent(
    const channel_pipeline::TypeErasedBox& evt) noexcept {
  const auto& event = evt.get<ThriftServerEvent>();
  if (event.type != ThriftServerEventType::ConnectionClosed) {
    return;
  }
  fireCloseCallback();
}

void ThriftServerAppAdapter::fireCloseCallback() noexcept {
  auto cb = closeCallback_.withWLock([](auto& fn) { return std::move(fn); });
  if (cb && evb_) {
    evb_->runInEventBaseThread(std::move(cb));
  }
}

channel_pipeline::Result ThriftServerAppAdapter::onRead(
    channel_pipeline::TypeErasedBox&& msg) noexcept {
  auto request = msg.take<ThriftServerRequestMessage>();
  DCHECK(request.streamId != 0) << "Invalid stream ID";
  return handleRequestResponse(std::move(request));
}

void ThriftServerAppAdapter::onException(
    folly::exception_wrapper&& e) noexcept {
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
  return writeResponse(makeWrongRpcKindMessage(streamId, kind));
}

channel_pipeline::Result ThriftServerAppAdapter::handleUnknownMethod(
    uint32_t streamId, std::string_view methodName) noexcept {
  return writeResponse(makeUnknownMethodMessage(streamId, methodName));
}

channel_pipeline::Result ThriftServerAppAdapter::writeResponse(
    ThriftServerResponseMessage&& message) noexcept {
  if (FOLLY_UNLIKELY(!pipeline_)) {
    return handleMissingPipeline();
  }
  return pipeline_->fireWrite(
      channel_pipeline::erase_and_box(std::move(message)));
}

channel_pipeline::Result
ThriftServerAppAdapter::handleMissingPipeline() noexcept {
  XLOG(ERR) << "Pipeline not set, cannot send response";
  return channel_pipeline::Result::Error;
}

void ThriftServerAppAdapter::close() noexcept {
  if (!pipeline_) {
    return;
  }
  pipeline_->fireEvent(
      channel_pipeline::erase_and_box(
          ThriftServerEvent{ThriftServerEventType::CloseConnection}));
}

} // namespace apache::thrift::fast_thrift::thrift
