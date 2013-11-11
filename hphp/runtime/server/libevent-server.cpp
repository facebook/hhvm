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

#include "hphp/runtime/server/libevent-server.h"

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/crash-reporter.h"
#include "hphp/runtime/base/url.h"
#include "hphp/runtime/server/http-protocol.h"
#include "hphp/runtime/server/server-name-indication.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/util/compatibility.h"
#include "hphp/util/logger.h"
#include "hphp/util/timer.h"

///////////////////////////////////////////////////////////////////////////////
// static handler

static void on_request(struct evhttp_request *request, void *obj) {
  assert(obj);
  ((HPHP::LibEventServer*)obj)->onRequest(request);
}

static void on_response(int fd, short what, void *obj) {
  assert(obj);
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

void LibEventJob::getRequestStart(struct timespec *reqStart) {
#ifdef EVHTTP_CONNECTION_GET_START
  evhttp_connection_get_start(request->evcon, reqStart);
#endif
}

///////////////////////////////////////////////////////////////////////////////
// LibEventTransportTraits

LibEventTransportTraits::LibEventTransportTraits(LibEventJobPtr job,
                                                 void *opaque,
                                                 int id) :
    server_((LibEventServer*)opaque),
    request_(job->request),
    transport_(server_, request_, id) {

#ifdef _EVENT_USE_OPENSSL
  if (evhttp_is_connection_ssl(request_->evcon)) {
    transport_.setSSL();
  }
#endif
}

///////////////////////////////////////////////////////////////////////////////
// constructor and destructor

LibEventServer::LibEventServer(const ServerOptions &options)
  : Server(options.m_address, options.m_port, options.m_numThreads),
    m_accept_sock(options.m_serverFD),
    m_accept_sock_ssl(options.m_sslFD),
    m_dispatcher(options.m_numThreads, RuntimeOption::ServerThreadRoundRobin,
                 RuntimeOption::ServerThreadDropCacheTimeoutSeconds,
                 RuntimeOption::ServerThreadDropStack,
                 this, RuntimeOption::ServerThreadJobLIFOSwitchThreshold,
                 RuntimeOption::ServerThreadJobMaxQueuingMilliSeconds,
                 kNumPriorities, Util::num_numa_nodes()),
    m_dispatcherThread(this, &LibEventServer::dispatch) {
  m_eventBase = event_base_new();
  m_server = evhttp_new(m_eventBase);
  m_server_ssl = nullptr;
  evhttp_set_connection_limit(m_server, RuntimeOption::ServerConnectionLimit);
  evhttp_set_gencb(m_server, on_request, this);
#ifdef EVHTTP_PORTABLE_READ_LIMITING
  evhttp_set_read_limit(m_server, RuntimeOption::RequestBodyReadLimit);
#endif
  m_responseQueue.create(m_eventBase);

  if (!options.m_takeoverFilename.empty()) {
    m_takeover_agent.reset(new TakeoverAgent(options.m_takeoverFilename));
  }
}

LibEventServer::~LibEventServer() {
  assert(getStatus() == RunStatus::STOPPED ||
         getStatus() == RunStatus::STOPPING ||
         getStatus() == RunStatus::NOT_YET_STARTED);
  // We can't free event base when server is still working on it.
  // This will cause a leak with event base, but normally this happens when
  // process exits, so we're probably fine.
  if (getStatus() != RunStatus::STOPPING) {
    event_base_free(m_eventBase);
  }
}

///////////////////////////////////////////////////////////////////////////////
// implementing HttpServer

void LibEventServer::addTakeoverListener(TakeoverListener* listener) {
  if (m_takeover_agent) {
    m_takeover_agent->addTakeoverListener(listener);
  }
}

void LibEventServer::removeTakeoverListener(TakeoverListener* listener) {
  if (m_takeover_agent) {
    m_takeover_agent->removeTakeoverListener(listener);
  }
}

int LibEventServer::useExistingFd(evhttp *server, int fd, bool needListen) {
  Logger::Info("inheritfd: using inherited fd %d for server", fd);

  int ret = -1;
  if (needListen) {
    ret = listen(fd, RuntimeOption::ServerBacklog);
    if (ret != 0) {
      Logger::Error("inheritfd: listen() failed: %s",
                    folly::errnoStr(errno).c_str());
      return -1;
    }
  }

  ret = evhttp_accept_socket(server, fd);
  if (ret < 0) {
    Logger::Error("evhttp_accept_socket: %s",
                  folly::errnoStr(errno).c_str());
    int errno_save = errno;
    close(fd);
    errno = errno_save;
    return -1;
  }
  return 0;
}

int LibEventServer::getAcceptSocket() {
  if (m_accept_sock >= 0) {
    if (useExistingFd(m_server, m_accept_sock, true /* listen */) != 0) {
      m_accept_sock = -1;
      return -1;
    }
    return 0;
  }

  const char *address = m_address.empty() ? nullptr : m_address.c_str();
  int ret = evhttp_bind_socket_backlog_fd(m_server, address,
                                          m_port, RuntimeOption::ServerBacklog);
  if (ret < 0) {
    if (errno == EADDRINUSE && m_takeover_agent) {
      ret = m_takeover_agent->takeover();
      if (ret < 0) {
        return -1;
      }
      if (useExistingFd(m_server, ret, false /* no listen */) != 0) {
        return -1;
      }
    } else {
      Logger::Error("Fail to bind port %d", m_port);
      return -1;
    }
  }
  m_accept_sock = ret;
  if (m_takeover_agent) {
    m_takeover_agent->requestShutdown();
    m_takeover_agent->setupFdServer(m_eventBase, m_accept_sock, this);
  }
  return 0;
}

int LibEventServer::onTakeoverRequest(TakeoverAgent::RequestType type) {
  int ret = -1;
  if (type == TakeoverAgent::RequestType::LISTEN_SOCKET) {
    // TODO: This is broken-sauce.  We should continue serving
    // requests from this process until shutdown.

    // Make evhttp forget our copy of the accept socket so we don't accept any
    // more connections and drop them.  Keep the socket open until we get the
    // shutdown request so that we can still serve AFDT requests (if the new
    // server crashes or something).  The downside is that it will take the LB
    // longer to figure out that we are broken.
    ret = evhttp_del_accept_socket(m_server, m_accept_sock);
    if (ret < 0) {
      // This will fail if we get a second AFDT request, but the spurious
      // log message is not too harmful.
      Logger::Error("Unable to delete accept socket");
    }
  } else if (type == TakeoverAgent::RequestType::TERMINATE) {
    ret = close(m_accept_sock);
    if (ret < 0) {
      Logger::Error("Unable to close accept socket");
      return -1;
    }
    m_accept_sock = -1;

    // Close SSL server
    if (m_server_ssl) {
      assert(m_accept_sock_ssl > 0);
      ret = evhttp_del_accept_socket(m_server_ssl, m_accept_sock_ssl);
      if (ret < 0) {
        Logger::Error("Unable to delete accept socket for SSL in evhttp");
        return -1;
      }
      ret = close(m_accept_sock_ssl);
      if (ret < 0) {
        Logger::Error("Unable to close accept socket for SSL");
        return -1;
      }
    }

  }
  return 0;
}

void LibEventServer::takeoverAborted() {
  if (m_accept_sock >= 0) {
    close(m_accept_sock);
    m_accept_sock = -1;
  }
}

int LibEventServer::getLibEventConnectionCount() {
  return evhttp_get_connection_count(m_server);
}

void LibEventServer::start() {
  if (getStatus() == RunStatus::RUNNING) return;

  if (getAcceptSocket() != 0) {
    throw FailedToListenException(m_address, m_port);
  }

  if (m_server_ssl != nullptr) {
    if (getAcceptSocketSSL() != 0) {
      Logger::Error("Fail to listen on ssl port %d", m_port_ssl);
      throw FailedToListenException(m_address, m_port_ssl);
    }
    Logger::Info("Listen on ssl port %d",m_port_ssl);
  }

  setStatus(RunStatus::RUNNING);
  m_dispatcher.start();
  m_dispatcherThread.start();
}

void LibEventServer::waitForEnd() {
  m_dispatcherThread.waitForEnd();
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
  event_add(&m_eventStop, nullptr);

  while (getStatus() != RunStatus::STOPPED) {
    event_base_loop(m_eventBase, EVLOOP_ONCE);
  }

  event_del(&m_eventStop);

  // flushing all responses
  if (!m_responseQueue.empty()) {
    m_responseQueue.process();
  }
  m_responseQueue.close();

  if (m_takeover_agent) {
    m_takeover_agent->stop();
  }

  // flushing all remaining events
  if (RuntimeOption::ServerGracefulShutdownWait) {
    dispatchWithTimeout(RuntimeOption::ServerGracefulShutdownWait);
  }
}

void LibEventServer::stop() {
  Lock lock(m_mutex);
  if (getStatus() != RunStatus::RUNNING || m_server == nullptr) return;

#define SHUT_FBLISTEN 3
  /*
   * Modifications to the Linux kernel to support shutting down a listen
   * socket for new connections only, but anything which has completed
   * the TCP handshake will still be accepted.  This allows for un-accepted
   * connections to be queued and then wait until all queued requests are
   * actively being processed.
   */
  if (RuntimeOption::ServerShutdownListenWait > 0 &&
      m_accept_sock != -1 && shutdown(m_accept_sock, SHUT_FBLISTEN) == 0) {
    int noWorkCount = 0;
    for (int i = 0; i < RuntimeOption::ServerShutdownListenWait; i++) {
      // Give the acceptor thread time to clean out all requests
      Logger::Info(
          "LibEventServer stopping port %d: [%d/%d] a/q/e %d/%d/%d",
          m_port, i, RuntimeOption::ServerShutdownListenWait,
          getActiveWorker(), getQueuedJobs(), getLibEventConnectionCount());
      sleep(1);

      // If we're not doing anything, break out quickly
      noWorkCount += (getQueuedJobs() == 0 && getActiveWorker() == 0);
      if (RuntimeOption::ServerShutdownListenNoWork > 0 &&
          noWorkCount >= RuntimeOption::ServerShutdownListenNoWork)
        break;
      if (getLibEventConnectionCount() == 0 &&
          getQueuedJobs() == 0 && getActiveWorker() == 0)
        break;
    }
    Logger::Info("LibEventServer stopped port %d: a/q/e %d/%d/%d",
        m_port, getActiveWorker(), getQueuedJobs(),
        getLibEventConnectionCount());
  }

  // inform LibEventServer::onRequest() to stop queuing
  setStatus(RunStatus::STOPPING);

  // stop JobQueue processing
  m_dispatcher.stop();

  // stop event loop
  setStatus(RunStatus::STOPPED);
  if (write(m_pipeStop.getIn(), "", 1) < 0) {
    // an error occured but we're in shutdown already, so ignore
  }
  m_dispatcherThread.waitForEnd();

  evhttp_free(m_server);
  m_server = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
// SSL handling

bool LibEventServer::certHandler(const std::string &server_name,
                                 const std::string &key_file,
                                 const std::string &crt_file) {
#ifdef _EVENT_USE_OPENSSL
  // Create an SSL_CTX for this cert pair.
  struct ssl_config tmp_config;
  tmp_config.cert_file = (char *)(crt_file.c_str());
  tmp_config.pk_file = (char *)(key_file.c_str());
  SSL_CTX *tmp_ctx = (SSL_CTX*)evhttp_init_openssl(&tmp_config);
  if (tmp_ctx) {
    ServerNameIndication::insertSNICtx(server_name, tmp_ctx);
    return true;
  }
#endif
  return false;
}

bool LibEventServer::enableSSL(int port) {
#ifdef _EVENT_USE_OPENSSL
  SSL_CTX *sslCTX = nullptr;
  struct ssl_config config;
  if (RuntimeOption::SSLCertificateFile != "" &&
      RuntimeOption::SSLCertificateKeyFile != "") {
    config.cert_file = (char*)RuntimeOption::SSLCertificateFile.c_str();
    config.pk_file = (char*)RuntimeOption::SSLCertificateKeyFile.c_str();
    sslCTX = (SSL_CTX *)evhttp_init_openssl(&config);
    if (sslCTX && !RuntimeOption::SSLCertificateDir.empty()) {
      ServerNameIndication::load(RuntimeOption::SSLCertificateDir,
                                 LibEventServer::certHandler);

      // Register our per-request server name indication callback.
      // We register our callback even if there's no additional certs so that
      // a cert added in the future will get picked up without a restart.
      SSL_CTX_set_tlsext_servername_callback(
        sslCTX,
        ServerNameIndication::callback);
    }
  } else {
    Logger::Error("Invalid certificate file or key file");
  }

  m_server_ssl = evhttp_new_openssl_ctx(m_eventBase, sslCTX);
  if (m_server_ssl == nullptr) {
    Logger::Error("evhttp_new_openssl_ctx failed");
    return false;
  }
  m_port_ssl = port;
  evhttp_set_connection_limit(m_server_ssl,
                              RuntimeOption::ServerConnectionLimit);
  evhttp_set_gencb(m_server_ssl, on_request, this);
  return true;
#else
  Logger::Error("A SSL enabled libevent is required");
  return false;
#endif
}

int LibEventServer::getAcceptSocketSSL() {
  if (m_accept_sock_ssl >= 0) {
    if (useExistingFd(m_server_ssl, m_accept_sock_ssl, true /*listen*/) != 0) {
      m_accept_sock_ssl = -1;
      return -1;
    }
    return 0;
  }

  const char *address = m_address.empty() ? nullptr : m_address.c_str();
  int ret = evhttp_bind_socket_backlog_fd(m_server_ssl, address,
      m_port_ssl, RuntimeOption::ServerBacklog);
  if (ret < 0) {
    Logger::Error("Failed to bind port %d for SSL", m_port_ssl);
    return -1;
  }
  Logger::Info("SSL enabled");
  m_accept_sock_ssl = ret;
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// request/response handling

void LibEventServer::onRequest(struct evhttp_request *request) {
  // If we are in the process of crashing, we want to reject incoming work.
  // This will prompt the load balancers to choose another server. Using
  // shutdown rather than close has the advantage that it makes fewer changes
  // to the process (eg, it doesn't close the FD so if the FD number were
  // corrupted it would be mostly harmless).
  //
  // Setting accept sock to -1 will leak FDs. But we're crashing anyways.
  if (IsCrashing) {
    if (m_accept_sock != -1) {
      shutdown(m_accept_sock, SHUT_FBLISTEN);
      m_accept_sock = -1;
    }
    if (m_accept_sock_ssl != -1) {
      shutdown(m_accept_sock_ssl, SHUT_FBLISTEN);
      m_accept_sock_ssl = -1;
    }
    return;
  }

  if (RuntimeOption::EnableKeepAlive &&
      RuntimeOption::ConnectionTimeoutSeconds > 0) {
    // before processing request, set the connection timeout
    // it's just writing a variable in libevent
    evhttp_connection_set_timeout(request->evcon,
                                  RuntimeOption::ConnectionTimeoutSeconds);
  }
  if (getStatus() == RunStatus::RUNNING) {
    RequestPriority priority = getRequestPriority(request);
    m_dispatcher.enqueue(LibEventJobPtr(new LibEventJob(request)), priority);
  } else {
    Logger::Error("throwing away one new request while shutting down");
  }
}

void LibEventServer::onResponse(int worker, evhttp_request *request,
                                int code, LibEventTransport *transport) {
  int nwritten = 0;
  bool skip_sync = false;

  if (request->evcon == nullptr) {
    evhttp_request_free(request);
    return;
  }

#ifdef _EVENT_USE_OPENSSL
  skip_sync = evhttp_is_connection_ssl(request->evcon);
#endif

  int totalSize = 0;

  if (RuntimeOption::LibEventSyncSend && !skip_sync) {
    auto const& reasonStr = transport->getResponseInfo();
    const char* reason = reasonStr.empty() ? HttpProtocol::GetReasonString(code)
                                           : reasonStr.c_str();
    timespec begin, end;
    Timer::GetMonotonicTime(begin);
#ifdef EVHTTP_SYNC_SEND_REPORT_TOTAL_LEN
    nwritten = evhttp_send_reply_sync(request, code, reason, nullptr, &totalSize);
#else
    nwritten = evhttp_send_reply_sync_begin(request, code, reason, nullptr);
#endif
    Timer::GetMonotonicTime(end);
    int64_t delay = gettime_diff_us(begin, end);
    transport->onFlushBegin(totalSize);
    transport->onFlushProgress(nwritten, delay);
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

LibEventServer::RequestPriority LibEventServer::getRequestPriority(
  struct evhttp_request* request) {
  string command = URL::getCommand(URL::getServerObject(request->uri));
  if (RuntimeOption::ServerHighPriorityEndPoints.find(command) ==
      RuntimeOption::ServerHighPriorityEndPoints.end()) {
    return PRIORITY_NORMAL;
  }
  return PRIORITY_HIGH;
}

///////////////////////////////////////////////////////////////////////////////
// PendingResponseQueue

PendingResponseQueue::PendingResponseQueue() {
  assert(RuntimeOption::ResponseQueueCount > 0);
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
  event_add(&m_event, nullptr);
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
    responses.insert(responses.end(),
                     q.m_responses.begin(), q.m_responses.end());
    q.m_responses.clear();
  }

  for (unsigned int i = 0; i < responses.size(); i++) {
    Response &res = *responses[i];
    evhttp_request *request = res.request;
    int code = res.code;

    if (request->evcon == nullptr) {
      evhttp_request_free(request);
      continue;
    }

    bool skip_sync = false;
#ifdef _EVENT_USE_OPENSSL
    skip_sync = evhttp_is_connection_ssl(request->evcon);
#endif

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
    } else if (RuntimeOption::LibEventSyncSend && !skip_sync) {
      evhttp_send_reply_sync_end(res.nwritten, request);
    } else {
      const char *reason = HttpProtocol::GetReasonString(code);
      evhttp_send_reply(request, code, reason, nullptr);
    }
  }
}

PendingResponseQueue::Response::Response()
  : request(nullptr), code(0), nwritten(0),
    chunked(false), firstChunk(false), chunk(nullptr) {
}

PendingResponseQueue::Response::~Response() {
  if (chunk) {
    evbuffer_free(chunk);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
