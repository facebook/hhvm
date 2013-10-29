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

#ifndef incl_HPHP_HTTP_SERVER_SERVER_H_
#define incl_HPHP_HTTP_SERVER_SERVER_H_

#include "hphp/runtime/server/takeover-agent.h"
#include "hphp/runtime/server/transport.h"
#include "hphp/util/exception.h"
#include "hphp/util/lock.h"

#include <chrono>
#include <memory>

/**
 * (1) For people who want to quickly come up with an HTTP server handling
 *     their specific requests, we really want to minimize writing an HTTP
 *     server to something like this,
 *
 *     class MyRequestHandler : public RequestHandler {
 *       public:
 *         virtual void handleRequest(Transport *transport) {
 *           // ...
 *         }
 *     };
 *
 *     Then, run a server like this,
 *
 *       ServerPtr server = make_shared<LibEventServer>("127.0.0.1", 80, 20);
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
 *     class MyTransport : public Transport {
 *       // implements transport-related functions
 *     };
 *
 *     class MyServer : public Server {
 *       // implements how to start/stop a server
 *     };
 *
 * (3) LibEventServer is pre-implemented with evhttp, and it has one thread
 *     listening on a socket and dispatching jobs to multiple worker threads.
 *
 */

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(Server);
DECLARE_BOOST_TYPES(ServerFactory);

/**
 * Base class of an HTTP request handler. Defining minimal interface an
 * HTTP request handler needs to implement.
 *
 * Note that each request handler may be invoked multiple times for different
 * requests.
 */
class RequestHandler {
public:
  explicit RequestHandler(int timeout) : m_timeout(timeout) {}
  virtual ~RequestHandler() {}

  /**
   * Sub-class handles a request by implementing this function.
   */
  virtual void handleRequest(Transport *transport) = 0;
  /**
   * Sub-class handles a request by implementing this function. This is called
   * when the server determines this request should not be processed (ie. due to
   * timeout).
   */
  virtual void abortRequest(Transport *transport) = 0;
  int getDefaultTimeout() const { return m_timeout; }
private:
  int m_timeout;
};

typedef std::function<std::unique_ptr<RequestHandler>()> RequestHandlerFactory;
typedef std::function<bool(const std::string&)> URLChecker;

/**
 * Base class of an HTTP server. Defining minimal interface an HTTP server
 * needs to implement.
 */
class Server {
public:
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
  /**
   * Constructor.
   */
  Server(const std::string &address, int port, int threadCount);

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
   * Add or remove a TakeoverListener to this server.
   *
   * This is a no-op for servers that do not support socket takeover.
   */
  virtual void addTakeoverListener(TakeoverListener* listener) {}
  virtual void removeTakeoverListener(TakeoverListener* listener) {}

  /**
   * Add additional worker threads
   */
  virtual void addWorkers(int numWorkers) = 0;

  /**
   * Informational.
   */
  std::string getAddress() const { return m_address;}
  int getPort() const { return m_port;}
  int getThreadCount() const { return m_threadCount;}

  RunStatus getStatus() const {
    return m_status;
  }
  void setStatus(RunStatus status) {
    m_status = status;
  }

  /**
   * Destructor.
   */
  virtual ~Server() {}

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
   * How many threads are actively working on handling requests.
   */
  virtual int getActiveWorker() = 0;

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

protected:
  std::string m_address;
  int m_port;
  int m_threadCount;
  mutable Mutex m_mutex;
  RequestHandlerFactory m_handlerFactory;
  URLChecker m_urlChecker;

private:
  RunStatus m_status;
};

class ServerOptions {
public:
  ServerOptions(const std::string &address,
                uint16_t port,
                int numThreads)
    : m_address(address),
      m_port(port),
      m_numThreads(numThreads),
      m_serverFD(-1),
      m_sslFD(-1),
      m_takeoverFilename() {
  }

  std::string m_address;
  uint16_t m_port;
  int m_numThreads;
  int m_serverFD;
  int m_sslFD;
  std::string m_takeoverFilename;
};

/**
 * A ServerFactory knows how to create Server objects.
 */
class ServerFactory : private boost::noncopyable {
public:
  virtual ~ServerFactory() {}

  virtual ServerPtr createServer(const ServerOptions &options) = 0;

  ServerPtr createServer(const std::string &address,
                         uint16_t port,
                         int numThreads);
};

/**
 * A registry mapping server type names to ServerFactory objects.
 *
 * This allows new server types to be plugged in dynamically, without having to
 * hard code the list of all possible server types.
 */
class ServerFactoryRegistry : private boost::noncopyable {
public:
  ServerFactoryRegistry();

  static ServerFactoryRegistry *getInstance();

  static ServerPtr createServer(const std::string &type,
                                const std::string &address,
                                uint16_t port,
                                int numThreads);

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
class ServerException : public Exception {
public:
  ServerException(const char *fmt, ...) ATTRIBUTE_PRINTF(2,3);
};

class FailedToListenException : public ServerException {
public:
  FailedToListenException(const std::string &addr, int port)
    : ServerException("Failed to listen on %s:%d", addr.c_str(), port) {
  }
};

class InvalidUrlException : public ServerException {
public:
  explicit InvalidUrlException(const char *part)
    : ServerException("Invalid URL: %s", part) {
  }
};

class InvalidMethodException : public ServerException {
public:
  explicit InvalidMethodException(const char *msg)
    : ServerException("Invalid method: %s", msg) {
  }
};

class InvalidHeaderException : public ServerException {
public:
  InvalidHeaderException(const char *name, const char *value)
    : ServerException("Invalid header: %s: %s", name, value) {
  }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_HTTP_SERVER_SERVER_H_
