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
#include <folly/Portability.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/EventBase.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>
#include <thrift/lib/cpp2/transport/core/RpcMetadataUtil.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * ThriftClientChannel - Thrift RequestChannel backed by fast_thrift
 * pipeline.
 *
 * This adapter translates Thrift's RequestChannel API to the fast_thrift
 * channel pipeline. Only request-response is implemented.
 *
 * This class implements the ClientInboundAppAdapter concept to receive
 * responses from the pipeline via onRead().
 *
 * Usage:
 *   auto channel = ThriftClientChannel::newChannel(evb, pipeline);
 *   auto client = MyService::newClient(std::move(channel));
 */
class ThriftClientChannel : public apache::thrift::RequestChannel {
 public:
  using UniquePtr = std::
      unique_ptr<ThriftClientChannel, folly::DelayedDestruction::Destructor>;

  ThriftClientChannel(const ThriftClientChannel&) = delete;
  ThriftClientChannel& operator=(const ThriftClientChannel&) = delete;
  ThriftClientChannel(ThriftClientChannel&&) = delete;
  ThriftClientChannel& operator=(ThriftClientChannel&&) = delete;

  // Factory method
  static UniquePtr newChannel(
      folly::EventBase* evb,
      uint16_t protocolId = apache::thrift::protocol::T_COMPACT_PROTOCOL);

  // Set the pipeline - must be called before sending requests. The
  // channel installs a DestructorGuard on the pipeline so the pipeline
  // cannot be destroyed while it is attached. Caller releases this hold
  // via resetPipeline() (or destroys the channel, which calls
  // resetPipeline implicitly).
  void setPipeline(
      apache::thrift::fast_thrift::channel_pipeline::PipelineImpl* pipeline);

  // Release the channel's hold on the pipeline. After this returns the
  // pipeline may be destroyed (modulo other guards) and sendRequest must
  // not be called.
  void resetPipeline() noexcept;

  using RequestChannel::sendRequestNoResponse;
  using RequestChannel::sendRequestResponse;
  using RequestChannel::sendRequestSink;
  using RequestChannel::sendRequestStream;

  // === Request-Response (IMPLEMENTED) ===
  void sendRequestResponse(
      const apache::thrift::RpcOptions& options,
      apache::thrift::MethodMetadata&& methodMetadata,
      apache::thrift::SerializedRequest&& request,
      std::shared_ptr<apache::thrift::transport::THeader> header,
      apache::thrift::RequestClientCallback::Ptr cob,
      std::unique_ptr<folly::IOBuf> frameworkMetadata) override;

  void sendRequestResponse(
      apache::thrift::RpcOptions&& options,
      apache::thrift::MethodMetadata&& methodMetadata,
      apache::thrift::SerializedRequest&& request,
      std::shared_ptr<apache::thrift::transport::THeader> header,
      apache::thrift::RequestClientCallback::Ptr cob,
      std::unique_ptr<folly::IOBuf> frameworkMetadata) override;

  // === Unsupported Operations (LOG(FATAL)) ===
  void sendRequestNoResponse(
      const apache::thrift::RpcOptions&,
      apache::thrift::MethodMetadata&&,
      apache::thrift::SerializedRequest&&,
      std::shared_ptr<apache::thrift::transport::THeader>,
      apache::thrift::RequestClientCallback::Ptr,
      std::unique_ptr<folly::IOBuf>) override {
    XLOG(FATAL) << "Not implemented";
  }

  void sendRequestStream(
      const apache::thrift::RpcOptions&,
      apache::thrift::MethodMetadata&&,
      apache::thrift::SerializedRequest&&,
      std::shared_ptr<apache::thrift::transport::THeader>,
      apache::thrift::StreamClientCallback*,
      std::unique_ptr<folly::IOBuf>) override {
    XLOG(FATAL) << "Not implemented";
  }

  void sendRequestSink(
      const apache::thrift::RpcOptions&,
      apache::thrift::MethodMetadata&&,
      apache::thrift::SerializedRequest&&,
      std::shared_ptr<apache::thrift::transport::THeader>,
      apache::thrift::SinkClientCallback*,
      std::unique_ptr<folly::IOBuf>) override {
    XLOG(FATAL) << "Not implemented";
  }

  void setCloseCallback(apache::thrift::CloseCallback*) override {
    XLOG(FATAL) << "Not implemented";
  }

  folly::EventBase* getEventBase() const override { return evb_; }
  uint16_t getProtocolId() override { return protocolId_; }

  // === TailEndpointHandler lifecycle ===
  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept { state_ = State::Closed; }
  void onPipelineActive() noexcept;
  void onPipelineInactive() noexcept;
  void onWriteReady() noexcept {}

  // === TailEndpointHandler interface ===
  // Called by the pipeline when a response message arrives
  apache::thrift::fast_thrift::channel_pipeline::Result onRead(
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept;

  // Called by the pipeline when an exception propagates to the application
  void onException(folly::exception_wrapper&& e) noexcept;

 protected:
  ThriftClientChannel(folly::EventBase* evb, uint16_t protocolId);
  ~ThriftClientChannel() override;

 private:
  enum class State { Open, Closing, Closed };

  // Common implementation for sending thrift requests
  void sendRequestInternal(
      const apache::thrift::RpcOptions& options,
      apache::thrift::MethodMetadata&& methodMetadata,
      apache::thrift::SerializedRequest&& request,
      std::shared_ptr<apache::thrift::transport::THeader> header,
      apache::thrift::RequestClientCallback::Ptr callbackPtr,
      std::unique_ptr<folly::IOBuf> frameworkMetadata);

  // Handle incoming request-response messages
  void handleRequestResponse(
      ThriftResponseMessage&& response,
      apache::thrift::RequestClientCallback::Ptr callback);

  // Error handling helpers (moved out of line to keep hot path small)
  FOLLY_NOINLINE void handleMissingPipeline(
      apache::thrift::RequestClientCallback::Ptr callbackPtr) noexcept;
  FOLLY_NOINLINE void handleNotOpen(
      apache::thrift::RequestClientCallback::Ptr callbackPtr) noexcept;
  FOLLY_NOINLINE void handleWriteError() noexcept;
  FOLLY_NOINLINE void handleMetadataError(
      apache::thrift::RequestClientCallback::Ptr callback,
      folly::exception_wrapper error) noexcept;
  FOLLY_NOINLINE void handleNullContext() noexcept;
  FOLLY_NOINLINE void handleResponseError(
      apache::thrift::RequestClientCallback::Ptr callback,
      folly::exception_wrapper ew) noexcept;

  // Per-request context allocated on the outbound path and consumed on the
  // inbound path. Transported opaquely as a TypeErasedPtr on the message
  // variants; only this channel casts it back. Owns the pending Ptr — its
  // auto-detach deleter fires onResponseError if the holder is destructed
  // without firing.
  struct ChannelCallbackContext {
    apache::thrift::RequestClientCallback::Ptr cb;

    explicit ChannelCallbackContext(
        apache::thrift::RequestClientCallback::Ptr cb)
        : cb(std::move(cb)) {}
  };

  folly::EventBase* evb_;
  apache::thrift::fast_thrift::channel_pipeline::PipelineImpl* pipeline_{
      nullptr};
  std::unique_ptr<folly::DelayedDestruction::DestructorGuard> pipelineGuard_;
  uint16_t protocolId_;
  State state_{State::Open};
  folly::exception_wrapper lastError_;
};

} // namespace apache::thrift::fast_thrift::thrift
