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

#include <memory>
#include <string_view>

#include <folly/ExceptionWrapper.h>
#include <folly/Expected.h>
#include <folly/Function.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/async/RpcOptions.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/EndpointAdapter.h>

namespace apache::thrift::fast_thrift::thrift::client {

/**
 * Handler invoked by an adapter when a request-response RPC completes.
 *
 * On success, receives the response data IOBuf; on failure (transport
 * error, undeclared exception, pipeline error, etc.) receives an
 * exception_wrapper. Always called on the adapter's EventBase thread.
 *
 * Response metadata is consumed and discarded by the adapter — generated
 * code only ever sees the data IOBuf or an exception_wrapper.
 */
using RequestResponseHandler =
    folly::Function<void(folly::Expected<
                         std::unique_ptr<folly::IOBuf>,
                         folly::exception_wrapper>&&) noexcept>;

/**
 * ClientInboundAppAdapter — client-side inbound endpoint concept.
 *
 * Must satisfy TailEndpointHandler (onRead + onException + lifecycle).
 */
template <typename A>
concept ClientInboundAppAdapter = channel_pipeline::TailEndpointHandler<A>;

/**
 * ClientOutboundAppAdapter concept — sends messages from client application
 * to pipeline.
 *
 * The client calls sendRequestResponse() to issue a request-response RPC; the
 * adapter is responsible for building the wire-level request message and
 * pushing it down the pipeline.
 *
 * Note: This is an interface contract, not an owned object.
 * It does NOT require DelayedDestructionBase.
 */
template <typename O>
concept ClientOutboundAppAdapter = requires(
    O o,
    const apache::thrift::RpcOptions& rpcOptions,
    std::string_view methodName,
    apache::thrift::RpcKind rpcKind,
    std::unique_ptr<folly::IOBuf> data) {
  {
    o.sendRequestResponse(
        rpcOptions,
        methodName,
        rpcKind,
        std::move(data),
        std::declval<RequestResponseHandler>())
  } noexcept -> std::same_as<void>;

  { o.getProtocolId() } noexcept -> std::same_as<uint16_t>;
};

} // namespace apache::thrift::fast_thrift::thrift::client
