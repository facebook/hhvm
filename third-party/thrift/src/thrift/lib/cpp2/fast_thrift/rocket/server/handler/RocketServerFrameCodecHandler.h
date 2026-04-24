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
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>

#include <folly/ExceptionWrapper.h>
#include <folly/io/IOBuf.h>

namespace apache::thrift::fast_thrift::rocket::server::handler {

/**
 * RocketServerFrameCodecHandler - Server-side codec for Rocket frames.
 *
 * Bridges the gap between FrameLengthParserHandler (which outputs raw IOBuf
 * frames) and RocketServerSetupFrameHandler (which expects ParsedFrame).
 *
 * Inbound (read path):
 *   std::unique_ptr<folly::IOBuf> ->
 * apache::thrift::fast_thrift::frame::read::ParsedFrame Parses the raw IOBuf
 * into a ParsedFrame and validates it.
 *
 * Outbound (write path):
 *   Passthrough. RocketServerRequestResponseFrameHandler already serializes
 *   responses to std::unique_ptr<folly::IOBuf> via
 * apache::thrift::fast_thrift::frame::write::serialize().
 *
 * Pipeline position:
 *   Transport -> FrameLengthParserHandler -> RocketServerFrameCodecHandler ->
 *   RocketServerSetupFrameHandler -> ...
 */
class RocketServerFrameCodecHandler {
 public:
  RocketServerFrameCodecHandler() = default;

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
    auto buf =
        msg.take<apache::thrift::fast_thrift::channel_pipeline::BytesPtr>();

    // Parse the frame
    auto frame =
        apache::thrift::fast_thrift::frame::read::tryParseFrame(std::move(buf));

    // Validate the frame before proceeding
    if (!frame.isValid()) {
      ctx.fireException(
          folly::make_exception_wrapper<std::runtime_error>(
              "Invalid frame received: failed frame validation"));
      return apache::thrift::fast_thrift::channel_pipeline::Result::Error;
    }

    return ctx.fireRead(
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(frame)));
  }

  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper e) noexcept {
    ctx.fireException(std::move(e));
  }

  // === OutboundHandler ===

  // Passthrough: RocketServerRequestResponseFrameHandler already serializes
  // responses to IOBuf via
  // apache::thrift::fast_thrift::frame::write::serialize().
  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onWrite(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    return ctx.fireWrite(std::move(msg));
  }

  template <typename Context>
  void onPipelineInactive(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onWriteReady(Context& /*ctx*/) noexcept {}
};

static_assert(
    apache::thrift::fast_thrift::channel_pipeline::DuplexHandler<
        RocketServerFrameCodecHandler,
        apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl>,
    "RocketServerFrameCodecHandler must satisfy DuplexHandler concept");

} // namespace apache::thrift::fast_thrift::rocket::server::handler
