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

#include <thrift/lib/cpp2/transport/http2/client/H2ClientConnection.h>

#include <glog/logging.h>
#include <folly/portability/GFlags.h>

#include <proxygen/lib/http/codec/HTTP1xCodec.h>
#include <proxygen/lib/http/codec/HTTP2Codec.h>
#include <proxygen/lib/http/codec/TransportDirection.h>
#include <proxygen/lib/http/session/HTTPTransaction.h>
#include <proxygen/lib/utils/WheelTimerInstance.h>
#include <thrift/lib/cpp/transport/TTransportException.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/transport/core/ThriftClient.h>
#include <thrift/lib/cpp2/transport/http2/client/ThriftTransactionHandler.h>
#include <thrift/lib/cpp2/transport/http2/common/SingleRpcChannel.h>
#include <wangle/acceptor/TransportInfo.h>

#include <algorithm>

namespace apache {
namespace thrift {

using apache::thrift::transport::TTransportException;
using folly::EventBase;
using proxygen::HTTPSessionBase;
using proxygen::HTTPTransaction;
using proxygen::HTTPUpstreamSession;
using proxygen::WheelTimerInstance;

std::unique_ptr<ClientConnectionIf> H2ClientConnection::newHTTP2Connection(
    folly::AsyncTransport::UniquePtr transport,
    FlowControlSettings flowControlSettings) {
  std::unique_ptr<H2ClientConnection> connection(new H2ClientConnection(
      std::move(transport),
      std::make_unique<proxygen::HTTP2Codec>(
          proxygen::TransportDirection::UPSTREAM),
      flowControlSettings));
  return std::move(connection);
}

H2ClientConnection::H2ClientConnection(
    folly::AsyncTransport::UniquePtr transport,
    std::unique_ptr<proxygen::HTTPCodec> codec,
    FlowControlSettings flowControlSettings)
    : evb_(transport->getEventBase()) {
  DCHECK(evb_ && evb_->isInEventBaseThread());
  auto localAddress = transport->getLocalAddress();
  auto peerAddress = transport->getPeerAddress();
  httpSession_ = new HTTPUpstreamSession(
      WheelTimerInstance(timeout_, evb_),
      std::move(transport),
      localAddress,
      peerAddress,
      std::move(codec),
      wangle::TransportInfo(),
      this);
  httpSession_->setFlowControl(
      flowControlSettings.initialReceiveWindow,
      flowControlSettings.receiveStreamWindowSize,
      flowControlSettings.receiveSessionWindowSize);
  // TODO: Improve the way max outging streams is set
  setMaxPendingRequests(100000);
  httpSession_->startNow();
}

H2ClientConnection::~H2ClientConnection() {
  closeNow();
}

std::shared_ptr<ThriftChannelIf> H2ClientConnection::getChannel() {
  DCHECK(evb_ && evb_->isInEventBaseThread());
  return std::make_shared<SingleRpcChannel>(
      *evb_, [this](auto* self) { return this->newTransaction(self); });
}

void H2ClientConnection::setMaxPendingRequests(uint32_t num) {
  DCHECK(evb_ && evb_->isInEventBaseThread());
  if (httpSession_) {
    httpSession_->setMaxConcurrentOutgoingStreams(num);
  }
}

void H2ClientConnection::setCloseCallback(
    ThriftClient* client, CloseCallback* cb) {
  if (cb == nullptr) {
    closeCallbacks_.erase(client);
  } else {
    closeCallbacks_[client] = cb;
  }
}

EventBase* H2ClientConnection::getEventBase() const {
  return evb_;
}

HTTPTransaction* H2ClientConnection::newTransaction(H2Channel* channel) {
  DCHECK(evb_ && evb_->isInEventBaseThread());
  if (!httpSession_) {
    throw TTransportException(
        TTransportException::NOT_OPEN, "HTTPSession is not open");
  }

  if (!httpSession_->supportsMoreTransactions()) {
    TTransportException ex(
        TTransportException::NOT_OPEN, "HTTPSession is saturated");

    // Might be able to create another transaction soon
    ex.setOptions(TTransportException::CHANNEL_IS_VALID);
    throw ex;
  }

  // These objects destroy themselves when done.
  auto handler = new ThriftTransactionHandler();
  auto txn = httpSession_->newTransactionWithError(handler);
  if (txn.hasError()) {
    delete handler;
    TTransportException ex(TTransportException::NETWORK_ERROR, txn.error());
    // Might be able to create another transaction soon
    ex.setOptions(TTransportException::CHANNEL_IS_VALID);
    throw ex;
  }
  handler->setChannel(
      std::dynamic_pointer_cast<H2Channel>(channel->shared_from_this()));
  return txn.value();
}

folly::AsyncTransport* H2ClientConnection::getTransport() {
  DCHECK(!evb_ || evb_->isInEventBaseThread());
  if (httpSession_) {
    return httpSession_->getTransport();
  } else {
    return nullptr;
  }
}

bool H2ClientConnection::good() {
  DCHECK(evb_ && evb_->isInEventBaseThread());
  auto transport = httpSession_ ? httpSession_->getTransport() : nullptr;
  // Check that the transport is good and that the connection is not draining.
  return transport && transport->good() && !httpSession_->isDraining();
}

ClientChannel::SaturationStatus H2ClientConnection::getSaturationStatus() {
  DCHECK(evb_ && evb_->isInEventBaseThread());
  if (httpSession_) {
    return ClientChannel::SaturationStatus(
        httpSession_->getNumOutgoingStreams(),
        httpSession_->getMaxConcurrentOutgoingStreams());
  } else {
    return ClientChannel::SaturationStatus();
  }
}

void H2ClientConnection::attachEventBase(EventBase* evb) {
  DCHECK(evb && evb->isInEventBaseThread());
  if (httpSession_) {
    httpSession_->attachThreadLocals(
        evb,
        nullptr,
        WheelTimerInstance(timeout_, evb),
        nullptr,
        [](proxygen::HTTPCodecFilter*) {},
        nullptr,
        nullptr);
  }
  evb_ = evb;
}

void H2ClientConnection::detachEventBase() {
  DCHECK(evb_->isInEventBaseThread());
  DCHECK(isDetachable());
  if (httpSession_) {
    httpSession_->detachThreadLocals();
  }
  evb_ = nullptr;
}

bool H2ClientConnection::isDetachable() {
  return !httpSession_ || httpSession_->isDetachable(true);
}

uint32_t H2ClientConnection::getTimeout() {
  return timeout_.count();
}

void H2ClientConnection::setTimeout(uint32_t ms) {
  timeout_ = std::chrono::milliseconds(ms);
  // TODO: need to change timeout in httpSession_.  This functionality
  // is also missing in JiaJie's HTTPClientChannel.
}

void H2ClientConnection::closeNow() {
  DCHECK(!evb_ || evb_->isInEventBaseThread());
  if (httpSession_) {
    if (!evb_) {
      attachEventBase(folly::EventBaseManager::get()->getEventBase());
    }

    // Fire close callbacks preemptively and reset info callback since it's
    // rarely possible that HTTPSession destruction is delayed and the callback
    // fires on destroyed H2ClientConnection.
    for (auto& cb : closeCallbacks_) {
      cb.second->channelClosed();
    }
    closeCallbacks_.clear();
    httpSession_->setInfoCallback(nullptr);
    httpSession_->dropConnection();
    httpSession_ = nullptr;
  }
}

CLIENT_TYPE H2ClientConnection::getClientType() {
  return THRIFT_HTTP2_CLIENT_TYPE;
}

void H2ClientConnection::onDestroy(const HTTPSessionBase&) {
  DCHECK(evb_ && evb_->isInEventBaseThread());
  for (auto& cb : closeCallbacks_) {
    cb.second->channelClosed();
  }
  closeCallbacks_.clear();
  httpSession_ = nullptr;
}

} // namespace thrift
} // namespace apache
