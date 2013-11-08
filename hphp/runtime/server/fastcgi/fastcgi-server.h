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

#ifndef incl_HPHP_RUNTIME_SERVER_FASTCGI_FASTCGI_SERVER_H_
#define incl_HPHP_RUNTIME_SERVER_FASTCGI_FASTCGI_SERVER_H_

#include "hphp/runtime/server/fastcgi/socket-connection.h"
#include "hphp/runtime/server/fastcgi/fastcgi-session.h"
#include "hphp/runtime/server/fastcgi/fastcgi-transport.h"
#include "hphp/runtime/server/fastcgi/fastcgi-worker.h"
#include "folly/io/IOBuf.h"
#include "folly/io/IOBufQueue.h"
#include "thrift/lib/cpp/async/TEventBaseManager.h"
#include "thrift/lib/cpp/async/TAsyncTransport.h"
#include "ti/proxygen/lib/workers/WorkerThread.h"
#include "ti/proxygen/lib/services/Acceptor.h"
#include "hphp/runtime/server/server.h"
#include "hphp/util/job-queue.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

class FastCGIServer;

class FastCGIAcceptor : public facebook::proxygen::Acceptor {
public:
  explicit FastCGIAcceptor(
      const facebook::proxygen::AcceptorConfiguration& config,
      FastCGIServer *server)
      : facebook::proxygen::Acceptor(config),
        m_server(server) {}
  virtual ~FastCGIAcceptor() {}

  virtual bool canAccept(
    const apache::thrift::transport::TSocketAddress& address) override;
  virtual void onNewConnection(
    apache::thrift::async::TAsyncSocket::UniquePtr sock,
    const apache::thrift::transport::TSocketAddress* peerAddress,
    const std::string& nextProtocolName,
    const facebook::proxygen::TransportInfo& tinfo) override;
  virtual void onConnectionsDrained() override;

private:
  FastCGIServer *m_server;

  static const int k_maxConns;
  static const int k_maxRequests;

  static const apache::thrift::transport::TSocketAddress s_unknownSocketAddress;
};


class FastCGITransport;

class FastCGIConnection
  : public SocketConnection,
    public apache::thrift::async::TAsyncTransport::ReadCallback,
    public ProtocolSession::Callback {
friend class FastCGITransport;
public:
  FastCGIConnection(
    FastCGIServer* server,
    apache::thrift::async::TAsyncTransport::UniquePtr sock,
    const apache::thrift::transport::TSocketAddress& localAddr,
    const apache::thrift::transport::TSocketAddress& peerAddr);
  virtual ~FastCGIConnection();

  virtual void getReadBuffer(void** bufReturn, size_t* lenReturn) override;
  virtual void readDataAvailable(size_t len) noexcept override;
  virtual void readEOF() noexcept override;
  virtual void readError(
    const apache::thrift::transport::TTransportException& ex)
    noexcept override;

  virtual ProtocolSessionHandlerPtr newSessionHandler(int handler_id) override;
  virtual void onSessionEgress(std::unique_ptr<folly::IOBuf> chain) override;
  virtual void onSessionError() override;
  virtual void onSessionClose() override;

  void setMaxConns(int max_conns);
  void setMaxRequests(int max_requests);

  apache::thrift::async::TEventBase* getEventBase() {
    return m_eventBase;
  }

private:
  void handleRequest(int transport_id);
  bool hasReadDataAvailable();

  static const uint32_t k_minReadSize;
  static const uint32_t k_maxReadSize;

  std::map<int, FastCGITransportPtr> m_transports;
  apache::thrift::async::TEventBase* m_eventBase;
  FastCGIServer* m_server;
  FastCGISession m_session;
  folly::IOBufQueue m_readBuf;
};


class FastCGIServer : public Server,
                      public apache::thrift::async::TAsyncTimeout {
public:
  FastCGIServer(const std::string &address,
                int port,
                int workers);
  ~FastCGIServer() {
    if (!m_done) {
      waitForEnd();
    }
  }

  virtual void addTakeoverListener(TakeoverListener* lisener) override;
  virtual void removeTakeoverListener(TakeoverListener* lisener) override;
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

  apache::thrift::async::TEventBaseManager *getEventBaseManager() {
    return &m_eventBaseManager;
  }

  apache::thrift::async::TEventBase *getEventBase() {
    return m_eventBaseManager.getEventBase();
  }

  virtual bool enableSSL(int) override {
    return false;
  }

  bool canAccept();

  void onConnectionsDrained();

  void handleRequest(FastCGITransportPtr transport);

private:
  enum RequestPriority {
    PRIORITY_NORMAL = 0,
    PRIORITY_HIGH,
    k_numPriorities
  };

  void timeoutExpired() noexcept;

  void terminateServer();

  // Forbidden copy constructor and assignment operator
  FastCGIServer(FastCGIServer const &) = delete;
  FastCGIServer& operator=(FastCGIServer const &) = delete;

  apache::thrift::async::TAsyncServerSocket::UniquePtr m_socket;
  apache::thrift::async::TEventBaseManager m_eventBaseManager;
  bool m_done{true};
  facebook::proxygen::WorkerThread m_worker;
  facebook::proxygen::AcceptorConfiguration m_socketConfig;
  std::unique_ptr<FastCGIAcceptor> m_acceptor;
  JobQueueDispatcher<FastCGIWorker> m_dispatcher;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_HTTP_SERVER_FASTCGI_FASTCGI_SERVER_H_

