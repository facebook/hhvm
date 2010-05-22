/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __HTTP_SERVER_SERVER_H__
#define __HTTP_SERVER_SERVER_H__

#include <runtime/base/server/transport.h>
#include <util/exception.h>
#include <util/lock.h>

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
 *       ServerPtr server(new TypedServer<LibEventServer, MyRequestHandler>
 *          ("127.0.0.1", 80, 20));
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

/**
 * Base class of an HTTP request handler. Defining minimal interface an
 * HTTP request handler needs to implement.
 *
 * Note that each request handler may be invoked multiple times for different
 * requests.
 */
class RequestHandler {
public:
  virtual ~RequestHandler() {}

  /**
   * Sub-class handles a request by implementing this function.
   */
  virtual void handleRequest(Transport *transport) = 0;
};

/**
 * Base class of an HTTP server. Defining minimal interface an HTTP server
 * needs to implement.
 */
class Server {
public:
  enum RunStatus {
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
   * This is for TypedServer to specialize a worker class to use.
   */
  virtual RequestHandler *createRequestHandler() = 0;
  virtual void releaseRequestHandler(RequestHandler *handler) = 0;
  virtual void onThreadExit(RequestHandler *handler) = 0;

  /**
   * Overwrite for URL blocking.
   */
  virtual bool shouldHandle(const std::string &cmd);

protected:
  std::string m_address;
  int m_port;
  int m_threadCount;
  mutable Mutex m_mutex;

private:
  RunStatus m_status;
};

/**
 * Binding different types together to form a concrete HTTP server that we
 * can run. By conforming to their owns interfaces, RequestHandler and
 * Server classes should be able to get mixed up and continue to work.
 */
template<class TServer, class TRequestHandler>
class TypedServer : public TServer {
public:
  TypedServer(const std::string &address, int port, int threadCount,
              int timeoutSeconds)
    : TServer(address, port, threadCount, timeoutSeconds) {
  }

  virtual RequestHandler *createRequestHandler() {
    return new TRequestHandler();
  }

  virtual void releaseRequestHandler(RequestHandler *handler) {
    delete handler;
  }

  virtual void onThreadExit(RequestHandler *handler) {
    delete handler;
  }
};

/**
 * All exceptions Server throws should derive from this base class.
 */
class ServerException : public Exception {
public:
  ServerException(const char *fmt, ...) {
    va_list ap; va_start(ap, fmt); format(fmt, ap); va_end(ap);
  }
};

class FailedToListenException : public ServerException {
public:
  FailedToListenException(const std::string &addr, int port)
    : ServerException("Failed to listen on %s:%d", addr.c_str(), port) {
  }
};

class InvalidUrlException : public ServerException {
public:
  InvalidUrlException(const char *part)
    : ServerException("Invalid URL: %s", part) {
  }
};

class InvalidMethodException : public ServerException {
public:
  InvalidMethodException(const char *msg)
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

#endif // __HTTP_SERVER_SERVER_H__
