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

#include "hphp/runtime/server/fastcgi/fastcgi-transport.h"
#include "hphp/runtime/server/fastcgi/fastcgi-server.h"
#include "hphp/runtime/server/transport.h"
#include "hphp/runtime/base/runtime-error.h"
#include "folly/io/IOBuf.h"
#include "folly/io/IOBufQueue.h"
#include "thrift/lib/cpp/async/TAsyncTransport.h"
#include "thrift/lib/cpp/async/TAsyncTimeout.h"
#include "thrift/lib/cpp/transport/TSocketAddress.h"
#include "hphp/util/logger.h"
#include "hphp/util/timer.h"
#include "folly/MoveWrapper.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>

using folly::IOBuf;
using folly::IOBufQueue;
using folly::io::Cursor;
using folly::io::QueueAppender;

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

FastCGITransport::FastCGITransport(FastCGIConnection* connection, int id)
  : m_connection(connection),
    m_id(id),
    m_remotePort(0),
    m_serverPort(0),
    m_method(Method::Unknown),
    m_requestSize(0),
    m_headersSent(false),
    m_readMore(false),
    m_waiting(0),
    m_readComplete(false) {}

const char *FastCGITransport::getUrl() {
  return m_requestURI.c_str();
}

const std::string FastCGITransport::getPathTranslated() {
  return m_pathTranslated;
}

const std::string FastCGITransport::getDocumentRoot() {
  return m_documentRoot;
}

const char *FastCGITransport::getRemoteHost() {
  return m_remoteHost.c_str();
}

const char *FastCGITransport::getRemoteAddr() {
  return m_remoteAddr.c_str();
}

uint16_t FastCGITransport::getRemotePort() {
  return m_remotePort;
}

const char *FastCGITransport::getServerName() {
  return m_serverName.c_str();
}

const char *FastCGITransport::getServerAddr() {
  return (!m_serverAddr.empty()) ? m_serverAddr.c_str() :
                                   Transport::getServerAddr();
}

uint16_t FastCGITransport::getServerPort() {
  return (m_serverPort != 0) ? m_serverPort : Transport::getServerPort();
}

const char *FastCGITransport::getServerSoftware() {
  return (!m_serverSoftware.empty()) ? m_serverSoftware.c_str() :
                                       Transport::getServerSoftware();
}

const void *FastCGITransport::getPostData(int &size) {
  assert(!m_readMore);
  return getPostDataImpl(size, false);
}

const void *FastCGITransport::getMorePostData(int &size) {
  m_readMore = true;
  return getPostDataImpl(size, true);
}

bool FastCGITransport::hasMorePostData() {
  m_monitor.lock();
  bool result = !m_readComplete || !m_bodyQueue.empty();
  m_monitor.unlock();
  return result;
}

const void *FastCGITransport::getPostDataImpl(int &size, bool progress) {
  const void* result = nullptr;
  size = 0;
  if (!progress && m_currBody != nullptr) {
    size = m_currBody->length();
    return m_currBody->data();
  }
  m_monitor.lock();
  while (size == 0) {
    if (m_bodyQueue.empty() && m_readComplete) {
      break;
    }
    while (!m_bodyQueue.empty()) {
      size = m_bodyQueue.front()->length();
      if (size != 0) {
        m_currBody = m_bodyQueue.pop_front();
        result = m_currBody->data();
        break;
      } else {
        m_bodyQueue.pop_front();
      }
    }
    while (size == 0 && m_bodyQueue.empty() && !m_readComplete) {
      m_waiting += 1;
      m_monitor.wait();
      m_waiting -= 1;
    }
  }
  if (m_waiting > 0) {
    m_monitor.notify();
  }
  m_monitor.unlock();
  return result;
}

Transport::Method FastCGITransport::getMethod() {
  return m_method;
}

const char *FastCGITransport::getExtendedMethod() {
  return m_extendedMethod.c_str();
}

std::string FastCGITransport::getHTTPVersion() const {
  return m_httpVersion;
}

int FastCGITransport::getRequestSize() const {
  return m_requestSize;
}

const char *FastCGITransport::getServerObject() {
  return m_serverObject.c_str();
}

std::string FastCGITransport::unmangleHeader(const std::string& name) {
  if (!boost::istarts_with(name, "HTTP_")) {
    return "";
  }

  std::string ret;
  bool is_upper = true;
  for (auto& c : name.substr(5)) {
    if (c == '_') {
      ret += '-';
      is_upper = true;
    } else {
      ret += is_upper ? toupper(c) : tolower(c);
      is_upper = false;
    }
  }
  return ret;
}

std::string FastCGITransport::mangleHeader(const std::string& name) {
  std::string ret;
  for (auto& c : name) {
    if (c == '-') {
      ret += '_';
    } else {
      ret += toupper(c);
    }
  }
  return "HTTP_" + ret;
}

