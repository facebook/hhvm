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

#include <chrono>
#include <string>
#include <unordered_map>

#include <folly/io/async/AsyncTransport.h>
#include <proxygen/lib/http/codec/HTTPCodec.h>
#include <proxygen/lib/http/codec/HTTPSettings.h>
#include <proxygen/lib/http/session/HTTPUpstreamSession.h>
#include <thrift/lib/cpp2/transport/core/ClientConnectionIf.h>
#include <thrift/lib/cpp2/transport/core/ThriftClientCallback.h>
#include <thrift/lib/cpp2/transport/http2/common/H2Channel.h>

namespace apache::thrift {

/**
 * HTTP/2 implementation of ClientConnectionIf.
 *
 * Static methods are provided to create HTTP1 or HTTP2 connections.
 * These methods optionally take a host and url parameter.  Some
 * servers will only work with specific values for these.  If these
 * parameters are not set, the implementation will use the most
 * efficient possible setting for these values.
 *
 * Host and url values can only be specified at connection creation
 * time - i.e., you cannot use different values (for url) for
 * different RPCs.
 *
 * This class maintains a nested Proxygen connection
 * (HTTPUpstreamSession).  If the Proxygen connection dies, we do not
 * attempt to recreate it, instead we pass this error to the callers.
 * In the future, we may change this (for now callers have to create a
 * new H2ClientConnection object and discard the old one).
 */
class H2ClientConnection : public ClientConnectionIf,
                           public proxygen::HTTPSession::InfoCallback {
 public:
  struct FlowControlSettings {
   private:
    // Stream and initial receive control window are 10MB
    static constexpr size_t kStreamWindow = 10U * (1U << 20);
    // Session (i.e connection) window is larger at 15MB
    static constexpr size_t kSessionWindow = 15U * (1U << 20);

   public:
    FlowControlSettings()
        : initialReceiveWindow(kStreamWindow),
          receiveStreamWindowSize(kStreamWindow),
          receiveSessionWindowSize(kSessionWindow) {}

    size_t initialReceiveWindow;
    size_t receiveStreamWindowSize;
    size_t receiveSessionWindowSize;
  };

  static std::unique_ptr<ClientConnectionIf> newHTTP2Connection(
      folly::AsyncTransport::UniquePtr transport,
      FlowControlSettings flowControlSettings = FlowControlSettings());

  ~H2ClientConnection() override;

  H2ClientConnection(const H2ClientConnection&) = delete;
  H2ClientConnection& operator=(const H2ClientConnection&) = delete;
  H2ClientConnection(H2ClientConnection&&) = delete;
  H2ClientConnection& operator=(H2ClientConnection&&) = delete;

  std::shared_ptr<ThriftChannelIf> getChannel() override;
  void setMaxPendingRequests(uint32_t num) override;
  void setCloseCallback(ThriftClient* client, CloseCallback* cb) override;
  folly::EventBase* getEventBase() const override;

  // Returns a new transaction that is bound to the channel parameter.
  // Throws TTransportException if unable to create a new transaction.
  proxygen::HTTPTransaction* newTransaction(H2Channel* channel);

  bool isStable();
  void setIsStable();

  folly::AsyncTransport* getTransport() override;
  bool good() override;
  ClientChannel::SaturationStatus getSaturationStatus() override;
  void attachEventBase(folly::EventBase* evb) override;
  void detachEventBase() override;
  bool isDetachable() override;
  uint32_t getTimeout() override;
  void setTimeout(uint32_t ms) override;
  void closeNow() override;
  CLIENT_TYPE getClientType() override;

  // HTTPSession::InfoCallback method
  void onDestroy(const proxygen::HTTPSessionBase&) override;

 private:
  H2ClientConnection(
      folly::AsyncTransport::UniquePtr transport,
      std::unique_ptr<proxygen::HTTPCodec> codec,
      FlowControlSettings flowControlSettings);

  proxygen::HTTPUpstreamSession* httpSession_;
  folly::EventBase* evb_{nullptr};
  std::chrono::milliseconds timeout_{
      apache::thrift::ThriftClientCallback::kDefaultTimeout};

  // A map of all registered CloseCallback objects keyed by the
  // ThriftClient objects that registered the callback.
  std::unordered_map<ThriftClient*, CloseCallback*> closeCallbacks_;
};

} // namespace apache::thrift
