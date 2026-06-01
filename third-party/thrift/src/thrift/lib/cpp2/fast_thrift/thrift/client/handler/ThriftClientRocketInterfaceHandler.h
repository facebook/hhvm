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
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/ContextImpl.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/util/RpcKindMapping.h>

namespace apache::thrift::fast_thrift::thrift::client::handler {

/**
 * ThriftClientRocketInterfaceHandler - Converts between Thrift and Rocket
 * message types.
 *
 * This handler is the boundary between the Thrift layer and the Rocket layer.
 * It is the last handler in the Thrift pipeline on the outbound path and the
 * first handler in the Thrift pipeline on the inbound path.
 *
 * Message flow:
 *   Outbound: [Thrift handlers] -> RocketInterfaceHandler -> [Rocket handlers]
 *   Inbound:  [Rocket handlers] -> RocketInterfaceHandler -> [Thrift handlers]
 *
 * Conversions:
 *   Outbound: ThriftRequestMessage → RocketRequestMessage
 *   Inbound:  RocketResponseMessage → ThriftResponseMessage
 *
 * Prerequisites:
 * - Outbound: ThriftRequestMessage.payload.metadata must contain
 *   pre-serialized metadata IOBuf (serialized by the channel)
 * - Inbound: RocketResponseMessage from downstream rocket handler
 */
class ThriftClientRocketInterfaceHandler {
 public:
  ThriftClientRocketInterfaceHandler() = default;

  // === HandlerLifecycle ===

  template <typename Context>
  void handlerAdded(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void handlerRemoved(Context& /*ctx*/) noexcept {}

  // === OutboundHandler ===

  template <typename Context>
  void onPipelineActivated(Context& /*ctx*/) noexcept {}

  /**
   * Convert ThriftRequestMessage to RocketRequestMessage.
   *
   * Expects metadata to be in IOBuf state (already serialized).
   */
  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onWrite(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto request = msg.take<ThriftRequestMessage>();

    // Build RocketRequestMessage from ThriftRequestMessage
    apache::thrift::fast_thrift::rocket::RocketRequestMessage rocketMsg{
        .frame =
            apache::thrift::fast_thrift::rocket::RocketFramePayload{
                .metadata = std::move(request.payload.metadata),
                .data = std::move(request.payload.data),
                .initialRequestN = request.payload.initialRequestN,
                .complete = request.payload.complete,
            },
        .requestHandle = request.requestHandle,
        .frameType = rpcKindToFrameType(request.payload.rpcKind),
    };

    return ctx.fireWrite(
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(rocketMsg)));
  }

  template <typename Context>
  void onPipelineDeactivated(Context& /*ctx*/) noexcept {}

  template <typename Context>
  void onWriteReady(Context& /*ctx*/) noexcept {}

  // === InboundHandler ===

  /**
   * Convert RocketResponseMessage to ThriftResponseMessage.
   *
   * Creates ThriftResponseMessage with ParsedFrame state (raw frame).
   */
  template <typename Context>
  apache::thrift::fast_thrift::channel_pipeline::Result onRead(
      Context& ctx,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto response =
        msg.take<apache::thrift::fast_thrift::rocket::RocketResponseMessage>();

    // Build ThriftResponseMessage with ParsedFrame state
    ThriftResponseMessage thriftMsg{
        .frame = std::move(response.frame),
        .requestHandle = response.requestHandle,
        .requestFrameType = response.requestFrameType,
    };

    return ctx.fireRead(
        apache::thrift::fast_thrift::channel_pipeline::erase_and_box(
            std::move(thriftMsg)));
  }

  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }
};

static_assert(
    apache::thrift::fast_thrift::channel_pipeline::DuplexHandler<
        ThriftClientRocketInterfaceHandler,
        apache::thrift::fast_thrift::channel_pipeline::detail::ContextImpl>,
    "ThriftClientRocketInterfaceHandler must satisfy DuplexHandler concept");

} // namespace apache::thrift::fast_thrift::thrift::client::handler
