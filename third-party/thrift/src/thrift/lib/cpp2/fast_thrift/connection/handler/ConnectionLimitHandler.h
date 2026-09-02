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

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <utility>

#include <glog/logging.h>
#include <folly/ExceptionWrapper.h>
#include <folly/logging/xlog.h>
#include <folly/observer/Observer.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/connection/common/ConnectionStats.h>
#include <thrift/lib/cpp2/fast_thrift/connection/common/Messages.h>

namespace apache::thrift::fast_thrift::connection::handler {

HANDLER_TAG(connection_limit_handler);

/**
 * Inbound handler that refuses connections once its IO thread is holding as
 * many as it is allowed.
 *
 * Sits above the connection builder and below the TLS handler, so it sees
 * every connection that survived the handshake and none that did not, and it
 * refuses before a Connection object is built. That is the point the classic
 * server enforces its own limit from, past the handshake — a socket that dies
 * mid-TLS is bounded by maxPendingConnections instead.
 *
 * Refusing means returning Error rather than forwarding: the inbound message
 * owns the transport, so dropping it closes the socket and the peer sees a
 * connection error.
 *
 * The count it compares against is the one ConnectionHandler keeps for its own
 * connection map, so the cap and the map cannot disagree about what is live.
 * That count is only mutated on this EventBase, and this handler only runs on
 * it, so the read needs no ordering beyond relaxed.
 *
 * Loopback peers are never refused. A server at its limit must still answer a
 * local health probe, or the agent that owns it will conclude the process is
 * dead and restart it.
 */
class ConnectionLimitHandler {
 public:
  // Both pointers are borrowed. `liveConnections` belongs to the
  // ConnectionHandler that owns this pipeline; `stats` may be null, in which
  // case refusals are logged but not counted.
  ConnectionLimitHandler(
      folly::observer::Observer<uint32_t> limit,
      const std::atomic<size_t>* liveConnections,
      ConnectionStatsShard* stats) noexcept
      : limit_(std::move(limit)),
        liveConnections_(liveConnections),
        stats_(stats) {
    DCHECK(liveConnections_ != nullptr);
  }

  template <typename Context>
  [[nodiscard]] channel_pipeline::Result onRead(
      Context& ctx, channel_pipeline::TypeErasedBox&& msg) noexcept {
    const uint32_t limit = limit_.get();
    if (limit == 0) {
      return ctx.fireRead(std::move(msg));
    }
    if (FOLLY_LIKELY(
            liveConnections_->load(std::memory_order_relaxed) < limit)) {
      return ctx.fireRead(std::move(msg));
    }
    const auto& incoming = msg.get<ConnectionMessage>();
    if (incoming.clientAddr.isLoopbackAddress()) {
      return ctx.fireRead(std::move(msg));
    }
    if (stats_ != nullptr) {
      stats_->connectionsRejected.incrementValue(1);
    }
    XLOG_EVERY_MS(WARN, 1000)
        << "Refusing connection from " << incoming.clientAddr.describe()
        << ": already holding " << limit << " connections on this thread";
    return channel_pipeline::Result::Error;
  }

  template <typename Context>
  void handlerAdded(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onPipelineActive(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onReadReady(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }

 private:
  // Read-optimized: an atomic load per accept unless the setting changed.
  folly::observer::AtomicObserver<uint32_t> limit_;
  const std::atomic<size_t>* liveConnections_;
  ConnectionStatsShard* stats_;
};

} // namespace apache::thrift::fast_thrift::connection::handler
