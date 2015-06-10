/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/server/fastcgi/fastcgi-session.h"
#include "hphp/runtime/server/fastcgi/fastcgi-transport.h"
#include "hphp/runtime/server/fastcgi/fastcgi-worker.h"
#include "hphp/runtime/server/server.h"
#include "hphp/util/job-queue.h"

#include "proxygen/lib/services/WorkerThread.h"

#include <folly/io/async/AsyncServerSocket.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/wangle/acceptor/Acceptor.h>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

class FastCGIServer;

/*
 * FastCGIAcceptor accepts new connections from a listening socket, wrapping
 * each one in a FastCGISession.
 */
struct FastCGIAcceptor : public folly::Acceptor {
  FastCGIAcceptor(const folly::ServerSocketConfig& config,
                  FastCGIServer *server)
    : folly::Acceptor(config)
    , m_server(server)
  {}

  virtual ~FastCGIAcceptor() {}

  bool canAccept(const folly::SocketAddress& address) override;
  void onNewConnection(folly::AsyncSocket::UniquePtr sock,
                       const folly::SocketAddress* peerAddress,
                       const std::string& nextProtocolName,
                       const folly::TransportInfo& tinfo) override;
  void onConnectionsDrained() override;

private:
  FastCGIServer* m_server;
};

/*
 * FastCGIServer uses a FastCGIAcceptor to listen for new connections from
 * FastCGI webservers. There are several different classes involved in serving
 * FastCGI requests; here's an overview of the ownership hierarchy:
 *
 * FastCGIServer
 *   FastCGIAcceptor
 *     FastCGISession (1 Acceptor creates many Sessions; they destruct when
 *                     no further work is left for them)
 *       FastCGITransport (shared by FastCGISession and VM)
 *
 * The server owns a thread-pool which both runs VMs and serves FastCGI
 * connections to webservers. The connection sockets and FastCGI protocol level
 * communication is managed by a FastCGISession. The session initiates requests
 * which use transports to track state and share data with the VM.
 *
 * The transport is responsible for processing the parameter and POST data for
 * consumtion by the VM and preparing headers and standard output to be sent
 * back by the session; it manages HTTP specific protocol communication.
 *
 * The session and transport borrow the server's thread-pool, however, with the
 * exception of the acceptor, none of the other FastCGI classes communicate with
 * the server directly.
 */
struct FastCGIServer : public Server,
                       public folly::AsyncTimeout {
  FastCGIServer(const std::string &address,
                int port,
                int workers,
                bool useFileSocket);
  ~FastCGIServer() {
    waitForEnd();
  }

  // These are currently unimplemented (TODO(#4129))
  void addTakeoverListener(TakeoverListener* lisener) override {}
  void removeTakeoverListener(TakeoverListener* lisener) override {}

  // Increases the size of the thread-pool for dispatching requests
  void addWorkers(int numWorkers) override {
    m_dispatcher.addWorkers(numWorkers);
  }

  // Configures m_socket and starts accepting connections in the event base
  void start() override;

  // Waits for the worker thread to complete
  void waitForEnd() override;

  // Terminates the server gracefully
  void stop() override;

  // Query information about the worker pool
  JobQueueDispatcher<FastCGIWorker>& getDispatcher() { return m_dispatcher; }
  int getActiveWorker() override { return m_dispatcher.getActiveWorker(); }
  int getQueuedJobs()   override { return m_dispatcher.getQueuedJobs();   }

  // Query the event manager
  folly::EventBaseManager *getEventBaseManager() { return &m_eventBaseManager; }

  // Return the number of active connections to webservers
  int getLibEventConnectionCount() override {
    return m_acceptor ? m_acceptor->getNumConnections() : 0;
  }

  // Don't support SSL- the webserver has its own SSL connection ot the client
  // anyway, and presumably our connection with the server is local.
  bool enableSSL(int) override { return false; }

  // Callback for FastCGIAcceptor that terminates the server
  void onConnectionsDrained();

private:
  // Priority of requests to dispatcher thread; currently we only use normal
  // priority
  enum RequestPriority {
    PRIORITY_NORMAL = 0,
    PRIORITY_HIGH,
    k_numPriorities
  };

  // Called by timer to terminate server when waiting for connections to drain
  // if connections are stalled
  void timeoutExpired() noexcept override;

  // Stops the server by waiting on thread-pool to become idle and then stopping
  // the dispatcher
  void terminateServer();

  // Forbidden copy constructor and assignment operator
  FastCGIServer(FastCGIServer const &) = delete;
  FastCGIServer& operator=(FastCGIServer const &) = delete;

  // Server socket
  folly::AsyncServerSocket::UniquePtr m_socket;

  // Thread-pooling for FastCGI connections and vm jobs
  proxygen::WorkerThread  m_worker;
  folly::EventBaseManager m_eventBaseManager;
  JobQueueDispatcher<FastCGIWorker> m_dispatcher;

  // Configuration for accepting webserver connections
  folly::ServerSocketConfig m_socketConfig;
  std::unique_ptr<FastCGIAcceptor> m_acceptor;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_HTTP_SERVER_FASTCGI_FASTCGI_SERVER_H_
