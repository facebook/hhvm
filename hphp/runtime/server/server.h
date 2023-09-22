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

#pragma once

#include <algorithm>
#include <chrono>
#include <functional>
#include <list>
#include <memory>
#include <string>

#include "hphp/runtime/server/cli-server.h"
#include "hphp/runtime/server/takeover-agent.h"
#include "hphp/runtime/server/transport.h"
#include "hphp/util/exception.h"
#include "hphp/util/health-monitor-types.h"
#include "hphp/util/lock.h"

/**
 * (1) For people who want to quickly come up with an HTTP server handling
 *     their specific requests, we really want to minimize writing an HTTP
 *     server to something like this,
 *
 *     struct MyRequestHandler : RequestHandler {
 *       virtual void handleRequest(Transport *transport) {
 *         // ...
 *       }
 *     };
 *
 *     Then, run a server like this,
 *
 *       auto server = std::make_shared<LibEventServer>("127.0.0.1", 80, 20);
 *       server->setRequestHandlerFactory<MyRequestHandler>();
 *       Server::InstallStopSignalHandlers(server);
 *       server->start();
 *
 *     This way, we can easily swap out an implementation like LibEventServer
 *     without any modifications to MyRequestHandler, if LibEventServer model
 *     doesn't perform well with the specific requests.
 *
 * (2) For people who are interested in implementing a high-performance HTTP
 *     server, derive a new class from Server just like LibEventServer
 *     does.
 *
 *     struct MyTransport : Transport {
 *       // implements transport-related functions
 *     };
 *
 *     struct MyServer : Server {
 *       // implements how to start/stop a server
 *     };
 *
 * (3) LibEventServer is pre-implemented with evhttp, and it has one thread
 *     listening on a socket and dispatching jobs to multiple worker threads.
 *
 */

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Server;
struct ServerFactory;
using ServerPtr = std::unique_ptr<Server>;
using ServerFactoryPtr = std::shared_ptr<ServerFactory>;

/**
 * Base class of an HTTP request handler. Defining minimal interface an
 * HTTP request handler needs to implement.
 *
 * Note that each request handler may be invoked multiple times for different
 * requests.
 */
struct RequestHandler {
  explicit RequestHandler() {}
  explicit RequestHandler(int timeout) : m_timeout(timeout) {}
  virtual ~RequestHandler() {}

  /**
   * Called before and after request-handling work.
   */
  virtual void setupRequest(Transport* /*transport*/) {}
  virtual void teardownRequest(Transport* /*transport*/) noexcept {}

  /**
   * Sub-class handles a request by implementing this function.
   */
  virtual void handleRequest(Transport* transport) = 0;

  /**
   * Sub-class handles a request by implementing this function. This is called
   * when the server determines this request should not be processed (e.g., due
   * to timeout).
   */
  virtual void abortRequest(Transport* transport) = 0;

  /**
   * Convenience wrapper around {setup,handle,teardown}Request().
   */
  void run(Transport* transport) {
    SCOPE_EXIT { teardownRequest(transport); };
    setupRequest(transport);
    handleRequest(transport);
  }

  /**
   * Write an entry to the handler's access log.
   */
  virtual void logToAccessLog(Transport* /*transport*/) {}

  virtual void setCliContext(CLIContext&& ctx) {}

  int getDefaultTimeout() const { return m_timeout; }

private:
  int m_timeout;
};

using RequestHandlerFactory = std::function<std::unique_ptr<RequestHandler>()>;
using URLChecker = std::function<bool(const std::string&)>;

/**
 * Base class of an HTTP server. Defining minimal interface an HTTP server
 * needs to implement.
 */
struct Server : IHostHealthObserver {
  enum class RunStatus {
    NOT_YET_STARTED = 0,
    RUNNING,
    STOPPING,
    STOPPED,
  };

  /**
   * Whether to turn on full stacktrace on internal server errors. Default is
   * true.
   */
  static bool StackTraceOnError;

