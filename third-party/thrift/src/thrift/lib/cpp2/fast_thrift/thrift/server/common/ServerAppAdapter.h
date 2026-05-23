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
#include <memory>
#include <ranges>
#include <string>
#include <utility>

#include <folly/ExceptionWrapper.h>
#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/EndpointAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/frame/ErrorCode.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

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
 * Codegen calls these from request handlers to push responses back into
 * the pipeline. Mirrors ClientOutboundAppAdapter on the client side.
 *
 * Templated helpers (writeSuccessResponse, writeDeclaredException) are
 * not part of the concept because concepts can't easily constrain
 * member templates over arbitrary Writer/Presult types; satisfying
 * implementations are expected to provide them as well.
 */
template <typename T>
concept ServerOutboundAppAdapter = requires(
    T& t,
    uint32_t streamId,
    std::unique_ptr<folly::IOBuf> data,
    std::unique_ptr<apache::thrift::ResponseRpcMetadata> metadata,
    apache::thrift::fast_thrift::frame::ErrorCode errorCode,
    apache::thrift::ResponseRpcErrorCode rpcErrorCode,
    std::string message,
    const folly::exception_wrapper& ew,
    apache::thrift::ErrorBlame blame) {
  {
    t.writeResponse(streamId, std::move(data), std::move(metadata))
  } noexcept -> std::same_as<channel_pipeline::Result>;
  {
    t.writeError(streamId, std::move(data), errorCode)
  } noexcept -> std::same_as<channel_pipeline::Result>;
  {
    t.writeFrameworkError(streamId, rpcErrorCode, std::move(message))
  } noexcept -> std::same_as<channel_pipeline::Result>;
  {
    t.writeUnknownException(streamId, ew, blame)
  } noexcept -> std::same_as<channel_pipeline::Result>;
  { t.startDrain() } noexcept;
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
