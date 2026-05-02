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
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Messages.h>

#include <folly/ExceptionWrapper.h>
#include <folly/io/IOBuf.h>

namespace apache::thrift::fast_thrift::rocket::server::handler {

/**
 * RocketServerFrameCodecHandler - Bidirectional codec for Rocket frames.
 *
 * The single point in the server pipeline that converts between typed
 * payloads and wire bytes:
 *
 * Inbound (read path):
 *   std::unique_ptr<folly::IOBuf> -> ParsedFrame
 *   Parses the raw IOBuf and validates it.
 *
 * Outbound (write path):
 *   RocketResponseMessage{frame = Composed*Frame variant alternative}
 *     -> std::unique_ptr<folly::IOBuf>
 *   Visits the variant and dispatches to the matching
 *   `frame::write::serialize(Composed*Frame&&)` overload.
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

    auto frame =
        apache::thrift::fast_thrift::frame::read::tryParseFrame(std::move(buf));

    if (!frame.isValid()) {
      ctx.fireException(
          folly::make_exception_wrapper<std::runtime_error>(
              "Invalid frame received: failed frame validation"));
      return apache::thrift::fast_thrift::channel_pipeline::Result::Error;
    }

    // Wrap in RocketRequestMessage so the inbound chain has a uniform
    // message type. StreamStateHandler stamps streamId/streamType later.
    // The cold path (onWrite serialize throw) emits the same message type
    // with the RocketRequestError variant; downstream handlers dispatch
    // on the variant rather than guessing the box type.
    RocketRequestMessage request;
    request.payload = std::move(frame);

    return ctx.fireRead(
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(request)));
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
    auto response = msg.take<RocketResponseMessage>();

    // Capture stream identity before the move into serialize(); on
    // throw, response.frame is in moved-from state.
    const uint32_t streamId = response.frame.streamId();
    const auto streamType = response.streamType;

    std::unique_ptr<folly::IOBuf> serializedFrame;
    try {
      // ComposedFrameVariant exposes serialize() directly — fold-expression
      // dispatch over the held alternative is internal to the variant.
      serializedFrame = std::move(response.frame).serialize();
    } catch (...) {
      // Outbound serialize threw — wire is untouched. Surface as a
      // per-request failure on the inbound chain so the App can clean up
      // its per-stream state; the connection stays up.
      // RocketServerStreamStateHandler will erase the stream entry via
      // the variant's terminal alt.
      //
      // NOTE: this catch block is currently uncovered by unit tests —
      // none of the variant alternatives' serialize() paths can be made
      // to throw via the public API, and the variant is concrete (no
      // injectable throwing alternative). The downstream consumers of
      // the constructed RocketRequestError are covered:
      // RocketServerStreamStateHandlerTest.RequestErrorRoutesViaStreamLookup,
      // ThriftServerTransportAdapterTest.OnTransportRequestWithErrorFiresExceptionUpThrift,
      // and
      // RocketServerIntegrationTest.PerRequestErrorReachesAppAndConnectionSurvives.
      RocketRequestMessage request;
      request.payload = RocketRequestError{
          .ew = folly::exception_wrapper(std::current_exception()),
          .streamId = streamId,
      };
      request.streamId = streamId;
      request.streamType = streamType;
      (void)ctx.fireRead(
          apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
              std::move(request)));
      return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
    }

    return ctx.fireWrite(
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(serializedFrame)));
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
