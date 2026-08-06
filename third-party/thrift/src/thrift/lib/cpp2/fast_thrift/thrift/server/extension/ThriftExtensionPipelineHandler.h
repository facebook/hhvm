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

#include <cstdint>
#include <utility>

#include <folly/ExceptionWrapper.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/extension/ThriftExtension.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/framework/ThriftPipelineHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/util/ResponsePayloads.h>

namespace apache::thrift::fast_thrift::thrift::server {

/**
 * Adapts a user extension handler H into a duplex channel_pipeline handler.
 * This is the only place that touches the raw TypeErasedBox and pipeline
 * context — the extension itself only ever sees a ThriftRequestView /
 * ThriftRequestMutator, so it cannot retain, replace, or manually forward a
 * message.
 *
 * Forwarding / rejection contract:
 *   - H::onRequest returns RequestVerdict::proceed() → the request is
 *     forwarded toward the tail (fireRead).
 *   - H::onRequest returns RequestVerdict::reject(cause) → the request is
 *     dropped and an application error carrying the cause's name/message is
 *     emitted back down the outbound path (fireWrite), echoing the request's
 *     streamId. As the error travels out, extensions that already ran see their
 *     onResponse/onWrite in reverse order — the pipeline provides that
 * ordering.
 *
 * Response side: if H additionally implements onResponse, it is invoked (via a
 * view or mutator) on every outbound response before it is forwarded.
 *
 * Lifetime: one instance per connection (constructed by the pipeline handler
 * factory); H is owned by value and its constructor arguments are forwarded
 * from addThriftExtension.
 */
template <typename H>
class ThriftExtensionPipelineHandler {
  static_assert(
      ThriftExtensionHandler<H>,
      "ThriftExtensionPipelineHandler<H>: H must implement onRequest taking "
      "either const ThriftRequestView& [read-only] or ThriftRequestMutator& "
      "[read/write], noexcept and returning RequestVerdict. The mutator is a "
      "view, so a read/write extension still gets every read accessor.");

 public:
  template <typename... Args>
  explicit ThriftExtensionPipelineHandler(Args&&... args)
      : handler_(std::forward<Args>(args)...) {}

  channel_pipeline::Result onRead(
      ThriftPipelineHandlerContext& ctx,
      channel_pipeline::TypeErasedBox&& msg) noexcept {
    auto& request = msg.get<ThriftServerRequestMessage>();
    // Capture before any potential reject drops the inbound message: the
    // synthesized response must echo this streamId to correlate on the client.
    const uint32_t streamId = request.streamId;

    // View form first: a mutator binds to a const view parameter, so testing
    // the view form first hands each extension exactly the access it declared.
    const RequestVerdict verdict = [&]() noexcept {
      if constexpr (HasRequestViewCallback<H>) {
        const ThriftRequestView view(request);
        return handler_.onRequest(view);
      } else {
        ThriftRequestMutator mutator(request);
        return handler_.onRequest(mutator);
      }
    }();

    if (!verdict.isRejected()) {
      return ctx.fireRead(std::move(msg));
    }
    auto response = makeUnknownExceptionMessage(streamId, verdict.cause());
    return ctx.fireWrite(channel_pipeline::erase_and_box(std::move(response)));
  }

  channel_pipeline::Result onWrite(
      ThriftPipelineHandlerContext& ctx,
      channel_pipeline::TypeErasedBox&& msg) noexcept {
    if constexpr (HasResponseCallback<H>) {
      auto& response = msg.get<ThriftServerResponseMessage>();
      if constexpr (HasResponseViewCallback<H>) {
        const ThriftResponseView view(response);
        handler_.onResponse(view);
      } else {
        ThriftResponseMutator mutator(response);
        handler_.onResponse(mutator);
      }
    }
    return ctx.fireWrite(std::move(msg));
  }

  void onException(
      ThriftPipelineHandlerContext& ctx,
      folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }

  void onPipelineActive(ThriftPipelineHandlerContext&) noexcept {}
  void onPipelineInactive(ThriftPipelineHandlerContext&) noexcept {}
  void onReadReady(ThriftPipelineHandlerContext&) noexcept {}
  void onWriteReady(ThriftPipelineHandlerContext&) noexcept {}
  void handlerAdded(ThriftPipelineHandlerContext&) noexcept {}
  void handlerRemoved(ThriftPipelineHandlerContext&) noexcept {}

 private:
  H handler_;
};

} // namespace apache::thrift::fast_thrift::thrift::server
