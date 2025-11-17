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

#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/Request.h>
#include <proxygen/lib/http/session/HTTPTransaction.h>
#include <proxygen/lib/http/session/HTTPUpstreamSession.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/PluggableFunction.h>
#include <thrift/lib/cpp2/async/ClientChannel.h>
#include <thrift/lib/cpp2/async/RpcOptions.h>

namespace apache::thrift {

/**
 * HTTPClientChannel
 *
 * This is a channel implementation that reads and writes
 * messages encoded using THttpProtocol.
 */
class HTTPClientChannel : public ClientChannel,
                          private proxygen::HTTPSessionBase::InfoCallback,
                          virtual public folly::DelayedDestruction {
 private:
  static inline constexpr std::chrono::milliseconds kDefaultTransactionTimeout =
      std::chrono::milliseconds(500);

 public:
  using Ptr =
      std::unique_ptr<HTTPClientChannel, folly::DelayedDestruction::Destructor>;

  static HTTPClientChannel::Ptr newHTTP2Channel(
      folly::AsyncTransport::UniquePtr transport,
      std::chrono::milliseconds sessionDefaultTimeout =
          kDefaultTransactionTimeout);

  void setHTTPHost(const std::string& host) { httpHost_ = host; }
  void setHTTPUrl(const std::string& url) { httpUrl_ = url; }

  // Sets the maximum pending outgoing requests allowed on this channel.
  // Subject to negotiation with the server, which may dictate a smaller
  // maximum.
  void setMaxPendingRequests(uint32_t num);

  void setProtocolId(uint16_t protocolId) { protocolId_ = protocolId; }

  // apache::thrift::ClientChannel methods

  void closeNow() override;

  bool good() override;

  SaturationStatus getSaturationStatus() override;

  void attachEventBase(folly::EventBase*) override;
  void detachEventBase() override;
  bool isDetachable() override;

  // Client timeouts for read, write.
  // Servers should use timeout methods on underlying transport.
  void setTimeout(uint32_t ms) override {
    timeout_ = std::chrono::milliseconds(ms);
  }
  uint32_t getTimeout() override { return timeout_.count(); }

  CLIENT_TYPE getClientType() override { return THRIFT_HTTP_CLIENT_TYPE; }

  // end apache::thrift::ClientChannel methods

  // folly::DelayedDestruction methods

  void destroy() override;

  // end folly::DelayedDestruction methods

  // apache::thrift::RequestChannel methods

  folly::EventBase* getEventBase() const override { return evb_; }

  using RequestChannel::sendRequestNoResponse;
  using RequestChannel::sendRequestResponse;
  using RequestChannel::sendRequestSink;
  using RequestChannel::sendRequestStream;

  void sendRequestResponse(
      const RpcOptions&,
      MethodMetadata&&,
      SerializedRequest&&,
      std::shared_ptr<apache::thrift::transport::THeader>,
      RequestClientCallback::Ptr,
      std::unique_ptr<folly::IOBuf>) override;

  void sendRequestNoResponse(
      const RpcOptions&,
      MethodMetadata&&,
      SerializedRequest&&,
      std::shared_ptr<apache::thrift::transport::THeader>,
      RequestClientCallback::Ptr,
      std::unique_ptr<folly::IOBuf>) override;

  void sendRequestStream(
      const RpcOptions&,
      MethodMetadata&&,
      SerializedRequest&&,
      std::shared_ptr<transport::THeader>,
      StreamClientCallback* clientCallback,
      std::unique_ptr<folly::IOBuf>) override {
    clientCallback->onFirstResponseError(
        folly::make_exception_wrapper<transport::TTransportException>(
            "This channel doesn't support stream RPC"));
  }

  void sendRequestSink(
      const RpcOptions&,
      MethodMetadata&&,
      SerializedRequest&&,
      std::shared_ptr<transport::THeader>,
      SinkClientCallback* clientCallback,
      std::unique_ptr<folly::IOBuf>) override {
    clientCallback->onFirstResponseError(
        folly::make_exception_wrapper<transport::TTransportException>(
            "This channel doesn't support sink RPC"));
  }

  void setCloseCallback(CloseCallback* cb) override { closeCallback_ = cb; }

  uint16_t getProtocolId() override { return protocolId_; }

  folly::AsyncTransport* getTransport() override {
    if (httpSession_) {
      return dynamic_cast<folly::AsyncTransport*>(httpSession_->getTransport());
    } else {
      return nullptr;
    }
  }

  // end apache::thrift::RequestChannel methods

  void setFlowControl(
      size_t initialReceiveWindow,
      size_t receiveStreamWindowSize,
      size_t receiveSessionWindowSize);

 protected:
  void sendRequest_(
      const RpcOptions&,
      bool oneway,
      std::unique_ptr<folly::IOBuf>,
      std::shared_ptr<apache::thrift::transport::THeader>,
      RequestClientCallback::Ptr);

  HTTPClientChannel(
      folly::AsyncTransport::UniquePtr transport,
      std::unique_ptr<proxygen::HTTPCodec> codec,
      std::chrono::milliseconds sessionDefaultTimeout);

  ~HTTPClientChannel() override;

 private:
  class HTTPTransactionCallback
      : public MessageChannel::SendCallback,
        public proxygen::HTTPTransactionHandler,
        public proxygen::HTTPTransaction::TransportCallback {
   public:
    HTTPTransactionCallback(bool oneway, RequestClientCallback::Ptr cb);

    ~HTTPTransactionCallback() override;

    // MessageChannel::SendCallback methods

    void sendQueued() override {}

    void messageSent() override;

    void messageSendError(folly::exception_wrapper&& ex) override;

    // end MessageChannel::SendCallback methods

    // proxygen::HTTPTransactionHandler methods

    void setTransaction(proxygen::HTTPTransaction* txn) noexcept override;

    void detachTransaction() noexcept override;

    void onHeadersComplete(
        std::unique_ptr<proxygen::HTTPMessage> msg) noexcept override;

    void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;

    void onChunkHeader(size_t /* length */) noexcept override {
      // HTTP/1.1 function, do not need attention here
    }

    void onChunkComplete() noexcept override {
      // HTTP/1.1 function, do not need attention here
    }

    void onTrailers(
        std::unique_ptr<proxygen::HTTPHeaders> trailers) noexcept override;

    void onEOM() noexcept override;

    void onUpgrade(proxygen::UpgradeProtocol /*protocol*/) noexcept override {
      // If code comes here, it is seriously wrong
      // TODO (geniusye) destroy the channel here
    }

    void onError(const proxygen::HTTPException& error) noexcept override;

    void onEgressPaused() noexcept override {
      // we could notify servicerouter to throttle on this channel
      // it is okay not to throttle too,
      // it won't immediately causing any problem
    }

    void onEgressResumed() noexcept override {
      // we could notify servicerouter to stop throttle on this channel
      // it is okay not to throttle too,
      // it won't immediately causing any problem
    }

    void onPushedTransaction(
        proxygen::HTTPTransaction* /*txn*/) noexcept override {}

    // end proxygen::HTTPTransactionHandler methods

    // proxygen::HTTPTransaction::TransportCallback methods

    // most of the methods in TransportCallback is not interesting to us,
    // thus, we don't have to handle them, except the one that notifies the
    // fact the request is sent.

    void firstHeaderByteFlushed() noexcept override {}
    void firstByteFlushed() noexcept override {}

    void lastByteFlushed() noexcept override;

    void lastByteAcked(
        std::chrono::milliseconds /*latency*/) noexcept override {}
    void headerBytesGenerated(
        proxygen::HTTPHeaderSize& /*size*/) noexcept override {}
    void headerBytesReceived(
        const proxygen::HTTPHeaderSize& /*size*/) noexcept override {}
    void bodyBytesGenerated(size_t /*nbytes*/) noexcept override {}
    void bodyBytesReceived(size_t /*size*/) noexcept override {}

    // end proxygen::HTTPTransaction::TransportCallback methods

    void requestError(folly::exception_wrapper ex);

    proxygen::HTTPTransaction* getTransaction() noexcept { return txn_; }

   private:
    bool oneway_;

    RequestClientCallback::Ptr cb_;

    proxygen::HTTPTransaction* txn_;
    std::unique_ptr<proxygen::HTTPMessage> msg_;
    std::unique_ptr<folly::IOBufQueue> body_;
    std::unique_ptr<proxygen::HTTPHeaders> trailers_;
  };

  void setHeaders(
      proxygen::HTTPHeaders& dstHeaders,
      const transport::THeader::StringToStringMap& srcHeaders);

  proxygen::HTTPMessage buildHTTPMessage(transport::THeader* header);

  // HTTPSession::InfoCallback methods

  void onDestroy(const proxygen::HTTPSessionBase&) override;

  // end HTTPSession::InfoCallback methods

  void setRequestHeaderOptions(apache::thrift::transport::THeader* header);

  proxygen::HTTPUpstreamSession* httpSession_{nullptr};
  folly::EventBase* evb_{nullptr};
  std::string httpHost_;
  std::string httpUrl_;
  std::chrono::milliseconds timeout_{kDefaultTransactionTimeout};
  uint16_t protocolId_{apache::thrift::protocol::T_BINARY_PROTOCOL};
  CloseCallback* closeCallback_{nullptr};
};

namespace detail {
THRIFT_PLUGGABLE_FUNC_DECLARE(
    void, setHTTPFrameworkMetadata, transport::THeader*, const RpcOptions&);
} // namespace detail

} // namespace apache::thrift