static const std::string
  s_contentLength("CONTENT_LENGTH"),
  s_contentType("CONTENT_TYPE");

/**
 * Passed an HTTP header like "Cookie" or "Cache-Control"
 **/
std::string FastCGITransport::getHeader(const char *name) {
  auto *header = getRawHeaderPtr(mangleHeader(name));
  if (header) {
    return *header;
  }
  if (strcasecmp(name, "Content-Length") == 0) {
    return getRawHeader(s_contentLength); // No HTTP_ prefix for CONTENT_LENGTH
  }
  if (strcasecmp(name, "Content-Type") == 0) {
    return getRawHeader(s_contentType); // No HTTP_ prefix for CONTENT_TYPE
  }
  return "";
}

/**
 * Passed a FastCGI mangled header like "HTTP_COOKIE" or "HTTP_CACHE_CONTROL"
 **/
std::string FastCGITransport::getRawHeader(const std::string& name) {
  auto header = getRawHeaderPtr(name);
  return (header == nullptr) ? std::string{""} : *header;
}

std::string* FastCGITransport::getRawHeaderPtr(const std::string& name) {
  assert(boost::to_upper_copy(name) == name);
  auto it = m_requestHeaders.find(name);
  return (it == m_requestHeaders.end()) ? nullptr : &it->second;
}

int FastCGITransport::getIntHeader(const std::string& name) {
  try {
    auto* key = getRawHeaderPtr(name);
    return (key != nullptr && !key->empty()) ? std::stoi(*key) : 0;
  } catch (std::invalid_argument&) {
    return 0;
  } catch (std::out_of_range&) {
    return 0;
  }
}

void FastCGITransport::getHeaders(HeaderMap &headers) {
  for (auto& pair : m_requestHeaders) {
    auto key = unmangleHeader(pair.first);
    if (!key.empty()) {
      headers[key] = { pair.second };
    }
  }
}

void FastCGITransport::getTransportParams(HeaderMap &serverParams) {
  for (auto& pair : m_requestHeaders) {
    serverParams[pair.first] = { pair.second };
  }
}

void FastCGITransport::addHeaderImpl(const char* name, const char* value) {
  CHECK(!m_headersSent);
  auto it = m_responseHeaders.find(name);
  if (it == m_responseHeaders.end()) {
    m_responseHeaders.emplace(name, std::vector<std::string>{value});
  } else {
    it->second.emplace_back(value);
  }
}

void FastCGITransport::removeHeaderImpl(const char* name) {
  CHECK(!m_headersSent);

  m_responseHeaders.erase(name);
}

static const std::string
  s_status("Status: "),
  s_colon(": "),
  s_newline("\r\n");

void FastCGITransport::sendResponseHeaders(IOBufQueue& queue, int code) {
  CHECK(!m_headersSent);
  m_headersSent = true;

  if (code != 200) {
    queue.append(s_status);
    queue.append(std::to_string(code));
    queue.append(s_newline);
  }

  for (auto& header : m_responseHeaders) {
    for (auto& value : header.second) {
      queue.append(header.first);
      queue.append(s_colon);
      queue.append(value);
      queue.append(s_newline);
    }
  }
  queue.append(s_newline);
}

void FastCGITransport::sendImpl(const void *data, int size, int code,
                                bool chunked) {
  IOBufQueue queue;
  if (!m_headersSent) {
    sendResponseHeaders(queue, code);
  }
  queue.append(IOBuf::copyBuffer(data, size));
  folly::MoveWrapper<std::unique_ptr<IOBuf>> chain_wrapper(queue.move());
  Callback* callback = m_callback;
  auto fn = [callback, chain_wrapper]() mutable {
    if (callback) {
      callback->onStdOut(std::move(*chain_wrapper));
    }
  };
  m_connection->getEventBase()->runInEventBaseThread(fn);
}

void FastCGITransport::onSendEndImpl() {
  Callback* callback = m_callback;
  auto fn = [callback]() mutable {
    if (callback) {
      callback->onComplete();
    }
  };
  m_connection->getEventBase()->runInEventBaseThread(fn);
}

void FastCGITransport::onBody(std::unique_ptr<folly::IOBuf> chain) {
  Cursor cursor(chain.get());
  size_t length = chain->computeChainDataLength();
  std::string s = cursor.readFixedString(length);
  m_monitor.lock();
  m_bodyQueue.append(std::move(chain));
  if (m_waiting > 0) {
    m_monitor.notify();
  }
  m_monitor.unlock();
}

void FastCGITransport::onBodyComplete() {
  m_monitor.lock();
  m_readComplete = true;
  if (m_waiting > 0) {
    m_monitor.notify();
  }
  m_monitor.unlock();
}

