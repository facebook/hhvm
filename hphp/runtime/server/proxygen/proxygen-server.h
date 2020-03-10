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

#ifndef incl_HPHP_HTTP_SERVER_PROXYGEN_SERVER_H_
#define incl_HPHP_HTTP_SERVER_PROXYGEN_SERVER_H_

#include "hphp/runtime/server/cert-reloader.h"
#include "hphp/runtime/server/proxygen/proxygen-transport.h"
#include "hphp/runtime/server/job-queue-vm-stack.h"
#include "hphp/runtime/server/server-worker.h"
#include "hphp/runtime/server/server.h"
#include <proxygen/lib/http/session/HTTPSessionAcceptor.h>
#include <proxygen/lib/services/WorkerThread.h>
#include <wangle/ssl/SSLContextConfig.h>
#include <wangle/ssl/TLSCredProcessor.h>
#include <folly/io/async/NotificationQueue.h>

#include <algorithm>
#include <folly/io/async/EventBaseManager.h>
#include <folly/stats/QuantileEstimator.h>
#include <memory>


namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
struct ProxygenJob : ServerJob {
  explicit ProxygenJob(std::shared_ptr<ProxygenTransport> transport);

  virtual void getRequestStart(struct timespec *reqStart);

  std::shared_ptr<ProxygenTransport> transport;
  struct timespec reqStart;
};

struct ProxygenTransportTraits;
using ProxygenWorker = ServerWorker<std::shared_ptr<ProxygenJob>,
                                    ProxygenTransportTraits>;

struct ProxygenServer;

struct HPHPSessionAcceptor : proxygen::HTTPSessionAcceptor {
  explicit HPHPSessionAcceptor(
    const proxygen::AcceptorConfiguration& config,
    ProxygenServer *server);
  ~HPHPSessionAcceptor() override {}

  proxygen::HTTPTransaction::Handler* newHandler(
    proxygen::HTTPTransaction& txn,
    proxygen::HTTPMessage *msg) noexcept override;

  bool canAccept(const folly::SocketAddress&) override;

  void onConnectionsDrained() override;

  void onIngressError(
#if PROXYGEN_HTTP_SESSION_USES_BASE
    const proxygen::HTTPSessionBase&,
#else
    const proxygen::HTTPSession&,
#endif
    proxygen::ProxygenError error) override;

  proxygen::HTTPSessionController* getController() override {
    return m_controllerPtr;
  }

  void setController(proxygen::HTTPSessionController* controller) {
    m_controllerPtr = controller;
  }

 private:
  ProxygenServer *m_server;
  proxygen::SimpleController m_simpleController{this};
  proxygen::HTTPSessionController* m_controllerPtr{&m_simpleController};
};

using ResponseMessageQueue = folly::NotificationQueue<ResponseMessage>;

struct HPHPWorkerThread : proxygen::WorkerThread {
  explicit HPHPWorkerThread(folly::EventBaseManager* ebm)
      : WorkerThread(ebm, "ProxygenWorker") {}
  ~HPHPWorkerThread() override {}
  void setup() override;
  void cleanup() override;
};

