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
#include "hphp/runtime/server/fastcgi/protocol-session-handler.h"
#include "folly/io/IOBuf.h"
#include "folly/io/IOBufQueue.h"
#include "thrift/lib/cpp/async/TAsyncTransport.h"
#include "thrift/lib/cpp/async/TAsyncTimeout.h"
#include "thrift/lib/cpp/transport/TSocketAddress.h"
#include "thrift/lib/cpp/concurrency/Monitor.h"

#include <map>
#include <vector>

namespace HPHP {

class FastCGITransport;

////////////////////////////////////////////////////////////////////////////////

class FastCGIConnection;

class FastCGITransport
  : public Transport,
    public ProtocolSessionHandler {

public:
  explicit FastCGITransport(FastCGIConnection *connection, int id);
  virtual ~FastCGITransport() {}

  virtual const char *getUrl() override;
  virtual const char *getRemoteHost() override;
  virtual const char *getRemoteAddr() override;
  virtual uint16_t getRemotePort() override;
  virtual const std::string getPathTranslated() override;
  virtual const std::string getDocumentRoot() override;
  virtual const char *getServerName() override;
  virtual const char *getServerAddr() override;
  virtual uint16_t getServerPort() override;
  virtual const char* getServerSoftware() override;

  virtual const void *getPostData(int &size) override;
  virtual bool hasMorePostData() override;
  virtual const void *getMorePostData(int &size) override;

  virtual Method getMethod() override;
  virtual const char *getExtendedMethod() override;

  virtual std::string getHTTPVersion() const override;

  virtual int getRequestSize() const override;

  virtual const char *getServerObject() override;

  virtual std::string getHeader(const char *name) override;
  virtual void getHeaders(HeaderMap &headers) override;
  virtual void getTransportParams(HeaderMap &serverParams) override;

  virtual void addHeaderImpl(const char *name, const char *value) override;
  virtual void removeHeaderImpl(const char *name) override;

  virtual void sendImpl(const void *data,
                        int size,
                        int code,
                        bool chunked) override;
  void sendResponseHeaders(folly::IOBufQueue& queue, int code);
  virtual void onSendEndImpl() override;

  // Implementing ProtocolSessionHandler
  virtual void onBody(std::unique_ptr<folly::IOBuf> chain) override;
  virtual void onBodyComplete() override;
  virtual void onHeader(std::unique_ptr<folly::IOBuf> key_chain,
                       std::unique_ptr<folly::IOBuf> value_chain) override;
  virtual void onHeadersComplete() override;

private:
  using ResponseHeaders =
    std::unordered_map<std::string, std::vector<std::string>>;

  std::string getRawHeader(const std::string& name);
  std::string* getRawHeaderPtr(const std::string& name);
  int getIntHeader(const std::string& name);

  /*
   * HTTP_IF_MODIFIED_SINCE -> If-Unmodified-Since
   */
  std::string unmangleHeader(const std::string& name);
  /*
   * If-Unmodified-Since -> HTTP_IF_MODIFIED_SINCE
   */
  std::string mangleHeader(const std::string& name);

  const void *getPostDataImpl(int &size, bool progress);

  FastCGIConnection* m_connection;
  int m_id;
  folly::IOBufQueue m_bodyQueue;
  std::unique_ptr<folly::IOBuf> m_currBody;
  std::unordered_map<std::string, std::string> m_requestHeaders;
  std::string m_pathTranslated;
  std::string m_requestURI;
  std::string m_documentRoot;
  std::string m_remoteHost;
  std::string m_remoteAddr;
  uint16_t m_remotePort;
  std::string m_serverName;
  std::string m_serverAddr;
  std::string m_serverSoftware;
  uint16_t m_serverPort;
  Method m_method;
  std::string m_extendedMethod;
  std::string m_httpVersion;
  std::string m_serverObject;
  size_t m_requestSize;
  ResponseHeaders m_responseHeaders;
  bool m_headersSent;
  bool m_readMore;

  apache::thrift::concurrency::Monitor m_monitor;
  int m_waiting;
  bool m_readComplete;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