void FastCGITransport::onHeader(std::unique_ptr<folly::IOBuf> key_chain,
                                std::unique_ptr<folly::IOBuf> value_chain) {
  Cursor cursor(key_chain.get());
  std::string key = cursor.readFixedString(key_chain->computeChainDataLength());
  cursor = Cursor(value_chain.get());
  std::string value = cursor.readFixedString(
                               value_chain->computeChainDataLength());
  m_requestHeaders.emplace(key, value);
}

static const std::string
  s_requestURI("REQUEST_URI"),
  s_remoteHost("REMOTE_HOST"),
  s_remoteAddr("REMOTE_ADDR"),
  s_serverName("SERVER_NAME"),
  s_serverAddr("SERVER_ADDR"),
  s_serverSoftware("SERVER_SOFTWARE"),
  s_extendedMethod("REQUEST_METHOD"),
  s_httpVersion("HTTP_VERSION"),
  s_documentRoot("DOCUMENT_ROOT"),
  s_remotePort("REMOTE_PORT"),
  s_serverPort("SERVER_PORT"),
  s_pathTranslated("PATH_TRANSLATED"),
  s_scriptName("SCRIPT_NAME"),
  s_scriptFilename("SCRIPT_FILENAME"),
  s_queryString("QUERY_STRING"),
  s_https("HTTPS");

void FastCGITransport::onHeadersComplete() {
  m_requestURI = getRawHeader(s_requestURI);
  m_remoteHost = getRawHeader(s_remoteHost);
  m_remoteAddr = getRawHeader(s_remoteAddr);
  m_serverName = getRawHeader(s_serverName);
  m_serverAddr = getRawHeader(s_serverAddr);
  m_serverSoftware = getRawHeader(s_serverSoftware);
  m_extendedMethod = getRawHeader(s_extendedMethod);
  m_httpVersion = getRawHeader(s_httpVersion);
  m_serverObject = getRawHeader(s_scriptName);
  m_pathTranslated = getRawHeader(s_pathTranslated);
  m_documentRoot = getRawHeader(s_documentRoot);
  if (!m_documentRoot.empty() &&
      m_documentRoot[m_documentRoot.length() - 1] != '/') {
    m_documentRoot += '/';
  }

  m_serverPort = getIntHeader(s_serverPort);
  m_requestSize = getIntHeader(s_contentLength);
  int port = getIntHeader(s_remotePort);
  if (port < std::numeric_limits<decltype(m_remotePort)>::min() ||
      port > std::numeric_limits<decltype(m_remotePort)>::max()) {
    port = 0;
  }
  m_remotePort = port;

  auto* value = getRawHeaderPtr(s_https);
  if (value != nullptr && !value->empty()) {
    auto lValue = std::string{*value};
    for (auto& c : lValue) {
      c = std::toupper(c);
    }
    // IIS sets this value but sets it to off when SSL is off.
    if (lValue != "OFF") {
      setSSL();
    }
  }

  // Treat everything apart from GET and HEAD as a post to be like php-src.
  if (m_extendedMethod == "GET") {
    m_method = Method::GET;
  } else if (m_extendedMethod == "HEAD") {
    m_method = Method::HEAD;
  } else {
    m_method = Method::POST;
  }

  if (m_pathTranslated.empty()) {
    // If someone follows http://wiki.nginx.org/HttpFastcgiModule they won't
    // pass in PATH_TRANSLATED and instead will just send SCRIPT_FILENAME
    m_pathTranslated = getRawHeader(s_scriptFilename);
  }

  // do a check for mod_proxy_cgi and remove the start portion of the string
  const std::string modProxy = "proxy:fcgi://";
  if (m_pathTranslated.find(modProxy) == 0) {
    m_pathTranslated = m_pathTranslated.substr(modProxy.length());
    // remove everything before the first / which is host:port
    int slashPos = m_pathTranslated.find('/');
    if (slashPos != String::npos) {
      m_pathTranslated = m_pathTranslated.substr(slashPos);
    }
  }

  // RequestURI needs path_translated to not include the document root
  if (!m_pathTranslated.empty()) {
    if (m_pathTranslated.find(m_documentRoot) == 0) {
      m_pathTranslated = m_pathTranslated.substr(m_documentRoot.length());
    } else {
      // if the document root isn't in the url set document root to /
      m_documentRoot = "/";
    }
  }

  auto* queryString = getRawHeaderPtr(s_queryString);
  if (queryString != nullptr && !queryString->empty()) {
    m_serverObject += "?" + *queryString;
  }

  m_connection->handleRequest(m_id);
}

///////////////////////////////////////////////////////////////////////////////
}

