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

#include <memory>
#include <string>
#include <vector>

#include <folly/container/F14Map.h>
#include <folly/io/async/DelayedDestruction.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/MetadataProtocol.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerAppAdapter.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * ThriftServerCompositeAppAdapter — pipeline tail that fans inbound requests
 * across an ordered list of ThriftServerAppAdapter children, dispatched by
 * method name. Mirrors Java's CompositeRpcServerHandler structurally.
 *
 * Routing is method-name only. addChild populates a flat name -> child map
 * (first-wins on duplicates, matching MultiplexAsyncProcessorFactory and
 * intentionally different from Java which throws). On each inbound message
 * the composite peeks the request metadata, looks up the owning child, and
 * forwards the box to that child's onRead unchanged.
 *
 * No streamId routing is needed: aux interfaces (Status / Monitoring /
 * Control / Security) are restricted by their marker base classes to
 * req-resp + oneway, and the rocket layer below the composite tracks
 * per-stream subscriptions on its own — RequestN, Cancel, sink chunks, etc.
 * never bubble up here.
 *
 * Pipeline integration: onRead and setPipeline shadow the base's
 * non-virtual methods. Callers must hold the composite by its typed pointer
 * (not as ThriftServerAppAdapter*) at the pipeline-wiring sites so the
 * shadowing setter resolves; the pipeline itself must also be templated on
 * this type for onRead to fire. FastThriftServer handles both when any
 * auxiliary handler is registered.
 */
class ThriftServerCompositeAppAdapter : public ThriftServerAppAdapter {
 public:
  // Mirrors the base's `Ptr` convention so the composite can be held in any
  // slot typed as ThriftServerAppAdapter::Ptr.
  using Ptr = std::unique_ptr<
      ThriftServerCompositeAppAdapter,
      folly::DelayedDestruction::Destructor>;

  ThriftServerCompositeAppAdapter() = default;

  // Append a child and merge its method names into the routing map.
  // Duplicate names are dropped with a WARN — first writer wins. All
  // children must be added before the composite is wired into a pipeline.
  void addChild(ThriftServerAppAdapter::Ptr child);

  // === TailEndpointHandler interface ===
  channel_pipeline::Result onRead(
      channel_pipeline::TypeErasedBox&& msg) noexcept;

  // Shadows base. Forwards to every child so they write responses through
  // the same pipeline.
  void setPipeline(channel_pipeline::PipelineImpl* pipeline) noexcept;

  // Shadow base lifecycle hooks so the composite drives all children
  // through the same state transitions. Children share the pipeline; their
  // gates (onRead reject when not Open, writeResponse reject when Closed)
  // need consistent state with the composite.
  void onPipelineActive() noexcept;
  void onPipelineInactive() noexcept;
  void onException(folly::exception_wrapper&& e) noexcept;

 private:
  std::vector<ThriftServerAppAdapter::Ptr> children_;
  folly::F14FastMap<std::string, ThriftServerAppAdapter*> methodMap_;
};

} // namespace apache::thrift::fast_thrift::thrift
