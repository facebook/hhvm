/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/server/http-server.h"
#include "hphp/runtime/base/configs/server.h"

namespace HPHP {

////////////////////////////////////////////////////////////////////////////////

bool FastCGIAcceptor::canAccept(const folly::SocketAddress& /*address*/) {
  // TODO: Support server IP whitelist.
  auto const cons = m_server->getLibEventConnectionCount();
  return (Cfg::Server::ConnectionLimit == 0 ||
          cons < Cfg::Server::ConnectionLimit);
}

void FastCGIAcceptor::onNewConnection(
  folly::AsyncTransportWrapper::UniquePtr sock,
  const folly::SocketAddress* peerAddress,
  const std::string& /*nextProtocolName*/,
  SecureTransportType /*secureProtocolType*/,
  const ::wangle::TransportInfo& /*tinfo*/) {
  folly::SocketAddress localAddress;
  try {
    sock->getLocalAddress(&localAddress);
  } catch (std::system_error&) {
    // If getSockName fails it's bad news; abort the connection
    return;
  }

  // Will delete itself when it gets a closing callback
  auto session = new FastCGISession(
      m_server->getEventBaseManager()->getExistingEventBase(),
      m_server->getDispatcher(),
      std::move(sock),
      localAddress,
      *peerAddress
    );

  // NB: ~ManagedConnection will call removeConnection() before the session
  //     destroys itself.
  Acceptor::addConnection(session);
};

void FastCGIAcceptor::onConnectionsDrained() {
  m_server->onConnectionsDrained();
}

////////////////////////////////////////////////////////////////////////////////

FastCGIServer::FastCGIServer(const std::string &address,
                             int port,
                             int workers,
                             bool useFileSocket)
  : Server(address, port),
    m_worker(&m_eventBaseManager),
    m_dispatcher(workers, workers,
                 Cfg::Server::ThreadDropCacheTimeoutSeconds,
                 Cfg::Server::ThreadDropStack,
                 this,
                 RuntimeOption::ServerThreadJobLIFOSwitchThreshold,
                 Cfg::Server::ThreadJobMaxQueuingMilliSeconds,
                 RequestPriority::k_numPriorities) {
  folly::SocketAddress sock_addr;
  if (useFileSocket) {
    sock_addr.setFromPath(address);
  } else if (address.empty()) {
    sock_addr.setFromHostPort("localhost", port);
    assert(sock_addr.isLoopbackAddress());
  } else {
    sock_addr.setFromHostPort(address, port);
  }
  m_socketConfig.bindAddress = sock_addr;
  m_socketConfig.acceptBacklog = Cfg::Server::Backlog;
  std::chrono::seconds timeout;
  if (Cfg::Server::ConnectionTimeoutSeconds >= 0) {
    timeout = std::chrono::seconds(Cfg::Server::ConnectionTimeoutSeconds);
  } else {
    // default to 2 minutes
    timeout = std::chrono::seconds(120);
  }
  m_socketConfig.connectionIdleTimeout = timeout;
}

void FastCGIServer::start() {
  // It's not safe to call this function more than once
  m_socket.reset(new folly::AsyncServerSocket(m_worker.getEventBase()));
  try {
    m_socket->bind(m_socketConfig.bindAddress);
  } catch (const std::system_error& ex) {
    Logger::Error(std::string(ex.what()));
    if (m_socketConfig.bindAddress.getFamily() == AF_UNIX) {
      throw FailedToListenException(m_socketConfig.bindAddress.getPath());
    }
    throw FailedToListenException(m_socketConfig.bindAddress.getAddressStr(),
                                  m_socketConfig.bindAddress.getPort());
  }
  if (m_socketConfig.bindAddress.getFamily() == AF_UNIX) {
    auto path = m_socketConfig.bindAddress.getPath();
    chmod(path.c_str(), 0760);
  }
  m_acceptor.reset(new FastCGIAcceptor(m_socketConfig, this));
  m_acceptor->init(m_socket.get(), m_worker.getEventBase());
  m_worker.getEventBase()->runInEventBaseThread([&] {
    if (!m_socket) {
      // Someone called stop before we got here. With the exception of a
      // second call to start being made this should be safe as any place
      // we mutate m_socket is done within the event base.
      return;
    }
    m_socket->listen(m_socketConfig.acceptBacklog);
    m_socket->startAccepting();
  });
  setStatus(RunStatus::RUNNING);
  folly::AsyncTimeout::attachEventBase(m_worker.getEventBase());
  m_worker.start();
  m_dispatcher.start();
}

void FastCGIServer::waitForEnd() {
  // When m_worker stops the server has stopped accepting new requests, there
  // may be pedning vm jobs. wait() is always safe to call regardless of thread
  m_worker.wait();
}

void FastCGIServer::stop() {
  if (getStatus() != RunStatus::RUNNING) return; // nothing to do

  setStatus(RunStatus::STOPPING);
  HttpServer::MarkShutdownStat(ShutdownEvent::SHUTDOWN_DRAIN_READS);

  m_worker.getEventBase()->runInEventBaseThread([&] {
    // Shutdown the server socket. Unfortunately, we will drop all unaccepted
    // connections; there is no way to do a partial shutdown of a server socket
    m_socket->stopAccepting();

    if (Cfg::Server::GracefulShutdownWait > 0) {
      // Gracefully drain any incomplete requests. We cannot go offline until
      // they are finished as we own their dispatcher and event base.
      if (m_acceptor) {
        m_acceptor->startDrainingAllConnections();
      }

      std::chrono::seconds s(Cfg::Server::GracefulShutdownWait);
      std::chrono::milliseconds m(s);
      scheduleTimeout(m);
    } else {
      // Drop all connections. We cannot shutdown until they stop because we
      // own their dispatcher and event base.
      if (m_acceptor) {
        m_acceptor->forceStop();
      }

      terminateServer();
    }
  });
}

void FastCGIServer::onConnectionsDrained() {
  // NOTE: called from FastCGIAcceptor::onConnectionsDrained()
  cancelTimeout();
  terminateServer();
}

void FastCGIServer::timeoutExpired() noexcept {
  // Acceptor failed to drain connections on time; drop them so that we can
  // shutdown.
  if (m_acceptor) {
    m_acceptor->forceStop();
  }

  terminateServer();
}

void FastCGIServer::terminateServer() {
  if (getStatus() != RunStatus::STOPPING) {
    setStatus(RunStatus::STOPPING);
  }
  // Wait for the server socket thread to stop running
  m_worker.stopWhenIdle();

  HttpServer::MarkShutdownStat(ShutdownEvent::SHUTDOWN_DRAIN_DISPATCHER);
  // Wait for VMs to shutdown
  m_dispatcher.stop();

  setStatus(RunStatus::STOPPED);
  HttpServer::MarkShutdownStat(ShutdownEvent::SHUTDOWN_DONE);

  // Notify HttpServer that we've shutdown
  for (auto listener: m_listeners) {
    listener->serverStopped(this);
  }
}

////////////////////////////////////////////////////////////////////////////////
}
