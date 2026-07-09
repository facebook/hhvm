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

#include <chrono>
#include <cstddef>
#include <cstdint>

#include <folly/SocketAddress.h>

#include <thrift/lib/cpp2/fast_thrift/frame/write/IntervalBatchingHandlerConfig.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * Shared configuration for FastThriftServer / FastThriftChannelServer.
 */
struct FastThriftServerConfig {
  // Address to bind to.
  folly::SocketAddress address;

  // Number of IO threads. Each thread runs its own EventBase and accepts
  // connections via SO_REUSEPORT.
  uint32_t numIOThreads{1};

  // Minimum payload size in bytes for MSG_ZEROCOPY. 0 disables zero-copy.
  size_t zeroCopyThreshold{0};

  // When true, auto-mount the ThriftMetadataService alongside the user
  // handler so introspection tools (e.g. Thrift Fiddle) can discover the
  // service schema. The response is built once at start() and served on
  // every getThriftServiceMetadata() RPC. For the response to be non-empty,
  // the underlying thrift_library must be built with `with_schema = True`.
  bool enableMetadataService{false};

  // When true, construct a per-connection ThriftConnContext on accept and
  // wire the ThriftServerRequestContextHandler +
  // ThriftServerConnectionContextHandler into the thrift pipeline, so each
  // request's ThriftRequestContext is populated with the ThriftConnContext.
  // The setOnConnectionAccepted callback receives a pointer to the
  // ThriftConnContext (or nullptr when this flag is off).
  bool enableRequestContext{false};

  // When true, populate each request's ThriftRequestContext with the inbound
  // custom headers (RequestRpcMetadata.otherMetadata) so handlers can read
  // them via getHeaders()/getHeader(). Requires enableRequestContext; ignored
  // when that flag is off. Only takes effect on FastThriftServer.
  bool enableRequestHeaders{false};

  // When true, insert WriteBufferBackpressureHandler into the thrift
  // pipeline. The handler buffers outbound responses when the downstream
  // pipeline returns Result::Backpressure (e.g. transport write buffer
  // full), drains them in FIFO on onWriteReady, and surfaces inbound
  // Backpressure while saturated so the transport pauses socket reads.
  bool enableWriteBufferBackpressure{false};

  // Outbound write batching. Default zero-interval flushes via LoopCallback
  // at end of each event loop iteration. Set batchingInterval > 0 to use
  // an HHWheelTimer-driven flush instead.
  frame::write::IntervalBatchingHandlerConfig batchingConfig{};

  // Per-connection terminal-phase deadlines. Consumed by
  // ThriftServerConnectionCloseHandler; defaults mirror its
  // kDefaultDrainTimeout / kDefaultReapTimeout.
  //
  //   drainTimeout — after a CloseConnection event, how long to wait for
  //     in-flight responses to flush before forcing the socket down.
  //   reapTimeout — after the socket is gone, how long to wait for
  //     in-flight handler callbacks to return before LOG(FATAL)ing.
  //     A handler that hasn't completed within this window is stuck and
  //     would otherwise silently leak the per-connection adapter.
  std::chrono::milliseconds drainTimeout{std::chrono::seconds{30}};
  std::chrono::milliseconds reapTimeout{std::chrono::seconds{60}};
};

} // namespace apache::thrift::fast_thrift::thrift
