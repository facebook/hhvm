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
#include "hphp/runtime/server/proxygen/proxygen-transport.h"

#include "hphp/runtime/server/proxygen/proxygen-server.h"
#include "hphp/runtime/server/server.h"
#include "hphp/util/logger.h"
#include "hphp/util/timer.h"

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <memory>
#include <set>

using proxygen::HTTPException;
using proxygen::HTTPHeaders;
using proxygen::HTTPMessage;
using proxygen::HTTPMethod;
using proxygen::HTTPTransaction;
using proxygen::HTTP_HEADER_CONNECTION;
using proxygen::HTTP_HEADER_CONTENT_LENGTH;
using proxygen::HTTP_HEADER_EXPECT;
using proxygen::HTTP_HEADER_HOST;
using proxygen::HTTP_HEADER_TRANSFER_ENCODING;
using folly::IOBuf;
using std::shared_ptr;
using std::unique_ptr;

using namespace proxygen;

namespace {
static std::set<std::string> s_post_methods{
  "OPTIONS",
  "REPORT",
  "PROPFIND",
  "PROPPATCH",
  "MKCOL",
  "MKCALENDAR",
  "PUT",
  "DELETE",
  "LOCK",
  "UNLOCK",
};
}

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

ServiceData::ExportedTimeSeries* ProxygenTransport::s_requestErrorCount;
ServiceData::ExportedTimeSeries* ProxygenTransport::s_requestNonErrorCount;

/**
 * The server side push handler is just here to catch errors and
 * egress state changes.
 */
struct PushTxnHandler : proxygen::HTTPPushTransactionHandler {
  PushTxnHandler(uint64_t pushId,
                 const std::shared_ptr<ProxygenTransport>& transport,
                 const char *host, const char *path,
                 uint8_t priority, const Array &promiseHeaders,
                 const Array &responseHeaders,
                 bool isSecure)
      : m_pushId(pushId),
        m_transport(transport) {
    createPushPromise(host, path, priority, promiseHeaders, isSecure);
    m_response.setStatusCode(200);
    addHeadersToMessage(m_response, responseHeaders);
  }

  proxygen::HTTPTransaction *getOrCreateTransaction(
    proxygen::HTTPTransaction *clientTxn, HTTPMessage** msg, bool newPushOk) {
    if (!m_pushTxn && newPushOk) {
      if (!clientTxn) {
        *msg = nullptr;
        return nullptr;
      }
      m_pushTxn = clientTxn->newPushedTransaction(this);
      *msg = &m_pushPromise;
    } else {
      if (m_egressError) {
        *msg = nullptr;
        return nullptr;
      }
      *msg = &m_response;
    }
    return m_pushTxn;
  }

  proxygen::HTTPTransaction *getTransaction() const {
    return m_pushTxn;
  }

  // HTTPPushTransactionHandler interface
  void setTransaction(proxygen::HTTPTransaction* txn)
    noexcept override {
    m_pushTxn = txn;
  };

  void detachTransaction() noexcept override {
    VLOG(5) << "detachTransaction PushTxnHandler=" << (uint64_t) this;
    m_pushTxn = nullptr;
    m_transport->removePushTxn(m_pushId);
    delete this;
  }

  void onError(const proxygen::HTTPException& error)
    noexcept override {
    Logger::Error("HPHP push txn transport error: %s",
                  error.describe().c_str());
    // Pushed transactions can't really have ingress errors
    m_egressError = true;
  }

  void onEgressPaused() noexcept override {}

  void onEgressResumed() noexcept override {}

 private:
  void createPushPromise(const char *host, const char *path,
                         uint8_t priority, const Array &headers,
                         bool isSecure) {
    m_pushPromise.setMethod(HTTPMethod::GET);
    m_pushPromise.setSecure(isSecure);
    m_pushPromise.setURL(path);
    m_pushPromise.setIsChunked(true); // implicitly chunked
    m_pushPromise.setPriority(priority);

    addHeadersToMessage(m_pushPromise, headers);
    m_pushPromise.getHeaders().set(HTTP_HEADER_HOST, host);
  }

