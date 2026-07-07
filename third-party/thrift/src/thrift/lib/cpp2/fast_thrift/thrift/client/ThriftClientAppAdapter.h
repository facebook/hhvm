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
#include <utility>

#include <folly/ExceptionWrapper.h>
#include <folly/Function.h>
#include <folly/Portability.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/logging/xlog.h>
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
  using CloseCallback = folly::Function<void() noexcept>;

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
    DCHECK(pipeline);
    if (pipeline_) {
      XLOG(FATAL) << "must reset pipeline before setting a new one";
    }
    pipeline_ = pipeline;
    pipelineGuard_ =
        std::make_unique<folly::DelayedDestruction::DestructorGuard>(pipeline);
  }

  /**
   * Release this adapter's hold on the pipeline. Owner must ensure no
   * cross-thread submitWrite callbacks are pending before calling.
   */
  void resetPipeline() noexcept {
    pipeline_ = nullptr;
    pipelineGuard_.reset();
  }

  uint16_t getProtocolId() const noexcept { return protocolId_; }

  void setProtocolId(uint16_t protocolId) noexcept { protocolId_ = protocolId; }

  void setCloseCallback(CloseCallback closeCallback) noexcept {
    closeCallback_ = std::move(closeCallback);
  }

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
  void handlerRemoved() noexcept {
    state_ = State::Closed;
    closeCallback_ = {};
  }
  void onPipelineActive() noexcept;
  void onPipelineInactive() noexcept;
  void onWriteReady() noexcept {}

  // === TailEndpointHandler interface ===

  apache::thrift::fast_thrift::channel_pipeline::Result onRead(
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept;

  void onException(folly::exception_wrapper&& e) noexcept;

 protected:
  ~ThriftClientAppAdapter() override { resetPipeline(); }

 private:
  enum class State { Open, Closing, Closed };

  void submitWrite(ThriftRequestMessage msg) noexcept;
  void submitWriteOnEventBase(ThriftRequestMessage msg) noexcept;

  FOLLY_NOINLINE void handleMissingPipeline(
      RequestResponseHandler handler) noexcept;
  FOLLY_NOINLINE void handleNullContext() noexcept;
  FOLLY_NOINLINE void handleResponseError(
      RequestResponseHandler handler, folly::exception_wrapper ew) noexcept;
  FOLLY_NOINLINE void handleNotOpen(ThriftRequestMessage msg) noexcept;
  FOLLY_NOINLINE void handleWriteError() noexcept;

  apache::thrift::fast_thrift::channel_pipeline::PipelineImpl* pipeline_{
      nullptr};
  std::unique_ptr<folly::DelayedDestruction::DestructorGuard> pipelineGuard_;
  CloseCallback closeCallback_;
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
