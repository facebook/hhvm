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

#ifndef incl_HPHP_FAKE_TRANSPORT_H_
#define incl_HPHP_FAKE_TRANSPORT_H_

#include "hphp/runtime/server/transport.h"

namespace HPHP {

/**
 * Fake Transport to be passed to the access log when a real transport is not
 * available
 */
class FakeTransport : public Transport {
public:
  std::string m_url;
  std::string m_remoteHost;
  uint16_t m_remotePort{0};
  int m_requestSize{0};
  Method m_method{Method::GET};
  std::string m_extended_method;
  std::string m_httpVersion{"1.1"};

  explicit FakeTransport(uint16_t code) {
    m_responseCode = code;
  }

  /**
   * Request URI.
   */
  virtual const char *getUrl() { return m_url.c_str(); }
  virtual const char *getRemoteHost() { return m_remoteHost.c_str(); }
  virtual uint16_t getRemotePort() { return m_remotePort; }
  virtual int getRequestSize() const { return m_requestSize; }

  /**
   * POST request's data.
   */
  virtual const void *getPostData(int &size) {
    LOG(FATAL) << "FakeTransport::getPostData";
    size = 0;
    return nullptr;
  }

  virtual Method getMethod() { return m_method; }
  virtual const char *getExtendedMethod() { return m_extended_method.c_str();}

  /**
   * What version of HTTP was the request?
   */
  virtual std::string getHTTPVersion() const {
    return m_httpVersion.c_str();
  }

  /**
   * Get request header(s).
   */
  virtual std::string getHeader(const char *name) {
    return "";
  };
  virtual void getHeaders(HeaderMap &headers) {
    LOG(FATAL) << "FakeTransport::getHeaders";
  }
  /**
   * Add/remove a response header.
   */
  virtual void addHeaderImpl(const char *name, const char *value) {
    LOG(FATAL) << "FakeTransport::addHeaderImpl";
  };
  virtual void removeHeaderImpl(const char *name) {
    LOG(FATAL) << "FakeTransport::removeHeaderImpl";
  }

  /**
   * Send back a response with specified code.
   * Caller deletes data, callee must copy
   */
  virtual void sendImpl(const void *data, int size, int code,
                        bool chunked, bool eom) {
    LOG(FATAL) << "FakeTransport::sendImpl";
  };

};

}
#endif
