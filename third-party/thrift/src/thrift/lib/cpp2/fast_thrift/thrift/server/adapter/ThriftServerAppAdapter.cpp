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

#include <folly/logging/xlog.h>

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
  resetPipeline();
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
  pipelineGuard_ =
      std::make_unique<folly::DelayedDestruction::DestructorGuard>(pipeline_);
  pipelineActive_ = true;
  evb_ = folly::getKeepAliveToken(pipeline_->eventBase());
}

void ThriftServerAppAdapter::resetPipeline() noexcept {
  folly::DelayedDestruction::DestructorGuard dg(this);
  pipelineActive_ = false;
  pipeline_ = nullptr;
  pipelineGuard_.reset();
  evb_ = {};
}

void ThriftServerAppAdapter::onEvent(
    ThriftServerEventType ev,
    const channel_pipeline::TypeErasedBox& /*evt*/) noexcept {
  // We subscribe only to ConnectionClosed, so that's the only event delivered.
  DCHECK(ev == ThriftServerEventType::ConnectionClosed);
  // Detach from pipeline. Stragglers (FastHandlerCallbacks still
  // alive, each holding a DG on us) call writeResponse, hit
  // pipelineActive_==false in writeResponseOnEventBase, and drop
  // silently. The pipeline is free to die — its DG count from us
  // just dropped.
  pipelineActive_ = false;
  pipelineGuard_.reset();
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
  handleRequestResponse(std::move(request));
  return channel_pipeline::Result::Success;
}

void ThriftServerAppAdapter::onException(
    folly::exception_wrapper&& e) noexcept {
  // Tail-of-pipeline contract: any exception reaching here is unhandled.
  // The pipeline is in a broken state — no point waiting for a graceful
  // drain (close() would). Tear it down immediately. PipelineImpl's
  // deactivate() is idempotent (state-gated), so racing with a
  // transport-initiated deactivate is safe — only the first call
  // cascades; the close handler's onPipelineInactive does the rest.
  XLOG_EVERY_MS(ERR, 60'000) << "Pipeline exception: " << e.what();
  if (pipeline_) {
    pipeline_->deactivate();
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

void ThriftServerAppAdapter::handleRequestResponse(
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
    handleWrongRpcKind(request.streamId, kind);
    return;
  }

  auto it = dispatch_.find(methodName);
  if (FOLLY_LIKELY(it != dispatch_.end())) {
    apache::thrift::ProtocolId protocol = metadata.protocol().value_or(0);
    it->second(
        this,
        request.streamId,
        std::move(rr.data),
        protocol,
        std::move(request.requestContext));
    return;
  }

  handleUnknownMethod(request.streamId, methodName);
}

void ThriftServerAppAdapter::handleWrongRpcKind(
    uint32_t streamId, apache::thrift::RpcKind kind) noexcept {
  XLOG(ERR) << "Unsupported RPC kind: " << static_cast<int>(kind);
  writeResponseOnEventBase(makeWrongRpcKindMessage(streamId, kind));
}

void ThriftServerAppAdapter::handleUnknownMethod(
    uint32_t streamId, std::string_view methodName) noexcept {
  writeResponseOnEventBase(makeUnknownMethodMessage(streamId, methodName));
}

void ThriftServerAppAdapter::writeResponse(
    ThriftServerResponseMessage&& message) noexcept {
  if (evb_->isInEventBaseThread()) {
    writeResponseOnEventBase(std::move(message));
  } else {
    evb_->runInEventBaseThread(
        [this,
         dg = folly::DelayedDestruction::DestructorGuard(this),
         message = std::move(message)]() mutable {
          writeResponseOnEventBase(std::move(message));
        });
  }
}

void ThriftServerAppAdapter::writeResponseOnEventBase(
    ThriftServerResponseMessage&& message) noexcept {
  // Straggler after force-close (or any post-ConnectionClosed write):
  // the pipeline may already be gone. Drop silently — pipeline_ may
  // be dangling, do NOT dereference.
  if (FOLLY_UNLIKELY(!pipelineActive_)) {
    return;
  }
  if (FOLLY_UNLIKELY(!pipeline_)) {
    handleMissingPipeline();
    return;
  }
  auto result =
      pipeline_->fireWrite(channel_pipeline::erase_and_box(std::move(message)));
  // A failed write means the response can't reach the wire — pipeline
  // is in a broken or torn-down state. Tear down immediately rather
  // than wait for a graceful drain. Idempotent at the pipeline level.
  if (FOLLY_UNLIKELY(result == channel_pipeline::Result::Error)) {
    pipeline_->deactivate();
  }
}

void ThriftServerAppAdapter::handleMissingPipeline() noexcept {
  XLOG(ERR) << "Pipeline not set, cannot send response";
}

void ThriftServerAppAdapter::close() noexcept {
  if (!pipeline_) {
    return;
  }
  pipeline_->fireEvent(
      ThriftServerEventType::CloseConnection,
      channel_pipeline::TypeErasedBox{});
}

} // namespace apache::thrift::fast_thrift::thrift
