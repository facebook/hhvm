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
#include <utility>

#include <folly/logging/xlog.h>

#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift {

void ThriftServerCompositeAppAdapter::addChild(
    ThriftServerAppAdapter::Ptr child) {
  CHECK(child) << "ThriftServerCompositeAppAdapter::addChild: null child";

  // First-wins: try_emplace keeps the existing entry, so methods registered
  // by an earlier child shadow same-named methods on a later child.
  auto* raw = child.get();
  for (auto name : child->methodNames()) {
    auto [_, inserted] = methodMap_.try_emplace(std::string(name), raw);
    if (!inserted) {
      XLOG(WARN) << "ThriftServerCompositeAppAdapter: method '" << name
                 << "' already claimed by an earlier child; dropping from new "
                    "child (index "
                 << children_.size() << ")";
    }
  }

  children_.push_back(std::move(child));
}

channel_pipeline::Result ThriftServerCompositeAppAdapter::onRead(
    channel_pipeline::TypeErasedBox&& msg) noexcept {
  // Peek (not take) so we can forward the original box to the chosen child
  // unchanged — child does its own take<> on the same message.
  const auto& request = msg.get<ThriftServerRequestMessage>();
  DCHECK(request.streamId != 0) << "Invalid stream ID";

  // Routing only depends on the request's method name, which every
  // initial-request payload alternative exposes via getRequestRpcMetadata
  // (the accessor is constrained on ThriftRequestPayloadConcept, so adding
  // stream/sink/fnf/bidi init payloads to the inbound variant Just Works
  // here). Continuation frames (RequestN, Cancel, sink chunks) never reach
  // the composite — the rocket pipeline tracks per-stream subscriptions
  // below us and only initial-request messages bubble up.
  const auto* metadata = request.payload.getRequestRpcMetadata();
  DCHECK(metadata != nullptr);

  std::string_view methodName;
  if (FOLLY_LIKELY(metadata->name().has_value())) {
    methodName = metadata->name()->view();
  }

  auto it = methodMap_.find(methodName);
  if (FOLLY_UNLIKELY(it == methodMap_.end())) {
    return writeFrameworkError(
        request.streamId,
        apache::thrift::ResponseRpcErrorCode::UNKNOWN_METHOD,
        std::string("Unknown method: ") + std::string(methodName));
  }

  return it->second->onRead(std::move(msg));
}

void ThriftServerCompositeAppAdapter::setPipeline(
    channel_pipeline::PipelineImpl* pipeline) noexcept {
  ThriftServerAppAdapter::setPipeline(pipeline);
  for (auto& child : children_) {
    child->setPipeline(pipeline);
  }
}

void ThriftServerCompositeAppAdapter::onPipelineActive() noexcept {
  ThriftServerAppAdapter::onPipelineActive();
  for (auto& child : children_) {
    child->onPipelineActive();
  }
}

void ThriftServerCompositeAppAdapter::onPipelineInactive() noexcept {
  ThriftServerAppAdapter::onPipelineInactive();
  for (auto& child : children_) {
    child->onPipelineInactive();
  }
}

void ThriftServerCompositeAppAdapter::onException(
    folly::exception_wrapper&& e) noexcept {
  ThriftServerAppAdapter::onException(folly::exception_wrapper{e});
  for (auto& child : children_) {
    child->onException(folly::exception_wrapper{e});
  }
}

} // namespace apache::thrift::fast_thrift::thrift
