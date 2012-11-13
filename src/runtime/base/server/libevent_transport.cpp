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

#include <runtime/base/server/libevent_transport.h>
#include <runtime/base/server/libevent_server.h>
#include <runtime/base/server/server.h>
#include <runtime/base/runtime_option.h>
#include <util/util.h>
#include <util/logger.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// libevent is not exposing this data structure, but we need it.
struct m_evkeyvalq {
  struct evkeyval *tqh_first;
};

LibEventTransport::LibEventTransport(LibEventServer *server,
                                     evhttp_request *request,
                                     int workerId)
  : m_server(server), m_request(request), m_eventBasePostData(NULL),
    m_workerId(workerId), m_sendStarted(false), m_sendEnded(false) {
  // HttpProtocol::PrepareSystemVariables needs this
  evbuffer *buf = m_request->input_buffer;
  ASSERT(buf);
  m_requestSize = EVBUFFER_LENGTH(buf);
  if (m_requestSize) {
    evbuffer_expand(buf, m_requestSize + 1); // allowing NULL termination
    // EVBUFFER_DATA(buf) might change after evbuffer_expand
    ((char*)EVBUFFER_DATA(buf))[m_requestSize] = '\0';
  }

  m_remote_host = m_request->remote_host;
  m_remote_port = m_request->remote_port;

  {
    char buf[6];
    snprintf(buf, 6, "%d.%d", m_request->major, m_request->minor);
    m_http_version = buf;
  }

  switch (m_request->type) {
  case EVHTTP_REQ_GET:
    m_method = Transport::GET;
    m_requestSize += 3;
    break;
  case EVHTTP_REQ_POST:
    m_method = Transport::POST;
    m_requestSize += 4;
    break;
  case EVHTTP_REQ_HEAD:
    m_method = Transport::HEAD;
    m_requestSize += 4;
    break;
  default:
    ASSERT(false);
    m_method = Transport::UnknownMethod;
    break;
  }
  m_extended_method = m_request->ext_method;

  ASSERT(m_request->input_headers);
  for (evkeyval *p = ((m_evkeyvalq*)m_request->input_headers)->tqh_first; p;
       p = p->next.tqe_next) {
    if (p->key && p->value) {
      m_requestHeaders[p->key].push_back(p->value);
      //key, value, ": " and CR/LF
      m_requestSize += strlen(p->key) + strlen(p->value) + 4;
    }
  }

  m_url = m_request->uri;
  m_requestSize += m_url.size();
  m_requestSize += m_http_version.size(); //version number in "HTTP/x.y"
  m_requestSize += 11; // HTTP/=5, 2 spaces for url, and CR/LF x2 (first+last)
}

const char *LibEventTransport::getUrl() {
  return m_url.c_str();
}

const char *LibEventTransport::getRemoteHost() {
  return m_remote_host.c_str();
}

uint16 LibEventTransport::getRemotePort() {
  return m_remote_port;
}

const void *LibEventTransport::getPostData(int &size) {
  if (m_sendEnded) {
    size = 0;
    return 0;
  }
  evbuffer *buf = m_request->input_buffer;

  ASSERT(buf);
  size = EVBUFFER_LENGTH(buf);
  return EVBUFFER_DATA(buf);
}

bool LibEventTransport::hasMorePostData() {
#ifdef EVHTTP_PORTABLE_READ_LIMITING
  if (m_request->ntoread <= 0) {
    if (m_eventBasePostData != NULL) {
      event_base_free(m_eventBasePostData);
      m_eventBasePostData = NULL;
    }
    return false;
  }
  return true;
#else
  return false;
#endif
}

const void *LibEventTransport::getMorePostData(int &size) {
#ifdef EVHTTP_PORTABLE_READ_LIMITING
  if (m_request->ntoread == 0) {
    if (m_eventBasePostData != NULL) {
      event_base_free(m_eventBasePostData);
      m_eventBasePostData = NULL;
    }
    size = 0;
    return NULL;
  }

  evbuffer *buf = m_request->input_buffer;
  ASSERT(buf);
  evbuffer_drain(buf, EVBUFFER_LENGTH(buf));

  if (evhttp_get_more_post_data(m_request, &m_eventBasePostData,
                                &m_moreDataRead)) {
    buf = m_request->input_buffer;
    ASSERT(buf);
    size = EVBUFFER_LENGTH(buf);
    evbuffer_expand(buf, size + 1); // allowing NULL termination
    // EVBUFFER_DATA(buf) might change after evbuffer_expand
    ((char*)EVBUFFER_DATA(buf))[size] = '\0';
    if (m_request->ntoread == 0) {
      if (m_eventBasePostData != NULL) {
        event_base_free(m_eventBasePostData);
        m_eventBasePostData = NULL;
      }
      evhttp_get_post_data_done(m_request);
    }
    return EVBUFFER_DATA(buf);
  }
  if (m_eventBasePostData != NULL) {
    event_base_free(m_eventBasePostData);
    m_eventBasePostData = NULL;
  }
  evhttp_get_post_data_done(m_request);
  size = 0;
  return NULL;
#else
  size = 0;
  return NULL;
#endif
}

