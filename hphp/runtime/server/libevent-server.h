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

#ifndef incl_HPHP_HTTP_SERVER_LIB_EVENT_SERVER_H_
#define incl_HPHP_HTTP_SERVER_LIB_EVENT_SERVER_H_

#include "hphp/runtime/server/server.h"
#include "hphp/runtime/server/libevent-transport.h"
#include "hphp/runtime/server/job-queue-vm-stack.h"
//#include "hphp/util/job-queue.h"
//#include "hphp/util/service-data.h"
#include "hphp/runtime/server/takeover-agent.h"
#include "hphp/runtime/server/server-worker.h"
#include "hphp/util/process.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Wrapping evhttp_request to keep track of queuing time: from onRequest() to
 * doJob().
 */
class LibEventJob : public ServerJob {
public:
  explicit LibEventJob(evhttp_request *req) : request(req) {}

  void getRequestStart(struct timespec *reqStart);

  evhttp_request *request;
};

class LibEventTransportTraits;
typedef ServerWorker<std::shared_ptr<LibEventJob>,
                     LibEventTransportTraits> LibEventWorker;

/**
 * Helper class for queuing up response sending back to event loop.
 */
class PendingResponseQueue {
public:
  PendingResponseQueue();

  bool empty();
  void create(event_base *eventBase);
  void enqueue(int worker, evhttp_request *request, int code, int nwritten);
  void enqueue(int worker, evhttp_request *request, int code, evbuffer *chunk,
               bool firstChunk);
  void enqueue(int worker, evhttp_request *request); // chunked encoding ended
  void process();
  void close();

private:
  class Response {
  public:
    Response();
    ~Response();

    evhttp_request *request;
    int code;
    int nwritten;

    bool chunked;
    bool firstChunk;
    evbuffer *chunk;
  };

  class ResponseQueue {
  public:
    Mutex m_mutex;
    std::deque<std::shared_ptr<Response>> m_responses;
  };

  // signal between worker thread and response processing thread
  event m_event;
  CPipe m_ready;
  std::vector<std::shared_ptr<ResponseQueue>> m_responseQueues;

  void enqueue(int worker, std::shared_ptr<Response> response);
};

/**
 * Implementing an evhttp based HTTP server with JobQueueDispatcher. This
 * server will have one dispather thread and multiple worker threads.
 */
class LibEventServer : public Server, public TakeoverAgent::Callback {
public:
  /**
   * Constructor and destructor.
   */
  LibEventServer(const std::string &address, int port, int thread)
      : LibEventServer(ServerOptions(address, port, thread)) {}
  explicit LibEventServer(const ServerOptions &options);
  ~LibEventServer();

  // implementing Server
  void start() override;
  void waitForEnd() override;
  void stop() override;
  int getActiveWorker() override {
    return m_dispatcher.getActiveWorker();
  }
  void addWorkers(int numWorkers) override {
    m_dispatcher.addWorkers(numWorkers);
  }
  int getQueuedJobs() override {
    return m_dispatcher.getQueuedJobs();
  }
  int getLibEventConnectionCount() override;

  void addTakeoverListener(TakeoverListener* listener) override;
  void removeTakeoverListener(TakeoverListener* listener) override;

  /**
   * Request handler called by evhttp library.
   */
  void onRequest(evhttp_request *request);
  void onChunkedRead();

  /**
   * Called by LibEventTransport when a response is fully prepared.
   */
  void onResponse(int worker, evhttp_request *request, int code,
                  LibEventTransport* transport);
  void onChunkedResponse(int worker, evhttp_request *request, int code,
                         evbuffer *chunk, bool firstChunk);
  void onChunkedResponseEnd(int worker, evhttp_request *request);
  void onChunkedRequest(evhttp_request *request);

  /**
   * To enable SSL of the current server, it will listen to an additional
   * port as specified in parameter.
   */
  bool enableSSL(int port) override;

  /**
   * TakeoverAgent::Callback
   */
  int onTakeoverRequest(TakeoverAgent::RequestType type) override;

  void takeoverAborted() override;

protected:
  virtual int getAcceptSocket();
  virtual int getAcceptSocketSSL();

protected:
  int m_accept_sock;
  int m_accept_sock_ssl;
  event_base *m_eventBase;
  evhttp *m_server;

  evhttp *m_server_ssl;
  int m_port_ssl;

  std::unique_ptr<TakeoverAgent> m_takeover_agent;

  // signal to stop the thread
  event m_eventStop;
  CPipe m_pipeStop;

private:
  enum RequestPriority {
    PRIORITY_NORMAL = 0,
    PRIORITY_HIGH,
    kNumPriorities
  };
  RequestPriority getRequestPriority(struct evhttp_request* request);

  int useExistingFd(evhttp *server, int fd, bool listen);

  static bool certHandler(const std::string &server_name,
                          const std::string& key_file,
                          const std::string& cert_file,
                          bool duplicate);

  JobQueueDispatcher<LibEventWorker> m_dispatcher;
  AsyncFunc<LibEventServer> m_dispatcherThread;

  PendingResponseQueue m_responseQueue;

  // dispatcher thread runs this function
  void dispatch();

  void dispatchWithTimeout(int timeoutSeconds);
};

class LibEventTransportTraits {
 public:

  LibEventTransportTraits(
    std::shared_ptr<LibEventJob> job, void *opaque, int id);

  Server *getServer() {
    return server_;
  }

  Transport *getTransport() {
    return &transport_;
  }

 private:
  LibEventServer *server_;
  evhttp_request *request_;
  LibEventTransport transport_;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_HTTP_SERVER_LIB_EVENT_SERVER_H_
