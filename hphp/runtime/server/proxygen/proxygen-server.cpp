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

#include "hphp/runtime/server/proxygen/proxygen-server.h"
#include <algorithm>
#include <exception>
#include <memory>
#include <thread>
#include "hphp/runtime/server/fake-transport.h"
#include "hphp/runtime/server/http-server.h"
#include "hphp/runtime/server/proxygen/proxygen-transport.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/base/crash-reporter.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/url.h"
#include "hphp/runtime/debugger/debugger.h"
#include "hphp/util/alloc.h"
#include "hphp/util/compatibility.h"
#include "hphp/util/process.h"

#include <folly/portability/Unistd.h>
#include <proxygen/lib/http/codec/HTTP2Constants.h>

namespace HPHP {

constexpr auto kPollInterval = std::chrono::milliseconds(60000); // 60 sec
constexpr uint32_t kStreamFlowControl = 1 << 20; // 1 MB
constexpr uint32_t kConnFlowControl = kStreamFlowControl * 1.5; // 1.5 MB
using folly::SocketAddress;
using folly::AsyncServerSocket;
using wangle::Acceptor;

HPHPSessionAcceptor::HPHPSessionAcceptor(
    const proxygen::AcceptorConfiguration& config,
    ProxygenServer *server)
      : HTTPSessionAcceptor(config),
        m_server(server) {
}

bool HPHPSessionAcceptor::canAccept(const SocketAddress& /*address*/) {
  // for now, we don't bother with the address whitelist
  return m_server->canAccept();
}

void HPHPSessionAcceptor::onIngressError(
#ifdef PROXYGEN_HTTP_SESSION_USES_BASE
  const proxygen::HTTPSessionBase& /*session*/,
#else
  const proxygen::HTTPSession& /*session*/,
#endif
  proxygen::ProxygenError error) {
  // This method is invoked when the HTTP library encountered some error before
  // it could completely parse the headers.  Most of these are HTTP garbage
  // (400 Bad Request) or client timeouts (408).
  FakeTransport transport((error == proxygen::kErrorTimeout) ? 408 : 400);
  transport.m_url = folly::to<std::string>("/onIngressError?error=",
                                           proxygen::getErrorString(error));
  m_server->onRequestError(&transport);
}

proxygen::HTTPTransaction::Handler*
HPHPSessionAcceptor::newHandler(proxygen::HTTPTransaction& /*txn*/,
                                proxygen::HTTPMessage* /*msg*/) noexcept {
  auto transport = std::make_shared<ProxygenTransport>(m_server);
  transport->setTransactionReference(transport);
  return transport.get();
}

void HPHPSessionAcceptor::onConnectionsDrained() {
  m_server->onConnectionsDrained();
}

ProxygenJob::ProxygenJob(std::shared_ptr<ProxygenTransport> t) :
    transport(t),
    reqStart(t->getRequestStart()) {
}

void ProxygenJob::getRequestStart(struct timespec *outReqStart) {
  *outReqStart = reqStart;
}

///////////////////////////////////////////////////////////////////////////////
// ProxygenTransportTraits

ProxygenTransportTraits::ProxygenTransportTraits(
  std::shared_ptr<ProxygenJob> job, void* opaque, int /*id*/)
    : server_((ProxygenServer*)opaque), transport_(std::move(job->transport)) {
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
ProxygenServer::ProxygenEventBaseObserver::ProxygenEventBaseObserver(
    uint32_t loop_sample_rate
  ) : m_sample_rate_(loop_sample_rate),
      m_busytime_estimator(std::chrono::seconds{60}),
      m_idletime_estimator(std::chrono::seconds{60}),
      m_evbLoopCountTimeSeries(ServiceData::createTimeSeries(
                                "proxygen_evb_loop_count",
                                {ServiceData::StatsType::COUNT},
                                {std::chrono::seconds(60)}, 60)) {
  m_counterCallback.init([this](std::map<std::string, int64_t>& values){
    auto now = ClockT::now();
    // export p90 p99 for evb busy time
    const std::array<double, 2> quantiles_busytime {0.9, 0.99};
    auto estimates = m_busytime_estimator.estimateQuantiles(
      quantiles_busytime, now).quantiles;

    values["proxygen_evb_busy_time_us.p90.60"] = estimates[0].second;
    values["proxygen_evb_busy_time_us.p99.60"] = estimates[1].second;

    // export p1 p10 for evb idle time
    const std::array<double, 2> quantiles_idletime {0.01, 0.1};
    estimates = m_idletime_estimator.estimateQuantiles(
      quantiles_idletime, now).quantiles;

    values["proxygen_evb_idle_time_us.p1.60"] = estimates[0].second;
    values["proxygen_evb_idle_time_us.p10.60"] = estimates[1].second;
  });
}

void ProxygenServer::ProxygenEventBaseObserver::loopSample(
  int64_t busytime /*usec */, int64_t idletime /* usec */) {
  auto now = ClockT::now();
  m_busytime_estimator.addValue(static_cast<double>(busytime), now);
  m_idletime_estimator.addValue(static_cast<double>(idletime), now);
  m_evbLoopCountTimeSeries->addValue(static_cast<int64_t>(getSampleRate()));
}

ProxygenServer::ProxygenServer(
  const ServerOptions& options
  ) : Server(options.m_address, options.m_port),
      m_accept_sock(options.m_serverFD),
      m_accept_sock_ssl(options.m_sslFD),
      m_worker(&m_eventBaseManager),
      m_dispatcher(options.m_maxThreads,
                   options.m_maxQueue,
                   RuntimeOption::ServerThreadDropCacheTimeoutSeconds,
                   RuntimeOption::ServerThreadDropStack,
                   this, RuntimeOption::ServerThreadJobLIFOSwitchThreshold,
                   RuntimeOption::ServerThreadJobMaxQueuingMilliSeconds,
                   kNumPriorities,
                   options.m_hugeThreads,
                   options.m_initThreads,
                   options.m_hugeStackKb,
                   options.m_extraKb,
                   options.m_legacyBehavior) {
  if (options.m_loop_sample_rate > 0) {
    m_worker.getEventBase()->setObserver(
      std::make_shared<ProxygenEventBaseObserver>(options.m_loop_sample_rate));
  }
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

  // Set flow control (for uploads) to 1MB.  We could also make this
  // configurable if needed
  m_httpsConfig.initialReceiveWindow = kStreamFlowControl;
  m_httpsConfig.receiveSessionWindowSize = kConnFlowControl;
  if (RuntimeOption::ServerEnableH2C) {
    m_httpConfig.allowedPlaintextUpgradeProtocols = {
      proxygen::http2::kProtocolCleartextString };
    m_httpConfig.initialReceiveWindow = kStreamFlowControl;
    m_httpConfig.receiveSessionWindowSize = kConnFlowControl;
  }

  if (!options.m_takeoverFilename.empty()) {
    m_takeover_agent.reset(new TakeoverAgent(options.m_takeoverFilename));
  }
  const std::vector<std::chrono::seconds> levels {
    std::chrono::seconds(10), std::chrono::seconds(120)};
  ProxygenTransport::s_requestErrorCount =
    ServiceData::createTimeSeries("http_response_error",
                                  {ServiceData::StatsType::COUNT},
                                  levels, 10);
  ProxygenTransport::s_requestNonErrorCount =
    ServiceData::createTimeSeries("http_response_nonerror",
                                  {ServiceData::StatsType::COUNT},
                                  levels, 10);
}

ProxygenServer::~ProxygenServer() {
  Logger::Verbose("%p: destroying ProxygenServer", this);
  waitForEnd();
  Logger::Info("%p: ProxygenServer destroyed", this);
}

int ProxygenServer::onTakeoverRequest(TakeoverAgent::RequestType type) {
  if (type == TakeoverAgent::RequestType::LISTEN_SOCKET) {
    // Subsequent calls to ProxygenServer::stop() won't do anything.
    // The server continues accepting until RequestType::TERMINATE is
    // seen.
    setStatus(RunStatus::STOPPING);
  } else if (type == TakeoverAgent::RequestType::TERMINATE) {
    stopListening(true /*hard*/);
    // No need to do m_takeover_agent->stop(), as the afdt server is
    // going to be closed when this returns.
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

std::unique_ptr<HPHPSessionAcceptor> ProxygenServer::createAcceptor(
    const proxygen::AcceptorConfiguration& config) {
  return std::make_unique<HPHPSessionAcceptor>(config, this);
}

void ProxygenServer::start() {
  m_httpServerSocket.reset(new AsyncServerSocket(m_worker.getEventBase()));
  bool needListen = true;
  auto failedToListen = [](const std::exception& ex,
                           const folly::SocketAddress& addr) {
    Logger::Error("failed to listen: %s", ex.what());
    throw FailedToListenException(addr.getAddressStr(), addr.getPort());
  };

  /*
   * Order of setting up m_httpServerSocket (for the main server only, not
   * including admin server, etc.).
   * (1) Try to use RuntimeOption::ServerPortFd (the inherited socket should
   * have already been bound to a proper port, but isn't listening yet).
   * (2) If (1) fails, and try to take over the socket of an old server.
   * (3) If both (1) and (2) fail, try to bind to RuntimeOption::ServerPort.
   */
  bool socketSetupSucceeded = false;
  if (m_accept_sock >= 0) {
    try {
      m_httpServerSocket->useExistingSocket(
        folly::NetworkSocket::fromFd(m_accept_sock));
      socketSetupSucceeded = true;
      Logger::Info("inheritfd: successfully inherited fd %d for server",
                   m_accept_sock);
    } catch (const std::exception& ex) {
      Logger::Warning("inheritfd: failed to inherit fd %d for server",
                      m_accept_sock);
    }
  }
  if (!socketSetupSucceeded && m_takeover_agent) {
    m_accept_sock = m_takeover_agent->takeover();
    if (m_accept_sock >= 0) {
      try {
        m_httpServerSocket->useExistingSocket(
          folly::NetworkSocket::fromFd(m_accept_sock));
        needListen = false;
        m_takeover_agent->requestShutdown();
        socketSetupSucceeded = true;
        Logger::Info("takeover: using takeover fd %d for server",
                     m_accept_sock);
      } catch (const std::exception& ex) {
        Logger::Warning("takeover: failed to takeover fd %d for server",
                        m_accept_sock);
      }
    }
  }
  if (!socketSetupSucceeded) {
    // make it possible to quickly reuse the port when needed.
    auto const allowReuse =
      RuntimeOption::StopOldServer || m_takeover_agent;
    m_httpServerSocket->setReusePortEnabled(allowReuse);
    try {
      m_httpServerSocket->bind(m_httpConfig.bindAddress);
    } catch (const std::exception& ex) {
      failedToListen(ex, m_httpConfig.bindAddress);
    }
  }

  if (m_takeover_agent) {
    m_takeover_agent->setupFdServer(m_worker.getEventBase()->getLibeventBase(),
                                    m_httpServerSocket->getNetworkSocket().toFd(), this);
  }

  m_httpAcceptor = createAcceptor(m_httpConfig);
  m_httpAcceptor->init(m_httpServerSocket.get(), m_worker.getEventBase());

  if (m_httpConfig.isSSL() || m_httpsConfig.isSSL()) {

    if (!RuntimeOption::SSLCertificateDir.empty()) {
      m_filePoller = std::make_unique<wangle::FilePoller>(
          kPollInterval);

      CertReloader::load(
        RuntimeOption::SSLCertificateDir,
        std::bind(&ProxygenServer::resetSSLContextConfigs,
          this,
          std::placeholders::_1));

      m_filePoller->addFileToTrack(
        RuntimeOption::SSLCertificateDir,
        [this, dir = RuntimeOption::SSLCertificateDir] {
          CertReloader::load(
            dir,
            std::bind(&ProxygenServer::resetSSLContextConfigs,
              this,
              std::placeholders::_1));
        });
    }
  }
  if (m_httpsConfig.isSSL()) {
    m_httpsServerSocket.reset(new AsyncServerSocket(m_worker.getEventBase()));
    try {
      if (m_accept_sock_ssl >= 0) {
        Logger::Info("inheritfd: using inherited fd %d for ssl",
                     m_accept_sock_ssl);
        m_httpsServerSocket->useExistingSocket(
          folly::NetworkSocket::fromFd(m_accept_sock_ssl));
      } else {
        m_httpsServerSocket->setReusePortEnabled(RuntimeOption::StopOldServer);
        m_httpsServerSocket->bind(m_httpsConfig.bindAddress);
      }
    } catch (const std::system_error& ex) {
      failedToListen(ex, m_httpsConfig.bindAddress);
    }

    m_httpsAcceptor = createAcceptor(m_httpsConfig);
    try {
      m_httpsAcceptor->init(m_httpsServerSocket.get(), m_worker.getEventBase());
    } catch (const std::exception& ex) {
      // Could be some cert thing
      failedToListen(ex, m_httpsConfig.bindAddress);
    }
  }

  if (!RuntimeOption::SSLTicketSeedFile.empty() &&
      (m_httpsAcceptor || m_httpConfig.isSSL())) {
    // setup ticket seed watcher
    const auto& ticketPath = RuntimeOption::SSLTicketSeedFile;
    auto seeds = wangle::TLSCredProcessor::processTLSTickets(ticketPath);
    if (seeds) {
      updateTLSTicketSeeds(std::move(*seeds));
    }
    m_credProcessor =
        std::make_unique<wangle::TLSCredProcessor>();
    m_credProcessor->setTicketPathToWatch(ticketPath);
    m_credProcessor->addTicketCallback([&](wangle::TLSTicketKeySeeds seeds) {
        updateTLSTicketSeeds(std::move(seeds));
    });
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
  folly::AsyncTimeout::attachEventBase(m_worker.getEventBase());
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
//    ServerPreShutdownWait + ServerShutdownListenWait +
//    ServerGracefulShutdownWait seconds
// 1. Set run status to STOPPING.  This should fail downstream healthchecks.
//    If it is the page server, it will continue accepting requests for
//    ServerPreShutdownWait seconds
// 2. Shutdown the listen sockets, this will
//     2.a Close any idle connections
//     2.b Send SPDY GOAWAY frames
//     2.c Insert Connection: close on HTTP/1.1 keep-alive connections as the
//         response headers are sent
// 3. If the server hasn't received the entire request body
//    ServerShutdownEOM seconds after shutdown starts, the request
//    will be aborted.  ServerShutdownEOM isn't required to be smaller
//    than ServerShutdownListenWait, but it makes sense to make it be,
//    in order to make shutdown faster.
// 4. After all requests finish executing OR all connections close OR
//    ServerShutdownListenWait seconds elapse, stop the VM.
//    Incomplete requests in the I/O thread will not be executed.
//    Stopping the VM is synchronous and all requests will run to
//    completion, unless the alarm fires.
// 5. Allow responses to drain for up to ServerGracefulShutdownWait seconds.
//    Note if shutting the VM down took non-zero time it's possible that the
//    alarm will fire first and kill this process.

void ProxygenServer::stop() {
  if (getStatus() != RunStatus::RUNNING ||
      m_shutdownState != ShutdownState::SHUTDOWN_NONE) return;

  Logger::Info("%p: Stopping ProxygenServer port=%d", this, m_port);

  setStatus(RunStatus::STOPPING);

  if (m_credProcessor) {
    m_credProcessor->stop();
  }

  if (m_filePoller) {
    m_filePoller->stop();
  }

  if (m_takeover_agent) {
    m_worker.getEventBase()->runInEventBaseThread([this] {
        m_takeover_agent->stop();
      });
  }

  // close listening sockets, this will initiate draining, including closing
  // idle conns
  m_worker.getEventBase()->runInEventBaseThread([this] {
      // Only wait ServerPreShutdownWait seconds for the page server.
      int delayMilliSeconds = RuntimeOption::ServerPreShutdownWait * 1000;
      if (delayMilliSeconds < 0 || getPort() != RuntimeOption::ServerPort) {
        delayMilliSeconds = 0;
      }
      m_worker.getEventBase()->runAfterDelay([this] { stopListening(); },
                                             delayMilliSeconds);
      reportShutdownStatus();
    });
}

void ProxygenServer::stopListening(bool hard) {
  m_shutdownState = ShutdownState::DRAINING_READS;
  HttpServer::MarkShutdownStat(ShutdownEvent::SHUTDOWN_DRAIN_READS);

  // triggers acceptStopped/sets acceptor state to Draining
  if (hard) {
    m_httpServerSocket.reset();
    m_httpsServerSocket.reset();
  } else {
    if (m_httpServerSocket) {
      m_httpServerSocket->stopAccepting();
    }
    if (m_httpsServerSocket) {
      m_httpsServerSocket->stopAccepting();
    }
  }

  if (RuntimeOption::ServerShutdownListenWait > 0) {
    std::chrono::seconds s(RuntimeOption::ServerShutdownListenWait);
    VLOG(4) << this << ": scheduling shutdown listen timeout="
            << s.count() << " port=" << m_port;
    scheduleTimeout(s);
    if (RuntimeOption::ServerShutdownEOMWait > 0) {
      int delayMilliSeconds = RuntimeOption::ServerShutdownEOMWait * 1000;
      m_worker.getEventBase()->runAfterDelay(
        [this] { abortPendingTransports(); }, delayMilliSeconds);
    }
  } else {
    doShutdown();
  }
}

void ProxygenServer::returnPartialPosts() {
  VLOG(2) << "Running returnPartialPosts for "
          << m_pendingTransports.size() << " pending transports";
  for (auto& transport : m_pendingTransports) {
    if (!transport.getClientComplete()) {
      transport.beginPartialPostEcho();
    }
  }
}

void ProxygenServer::abortPendingTransports() {
  if (!m_pendingTransports.empty()) {
    Logger::Warning("aborting %lu incomplete requests",
                    m_pendingTransports.size());
    // Avoid iterating the list, as abort() will unlink(), leaving the
    // list iterator in a corrupt state.
    do {
      auto& transport = m_pendingTransports.front();
      transport.abort();                // will unlink()
    } while (!m_pendingTransports.empty());
  }
  // Accelerate shutdown if all requests that were enqueued are done,
  // since no more is coming in.
  if (m_enqueuedCount == 0 &&
      m_shutdownState == ShutdownState::DRAINING_READS) {
    doShutdown();
  }
}

void ProxygenServer::onConnectionsDrained() {
  ++m_drainCount;
  Logger::Info("All connections drained from ProxygenServer drainCount=%d",
               m_drainCount);
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
    case ShutdownState::SHUTDOWN_NONE:
      // Transition from SHUTDOWN_NONE to DRAINING_READS needs to happen
      // explicitly through `stopListening`, not here.
      not_reached();
      break;
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
  }
}

void ProxygenServer::stopVM() {
  m_shutdownState = ShutdownState::STOPPING_VM;
  HttpServer::MarkShutdownStat(ShutdownEvent::SHUTDOWN_DRAIN_DISPATCHER);
  // we can't call m_dispatcher.stop() from the event loop, because it blocks
  // all I/O.  Spawn a thread to call it and callback when it's done.
  std::thread vmStopper([this] {
      purge_all();
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
  HttpServer::MarkShutdownStat(ShutdownEvent::SHUTDOWN_DRAIN_WRITES);
  if (!drained() && RuntimeOption::ServerGracefulShutdownWait > 0) {
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
  Logger::Info("%p: forceStop ProxygenServer port=%d, enqueued=%d, conns=%d",
               this, m_port, m_enqueuedCount, getLibEventConnectionCount());
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

  HttpServer::MarkShutdownStat(ShutdownEvent::SHUTDOWN_DONE);

  for (auto listener: m_listeners) {
    listener->serverStopped(this);
  }
}

void ProxygenServer::reportShutdownStatus() {
  if (m_port != RuntimeOption::ServerPort) return;
  if (getStatus() == RunStatus::STOPPED) return;
  Logger::FInfo("Shutdown state={}, a/q/e/p {}/{}/{}/{}, RSS={}Mb",
                static_cast<int>(m_shutdownState),
                getActiveWorker(),
                getQueuedJobs(),
                getLibEventConnectionCount(),
                m_pendingTransports.size(),
                Process::GetMemUsageMb());
  m_worker.getEventBase()->runAfterDelay([this]{reportShutdownStatus();}, 500);
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

void ProxygenServer::resetSSLContextConfigs(
  const std::vector<CertKeyPair>& paths) {
  std::vector<wangle::SSLContextConfig> configs;
  // always include the default cert/key
  auto cfg = createContextConfig({
        RuntimeOption::SSLCertificateFile,
        RuntimeOption::SSLCertificateKeyFile}, true);
  configs.emplace_back(cfg);

  for (const auto& path : paths) {
    configs.emplace_back(createContextConfig(path));
  }

#ifdef FACEBOOK // proxygen update
  auto evb = m_worker.getEventBase();
  evb->runInEventBaseThread([this, configs] {
      if (m_httpsAcceptor && m_httpsConfig.isSSL()) {
        m_httpsAcceptor->getServerSocketConfig(
          ).updateSSLContextConfigs(configs);
        m_httpsAcceptor->resetSSLContextConfigs();
      }
      if (m_httpAcceptor && m_httpConfig.isSSL()) {
        m_httpAcceptor->getServerSocketConfig(
          ).updateSSLContextConfigs(configs);
        m_httpAcceptor->resetSSLContextConfigs();
      }
  });
#endif
}

void ProxygenServer::updateTLSTicketSeeds(wangle::TLSTicketKeySeeds seeds) {
  auto evb = m_worker.getEventBase();
  evb->runInEventBaseThread([seeds = std::move(seeds), this] {
      if (m_httpsAcceptor) {
        m_httpsAcceptor->setTLSTicketSecrets(
            seeds.oldSeeds, seeds.currentSeeds, seeds.newSeeds);
      }
      if (m_httpAcceptor && m_httpConfig.isSSL()) {
        m_httpAcceptor->setTLSTicketSecrets(
            seeds.oldSeeds, seeds.currentSeeds, seeds.newSeeds);
      }
  });
}

bool ProxygenServer::enableSSL(int port) {
  if (port == 0) {
    return false;
  }
  SocketAddress address = m_httpConfig.bindAddress;
  address.setPort(port);

  m_httpsConfig.bindAddress = address;
  m_httpsConfig.strictSSL = false;

  m_httpsConfig.sslContextConfigs.emplace_back(
      createContextConfig({
        RuntimeOption::SSLCertificateFile,
        RuntimeOption::SSLCertificateKeyFile}, true));
  m_https = true;
  return true;
}

bool ProxygenServer::enableSSLWithPlainText() {
  m_httpConfig.strictSSL = false;
  m_httpConfig.sslContextConfigs.emplace_back(
      createContextConfig({
        RuntimeOption::SSLCertificateFile,
        RuntimeOption::SSLCertificateKeyFile}, true));
  m_httpConfig.allowInsecureConnectionsOnSecureServer = true;
  return true;
}

wangle::SSLContextConfig ProxygenServer::createContextConfig(
    const CertKeyPair& path,
    bool isDefault) {
  wangle::SSLContextConfig sslCtxConfig;

  if (RuntimeOption::SSLClientAuthLevel == 2) {
    sslCtxConfig.clientCAFile = RuntimeOption::SSLClientCAFile;
    sslCtxConfig.clientVerification =
      folly::SSLContext::SSLVerifyPeerEnum::VERIFY_REQ_CLIENT_CERT;
  } else if (RuntimeOption::SSLClientAuthLevel == 1) {
    sslCtxConfig.clientCAFile = RuntimeOption::SSLClientCAFile;
    sslCtxConfig.clientVerification =
      folly::SSLContext::SSLVerifyPeerEnum::VERIFY;
  } else {
    sslCtxConfig.clientVerification =
      folly::SSLContext::SSLVerifyPeerEnum::NO_VERIFY;
  }

  try {
    sslCtxConfig.setCertificate(path.certPath, path.keyPath, "");
    sslCtxConfig.sslVersion = folly::SSLContext::TLSv1;
    sslCtxConfig.isDefault = isDefault;
    sslCtxConfig.setNextProtocols({
      std::begin(RuntimeOption::ServerNextProtocols),
      std::end(RuntimeOption::ServerNextProtocols)
    });
  } catch (const std::exception& ex) {
    Logger::Error("Invalid certificate file or key file: %s", ex.what());
  }
  return sslCtxConfig;
}

void ProxygenServer::onRequest(std::shared_ptr<ProxygenTransport> transport) {
  if (CrashingThread.load(std::memory_order_relaxed) != 0) {
    Logger::Error("Discarding request while crashing");
    if (m_shutdownState == ShutdownState::SHUTDOWN_NONE) {
      m_shutdownState = ShutdownState::DRAINING_READS;
    }
    m_httpServerSocket.reset();
    m_httpsServerSocket.reset();
    if (RuntimeOption::Server503OnShutdownAbort) {
      transport->sendString("", 503);
      transport->onSendEnd();
    } else {
      transport->abort();
    }
    return;
  }

  if (getStatus() == RunStatus::RUNNING ||
      (getStatus() == RunStatus::STOPPING &&
       m_shutdownState <= ShutdownState::DRAINING_READS)) {
    RequestPriority priority = getRequestPriority(transport->getUrl());
    VLOG(4) << this << ": enqueing request with path=" << transport->getUrl() <<
      " and priority=" << priority;
    m_enqueuedCount++;
    transport->setEnqueued();
    m_dispatcher.enqueue(std::make_shared<ProxygenJob>(transport), priority);
  } else {
    // VM is shutdown
    if (RuntimeOption::Server503OnShutdownAbort) {
      transport->sendString("", 503);
      transport->onSendEnd();
    } else {
      transport->abort();
    }
    Logger::Error("%p: throwing away one new request while shutting down",
                  this);
  }
}

void ProxygenServer::decrementEnqueuedCount() {
  m_enqueuedCount--;
  if (m_enqueuedCount == 0 && isScheduled()) {
    // If all requests that got enqueued are done, and no more request
    // is coming in, accelerate shutdown.
    if ((m_shutdownState == ShutdownState::DRAINING_READS &&
         m_pendingTransports.empty()) ||
        m_shutdownState == ShutdownState::DRAINING_WRITES) {
      cancelTimeout();
      doShutdown();
    }
  }
}

ProxygenServer::RequestPriority ProxygenServer::getRequestPriority(
  const char *uri) {
  auto const command = URL::getCommand(URL::getServerObject(uri));
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