  void addHeadersToMessage(HTTPMessage& message, const Array &headers) {
    for (ArrayIter iter(headers); iter; ++iter) {
      Variant key = iter.first();
      auto header = iter.second().toString();
      if (key.isString() && !key.toString().empty()) {
        message.getHeaders().add(key.toString().data(), header.data());
      } else {
        int pos = header.find(": ");
        if (pos >= 0) {
          std::string name = header.substr(0, pos).data();
          std::string value = header.substr(pos + 2).data();
          message.getHeaders().add(name, value);
        } else {
          Logger::Error("throwing away bad header: %s", header.data());
        }
      }
    }
  }

  uint64_t m_pushId;
  std::shared_ptr<ProxygenTransport> m_transport;
  proxygen::HTTPTransaction *m_pushTxn{nullptr};
  proxygen::HTTPMessage m_pushPromise;
  proxygen::HTTPMessage m_response;
  bool m_egressError{false};
};

///////////////////////////////////////////////////////////////////////////////

ProxygenTransport::ProxygenTransport(ProxygenServer *server)
  : m_server(server)
{
  m_server->addPendingTransport(*this);
}

ProxygenTransport::~ProxygenTransport() {
  VLOG(5) << "destroying ProxygenTransport " << (uint64_t) this;
  if (m_enqueued) {
    m_server->decrementEnqueuedCount();
  }
  {
    Lock lock(this);
    CHECK(m_pushHandlers.empty());
  }
  // Destructor of IntrusiveListHook will automatically unlink()
}

bool ProxygenTransport::bufferRequest() const {
  return (m_method != Transport::Method::POST ||
          RuntimeOption::RequestBodyReadLimit <= 0);
}

void ProxygenTransport::onHeadersComplete(
  unique_ptr<HTTPMessage> msg) noexcept {

  Timer::GetMonotonicTime(m_requestStart);
  m_request = std::move(msg);
  if (m_request->isSecure()) {
    setSSL();
  }
  m_request->dumpMessage(4);
  auto method = m_request->getMethod();
  const auto& methodStr = m_request->getMethodString();
  if (method == HTTPMethod::GET) {
    m_method = Transport::Method::GET;
  } else if (method == HTTPMethod::POST ||
             s_post_methods.find(methodStr) != s_post_methods.end()) {
    m_method = Transport::Method::POST;
  } else if (method == HTTPMethod::HEAD) {
    m_method = Transport::Method::HEAD;
  } else if (method == HTTPMethod::CONNECT) {
    sendErrorResponse(400 /* Bad Request */);
    return;
  } else {
    // looks like proxygen HTTP parser understands a few more methods
    // than libevent:
    //   TRACE, COPY, MOVE, MKACTIVITY, CHECKOUT, MERGE, MSEARCH, NOTIFY,
    //   SUBSCRIBE, UNSUBSCRIBE, PATCH
    sendErrorResponse(400 /* Bad Request */);
    return;
  }
  m_extended_method = methodStr.c_str();

  const auto& headers = m_request->getHeaders();
  headers.forEach([&] (const std::string &header, const std::string &val) {
      m_requestHeaders[header.c_str()].push_back(val.c_str());
    });

  if (m_method == Transport::Method::POST && m_request->isHTTP1_1()) {
    // fail fast if the post is too large, but only bother resolving host
    // if content_length is larger than the minimum setting.
    auto clen_str = getHeader("Content-Length");
    auto content_length = strtoll(clen_str.c_str(), nullptr, 10);
    auto max_post = RuntimeOption::LowestMaxPostSize;
    if (content_length > max_post) {
      auto host = headers.getSingleOrEmpty(HTTP_HEADER_HOST);
      if (auto vhost = VirtualHost::Resolve(host)) {
        max_post = vhost->getMaxPostSize();
      } else {
        max_post = RuntimeOption::MaxPostSize;
      }
    }
    auto post_too_big = false;
    if (content_length < 0 || content_length > max_post) {
      Logger::Warning("POST Content-Length of %lld bytes exceeds "
                      "the limit of %" PRId64 " bytes",
                      content_length, max_post);
      post_too_big = true;
    }
    const std::string& expectation =
      headers.getSingleOrEmpty(HTTP_HEADER_EXPECT);
    if (!expectation.empty()) {
      bool sendEom = false;
      HTTPMessage response;
      if (!boost::iequals(expectation, "100-continue")) {
        response.setStatusCode(417);
        response.setStatusMessage(HTTPMessage::getDefaultReason(417));
        response.getHeaders().add(HTTP_HEADER_CONNECTION, "close");
        sendEom = true;
      } else if (post_too_big) {
        // got expect 100-continue, but content_length is too big.
        response.setStatusCode(413 /* Payload Too Large */);
        response.setStatusMessage(HTTPMessage::getDefaultReason(413));
        response.getHeaders().add(HTTP_HEADER_CONNECTION, "close");
        sendEom = true;
      } else {
        response.setStatusCode(100);
        response.setStatusMessage(HTTPMessage::getDefaultReason(100));
      }
      response.setHTTPVersion(1, 1);
      response.dumpMessage(4);
      m_clientTxn->sendHeaders(response);
      if (sendEom) {
        m_responseCode = response.getStatusCode();
        m_responseCodeInfo = response.getStatusMessage();
        m_server->onRequestError(this);
        m_clientTxn->sendEOM();
        // this object is no longer valid
        return;
      }
    } else if (post_too_big) {
      // did not receive "expect" header, but too much post data.
      sendErrorResponse(413 /* Payload Too Large */);
      return;
    }
  }

  if (m_shouldRepost) {
    VLOG(2) << "Reposting transaction's completed receiving header,"
            << " beginning partial post";
    beginPartialPostEcho();
  }
  if (!bufferRequest()) {
    m_server->onRequest(shared_from_this());
  } // otherwise we wait for EOM
}

