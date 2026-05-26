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
#include <utility>

#include <folly/ExceptionWrapper.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/connection/ConnectionFactory.h>
#include <thrift/lib/cpp2/fast_thrift/connection/common/Messages.h>

namespace apache::thrift::fast_thrift::connection {

/**
 * Middle handler in the acceptance pipeline. Receives ConnectionMessage
 * (ready transport + peer addr), calls factory.getConnection() to build a
 * typed Connection, and fires that Connection downstream as the new
 * pipeline message.
 *
 * The connection layer is generic on F (and therefore on the Connection
 * type F produces); subsequent handlers (acceptance hook, installer) are
 * templated on FactoryConnectionType<F>.
 */
template <ConnectionFactory F>
class ConnectionBuilderHandler {
 public:
  explicit ConnectionBuilderHandler(F& factory) noexcept : factory_(factory) {}

  ConnectionBuilderHandler(const ConnectionBuilderHandler&) = delete;
  ConnectionBuilderHandler& operator=(const ConnectionBuilderHandler&) = delete;
  ConnectionBuilderHandler(ConnectionBuilderHandler&&) = delete;
  ConnectionBuilderHandler& operator=(ConnectionBuilderHandler&&) = delete;
  ~ConnectionBuilderHandler() = default;

  // === Inbound handler — acceptance pipeline is one-way ===

  template <typename Context>
  channel_pipeline::Result onRead(
      Context& ctx, channel_pipeline::TypeErasedBox&& msg) noexcept {
    auto ready = msg.take<ConnectionMessage>();
    auto conn = factory_.getConnection(std::move(ready.transport));
    return ctx.fireRead(channel_pipeline::erase_and_box(std::move(conn)));
  }

  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    XLOG(ERR) << "ConnectionBuilderHandler: " << e.what();
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
  F& factory_;
};

} // namespace apache::thrift::fast_thrift::connection
