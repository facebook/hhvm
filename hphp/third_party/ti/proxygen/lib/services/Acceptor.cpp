// Copyright 2004-present Facebook.  All rights reserved.
#include "ti/proxygen/lib/services/Acceptor.h"

#include "folly/ScopeGuard.h"
#include "thrift/lib/cpp/async/TAsyncSocket.h"
#include "thrift/lib/cpp/async/TEventBase.h"
#include "ti/proxygen/lib/utils/Exception.h"
#include "ti/proxygen/lib/services/ManagedConnection.h"
#include "ti/proxygen/lib/utils/Time.h"
#include "hphp/third_party/stubs/glog/portability.h"

#include <boost/cast.hpp>
#include <fcntl.h>
#include <fstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

using apache::thrift::async::TAsyncServerSocket;
using apache::thrift::async::TAsyncSocket;
using apache::thrift::async::TAsyncTimeoutSet;
using apache::thrift::async::TEventBase;
using apache::thrift::transport::TSocketAddress;
using apache::thrift::transport::TTransportException;
using std::shared_ptr;
using std::chrono::milliseconds;
using std::chrono::microseconds;
using std::string;
using std::filebuf;
using std::ifstream;
using std::ios;

namespace facebook { namespace proxygen {

static const std::string empty_string;

Acceptor::Acceptor(const AcceptorConfiguration& accConfig) :
  accConfig_(accConfig),
  socketOptions_(accConfig.getSocketOptions()) {
}

void
Acceptor::init(TAsyncServerSocket *serverSocket,
               TEventBase *eventBase) {
  CHECK_NULL(this->base_);

  base_ = eventBase;
  state_ = State::kRunning;
  downstreamConnectionManager_.reset(new ConnectionManager(
      eventBase, accConfig_.getConnectionIdleTime(), this));
  upstreamConnectionManager_.reset(new ConnectionManager(
      eventBase, accConfig_.getConnectionIdleTime()));
  transactionTimeouts_.reset(new TAsyncTimeoutSet(
      eventBase, accConfig_.getTransactionIdleTime()));
  tcpEventsTimeouts_.reset(new TAsyncTimeoutSet(
      eventBase, accConfig_.getTcpEventsConfig().getTimeout()));

  serverSocket->addAcceptCallback(this, eventBase);
  // SO_KEEPALIVE is the only setting that is inherited by accepted
  // connections so only apply this setting
  for (const auto& option: socketOptions_) {
    if (option.first.level == SOL_SOCKET &&
        option.first.optname == SO_KEEPALIVE && option.second == 1) {
      serverSocket->setKeepAliveEnabled(true);
      break;
    }
  }
}

Acceptor::~Acceptor(void) {
}

void
Acceptor::closeIdleConnections() {
  if (downstreamConnectionManager_) {
    downstreamConnectionManager_->closeIdleConnections();
  }
}

bool Acceptor::canAccept(const TSocketAddress& address) {
    return true;
  }

void
Acceptor::connectionAccepted(
    int fd, const TSocketAddress& clientAddr) noexcept {
  if (!canAccept(clientAddr)) {
    close(fd);
    return;
  }
  auto acceptTime = getCurrentTime();
    TransportInfo tinfo;
    tinfo.acceptTime = acceptTime;
    TAsyncSocket::UniquePtr sock(new TAsyncSocket(base_, fd));
    connectionReady(std::move(sock), clientAddr, empty_string, tinfo);
  }

void
Acceptor::connectionReady(
    TAsyncSocket::UniquePtr sock,
    const TSocketAddress& clientAddr,
    const string& nextProtocolName,
    TransportInfo& tinfo) {
  // Limit the number of reads from the socket per poll loop iteration,
  // both to keep memory usage under control and to prevent one fast-
  // writing client from starving other connections.
  sock->setMaxReadsPerEvent(16);
  tinfo.initWithSocket(sock.get());
  for (const auto& opt: socketOptions_) {
    opt.first.apply(sock->getFd(), opt.second);
  }
  onNewConnection(std::move(sock), &clientAddr, nextProtocolName, tinfo);
}

void
Acceptor::acceptError(const std::exception& ex) noexcept {
  // An error occurred.
  // The most likely error is out of FDs.  TAsyncServerSocket will back off
  // briefly if we are out of FDs, then continue accepting later.
  // Just log a message here.
  LOG(ERROR) << "error accepting on acceptor socket: " << ex.what();
}

void
Acceptor::acceptStopped() noexcept {
  // Close all of the idle connections
  closeIdleConnections();

  // If we haven't yet finished draining, begin doing so by marking ourselves
  // as in the draining state. We must be sure to hit checkDrained() here, as
  // if we're completely idle, we can should consider ourself drained
  // immediately (as there is no outstanding work to complete to cause us to
  // re-evaluate this).
  if (state_ != State::kDone) {
    state_ = State::kDraining;
    checkDrained();
  }
}

void
Acceptor::onEmpty(const ConnectionManager& cm) {
  if (state_ == State::kDraining) {
    checkDrained();
  }
}

void
Acceptor::checkDrained() {
  CHECK(state_ == State::kDraining);
  if (forceShutdownInProgress_ ||
      (downstreamConnectionManager_->getNumConnections() != 0)) {
    return;
  }

  LOG(INFO) << "All connections drained from Acceptor=" << this << " in thread "
            << base_;

  downstreamConnectionManager_.reset();

  state_ = State::kDone;

  onConnectionsDrained();
}

milliseconds
Acceptor::getConnTimeout() const {
  return accConfig_.getConnectionIdleTime();
}

void Acceptor::addConnection(ManagedConnection *conn) {
  // Add the socket to the timeout manager so that it can be cleaned
  // up after being left idle for a long time.
  downstreamConnectionManager_->addConnection(conn, true);
}

void
Acceptor::forceStop() {
  base_->runInEventBaseThread([&] { dropAllConnections(); });
}

void
Acceptor::dropAllConnections() {
  if (downstreamConnectionManager_) {
    LOG(INFO) << "Dropping all connections from Acceptor=" << this <<
      " in thread " << base_;
    assert(base_->isInEventBaseThread());
    forceShutdownInProgress_ = true;
    downstreamConnectionManager_->dropAllConnections();
  }
}

void Acceptor::flushStats() {
}

}} // facebook::proxygen
