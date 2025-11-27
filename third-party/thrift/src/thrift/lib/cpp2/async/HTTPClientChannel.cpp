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

#include <thrift/lib/cpp2/async/HTTPClientChannel.h>

#include <utility>

#include <fmt/core.h>
#include <folly/base64.h>
#include <proxygen/lib/http/HTTPCommonHeaders.h>
#include <proxygen/lib/http/HTTPMethod.h>
#include <proxygen/lib/http/codec/HTTP1xCodec.h>
#include <proxygen/lib/http/codec/HTTP2Codec.h>
#include <proxygen/lib/http/session/HTTPTransaction.h>
#include <thrift/lib/cpp/protocol/TProtocolTypes.h>
#include <thrift/lib/cpp2/async/ResponseChannel.h>
#include <wangle/ssl/SSLContextConfig.h>

using apache::thrift::transport::THeader;
using apache::thrift::transport::TTransportException;
using folly::EventBase;
using proxygen::WheelTimerInstance;
using std::unique_ptr;

namespace apache::thrift {

namespace detail {
THRIFT_PLUGGABLE_FUNC_REGISTER(
    void, setHTTPFrameworkMetadata, THeader*, const RpcOptions&) {}
} // namespace detail

HTTPClientChannel::Ptr HTTPClientChannel::newHTTP2Channel(
    folly::AsyncTransport::UniquePtr transport,
    std::chrono::milliseconds sessionDefaultTimeout) {
  return HTTPClientChannel::Ptr(new HTTPClientChannel(
      std::move(transport),
      std::make_unique<proxygen::HTTP2Codec>(
          proxygen::TransportDirection::UPSTREAM),
      sessionDefaultTimeout));
}

HTTPClientChannel::HTTPClientChannel(
    folly::AsyncTransport::UniquePtr transport,
    std::unique_ptr<proxygen::HTTPCodec> codec,
    std::chrono::milliseconds sessionDefaultTimeout)
    : evb_(transport->getEventBase()) {
  auto localAddress = transport->getLocalAddress();
  auto peerAddress = transport->getPeerAddress();

  httpSession_ = new proxygen::HTTPUpstreamSession(
      WheelTimerInstance(sessionDefaultTimeout, evb_),
      std::move(transport),
      localAddress,
      peerAddress,
      std::move(codec),
      wangle::TransportInfo(),
      this);
}

HTTPClientChannel::~HTTPClientChannel() {
  closeNow();
}

void HTTPClientChannel::setMaxPendingRequests(uint32_t num) {
  if (httpSession_) {
    httpSession_->setMaxConcurrentOutgoingStreams(num);
  }
}

// apache::thrift::ClientChannel methods

bool HTTPClientChannel::good() {
  auto transport = httpSession_ ? httpSession_->getTransport() : nullptr;
  return transport && transport->good();
}

ClientChannel::SaturationStatus HTTPClientChannel::getSaturationStatus() {
  if (httpSession_) {
    return SaturationStatus(
        httpSession_->getNumOutgoingStreams(),
        httpSession_->getMaxConcurrentOutgoingStreams());
  } else {
    return SaturationStatus();
  }
}

void HTTPClientChannel::closeNow() {
  if (httpSession_) {
    httpSession_->dropConnection();
    httpSession_ = nullptr;
  }
}

void HTTPClientChannel::attachEventBase(EventBase* eventBase) {
  assert(eventBase->isInEventBaseThread());
  if (httpSession_) {
    httpSession_->attachThreadLocals(
        eventBase,
        nullptr,
        WheelTimerInstance(timeout_, eventBase),
        nullptr,
        [](proxygen::HTTPCodecFilter*) {},
        nullptr,
        nullptr);
  }
  evb_ = eventBase;
}

void HTTPClientChannel::detachEventBase() {
  assert(evb_->isInEventBaseThread());
  assert(isDetachable());
  if (httpSession_) {
    httpSession_->detachThreadLocals();
  }
  evb_ = nullptr;
}

bool HTTPClientChannel::isDetachable() {
  return !httpSession_ || httpSession_->isDetachable();
}

// end apache::thrift::ClientChannel methods

// folly::DelayedDestruction methods

void HTTPClientChannel::destroy() {
  closeNow();
  folly::DelayedDestruction::destroy();
}

// end folly::DelayedDestruction methods

// apache::thrift::RequestChannel methods

void HTTPClientChannel::sendRequestNoResponse(
    const RpcOptions& rpcOptions,
    MethodMetadata&& methodMetadata,
    SerializedRequest&& serializedRequest,
    std::shared_ptr<THeader> header,
    RequestClientCallback::Ptr cb,
    std::unique_ptr<folly::IOBuf> /*unused - framework metadata unsupported*/) {
  auto buf = LegacySerializedRequest(
                 header->getProtocolId(),
                 methodMetadata.name_view(),
                 std::move(serializedRequest))
                 .buffer;

  sendRequest_(
      rpcOptions, true, std::move(buf), std::move(header), std::move(cb));
}

void HTTPClientChannel::sendRequestResponse(
    const RpcOptions& rpcOptions,
    MethodMetadata&& methodMetadata,
    SerializedRequest&& serializedRequest,
    std::shared_ptr<THeader> header,
    RequestClientCallback::Ptr cb,
    std::unique_ptr<folly::IOBuf> /*unused - framework metadata unsupported*/) {
  auto buf = LegacySerializedRequest(
                 header->getProtocolId(),
                 methodMetadata.name_view(),
                 std::move(serializedRequest))
                 .buffer;

  sendRequest_(
      rpcOptions, false, std::move(buf), std::move(header), std::move(cb));
}

namespace {
class HTTPTransactionCallback
    : public MessageChannel::SendCallback,
      public proxygen::HTTPTransactionHandler,
      public proxygen::HTTPTransaction::TransportCallback {
 public:
  HTTPTransactionCallback(bool oneway, RequestClientCallback::Ptr cb)
      : oneway_(oneway), cb_(std::move(cb)), txn_(nullptr) {}

  ~HTTPTransactionCallback() override {
    if (txn_) {
      txn_->setHandler(nullptr);
      txn_->setTransportCallback(nullptr);
    }
  }

  // MessageChannel::SendCallback methods

  void sendQueued() override {}

  void messageSent() override {
    if (cb_ && oneway_) {
      cb_.release()->onResponse({});
    }
  }

  void messageSendError(folly::exception_wrapper&& ex) override {
    requestError(std::move(ex));
  }

  // end MessageChannel::SendCallback methods

  // proxygen::HTTPTransactionHandler methods

  void setTransaction(proxygen::HTTPTransaction* txn) noexcept override {
    txn_ = txn;
    // If Transaction is created through HTTPSession::newTransaction,
    // handler is already set, thus no need for txn_->setHandler(this);
    txn_->setTransportCallback(this);
  }

  void detachTransaction() noexcept override {
    VLOG(5) << "HTTPTransaction on memory " << this << " is detached.";
    delete this;
  }

  void onHeadersComplete(
      std::unique_ptr<proxygen::HTTPMessage> msg) noexcept override {
    msg_ = std::move(msg);
  }

  void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override {
    if (!body_) {
      body_ = std::make_unique<folly::IOBufQueue>();
    }
    body_->append(std::move(body));
  }

  void onChunkHeader(size_t /* length */) noexcept override {
    // HTTP/1.1 function, do not need attention here
  }

  void onChunkComplete() noexcept override {
    // HTTP/1.1 function, do not need attention here
  }

  void onTrailers(
      std::unique_ptr<proxygen::HTTPHeaders> trailers) noexcept override {
    trailers_ = std::move(trailers);
  }

  void onEOM() noexcept override {
    if (!oneway_ && cb_) {
      if (!body_) {
        requestError(
            folly::make_exception_wrapper<transport::TTransportException>(
                fmt::format(
                    "Empty HTTP response, {}",
                    (msg_ ? folly::to<std::string>(
                                msg_->getStatusCode(),
                                ", ",
                                msg_->getStatusMessage())
                          : "Empty Header"))));
        return;
      }

      auto header = std::make_unique<transport::THeader>();
      header->setClientType(THRIFT_HTTP_CLIENT_TYPE);
      apache::thrift::transport::THeader::StringToStringMap readHeaders;
      msg_->getHeaders().forEach(
          [&readHeaders](const std::string& key, const std::string& val) {
            readHeaders[key] = val;
          });
      header->setReadHeaders(std::move(readHeaders));
      auto body = body_->move();
      body_.reset();
      cb_.release()->onResponse(
          ClientReceiveState(-1, std::move(body), std::move(header), nullptr));
    }
  }

  void onUpgrade(proxygen::UpgradeProtocol /*protocol*/) noexcept override {
    // If code comes here, it is seriously wrong
    // TODO (geniusye) destroy the channel here
  }

  void onError(const proxygen::HTTPException& error) noexcept override {
    if (!oneway_) {
      if (error.getProxygenError() == proxygen::ProxygenError::kErrorTimeout) {
        TTransportException ex(TTransportException::TIMED_OUT, "Timed Out");
        ex.setOptions(TTransportException::CHANNEL_IS_VALID);
        requestError(
            folly::make_exception_wrapper<TTransportException>(std::move(ex)));
      } else {
        requestError(
            folly::make_exception_wrapper<transport::TTransportException>(
                error.what()));
      }
    }
  }

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

  void lastByteFlushed() noexcept override {
    if (cb_ && oneway_) {
      cb_.release()->onResponse({});
    }
  }

  void lastByteAcked(std::chrono::milliseconds /*latency*/) noexcept override {}
  void headerBytesGenerated(
      proxygen::HTTPHeaderSize& /*size*/) noexcept override {}
  void headerBytesReceived(
      const proxygen::HTTPHeaderSize& /*size*/) noexcept override {}
  void bodyBytesGenerated(size_t /*nbytes*/) noexcept override {}
  void bodyBytesReceived(size_t /*size*/) noexcept override {}

  // end proxygen::HTTPTransaction::TransportCallback methods

  void requestError(folly::exception_wrapper ex) {
    if (cb_) {
      cb_.release()->onResponseError(std::move(ex));
    }
  }

 private:
  bool oneway_;

  RequestClientCallback::Ptr cb_;

  proxygen::HTTPTransaction* txn_;
  std::unique_ptr<proxygen::HTTPMessage> msg_;
  std::unique_ptr<folly::IOBufQueue> body_;
  std::unique_ptr<proxygen::HTTPHeaders> trailers_;
};
} // namespace

void HTTPClientChannel::sendRequest_(
    const RpcOptions& rpcOptions,
    bool oneway,
    unique_ptr<IOBuf> buf,
    std::shared_ptr<THeader> header,
    RequestClientCallback::Ptr cb) {
  DestructorGuard dg(this);

  std::chrono::milliseconds timeout(timeout_);
  if (rpcOptions.getTimeout() > std::chrono::milliseconds(0)) {
    timeout = rpcOptions.getTimeout();
  }

  // Do not try to keep the raw pointer of this out of this function,
  // it is a self-destruct-object, it can dangle at some time later,
  // instead, only react to its callback
  auto httpCallback = new HTTPTransactionCallback(oneway, std::move(cb));

  if (!httpSession_) {
    TTransportException ex(
        TTransportException::NOT_OPEN, "HTTPSession is not open");
    httpCallback->messageSendError(
        folly::make_exception_wrapper<TTransportException>(std::move(ex)));
    delete httpCallback;
    return;
  }

  if (!httpSession_->supportsMoreTransactions()) {
    TTransportException ex(
        TTransportException::NOT_OPEN, "HTTPSession is saturated");

    // Might be able to create another transaction soon
    ex.setOptions(TTransportException::CHANNEL_IS_VALID);
    httpCallback->messageSendError(
        folly::make_exception_wrapper<TTransportException>(std::move(ex)));
    delete httpCallback;
    return;
  }

  auto res = httpSession_->newTransactionWithError(httpCallback);

  if (res.hasError()) {
    TTransportException ex(TTransportException::NOT_OPEN, res.error());
    // Might be able to create another transaction soon
    ex.setOptions(TTransportException::CHANNEL_IS_VALID);
    httpCallback->messageSendError(
        folly::make_exception_wrapper<TTransportException>(std::move(ex)));
    delete httpCallback;
    return;
  }
  auto& txn = res.value();

  if (timeout.count()) {
    txn->setIdleTimeout(timeout);
  }
  if (transactionObserverCreator_) {
    if (auto observer = transactionObserverCreator_->create()) {
      txn->addObserver(std::move(observer));
    }
  }

  setRequestHeaderOptions(header.get());
  addRpcOptionHeaders(header.get(), rpcOptions);
  detail::setHTTPFrameworkMetadata(header.get(), rpcOptions);

  auto msg = buildHTTPMessage(header.get());

  txn->sendHeaders(msg);
  txn->sendBody(std::move(buf));
  txn->sendEOM();
}

// end apache::thrift::RequestChannel methods

// HTTPSession::InfoCallback methods

void HTTPClientChannel::onDestroy(const proxygen::HTTPSessionBase&) {
  if (closeCallback_) {
    closeCallback_->channelClosed();
  }
  httpSession_ = nullptr;
  closeCallback_ = nullptr;
}

// end HTTPSession::InfoCallback methods

void HTTPClientChannel::setRequestHeaderOptions(THeader* header) {
  header->setClientType(THRIFT_HTTP_CLIENT_TYPE);
  header->forceClientType(true);
}

void HTTPClientChannel::setHeaders(
    proxygen::HTTPHeaders& dstHeaders,
    const transport::THeader::StringToStringMap& srcHeaders) {
  for (const auto& header : srcHeaders) {
    if (header.first.find(":") != std::string::npos) {
      auto name = folly::base64URLEncode(header.first);
      auto value = folly::base64URLEncode(header.second);
      dstHeaders.rawSet(
          folly::to<std::string>("encode_", name),
          folly::to<std::string>(name, "_", value));
    } else {
      dstHeaders.rawSet(header.first, header.second);
    }
  }
}

proxygen::HTTPMessage HTTPClientChannel::buildHTTPMessage(THeader* header) {
  preprocessHeader(header);

  proxygen::HTTPMessage msg;
  msg.setMethod(proxygen::HTTPMethod::POST);
  msg.setURL(httpUrl_);
  msg.setHTTPVersion(1, 1);
  msg.setIsChunked(false);
  auto& headers = msg.getHeaders();

  {
    auto wh = header->releaseWriteHeaders();
    setHeaders(headers, wh);
  }

  {
    auto eh = header->getExtraWriteHeaders();
    if (eh) {
      setHeaders(headers, *eh);
    }
  }

  headers.set(proxygen::HTTPHeaderCode::HTTP_HEADER_HOST, httpHost_);
  headers.set(
      proxygen::HTTPHeaderCode::HTTP_HEADER_USER_AGENT, "C++/THttpClient");
  headers.set(
      proxygen::HTTPHeaderCode::HTTP_HEADER_CONTENT_TYPE,
      "application/x-thrift");

  switch (protocolId_) {
    case protocol::T_BINARY_PROTOCOL:
      headers.set(
          proxygen::HTTPHeaderCode::HTTP_HEADER_X_THRIFT_PROTOCOL, "binary");
      break;
    case protocol::T_COMPACT_PROTOCOL:
      headers.set(
          proxygen::HTTPHeaderCode::HTTP_HEADER_X_THRIFT_PROTOCOL, "compact");
      break;
    case protocol::T_JSON_PROTOCOL:
      headers.set(
          proxygen::HTTPHeaderCode::HTTP_HEADER_X_THRIFT_PROTOCOL, "json");
      break;
    case protocol::T_SIMPLE_JSON_PROTOCOL:
      headers.set(
          proxygen::HTTPHeaderCode::HTTP_HEADER_X_THRIFT_PROTOCOL,
          "simplejson");
      break;
    default:
      // Do nothing
      break;
  }

  return msg;
}

void HTTPClientChannel::setFlowControl(
    size_t initialReceiveWindow,
    size_t receiveStreamWindowSize,
    size_t receiveSessionWindowSize) {
  if (httpSession_) {
    httpSession_->setFlowControl(
        initialReceiveWindow,
        receiveStreamWindowSize,
        receiveSessionWindowSize);
  }
}

void HTTPClientChannel::setSessionWriteBufferLimit(uint32_t limit) {
  if (httpSession_) {
    httpSession_->setWriteBufferLimit(limit);
  }
}

// HTTPTransactionCallback methods

} // namespace apache::thrift
