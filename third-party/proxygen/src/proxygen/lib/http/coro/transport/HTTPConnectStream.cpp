/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/transport/HTTPConnectStream.h"
#include <folly/logging/xlog.h>

namespace proxygen::coro {

HTTPConnectStream::HTTPConnectStream(Ownership ownership,
                                     HTTPCoroSession* session,
                                     RequestHeaderMap connectHeaders,
                                     size_t egressBufferSize)
    : session_(ownership == Ownership::Unique ? session : nullptr),
      eventBase_(session->getEventBase()),
      egressBufferSize_(egressBufferSize),
      egressSource_(new HTTPStreamSource(
          session->getEventBase(), folly::none, this, egressBufferSize)),
      localAddr_(session->getLocalAddress()),
      peerAddr_(session->getPeerAddress()),
      connectHeaders_(std::move(connectHeaders)) {
  egressSource_->setHeapAllocated();
  if (session_) {
    session_->addLifecycleObserver(this);
  }
}

HTTPConnectStream::~HTTPConnectStream() {
  close();
  if (session_) {
    session_->removeLifecycleObserver(this);
    session_->initiateDrain();
  }
  if (egressSource_) {
    egressSource_->setCallback(nullptr);
  }
}

folly::coro::Task<std::unique_ptr<HTTPConnectStream>>
HTTPConnectStream::connect(HTTPCoroSession* session,
                           HTTPCoroSession::RequestReservation reservation,
                           std::string authority,
                           std::chrono::milliseconds timeout,
                           RequestHeaderMap connectHeaders,
                           size_t egressBufferSize) {
  std::unique_ptr<HTTPConnectStream> transport{new HTTPConnectStream(
      Ownership::Shared, session, std::move(connectHeaders), egressBufferSize)};
  co_await transport->connectImpl(
      session, std::move(reservation), std::move(authority), timeout);
  co_return transport;
}

folly::coro::Task<std::unique_ptr<HTTPConnectStream>>
HTTPConnectStream::connectUnique(
    HTTPCoroSession* session,
    HTTPCoroSession::RequestReservation reservation,
    std::string authority,
    std::chrono::milliseconds timeout,
    RequestHeaderMap connectHeaders,
    size_t egressBufferSize) {
  std::unique_ptr<HTTPConnectStream> transport{new HTTPConnectStream(
      Ownership::Unique, session, std::move(connectHeaders), egressBufferSize)};
  co_await transport->connectImpl(
      session, std::move(reservation), std::move(authority), timeout);
  co_return transport;
}

folly::coro::Task<void> HTTPConnectStream::connectImpl(
    HTTPCoroSession* session,
    HTTPCoroSession::RequestReservation reservation,
    std::string authority,
    std::chrono::milliseconds timeout) {
  auto connectRequest = std::make_unique<HTTPMessage>();
  connectRequest->setMethod(HTTPMethod::CONNECT);
  connectRequest->setHTTPVersion(1, 1);
  uint16_t upstreamPort = 0;
  auto connectURL = connectRequest->setURL(authority);
  if (connectURL.valid()) {
    upstreamPort = connectURL.port();
  }
  connectRequest->getHeaders().set(HTTP_HEADER_HOST, authority);
  for (auto& header : connectHeaders_) {
    connectRequest->getHeaders().add(header.first, std::move(header.second));
  }
  connectHeaders_.clear();
  egressSource_->headers(std::move(connectRequest), false);
  ingressSource_ = std::make_unique<HTTPSourceHolder>(
      co_await session->sendRequest(egressSource_, std::move(reservation)));
  ingressSource_->setReadTimeout(timeout);
  while (true) {
    auto headerEvent = co_await ingressSource_->readHeaderEvent();
    if (!headerEvent.isFinal()) {
      continue;
    }
    auto status = headerEvent.headers->getStatusCode();
    if (status / 100 != 2) {
      egressSource_->abort(HTTPErrorCode::CONNECT_ERROR);
      co_yield folly::coro::co_error(std::runtime_error(folly::to<std::string>(
          "CONNECT to ", authority, " failed status=", status)));
    }
    auto upstreamAddress =
        headerEvent.headers->getHeaders().getSingleOrEmpty("X-Connected-To");
    if (!upstreamAddress.empty()) {
      try {
        folly::SocketAddress peerAddr;
        peerAddr.setFromIpPort(upstreamAddress, upstreamPort);
        peerAddr_ = std::move(peerAddr);
      } catch (const std::exception& ex) {
        XLOG(ERR) << "Upstream returned invalid X-Connected-To: "
                  << upstreamAddress << " err=" << ex.what();
      }
    }
    break; // meh
  }
  // Successfully connected!
  egressSource_->setStreamID(*ingressSource_->getStreamID());
  co_return;
}

void HTTPConnectStream::bytesProcessed(HTTPCodec::StreamID id,
                                       size_t amount,
                                       size_t toAck) {
  if (callback_) {
    callback_->bytesProcessed(id, amount, toAck);
  }
}

void HTTPConnectStream::windowOpen(HTTPCodec::StreamID id) {
  if (callback_) {
    callback_->windowOpen(id);
  }
}

void HTTPConnectStream::sourceComplete(HTTPCodec::StreamID id,
                                       folly::Optional<HTTPError> error) {
  egressSource_ = nullptr;
  egressError_ = std::move(error);
  if (callback_) {
    callback_->sourceComplete(id, egressError_);
  }
}

void HTTPConnectStream::shutdownRead() {
  if (canRead()) {
    ingressSource_->stopReading();
  }
}

void HTTPConnectStream::shutdownWrite() {
  if (canWrite()) {
    egressSource_->eom();
  }
}

void HTTPConnectStream::close() {
  shutdownRead();
  shutdownWrite();
}

bool HTTPConnectStream::canRead() const {
  return *ingressSource_;
}

bool HTTPConnectStream::canWrite() const {
  return (egressSource_ && !egressSource_->isEOMSeen() && !egressError_);
}

} // namespace proxygen::coro
