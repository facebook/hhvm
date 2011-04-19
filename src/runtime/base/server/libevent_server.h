/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef __HTTP_SERVER_LIB_EVENT_SERVER_H__
#define __HTTP_SERVER_LIB_EVENT_SERVER_H__

#include <runtime/base/server/server.h>
#include <runtime/base/server/libevent_transport.h>
#include <runtime/base/timeout_thread.h>
#include <util/job_queue.h>
#include <util/process.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Wrapping evhttp_request to keep track of queuing time: from onRequest() to
 * doJob().
 */
DECLARE_BOOST_TYPES(LibEventJob);
class LibEventJob {
public:
  LibEventJob(evhttp_request *req);

  const timespec &getStartTimer() const { return start;}
  void stopTimer();

  evhttp_request *request;

private:
  timespec start;
};

/**
 * Required class for JobQueueDispatcher, who will call this class's doJob()
 * with one HTTP request after another. All this class does is to delegate
 * the request to an HttpRequestHandler.
 */
class LibEventWorker : public JobQueueWorker<LibEventJobPtr, true> {
public:
  LibEventWorker();
  virtual ~LibEventWorker();

  /**
   * Request handler called by LibEventServer.
   */
  virtual void doJob(LibEventJobPtr job);

  /**
   * Called when thread enters and exits.
   */
  virtual void onThreadEnter();
  virtual void onThreadExit();

private:
  RequestHandler *m_handler;
};

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
  DECLARE_BOOST_TYPES(Response);
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

  DECLARE_BOOST_TYPES(ResponseQueue);
  class ResponseQueue {
  public:
    Mutex m_mutex;
    std::deque<ResponsePtr> m_responses;
  };

  // signal between worker thread and response processing thread
  event m_event;
  CPipe m_ready;
  ResponseQueuePtrVec m_responseQueues;

  void enqueue(int worker, ResponsePtr response);
};

/**
 * Implementing an evhttp based HTTP server with JobQueueDispatcher. This
 * server will have one dispather thread and multiple worker threads.
 */
class LibEventServer : public Server {
public:
  /**
   * Constructor and destructor.
   */
  LibEventServer(const std::string &address, int port, int thread,
                 int timeoutSeconds);
  ~LibEventServer();

  // implemting Server
  virtual void start();
  virtual void waitForEnd();
  virtual void stop();
  virtual int getActiveWorker() {
    return m_dispatcher.getActiveWorker();
  }
  virtual int getQueuedJobs() {
    return m_dispatcher.getQueuedJobs();
  }

  void onThreadEnter();

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
  virtual bool enableSSL(void *sslCTX, int port);

  // Whether the server may reset the request handler, e.g., the RPC server.
  virtual bool supportReset() { return false; }

protected:
  virtual int getAcceptSocket();
  virtual int getAcceptSocketSSL();

  int m_accept_sock;
  int m_accept_sock_ssl;
  event_base *m_eventBase;
  evhttp *m_server;

  evhttp *m_server_ssl;
  int m_port_ssl;

  // signal to stop the thread
  event m_eventStop;
  CPipe m_pipeStop;

  TimeoutThread m_timeoutThreadData;
  AsyncFunc<TimeoutThread> m_timeoutThread;

private:
  JobQueueDispatcher<LibEventJobPtr, LibEventWorker> m_dispatcher;
  AsyncFunc<LibEventServer> m_dispatcherThread;

  PendingResponseQueue m_responseQueue;

  // dispatcher thread runs this function
  void dispatch();

  void dispatchWithTimeout(int timeoutSeconds);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HTTP_SERVER_LIB_EVENT_SERVER_H__
