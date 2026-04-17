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
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>

namespace apache::thrift::fast_thrift::rocket::client {

/**
 * RocketClientAppInboundHandler — pipeline-facing endpoint concept.
 *
 * Equivalent to channel_pipeline::TailEndpointHandler. Satisfied by
 * RocketClientAppAdapter, which bridges the pipeline to the consumer's
 * onResponse/onError callbacks.
 */
template <typename H>
concept RocketClientAppInboundHandler = requires(
    H h, channel_pipeline::TypeErasedBox&& msg, folly::exception_wrapper&& e) {
  {
    h.onRead(std::move(msg))
  } noexcept -> std::same_as<channel_pipeline::Result>;
  { h.onException(std::move(e)) } noexcept -> std::same_as<void>;
};

/**
 * RocketClientAppOutboundHandler — concept for sending messages into
 * the rocket client pipeline on the outbound (request) path.
 *
 * Satisfied by RocketClientAppAdapter. Consumers use this interface
 * to send rocket request messages.
 */
template <typename H>
concept RocketClientAppOutboundHandler =
    requires(H h, RocketRequestMessage&& msg) {
      {
        h.write(std::move(msg))
      } noexcept -> std::same_as<channel_pipeline::Result>;
    };

} // namespace apache::thrift::fast_thrift::rocket::client
