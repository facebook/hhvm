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

#include <runtime/base/server/libevent_server.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/memory/memory_manager.h>
#include <runtime/base/server/server_stats.h>
#include <runtime/base/server/http_protocol.h>

///////////////////////////////////////////////////////////////////////////////
// static handler

static void on_request(struct evhttp_request *request, void *obj) {
  ASSERT(obj);
  ((HPHP::LibEventServer*)obj)->onRequest(request);
}

static void on_response(int fd, short what, void *obj) {
  ASSERT(obj);
  ((HPHP::PendingResponseQueue*)obj)->process();
}

static void on_timer(int fd, short events, void *context) {
  event_base_loopbreak((struct event_base *)context);
}

static void on_thread_stop(int fd, short events, void *context) {
  event_base_loopbreak((struct event_base *)context);
}

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// LibEventJob

LibEventJob::LibEventJob(evhttp_request *req) : request(req) {
  if (RuntimeOption::EnableStats && RuntimeOption::EnableWebStats) {
    clock_gettime(CLOCK_MONOTONIC, &start);
  }
}

void LibEventJob::stopTimer() {
  if (RuntimeOption::EnableStats && RuntimeOption::EnableWebStats) {
    timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);
    time_t dsec = end.tv_sec - start.tv_sec;
    long dnsec = end.tv_nsec - start.tv_nsec;
    int64 dusec = dsec * 1000000 + dnsec / 1000;
    ServerStats::Log("page.wall.queuing", dusec);
  }
}

///////////////////////////////////////////////////////////////////////////////
// LibEventWorker

LibEventWorker::LibEventWorker() : m_handler(NULL) {
}

LibEventWorker::~LibEventWorker() {
}

void LibEventWorker::doJob(LibEventJobPtr job) {
  job->stopTimer();
  evhttp_request *request = job->request;
  ASSERT(m_opaque);
  LibEventServer *server = (LibEventServer*)m_opaque;

  if (m_handler == NULL) {
    m_handler = server->createRequestHandler();
    ASSERT(m_handler);
  }

  LibEventTransport transport(server, request, m_id);
  bool error = true;
  std::string errorMsg;
  try {
    std::string cmd = transport.getCommand();
    cmd = std::string("/") + cmd;
    if (server->shouldHandle(cmd)) {
      m_handler->handleRequest(&transport);
      error = false;
    } else {
      transport.sendString("Not Found", 404);
      return;
    }
  } catch (Exception &e) {
    if (Server::StackTraceOnError) {
      errorMsg = e.what();
    } else {
      errorMsg = e.getMessage();
    }
  } catch (std::exception &e) {
    errorMsg = e.what();
  } catch (...) {
    errorMsg = "(unknown exception)";
  }

  if (error) {
    if (RuntimeOption::ServerErrorMessage) {
      transport.sendString(errorMsg, 500);
    } else {
      transport.sendString(RuntimeOption::FatalErrorMessage, 500);
    }
  }
}

void LibEventWorker::onThreadEnter() {
  ASSERT(m_opaque);
  LibEventServer *server = (LibEventServer*)m_opaque;
  server->onThreadEnter();
}

void LibEventWorker::onThreadExit() {
  ASSERT(m_opaque);
  LibEventServer *server = (LibEventServer*)m_opaque;
  server->onThreadExit(m_handler);
  MemoryManager::TheMemoryManager().get()->cleanup();
}

///////////////////////////////////////////////////////////////////////////////
// constructor and destructor

LibEventServer::LibEventServer(const std::string &address, int port,
                               int thread, int timeoutSeconds)
  : Server(address, port, thread),
    m_accept_sock(-1),
    m_timeoutThreadData(thread, timeoutSeconds),
    m_timeoutThread(&m_timeoutThreadData, &TimeoutThread::run),
    m_dispatcher(thread, this),
    m_dispatcherThread(this, &LibEventServer::dispatch) {
  m_eventBase = event_base_new();
  m_server = evhttp_new(m_eventBase);
  evhttp_set_gencb(m_server, on_request, this);
  m_responseQueue.create(m_eventBase);
}

