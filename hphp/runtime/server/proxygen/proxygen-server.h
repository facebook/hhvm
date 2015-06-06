/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/server/proxygen/proxygen-transport.h"
#include "hphp/runtime/server/job-queue-vm-stack.h"
#include "hphp/runtime/server/server-worker.h"
#include "hphp/runtime/server/server.h"
#include <proxygen/lib/http/session/HTTPSessionAcceptor.h>
#include <proxygen/lib/services/WorkerThread.h>
#include <proxygen/lib/ssl/SSLContextConfig.h>
#include <thrift/lib/cpp/async/TNotificationQueue.h>

#include <algorithm>
#include <folly/io/async/EventBaseManager.h>
#include <memory>


namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
class ProxygenJob : public ServerJob {
public:
  explicit ProxygenJob(std::shared_ptr<ProxygenTransport> transport);

  virtual void getRequestStart(struct timespec *reqStart);

  std::shared_ptr<ProxygenTransport> transport;
  struct timespec reqStart;
};

class ProxygenTransportTraits;
typedef ServerWorker<std::shared_ptr<ProxygenJob>,
  ProxygenTransportTraits> ProxygenWorker;

class ProxygenServer;

class HPHPSessionAcceptor : public proxygen::HTTPSessionAcceptor {
 public:
  explicit HPHPSessionAcceptor(
    const proxygen::AcceptorConfiguration& config,
    ProxygenServer *server);
  virtual ~HPHPSessionAcceptor() {}

  proxygen::HTTPTransaction::Handler* newHandler(
    proxygen::HTTPTransaction& txn,
    proxygen::HTTPMessage *msg) noexcept override;

  bool canAccept(const folly::SocketAddress&) override;

  void onConnectionsDrained() override;

  void onIngressError(const proxygen::HTTPSession&,
                      proxygen::ProxygenError error) override;

 private:
  ProxygenServer *m_server;
};

typedef apache::thrift::async::TNotificationQueue<ResponseMessage>
  ResponseMessageQueue;

class HPHPWorkerThread : public proxygen::WorkerThread {
 public:
  explicit HPHPWorkerThread(folly::EventBaseManager* ebm)
      : WorkerThread(ebm) {}
  virtual ~HPHPWorkerThread() {}
  virtual void setup() override;
  virtual void cleanup() override;
};

class ProxygenServer : public Server,
                       public ResponseMessageQueue::Consumer,
                       public apache::thrift::async::TAsyncTimeout,
                       public TakeoverAgent::Callback {
 public:
  explicit ProxygenServer(const ServerOptions& options);

  ~ProxygenServer() {
    Logger::Verbose("%p: destroying ProxygenServer", this);
    waitForEnd();
    Logger::Verbose("%p: ProxygenServer destroyed", this);
  }

  void addTakeoverListener(TakeoverListener* listener) override;
  void removeTakeoverListener(TakeoverListener* listener) override;
  virtual void addWorkers(int numWorkers) override {
    m_dispatcher.addWorkers(numWorkers);
  }
  virtual void start() override;
  virtual void waitForEnd() override;
  virtual void stop() override;
  virtual int getActiveWorker() override {
    return m_dispatcher.getActiveWorker();
  }
  virtual int getQueuedJobs() override {
    return m_dispatcher.getQueuedJobs();
  }
  virtual int getLibEventConnectionCount() override;
  virtual bool enableSSL(int port) override;

  folly::EventBase *getEventBase() {
    return m_eventBaseManager.getEventBase();
  }

  void messageAvailable(ResponseMessage&& message) override {
    message.m_transport->messageAvailable(std::move(message));
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

 protected:
  // TODO: Share with LibEventServer?
  enum RequestPriority {
    PRIORITY_NORMAL = 0,
    PRIORITY_HIGH,
    kNumPriorities
  };
  RequestPriority getRequestPriority(const char *uri);

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

  void doShutdown();

  void stopVM();

  void vmStopped();

  void forceStop();

  bool initialCertHandler(const std::string& server_name,
                          const std::string& key_file,
                          const std::string& cert_file,
                          bool duplicate);

  bool dynamicCertHandler(const std::string& server_name,
                          const std::string& key_file,
                          const std::string& cert_file);

  bool sniNoMatchHandler(const char *server_name);

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
  proxygen::SSLContextConfig m_sslCtxConfig;
  std::unique_ptr<HPHPSessionAcceptor> m_httpAcceptor;
  std::unique_ptr<HPHPSessionAcceptor> m_httpsAcceptor;

  JobQueueDispatcher<ProxygenWorker> m_dispatcher;
  ResponseMessageQueue m_responseQueue;
  std::unique_ptr<TakeoverAgent> m_takeover_agent;
};

class ProxygenTransportTraits {
 public:
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