void ProxygenTransport::onBody(std::unique_ptr<folly::IOBuf> chain) noexcept {
  VLOG(4) << *m_clientTxn << "Recevied body len="
          << chain->computeChainDataLength();
  m_requestBodyLength += chain->computeChainDataLength();
  if (bufferRequest()) {
    CHECK(!m_enqueued);
    if (m_reposting) {
      VLOG(2) << "Reposting transaction " << *m_clientTxn
              << " received body bytes";
      if (m_clientTxn && !m_egressError) {
        VLOG(2) << "Reposting transaction " << *m_clientTxn
                << " is sending received body bytes";
        m_clientTxn->sendBody(std::move(chain));
      }
    } else {
      m_bodyData.append(std::move(chain));
    }
  } else {
    Lock lock(this);
    m_bodyData.append(std::move(chain));
    notify();
    if (m_bodyData.chainLength() >= RuntimeOption::RequestBodyReadLimit) {
      VLOG(4) << *m_clientTxn << "Buffer max reached, pausing ingress";
      m_clientTxn->pauseIngress();
    }
  }
}

void ProxygenTransport::onEOM() noexcept {
  VLOG(4) << *m_clientTxn << "received eom";
  if (bufferRequest()) {
    CHECK(!m_enqueued);
    if (m_reposting) {
      VLOG(2) << "Transaction " << *m_clientTxn << " is reposting";
      if (m_clientTxn && !m_egressError) {
        VLOG(2) << "Transaction " << *m_clientTxn
                << " is sending EOM for repost";
        m_clientTxn->sendEOM();
      }
      return;
    }
    if (!m_bodyData.empty()) {
      // TODO: let's remove gratuitous mallocs and copies please
      m_currentBodyBuf = m_bodyData.move();
      m_currentBodyBuf->coalesce();
      m_firstBody = true;
      VLOG(4) << *m_clientTxn << "m_currentBodyBuf.length=" <<
        m_currentBodyBuf->length();
    }
    m_clientComplete = true;
    m_server->onRequest(shared_from_this());
    return;
  } else {
    requestDoneLocking();
  }
}

const char *ProxygenTransport::getUrl() {
  return m_request->getURL().c_str();
}

