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

#include <deque>
#include <memory>
#include <utility>

#include <glog/logging.h>

#include <folly/Function.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/DelayedDestruction.h>

#include <thrift/lib/cpp2/async/ClientChannel.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>

namespace folly {
class EventBase;
class IOBuf;
} // namespace folly

namespace apache::thrift {

namespace transport {
class THeader;
} // namespace transport

// Simple RequestChannel wrapper that automatically re-creates underlying
// RequestChannel in case request is about to be sent over a bad channel.
class ReconnectingRequestChannel : public RequestChannel,
                                   public folly::AsyncSocket::ConnectCallback {
 public:
  using Impl = ClientChannel;
  using ImplPtr = std::shared_ptr<Impl>;
  using ImplCreator = folly::Function<ImplPtr(folly::EventBase&)>;
  using ImplCreatorWithCallback = folly::Function<ImplPtr(
      folly::EventBase&, folly::AsyncSocket::ConnectCallback&)>;

  /**
   * DEPRECATED: Please use the newChannel() variant with a callback, which
   * supports queuing of requests while a channel is being reestablished
   * following a disconnect.
   */
  [[deprecated(
      "Use newChannel(folly::EventBase&, ImplCreatorWithCallback)")]] static std::
      unique_ptr<
          ReconnectingRequestChannel,
          folly::DelayedDestruction::Destructor>
      newChannel(folly::EventBase& evb, ImplCreator implCreator) {
    return {new ReconnectingRequestChannel(evb, std::move(implCreator)), {}};
  }

  static std::unique_ptr<
      ReconnectingRequestChannel,
      folly::DelayedDestruction::Destructor>
  newChannel(
      folly::EventBase& evb, ImplCreatorWithCallback implCreatorWithCallback) {
    return {
        new ReconnectingRequestChannel(evb, std::move(implCreatorWithCallback)),
        {}};
  }

  using RequestChannel::sendRequestNoResponse;
  using RequestChannel::sendRequestResponse;
  using RequestChannel::sendRequestSink;
  using RequestChannel::sendRequestStream;

  void sendRequestResponse(
      const RpcOptions& options,
      MethodMetadata&& methodMetadata,
      SerializedRequest&& request,
      std::shared_ptr<transport::THeader> header,
      RequestClientCallback::Ptr cob,
      std::unique_ptr<folly::IOBuf> frameworkMetadata) override;

  void sendRequestNoResponse(
      const RpcOptions&,
      MethodMetadata&& methodMetadata,
      SerializedRequest&& request,
      std::shared_ptr<transport::THeader>,
      RequestClientCallback::Ptr,
      std::unique_ptr<folly::IOBuf> frameworkMetadata) override;

  void sendRequestStream(
      const RpcOptions&,
      MethodMetadata&& methodMetadata,
      SerializedRequest&& request,
      std::shared_ptr<transport::THeader>,
      StreamClientCallback*,
      std::unique_ptr<folly::IOBuf> frameworkMetadata) override;

  void sendRequestSink(
      const apache::thrift::RpcOptions& rpcOptions,
      MethodMetadata&& methodMetadata,
      SerializedRequest&& request,
      std::shared_ptr<apache::thrift::transport::THeader> header,
      apache::thrift::SinkClientCallback* cb,
      std::unique_ptr<folly::IOBuf> frameworkMetadata) override;

  void setCloseCallback(CloseCallback*) override {
    LOG(FATAL) << "Not supported";
  }

  folly::EventBase* getEventBase() const override { return &evb_; }

  uint16_t getProtocolId() override {
    if (!isChannelGood()) {
      if (useRequestQueue_) {
        reconnectRequestChannelWithCallback();
      } else {
        reconnectRequestChannel();
      }
    }
    return impl_->getProtocolId();
  }

  void terminateInteraction(InteractionId id) override;

  InteractionId createInteraction(ManagedStringView&& name) override;

  InteractionId registerInteraction(
      ManagedStringView&& name, int64_t id) override;

  // Avoid using this method, unless really necessary and you understand how in
  // your system the request and socket->connect are working.
  void setRequestQueueSize(size_t s) { requestQueueLimit = s; }

  void connectSuccess() noexcept override;
  void connectErr(const folly::AsyncSocketException& ex) noexcept override;

 protected:
  ~ReconnectingRequestChannel() override {
    while (!requestQueue_.empty()) {
      requestQueue_.front()(/*failRequest=*/true);
      requestQueue_.pop_front();
    }
  }

 private:
  ReconnectingRequestChannel(folly::EventBase& evb, ImplCreator implCreator)
      : implCreator_(std::move(implCreator)),
        evb_(evb),
        useRequestQueue_(false),
        isReconnecting_(false),
        isCreatingChannel_(false) {}
  // With this contructor, each time when Channel Reconnecting happens, requests
  // will be first stored in a queue, and then let the socket->connect 's
  // callback to send them.
  ReconnectingRequestChannel(
      folly::EventBase& evb, ImplCreatorWithCallback implCreatorWithCallback)
      : implCreatorWithCallback_(std::move(implCreatorWithCallback)),
        evb_(evb),
        useRequestQueue_(true),
        isReconnecting_(false),
        isCreatingChannel_(false) {}

  bool isChannelGood();
  void reconnectRequestChannel();
  void reconnectRequestChannelWithCallback();
  void sendQueuedRequests();

  // An arbitrary number that ensure memory consuption will be limited to
  // request queue. If the queue reaches the limit, we will reject new request
  // immediatly. This is a safe guard that we expect to be rarely hit.
  size_t requestQueueLimit = 1024;

  ImplPtr impl_;
  ImplCreator implCreator_;
  ImplCreatorWithCallback implCreatorWithCallback_;
  folly::EventBase& evb_;
  std::deque<folly::Function<void(/*failRequest=*/bool)>> requestQueue_;
  const bool useRequestQueue_;
  bool isReconnecting_;
  bool isCreatingChannel_;
};

} // namespace apache::thrift
