/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/server/fastcgi/fastcgi-server.h"
#include "hphp/runtime/server/fastcgi/fastcgi-transport.h"
#include "hphp/runtime/server/fastcgi/fastcgi-session.h"
#include "hphp/runtime/server/fastcgi/fastcgi-worker.h"
#include "hphp/runtime/server/fastcgi/socket-connection.h"
#include "folly/io/IOBuf.h"
#include "folly/io/IOBufQueue.h"
#include "thrift/lib/cpp/async/TEventBaseManager.h"
#include "thrift/lib/cpp/async/TAsyncTransport.h"
#include "ti/proxygen/lib/workers/WorkerThread.h"
#include "ti/proxygen/lib/services/Acceptor.h"

namespace HPHP {

using folly::IOBuf;
using folly::IOBufQueue;
using folly::io::Cursor;
using apache::thrift::async::TEventBase;
using apache::thrift::async::TAsyncTransport;
using apache::thrift::async::TAsyncServerSocket;
using apache::thrift::async::TAsyncTimeout;
using apache::thrift::transport::TSocketAddress;
using apache::thrift::transport::TTransportException;

////////////////////////////////////////////////////////////////////////////////

const int FastCGIAcceptor::k_maxConns = 50;
const int FastCGIAcceptor::k_maxRequests = 1000;
const TSocketAddress FastCGIAcceptor::s_unknownSocketAddress("0.0.0.0", 0);

bool FastCGIAcceptor::canAccept(const TSocketAddress& address) {
  // TODO: Support server IP whitelist.
  return m_server->canAccept();
}

void FastCGIAcceptor::onNewConnection(
    apache::thrift::async::TAsyncSocket::UniquePtr sock,
    const apache::thrift::transport::TSocketAddress* peerAddress,
    const std::string& nextProtocolName,
    const facebook::proxygen::TransportInfo& tinfo)
{
  TSocketAddress localAddress;
  try {
    sock->getLocalAddress(&localAddress);
  } catch (...) {
    localAddress = s_unknownSocketAddress;
  }

  FastCGIConnection* conn = new FastCGIConnection(
      m_server,
      std::move(sock),
      localAddress,
      *peerAddress
    );

  Acceptor::addConnection(conn);
};

void FastCGIAcceptor::onConnectionsDrained() {
  m_server->onConnectionsDrained();
}

////////////////////////////////////////////////////////////////////////////////

FastCGIConnection::FastCGIConnection(
  FastCGIServer* server,
  TAsyncTransport::UniquePtr sock,
  const TSocketAddress& localAddr,
  const TSocketAddress& peerAddr)
  : SocketConnection(std::move(sock), localAddr, peerAddr),
    m_server(server) {
  m_eventBase = m_server->getEventBaseManager()->getExistingEventBase();
  assert(m_eventBase != nullptr);
  m_sock->setReadCallback(this);
  m_session.setCallback(this);
}

FastCGIConnection::~FastCGIConnection() {
  m_transports.clear();
}

const uint32_t FastCGIConnection::k_minReadSize = 1460;
const uint32_t FastCGIConnection::k_maxReadSize = 4000;

void FastCGIConnection::getReadBuffer(void** bufReturn, size_t* lenReturn) {
  std::pair<void*, uint32_t> readSpace =
    m_readBuf.preallocate(k_minReadSize, k_maxReadSize);
  *bufReturn = readSpace.first;
  *lenReturn = readSpace.second;
}

void FastCGIConnection::readDataAvailable(size_t len) noexcept {
  DestructorGuard dg(this);

  m_readBuf.postallocate(len);
  resetTimeout();

  size_t length = m_session.onIngress(m_readBuf.front());
  m_readBuf.split(length);
}

void FastCGIConnection::readEOF() noexcept {
  shutdownTransport();
}

void FastCGIConnection::readError(const TTransportException& ex) noexcept {
  shutdownTransport();
}

bool FastCGIConnection::hasReadDataAvailable() {
  const IOBuf* chain = m_readBuf.front();
  return ((chain != nullptr) && (chain->length() != 0));
}

ProtocolSessionHandlerPtr FastCGIConnection::newSessionHandler(
                                               int transport_id) {
  auto transport = std::make_shared<FastCGITransport>(this, transport_id);
  m_transports[transport_id] = transport;
  return transport;
}

void FastCGIConnection::onSessionEgress(std::unique_ptr<IOBuf> chain) {
  m_sock->writeChain(nullptr, std::move(chain));
}

void FastCGIConnection::onSessionError() {
  onSessionClose();
}

void FastCGIConnection::onSessionClose() {
  shutdownTransport();
}

void FastCGIConnection::setMaxConns(int max_conns) {
  m_session.setMaxConns(max_conns);
}

void FastCGIConnection::setMaxRequests(int max_requests) {
  m_session.setMaxRequests(max_requests);
}

void FastCGIConnection::handleRequest(int transport_id) {
  assert(m_transports.count(transport_id));
  m_server->handleRequest(m_transports[transport_id]);
  m_transports.erase(transport_id);
}

////////////////////////////////////////////////////////////////////////////////

FastCGIServer::FastCGIServer(const std::string &address,
                             int port,
                             int workers)
  : Server(address, port, workers),
    m_worker(&m_eventBaseManager),
    m_dispatcher(workers,
                 RuntimeOption::ServerThreadRoundRobin,
                 RuntimeOption::ServerThreadDropCacheTimeoutSeconds,
                 RuntimeOption::ServerThreadDropStack,
                 this,
                 RuntimeOption::ServerThreadJobLIFOSwitchThreshold,
                 RuntimeOption::ServerThreadJobMaxQueuingMilliSeconds,
                 RequestPriority::k_numPriorities) {
  TSocketAddress sock_addr;
  if (sock_addr.empty()) {
    sock_addr.setFromLocalPort(port);
  } else {
    sock_addr.setFromHostPort(address, port);
  }
  m_socketConfig.setAddress(sock_addr);
  m_socketConfig.setAcceptBacklog(RuntimeOption::ServerBacklog);
  std::chrono::seconds timeout;
  if (RuntimeOption::ConnectionTimeoutSeconds > 0) {
    timeout = std::chrono::seconds(RuntimeOption::ConnectionTimeoutSeconds);
  } else {
    // default to 2 minutes
    timeout = std::chrono::seconds(120);
  }
  m_socketConfig.setConnectionIdleTime(timeout);
}

void FastCGIServer::addTakeoverListener(TakeoverListener* lisener) {
  // TODO? - LibEventServer doesn't implement it
}

void FastCGIServer::removeTakeoverListener(TakeoverListener* lisener) {
  // TODO? - LibEventServer doesn't implement it
}

void FastCGIServer::start() {
  m_socket.reset(new TAsyncServerSocket(m_worker.getEventBase()));
  try {
    m_socket->bind(m_socketConfig.getAddress());
  } catch (const apache::thrift::transport::TTransportException& ex) {
    LOG(ERROR) << ex.what();
    throw FailedToListenException(m_socketConfig.getAddress().getAddressStr(),
                                  m_socketConfig.getAddress().getPort());
  }
  m_acceptor.reset(new FastCGIAcceptor(m_socketConfig, this));
  m_acceptor->init(m_socket.get(), m_worker.getEventBase());
  m_worker.getEventBase()->runInEventBaseThread([&] {
      if (!m_socket) {
        // Someone called stop before we got here
        return;
      }
      m_socket->listen(m_socketConfig.getAcceptBacklog());
      m_socket->startAccepting();
    });
  setStatus(RunStatus::RUNNING);
  TAsyncTimeout::attachEventBase(m_worker.getEventBase());
  m_done = false;
  m_worker.start();
  m_dispatcher.start();
}

void FastCGIServer::waitForEnd() {
  if (!m_done) {
    m_done = true;
    m_worker.wait();
  }
}

void FastCGIServer::stop() {
  setStatus(RunStatus::STOPPING);

  m_worker.getEventBase()->runInEventBaseThread([&] {
      m_socket.reset();

      if (RuntimeOption::ServerGracefulShutdownWait > 0) {
        std::chrono::seconds s(RuntimeOption::ServerGracefulShutdownWait);
        std::chrono::milliseconds m(s);
        scheduleTimeout(m);
      } else {
        terminateServer();
      }
    });
}

void FastCGIServer::onConnectionsDrained() {
  cancelTimeout();
  terminateServer();
}

void FastCGIServer::timeoutExpired() noexcept {
  if (m_acceptor) {
    m_acceptor->forceStop();
  }

  terminateServer();
}

void FastCGIServer::terminateServer() {
  m_worker.stopWhenIdle();
  m_dispatcher.stop();

  setStatus(RunStatus::STOPPED);
}

bool FastCGIServer::canAccept() {
  return (RuntimeOption::ServerConnectionLimit == 0 ||
          getLibEventConnectionCount() < RuntimeOption::ServerConnectionLimit);
}

int FastCGIServer::getLibEventConnectionCount() {
  uint32_t conns = m_acceptor->getNumConnections();
  if (m_acceptor) {
    conns += m_acceptor->getNumConnections();
  }
  return conns;
}

void FastCGIServer::handleRequest(FastCGITransportPtr transport) {
  auto job = std::make_shared<FastCGIJob>(transport);
  m_dispatcher.enqueue(job);
}

////////////////////////////////////////////////////////////////////////////////
}

