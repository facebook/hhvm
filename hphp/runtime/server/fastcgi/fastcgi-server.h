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

#ifndef incl_HPHP_RUNTIME_SERVER_FASTCGI_FASTCGI_SERVER_H_
#define incl_HPHP_RUNTIME_SERVER_FASTCGI_FASTCGI_SERVER_H_

#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/AsyncServerSocket.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/wangle/acceptor/Acceptor.h>
#include <memory>
#include <thrift/lib/cpp/async/TAsyncTransport.h>
#include <thrift/lib/cpp/async/TAsyncSSLSocket.h>

#include "hphp/runtime/server/fastcgi/fastcgi-session.h"
#include "hphp/runtime/server/fastcgi/fastcgi-transport.h"
#include "hphp/runtime/server/fastcgi/fastcgi-worker.h"
#include "hphp/runtime/server/fastcgi/socket-connection.h"
#include "hphp/runtime/server/server.h"
#include "hphp/util/job-queue.h"
#include "proxygen/lib/services/AcceptorConfiguration.h"
#include "proxygen/lib/services/WorkerThread.h"
#include "thrift/lib/cpp/async/TAsyncServerSocket.h"
#include "thrift/lib/cpp/async/TEventBaseManager.h"


namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

class FastCGIServer;

/*
 * FastCGIAcceptor accepts new connections from a listening socket, wrapping
 * each one in a FastCGIConnection.
 */
class FastCGIAcceptor : public ::folly::Acceptor {
public:
  explicit FastCGIAcceptor(
      const ::folly::ServerSocketConfig& config,
      FastCGIServer *server)
      : ::folly::Acceptor(config),
        m_server(server) {}
  virtual ~FastCGIAcceptor() {}

  bool canAccept(
    const folly::SocketAddress& address) override;
  void onNewConnection(
    folly::AsyncSocket::UniquePtr sock,
    const folly::SocketAddress* peerAddress,
    const std::string& nextProtocolName,
    const ::folly::TransportInfo& tinfo) override;
  void onConnectionsDrained() override;

private:
  FastCGIServer *m_server;

  static const int k_maxConns;
  static const int k_maxRequests;

  static const folly::SocketAddress s_unknownSocketAddress;
};


class FastCGITransport;

/*
 * FastCGIConnection represents a connection to a FastCGI client, usually a web
 * server such as apache or nginx. It owns a single FastCGISession, which in
 * turn is responsible for manging the many requests multiplexed through this
 * connection.
 */
class FastCGIConnection
  : public SocketConnection,
    public apache::thrift::async::TAsyncTransport::ReadCallback,
    public apache::thrift::async::TAsyncTransport::WriteCallback,
    public FastCGISession::Callback {
friend class FastCGITransport;
public:
  FastCGIConnection(
    FastCGIServer* server,
    folly::AsyncSocket::UniquePtr sock,
    const folly::SocketAddress& localAddr,
    const folly::SocketAddress& peerAddr);
  ~FastCGIConnection() override;

  void getReadBuffer(void** bufReturn, size_t* lenReturn) override;
  void readDataAvailable(size_t len) noexcept override;
  void readEOF() noexcept override;
  void readError(
    const apache::thrift::transport::TTransportException& ex)
    noexcept override;

  std::shared_ptr<ProtocolSessionHandler>
    newSessionHandler(int handler_id) override;
  void onSessionEgress(std::unique_ptr<folly::IOBuf> chain) override;
  void writeError(size_t bytes,
    const apache::thrift::transport::TTransportException& ex)
    noexcept override;
  void writeSuccess() noexcept override;
  void onSessionError() override;
  void onSessionClose() override;

  void setMaxConns(int max_conns);
  void setMaxRequests(int max_requests);

  folly::EventBase* getEventBase() {
    return m_eventBase;
  }

private:
  void handleRequest(int transport_id);
  bool hasReadDataAvailable();

  static const uint32_t k_minReadSize;
  static const uint32_t k_maxReadSize;

  std::unordered_map<int, std::shared_ptr<FastCGITransport>> m_transports;
  folly::EventBase* m_eventBase;
  FastCGIServer* m_server;
  FastCGISession m_session;
  folly::IOBufQueue m_readBuf;
  bool m_shutdown{false};
  uint32_t m_writeCount{0};
};

/*
 * FastCGIServer uses a FastCGIAcceptor to listen for new connections from
 * FastCGI clients. There are many different classes involved in serving
 * FastCGI requests; here's an overview of the ownership hierarchy:
 *
 * FastCGIServer
 *   FastCGIAcceptor
 *     FastCGIConnection (1 Acceptor owns many Connections)
 *       FastCGISession
 *         FastCGITransaction (1 Session owns many Transactions)
 *           FastCGITransport
 */
class FastCGIServer : public Server,
                      public apache::thrift::async::TAsyncTimeout {
public:
  FastCGIServer(const std::string &address,
                int port,
                int workers,
                bool useFileSocket);
  ~FastCGIServer() {
    if (!m_done) {
      waitForEnd();
    }
  }

  void addTakeoverListener(TakeoverListener* lisener) override;
  void removeTakeoverListener(TakeoverListener* lisener) override;
  void addWorkers(int numWorkers) override {
    m_dispatcher.addWorkers(numWorkers);
  }
  void start() override;
  void waitForEnd() override;
  void stop() override;
  int getActiveWorker() override {
    return m_dispatcher.getActiveWorker();
  }
  int getQueuedJobs() override {
    return m_dispatcher.getQueuedJobs();
  }
  int getLibEventConnectionCount() override;

  folly::EventBaseManager *getEventBaseManager() {
    return &m_eventBaseManager;
  }

  folly::EventBase *getEventBase() {
    return m_eventBaseManager.getEventBase();
  }

  bool enableSSL(int) override {
    return false;
  }

  bool canAccept();

  void onConnectionsDrained();

  void handleRequest(std::shared_ptr<FastCGITransport> transport);

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
  ::proxygen::WorkerThread m_worker;
  ::folly::ServerSocketConfig m_socketConfig;
  std::unique_ptr<FastCGIAcceptor> m_acceptor;
  JobQueueDispatcher<FastCGIWorker> m_dispatcher;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_HTTP_SERVER_FASTCGI_FASTCGI_SERVER_H_
