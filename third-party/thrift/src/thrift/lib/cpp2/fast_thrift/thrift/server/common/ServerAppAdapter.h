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

#include <concepts>
#include <ranges>
#include <utility>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/EndpointAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/common/Messages.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * ServerInboundAppAdapter — server-side inbound endpoint concept. The
 * pipeline calls these methods on an adapter when a request arrives.
 * Mirrors ClientInboundAppAdapter on the client side.
 */
template <typename T>
concept ServerInboundAppAdapter = channel_pipeline::TailEndpointHandler<T>;

/**
 * ServerOutboundAppAdapter — server-side outbound endpoint concept.
 * Codegen builds a ThriftServerResponseMessage via the helpers in
 * util/ResponsePayloads.h and hands it to writeResponse(). The adapter
 * also owns the close-initiation API: close() fires a
 * ThriftServerEventType::CloseConnection through the pipeline so the
 * resident ThriftServerConnectionCloseHandler can drive the terminal
 * state machine. Mirrors ClientOutboundAppAdapter on the client side.
 */
template <typename T>
concept ServerOutboundAppAdapter =
    requires(T& t, ThriftServerResponseMessage&& message) {
      { t.writeResponse(std::move(message)) } noexcept -> std::same_as<void>;
      { t.close() } noexcept;
    };

/**
 * ServerComposableAppAdapter — wiring + routing-metadata concept.
 * An adapter satisfies this when it can be plugged into a routing fabric
 * (e.g. ThriftServerCompositeAppAdapter): it declares the method names it
 * owns and accepts a pipeline pointer at setup time. Orthogonal to
 * inbound/outbound data flow — purely about composition wiring.
 */
template <typename T>
concept ServerComposableAppAdapter =
    requires(T& t, channel_pipeline::PipelineImpl* pipe) {
      { t.methodNames() } -> std::ranges::range;
      { t.setPipeline(pipe) } noexcept;
    };

} // namespace apache::thrift::fast_thrift::thrift
