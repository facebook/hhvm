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

#include <folly/ExceptionWrapper.h>
#include <folly/lang/Hint.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Handler.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>

namespace apache::thrift::fast_thrift::rocket::client::handler {

/**
 * RocketClientStatsHandler - Stamps transport-level per-request stats onto
 * the inbound response as it travels up the pipeline.
 *
 * Fills the response-side wire sizes that are observable at the rocket layer
 * (`responseWireSizeBytes` = data bytes, `responseMetadataAndPayloadSizeBytes`
 * = metadata + data bytes) directly from the parsed frame. The bridge reads
 * `RocketResponseMessage::stats` when converting to the thrift response and
 * augments it with the thrift-layer-only fields.
 *
 * In-process per-request errors (`RocketResponseError`) and connection-level
 * frames carry no wire payload, so stats are left at their zero defaults.
 */
class RocketClientStatsHandler {
 public:
  RocketClientStatsHandler() = default;

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
    auto& response = msg.get<RocketResponseMessage>();
    if (FOLLY_LIKELY(
            response.payload
                .is<apache::thrift::fast_thrift::frame::read::ParsedFrame>())) {
      const auto& parsed =
          response.payload
              .get<apache::thrift::fast_thrift::frame::read::ParsedFrame>();
      response.stats.responseWireSizeBytes = parsed.dataSize();
      response.stats.responseMetadataAndPayloadSizeBytes = parsed.payloadSize();
    }
    return ctx.fireRead(std::move(msg));
  }

  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }
};

static_assert(
    apache::thrift::fast_thrift::channel_pipeline::InboundHandler<
        RocketClientStatsHandler,
        apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl>,
    "RocketClientStatsHandler must satisfy InboundHandler concept");

} // namespace apache::thrift::fast_thrift::rocket::client::handler
