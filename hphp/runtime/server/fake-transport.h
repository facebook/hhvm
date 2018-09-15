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

#ifndef incl_HPHP_FAKE_TRANSPORT_H_
#define incl_HPHP_FAKE_TRANSPORT_H_

#include "hphp/runtime/server/transport.h"

namespace HPHP {

const StaticString s_fake("fake");

/**
 * Fake Transport to be passed to the access log when a real transport is not
 * available
 */
struct FakeTransport final : Transport {
  std::string m_url;
  std::string m_remoteHost;
  uint16_t m_remotePort{0};
  size_t m_requestSize{0};
  Method m_method{Method::GET};
  std::string m_extended_method;
  std::string m_httpVersion{"1.1"};

  explicit FakeTransport(uint16_t code) {
    m_responseCode = code;
  }

  /**
   * Request URI.
   */
  const char *getUrl() override { return m_url.c_str(); }
  const char *getRemoteHost() override { return m_remoteHost.c_str(); }
  uint16_t getRemotePort() override { return m_remotePort; }
  size_t getRequestSize() const override { return m_requestSize; }

  /**
   * POST request's data.
   */
  const void *getPostData(size_t &size) override {
    LOG(FATAL) << "FakeTransport::getPostData";
    size = 0;
    return nullptr;
  }

  Method getMethod() override { return m_method; }
  const char *getExtendedMethod() override { return m_extended_method.c_str();}

  /**
   * What version of HTTP was the request?
   */
  std::string getHTTPVersion() const override {
    return m_httpVersion.c_str();
  }

  /**
   * Get request header(s).
   */
  std::string getHeader(const char* /*name*/) override { return ""; };
  void getHeaders(HeaderMap& /*headers*/) override {
    LOG(FATAL) << "FakeTransport::getHeaders";
  }
  /**
   * Add/remove a response header.
   */
  void addHeaderImpl(const char* /*name*/, const char* /*value*/) override {
    LOG(FATAL) << "FakeTransport::addHeaderImpl";
  };
  void removeHeaderImpl(const char* /*name*/) override {
    LOG(FATAL) << "FakeTransport::removeHeaderImpl";
  }
  /**
   * Get a description of the type of transport.
   */
  String describe() const override {
    return s_fake;
  }


  /**
   * Send back a response with specified code.
   * Caller deletes data, callee must copy
   */
  void sendImpl(const void* /*data*/, int /*size*/, int /*code*/,
                bool /*chunked*/, bool /*eom*/) override {
    LOG(FATAL) << "FakeTransport::sendImpl";
  };

};

}
#endif
