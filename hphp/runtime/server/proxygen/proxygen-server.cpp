/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/server/proxygen/proxygen-server.h"
#include <algorithm>
#include <exception>
#include <memory>
#include <thread>
#include "hphp/runtime/server/fake-transport.h"
#include "hphp/runtime/server/proxygen/proxygen-transport.h"
#include "hphp/runtime/server/server-name-indication.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/base/crash-reporter.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/url.h"
#include "hphp/runtime/debugger/debugger.h"
#include "hphp/util/compatibility.h"

namespace HPHP {

using folly::EventBase;
using folly::SocketAddress;
using folly::AsyncTimeout;
using apache::thrift::transport::TTransportException;
using folly::AsyncServerSocket;
using wangle::Acceptor;
using proxygen::SPDYCodec;
using std::shared_ptr;
using std::string;

HPHPSessionAcceptor::HPHPSessionAcceptor(
    const proxygen::AcceptorConfiguration& config,
    ProxygenServer *server)
      : HTTPSessionAcceptor(config),
        m_server(server) {
}

bool HPHPSessionAcceptor::canAccept(const SocketAddress& address) {
  // for now, we don't bother with the address whitelist
  return m_server->canAccept();
}

void HPHPSessionAcceptor::onIngressError(const proxygen::HTTPSession& session,
                                         proxygen::ProxygenError error) {
  // This method is invoked when the HTTP library encountered some error before
  // it could completely parse the headers.  Most of these are HTTP garbage
  // (400 Bad Request) or client timeouts (408).
  FakeTransport transport((error == proxygen::kErrorTimeout) ? 408 : 400);
  transport.m_url = folly::to<string>("/onIngressError?error=",
                                      proxygen::getErrorString(error));
  m_server->onRequestError(&transport);
}

proxygen::HTTPTransaction::Handler* HPHPSessionAcceptor::newHandler(
  proxygen::HTTPTransaction& txn,
  proxygen::HTTPMessage *msg) noexcept {
  auto transport = std::make_shared<ProxygenTransport>(
    m_server, m_server->getEventBase());
  transport->setTransactionReference(transport);
  return transport.get();
}

void HPHPSessionAcceptor::onConnectionsDrained() {
  m_server->onConnectionsDrained();
}


ProxygenJob::ProxygenJob(shared_ptr<ProxygenTransport> t) :
    transport(t),
    reqStart(t->getRequestStart()) {
}

void ProxygenJob::getRequestStart(struct timespec *outReqStart) {
  *outReqStart = reqStart;
}

///////////////////////////////////////////////////////////////////////////////
// ProxygenTransportTraits

ProxygenTransportTraits::ProxygenTransportTraits(
    std::shared_ptr<ProxygenJob> job,
    void *opaque,
    int id)
  : server_((ProxygenServer*)opaque)
  , transport_(std::move(job->transport))
{
  VLOG(4) << "executing request with path=" << transport_->getUrl();
}

ProxygenTransportTraits::~ProxygenTransportTraits() {
  // ProxygenTransport must be released in worker thread
  ProxygenTransport *transport = transport_.get();
  transport->finish(std::move(transport_));
}

void HPHPWorkerThread::setup() {
  WorkerThread::setup();
  hphp_thread_init();
}

void HPHPWorkerThread::cleanup() {
  hphp_thread_exit();
  WorkerThread::cleanup();
}

///////////////////////////////////////////////////////////////////////////////
ProxygenServer::ProxygenServer(
    const ServerOptions& options
  ) : Server(options.m_address, options.m_port, options.m_numThreads),
      m_accept_sock(options.m_serverFD),
      m_accept_sock_ssl(options.m_sslFD),
      m_worker(&m_eventBaseManager),
      m_dispatcher(options.m_numThreads, RuntimeOption::ServerThreadRoundRobin,
                   RuntimeOption::ServerThreadDropCacheTimeoutSeconds,
                   RuntimeOption::ServerThreadDropStack,
                   this, RuntimeOption::ServerThreadJobLIFOSwitchThreshold,
                   RuntimeOption::ServerThreadJobMaxQueuingMilliSeconds,
                   kNumPriorities, RuntimeOption::QueuedJobsReleaseRate) {

  SocketAddress address;
  if (options.m_address.empty()) {
    address.setFromLocalPort(options.m_port);
  } else {
    address.setFromHostPort(options.m_address, options.m_port);
  }
  m_httpConfig.bindAddress = address;
  m_httpConfig.acceptBacklog = RuntimeOption::ServerBacklog;
  m_httpsConfig.acceptBacklog = RuntimeOption::ServerBacklog;
  // TODO: proxygen only supports downstream keep-alive
  std::chrono::seconds timeout;
  if (RuntimeOption::ConnectionTimeoutSeconds > 0) {
    timeout = std::chrono::seconds(RuntimeOption::ConnectionTimeoutSeconds);
  } else {
    // default to 50s (to match libevent)
    timeout = std::chrono::seconds(50);
  }
  m_httpConfig.connectionIdleTimeout = timeout;
  m_httpConfig.transactionIdleTimeout = timeout;
  m_httpsConfig.connectionIdleTimeout = timeout;
  m_httpsConfig.transactionIdleTimeout = timeout;

  if (!options.m_takeoverFilename.empty()) {
    m_takeover_agent.reset(new TakeoverAgent(options.m_takeoverFilename));
  }
}

int ProxygenServer::onTakeoverRequest(TakeoverAgent::RequestType type) {
  if (type == TakeoverAgent::RequestType::LISTEN_SOCKET) {
    // TODO: don't pause here, wait for the TERMINATE message to take
    // any action.  For now I'm copying broken LibEventServer
    // behavior.
    m_httpServerSocket->pauseAccepting();
  } else if (type == TakeoverAgent::RequestType::TERMINATE) {
    stopListening(true /*hard*/);
  }
  return 0;
}

void ProxygenServer::takeoverAborted() {
  m_httpServerSocket.reset();
}

void ProxygenServer::addTakeoverListener(TakeoverListener* listener) {
  if (m_takeover_agent) {
    m_takeover_agent->addTakeoverListener(listener);
  }
}

void ProxygenServer::removeTakeoverListener(TakeoverListener* listener) {
  if (m_takeover_agent) {
    m_takeover_agent->removeTakeoverListener(listener);
  }
}

void ProxygenServer::start() {
  m_httpServerSocket.reset(new AsyncServerSocket(m_worker.getEventBase()));
  bool needListen = true;
  auto failedToListen = [](const std::exception& ex,
                           const folly::SocketAddress& addr) {
    Logger::Error("%s", ex.what());
    throw FailedToListenException(addr.getAddressStr(),
                                  addr.getPort());
  };

  try {
    if (m_accept_sock >= 0) {
      Logger::Info("inheritfd: using inherited fd %d for server",
                   m_accept_sock);
      m_httpServerSocket->useExistingSocket(m_accept_sock);
    } else {
      m_httpServerSocket->bind(m_httpConfig.bindAddress);
    }
  } catch (const std::system_error& ex) {
    bool takoverSucceeded = false;
    if (ex.code().value() == EADDRINUSE &&
        m_takeover_agent) {
      m_accept_sock = m_takeover_agent->takeover();
      if (m_accept_sock >= 0) {
        Logger::Info("takeover: using takeover fd %d for server",
                     m_accept_sock);
        m_httpServerSocket->useExistingSocket(m_accept_sock);
        needListen = false;
        m_takeover_agent->requestShutdown();
        takoverSucceeded = true;
      }
    }
    if (!takoverSucceeded) {
      failedToListen(ex, m_httpConfig.bindAddress);
    }
  }
  if (m_takeover_agent) {
    m_takeover_agent->setupFdServer(m_worker.getEventBase()->getLibeventBase(),
                                    m_httpServerSocket->getSocket(), this);
  }

  m_httpAcceptor.reset(new HPHPSessionAcceptor(m_httpConfig, this));
  m_httpAcceptor->init(m_httpServerSocket.get(), m_worker.getEventBase());
  if (m_httpsConfig.isSSL()) {
    m_httpsServerSocket.reset(new AsyncServerSocket(m_worker.getEventBase()));
    try {
      if (m_accept_sock_ssl >= 0) {
        Logger::Info("inheritfd: using inherited fd %d for ssl",
                     m_accept_sock_ssl);
        m_httpsServerSocket->useExistingSocket(m_accept_sock_ssl);
      } else {
        m_httpsServerSocket->bind(m_httpsConfig.bindAddress);
      }
    } catch (const TTransportException& ex) {
      failedToListen(ex, m_httpsConfig.bindAddress);
    }

    m_httpsAcceptor.reset(new HPHPSessionAcceptor(m_httpsConfig, this));
    try {
      m_httpsAcceptor->init(m_httpsServerSocket.get(), m_worker.getEventBase());
    } catch (const std::exception& ex) {
      // Could be some cert thing
      failedToListen(ex, m_httpsConfig.bindAddress);
    }
  }
  if (needListen) {
    try {
      m_httpServerSocket->listen(m_httpConfig.acceptBacklog);
    } catch (const std::system_error& ex) {
      failedToListen(ex, m_httpConfig.bindAddress);
    }
  }
  if (m_httpsServerSocket) {
    try {
      m_httpsServerSocket->listen(m_httpsConfig.acceptBacklog);
    } catch (const std::system_error& ex) {
      failedToListen(ex, m_httpsConfig.bindAddress);
    }
  }
  m_httpServerSocket->startAccepting();
  if (m_httpsServerSocket) {
    m_httpsServerSocket->startAccepting();
  }
  startConsuming(m_worker.getEventBase(), &m_responseQueue);

  setStatus(RunStatus::RUNNING);
  AsyncTimeout::attachEventBase(m_worker.getEventBase());
  m_worker.start();
  m_dispatcher.start();
}

void ProxygenServer::waitForEnd() {
  Logger::Info("%p: Waiting for ProxygenServer port=%d", this, m_port);
  // m_worker.wait is always safe to call from any thread at any time.
  m_worker.wait();
}

// Server shutdown - Explained
//
// 0. An alarm may be set in http-server.cpp to kill this process after
//    ServerShutdownListenWait + ServerGracefulShutdownWait seconds
//
// 1. Set run status to STOPPING.  This should fail downstream healthchecks
// 2. TODO: there should be a timeout for failing healthchecks only
// 3. Shutdown the listen sockets, this will
//     3.a Close any idle connections
//     3.b Send SPDY GOAWAY frames
//     3.c Insert Connection: close on HTTP/1.1 keep-alive connections as the
//         response headers are sent
//    Note: LibEventServer doesn't close the listen sockets if there is no
//    ServerShutdownListenWait.
// 4. After all connections close OR ServerShutdownListenWait seconds
//    elapse, stop the VM.  Incomplete requests in the I/O thread will not be
//    executed.  Stopping the VM is synchronous and all requests will run to
//    completion, unless the alarm fires.
// 5. Allow responses to drain for up to ServerGracefulShutdownWait seconds.
//    Note if shutting the VM down took non-zero time it's possible that the
//    alarm will fire first and kill this process.

void ProxygenServer::stop() {
  if (getStatus() != RunStatus::RUNNING ||
      m_shutdownState != ShutdownState::SHUTDOWN_NONE) return;

  Logger::Info("%p: Stopping ProxygenServer port=%d", this, m_port);

  setStatus(RunStatus::STOPPING);
  // TODO: allow a configurable timeout before proceeding to the next phase

  // close listening sockets, this will initiate draining, including closing
  // idle conns
  m_worker.getEventBase()->runInEventBaseThread([&] {
      if (m_takeover_agent) {
        m_takeover_agent->stop();
      }

      stopListening();
    });
}

void ProxygenServer::stopListening(bool hard) {
  m_shutdownState = ShutdownState::DRAINING_READS;

#define SHUT_FBLISTEN 3
  /*
   * Modifications to the Linux kernel to support shutting down a listen
   * socket for new connections only, but anything which has completed
   * the TCP handshake will still be accepted.  This allows for un-accepted
   * connections to be queued and then wait until all queued requests are
   * actively being processed.
   */

  // triggers acceptStopped/sets acceptor state to Draining
  if (hard) {
    m_httpServerSocket.reset();
    m_httpsServerSocket.reset();
  } else {
    if (m_httpServerSocket) {
      m_httpServerSocket->stopAccepting(SHUT_FBLISTEN);
    }
    if (m_httpsServerSocket) {
      m_httpsServerSocket->stopAccepting(SHUT_FBLISTEN);
    }
  }

  if (RuntimeOption::ServerShutdownListenWait > 0) {
    std::chrono::seconds s(RuntimeOption::ServerShutdownListenWait);
    VLOG(4) << this << ": scheduling shutdown listen timeout=" <<
      s.count() << " port=" << m_port;
    scheduleTimeout(s);
  } else {
    doShutdown();
  }
}

void ProxygenServer::onConnectionsDrained() {
  VLOG(2) << "All connections drained from ProxygenServer drainCount=" <<
    m_drainCount;

  ++m_drainCount;
  if (!drained()) {
    // both servers have to finish
    Logger::Verbose("%p: waiting for other server port=%d", this, m_port);
    return;
  }

  // Stop the graceful shutdown timer
  cancelTimeout();

  // proceed to next shutdown phase
  doShutdown();
}

void ProxygenServer::timeoutExpired() noexcept {
  Logger::Info("%p: shutdown timer expired for ProxygenServer port=%d, "
               "state=%d, a/q/e %d/%d/%d", this, m_port, (int)m_shutdownState,
               getActiveWorker(), getQueuedJobs(),
               getLibEventConnectionCount());
  // proceed to next shutdown phase
  doShutdown();
}

void ProxygenServer::doShutdown() {
  switch(m_shutdownState) {
    case ShutdownState::DRAINING_READS:
      // Even though connections may be open for reading, they will not be
      // executed in the VM
      stopVM();
      break;
    case ShutdownState::STOPPING_VM:
      // this is a no-op, and can happen when we triggered VM shutdown from
      // the timeout code path, but the connections drain while waiting for
      // shutdown.  We let the VM shutdown continue and it will advance us
      // to the next state.
      break;
    case ShutdownState::DRAINING_WRITES:
      forceStop();
      break;
    default:
      CHECK(false);
  }
}

void ProxygenServer::stopVM() {
  m_shutdownState = ShutdownState::STOPPING_VM;
  // we can't call m_dispatcher.stop() from the event loop, because it blocks
  // all I/O.  Spawn a thread to call it and callback when it's done.
  std::thread vmStopper([this] {
      Logger::Info("%p: Stopping dispatcher port=%d", this, m_port);
      m_dispatcher.stop();
      Logger::Info("%p: Dispatcher stopped port=%d.  conns=%d", this, m_port,
                   getLibEventConnectionCount());
      m_worker.getEventBase()->runInEventBaseThread([this] {
          vmStopped();
        });
    });
  vmStopper.detach();
}

void ProxygenServer::vmStopped() {
  m_shutdownState = ShutdownState::DRAINING_WRITES;
  if (!drained() && RuntimeOption::ServerGracefulShutdownWait > 0 &&
      m_enqueuedCount > 0) {
    m_worker.getEventBase()->runInEventBaseThread([&] {
        std::chrono::seconds s(RuntimeOption::ServerGracefulShutdownWait);
        VLOG(4) << this << ": scheduling graceful timeout=" << s.count() <<
          " port=" << m_port;
        scheduleTimeout(s);
      });
  } else {
    forceStop();
  }
}

void ProxygenServer::forceStop() {
  Logger::Info("%p: forceStop ProxygenServer port=%d", this, m_port);

  m_httpServerSocket.reset();
  m_httpsServerSocket.reset();

  // Drops all open connections
  if (m_httpAcceptor && m_httpAcceptor->getState() < Acceptor::State::kDone) {
    m_httpAcceptor->forceStop();
  }
  if (m_httpsAcceptor && m_httpAcceptor->getState() < Acceptor::State::kDone) {
    m_httpsAcceptor->forceStop();
  }

  // No more responses coming from worker threads
  stopConsuming();
  Logger::Verbose("%p: Stopped response queue consumer port=%d", this, m_port);

  // The worker should exit gracefully
  m_worker.stopWhenIdle();
  Logger::Verbose("%p: i/o thread notified to stop port=%d", this, m_port);

  // Aaaand we're done - oops not thread safe.  Does it matter?
  setStatus(RunStatus::STOPPED);
  for (auto listener: m_listeners) {
    listener->serverStopped(this);
  }
}

bool ProxygenServer::canAccept() {
  return (RuntimeOption::ServerConnectionLimit == 0 ||
          getLibEventConnectionCount() < RuntimeOption::ServerConnectionLimit);
}

int ProxygenServer::getLibEventConnectionCount() {
  uint32_t conns = m_httpAcceptor->getNumConnections();
  if (m_httpsAcceptor) {
    conns += m_httpsAcceptor->getNumConnections();
  }
  return conns;
}

bool ProxygenServer::initialCertHandler(const std::string &server_name,
                                        const std::string &key_file,
                                        const std::string &cert_file,
                                        bool duplicate) {
  if (duplicate) {
    return true;
  }
  try {
    proxygen::SSLContextConfig sslCtxConfig;
    sslCtxConfig.setCertificate(cert_file, key_file, "");
    sslCtxConfig.sslVersion = folly::SSLContext::TLSv1;
    sslCtxConfig.sniNoMatchFn =
      std::bind(&ProxygenServer::sniNoMatchHandler, this,
                std::placeholders::_1);
    sslCtxConfig.setNextProtocols({
      std::begin(RuntimeOption::ServerNextProtocols),
      std::end(RuntimeOption::ServerNextProtocols)
    });
    m_httpsConfig.sslContextConfigs.emplace_back(sslCtxConfig);
    return true;
  } catch (const std::exception &ex) {
    Logger::Error(folly::to<std::string>(
      "Invalid certificate file or key file: ", ex.what()));
    return false;
  }
}

bool ProxygenServer::dynamicCertHandler(const std::string &server_name,
                                        const std::string &key_file,
                                        const std::string &cert_file) {
  try {
    proxygen::SSLContextConfig sslCtxConfig;
    sslCtxConfig.setCertificate(cert_file, key_file, "");
    sslCtxConfig.sslVersion = folly::SSLContext::TLSv1;
    sslCtxConfig.sniNoMatchFn =
      std::bind(&ProxygenServer::sniNoMatchHandler, this,
                std::placeholders::_1);
    sslCtxConfig.setNextProtocols({
      std::begin(RuntimeOption::ServerNextProtocols),
      std::end(RuntimeOption::ServerNextProtocols)
    });
    m_httpsAcceptor->addSSLContextConfig(sslCtxConfig);
    return true;
  } catch (const std::exception &ex) {
    Logger::Error(folly::to<string>("Invalid certificate file or key file: ",
                                    ex.what()));
    return false;
  }
}

bool ProxygenServer::sniNoMatchHandler(const char *server_name) {
  string fqdn(server_name);
  size_t pos = fqdn.find('.');
  std::string wildcard;
  if (pos != std::string::npos) {
    wildcard = fqdn.substr(pos + 1);
  }

  LOG(INFO) << "looking for '" << wildcard << "', '" << fqdn << "'";
  return ServerNameIndication::loadFromFile(
    wildcard, false,
    std::bind(&ProxygenServer::dynamicCertHandler, this,
              std::placeholders::_1,
              std::placeholders::_2,
              std::placeholders::_3)) ||
    ServerNameIndication::loadFromFile(
      fqdn, false,
      std::bind(&ProxygenServer::dynamicCertHandler, this,
                std::placeholders::_1,
                std::placeholders::_2,
                std::placeholders::_3));
}

bool ProxygenServer::enableSSL(int port) {
  if (port == 0) {
    return false;
  }
  SocketAddress address = m_httpConfig.bindAddress;
  address.setPort(port);

  m_httpsConfig.bindAddress = address;
  m_httpsConfig.strictSSL = false;
  if (RuntimeOption::SSLCertificateFile != "" &&
      RuntimeOption::SSLCertificateKeyFile != "") {
    try {
      m_sslCtxConfig.setCertificate(RuntimeOption::SSLCertificateFile,
                                    RuntimeOption::SSLCertificateKeyFile, "");
      m_sslCtxConfig.sslVersion = folly::SSLContext::TLSv1;
      m_sslCtxConfig.isDefault = true;
      m_sslCtxConfig.sniNoMatchFn =
        std::bind(&ProxygenServer::sniNoMatchHandler, this,
                  std::placeholders::_1);
      m_sslCtxConfig.setNextProtocols({
        std::begin(RuntimeOption::ServerNextProtocols),
        std::end(RuntimeOption::ServerNextProtocols)
      });
    } catch (const std::exception &ex) {
      Logger::Error(folly::to<string>("Invalid certificate file or key file: ",
                                      ex.what()));
    }
    if (!RuntimeOption::SSLCertificateDir.empty()) {
      ServerNameIndication::load(RuntimeOption::SSLCertificateDir,
                                 std::bind(&ProxygenServer::initialCertHandler,
                                           this,
                                           std::placeholders::_1,
                                           std::placeholders::_2,
                                           std::placeholders::_3,
                                           std::placeholders::_4));
    }
  } else {
    Logger::Error("Invalid certificate file or key file");
  }



  m_httpsConfig.sslContextConfigs.emplace_back(m_sslCtxConfig);
  m_https = true;
  return true;
}

void ProxygenServer::onRequest(shared_ptr<ProxygenTransport> transport) {
  // If we are in the process of crashing, we want to reject incoming work.
  // This will prompt the load balancers to choose another server. Using
  // shutdown rather than close has the advantage that it makes fewer changes
  // to the process (eg, it doesn't close the FD so if the FD number were
  // corrupted it would be mostly harmless).
  //
  // Note: the above comment came from LibEventServer, but ProxygenServer
  // just invokes AsyncServerSocket::destroy, which just calls close.
  if (IsCrashing) {
    Logger::Error("Discarding request while crashing");
    if (m_shutdownState == ShutdownState::SHUTDOWN_NONE) {
      m_shutdownState = ShutdownState::DRAINING_READS;
    }
    m_httpServerSocket.reset();
    m_httpsServerSocket.reset();
    transport->timeoutExpired();
    return;
  }

  if (getStatus() == RunStatus::RUNNING ||
      (getStatus() == RunStatus::STOPPING &&
       m_shutdownState == ShutdownState::DRAINING_READS)) {
    RequestPriority priority = getRequestPriority(transport->getUrl());
    VLOG(4) << this << ": enqueing request with path=" << transport->getUrl() <<
      " and priority=" << priority;
    m_enqueuedCount++;
    m_dispatcher.enqueue(std::make_shared<ProxygenJob>(transport), priority);
  } else {
    // VM is shutdown
    transport->timeoutExpired();
    Logger::Error("%p: throwing away one new request while shutting down",
                  this);
  }
}

void ProxygenServer::decrementEnqueuedCount() {
  m_enqueuedCount--;
  if (m_enqueuedCount == 0 &&
      m_shutdownState == ShutdownState::DRAINING_WRITES &&
      isScheduled()) {
    // If all requests that got enqueued are done, accelerate shutdown.
    // All other connections must be reading new requests, which cannot be
    // executed.
    cancelTimeout();
    timeoutExpired();
  }
}
ProxygenServer::RequestPriority ProxygenServer::getRequestPriority(
  const char *uri) {
  string command = URL::getCommand(URL::getServerObject(uri));
  if (RuntimeOption::ServerHighPriorityEndPoints.find(command) ==
      RuntimeOption::ServerHighPriorityEndPoints.end()) {
    return PRIORITY_NORMAL;
  }
  return PRIORITY_HIGH;
}

void ProxygenServer::onRequestError(Transport* transport) {
  if (!m_handler) {
    m_handler = createRequestHandler();
    if (!m_handler) {
      return;
    }
  }

  try {
    timespec start;
    Timer::GetMonotonicTime(start);
    transport->onRequestStart(start);
    m_handler->logToAccessLog(transport);
  } catch (...) {}
}

///////////////////////////////////////////////////////////////////////////////
}
