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

#include <folly/ExceptionWrapper.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>

namespace apache::thrift::fast_thrift::connection {

/**
 * Optional middleware between ConnectionBuilderHandler and ConnectionInstaller.
 * Receives a typed Connection, runs a caller-supplied hook (e.g. the
 * thrift-side onConnectionAccepted callback), and fires the Connection
 * downstream unchanged.
 *
 * Templated on the concrete Connection type so the hook gets typed access
 * (e.g. to read a per-connection ThriftConnContext). Only inserted into
 * the acceptance pipeline when ConnectionHandler::setConnectionFactory is
 * called with a non-empty onAccept callback.
 */
template <typename Conn>
class ConnectionAcceptCallbackHandler {
 public:
  using OnAcceptFn = std::function<void(Conn&)>;

  explicit ConnectionAcceptCallbackHandler(OnAcceptFn onAccept) noexcept
      : onAccept_(std::move(onAccept)) {
    DCHECK(onAccept_);
  }

  ConnectionAcceptCallbackHandler(const ConnectionAcceptCallbackHandler&) =
      delete;
  ConnectionAcceptCallbackHandler& operator=(
      const ConnectionAcceptCallbackHandler&) = delete;
  ConnectionAcceptCallbackHandler(ConnectionAcceptCallbackHandler&&) = delete;
  ConnectionAcceptCallbackHandler& operator=(
      ConnectionAcceptCallbackHandler&&) = delete;
  ~ConnectionAcceptCallbackHandler() = default;

  // === Inbound handler — acceptance pipeline is one-way ===

  template <typename Context>
  channel_pipeline::Result onRead(
      Context& ctx, channel_pipeline::TypeErasedBox&& msg) noexcept {
    auto conn = msg.take<Conn>();
    onAccept_(conn);
    return ctx.fireRead(channel_pipeline::erase_and_box(std::move(conn)));
  }

  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    XLOG(ERR) << "ConnectionAcceptCallbackHandler: " << e.what();
    ctx.fireException(std::move(e));
  }

  template <typename Context>
  void handlerAdded(Context& /*ctx*/) noexcept {}
  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {}
  template <typename Context>
  void onPipelineActive(Context& /*ctx*/) noexcept {}
  template <typename Context>
  void onReadReady(Context& /*ctx*/) noexcept {}

 private:
  OnAcceptFn onAccept_;
};

} // namespace apache::thrift::fast_thrift::connection
