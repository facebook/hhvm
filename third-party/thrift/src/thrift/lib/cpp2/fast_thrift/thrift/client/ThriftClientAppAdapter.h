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
#include <folly/io/IOBuf.h>
#include <folly/io/async/DelayedDestruction.h>
#include <thrift/lib/cpp2/async/RpcOptions.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/FastThriftAdapterBase.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/common/ClientAppAdapter.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * ThriftClientAppAdapter — lightweight, non-owning base class for generated
 * fast thrift clients.
 *
 * Unlike ThriftClientChannel (which adapts the Thrift RequestChannel API),
 * this class bypasses RequestChannel entirely and interacts directly with
 * the pipeline. Generated clients hold one of these and call
 * sendRequestResponse() to issue request-response RPCs.
 *
 * Non-owning: holds a raw PipelineImpl*, does NOT own the transport or
 * pipeline (the pipeline owner keeps those alive).
 *
 * Implements the ClientInboundAppAdapter concept (onRead / onException).
 */
class ThriftClientAppAdapter : public folly::DelayedDestruction,
                               public FastThriftAdapterBase {
 public:
  using RequestResponseHandler =
      apache::thrift::fast_thrift::thrift::client::RequestResponseHandler;

  using Ptr = std::unique_ptr<ThriftClientAppAdapter, Destructor>;

  ThriftClientAppAdapter() = default;

  explicit ThriftClientAppAdapter(uint16_t protocolId)
      : protocolId_{protocolId} {}

  ThriftClientAppAdapter(const ThriftClientAppAdapter&) = delete;
  ThriftClientAppAdapter& operator=(const ThriftClientAppAdapter&) = delete;
  ThriftClientAppAdapter(ThriftClientAppAdapter&&) = delete;
  ThriftClientAppAdapter& operator=(ThriftClientAppAdapter&&) = delete;

  void setPipeline(
      apache::thrift::fast_thrift::channel_pipeline::PipelineImpl* pipeline) {
    pipeline_ = pipeline;
  }

  uint16_t getProtocolId() const noexcept { return protocolId_; }

  void setProtocolId(uint16_t protocolId) noexcept { protocolId_ = protocolId; }

  /**
   * Issue a request-response RPC.
   *
   * Builds the request metadata + message internally so callers (generated
   * code) don't need to know about ThriftRequestMessage. If called on the
   * pipeline's EventBase thread, fires immediately; otherwise schedules on
   * the EventBase.
   */
  void sendRequestResponse(
      const apache::thrift::RpcOptions& rpcOptions,
      std::string_view methodName,
      apache::thrift::RpcKind rpcKind,
      std::unique_ptr<folly::IOBuf> data,
      RequestResponseHandler handler) noexcept;

  // === TailEndpointHandler lifecycle ===
  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}
  void onWriteReady() noexcept {}

  // === TailEndpointHandler interface ===

  apache::thrift::fast_thrift::channel_pipeline::Result onRead(
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept;

  void onException(folly::exception_wrapper&& e) noexcept;

 protected:
  ~ThriftClientAppAdapter() override = default;

 private:
  enum class State { Open, Closing, Closed };

  void submitWrite(ThriftRequestMessage msg) noexcept;
  void submitWriteOnEventBase(ThriftRequestMessage msg) noexcept;

  apache::thrift::fast_thrift::channel_pipeline::PipelineImpl* pipeline_{
      nullptr};
  uint16_t protocolId_{0};
  State state_{State::Open};
  folly::exception_wrapper lastError_;
};

static_assert(
    client::ClientOutboundAppAdapter<ThriftClientAppAdapter>,
    "ThriftClientAppAdapter must satisfy ClientOutboundAppAdapter concept");

static_assert(
    client::ClientInboundAppAdapter<ThriftClientAppAdapter>,
    "ThriftClientAppAdapter must satisfy ClientInboundAppAdapter concept");

} // namespace apache::thrift::fast_thrift::thrift