struct ProxygenServer : Server,
                        ResponseMessageQueue::Consumer,
                        folly::AsyncTimeout,
                        TakeoverAgent::Callback {
  explicit ProxygenServer(const ServerOptions& options);
  ~ProxygenServer() override;

  void addTakeoverListener(TakeoverListener* listener) override;
  void removeTakeoverListener(TakeoverListener* listener) override;
  void saturateWorkers() override {
    m_dispatcher.saturateWorkers();
  }
  void start() override;
  void waitForEnd() override;
  void stop() override;
  size_t getMaxThreadCount() override {
    return m_dispatcher.getMaxThreadCount();
  }
  int getActiveWorker() override {
    return m_dispatcher.getActiveWorker();
  }
  void updateMaxActiveWorkers(int num) override {
    return m_dispatcher.updateMaxActiveWorkers(num);
  }
  int getQueuedJobs() override {
    return m_dispatcher.getQueuedJobs();
  }
  int getLibEventConnectionCount() override;
  bool enableSSL(int port) override;
  bool enableSSLWithPlainText() override;

  void setMaxThreadCount(int max) {
    return m_dispatcher.setMaxThreadCount(max);
  }

  folly::EventBase *getEventBase() {
    return m_eventBaseManager.getEventBase();
  }

  void messageAvailable(ResponseMessage&& message) noexcept override {
    auto m_transport = message.m_transport;
    m_transport->messageAvailable(std::move(message));
  }

  bool canAccept();

  void onConnectionsDrained();

  /**
   * TakeoverAgent::Callback
   */
  int onTakeoverRequest(TakeoverAgent::RequestType type) override;

  void takeoverAborted() override;

  // Methods invoked by ProxygenTransport, virtual for mocking
  virtual void onRequest(std::shared_ptr<ProxygenTransport> transport);

  virtual void putResponseMessage(ResponseMessage&& message) {
    m_responseQueue.putMessage(std::move(message));
  }

  virtual void decrementEnqueuedCount();

  virtual void onRequestError(Transport* transport);

  void addPendingTransport(ProxygenTransport& transport) {
    if (partialPostEchoEnabled()) {
      const auto status = getStatus();
      transport.setShouldRepost(status == RunStatus::STOPPING
                                || status == RunStatus::STOPPED);
    }
    m_pendingTransports.push_back(transport);
  }

 private:
  class ProxygenEventBaseObserver : public folly::EventBaseObserver {
   public:
     using ClockT = std::chrono::steady_clock;

     explicit ProxygenEventBaseObserver(uint32_t loop_sample_rate);

     ~ProxygenEventBaseObserver() = default;

     uint32_t getSampleRate() const override {
       return m_sample_rate_;
     };

     void loopSample(int64_t busytime /* usec */, int64_t idletime) override;

   private:
     const uint32_t m_sample_rate_;
     folly::SlidingWindowQuantileEstimator<ClockT> m_busytime_estimator;
     folly::SlidingWindowQuantileEstimator<ClockT> m_idletime_estimator;

     ServiceData::ExportedTimeSeries* m_evbLoopCountTimeSeries;
     ServiceData::CounterCallback m_counterCallback;
  };

 protected:
  enum RequestPriority {
    PRIORITY_NORMAL = 0,
    PRIORITY_HIGH,
    kNumPriorities
  };
  RequestPriority getRequestPriority(const char *uri);

  // Ordering of shutdown states corresponds to the phases in time,
  // and we rely on the ordering.  State transition graph is linear
  // here.
  enum class ShutdownState {
    SHUTDOWN_NONE,
    DRAINING_READS,
    STOPPING_VM,
    DRAINING_WRITES
  };

  void timeoutExpired() noexcept override;

  bool drained() const {
    return (m_https ? m_drainCount > 1 : m_drainCount > 0);
  }

  // These functions can only be called from the m_worker thread
  void stopListening(bool hard = false);

  virtual bool partialPostEchoEnabled() { return false; }

  void returnPartialPosts();

  void abortPendingTransports();

  void doShutdown();

  void stopVM();

  void vmStopped();

  void forceStop();

  void reportShutdownStatus();

  void resetSSLContextConfigs(
    const std::vector<CertKeyPair>& paths);

  wangle::SSLContextConfig createContextConfig(
    const CertKeyPair& path,
    bool isDefault=false);

  void updateTLSTicketSeeds(wangle::TLSTicketKeySeeds seeds);

  virtual std::unique_ptr<HPHPSessionAcceptor> createAcceptor(
    const proxygen::AcceptorConfiguration& config);

  // Forbidden copy constructor and assignment operator
  ProxygenServer(ProxygenServer const &) = delete;
  ProxygenServer& operator=(ProxygenServer const &) = delete;
  int m_accept_sock{-1};
  int m_accept_sock_ssl{-1};
  bool m_https{false};
  uint32_t m_drainCount{0};
  uint32_t m_enqueuedCount{0};
  ShutdownState m_shutdownState{ShutdownState::SHUTDOWN_NONE};
  std::unique_ptr<RequestHandler> m_handler;
  folly::AsyncServerSocket::UniquePtr m_httpServerSocket;
  folly::AsyncServerSocket::UniquePtr m_httpsServerSocket;
  folly::EventBaseManager m_eventBaseManager;
  HPHPWorkerThread m_worker;
  proxygen::AcceptorConfiguration m_httpConfig;
  proxygen::AcceptorConfiguration m_httpsConfig;
  std::unique_ptr<HPHPSessionAcceptor> m_httpAcceptor;
  std::unique_ptr<HPHPSessionAcceptor> m_httpsAcceptor;
  std::unique_ptr<wangle::FilePoller> m_filePoller;

  JobQueueDispatcher<ProxygenWorker> m_dispatcher;
  ResponseMessageQueue m_responseQueue;
  std::unique_ptr<TakeoverAgent> m_takeover_agent;
  ProxygenTransportList m_pendingTransports;
  std::unique_ptr<wangle::TLSCredProcessor> m_credProcessor;
};

struct ProxygenTransportTraits {
  ProxygenTransportTraits(std::shared_ptr<ProxygenJob> job,
    void *opaque, int id);
  ~ProxygenTransportTraits();

  Server *getServer() const {
    return server_;
  }

  Transport *getTransport() const {
    return transport_.get();
  }

 private:
  ProxygenServer *server_;
  std::shared_ptr<ProxygenTransport> transport_;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_HTTP_SERVER_PROXYGEN_SERVER_H_
