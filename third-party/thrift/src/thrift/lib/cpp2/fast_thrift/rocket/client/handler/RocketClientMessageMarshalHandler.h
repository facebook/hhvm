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

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Handler.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/ComposedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>

#include <folly/ExceptionWrapper.h>

namespace apache::thrift::fast_thrift::rocket::client::handler {

/**
 * RocketClientMessageMarshalHandler - Marshals between the wire frame
 * representation and the rocket-layer message envelope on the client.
 *
 * Client-side counterpart to `RocketServerMessageMarshalHandler`. This is
 * the rocket↔frame boundary. The actual wire codec
 * (`BytesPtr ↔ ParsedFrame` / `ComposedFrame → BytesPtr`) lives in
 * `frame::handler::FrameCodecHandler` and is shared across client and
 * server; this handler only does the per-side message wrap/unwrap.
 *
 * Inbound (read path):
 *   `ParsedFrame` → `RocketResponseMessage`
 *   Wraps the parsed wire frame in the response envelope so downstream
 *   rocket handlers (StreamStateHandler, RequestResponseHandler, …) see a
 *   uniform message shape. `requestContext` / `streamType` are stamped
 *   further downstream by `RocketClientStreamStateHandler`.
 *
 * Outbound (write path):
 *   `RocketRequestMessage` → `ComposedFrame`
 *   Extracts the logical frame from the request envelope and fires it
 *   downstream toward `FrameFragmentationHandler` and `FrameCodecHandler`.
 *
 * Pipeline position:
 *   ... → FrameCodecHandler → FrameDefragmentationHandler →
 *     RocketClientMessageMarshalHandler → RocketClientSetupFrameHandler →
 *     ...
 *
 * Splitting codec from marshal is what makes room for the frame-level
 * fragmentation handlers, which operate on the bare `ParsedFrame` /
 * `ComposedFrame` rather than on rocket envelopes. Pipelines that do not
 * need fragmentation can keep using the combined
 * `RocketClientFrameCodecHandler` instead of this pair.
 */
class RocketClientMessageMarshalHandler {
 public:
  RocketClientMessageMarshalHandler() = default;

  // === HandlerLifecycle ===

  template <typename Context>
  void handlerAdded(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {}

  // === InboundHandler ===

  template <typename Context>
  void onPipelineActive(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onReadReady(Context& /*ctx*/) noexcept {}

  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onRead(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto frame =
        msg.take<apache::thrift::fast_thrift::frame::read::ParsedFrame>();
    RocketResponseMessage response;
    response.payload = std::move(frame);
    return ctx.fireRead(
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(response)));
  }

  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper e) noexcept {
    ctx.fireException(std::move(e));
  }

  // === OutboundHandler ===

  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onWrite(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto request = msg.take<RocketRequestMessage>();
    return ctx.fireWrite(
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(request.frame)));
  }

  template <typename Context>
  void onPipelineInactive(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onWriteReady(Context& /*ctx*/) noexcept {}
};

static_assert(
    apache::thrift::fast_thrift::channel_pipeline::DuplexHandler<
        RocketClientMessageMarshalHandler,
        apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl>,
    "RocketClientMessageMarshalHandler must satisfy DuplexHandler concept");

} // namespace apache::thrift::fast_thrift::rocket::client::handler