LibEventServer::~LibEventServer() {
  ASSERT (getStatus() == STOPPED || getStatus() == STOPPING ||
          getStatus() == NOT_YET_STARTED);
  // We can't free event base when server is still working on it.
  // This will cause a leak with event base, but normally this happens when
  // process exits, so we're probably fine.
  if (getStatus() != STOPPING) {
    event_base_free(m_eventBase);
  }
}

///////////////////////////////////////////////////////////////////////////////
// implementing HttpServer

int LibEventServer::getAcceptSocket() {
  m_accept_sock = evhttp_bind_socket(m_server, m_address.c_str(), m_port);
  return m_accept_sock;
}

void LibEventServer::start() {
  if (getStatus() == RUNNING) return;

  if (getAcceptSocket() != 0) {
    throw FailedToListenException(m_address, m_port);
  }

  setStatus(RUNNING);
  m_dispatcher.start();
  m_dispatcherThread.start();
  m_timeoutThread.start();
}

void LibEventServer::waitForEnd() {
  m_dispatcherThread.waitForEnd();

  m_timeoutThreadData.stop();
  m_timeoutThread.waitForEnd();
}

void LibEventServer::dispatchWithTimeout(int timeoutSeconds) {
  struct timeval timeout;
  timeout.tv_sec = timeoutSeconds;
  timeout.tv_usec = 0;

  event eventTimeout;
  event_set(&eventTimeout, -1, 0, on_timer, m_eventBase);
  event_base_set(m_eventBase, &eventTimeout);
  event_add(&eventTimeout, &timeout);

  event_base_loop(m_eventBase, EVLOOP_ONCE);

  event_del(&eventTimeout);
}

void LibEventServer::dispatch() {
  m_pipeStop.open();
  event_set(&m_eventStop, m_pipeStop.getOut(), EV_READ|EV_PERSIST,
            on_thread_stop, m_eventBase);
  event_base_set(m_eventBase, &m_eventStop);
  event_add(&m_eventStop, NULL);

  while (getStatus() != STOPPED) {
    event_base_loop(m_eventBase, EVLOOP_ONCE);
  }

  event_del(&m_eventStop);

  // flushing all responses
  if (!m_responseQueue.empty()) {
    m_responseQueue.process();
  }
  m_responseQueue.close();

  // flusing all remaining events
  if (RuntimeOption::ServerGracefulShutdownWait) {
    dispatchWithTimeout(RuntimeOption::ServerGracefulShutdownWait);
  }
}

