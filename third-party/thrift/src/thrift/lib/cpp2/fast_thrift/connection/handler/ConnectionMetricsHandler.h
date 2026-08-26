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

#include <glog/logging.h>
#include <folly/ExceptionWrapper.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/connection/common/ConnectionStats.h>

namespace apache::thrift::fast_thrift::connection::handler {

HANDLER_TAG(connection_metrics_handler);

/**
 * Inbound handler that counts connections. Completely pass-through — it never
 * inspects a message.
 *
 * Sits at the tail of the acceptance pipeline, below the connection builder,
 * so every message reaching it is a connection that survived TLS and was
 * built. That is what the classic server's connAccepted() counts too: it
 * fires from the routing handlers at connection-object creation, past the
 * handshake. A socket that dies mid-TLS is counted by neither.
 *
 * The acceptance pipeline is per-EventBase rather than per-connection, so
 * each accepted connection crosses this handler as one message. The shard is
 * resolved once, when ConnectionHandler builds the pipeline on its own
 * EventBase, and every write comes from that same thread — which is what lets
 * the counters stay non-atomic.
 *
 * The active gauge is only incremented here. Its decrement lives in
 * ConnectionHandler::onConnectionClosed, because a closing connection does
 * not travel back through the acceptance pipeline — it reports via its own
 * close callback. That callback is the same place the handler's connection
 * map is pruned, so the gauge and the map cannot disagree.
 */
class ConnectionMetricsHandler {
 public:
  // Borrowed for the handler's lifetime. The shard belongs to the server,
  // which outlives every per-EventBase acceptance pipeline.
  explicit ConnectionMetricsHandler(ConnectionStatsShard* stats) noexcept
      : stats_(stats) {
    DCHECK(stats_ != nullptr);
  }

  template <typename Context>
  [[nodiscard]] channel_pipeline::Result onRead(
      Context& ctx, channel_pipeline::TypeErasedBox&& msg) noexcept {
    DCHECK(stats_ != nullptr);
    stats_->connectionsAccepted.incrementValue(1);
    stats_->connectionsActive.incrementValue(1);
    return ctx.fireRead(std::move(msg));
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
  ConnectionStatsShard* stats_;
};

} // namespace apache::thrift::fast_thrift::connection::handler