Transport::Method LibEventTransport::getMethod() {
  return m_method;
}

const char *LibEventTransport::getExtendedMethod() {
  return m_extended_method;
}

std::string LibEventTransport::getHTTPVersion() const {
  return m_http_version;
}

int LibEventTransport::getRequestSize() const {
  return m_requestSize;
}

std::string LibEventTransport::getHeader(const char *name) {
  ASSERT(name && *name);

  HeaderMap::const_iterator iter = m_requestHeaders.find(name);
  if (iter != m_requestHeaders.end()) {
    return iter->second[0];
  }
  return "";
}

void LibEventTransport::getHeaders(HeaderMap &headers) {
  if (&m_requestHeaders != &headers) {
    headers = m_requestHeaders;
  }
}

void LibEventTransport::addHeaderImpl(const char *name, const char *value) {
  ASSERT(name && *name);
  ASSERT(value);
  ASSERT(m_request->output_headers);

  if (m_sendStarted) {
    Logger::Error("trying to add header '%s: %s' after 1st chunk",
                  name, value);
    return;
  }

  int ret = evhttp_add_header(m_request->output_headers, name, value);
  if (ret < 0) {
    Logger::Error("failed to add header '%s: %s'", name, value);
  }
}

void LibEventTransport::removeHeaderImpl(const char *name) {
  ASSERT(name && *name);
  ASSERT(m_request->output_headers);

  if (m_sendStarted) {
    Logger::Error("trying to remove header '%s' after 1st chunk", name);
    return;
  }

  evhttp_remove_header(m_request->output_headers, name);
}

void LibEventTransport::addRequestHeaderImpl(const char *name,
                                             const char *value) {
  ASSERT(name && *name);
  ASSERT(value);
  ASSERT(m_request->input_headers);

  int ret = evhttp_add_header(m_request->input_headers, name, value);
  if (ret < 0) {
    Logger::Error("failed to add header '%s: %s'", name, value);
    return;
  }
  m_requestHeaders[name].push_back(value);
}

void LibEventTransport::removeRequestHeaderImpl(const char *name) {
  ASSERT(name && *name);
  ASSERT(m_request->input_headers);
  evhttp_remove_header(m_request->input_headers, name);
  m_requestHeaders.erase(name);
}

bool LibEventTransport::isServerStopping() {
  return m_server->getStatus() == Server::STOPPED;
}

void LibEventTransport::sendImpl(const void *data, int size, int code,
                                 bool chunked) {
  ASSERT(data);
  ASSERT(!m_sendEnded);
  ASSERT(!m_sendStarted || chunked);

  if (chunked) {
    ASSERT(m_method != HEAD);
    evbuffer *chunk = evbuffer_new();
    evbuffer_add(chunk, data, size);
    /*
     * Chunked replies are sent async, so there is no way to know the
     * time it took to flush the response, but tracking the bytes sent is
     * very useful.
     */
    onChunkedProgress(size);
    m_server->onChunkedResponse(m_workerId, m_request, code, chunk,
                               !m_sendStarted);
  } else {
    if (m_method != HEAD) {
      evbuffer_add(m_request->output_buffer, data, size);
    } else if (!evhttp_find_header(m_request->output_headers,
                                   "Content-Length")) {
      char buf[11];
      snprintf(buf, sizeof(buf), "%d", size);
      addHeaderImpl("Content-Length", buf);
    }
    m_server->onResponse(m_workerId, m_request, code, this);
    m_sendEnded = true;
  }
  m_sendStarted = true;
}

void LibEventTransport::onSendEndImpl() {
  if (m_chunkedEncoding) {
    m_server->onChunkedResponseEnd(m_workerId, m_request);
    m_sendEnded = true;
  } else {
    ASSERT(m_sendEnded); // otherwise, we didn't call send for this request
  }
}

///////////////////////////////////////////////////////////////////////////////
}
