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

#ifndef incl_HPHP_RUNTIME_SERVER_FASTCGI_FASTCGI_TRANSPORT_H_
#define incl_HPHP_RUNTIME_SERVER_FASTCGI_FASTCGI_TRANSPORT_H_

#include "hphp/runtime/server/transport.h"
#include "hphp/util/synchronizable.h"

#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>

#include <type_traits>
#include <unordered_map>

namespace HPHP {

////////////////////////////////////////////////////////////////////////////////

/*
 * The FastCGI protocol transmits HTTP headers as key-value pairs in a stream
 * using the FCGI_PARAMS record. The keys represent mangled HTTP header strings
 * where _ replaces - and all characters are uppercased. Some of this header
 * information is directly queried by the VM through accessors on the transport
 * class. The table below contains the types and mappings for these keys and
 * is used to instantiate the getters on the transport class.
 */
#define FCGI_PROTOCOL_HEADERS                       \
  /* Request URI */                                 \
  O(REQUEST_URI,     RAW_STRING, getUrl)            \
  O(REMOTE_HOST,     RAW_STRING, getRemoteHost)     \
  O(REMOTE_ADDR,     RAW_STRING, getRemoteAddr)     \
  O(REMOTE_PORT,     UINT16,     getRemotePort)     \
  O(PATH_INFO,       STD_STRING, getPathInfo)       \
  /* Server headers */                              \
  O(SERVER_NAME,     RAW_STRING, getServerName)     \
  /* Protocol information */                        \
  O(REQUEST_METHOD,  RAW_STRING, getExtendedMethod) \
/**/

class FastCGISession;

/*
 * FastCGITransport is used to communicate between a PHP (VM) thread running a
 * FastCGI request and the FastCGISession representing the connection to
 * the client.
 *
 * The transport is at times accessed from the event-loop reading FastCGI
 * records, and the runloop executing the PHP virtual machine. As a result
 * it's important to understand the life-cycle of a transaction and the
 * concurrency contract that it has with both the VM and the session.
 *
 * ===== Transport life cycle =====
 *
 * The transport exists in the heap and is always wrapped in a shared pointer.
 * In general the transport is around for as long as any of the session, the
 * vm, or the server require access to it.
 *
 * Once the server calls onHeadersComplete() it guarantees that it will remain
 * live until the transport calls onComplete() and that it will only access the
 * transport via onBody() and onBodyComplete(). Once the transport calls
 * onComplete() the session may destroy itself and the transport will no longer
 * attempt to call any session callbacks.
 *
 * Once the session calls onBodyComplete() it will not make any further calls
 * to the transport, though it will remain available to receive calls to
 * onStdOut() and onComplete().
 *
 * ===== Transport concurrency =====
 *
 * While requests are executing it is important that the transport data remain
 * synchronized across calls from both the vm and the application server. When
 * no request is active the session is free to call reset() which will clear all
 * state data. During an active request the session will only write POST data
 * and will do so using the onBody() callback which will perform thread safe
 * writes to an internal buffer. Once onBodyComplete() is called the session
 * will not attempt to write further POST data until the request completes.
 *
 * The session callback onStdOut() is used to write data from the request out
 * to the webserver. It is always called from the VM event base; the session
 * is responsible for ensuring that the data is written in a thread-safe
 * fashion (generally by moving the data into the socket event base). The only
 * other session callback sent from the VM event base is onComplete() and it
 * do is handled appropriately in the session.
 *
 * Once the VM calls onSendEndImpl() it will cease transmit data to the remote
 * client. The transport may call onComplete() on the session and will stop
 * any remaining contact with the session.
 *
 * Transport CB           Session CB          Session Acc          VM Acc
 * [Constructor]          None                R/W                  None
 * onHeadersComplete()    None                W POST data          R/W
 * onBodyComplete()       None                None (remains live)  R/W
 * onSendEndImpl()        onComplete()        None (may destroy)   R/W
 */
struct FastCGITransport : public Transport, private Synchronizable {
  explicit FastCGITransport(FastCGISession* session) : m_session(session) {}
  virtual ~FastCGITransport() {}

  ///////////////////////////////////////////////////////////////////////////
  // FastCGISession callbacks
  //
  // The session uses these callbacks to populate the transport with request
  // data and inform the transport of the state of execution.
  //
  // onHeader() is always called when only the session thread is present which
  //            obviates the need for any sort of locking
  //
  // onHeadersComplete() is called when the final header is received but before
  //                     the request thread is started; it is the final chance
  //                     to populate internal structures from the session thread
  //
  // onBody() is called when the request is already in-flight as new POST data
  //          becomes available, any use of internal structures must be locked;
  //          only the POST data buffer can be written and no data can be read
  //
  // onBodyComplete() is called when no further ingress will occur; the vm may
  //                  be informed that the POST data is complete the next time
  //                  it queries for it; we will not be called from the session
  //                  thread again until we have called onComplete() on the
  //                  session, we may-however continue to call onStdOut to
  //                  write response data

  // Callbacks for new POST data (body == POST data)- appends data to a queue
  // to be processed by the executing script.
  void onBody(std::unique_ptr<folly::IOBuf> chain);
  void onBodyComplete();

  // Similar callbacks for receiving request headers
  void onHeader(std::unique_ptr<folly::IOBuf> key_chain,
                std::unique_ptr<folly::IOBuf> value_chain);
  void onHeadersComplete();

  ///////////////////////////////////////////////////////////////////////////
  // Transport implementation
  //
  // These are callbacks used by the VM to access information about the request;
  // they won't be called until onHeadersComplete() has returned, until such
  // time, the data they access need not be available. Once onSendEndImpl has
  // been called the VM will stop accessing data until onHeadersComplete() is
  // called again.

