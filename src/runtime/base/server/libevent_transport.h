/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __HTTP_SERVER_LIB_EVENT_TRANSPORT_H__
#define __HTTP_SERVER_LIB_EVENT_TRANSPORT_H__

#include <runtime/base/server/transport.h>
#include <evhttp.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class LibEventServer;
class LibEventTransport : public Transport {
public:
  LibEventTransport(LibEventServer *server, evhttp_request *request,
                    int workerId);

  /**
   * Implementing Transport...
   */
  virtual const char *getUrl();
  virtual const char *getRemoteHost();
  virtual const void *getPostData(int &size);
  virtual Method getMethod();
  virtual const char *getExtendedMethod();
  virtual std::string getHTTPVersion() const;
  virtual std::string getHeader(const char *name);
  virtual void getHeaders(HeaderMap &headers);
  virtual void addHeaderImpl(const char *name, const char *value);
  virtual void removeHeaderImpl(const char *name);
  virtual void addRequestHeaderImpl(const char *name, const char *value);
  virtual void removeRequestHeaderImpl(const char *name);
  virtual void sendImpl(const void *data, int size, int code, bool chunked);
  virtual void onSendEndImpl();
  virtual bool isServerStopping();

private:
  LibEventServer *m_server;
  evhttp_request *m_request;
  int m_workerId;
  std::string m_url;
  std::string m_remote_host;
  std::string m_http_version;
  Method m_method;
  const char *m_extended_method;
  HeaderMap m_requestHeaders;
  bool m_sendStarted;
  bool m_sendEnded;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HTTP_SERVER_LIB_EVENT_TRANSPORT_H__
