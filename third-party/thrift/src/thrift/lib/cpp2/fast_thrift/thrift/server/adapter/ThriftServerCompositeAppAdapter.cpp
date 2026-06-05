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

#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerCompositeAppAdapter.h>

#include <string>
#include <string_view>
#include <utility>

#include <folly/logging/xlog.h>

#include <thrift/lib/cpp2/fast_thrift/thrift/common/ThriftRequestPayloads.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/common/ThriftResponsePayloads.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Event.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponsePayloads.h>

namespace apache::thrift::fast_thrift::thrift {

ThriftServerCompositeAppAdapter::~ThriftServerCompositeAppAdapter() {
  resetPipeline();
  // Fallback: if onPipelineInactive never ran (e.g. composite destroyed
  // without prior pipeline tear-down), fire the close callback synchronously.
  auto cb = closeCallback_.withWLock([](auto& fn) { return std::move(fn); });
  if (cb) {
    cb();
  }
}

void ThriftServerCompositeAppAdapter::setCloseCallback(
    std::function<void()> cb) {
  closeCallback_.withWLock([&](auto& fn) { fn = std::move(cb); });
}

channel_pipeline::Result ThriftServerCompositeAppAdapter::onRead(
    channel_pipeline::TypeErasedBox&& msg) noexcept {
  // Peek (not take) so we can forward the original box to the chosen child
  // unchanged — child does its own take<> on the same message.
  const auto& request = msg.get<ThriftServerRequestMessage>();
  DCHECK(request.streamId != 0) << "Invalid stream ID";

  const auto* metadata = request.payload.getRequestRpcMetadata();
  DCHECK(metadata != nullptr);

  std::string_view methodName;
  if (FOLLY_LIKELY(metadata->name().has_value())) {
    methodName = metadata->name()->view();
  }

  auto it = methodMap_.find(methodName);
  if (FOLLY_UNLIKELY(it == methodMap_.end())) {
    return writeUnknownMethodError(request.streamId, methodName);
  }
  return it->second.invoke(it->second.owner, std::move(msg));
}

void ThriftServerCompositeAppAdapter::onException(
    folly::exception_wrapper&& e) noexcept {
  // Tail-of-pipeline contract: any exception reaching here is unhandled.
  // Tear down the pipeline immediately (no graceful drain). Children
  // also get the notification for any per-child cleanup, but the
  // pipeline teardown is owned here; their own onException → deactivate
  // calls are absorbed by PipelineImpl::deactivate's idempotency.
  XLOG_EVERY_MS(ERR, 60'000) << "Pipeline exception: " << e.what();
  for (auto& child : children_) {
    child.vtable->onException(child.owner, folly::exception_wrapper{e});
  }
  if (pipeline_) {
    pipeline_->deactivate();
  }
}

void ThriftServerCompositeAppAdapter::setPipeline(
    channel_pipeline::PipelineImpl* pipeline) noexcept {
  pipeline_ = pipeline;
  pipelineGuard_ =
      std::make_unique<folly::DelayedDestruction::DestructorGuard>(pipeline_);
  for (auto& child : children_) {
    child.vtable->setPipeline(child.owner, pipeline);
  }
}

void ThriftServerCompositeAppAdapter::resetPipeline() noexcept {
  folly::DelayedDestruction::DestructorGuard dg(this);
  // Mirror setPipeline's fan-out: each child holds its own pipelineGuard_
  // taken when we forwarded setPipeline to them. Release theirs first so
  // their later dtors don't fire pipeline-dtor cascades.
  for (auto& child : children_) {
    child.vtable->resetPipeline(child.owner);
  }
  pipeline_ = nullptr;
  pipelineGuard_.reset();
}

void ThriftServerCompositeAppAdapter::handlerAdded() noexcept {
  for (auto& child : children_) {
    child.vtable->handlerAdded(child.owner);
  }
}

void ThriftServerCompositeAppAdapter::handlerRemoved() noexcept {
  for (auto& child : children_) {
    child.vtable->handlerRemoved(child.owner);
  }
}

void ThriftServerCompositeAppAdapter::onPipelineActive() noexcept {
  for (auto& child : children_) {
    child.vtable->onPipelineActive(child.owner);
  }
}

void ThriftServerCompositeAppAdapter::onPipelineInactive() noexcept {
  // Fan out to children only. The user closeCallback is NOT fired here
  // — that fires on the ConnectionClosed broadcast from the
  // ThriftServerConnectionCloseHandler, which is the edge that guarantees
  // in-flight handler callbacks have settled (or LOG(FATAL)'d on reap timeout).
  for (auto& child : children_) {
    child.vtable->onPipelineInactive(child.owner);
  }
}

void ThriftServerCompositeAppAdapter::onEvent(
    ThriftServerEventType ev,
    const channel_pipeline::TypeErasedBox& /*evt*/) noexcept {
  // We subscribe only to ConnectionClosed, so that's the only event delivered.
  DCHECK(ev == ThriftServerEventType::ConnectionClosed);
  // Deferred onto the EventBase because the callback's typical action
  // is to destroy *us*, and we're called from inside a pipeline
  // event-dispatch walk. Skip the move-out when pipeline_ is null so
  // the dtor fallback still has the callback to fire.
  if (!pipeline_) {
    return;
  }
  auto cb = closeCallback_.withWLock([](auto& fn) { return std::move(fn); });
  if (cb) {
    pipeline_->eventBase()->runInEventBaseThread(std::move(cb));
  }
}

void ThriftServerCompositeAppAdapter::onWriteReady() noexcept {
  for (auto& child : children_) {
    child.vtable->onWriteReady(child.owner);
  }
}

void ThriftServerCompositeAppAdapter::warnDuplicateMethod(
    std::string_view name) const {
  XLOG(WARN) << "ThriftServerCompositeAppAdapter: method '" << name
             << "' already claimed by an earlier child; dropping from new "
                "child (index "
             << children_.size() << ")";
}

channel_pipeline::Result
ThriftServerCompositeAppAdapter::writeUnknownMethodError(
    uint32_t streamId, std::string_view methodName) noexcept {
  if (FOLLY_UNLIKELY(!pipeline_)) {
    XLOG(ERR) << "Pipeline not set, cannot send unknown-method error";
    return channel_pipeline::Result::Error;
  }
  auto result = pipeline_->fireWrite(
      channel_pipeline::erase_and_box(
          makeUnknownMethodMessage(streamId, methodName)));
  // Failed write means pipeline is broken — tear down immediately.
  // Idempotent at the pipeline level.
  if (FOLLY_UNLIKELY(result == channel_pipeline::Result::Error)) {
    pipeline_->deactivate();
  }
  return result;
}

void ThriftServerCompositeAppAdapter::close() noexcept {
  if (FOLLY_UNLIKELY(!pipeline_)) {
    return;
  }
  pipeline_->fireEvent(
      ThriftServerEventType::CloseConnection,
      channel_pipeline::TypeErasedBox{});
}

} // namespace apache::thrift::fast_thrift::thrift