  /**
   * ...so that we can grarefully stop these servers on signals.
   */
  static void InstallStopSignalHandlers(ServerPtr server);

public:
  struct ServerEventListener {
    virtual ~ServerEventListener() {}
    virtual void serverStopped(Server* /*server*/) {}
  };

  Server(const std::string &address, int port);

  /**
   * Set the RequestHandlerFactory that this server will use.
   * This must be called before start().
   */
  void setRequestHandlerFactory(RequestHandlerFactory f) {
    m_handlerFactory = f;
  }
  /**
   * Helper function to set the RequestHandlerFactory to a
   * GenericRequestHandlerFactory for the specified handler type.
   */
  template<class TRequestHandler>
  void setRequestHandlerFactory(int timeout) {
    setRequestHandlerFactory([timeout] {
      return std::unique_ptr<RequestHandler>(new TRequestHandler(timeout));
    });
  }

  /**
   * Set the URLChecker function which determines which paths this server is
   * allowed to server.
   *
   * Defaults to SatelliteServerInfo::checkURL()
   */
  void setUrlChecker(const URLChecker& checker) {
    m_urlChecker = checker;
  }

  /**
   * Add or remove a ServerEventListener.
   */
  void addServerEventListener(ServerEventListener* listener) {
    m_listeners.push_back(listener);
  }
  void removeServerEventListener(ServerEventListener* listener) {
    auto it = std::find(m_listeners.begin(), m_listeners.end(), listener);
    if (it != m_listeners.end()) {
      m_listeners.erase(it);
    }
  }

  /**
   * Add or remove a TakeoverListener to this server.
   *
   * This is a no-op for servers that do not support socket takeover.
   */
  virtual void addTakeoverListener(TakeoverListener* /*listener*/) {}
  virtual void removeTakeoverListener(TakeoverListener* /*listener*/) {}

  /**
   * Add additional worker threads
   */
  virtual void saturateWorkers() = 0;

  /**
   * Informational.
   */
  std::string getAddress() const { return m_address;}
  int getPort() const { return m_port;}

  RunStatus getStatus() const {
    return m_status.load(std::memory_order_acquire);
  }
  void setStatus(RunStatus status) {
    m_status.store(status, std::memory_order_release);
  }

  /**
   * IHostHealthObserver interface.  Note that m_status doesn't
   * contain server health information.
   */
  void notifyNewStatus(HealthLevel newLevel) override {
    m_healthLevel = newLevel;
  }
  HealthLevel getHealthLevel() override {
    return m_healthLevel;
  }

  /**
   * Destructor.
   */
  ~Server() override {}

  /**
   * Start this web server. Note this is a non-blocking call.
   */
  virtual void start() = 0;

  /**
   * Block until web server is stopped.
   */
  virtual void waitForEnd() = 0;

  /**
   * Gracefully stop this web server. We will stop accepting new connections
   * and finish ongoing requests without being interrupted in the middle of
   * them. Note this is a non-blocking call and it will return immediately.
   * At background, it will eventually make the thread calling start() quit.
   */
  virtual void stop() = 0;

  /**
   * How many threads can be available for handling requests.
   */
  virtual size_t getMaxThreadCount() = 0;

  /**
   * How many threads are actively working on handling requests.
   */
  virtual int getActiveWorker() = 0;

  /**
   * Update the maximum number of threads allowed to handle requests at a
   * time.
   */
  virtual void updateMaxActiveWorkers(int) = 0;

  /**
   * How many jobs are queued waiting to be handled.
   */
  virtual int getQueuedJobs() = 0;

  virtual int getLibEventConnectionCount() = 0;

  /**
   * Create a new RequestHandler.
   */
  std::unique_ptr<RequestHandler> createRequestHandler() {
    return m_handlerFactory();
  }

  /**
   * Check whether a request to the specified server path is allowed.
   */
  bool shouldHandle(const std::string &path) {
    return m_urlChecker(path);
  }

  /**
   * To enable SSL of the current server, it will listen to an additional
   * port as specified in parameter.
   */
  virtual bool enableSSL(int port) = 0;

