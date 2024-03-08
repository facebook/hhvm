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

#include "hphp/runtime/server/fastcgi/fastcgi-transport.h"
#include "hphp/runtime/base/configs/server.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/server/fastcgi/fastcgi-server.h"
#include "hphp/runtime/server/http-protocol.h"
#include "hphp/runtime/server/transport.h"

#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>

namespace HPHP {

using folly::IOBufQueue;
using folly::io::Cursor;

///////////////////////////////////////////////////////////////////////////////

/*
 * The logic used here for reading POST data buffers is mostly borrowed from
 * proxygen server. Currently the RequestBodyReadLimit is not supported by
 * FastCGI so we don't ever pause ingress and POST data is always received
 * during VM execution.
 *
 * NB: locking is important when accessing m_bodyQueue as the session will
 *     also write into that structure via onBody.
 */
const void *FastCGITransport::getPostData(size_t& size) {
  // the API contract is that you can call getPostData repeatedly until
  // you call getMorePostData
  if (m_firstBody) {
    CHECK(m_currBody);
    size = m_currBody->length();
    return m_currBody->data();
  }
  return getMorePostData(size);
}

const void *FastCGITransport::getMorePostData(size_t& size) {
  // session will terminate the request if we don't receive data in
  // this much time
  long maxWait = Cfg::Server::ConnectionTimeoutSeconds;
  if (maxWait <= 0) {
    maxWait = 50; // this was the default read timeout in LibEventServer
  }

  Lock lock(this);
  while (m_bodyQueue.empty() && !m_bodyComplete) {
    wait(maxWait);
  }

  // For chunk encodings, we way receive an EOM with no data, such that
  // hasMorePostData returns true (because client is not yet complete),
  // client sends EOM, getMorePostData should return 0/nullptr
  size = 0;
  if (!m_bodyQueue.empty()) {
    // this is the first body if it wasn't set and buf is unset
    m_firstBody = !(m_firstBody && m_currBody);
    m_currBody = m_bodyQueue.pop_front();

    CHECK(m_currBody && m_currBody->length() > 0);
    size = m_currBody->length();
    return m_currBody->data();
  }

  return nullptr;
}

bool FastCGITransport::hasMorePostData() {
  Lock lock(this);
  return !m_bodyComplete || !m_bodyQueue.empty();
}

///////////////////////////////////////////////////////////////////////////////

// takes an unmangled header name
std::string FastCGITransport::getHeader(const char* name) {
  std::string key("HTTP_");
  for (auto p = name; *p; ++p) {
    key += (*p == '-') ? '_' : toupper(*p);
  }

  if (m_requestParams.count(key)) {
    return m_requestParams[key];
  }

  // Special headers that are not prefixed with HTTP_
  if (strcasecmp(name, "Authorization") == 0) {
    return getParamTyped<std::string>("Authorization");
  }

  if (strcasecmp(name, "Content-Length") == 0) {
    return getParamTyped<std::string>("CONTENT_LENGTH");
  }

  if (strcasecmp(name, "Content-Type") == 0) {
    return getParamTyped<std::string>("CONTENT_TYPE");
  }

  return "";
}

const HeaderMap& FastCGITransport::getHeaders() {
  // lazily construct unmangled headers
  if (m_unmangledRequestParams.empty()) {
    for (auto& pair : m_requestParams) {
      auto key = unmangleHeader(pair.first);
      if (!key.empty()) {
        m_unmangledRequestParams[key] = { pair.second };
      }
    }
  }
  return m_unmangledRequestParams;
}

void FastCGITransport::getTransportParams(HeaderMap& serverParams) {
  for (auto& pair : m_requestParams) {
    serverParams[pair.first] = { pair.second };
  }
}

///////////////////////////////////////////////////////////////////////////////

void FastCGITransport::addHeaderImpl(const char* name, const char* value) {
  CHECK(!m_headersSent);

  m_responseHeaders[name].emplace_back(value);
}

void FastCGITransport::removeHeaderImpl(const char* name) {
  CHECK(!m_headersSent);

  m_responseHeaders.erase(name);
}

void FastCGITransport::sendResponseHeaders(IOBufQueue& queue, int code) {
  auto appender = [&](folly::StringPiece sp) mutable { queue.append(sp); };
  if (code != 200) {
    auto info = getResponseInfo();
    auto reason = !info.empty() ? info : HttpProtocol::GetReasonString(code);

    folly::format("Status: {} {}\r\n", code, reason)(appender);
  }

  for (auto& header : m_responseHeaders) {
    for (auto& value : header.second) {
      folly::format("{}: {}\r\n", header.first, value)(appender);
    }
  }

  queue.append("\r\n", 2);
}

///////////////////////////////////////////////////////////////////////////////

void FastCGITransport::sendImpl(const void* data, int size, int code,
                                bool /*chunked*/, bool eom) {
  if (!m_headersSent) {
    m_headersSent = true;
    sendResponseHeaders(m_txBuf, code);
  }

  m_txBuf.append(data, size);
  m_session->onStdOut(m_txBuf.move()); // session will handle locking

  if (eom) {
    onSendEndImpl();
  }
}

void FastCGITransport::onSendEndImpl() {
  // Don't send onComplete() more than once (required because of the eom flag
  // on sendImpl).
  if (m_sendEnded) {
    return;
  }

  m_sendEnded = true;
  // NB: onSendEnd() is only sent when the VM is finished with the transport.
  //     at this point we are free to do whatever we'd like with the transport.
  m_session->onComplete();
}

///////////////////////////////////////////////////////////////////////////////

void FastCGITransport::onBody(std::unique_ptr<folly::IOBuf> chain) {
  Lock lock(this);
  m_bodyQueue.append(std::move(chain));
  notify(); // wake-up the VM
}

void FastCGITransport::onBodyComplete() {
  Lock lock(this);
  m_bodyComplete = true;
  notify(); // wake-up the VM
}

void FastCGITransport::onHeader(std::unique_ptr<folly::IOBuf> key_chain,
                                std::unique_ptr<folly::IOBuf> value_chain) {
  Cursor keyCur(key_chain.get());
  auto key = keyCur.readFixedString(key_chain->computeChainDataLength());

  // Don't allow requests to inject an HTTP_PROXY environment variable by
  // sending a Proxy header.
  if (strcasecmp(key.c_str(), "HTTP_PROXY") == 0) return;

  Cursor valCur(value_chain.get());
  auto value = valCur.readFixedString(value_chain->computeChainDataLength());

  m_requestParams[key] = value;

  // if we've already built the unmangled map, add to that as well
  if (!m_unmangledRequestParams.empty()) {
    auto unmangledKey = unmangleHeader(key);
    if (!unmangledKey.empty()) {
      m_unmangledRequestParams[unmangledKey] = { value };
    }
  }
}

void FastCGITransport::onHeadersComplete() {
  m_scriptName   = getParamTyped<std::string>("SCRIPT_FILENAME");
  m_docRoot      = getParamTyped<std::string>("DOCUMENT_ROOT");
  m_pathTrans    = getParamTyped<std::string>("PATH_TRANSLATED");
  m_serverObject = getParamTyped<std::string>("SCRIPT_NAME");

  if (!m_docRoot.empty() && *m_docRoot.rbegin() != '/') {
    m_docRoot += '/';
  }

  if (m_scriptName.empty() || Cfg::Server::FixPathInfo) {
    // According to php-fpm, some servers don't set SCRIPT_FILENAME. In
    // this case, it uses PATH_TRANSLATED.
    // Added runtime option to change m_scriptFilename to s_pathTran
    // which will allow mod_fastcgi and mod_action to work correctly.
    m_scriptName = getPathTranslated();
  }

  [&] { // do a check for mod_proxy_fcgi and remove the extra portions of
        // the string
    if (!strncmp(m_scriptName.c_str(), "proxy:", sizeof("proxy:") - 1)) {
      folly::StringPiece newName {m_scriptName};

      // remove the proxy:type + :// from the start.
      auto proxyPos = newName.find("://");
      if (proxyPos == std::string::npos) return; // invalid mod_proxy

      newName.advance(proxyPos + sizeof("://"));

      // remove everything before the first / which is host:port
      auto slashPos = newName.find('/');
      if (slashPos == std::string::npos) {
        m_scriptName.clear(); // empty path
        return;
      }
      newName.advance(slashPos);

      // remove everything after the first ?
      auto questionPos = newName.find('?');
      if (questionPos != std::string::npos) {
        newName.subtract(newName.size() - questionPos);
      }

      m_scriptName = newName.str();
    }
  }();

  // RequestURI needs script_filename and path_translated to not include
  // the document root
  if (!m_scriptName.empty()) {
    if (m_scriptName.find(m_docRoot) == 0) {
      m_scriptName.erase(0, m_docRoot.length());
    } else {
      // if the document root isn't in the url set document root to /
      m_docRoot = "/";
    }
  }

  // XXX: This was originally done before remapping scriptName but that seemed
  // wrong as the value of docRoot may change. I haven't been able to confirm
  // that this is correct either.
  if (m_pathTrans.find(m_docRoot) == 0) {
    m_pathTrans.erase(0, m_docRoot.length());
  }

  auto qs = getParamTyped<std::string>("QUERY_STRING");
  if (!qs.empty()) {
    m_serverObject += "?";
    m_serverObject += qs;
  }

  // Treat everything apart from GET and HEAD as a post to be like php-src.
  auto const ex = getExtendedMethod();
  if (!strcmp(ex, "GET")) {
    m_method = Method::GET;
  } else if (!strcmp(ex, "HEAD")) {
    m_method = Method::HEAD;
  } else {
    m_method = Method::POST;
  }

  // IIS sets this value but sets it to off when SSL is off.
  if (m_requestParams.count("HTTPS") && !m_requestParams["HTTPS"].empty() &&
      strcasecmp(m_requestParams["HTTPS"].c_str(), "OFF")) {
    setSSL();
  }
}

///////////////////////////////////////////////////////////////////////////////

std::string FastCGITransport::unmangleHeader(const std::string& name) const {
  if (name == "Authorization") {
    return name; // Already unmangled
  }

  if (name == "CONTENT_LENGTH") {
    return "Content-Length";
  }

  if (name == "CONTENT_TYPE") {
    return "Content-Type";
  }

  if (strncasecmp(name.c_str(), "HTTP_", 5)) {
    return "";
  }

  std::string ret;
  bool is_upper = true;
  for (auto const& c : folly::StringPiece(name, 5)) {
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

///////////////////////////////////////////////////////////////////////////////
}