const char *ProxygenTransport::getRemoteHost() {
  if (m_addressStr.empty()) {
    m_addressStr = m_clientAddress.getAddressStr();
  }
  return m_addressStr.c_str();
}

uint16_t ProxygenTransport::getRemotePort() {
  return m_clientAddress.getPort();
}

const void *ProxygenTransport::getPostData(size_t &size) {
  if (m_sendEnded) {
    size = 0;
    return 0;
  }

  // the API contract is that you can call getPostData repeatedly until
  // you call getMorePostData
  if (m_firstBody) {
    CHECK(m_currentBodyBuf);
    size = m_currentBodyBuf->length();
    return m_currentBodyBuf->data();
  }
  return getMorePostData(size);
}

bool ProxygenTransport::hasMorePostData() {
  if (bufferRequest()) {
    CHECK(m_clientComplete);
    return false;
  }
  Lock lock(this);
  // We have more post data if a) there's some in the queue or b) the client
  // EOM hasn't come yet
  bool result =  (m_method == Transport::Method::POST &&
                  (!m_bodyData.empty() || !m_clientComplete));
  VLOG(4) << "hasMorePostData=" << result;
  return result;
}

const void *ProxygenTransport::getMorePostData(size_t &size) {
  if (bufferRequest()) {
    CHECK(m_clientComplete);
    size = 0;
    return nullptr;
  }

  // proxygen will send onTimeout if we don't receive data in this much time
  long maxWait = RuntimeOption::ConnectionTimeoutSeconds;
  if (maxWait <= 0) {
    maxWait = 50; // this was the default read timeout in LibEventServer
  }
  Lock lock(this);
  while (m_bodyData.empty() && !m_clientComplete) {
    VLOG(4) << "waiting for POST data for maxWait=" << maxWait;
    wait(maxWait);
  }
  auto oldLength = m_bodyData.chainLength();

  // For chunk encodings, we way receive an EOM with no data, such that
  // hasMorePostData returns true (because client is not yet complete),
  // client sends EOM, getMorePostData should return 0/nullptr
  size = 0;
  const void *data = nullptr;
  while (!m_bodyData.empty()) {
    // this is the first body if it wasn't set and buf is unset
    m_firstBody = !(m_firstBody && m_currentBodyBuf);
    m_currentBodyBuf = m_bodyData.pop_front();
    CHECK(m_currentBodyBuf && m_currentBodyBuf->length() > 0);
    size = m_currentBodyBuf->length();
    data = m_currentBodyBuf->data();
    break;
  }
  // RequestBodyReadLimit is int64_t and could be -1
  if (oldLength >= RuntimeOption::RequestBodyReadLimit &&
      m_bodyData.chainLength() < RuntimeOption::RequestBodyReadLimit) {
    VLOG(4) << "resuming ingress";
    m_server->putResponseMessage(ResponseMessage(
        shared_from_this(), ResponseMessage::Type::RESUME_INGRESS));
  }
  VLOG(4) << "returning POST body chunk size=" << size;
  return data;
}

Transport::Method ProxygenTransport::getMethod() {
  return m_method;
}

bool ProxygenTransport::getClientComplete() {
  return m_clientComplete;
}

const char *ProxygenTransport::getExtendedMethod() {
  return m_extended_method;
}

std::string ProxygenTransport::getHTTPVersion() const {
  return m_request->getVersionString();
}

size_t ProxygenTransport::getRequestSize() const {
  return m_requestBodyLength; // header length isn't tracked
}

std::string ProxygenTransport::getHeader(const char *name) {
  assert(name && *name);

  HeaderMap::const_iterator iter = m_requestHeaders.find(name);
  if (iter != m_requestHeaders.end()) {
    return iter->second[0];
  }
  return "";
}

void ProxygenTransport::getHeaders(HeaderMap &headers) {
  if (&m_requestHeaders != &headers) {
    headers = m_requestHeaders;
  }
}

