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
#include <memory>
#include <utility>
#include <variant>
#include <vector>

#include <boost/intrusive_ptr.hpp>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerCompositeAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerTransportAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/context/ThriftConnContext.h>

namespace apache::thrift::fast_thrift::thrift::server {

/**
 * ThriftServerConnection — full server-side connection state for a single
 * accepted client. Owns both the thrift pipeline and (transitively, via
 * the ThriftServerTransportAdapter) the underlying rocket pipeline and
 * transport.
 *
 *   thrift pipeline:
 *     ThriftServerTransportAdapter → [thrift handlers] → tail adapter
 *   rocket pipeline (owned by ThriftServerTransportAdapter):
 *     TransportHandler → [rocket handlers] → RocketServerAppAdapter
 *
 * Non-templated: the variation in tail shape (a single user adapter vs a
 * composite fronting user + monitoring/status/debug/metadata) is hidden
 * inside `tail` as a variant.
 */
struct ThriftServerConnection {
  /**
   * Tail when the pipeline serves a single user adapter directly.
   */
  struct SimpleTail {
    ThriftServerAppAdapter::Ptr adapter;
  };

  /**
   * Tail when the pipeline fans out to a composite that routes by method
   * name to the user adapter plus monitoring/status/debug/metadata
   * children. The composite borrows raw T* into its children, so children
   * must outlive the composite — declaration order (children first) makes
   * this hold automatically since member dtors run in reverse.
   */
  struct CompositeTail {
    std::vector<ThriftServerAppAdapter::Ptr> children;
    ThriftServerCompositeAppAdapter::Ptr adapter;
  };

  // Tail declared first so it is destroyed last — must outlive the
  // pipeline, which holds a raw tail pointer.
  std::variant<SimpleTail, CompositeTail> tail;

  // Buffer allocator used by the thrift pipeline.
  channel_pipeline::SimpleBufferAllocator thriftAllocator;

  // Head of the thrift pipeline. Owns the rocket connection (transport
  // handler, app adapter, rocket pipeline) via its rocketConnection()
  // member.
  std::unique_ptr<ThriftServerTransportAdapter> thriftTransportAdapter;

  // Thrift pipeline. Destroyed first among the owned fields here.
  channel_pipeline::PipelineImpl::Ptr thriftPipeline;

  // Per-connection thrift context. Null when the factory's
  // enableRequestContext is false. Co-owned with the pipeline's
  // ThriftServerConnectionContextHandler via refcount.
  boost::intrusive_ptr<ThriftConnContext> connContext;

  // Forceful close: tears the thrift pipeline down; the rocket
  // connection tears down via the transport adapter's handlerRemoved.
  // Deactivate before close so the bridge's connected_ guard is satisfied
  // before handlerRemoved runs (its DCHECK fires otherwise).
  void close() noexcept {
    if (thriftPipeline) {
      thriftPipeline->deactivate();
      thriftPipeline->close();
      thriftPipeline.reset();
    }
  }

  // Initiate graceful drain on the tail adapter — sends a peer-
  // disconnect frame and defers the close callback until in-flight
  // requests complete. Must run on the thrift pipeline's EventBase.
  void drain() noexcept {
    std::visit([](auto& t) { t.adapter->startDrain(); }, tail);
  }

  // Wire a callback fired once when the connection reaches Closed. The cb
  // lives on the pipeline tail adapter in both cases, so onPipelineInactive
  // (the canonical "connection done" edge) fires it.
  void setCloseCallback(std::function<void()> cb) {
    std::visit(
        [&](auto& t) { t.adapter->setCloseCallback(std::move(cb)); }, tail);
  }
};

} // namespace apache::thrift::fast_thrift::thrift::server
