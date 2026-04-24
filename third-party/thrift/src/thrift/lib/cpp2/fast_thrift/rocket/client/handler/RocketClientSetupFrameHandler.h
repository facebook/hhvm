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

#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Handler.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>

namespace apache::thrift::fast_thrift::rocket::client::handler {

/**
 * RocketClientSetupFrameHandler - Pipeline handler for client-side SETUP frame
 * handling.
 *
 * This handler sends the RSocket SETUP frame when the connection is
 * established (via onConnect). The setup frame is sent immediately when
 * the pipeline fires the connect event, before any application requests.
 *
 * The handler supports a factory function that is called at connect time
 * for dynamic metadata creation.
 *
 * Pipeline position: (Inbound handler - sends on connect, passes through reads)
 *   App <- RocketClientSetupFrameHandler <- StreamHandler <- FrameHandler <-
 * Transport
 */
class RocketClientSetupFrameHandler {
 public:
  // RSocket protocol version
  static constexpr uint16_t kRSocketMajorVersion = 1;
  static constexpr uint16_t kRSocketMinorVersion = 0;

  // Keepalive and lifetime constants (in milliseconds).
  // These are set to the max allowed value (2^31 - 1) since client-side
  // keepalive is not supported (server-to-client keepalive is supported).
  static constexpr uint32_t kMaxKeepaliveTime = (1u << 31) - 1;
  static constexpr uint32_t kMaxLifetime = (1u << 31) - 1;

  // Factory function type for dynamic metadata creation at connect time
  using SetupFactory = std::function<std::pair<
      std::unique_ptr<folly::IOBuf>,
      std::unique_ptr<folly::IOBuf>>()>;

  /**
   * Construct with a factory function for dynamic metadata creation.
   * The factory is called at connect time to create the metadata/data.
   */
  explicit RocketClientSetupFrameHandler(SetupFactory factory)
      : setupFactory_(std::move(factory)) {}

  // === HandlerLifecycle ===

  template <typename Context>
  void handlerAdded(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {}

  // === InboundHandler ===

  template <typename Context>
  void onPipelineActivated(Context& ctx) noexcept {
    if (setupSent_) {
      return;
    }

    setupSent_ = true;
    // Since we have established a connection, we should send the setup
    // frame immediately.
    auto result = ctx.fireWrite(
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            makeSetupMessage()));
    if (result ==
        apache::thrift::fast_thrift::channel_pipeline::Result::Error) {
      // If we failed to send the setup frame, we should close the pipeline
      ctx.close();
      return;
    }
  }

  template <typename Context>
  void onReadReady(Context& /*ctx*/) noexcept {}

  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onRead(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    return ctx.fireRead(std::move(msg));
  }

  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }

  // === OutboundHandler (for disconnect notification) ===

  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onWrite(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    return ctx.fireWrite(std::move(msg));
  }

  template <typename Context>
  void onPipelineDeactivated(Context& /*ctx*/) noexcept {
    setupSent_ = false;
  }

  template <typename Context>
  void onWriteReady(Context& /*ctx*/) noexcept {}

 private:
  RocketRequestMessage makeSetupMessage() {
    auto [metadata, data] = setupFactory_();
    auto serializedFrame = apache::thrift::fast_thrift::frame::write::serialize(
        apache::thrift::fast_thrift::frame::write::SetupHeader{
            .majorVersion = kRSocketMajorVersion,
            .minorVersion = kRSocketMinorVersion,
            .keepaliveTime = kMaxKeepaliveTime,
            .maxLifetime = kMaxLifetime,
        },
        std::move(metadata),
        std::move(data));

    return RocketRequestMessage{
        .frame = std::move(serializedFrame),
        .frameType = apache::thrift::fast_thrift::frame::FrameType::SETUP,
    };
  }

  // Factory function for creating setup metadata and data
  bool setupSent_{false};
  SetupFactory setupFactory_;
};

static_assert(
    apache::thrift::fast_thrift::channel_pipeline::InboundHandler<
        RocketClientSetupFrameHandler,
        apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl> &&
        apache::thrift::fast_thrift::channel_pipeline::OutboundHandler<
            RocketClientSetupFrameHandler,
            apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl>,
    "RocketClientSetupFrameHandler must satisfy InboundHandler and OutboundHandler concepts");

} // namespace apache::thrift::fast_thrift::rocket::client::handler
