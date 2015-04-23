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

#ifndef incl_HPHP_PROXYGEN_SERVER_TRANSPORT_H_
#define incl_HPHP_PROXYGEN_SERVER_TRANSPORT_H_

#include "hphp/runtime/server/transport.h"
#include <algorithm>
#include <memory>
#include "hphp/util/logger.h"
#include "hphp/util/lock.h"
#include "hphp/util/synchronizable.h"
#include <proxygen/lib/http/session/HTTPTransaction.h>
#include <thrift/lib/cpp/async/TAsyncTransport.h>
#include <thrift/lib/cpp/async/TAsyncTimeout.h>
#include <folly/IPAddress.h>

namespace HPHP {
class ProxygenServer;
class ProxygenTransport;

////////////////////////////////////////////////////////////////////////////////
/** Message passed from dispatch thread to I/O thread
 *
 * These messages hold a reference to the transport so it doesn't get deleted
 * out from under them in transit.
 */
class ResponseMessage {
  public:
  enum class Type {
    HEADERS,
    BODY,
    EOM,
    FINISH,
    RESUME_INGRESS, // a bit of a hack to have this here
  };

  explicit ResponseMessage(std::shared_ptr<ProxygenTransport> transport,
                           Type t, uint64_t pushId = 0, bool chunked = true,
                           const void *data = nullptr, int size = 0,
                           bool eom = false)
    : m_transport(transport),
      m_type(t),
      m_pushId(pushId),
      m_chunked(chunked),
      m_eom(eom) {
    if (size > 0 && (m_type == Type::BODY || m_type == Type::HEADERS)) {
        // sad panda copy.  TODO (t4362832): change sendImpl to take IOBuf
        m_chunk = std::move(folly::IOBuf::copyBuffer(data, size));
      }
    };

  ResponseMessage(ResponseMessage&& m) noexcept
  : m_transport(std::move(m.m_transport)),
    m_type(m.m_type),
    m_pushId(m.m_pushId),
    m_chunked(m.m_chunked),
    m_chunk(std::move(m.m_chunk)),
    m_eom(m.m_eom) {}

  explicit ResponseMessage(std::shared_ptr<ProxygenTransport> &&transport)
      : m_transport(std::move(transport)),
        m_type(Type::FINISH) {}

