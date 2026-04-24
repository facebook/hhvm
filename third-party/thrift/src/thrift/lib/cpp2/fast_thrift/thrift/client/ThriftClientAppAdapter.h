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

#include <fmt/core.h>
#include <folly/ExceptionWrapper.h>
#include <folly/Expected.h>
#include <folly/Function.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/DirectStreamMap.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/FastThriftAdapterBase.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/common/ClientAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/util/ErrorDecoding.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * ThriftClientAppAdapter — lightweight, non-owning base class for generated
 * fast thrift clients.
 *
 * Unlike ThriftClientChannel (which adapts the Thrift RequestChannel API),
 * this class bypasses RequestChannel entirely and interacts directly with
 * the pipeline. Generated clients derive from this and call
 * write() to issue RPCs.
 *
 * Non-owning: holds a raw PipelineImpl*, does NOT own the transport or
 * pipeline (the pipeline owner keeps those alive).
 *
 * Implements the ClientInboundAppAdapter concept (onRead / onException).
 */
class ThriftClientAppAdapter : public folly::DelayedDestruction,
                               public FastThriftAdapterBase {
 public:
  using ResponseHandler =
      folly::Function<void(folly::Expected<
                           ThriftResponseMessage,
                           folly::exception_wrapper>&&) noexcept>;

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
   * Send a request message into the pipeline.
   * If called on the pipeline's EventBase thread, fires immediately.
   * Otherwise, schedules on the EventBase.
   *
   * If fireWrite returns Error, removes the handler from the pending map
   * and invokes it with an exception_wrapper.
   */
  void write(
      ResponseHandler handler,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    if (FOLLY_UNLIKELY(!pipeline_)) {
      handler(
          folly::makeUnexpected(
              folly::make_exception_wrapper<
                  apache::thrift::TApplicationException>(
                  apache::thrift::TApplicationException::INTERNAL_ERROR,
                  "Pipeline not set")));
      return;
    }

    auto* eb = pipeline_->eventBase();
    if (eb->isInEventBaseThread()) {
      writeOnEventBase(std::move(handler), std::move(msg));
    } else {
      eb->runInEventBaseThread(
          [this, msg = std::move(msg), handler = std::move(handler)]() mutable {
            writeOnEventBase(std::move(handler), std::move(msg));
          });
    }
  }

  // === TailEndpointHandler lifecycle ===
  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}
  void onWriteReady() noexcept {}

  // === TailEndpointHandler interface ===

  apache::thrift::fast_thrift::channel_pipeline::Result onRead(
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept {
    auto response = msg.take<ThriftResponseMessage>();
    auto it = pendingRequests_.find(response.requestHandle);
    if (it == pendingRequests_.end()) {
      XLOG(WARN) << "Response for unknown handle: " << response.requestHandle;
      return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
    }
    auto handler = std::move(it->second);
    pendingRequests_.erase(it);

    DCHECK(
        response.requestFrameType ==
        apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE)
        << "Unsupported frame type: "
        << static_cast<int>(response.requestFrameType);

    handler(handleRequestResponse(std::move(response)));
    return apache::thrift::fast_thrift::channel_pipeline::Result::Success;
  }

  void onException(folly::exception_wrapper&& e) noexcept {
    XLOG(ERR) << "Pipeline exception: " << e.what();
    if (state_ == State::Closed) {
      return;
    }
    if (state_ == State::Open) {
      lastError_ = e;
    }

    auto* tex =
        e.get_exception<apache::thrift::transport::TTransportException>();
    if (tex &&
        tex->getType() ==
            apache::thrift::transport::TTransportException::NOT_OPEN) {
      // CONNECTION_CLOSE: graceful drain — reject new writes, let inflight
      // responses complete
      state_ = State::Closing;
      return;
    }

    state_ = State::Closed;

    pendingRequests_.forEach([&](uint64_t, ResponseHandler& handler) {
      handler(folly::makeUnexpected(e));
    });
    pendingRequests_.clear();

    if (pipeline_) {
      pipeline_->close();
    }
  }

 protected:
  ~ThriftClientAppAdapter() override = default;

 private:
  enum class State { Open, Closing, Closed };

  void writeOnEventBase(
      ResponseHandler handler,
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox
          msg) noexcept {
    if (FOLLY_UNLIKELY(state_ != State::Open)) {
      handler(
          folly::makeUnexpected(
              folly::make_exception_wrapper<
                  apache::thrift::transport::TTransportException>(
                  apache::thrift::transport::TTransportException::NOT_OPEN,
                  lastError_ ? fmt::format(
                                   "Connection not open: {}",
                                   lastError_.what().toStdString())
                             : "Connection not open")));
      return;
    }
    auto requestId = nextRequestId_++;
    auto it = pendingRequests_.emplace(requestId, std::move(handler)).first;
    ThriftRequestMessage& reqMsg = msg.get<ThriftRequestMessage>();
    reqMsg.requestHandle = requestId;
    auto result = pipeline_->fireWrite(std::move(msg));
    if (FOLLY_UNLIKELY(
            result ==
            apache::thrift::fast_thrift::channel_pipeline::Result::Error)) {
      auto respHandler = std::move(it->second);
      respHandler(
          folly::makeUnexpected(
              folly::make_exception_wrapper<
                  apache::thrift::TApplicationException>(
                  apache::thrift::TApplicationException::INTERNAL_ERROR,
                  "Pipeline write failed")));
      pendingRequests_.erase(it);
    }
  }

  apache::thrift::fast_thrift::channel_pipeline::PipelineImpl* pipeline_{
      nullptr};
  uint16_t protocolId_{0};
  apache::thrift::fast_thrift::frame::read::DirectStreamMap<
      ResponseHandler,
      uint32_t,
      apache::thrift::fast_thrift::frame::read::SequentialIndex>
      pendingRequests_;
  uint32_t nextRequestId_{0};
  State state_{State::Open};
  folly::exception_wrapper lastError_;
};

static_assert(
    ClientOutboundAppAdapter<ThriftClientAppAdapter>,
    "ThriftClientAppAdapter must satisfy ClientOutboundAppAdapter concept");

static_assert(
    ClientInboundAppAdapter<ThriftClientAppAdapter>,
    "ThriftClientAppAdapter must satisfy ClientInboundAppAdapter concept");

} // namespace apache::thrift::fast_thrift::thrift
