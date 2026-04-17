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
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/EndpointAdapter.h>

namespace apache::thrift::fast_thrift::transport {

/**
 * InboundTransportHandler concept - receives data from the transport layer.
 *
 * The transport calls these methods when data arrives from the network.
 * Typically implemented by the pipeline's head handler.
 *
 * This is a push model — the transport pushes data to the handler.
 *
 * Note: This is an interface contract, not an owned object.
 * It does NOT require DelayedDestructionBase.
 */
template <typename H>
concept InboundTransportHandler = requires(
    H h, channel_pipeline::BytesPtr bytes, folly::exception_wrapper e) {
  { h.onRead(std::move(bytes)) } noexcept -> std::same_as<void>;
  { h.onClose(std::move(e)) } noexcept -> std::same_as<void>;
};

/**
 * OutboundTransportHandler concept — refines TailEndpointHandler with
 * backpressure control.
 *
 * The pipeline calls onWrite() to push data toward the transport.
 * pauseRead/resumeRead provide backpressure signaling.
 *
 * Note: This is an interface contract, not an owned object.
 * It does NOT require DelayedDestructionBase.
 */
template <typename T>
concept OutboundTransportHandler =
    channel_pipeline::TailEndpointHandler<T> && requires(T t) {
      { t.pauseRead() } noexcept -> std::same_as<void>;
      { t.resumeRead() } noexcept -> std::same_as<void>;
    };

} // namespace apache::thrift::fast_thrift::transport