void ProxygenTransport::addHeaderImpl(const char *name, const char *value) {
  assert(name && *name);
  assert(value);

  if (m_sendStarted) {
    Logger::Error("trying to add header '%s: %s' after 1st chunk",
                  name, value);
    return;
  }

  m_response.getHeaders().add(name, value);
}

void ProxygenTransport::removeHeaderImpl(const char *name) {
  assert(name && *name);

  if (m_sendStarted) {
    Logger::Error("trying to remove header '%s' after 1st chunk", name);
    return;
  }

  m_response.getHeaders().remove(name);
}

void ProxygenTransport::addRequestHeaderImpl(const char *name,
                                             const char *value) {
  assert(name && *name);
  assert(value);

  m_request->getHeaders().add(name, value);
  m_requestHeaders[name].push_back(value);
}

void ProxygenTransport::removeRequestHeaderImpl(const char *name) {
  assert(name && *name);
  m_request->getHeaders().remove(name);
  m_requestHeaders.erase(name);
}

bool ProxygenTransport::isServerStopping() {
  return m_server->getStatus() == Server::RunStatus::STOPPED;
}

void ProxygenTransport::sendErrorResponse(uint32_t code) noexcept {
  HTTPMessage response;
  response.setHTTPVersion(1, 1);
  response.setStatusCode(code);
  response.setStatusMessage(HTTPMessage::getDefaultReason(code));
  response.getHeaders().add(HTTP_HEADER_CONNECTION, "close");

  CHECK(!m_sendStarted);
  m_sendStarted = true;
  m_sendEnded = true;
  m_responseCode = code;
  m_responseCodeInfo = response.getStatusMessage();
  m_server->onRequestError(this);

  CHECK(m_clientTxn && !m_egressError);
  m_clientTxn->sendHeaders(response);
  m_clientTxn->sendEOM();
}

void ProxygenTransport::onError(const HTTPException& err) noexcept {
  Logger::Error("HPHP transport error: %s", err.describe().c_str());

  if (err.hasProxygenError() &&
        err.getProxygenError() == ProxygenError::kErrorTimeout &&
        !m_egressError && m_clientTxn && m_clientTxn->canSendHeaders()) {
    sendErrorResponse(408 /* Request Timeout */);
  } else {
    // We either have an error here that we are not mapping to a http response
    // or m_clientTxn is in a state where it is not able to send response
    // headers (as part of a well formed response) and so we simply abort the
    // connection
    this->abort();

    // We could also check err.getDirection() to see if it's an egress error,
    // but sending abort here guarantees than any subsequent send* calls are
    // going to abort.
    m_egressError = true;
  }

  requestDoneLocking();
}

void ProxygenTransport::finish(shared_ptr<ProxygenTransport> &&transport) {
  m_server->putResponseMessage(ResponseMessage(std::move(transport)));
}

HTTPTransaction *ProxygenTransport::getTransaction(uint64_t id,
                                                   HTTPMessage **msg,
                                                   bool newPushOk) {
  if (id == 0) {
    if (m_egressError) {
      *msg = nullptr;
      return nullptr;
    }
    *msg = &m_response;
    return m_clientTxn;
  }
  Lock lock(this);
  auto it = m_pushHandlers.find(id);
  if (it == m_pushHandlers.end()) {
    *msg = nullptr;
    return nullptr;
  }
  // If the current transaction has already terminated in error, don't allow
  // a new push, but additional egress on an already created txn is OK.
  return it->second->getOrCreateTransaction(m_clientTxn, msg,
                                            newPushOk && !m_egressError);
}