  /**
   * To enable SSL in addition to plaintext of the current server.
   */
  virtual bool enableSSLWithPlainText() {
    return false;
  }

protected:
  std::string m_address;
  int m_port;
  mutable Mutex m_mutex;
  RequestHandlerFactory m_handlerFactory;
  URLChecker m_urlChecker;
  std::list<ServerEventListener*> m_listeners;

private:
  std::atomic<RunStatus> m_status{RunStatus::NOT_YET_STARTED};
  HealthLevel m_healthLevel{HealthLevel::Bold};
};

struct ServerOptions {
  ServerOptions(const std::string &address,
                uint16_t port,
                int maxThreads,
                int initThreads = -1,
                int maxQueue = -1,
                bool legacyBehavior = true)
    : m_address(address),
      m_port(port),
      m_maxThreads(maxThreads),
      m_initThreads(initThreads),
      m_maxQueue(maxQueue == -1 ? maxThreads : maxQueue),
      m_legacyBehavior(legacyBehavior) {
    assertx(m_maxThreads >= 0);
    if (m_initThreads < 0 || m_initThreads > m_maxThreads) {
      m_initThreads = m_maxThreads;
    }
  }

  std::string m_address;
  uint16_t m_port;
  int m_maxThreads;
  int m_initThreads;
  int m_maxQueue;
  bool m_legacyBehavior;
  int m_serverFD{-1};
  int m_sslFD{-1};
  std::string m_takeoverFilename;
  bool m_useFileSocket{false};
  int m_hugeThreads{0};
  unsigned m_hugeStackKb{0};
  unsigned m_extraKb{0};
  uint32_t m_loop_sample_rate{0};
};

/**
 * A ServerFactory knows how to create Server objects.
 */
struct ServerFactory {
  ServerFactory() {}
  virtual ~ServerFactory() {}

  ServerFactory(const ServerFactory&) = delete;
  ServerFactory& operator=(const ServerFactory&) = delete;

  virtual ServerPtr createServer(const ServerOptions &options) = 0;

  ServerPtr createServer(const std::string &address,
                         uint16_t port,
                         int maxThreads,
                         int initThreads = -1);
};

/**
 * A registry mapping server type names to ServerFactory objects.
 *
 * This allows new server types to be plugged in dynamically, without having to
 * hard code the list of all possible server types.
 */
struct ServerFactoryRegistry {
  ServerFactoryRegistry();

  ServerFactoryRegistry(const ServerFactoryRegistry&) = delete;
  ServerFactoryRegistry& operator=(const ServerFactoryRegistry&) = delete;

  static ServerFactoryRegistry *getInstance();

  static ServerPtr createServer(const std::string &type,
                                const std::string &address,
                                uint16_t port,
                                int maxThreads,
                                int initThreads = -1);

  void registerFactory(const std::string &name,
                       const ServerFactoryPtr &factory);

  ServerFactoryPtr getFactory(const std::string &name);

private:
  Mutex m_lock;
  std::map<std::string, ServerFactoryPtr> m_factories;
};

/**
 * All exceptions Server throws should derive from this base class.
 */
struct ServerException : Exception {
  ServerException(ATTRIBUTE_PRINTF_STRING const char *fmt, ...)
    ATTRIBUTE_PRINTF(2,3);
};

struct FailedToListenException : ServerException {
  explicit FailedToListenException(const std::string &addr)
    : ServerException("Failed to listen to unix socket at %s", addr.c_str()) {
  }
  FailedToListenException(const std::string &addr, int port)
    : ServerException("Failed to listen on %s:%d", addr.c_str(), port) {
  }
};

struct InvalidUrlException : ServerException {
  explicit InvalidUrlException(const char *part)
    : ServerException("Invalid URL: %s", part) {
  }
};

struct InvalidMethodException : ServerException {
  explicit InvalidMethodException(const char *msg)
    : ServerException("Invalid method: %s", msg) {
  }
};

struct InvalidHeaderException : ServerException {
  InvalidHeaderException(const char *name, const char *value)
    : ServerException("Invalid header: %s: %s", name, value) {
  }
};

///////////////////////////////////////////////////////////////////////////////
}

