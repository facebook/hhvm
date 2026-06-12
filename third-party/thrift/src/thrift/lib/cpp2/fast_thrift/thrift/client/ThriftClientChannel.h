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

#include <folly/ExceptionWrapper.h>
#include <folly/Portability.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/EventBase.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/common/RocketClientConnection.h>
#include <thrift/lib/cpp2/transport/core/RpcMetadataUtil.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * ThriftClientChannel - Thrift RequestChannel backed by the fast_thrift
 * rocket transport.
 *
 * This is a flavor of fast_thrift where only the transport changed: the
 * channel owns all thrift-layer work itself (request metadata, serialization,
 * response decoding, ClientReceiveState construction) — mirroring the legacy
 * RocketClientChannel — and drives the fast_thrift rocket pipeline directly
 * via its RocketClientAppAdapter instead of going through a separate thrift
 * pipeline.
 *
 * The caller builds the RocketClientConnection (the rocket pipeline) and hands
 * it to newChannel(); the channel takes ownership and registers itself as the
 * connection's response/lifecycle sink.
 *
 * Only request-response is implemented.
 *
 * Usage:
 *   auto connection = makeRocketClientConnection(...);  // caller-built
 *   auto channel = ThriftClientChannel::newChannel(std::move(connection));
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

  // Factory method. Takes ownership of a caller-built rocket connection.
  static UniquePtr newChannel(
      std::unique_ptr<rocket::client::RocketClientConnection> connection,
      uint16_t protocolId = apache::thrift::protocol::T_COMPACT_PROTOCOL);

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

 protected:
  ThriftClientChannel(
      std::unique_ptr<rocket::client::RocketClientConnection> connection,
      uint16_t protocolId);
  ~ThriftClientChannel() override;

 private:
  enum class State { Open, Closing, Closed };

  // Per-request context allocated on the outbound path and consumed on the
  // inbound path. Transported opaquely as a TypeErasedPtr on the rocket
  // message; only this channel casts it back. Owns the pending Ptr — its
  // auto-detach deleter fires onResponseError if the holder is destructed
  // without firing.
  struct ChannelCallbackContext {
    apache::thrift::RequestClientCallback::Ptr cb;

    explicit ChannelCallbackContext(
        apache::thrift::RequestClientCallback::Ptr cb)
        : cb(std::move(cb)) {}
  };

  // Common implementation for sending thrift requests.
  void sendRequestInternal(
      const apache::thrift::RpcOptions& options,
      apache::thrift::MethodMetadata&& methodMetadata,
      apache::thrift::SerializedRequest&& request,
      std::shared_ptr<apache::thrift::transport::THeader> header,
      apache::thrift::RequestClientCallback::Ptr callbackPtr,
      std::unique_ptr<folly::IOBuf> frameworkMetadata);

  // Registered as the rocket connection's response sink. Receives a
  // rocket::RocketResponseMessage, decodes it, and resolves the callback.
  apache::thrift::fast_thrift::channel_pipeline::Result onResponse(
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox&&
          msg) noexcept;

  // Registered as the rocket connection's error sink.
  void onError(folly::exception_wrapper&& e) noexcept;

  // Disconnect edge from the rocket connection. The channel is always handed
  // an already-connected connection, so it does not observe connect — it only
  // needs to know when the connection goes down to stop accepting writes.
  void onDisconnect() noexcept;

  // Decode a single request-response and build the ClientReceiveState.
  void handleRequestResponse(
      apache::thrift::fast_thrift::frame::read::ParsedFrame&& frame,
      apache::thrift::RequestClientCallback::Ptr callback);

  // Error handling helpers (out of line to keep the hot path small).
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

  std::unique_ptr<rocket::client::RocketClientConnection> connection_;
  folly::EventBase* evb_;
  uint16_t protocolId_;
  State state_{State::Open};
  folly::exception_wrapper lastError_;
};

} // namespace apache::thrift::fast_thrift::thrift
