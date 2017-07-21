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
#include "hphp/runtime/base/libevent-http-client.h"
#include <map>
#include <vector>

#include <folly/Conv.h>

#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/util/compression.h"
#include "hphp/util/logger.h"
#include "hphp/util/timer.h"

// libevent is not exposing this data structure, but we need it.
struct evkeyvalq_ {
  struct evkeyval *tqh_first;
};

///////////////////////////////////////////////////////////////////////////////
// static handlers delegating work to instance ones

static void on_request_completed(struct evhttp_request *req, void *obj) {
  assert(obj);
  ((HPHP::LibEventHttpClient*)obj)->onRequestCompleted(req);
}

static void
on_connection_closed(struct evhttp_connection* /*conn*/, void* obj) {
  assert(obj);
  ((HPHP::LibEventHttpClient*)obj)->onConnectionClosed();
}

static void timer_callback(int /*fd*/, short /*events*/, void* context) {
  event_base_loopbreak((struct event_base *)context);
}

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// connection pooling

static std::string get_hash(const std::string &address, int port) {
  std::string hash = address;
  if (port != 80) {
    hash += ':';
    hash += folly::to<std::string>(port);
  }
  return hash;
}

ReadWriteMutex LibEventHttpClient::ConnectionPoolMutex;
std::map<std::string,int> LibEventHttpClient::ConnectionPoolConfig;
std::map<std::string,std::vector<LibEventHttpClientPtr>>
  LibEventHttpClient::ConnectionPool;

void LibEventHttpClient::SetCache(const std::string &address, int port,
                                  int maxConnection) {
  auto const hash = get_hash(address, port);

  WriteLock lock(ConnectionPoolMutex);
  if (maxConnection > 0) {
    ConnectionPoolConfig[hash] = maxConnection;
  } else {
    ConnectionPoolConfig.erase(hash);
  }
}

