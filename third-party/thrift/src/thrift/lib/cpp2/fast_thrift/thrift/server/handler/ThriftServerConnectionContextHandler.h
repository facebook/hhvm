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

#include <utility>

#include <boost/intrusive_ptr.hpp>

#include <folly/ExceptionWrapper.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/context/ThriftConnContext.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * ThriftServerConnectionContextHandler — duplex pipeline handler that owns
 * the per-connection ThriftConnContext and stamps it onto every inbound
 * request passing through.
 *
 * Lives in the thrift pipeline between ThriftServerTransportAdapter (head)
 * and the tail app adapter. The handler is the lifecycle owner of record
 * for the ThriftConnContext: it is constructed when the connection is
 * accepted and destroyed when the pipeline tears down. Per-request handles
 * (e.g. via the request message's `connContext` field) keep the context
 * alive across in-flight async work because ThriftConnContext is
 * intrusively refcounted.
 */
template <typename Context>
class ThriftServerConnectionContextHandler {
 public:
  explicit ThriftServerConnectionContextHandler(
      boost::intrusive_ptr<ThriftConnContext> connContext) noexcept
      : connContext_(std::move(connContext)) {}

  // HandlerLifecycle
  void handlerAdded(Context& /*ctx*/) noexcept {}
  void handlerRemoved(Context& /*ctx*/) noexcept {}

  // InboundHandler
  channel_pipeline::Result onRead(
      Context& ctx, channel_pipeline::TypeErasedBox&& msg) noexcept {
    auto& request = msg.get<ThriftServerRequestMessage>();
    request.connContext = connContext_;
    return ctx.fireRead(std::move(msg));
  }

  void onReadReady(Context& /*ctx*/) noexcept {}

  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }

  void onPipelineActive(Context& /*ctx*/) noexcept {}

  // OutboundHandler — pure pass-through.
  channel_pipeline::Result onWrite(
      Context& ctx, channel_pipeline::TypeErasedBox&& msg) noexcept {
    return ctx.fireWrite(std::move(msg));
  }

  void onWriteReady(Context& /*ctx*/) noexcept {}

  void onPipelineInactive(Context& /*ctx*/) noexcept {}

  // Accessor for tests / future extension points (TLS handshake info, etc.).
  const boost::intrusive_ptr<ThriftConnContext>& getConnectionContext()
      const noexcept {
    return connContext_;
  }

 private:
  boost::intrusive_ptr<ThriftConnContext> connContext_;
};

} // namespace apache::thrift::fast_thrift::thrift
