/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include <folly/IntrusiveList.h>
#include <folly/IPAddress.h>

namespace HPHP {
struct ProxygenServer;
struct ProxygenTransport;

////////////////////////////////////////////////////////////////////////////////
/** Message passed from dispatch thread to I/O thread
 *
 * These messages hold a reference to the transport so it doesn't get deleted
 * out from under them in transit.
 */
struct ResponseMessage {
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
        m_chunk = folly::IOBuf::copyBuffer(data, size);
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

struct PushTxnHandler;

///////////////////////////////////////////////////////////////////////////////

/**
 * A class defining an interface that request handler can use to query
 * transport related information.
 *
 * Note that one transport object is created for each request.  The transport
 * accessed by the I/O thread and dispatch thread, but it should be OK, right?
 */
struct ProxygenTransport final
  : Transport
  , proxygen::HTTPTransactionHandler
  , std::enable_shared_from_this<ProxygenTransport>
  , Synchronizable
{
  explicit ProxygenTransport(ProxygenServer *server);
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
  const char *getUrl() override;
  const char *getRemoteHost() override;
  uint16_t getRemotePort() override;

  /**
   * POST request's data.
   */
  const void *getPostData(size_t &size) override;
  bool hasMorePostData() override;
  const void *getMorePostData(size_t &size) override;

  // TODO: is get getFiles required?

  /**
   * Is this a GET, POST or anything?
   */
  Method getMethod() override;
  const char *getExtendedMethod() override;

  /**
   * What version of HTTP was the request?
   */
  std::string getHTTPVersion() const override;

  /**
   * Has the client received an EOM yet?
   */
  bool getClientComplete();

  /**
   * Get http request size.
   */
  size_t getRequestSize() const override;

  /**
   * Get request header(s).
   */
  std::string getHeader(const char *name) override;
  void getHeaders(HeaderMap &headers) override;

  /**
   * Add/remove a response header.
   */
  void addHeaderImpl(const char *name, const char *value) override;
  void removeHeaderImpl(const char *name) override;

  /**
   * Add/remove a request header. Default is no-op, because not all transports
   * need to support incoming request header manipulations.
   */
  void addRequestHeaderImpl(const char *name, const char *value) override;
  void removeRequestHeaderImpl(const char *name) override;

  /**
   * Send back a response with specified code.
   * Caller deletes data, callee must copy
   */
  void sendImpl(const void *data, int size, int code,
                bool chunked, bool eom) override;

  /**
   * Override to implement more send end logic.
   */
  void onSendEndImpl() override;

  bool supportsServerPush() override;

  int64_t pushResource(const char *host, const char *path,
                       uint8_t priority, const Array &headers,
                       const void *data, int size, bool eom) override;

  void pushResourceBody(int64_t id,
                        const void *data, int size, bool eom) override;

  /**
   * Need this implementation to break keep-alive connections.
   */
  bool isServerStopping() override;

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
  void setTransaction(proxygen::HTTPTransaction* txn) noexcept override {
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

  void detachTransaction() noexcept override {
    VLOG(5) << "detachTransaction ProxygenTransport=" << (uint64_t) this;
    {
      Lock lock(this);
      m_clientTxn = nullptr;
    }
    m_transactionReference.reset();
  }

  void onHeadersComplete(
    std::unique_ptr<proxygen::HTTPMessage> msg) noexcept override;

  void onBody(std::unique_ptr<folly::IOBuf> chain) noexcept override;

  void onChunkHeader(size_t length) noexcept override {};

  void onChunkComplete() noexcept override {};

  void onTrailers(
    std::unique_ptr<proxygen::HTTPHeaders> trailers) noexcept override {
    Logger::Error("HPHP ate the trailers");
  }

  void onUpgrade(proxygen::UpgradeProtocol protocol) noexcept override {
    Logger::Error("HPHP received upgrade");
  }

  /**
   * After this callback is received, there will be no more normal
   * ingress callbacks received (onEgress*() and onError() may still
   * be invoked). The Handler should consider ingress complete after
   * receiving this message. This Transaction is still valid, and work
   * may still occur on it until detachTransaction is called.
   */
  void onEOM() noexcept override;

  // error functions always use requestDoneLocking in case we are on
  // the POST path
  void onError(const proxygen::HTTPException& error) noexcept override;

  void onEgressPaused() noexcept override { }

  void onEgressResumed() noexcept override { }

  void messageAvailable(ResponseMessage&& message);

  void beginPartialPostEcho();
  /**
   * The transaction is aborted when there are errors, or during
   * shutdown when the server has stopped enqueuing requests, or
   * when the post request takes too long before we see EOM during
   * server shutdown.
   */
  void abort();

  void removePushTxn(uint64_t id) {
    Lock lock(this);
    m_pushHandlers.erase(id);
  }

  void setEnqueued() {
    m_enqueued = true;
    unlink();
  }

 private:
  bool bufferRequest() const;

  void unlink() {
    m_listHook.unlink();
  }

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
  // Set to true when sending a partial post back to
  // the slb due to impending server death
  bool m_repost{false};
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

 public:
  // List of ProxygenTransport not yet handed to the server will sit
  // in a list, so that we can abort them if they take too long.
  // Every ProxygenTransport is in the list initially, and gets
  // unlinked when it is enqueued or aborted, or destructed if it is
  // never handed to the server.  m_enqueued implies !is_linked(), but
  // aborted ones can have !m_enqueued && !is_linked().
  folly::IntrusiveListHook m_listHook;

  // Initialized upon ProxygenServer creation.  These counters work at Transport
  // level and include requests that are dropped before getting executed.
  static ServiceData::ExportedTimeSeries* s_requestErrorCount;
  static ServiceData::ExportedTimeSeries* s_requestNonErrorCount;
};

using ProxygenTransportList =
  folly::IntrusiveList<ProxygenTransport, &ProxygenTransport::m_listHook>;

///////////////////////////////////////////////////////////////////////////////
}

#endif