LibEventHttpClientPtr LibEventHttpClient::Get(const std::string &address,
                                              int port) {
  auto const hash = get_hash(address, port);

  int maxConnection = 0;
  {
    ReadLock lock(ConnectionPoolMutex);
    auto iter = ConnectionPoolConfig.find(hash);
    if (iter == ConnectionPoolConfig.end()) {
      // not configured to cache
      ServerStats::Log("evhttp.skip", 1);
      ServerStats::Log("evhttp.skip." + hash, 1);
      return LibEventHttpClientPtr(new LibEventHttpClient(address, port));
    }
    maxConnection = iter->second;
  }

  WriteLock lock(ConnectionPoolMutex);
  auto& pool = ConnectionPool[hash];
  for (unsigned int i = 0; i < pool.size(); i++) {
    auto client = pool[i];
    if (!client->m_busy) {
      client->m_busy = true;
      ServerStats::Log("evhttp.hit", 1);
      ServerStats::Log("evhttp.hit." + hash, 1);
      return client;
    }
  }

  LibEventHttpClientPtr ret(new LibEventHttpClient(address, port));
  if ((int)pool.size() < maxConnection) {
    if (pool.empty()) {
      pool.reserve(maxConnection);
    }
    pool.push_back(ret);
  }
  ServerStats::Log("evhttp.miss", 1);
  ServerStats::Log("evhttp.miss." + hash, 1);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// constructor and destructor

LibEventHttpClient::LibEventHttpClient(const std::string &address, int port)
  : m_busy(true), m_address(address), m_port(port), m_requests(0),
    m_conn(nullptr), m_thread(nullptr), m_code(0), m_response(nullptr), m_len(0) {
  m_eventBase = event_base_new();
}

LibEventHttpClient::~LibEventHttpClient() {
  clear();

  // reset all per-connection data structures
  if (m_conn) {
    evhttp_connection_free(m_conn);
  }
  if (m_eventBase) {
    event_base_free(m_eventBase);
  }
}

void LibEventHttpClient::clear() {
  // reset all per-request data structures
  if (m_thread) {
    m_thread->waitForEnd();
    delete m_thread;
    m_thread = nullptr;
  }
  m_url.clear();
  m_code = 0;
  m_codeLine.clear();
  m_len = 0;
  if (m_response) {
    free(m_response);
    m_response = nullptr;
  }
  m_responseHeaders.clear();
}

void LibEventHttpClient::release() {
  clear();
  m_busy = false;
}

///////////////////////////////////////////////////////////////////////////////

bool LibEventHttpClient::send(const std::string &url,
                              const std::vector<std::string> &headers,
                              int timeoutSeconds, bool async,
                              const void *data /* = NULL */,
                              int size /* = 0 */) {
  clear();
  m_url = url;

  evhttp_request* request = evhttp_request_new(on_request_completed, this);

  // request headers
  bool keepalive = true;
  bool addHost = true;
  for (unsigned int i = 0; i < headers.size(); i++) {
    const std::string &header = headers[i];
    size_t pos = header.find(':');
    if (pos != std::string::npos && header[pos + 1] == ' ') {
      std::string name = header.substr(0, pos);
      if (strcasecmp(name.c_str(), "Connection") == 0) {
        keepalive = false;
      } else if (strcasecmp(name.c_str(), "Host") == 0) {
        addHost = false;
      }
      int ret = evhttp_add_header(request->output_headers,
                                  name.c_str(), header.c_str() + pos + 2);
      if (ret >= 0) {
        continue;
      }
    }
    Logger::Error("invalid request header: [%s]", header.c_str());
  }
  if (keepalive) {
    evhttp_add_header(request->output_headers, "Connection", "keep-alive");
  }
  if (addHost) {
    // REVIEW: libevent never sends a Host header (nor does it properly send
    // HTTP 400 for HTTP/1.1 requests without such a header), in blatant
    // violation of RFC2616; this should perhaps be fixed in the library
    // proper.  For now, add it if it wasn't set by the caller.
    if (m_port == 80) {
      evhttp_add_header(request->output_headers, "Host", m_address.c_str());
    } else {
      std::ostringstream ss;
      ss << m_address << ":" << m_port;
      evhttp_add_header(request->output_headers, "Host", ss.str().c_str());
    }
  }

  // post data
  if (data && size) {
    evbuffer_add(request->output_buffer, data, size);
  }

  // url
  evhttp_cmd_type cmd = data ? EVHTTP_REQ_POST : EVHTTP_REQ_GET;

  // if we have a cached connection, we need to pump the event loop to clear
  // any "connection closed" events that may be sitting there patiently.
  if (m_conn) {
    event_base_loop(m_eventBase, EVLOOP_NONBLOCK);
  }

  // even if we had an m_conn immediately above, it may have been cleared out
  // by onConnectionClosed().
  if (m_conn == nullptr) {
    m_conn = evhttp_connection_new(m_address.c_str(), m_port);
    evhttp_connection_set_closecb(m_conn, on_connection_closed, this);
    evhttp_connection_set_base(m_conn, m_eventBase);
  }

  int ret = evhttp_make_request(m_conn, request, cmd, url.c_str());
  if (ret != 0) {
    Logger::Error("evhttp_make_request failed");
    return false;
  }

  if (timeoutSeconds > 0) {
    struct timeval timeout;
    timeout.tv_sec = timeoutSeconds;
    timeout.tv_usec = 0;

    event_set(&m_eventTimeout, -1, 0, timer_callback, m_eventBase);
    event_base_set(m_eventBase, &m_eventTimeout);
    event_add(&m_eventTimeout, &timeout);
  }

  if (async) {
    m_thread = new AsyncFunc<LibEventHttpClient>
      (this, &LibEventHttpClient::sendImpl);
    m_thread->start();
  } else {
    IOStatusHelper io("libevent_http", m_address.c_str(), m_port);
    sendImpl();
  }
  return true;
}

void LibEventHttpClient::sendImpl() {
  SlowTimer timer(RuntimeOption::HttpSlowQueryThreshold, "evhttp",
                  m_url.c_str());
  event_base_dispatch(m_eventBase);
  event_del(&m_eventTimeout);
}

void LibEventHttpClient::onRequestCompleted(evhttp_request* request) {
  if (!request) {
    // eek -- this is just a clean-up notification because the connection's
    // been closed, but we already dealt with it in onConnectionClosed
    return;
  }

  // response code line
  m_code = request->response_code;
  if (request->response_code_line) {
    m_codeLine = request->response_code_line;
  }

  bool gzip = false;
  // response headers
  for (evkeyval *p = ((evkeyvalq_*)request->input_headers)->tqh_first; p;
       p = p->next.tqe_next) {
    if (p->key && p->value) {
      if (strcasecmp(p->key, "Content-Encoding") == 0 &&
          strncmp(p->value, "gzip", 4) == 0 &&
          (!p->value[4] || isspace(p->value[4]))) {
        // in the (illegal) case of multiple Content-Encoding headers, any one
        // with the value 'gzip' means we treat it as gzip.
        gzip = true;
      }
      m_responseHeaders.push_back(std::string(p->key) + ": " + p->value);
    }
  }

  // response body
  m_len = EVBUFFER_LENGTH(request->input_buffer);
  if (gzip) {
    m_response =
      gzdecode((const char*)EVBUFFER_DATA(request->input_buffer), m_len);
  } else {
    m_response = (char*)malloc(m_len + 1);
    strncpy(m_response, (char*)EVBUFFER_DATA(request->input_buffer), m_len);
    m_response[m_len] = '\0';
  }

  ++m_requests;
  event_base_loopbreak(m_eventBase);
}

void LibEventHttpClient::onConnectionClosed() {
  m_conn = nullptr;
  m_requests = 0;
}

char *LibEventHttpClient::recv(int &len) {
  if (m_thread) {
    m_thread->waitForEnd();
    delete m_thread;
    m_thread = nullptr;
  }

  char *ret = m_response;
  len = m_len;
  m_response = nullptr;
  m_len = 0;
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}
