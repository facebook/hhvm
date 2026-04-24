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
#include <cstring>

#include <folly/lang/Hint.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameViews.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>

namespace apache::thrift::fast_thrift::rocket::server::handler {

namespace setup_error {
constexpr uint32_t kInvalidSetup = 0x00000001;
constexpr uint32_t kUnsupportedSetup = 0x00000002;
} // namespace setup_error

struct SetupParameters {
  uint16_t majorVersion{0};
  uint16_t minorVersion{0};
  uint32_t keepaliveTime{0};
  uint32_t maxLifetime{0};
  bool hasLease{false};
};

/**
 * RocketServerSetupFrameHandler - Pipeline handler for RSocket SETUP frame
 * validation.
 *
 * This handler sits before RocketServerStreamStateHandler in the pipeline,
 * validating the SETUP frame on connection establishment. After setup
 * completes, it becomes a near-zero-cost passthrough.
 *
 * Pipeline position:
 *   Transport <-> FrameHandler <-> RocketServerSetupFrameHandler <->
 * StreamHandler
 *   <-> App
 *
 * Two-phase design:
 * - Phase 1 (awaiting setup): Validates the first frame is SETUP with
 *   correct version and timer values. Rejects invalid setups with ERROR.
 * - Phase 2 (setup complete): Passthrough for all frames, but rejects
 *   duplicate SETUP frames.
 */
class RocketServerSetupFrameHandler {
 public:
  static constexpr uint16_t kRSocketMajorVersion = 1;

  RocketServerSetupFrameHandler() = default;

  // === HandlerLifecycle ===

  template <typename Context>
  void handlerAdded(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {
    setupComplete_ = false;
    params_ = SetupParameters{};
  }

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
    if (FOLLY_UNLIKELY(!setupComplete_)) {
      return handleAwaitingSetup(ctx, std::move(msg));
    }

    auto& frame =
        msg.get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();
    if (FOLLY_UNLIKELY(
            frame.type() ==
            apache::thrift::fast_thrift::frame::FrameType::SETUP)) {
      XLOG(ERR) << "Received duplicate SETUP frame after setup complete";
      return sendError(
          ctx, setup_error::kInvalidSetup, "SETUP frame already received");
    }

    return ctx.fireRead(std::move(msg));
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
    return ctx.fireWrite(std::move(msg));
  }

  template <typename Context>
  void onPipelineInactive(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onWriteReady(Context& /*ctx*/) noexcept {}

  // === Accessors ===

  bool isSetupComplete() const noexcept { return setupComplete_; }

  const SetupParameters& setupParameters() const noexcept { return params_; }

 private:
  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result handleAwaitingSetup(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto& frame =
        msg.get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();

    if (frame.type() != apache::thrift::fast_thrift::frame::FrameType::SETUP) {
      XLOG(ERR) << "Expected SETUP frame, got " << frame.typeName();
      return sendError(
          ctx, setup_error::kInvalidSetup, "First frame must be SETUP");
    }

    apache::thrift::fast_thrift::frame::read::SetupView view(frame);

    uint16_t majorVersion = view.majorVersion();
    if (majorVersion != kRSocketMajorVersion) {
      XLOG(ERR) << "Unsupported RSocket major version: " << majorVersion;
      return sendError(
          ctx, setup_error::kUnsupportedSetup, "Unsupported major version");
    }

    uint32_t keepaliveTime = view.keepaliveTime();
    if (keepaliveTime == 0) {
      XLOG(ERR) << "SETUP keepaliveTime must be > 0";
      return sendError(
          ctx, setup_error::kInvalidSetup, "keepaliveTime must be > 0");
    }

    uint32_t maxLifetime = view.maxLifetime();
    if (maxLifetime == 0) {
      XLOG(ERR) << "SETUP maxLifetime must be > 0";
      return sendError(
          ctx, setup_error::kInvalidSetup, "maxLifetime must be > 0");
    }

    params_.majorVersion = majorVersion;
    params_.minorVersion = view.minorVersion();
    params_.keepaliveTime = keepaliveTime;
    params_.maxLifetime = maxLifetime;
    params_.hasLease = view.hasLease();

    setupComplete_ = true;

    // SETUP is a connection-level protocol frame consumed by this handler.
    // It is not forwarded to downstream handlers.
    return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
  }

  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result sendError(
      Context& ctx, uint32_t errorCode, const char* message) noexcept {
    try {
      auto errorData = ctx.copyBuffer(message, std::strlen(message));
      auto errorFrame = apache::thrift::fast_thrift::frame::write::serialize(
          apache::thrift::fast_thrift::frame::write::ErrorHeader{
              .streamId = 0, .errorCode = errorCode},
          nullptr,
          std::move(errorData));

      auto writeResult = ctx.fireWrite(
          apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
              std::move(errorFrame)));
      if (writeResult !=
          apache::thrift::fast_thrift::channel_pipeline::Result::Success) {
        XLOG(WARN) << "Failed to deliver ERROR frame for errorCode="
                   << errorCode;
      }
    } catch (...) {
      XLOG(ERR) << "Failed to send ERROR frame for errorCode=" << errorCode;
    }

    // Per RSocket spec, setup errors require connection termination.
    ctx.close();

    return apache::thrift::fast_thrift::channel_pipeline::Result::Error;
  }

  bool setupComplete_{false};
  SetupParameters params_;
};

} // namespace apache::thrift::fast_thrift::rocket::server::handler