  void addHeaderImpl(const char *name, const char *value) override;
  void removeHeaderImpl(const char *name) override;
  void sendResponseHeaders(folly::IOBufQueue& queue, int code);

  void sendImpl(const void *data,
                int size,
                int code,
                bool chunked,
                bool eom) override;
  void onSendEndImpl() override;

  // POST request data
  const void* getPostData(int& size) override;
  const void* getMorePostData(int& size) override;
  bool hasMorePostData() override;

  // HEADER data
  std::string getHeader(const char* name) override;          // unmangled name
  void getHeaders(HeaderMap& headers) override;              // HTTP headers
  void getTransportParams(HeaderMap& serverParams) override; // FCGI parameters

  // Modified properties
  // These paramaters are also passed as FCGI_PARAMS but require modifications
  // which cause them to differ from their original parameters
  const std::string getDocumentRoot()   override { return m_docRoot; }
  const std::string getScriptFilename() override { return m_scriptName; }
  const std::string getPathTranslated() override { return m_pathTrans; }

  // Derived properties
  // These parameters were not part of the original FCGI_PARAMS record but are
  // derived from values therein
  Method getMethod()            override { return m_method; }
  const char* getServerObject() override { return m_serverObject.c_str(); }
  bool isPathInfoSet() override { return m_requestParams.count("PATH_INFO"); }

  // Properties with default values
  // These parameters are overrides of the values on transport if present
  const std::string& getServerAddr() override {
    auto const str = getParamTyped<const std::string*>("SERVER_ADDR");
    return str ? *str : Transport::getServerAddr();
  }

  uint16_t getServerPort() override {
    auto port = getParamTyped<uint16_t>("SERVER_PORT");
    return port ? port : Transport::getServerPort();
  }

  const char* getServerSoftware() override {
    auto str = getParamTyped<const char*>("SERVER_SOFTWARE");
    return *str ? str : Transport::getServerSoftware();
  }

  std::string getHTTPVersion() const override {
    auto str = getParamTyped<std::string>("HTTP_VERSION");
    return !str.empty() ? str : Transport::getHTTPVersion();
  }

  // Request parameter getters
  // These properties can be extracted directly from the request parameters
  int getRequestSize() const override {
    return getParamTyped<int>("CONTENT_LENGTH");
  }

#define RAW_STRING const char*
#define STD_STRING const std::string
#define UINT16 uint16_t
#define O(param, type, method)          \
  type method() override {              \
    return getParamTyped<type>(#param); \
  }
  FCGI_PROTOCOL_HEADERS
#undef RAW_STRING
#undef STD_STRING
#undef O
#undef FCGI_PROTOCOL_HEADERS

private:
  ///////////////////////////////////////////////////////////////////////////
  // Header manipulation and lookup
  //
  // Headers come from FastCGI webservers via the FCGI_PARAMS record in mangled
  // form and need to be transfomred into their more familiar HTTP header form
  // before passing them into the VM.
  //
  // (mangled) HTTP_IF_UNMODIFIED_SINCE <-> If-Unmodified-Since (normal)

  std::string unmangleHeader(const std::string& name) const;

  ///////////////////////////////////////////////////////////////////////////
  // Request/response state
  //
  // The transport is basically a bag of data and state. Everything here is
  // either request data or response state. Once the request is sent and the
  // onSendEndImpl() callback is triggered we are free to reset ourselves for
  // another request.

  // Session for communicating with the webserver
  FastCGISession* m_session;

  // POST request data
  // NB: m_bodyQueue is the only field session can write while the request is
  //     being processed
  folly::IOBufQueue m_bodyQueue;            // unprocessed POST data
  std::unique_ptr<folly::IOBuf> m_currBody; // POST data last returned to VM
  bool m_firstBody{false};
  bool m_bodyComplete{false};

  // Response headers
  HeaderMap m_responseHeaders;
  bool m_headersSent{false}; // set once headers have been transmitted and may
                             // no longer be changed
  bool m_sendEnded{false};   // onComplete has been sent already

  // Request parameters
  std::string m_docRoot;
  std::string m_scriptName;
  std::string m_pathTrans;
  std::string m_serverObject;
  Method m_method{Method::Unknown};
  std::unordered_map<std::string, std::string> m_requestParams;

  folly::IOBufQueue m_txBuf; // buffer for sending messages

  /*
   * Convenient templates for extracting typed header information.
   */
  template<typename T>
  typename std::enable_if<std::is_integral<T>::value, T>
  ::type getParamTyped(const char* key) const {
    auto pos = m_requestParams.find(key);
    if (pos == m_requestParams.end()) return 0;

    try {
      return folly::to<T>(pos->second);
    } catch (std::range_error&) {
      return 0;
    }
  }

  template<typename T>
  typename std::enable_if<
    std::is_same<typename std::remove_cv<T>::type, std::string>::value,
    std::string
  >::type getParamTyped(const char* key) const {
    auto pos = m_requestParams.find(key);
    if (pos == m_requestParams.end()) return "";

    return pos->second;
  }

  template<typename T>
  typename std::enable_if<
    std::is_same<T, const std::string*>::value,
    const std::string*
  >::type getParamTyped(const char* key) const {
    auto pos = m_requestParams.find(key);
    if (pos == m_requestParams.end()) return nullptr;

    return &pos->second;
  }

  template<typename T>
  typename std::enable_if<
    std::is_same<typename std::remove_cv<T>::type, const char*>::value,
    const char*
  >::type getParamTyped(const char* key) const {
    auto pos = m_requestParams.find(key);
    if (pos == m_requestParams.end()) return "";

    // This is safe because unordered_map only invalidates pointers that have
    // been erased.
    return pos->second.c_str();
  }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