void ProxygenTransport::messageAvailable(ResponseMessage&& message) {
  if (!m_clientTxn) {
    return;
  }

  HTTPMessage *msg = nullptr;
  bool newPushOk = (message.m_type == ResponseMessage::Type::HEADERS);
  HTTPTransaction *txn = getTransaction(message.m_pushId, &msg, newPushOk);
  if (!txn) {
    // client is gone, just eat the msg
    VLOG(4) << "client is gone, eating the msg";
    return;
  }
  switch (message.m_type) {
    case ResponseMessage::Type::HEADERS:
      CHECK(msg);
      txn->sendHeaders(*msg);
      if (!message.m_chunk && !message.m_eom) {
        break;
      } // else fall through
    case ResponseMessage::Type::BODY:
      if (message.m_chunk && m_method != Transport::Method::HEAD) {
        // TODO: experiment with disabling this chunked flag and letting
        // proxygen framework do the chunking
        if (message.m_chunked) {
          txn->sendChunkHeader(message.m_chunk->length());
          txn->sendBody(std::move(message.m_chunk));
          txn->sendChunkTerminator();
        } else {
          txn->sendBody(std::move(message.m_chunk));
        }
      }
      if (!message.m_eom) {
        break;
      } // else fall through
    case ResponseMessage::Type::EOM:
      txn->sendEOM();
      break;
    case ResponseMessage::Type::FINISH:
      // need to make sure the last reference is deleted in this thread

      // The VM thread is done, any outstanding push txns need aborts.
      // It's possible that we could drive pushes from somewhere other
      // than the VM thread, in which case we'll need a different saftey
      // mechanism
      {
        Lock lock(this);
        // Note that pushTxn->sendAbort() can remove pushTxn from
        // m_pushHandlers, so we need to be careful about how we
        // iterate m_pushHandlers.
        for (auto it = m_pushHandlers.begin(); it != m_pushHandlers.end(); ) {
          auto pushTxn = it++->second->getTransaction();
          if (pushTxn && !pushTxn->isEgressEOMSeen()) {
            LOG(ERROR) << "Aborting unfinished push txn=" << *pushTxn;
            pushTxn->sendAbort();
          }
        }
      }
      break;
    case ResponseMessage::Type::RESUME_INGRESS:
      txn->resumeIngress();
      break;
  }
}

void ProxygenTransport::sendImpl(const void *data, int size, int code,
                                 bool chunked, bool eom) {
  assert(data);
  assert(!m_sendStarted || chunked);
  if (m_sendEnded) {
    // This should never happen, but when it does we have to bail out,
    // since there's no sensible way to send data at this point and
    // trying to do so will horribly corrupt memory.
    // TODO #2821803: Figure out whether this is caused by another bug
    // somewhere.
    return;
  }

  VLOG(4) << "sendImpl called with data size=" << size << ", code=" << code
          << ", chunked=" << chunked << ", eom=" << eom;
  eom |= !chunked;
  if (!m_sendStarted) {
    if (!chunked) {
      if (!m_response.getHeaders().exists(HTTP_HEADER_CONTENT_LENGTH)) {
        m_response.getHeaders().add(HTTP_HEADER_CONTENT_LENGTH,
                                    folly::to<std::string>(size));
      }
    } else {
      // Explicitly add Transfer-Encoding: chunked here.  libproxygen will only
      // add it for keep-alive connections
      m_response.getHeaders().set(HTTP_HEADER_TRANSFER_ENCODING, "chunked");
    }
    m_response.setStatusCode(code);
    auto const& reasonStr = getResponseInfo();
    const char* reason = reasonStr.empty() ?
      HTTPMessage::getDefaultReason(code) : reasonStr.c_str();
    m_response.setStatusMessage(reason);
    m_response.setHTTPVersion(1, 1);
    m_response.setIsChunked(chunked);
    m_response.dumpMessage(4);
    m_server->putResponseMessage(
      ResponseMessage(shared_from_this(),
                      ResponseMessage::Type::HEADERS, 0,
                      chunked, data, size, eom));
    m_sendStarted = true;
  } else {
    m_server->putResponseMessage(
      ResponseMessage(shared_from_this(),
                      ResponseMessage::Type::BODY, 0,
                      chunked, data, size, eom));
  }

  if (eom) {
    m_sendEnded = true;
  }

  if (chunked) {
    /*
     * Chunked replies are sent async, so there is no way to know the
     * time it took to flush the response, but tracking the bytes sent is
     * very useful.
     */
    onChunkedProgress(size);
  }
  if (code >= 500) {
    s_requestErrorCount->addValue(1);
  } else {
    s_requestNonErrorCount->addValue(1);
  }
}

