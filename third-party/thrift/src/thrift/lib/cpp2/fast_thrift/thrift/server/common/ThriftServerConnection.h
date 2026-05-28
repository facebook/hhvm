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
#include <glog/logging.h>

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

  // Tail of the thrift pipeline. `std::monostate` is the empty state used
  // by the dtor to explicitly destroy the active alternative — declaration
  // order is NOT the source of truth for teardown sequencing; see ~dtor.
  std::variant<std::monostate, SimpleTail, CompositeTail> tail;

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

  // Begin reading. May synchronously drain pre-received bytes (e.g. the
  // post-StopTLS handoff buffer) and dispatch the first request inline, so
  // callers must complete every piece of accept-time setup
  // (onConnectionAccepted hook, registration in the owning
  // connection-manager map) before calling this.
  void start() noexcept {
    DCHECK(!started_) << "ThriftServerConnection::start called twice";
    started_ = true;
    thriftTransportAdapter->rocketConnection().transportHandler->onConnect();
  }

  // Initiate close. From the owner's perspective this is fire-and-wait:
  // call close(), then wait for the closeCallback to fire before
  // dropping the ThriftServerConnection. Internally we just trigger
  // drain — the pipeline-resident ThriftServerConnectionCloseHandler owns the
  // terminal state machine (drain timeout, reap, LOG(FATAL) on stuck
  // callbacks) and fires ConnectionClosed inbound when settling is
  // complete, which the tail adapter turns into the closeCallback.
  // Must run on the thrift pipeline's EventBase.
  void close() noexcept {
    visitTail([](auto& t) { t.adapter->close(); });
  }

  // Alias for close(). Pre-existing ConnectionHandler two-phase shutdown
  // (drain-all then force-close stragglers) still calls drain(); the
  // distinction is no longer meaningful because the pipeline-resident
  // ThriftServerConnectionCloseHandler now owns the drain+reap timeouts
  // internally. ConnectionHandler should collapse to a single close() phase as
  // follow-up.
  void drain() noexcept { close(); }

  // Wire a callback fired once when the connection has fully closed
  // (all in-flight handler callbacks have settled or LOG(FATAL)'d).
  // The cb lives on the pipeline tail adapter, which fires it in
  // response to the ConnectionClosed inbound event from
  // ThriftServerConnectionCloseHandler.
  void setCloseCallback(std::function<void()> cb) {
    visitTail([&](auto& t) { t.adapter->setCloseCallback(std::move(cb)); });
  }

  // Move-only. User-defined dtor below would otherwise suppress implicit
  // moves, breaking factory paths that move-construct (e.g.,
  // TypeErasedBox::take<T>).
  ThriftServerConnection() = default;
  ThriftServerConnection(ThriftServerConnection&&) noexcept = default;
  ThriftServerConnection& operator=(ThriftServerConnection&&) noexcept =
      default;
  ThriftServerConnection(const ThriftServerConnection&) = delete;
  ThriftServerConnection& operator=(const ThriftServerConnection&) = delete;

  // Explicit ordered teardown. Two correctness constraints:
  //
  //   1. Pipeline-dtor walks handler nodes and may fire callbacks on the
  //      tail adapter (raw pointer). So the tail must still be alive when
  //      thriftPipeline destroys.
  //   2. The tail adapter holds a DestructorGuard on thriftPipeline (set
  //      in setPipeline). If we let the adapter's own dtor release that
  //      guard, the pipeline's deferred destroy fires synchronously
  //      inside ~adapter -> UAF on the partially-destroyed tail.
  //
  // Every step is explicit — do NOT rely on member declaration order.
  // Each Ptr/visit is null-tolerant so a moved-from instance's dtor is a
  // no-op (Ptrs/variant are reset by the move).
  ~ThriftServerConnection() {
    // 1. Release every DG on thriftPipeline so the pipeline is fully
    //    ungated for step 3. Both the head (thriftTransportAdapter) and
    //    the tail adapter took a DG in setPipeline; if either still holds
    //    one when its dtor runs, the pipeline's deferred destroy fires
    //    synchronously inside ~adapter and walks the already-freed peer.
    //    Monostate is legitimate (moved-from / explicitly cleared); skip
    //    silently rather than DCHECK like the public-API visitTail.
    std::visit(
        [](auto& t) {
          using T = std::decay_t<decltype(t)>;
          if constexpr (!std::is_same_v<T, std::monostate>) {
            if (t.adapter) {
              t.adapter->resetPipeline();
            }
          }
        },
        tail);
    if (thriftTransportAdapter) {
      thriftTransportAdapter->resetPipeline();
    }
    // 2. Drop intrusive_ptr early so any pipeline-handler co-owning it
    //    sees a single ref and releases cleanly during step 3.
    connContext.reset();
    // 3. Destroy the thrift pipeline. Both head and tail are still
    //    alive, so any handler-removed callback into them is safe.
    thriftPipeline.reset();
    // 4. Destroy the tail adapter(s). monostate is the empty state;
    //    assigning it explicitly destroys the active SimpleTail /
    //    CompositeTail (composite first, then its children).
    tail = std::monostate{};
    // 5. Tear down the transport adapter (which owns the rocket
    //    connection: rocket pipeline, rocket adapter, transport).
    thriftTransportAdapter.reset();
    // 6. thriftAllocator is a value member; it has no .reset() to call.
    //    It destructs at the end of this dtor as part of the implicit
    //    member-destruction phase.
  }

 private:
  // visitTail — std::visit wrapper. monostate is the dtor-only empty
  // state; no public caller should ever observe it.
  template <typename F>
  void visitTail(F&& f) {
    std::visit(
        [&](auto& t) {
          if constexpr (std::is_same_v<
                            std::decay_t<decltype(t)>,
                            std::monostate>) {
            DCHECK(false) << "invalid app adapter";
          } else {
            std::forward<F>(f)(t);
          }
        },
        tail);
  }

  bool started_{false};
};

} // namespace apache::thrift::fast_thrift::thrift::server
