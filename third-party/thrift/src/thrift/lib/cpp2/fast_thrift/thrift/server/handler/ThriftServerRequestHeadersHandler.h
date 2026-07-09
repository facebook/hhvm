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

#include <glog/logging.h>
#include <folly/ExceptionWrapper.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * ThriftServerRequestHeadersHandler — duplex pipeline handler that moves the
 * inbound custom request headers (RequestRpcMetadata.otherMetadata) off the
 * request payload and onto the per-request ThriftRequestContext, so the tail
 * app adapter can read them via ThriftRequestContext::getHeaders().
 *
 * Pipeline order: MUST sit downstream of ThriftServerRequestContextHandler
 * (which allocates the per-request ThriftRequestContext). Only populates a
 * field; it does not allocate the request context itself. The header map is
 * moved out of the metadata; nothing downstream reads otherMetadata.
 */
template <typename Context>
class ThriftServerRequestHeadersHandler {
 public:
  // HandlerLifecycle
  void handlerAdded(Context& /*ctx*/) noexcept {}
  void handlerRemoved(Context& /*ctx*/) noexcept {}

  // InboundHandler
  channel_pipeline::Result onRead(
      Context& ctx, channel_pipeline::TypeErasedBox&& msg) noexcept {
    auto& request = msg.get<ThriftServerRequestMessage>();
    DCHECK(request.requestContext)
        << "request.requestContext not found; "
           "ThriftServerRequestContextHandler must run upstream";
    if (request.payload.template is<ThriftRequestResponsePayload>()) {
      auto& rr = request.payload.template get<ThriftRequestResponsePayload>();
      if (rr.metadata && rr.metadata->otherMetadata().has_value()) {
        request.requestContext->setHeaders(
            std::move(*rr.metadata->otherMetadata()));
      }
    }
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
};

} // namespace apache::thrift::fast_thrift::thrift