void ProxygenTransport::onSendEndImpl() {
  if (!m_sendEnded) {
    VLOG(4) << "onSendEndImpl called";
    m_server->putResponseMessage(
        ResponseMessage(shared_from_this(), ResponseMessage::Type::EOM));
    m_sendEnded = true;
  }
}

bool ProxygenTransport::supportsServerPush() {
  Lock lock(this);
  return (m_clientTxn &&
          m_clientTxn->supportsPushTransactions() &&
          isHTTP2CodecProtocol(
            m_clientTxn->getTransport().getCodec().getProtocol()) &&
          !m_sendEnded);
}

int64_t ProxygenTransport::pushResource(const char *host, const char *path,
                                        uint8_t priority,
                                        const Array &promiseHeaders,
                                        const Array &responseHeaders,
                                        const void *data, int size,
                                        bool eom) {
  if (!supportsServerPush()) {
    return 0;
  }

  int64_t pushId = m_nextPushId++;
  PushTxnHandler *handler = new PushTxnHandler(
    pushId, shared_from_this(),
    host, path, priority, promiseHeaders, responseHeaders,
    m_request && m_request->isSecure());
  {
    Lock lock(this);
    m_pushHandlers[pushId] = handler;
  }

  // Push Promise
  m_server->putResponseMessage(
    ResponseMessage(shared_from_this(), ResponseMessage::Type::HEADERS,
                    pushId));

  m_server->putResponseMessage(
    ResponseMessage(shared_from_this(),
                    ResponseMessage::Type::HEADERS,
                    pushId, false, data, size, eom));
  return pushId;
}

void ProxygenTransport::pushResourceBody(int64_t id, const void *data,
                                         int size, bool eom) {
  if (id == 0 || (size <= 0 && !eom)) {
    return;
  }
  m_server->putResponseMessage(
    ResponseMessage(shared_from_this(), ResponseMessage::Type::BODY,
                    id, false, data, size, eom));
}

void ProxygenTransport::beginPartialPostEcho() {
  VLOG(2) << "Beginning partial post";
  if (!bufferRequest() || m_reposting || !m_clientTxn || m_egressError
      || getClientComplete() || !m_clientTxn->canSendHeaders()) {
    VLOG(2) << "beginPartialPostEcho cannot proceed, "
            << "bufferRequest() = " << bufferRequest()
            << "m_reposting = " << m_reposting
            << "m_clientTxn = " << m_clientTxn
            << "m_egressError = " << m_egressError
            << "getClientComplete() = " << getClientComplete()
            << "canSendHeaders() = " << m_clientTxn->canSendHeaders();
    return;
  }
  VLOG(2) << "beginPartialPostEcho is proceeding, "
          << "bufferRequest() = " << bufferRequest()
          << "m_reposting = " << m_reposting
          << "m_clientTxn = " << m_clientTxn
          << "m_egressError = " << m_egressError
          << "getClientComplete() = " << getClientComplete()
          << "canSendHeaders() = " << m_clientTxn->canSendHeaders();
  CHECK(!m_enqueued);
  m_reposting = true;
  HTTPMessage response;
  response.setHTTPVersion(1,1);
  response.setIsChunked(true);
  response.setStatusCode(RuntimeOption::ServerPartialPostStatusCode);
  response.setStatusMessage("Partial post");

  // All of the clients headers should be retained downstream,
  // so all we need to echo back is the request body
  auto& headers = response.getHeaders();
  headers.add(HTTP_HEADER_TRANSFER_ENCODING, "chunked");

  m_clientTxn->sendHeaders(response);
  if (!m_bodyData.empty()) {
    VLOG(2) << "Reposting body bytes for client transaction " << *m_clientTxn;
    m_clientTxn->sendBody(m_bodyData.move());
  }
}

void ProxygenTransport::abort() {
  unlink();
  if (m_clientTxn) {
    m_clientTxn->sendAbort();
  }
  s_requestErrorCount->addValue(1);
}

///////////////////////////////////////////////////////////////////////////////
}
