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

#include <folly/Function.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/RocketClientEventFactory.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/adapter/RocketClientAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>

namespace apache::thrift::fast_thrift::rocket::client {

/**
 * RocketClientConnectionT — owns the rocket pipeline and its
 * resources for a single client connection.
 *
 * Groups the rocket-layer objects that together form the transport
 * underneath a thrift client pipeline:
 *
 *   RocketClientAppAdapter → [rocket handlers] → TransportHandler
 *
 * Templated on the transport's write-completion event factory (see
 * WriteCompletion.h). The default NoOp factory means no per-write event is
 * fired — zero cost. Connections that want write tracking instantiate with
 * RocketClientEventFactory (see the RocketClientTrackingConnection alias) and
 * build the pipeline with RocketClientEventId plus a subscriber.
 *
 * Lifecycle is split between disconnect() and destroy():
 *   - disconnect(ew) deactivates the pipeline and closes the underlying
 *     socket. If `ew` is non-empty it is fired up the pipeline as the
 *     reason for the disconnect; otherwise disconnect is a pure lifecycle
 *     signal. Handlers see onPipelineInactive. The pipeline structure
 *     itself is preserved (handlers stay attached).
 *   - destroy() closes the pipeline (handlers see handlerRemoved) and
 *     releases the transport handler.
 */
template <
    transport::WriteCompleteEventFactory Factory =
        transport::NoOpWriteCompleteEventFactory>
struct RocketClientConnectionT {
  using OnConnectFn = folly::Function<void() noexcept>;
  using OnDisconnectFn = folly::Function<void() noexcept>;
  using TransportHandler = transport::TransportHandlerT<Factory>;

  RocketClientConnectionT() = default;
  RocketClientConnectionT(const RocketClientConnectionT&) = delete;
  RocketClientConnectionT& operator=(const RocketClientConnectionT&) = delete;
  RocketClientConnectionT(RocketClientConnectionT&&) = delete;
  RocketClientConnectionT& operator=(RocketClientConnectionT&&) = delete;
  ~RocketClientConnectionT() { destroy(); }

  // Member destruction runs in reverse declaration order. Allocator is
  // referenced by raw pointer from the pipeline, so it must outlive the
  // pipeline; the endpoints hold DestructorGuards on the pipeline, so they
  // must outlive it as well. Declared accordingly.
  channel_pipeline::SimpleBufferAllocator allocator;
  channel_pipeline::PipelineImpl::Ptr pipeline;
  typename TransportHandler::Ptr transportHandler;
  rocket::client::RocketClientAppAdapter::Ptr appAdapter{
      new rocket::client::RocketClientAppAdapter()};

  /**
   * Subscribe to connection lifecycle events:
   *   - onConnect:    transport socket is up, ready to send/receive
   *   - onDisconnect: transport went down, no more traffic possible
   *
   * Destroy is intentionally not exposed here — it is always
   * owner-initiated (via destroy()), runs synchronously, and has no
   * external observer that wouldn't already know it called destroy.
   *
   * Replaces any previously-registered handlers.
   */
  void setLifecycleHandlers(
      OnConnectFn onConnect, OnDisconnectFn onDisconnect) noexcept {
    appAdapter->setLifecycleHandlers(
        std::move(onConnect), std::move(onDisconnect));
  }

  /**
   * Deactivate the pipeline and close the underlying socket. Idempotent.
   * Pipeline + transport handler remain owned; call destroy() to release
   * them. If `ew` is non-empty it is fired up the pipeline before
   * deactivation so handlers can resolve in-flight requests with the
   * actual reason.
   */
  void disconnect(folly::exception_wrapper ew = {}) noexcept {
    if (disconnected_) {
      return;
    }
    disconnected_ = true;
    if (transportHandler) {
      transportHandler->close(std::move(ew));
    }
  }

  /**
   * Tear down the rocket pipeline (handlerRemoved fan-out) and release
   * the transport + appAdapter. Implies disconnect() if not already
   * disconnected. Idempotent.
   *
   * Order matters: both endpoints (transport + appAdapter) hold a
   * DestructorGuard on the pipeline, so the pipeline cannot actually be
   * destroyed until both call resetPipeline(). After that the pipeline
   * is closed and dropped, then the endpoints themselves.
   */
  void destroy() noexcept {
    disconnect();
    if (transportHandler) {
      transportHandler->resetPipeline();
    }
    if (appAdapter) {
      appAdapter->resetPipeline();
    }
    if (pipeline) {
      pipeline->close();
      pipeline.reset();
    }
    transportHandler.reset();
    appAdapter.reset();
  }

  /**
   * Legacy combined path: disconnect with `e` as the reason, then destroy.
   * Retained as a shim so the thrift-side bridge can migrate in a
   * follow-up commit; new callers should use disconnect(ew) + destroy().
   */
  void close(folly::exception_wrapper&& e) noexcept {
    disconnect(std::move(e));
    destroy();
  }

 private:
  bool disconnected_{false};
};

// Default: write-completion events disabled (NoOp factory) — zero per-write
// cost. Use RocketClientConnection::TransportHandler when building the
// pipeline.
using RocketClientConnection = RocketClientConnectionT<>;

// Opt-in: fires a RocketClientEvent on each socket write completion for write
// tracking. The pipeline must be built with RocketClientEventId and a
// subscriber for the events to be delivered.
using RocketClientTrackingConnection =
    RocketClientConnectionT<RocketClientEventFactory>;

} // namespace apache::thrift::fast_thrift::rocket::client