  std::shared_ptr<ProxygenTransport> m_transport;
  Type m_type;
  uint64_t m_pushId{0};
  bool m_chunked{false};
  std::unique_ptr<folly::IOBuf> m_chunk;
  bool m_eom{false};
};

class PushTxnHandler;

///////////////////////////////////////////////////////////////////////////////

/**
 * A class defining an interface that request handler can use to query
 * transport related information.
 *
 * Note that one transport object is created for each request.  The transport
 * accessed by the I/O thread and dispatch thread, but it should be OK, right?
 */
class ProxygenTransport : public Transport,
  public proxygen::HTTPTransactionHandler,
  public std::enable_shared_from_this<ProxygenTransport>,
  public apache::thrift::async::TAsyncTimeout,
  public Synchronizable {
public:
  explicit ProxygenTransport(ProxygenServer *server,
                             folly::EventBase *eventBase);
  virtual ~ProxygenTransport();

  ///////////////////////////////////////////////////////////////////////////
  // Functions sub-classes have to implement.

  const std::string& getServerAddr() override {
    return m_localAddr;
  };
  uint16_t getServerPort() override {
    return m_localPort;
  };

  /**
   * Request URI.
   */
  virtual const char *getUrl();
  virtual const char *getRemoteHost();
  virtual uint16_t getRemotePort();

  /**
   * POST request's data.
   */
  virtual const void *getPostData(int &size);
  virtual bool hasMorePostData();
  virtual const void *getMorePostData(int &size);

  // TODO: is get getFiles required?

  /**
   * Is this a GET, POST or anything?
   */
  virtual Method getMethod();
  virtual const char *getExtendedMethod();

  /**
   * What version of HTTP was the request?
   */
  virtual std::string getHTTPVersion() const;

  /**
   * Get http request size.
   */
  virtual int getRequestSize() const;

  /**
   * Get request header(s).
   */
  virtual std::string getHeader(const char *name);
  virtual void getHeaders(HeaderMap &headers);

  /**
   * Add/remove a response header.
   */
  virtual void addHeaderImpl(const char *name, const char *value);
  virtual void removeHeaderImpl(const char *name);

  /**
   * Add/remove a request header. Default is no-op, because not all transports
   * need to support incoming request header manipulations.
   */
  virtual void addRequestHeaderImpl(const char *name, const char *value);
  virtual void removeRequestHeaderImpl(const char *name);

  /**
   * Send back a response with specified code.
   * Caller deletes data, callee must copy
   */
  virtual void sendImpl(const void *data, int size, int code,
                        bool chunked, bool eom);

  /**
   * Override to implement more send end logic.
   */
  virtual void onSendEndImpl();

  virtual bool supportsServerPush();

  virtual int64_t pushResource(const char *host, const char *path,
                               uint8_t priority, const Array &headers,
                               const void *data, int size, bool eom);

  virtual void pushResourceBody(int64_t id,
                                const void *data, int size, bool eom);

  /**
   * Need this implementation to break keep-alive connections.
   */
  virtual bool isServerStopping();

  void finish(std::shared_ptr<ProxygenTransport> &&transport);

  // TODO: are isUploadedFile/moveUploadedFile required to support rfc1867?

  // TODO: implement IDebuggable

  struct timespec getRequestStart() const {
    return m_requestStart;
  }

  void setTransactionReference(const std::shared_ptr<ProxygenTransport>& t) {
    // This object holds a reference on behalf of the HTTPTransaction, because
    // that API only knows about naked pointers
    m_transactionReference = t;
  }

  // HTTPTransactionHandler interface
  virtual void setTransaction(proxygen::HTTPTransaction* txn)
    noexcept {
    {
      Lock lock(this);
      m_clientTxn = txn;
    }
    m_clientTxn->getPeerAddress(m_clientAddress);
    folly::SocketAddress localAddr;
    m_clientTxn->getLocalAddress(localAddr);
    folly::IPAddress ipAddr(localAddr.getIPAddress());
    m_localAddr = ipAddr.toFullyQualified();
    m_localPort = localAddr.getPort();
  };

  virtual void detachTransaction() noexcept {
    VLOG(5) << "detachTransaction ProxygenTransport=" << (uint64_t) this;
    {
      Lock lock(this);
      m_clientTxn = nullptr;
    }
    m_transactionReference.reset();
  }

  virtual void onHeadersComplete(
    std::unique_ptr<proxygen::HTTPMessage> msg) noexcept;

  virtual void onBody(std::unique_ptr<folly::IOBuf> chain) noexcept;

  virtual void onChunkHeader(size_t length) noexcept {};

  virtual void onChunkComplete() noexcept {};

  virtual void onTrailers(
    std::unique_ptr<proxygen::HTTPHeaders> trailers) noexcept {
    Logger::Error("HPHP ate the trailers");
  }

  virtual void onUpgrade(
      proxygen::UpgradeProtocol protocol) noexcept {
    Logger::Error("HPHP received upgrade");
  }

  /**
   * After this callback is received, there will be no more normal ingress
   * callbacks received (onEgress*(), onError(), and onTimeout() may still
   * be invoked). The Handler should consider ingress complete after
   * receiving this message. This Transaction is still valid, and work
   * may still occur on it until detachTransaction is called.
   */
  virtual void onEOM() noexcept;

  // error functions always use requestDoneLocking in case we are on
  // the POST path
  virtual void onError(const proxygen::HTTPException& error) noexcept;

  virtual void onEgressPaused() noexcept {}

  virtual void onEgressResumed() noexcept {}

  void messageAvailable(ResponseMessage&& message);

  void timeoutExpired() noexcept;

  void removePushTxn(uint64_t id) {
    Lock lock(this);
    m_pushHandlers.erase(id);
  }

 private:
  bool bufferRequest() const;

  void sendErrorResponse(uint32_t code) noexcept;

  void requestDoneLocking() {
    Lock lock(this);
    m_clientComplete = true;
    notify();
  }

  proxygen::HTTPTransaction* getTransaction(
    uint64_t id, proxygen::HTTPMessage **msg, bool newPushOk);

  // Tracks HTTPTransaction's reference to this object
  std::shared_ptr<ProxygenTransport> m_transactionReference;
  ProxygenServer *m_server;
  folly::EventBase *m_eventBase;
  proxygen::HTTPTransaction *m_clientTxn{nullptr}; // locked
  folly::SocketAddress m_clientAddress;
  std::string m_addressStr;
  std::unique_ptr<proxygen::HTTPMessage> m_request;
  size_t m_requestBodyLength{0};

  // There are two modes of operation for reading POST bodies.  When
  // RequestBodyReadLimit is set, the request is enqueued for a worker thread
  // as soon as the headers are received.  The body is read from the network
  // concurrently with the code to process the body.  In this case m_bodyData
  // and m_clientComplete are protected by a lock.  Also note that the worker
  // thread can block waiting for POST data in this case.
  //
  // When RequestBodyReadLimit is not set, the request is not enqueued for
  // a worker until the entire body has been read, so only one thread will
  // access m_bodyData and m_clientComplete at one time.
  folly::IOBufQueue m_bodyData{folly::IOBufQueue::cacheChainLength()};
  bool m_clientComplete{false};

  bool m_firstBody{false};
  bool m_enqueued{false};
  std::unique_ptr<folly::IOBuf> m_currentBodyBuf;
  proxygen::HTTPMessage m_response;
  bool m_sendStarted{false};
  Method m_method{Method::GET};
  const char *m_extended_method{nullptr};
  HeaderMap m_requestHeaders;
  struct timespec m_requestStart;
  std::string m_localAddr;
  uint16_t m_localPort{0};
  int64_t m_nextPushId{1};
  std::map<uint64_t, PushTxnHandler*> m_pushHandlers; // locked
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
