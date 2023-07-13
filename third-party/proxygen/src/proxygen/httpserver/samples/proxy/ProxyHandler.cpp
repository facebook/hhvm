/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ProxyHandler.h"

#include <folly/io/SocketOptionMap.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/portability/GFlags.h>
#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/ResponseBuilder.h>
#include <proxygen/lib/http/session/HTTPUpstreamSession.h>
#include <proxygen/lib/utils/URL.h>

#include "ProxyStats.h"

using namespace proxygen;
using std::string;
using std::unique_ptr;

DEFINE_int32(proxy_connect_timeout, 1000, "connect timeout in milliseconds");

namespace {
static const uint32_t kMinReadSize = 1460;
static const uint32_t kMaxReadSize = 4000;

static const uint8_t READS_SHUTDOWN = 1;
static const uint8_t WRITES_SHUTDOWN = 2;
static const uint8_t CLOSED = READS_SHUTDOWN | WRITES_SHUTDOWN;
} // namespace

namespace ProxyService {

ProxyHandler::ProxyHandler(ProxyStats* stats, folly::HHWheelTimer* timer)
    : stats_(stats), connector_{this, timer}, serverHandler_(*this) {
}

ProxyHandler::~ProxyHandler() {
  VLOG(4) << "deleting ProxyHandler";
}

void ProxyHandler::onRequest(std::unique_ptr<HTTPMessage> headers) noexcept {
  // This HTTP proxy does not obey the rules in the spec, such as stripping
  // hop-by-hop headers.  Example only!

  stats_->recordRequest();
  request_ = std::move(headers);
  proxygen::URL url(request_->getURL());

  folly::SocketAddress addr;
  try {
    // Note, this does a synchronous DNS lookup which is bad in event driven
    // code
    addr.setFromHostPort(url.getHost(), url.getPort());
  } catch (...) {
    ResponseBuilder(downstream_)
        .status(503, "Bad Gateway")
        .body(folly::to<string>("Could not parse server from URL: ",
                                request_->getURL()))
        .sendWithEOM();
    return;
  }

  downstream_->pauseIngress();
  LOG(INFO) << "Trying to connect to " << addr;
  auto evb = folly::EventBaseManager::get()->getEventBase();
  if (request_->getMethod() == HTTPMethod::CONNECT) {
    upstreamSock_ = folly::AsyncSocket::newSocket(evb);
    upstreamSock_->connect(this, addr, FLAGS_proxy_connect_timeout);
  } else {
    // A more sophisticated proxy would have a connection pool here
    const folly::SocketOptionMap opts{{{SOL_SOCKET, SO_REUSEADDR}, 1}};
    downstream_->pauseIngress();
    connector_.connect(folly::EventBaseManager::get()->getEventBase(),
                       addr,
                       std::chrono::milliseconds(FLAGS_proxy_connect_timeout),
                       opts);
  }
}

void ProxyHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {
  DestructorGuard dg(this);
  if (txn_) {
    LOG(INFO) << "Forwarding " << ((body) ? body->computeChainDataLength() : 0)
              << " body bytes to server";
    txn_->sendBody(std::move(body));
  } else if (upstreamSock_) {
    upstreamEgressPaused_ = true;
    upstreamSock_->writeChain(this, std::move(body));
    if (upstreamEgressPaused_) {
      downstreamIngressPaused_ = true;
      onServerEgressPaused();
    }
  } else {
    LOG(WARNING) << "Dropping " << ((body) ? body->computeChainDataLength() : 0)
                 << " body bytes to server";
  }
}

void ProxyHandler::onEOM() noexcept {
  if (txn_) {
    LOG(INFO) << "Forwarding client EOM to server";
    txn_->sendEOM();
  } else if (upstreamSock_) {
    LOG(INFO) << "Closing upgraded socket";
    sockStatus_ |= WRITES_SHUTDOWN;
    upstreamSock_->shutdownWrite();
  } else {
    LOG(INFO) << "Dropping client EOM to server";
  }
}

void ProxyHandler::connectSuccess(HTTPUpstreamSession* session) {
  DestructorGuard dg(this);
  LOG(INFO) << "Established " << *session;
  session_ = std::make_unique<SessionWrapper>(session);
  txn_ = session->newTransaction(&serverHandler_);
  LOG(INFO) << "Forwarding client request: " << request_->getURL()
            << " to server";
  txn_->sendHeaders(*request_);
  downstream_->resumeIngress();
}

void ProxyHandler::connectError(const folly::AsyncSocketException& ex) {
  DestructorGuard dg(this);
  LOG(ERROR) << "Failed to connect: " << folly::exceptionStr(ex);
  if (!clientTerminated_) {
    ResponseBuilder(downstream_).status(503, "Bad Gateway").sendWithEOM();
  } else {
    abortDownstream();
    checkForShutdown();
  }
}

void ProxyHandler::onServerHeadersComplete(
    unique_ptr<HTTPMessage> msg) noexcept {
  CHECK(!clientTerminated_);
  LOG(INFO) << "Forwarding " << msg->getStatusCode() << " response to client";
  downstream_->sendHeaders(*msg);
}

void ProxyHandler::onServerBody(std::unique_ptr<folly::IOBuf> chain) noexcept {
  CHECK(!clientTerminated_);
  LOG(INFO) << "Forwarding " << ((chain) ? chain->computeChainDataLength() : 0)
            << " body bytes to client";
  downstream_->sendBody(std::move(chain));
}

void ProxyHandler::onServerEOM() noexcept {
  if (!clientTerminated_) {
    LOG(INFO) << "Forwarding server EOM to client";
    downstream_->sendEOM();
  }
}

void ProxyHandler::detachServerTransaction() noexcept {
  txn_ = nullptr;
  checkForShutdown();
}

void ProxyHandler::onServerError(const HTTPException& error) noexcept {
  LOG(ERROR) << "Server error: " << error;
  abortDownstream();
}

void ProxyHandler::onServerEgressPaused() noexcept {
  if (!clientTerminated_) {
    downstream_->pauseIngress();
  }
}

void ProxyHandler::onServerEgressResumed() noexcept {
  if (!clientTerminated_) {
    downstream_->resumeIngress();
  }
}

void ProxyHandler::requestComplete() noexcept {
  clientTerminated_ = true;
  checkForShutdown();
}

void ProxyHandler::onError(ProxygenError err) noexcept {
  LOG(ERROR) << "Client error: " << proxygen::getErrorString(err);
  DestructorGuard dg(this);
  clientTerminated_ = true;
  if (txn_) {
    LOG(ERROR) << "Aborting server txn: " << *txn_;
    txn_->sendAbort();
  } else if (upstreamSock_) {
    upstreamSock_.reset();
  }
  checkForShutdown();
}

void ProxyHandler::onEgressPaused() noexcept {
  if (txn_) {
    txn_->pauseIngress();
  } else if (upstreamSock_) {
    upstreamSock_->setReadCB(nullptr);
  }
}

void ProxyHandler::onEgressResumed() noexcept {
  if (txn_) {
    txn_->resumeIngress();
  } else if (upstreamSock_) {
    upstreamSock_->setReadCB(this);
  }
}

void ProxyHandler::abortDownstream() {
  if (!clientTerminated_) {
    downstream_->sendAbort();
  }
}

bool ProxyHandler::checkForShutdown() {
  if (clientTerminated_ && !txn_ &&
      (!upstreamSock_ || (sockStatus_ == CLOSED && !upstreamEgressPaused_))) {
    destroy();
    return true;
  }
  return false;
}

void ProxyHandler::connectSuccess() noexcept {
  LOG(INFO) << "Connected to upstream " << upstreamSock_;
  DestructorGuard dg(this);
  ResponseBuilder(downstream_).status(200, "OK").send();
  upstreamSock_->setReadCB(this);
  downstream_->resumeIngress();
}

void ProxyHandler::connectErr(const folly::AsyncSocketException& ex) noexcept {
  connectError(ex);
}

void ProxyHandler::getReadBuffer(void** bufReturn, size_t* lenReturn) {
  std::pair<void*, uint32_t> readSpace =
      body_.preallocate(kMinReadSize, kMaxReadSize);
  *bufReturn = readSpace.first;
  *lenReturn = readSpace.second;
}

void ProxyHandler::readDataAvailable(size_t len) noexcept {
  body_.postallocate(len);
  downstream_->sendBody(body_.move());
}

void ProxyHandler::readEOF() noexcept {
  sockStatus_ |= READS_SHUTDOWN;
  onServerEOM();
}

void ProxyHandler::readErr(const folly::AsyncSocketException& ex) noexcept {
  DestructorGuard dg(this);
  LOG(ERROR) << "Server read error: " << folly::exceptionStr(ex);
  abortDownstream();
  upstreamSock_.reset();
  checkForShutdown();
}

void ProxyHandler::writeSuccess() noexcept {
  DestructorGuard dg(this);
  upstreamEgressPaused_ = false;
  if (downstreamIngressPaused_) {
    downstreamIngressPaused_ = false;
    onServerEgressResumed();
  }
  checkForShutdown();
}

void ProxyHandler::writeErr(size_t /*bytesWritten*/,
                            const folly::AsyncSocketException& ex) noexcept {
  LOG(ERROR) << "Server write error: " << folly::exceptionStr(ex);
  DestructorGuard dg(this);
  upstreamEgressPaused_ = false;
  abortDownstream();
  upstreamSock_.reset();
  checkForShutdown();
}

} // namespace ProxyService