void LibEventServer::stop() {
  Lock lock(m_mutex);
  if (getStatus() != RUNNING || m_server == NULL) return;

  // inform LibEventServer::onRequest() to stop queuing
  setStatus(STOPPING);

  // stop JobQueue processing
  m_dispatcher.stop();

  // stop event loop
  setStatus(STOPPED);
  if (write(m_pipeStop.getIn(), "", 1) < 0) {
    // an error occured but we're in shutdown already, so ignore
  }
  m_dispatcherThread.waitForEnd();
  evhttp_free(m_server);
  m_server = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// request/response handling

void LibEventServer::onThreadEnter() {
  m_timeoutThreadData.registerRequestThread
    (&ThreadInfo::s_threadInfo->m_reqInjectionData);
}

void LibEventServer::onRequest(struct evhttp_request *request) {
  if (RuntimeOption::EnableKeepAlive &&
      RuntimeOption::ConnectionTimeoutSeconds > 0) {
    // before processing request, set the connection timeout
    // it's just writing a variable in libevent
    evhttp_connection_set_timeout(request->evcon,
                                  RuntimeOption::ConnectionTimeoutSeconds);
  }
  if (getStatus() == RUNNING) {
    m_dispatcher.enqueue(LibEventJobPtr(new LibEventJob(request)));
  } else {
    Logger::Error("throwing away one new request while shutting down");
  }
}

void LibEventServer::onResponse(int worker, evhttp_request *request,
                                int code) {
  int nwritten = 0;
  if (RuntimeOption::LibEventSyncSend) {
    const char *reason = HttpProtocol::GetReasonString(code);
    nwritten = evhttp_send_reply_sync_begin(request, code, reason, NULL);
  }
  m_responseQueue.enqueue(worker, request, code, nwritten);
}

void LibEventServer::onChunkedResponse(int worker, evhttp_request *request,
                                       int code, evbuffer *chunk,
                                       bool firstChunk) {
  m_responseQueue.enqueue(worker, request, code, chunk, firstChunk);
}

void LibEventServer::onChunkedResponseEnd(int worker,
                                          evhttp_request *request) {
  m_responseQueue.enqueue(worker, request);
}

///////////////////////////////////////////////////////////////////////////////
// PendingResponseQueue

PendingResponseQueue::PendingResponseQueue() {
  ASSERT(RuntimeOption::ResponseQueueCount > 0);
  for (int i = 0; i < RuntimeOption::ResponseQueueCount; i++) {
    m_responseQueues.push_back(ResponseQueuePtr(new ResponseQueue()));
  }
}

bool PendingResponseQueue::empty() {
  for (int i = 0; i < RuntimeOption::ResponseQueueCount; i++) {
    ResponseQueue &q = *m_responseQueues[i];
    Lock lock(q.m_mutex);
    if (!q.m_responses.empty()) return false;
  }
  return true;
}

void PendingResponseQueue::create(event_base *eventBase) {
  if (!m_ready.open()) {
    throw FatalErrorException("unable to create pipe for ready signal");
  }
  event_set(&m_event, m_ready.getOut(), EV_READ|EV_PERSIST, on_response, this);
  event_base_set(eventBase, &m_event);
  event_add(&m_event, NULL);
}

void PendingResponseQueue::close() {
  event_del(&m_event);
}

void PendingResponseQueue::enqueue(int worker, ResponsePtr response) {
  {
    int i = worker % RuntimeOption::ResponseQueueCount;
    ResponseQueue &q = *m_responseQueues[i];
    Lock lock(q.m_mutex);
    q.m_responses.push_back(response);
  }

  // signal to call process()
  if (write(m_ready.getIn(), &response, 1) < 0) {
    // an error occured but nothing we can really do
  }
}

void PendingResponseQueue::enqueue(int worker, evhttp_request *request,
                                   int code, int nwritten) {
  ResponsePtr res(new Response());
  res->request = request;
  res->code = code;
  res->nwritten = nwritten;
  enqueue(worker, res);
}

void PendingResponseQueue::enqueue(int worker, evhttp_request *request,
                                   int code, evbuffer *chunk,
                                   bool firstChunk) {
  ResponsePtr res(new Response());
  res->request = request;
  res->code = code;
  res->chunked = true;
  res->chunk = chunk;
  res->firstChunk = firstChunk;
  enqueue(worker, res);
}

void PendingResponseQueue::enqueue(int worker, evhttp_request *request) {
  ResponsePtr res(new Response());
  res->request = request;
  res->chunked = true;
  enqueue(worker, res);
}

void PendingResponseQueue::process() {
  // clean up the pipe for next signals
  char buf[512];
  if (read(m_ready.getOut(), buf, sizeof(buf)) < 0) {
    // an error occured but nothing we can really do
  }

  // making a copy so we don't hold up the mutex very long
  ResponsePtrVec responses;
  for (int i = 0; i < RuntimeOption::ResponseQueueCount; i++) {
    ResponseQueue &q = *m_responseQueues[i];
    Lock lock(q.m_mutex);
    responses.insert(responses.end(),q.m_responses.begin(),q.m_responses.end());
    q.m_responses.clear();
  }

  for (unsigned int i = 0; i < responses.size(); i++) {
    Response &res = *responses[i];
    evhttp_request *request = res.request;
    int code = res.code;

    if (res.chunked) {
      if (res.chunk) {
        if (res.firstChunk) {
          const char *reason = HttpProtocol::GetReasonString(code);
          evhttp_send_reply_start(request, code, reason);
        }
        evhttp_send_reply_chunk(request, res.chunk);
      } else {
        evhttp_send_reply_end(request);
      }
    } else if (RuntimeOption::LibEventSyncSend) {
      evhttp_send_reply_sync_end(res.nwritten, request);
    } else {
      const char *reason = HttpProtocol::GetReasonString(code);
      evhttp_send_reply(request, code, reason, NULL);
    }
  }
}

PendingResponseQueue::Response::Response()
  : request(NULL), code(0), nwritten(0),
    chunked(false), firstChunk(false), chunk(NULL) {
}

PendingResponseQueue::Response::~Response() {
  if (chunk) {
    evbuffer_free(chunk);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
