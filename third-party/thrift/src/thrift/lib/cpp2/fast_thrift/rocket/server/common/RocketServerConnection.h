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
#include <thrift/lib/cpp2/fast_thrift/rocket/server/adapter/RocketServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>

namespace apache::thrift::fast_thrift::rocket::server {

/**
 * RocketServerConnection — owns the rocket pipeline and its
 * resources for a single server connection.
 *
 * Groups the rocket-layer objects that together form the transport
 * underneath a thrift server pipeline:
 *
 *   TransportHandler → [rocket handlers] → RocketServerAppAdapter
 *
 * Lifecycle is split between disconnect() and destroy():
 *   - disconnect(ew) closes the underlying socket. If `ew` is non-empty it
 *     is fired up the pipeline as the reason for the disconnect; otherwise
 *     disconnect is a pure lifecycle signal. Handlers observe
 *     onPipelineInactive as the rocket pipeline winds down. The pipeline
 *     structure itself is preserved (handlers stay attached).
 *   - destroy() closes the pipeline (handlers see handlerRemoved) and
 *     releases the transport handler + appAdapter.
 *
 * The pre-existing close(ew) entry point is retained as a backwards-compat
 * shim for ConnectionHandler-driven teardown; it composes disconnect() +
 * destroy().
 */
struct RocketServerConnection {
  using OnConnectFn = folly::Function<void() noexcept>;
  using OnDisconnectFn = folly::Function<void() noexcept>;

  RocketServerConnection() = default;
  ~RocketServerConnection() { destroy(); }

  RocketServerConnection(const RocketServerConnection&) = delete;
  RocketServerConnection& operator=(const RocketServerConnection&) = delete;
  RocketServerConnection(RocketServerConnection&&) = default;
  RocketServerConnection& operator=(RocketServerConnection&&) = default;

  rocket::server::RocketServerAppAdapter::Ptr appAdapter{
      new rocket::server::RocketServerAppAdapter()};
  transport::TransportHandler::Ptr transportHandler;
  channel_pipeline::PipelineImpl::Ptr pipeline;
  channel_pipeline::SimpleBufferAllocator allocator;

  bool disconnected_{false};

  /**
   * Subscribe to connection lifecycle events:
   *   - onConnect:    rocket pipeline became active (socket ready)
   *   - onDisconnect: rocket pipeline went inactive (socket down)
   *
   * Destroy is intentionally not exposed here — it is always
   * owner-initiated (via destroy()) and runs synchronously.
   *
   * Replaces any previously-registered handlers.
   */
  void setLifecycleHandlers(
      OnConnectFn onConnect, OnDisconnectFn onDisconnect) noexcept {
    appAdapter->setLifecycleHandlers(
        std::move(onConnect), std::move(onDisconnect));
  }

  /**
   * Close the underlying socket. Idempotent. Pipeline + transport handler
   * remain owned; call destroy() to release them. If `ew` is non-empty it
   * is fired up the pipeline before deactivation so handlers can resolve
   * in-flight state with the actual reason.
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
   * Order matters: transport stays alive while the pipeline runs
   * handlerRemoved, then transport and appAdapter are destroyed last.
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
      // Satisfy the adapter's handlerRemoved contract: deactivate before
      // close. The transport's beginClose normally fires deactivate from
      // disconnect(), but only when its state is Open — owner-initiated
      // teardown that bypasses the socket lifecycle would otherwise reach
      // close() with the adapter still flagged as connected.
      pipeline->deactivate();
      pipeline->close();
      pipeline.reset();
    }
    transportHandler.reset();
    appAdapter.reset();
  }

  /**
   * Legacy combined path: disconnect with `e` as the reason, then destroy.
   * Retained as a shim so ConnectionHandler-driven teardown keeps working
   * unchanged; new callers should use disconnect(ew) + destroy().
   */
  void close(folly::exception_wrapper&& e) noexcept {
    disconnect(std::move(e));
    destroy();
  }
};

} // namespace apache::thrift::fast_thrift::rocket::server
