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

#include "hphp/runtime/base/url-file.h"
#include <vector>
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/ext/pcre/ext_pcre.h"
#include "hphp/runtime/ext/stream/ext_stream.h"
#include "hphp/runtime/ext/url/ext_url.h"
#include "hphp/runtime/base/php-globals.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

const StaticString s_http_response_header("http_response_header");
const StaticString s_http("http");
const StaticString s_tcp_socket("tcp_socket");

///////////////////////////////////////////////////////////////////////////////

UrlFile::UrlFile(const char *method /* = "GET" */,
                 const Array& headers /* = null_array */,
                 const String& postData /* = null_string */,
                 int maxRedirect /* = 20 */,
                 int timeout /* = -1 */,
                 bool ignoreErrors /* = false */)
                 : MemFile(s_http, s_tcp_socket) {
  m_get = (method == nullptr || strcasecmp(method, "GET") == 0);
  m_method = method;
  m_headers = headers;
  m_postData = postData;
  m_maxRedirect = maxRedirect;
  m_timeout = timeout;
  m_ignoreErrors = ignoreErrors;
  setIsLocal(false);
}

void UrlFile::sweep() {
  using std::string;
  m_error.~string();
  MemFile::sweep();
}

const StaticString
  s_remove_user_pass_pattern("#://[^@]+@#"),
  s_remove_user_pass_replace("://");

void UrlFile::setProxy(const String& proxy_host, int proxy_port,
                       const String& proxy_user, const String& proxy_pass) {
  m_proxyHost = proxy_host.c_str();
  m_proxyPort = proxy_port;
  m_proxyUsername = proxy_user.c_str();
  m_proxyPassword = proxy_pass.c_str();
}

bool UrlFile::open(const String& input_url, const String& mode) {
  String url = input_url;
  const char* modestr = mode.c_str();
  if (strchr(modestr, '+') || strchr(modestr, 'a') || strchr(modestr, 'w')) {
    std::string msg = "cannot open a url stream for write/append operation: ";
    msg += url.c_str();
    m_error = msg;
    return false;
  }
  HttpClient http(m_timeout, m_maxRedirect);
  auto ctx = this->getStreamContext();
  if (ctx) {
    http.setStreamContextOptions(ctx->getOptions());
  }
  m_response.clear();

  if (!m_proxyHost.empty()) {
    http.proxy(m_proxyHost, m_proxyPort, m_proxyUsername, m_proxyPassword);
  }

  HeaderMap *pHeaders = nullptr;
  HeaderMap requestHeaders;
  if (!m_headers.empty()) {
    pHeaders = &requestHeaders;
    for (ArrayIter iter(m_headers); iter; ++iter) {
      requestHeaders[std::string(iter.first().toString().data())].
        push_back(iter.second().toString().data());
    }
  }

  Variant user = f_parse_url(url, k_PHP_URL_USER);
  if (user.isString()) {
    Variant pass = f_parse_url(url, k_PHP_URL_PASS);
    http.auth(user.toString().c_str(), pass.toString().c_str());
    url = HHVM_FN(preg_replace)(
      s_remove_user_pass_pattern,
      s_remove_user_pass_replace,
      url,
      1
    );
  }

  int code;
  std::vector<String> responseHeaders;
  if (m_get) {
    code = http.get(url.c_str(), m_response, pHeaders, &responseHeaders);
  } else {
    code = http.request(m_method,
                        url.c_str(), m_postData.data(), m_postData.size(),
                        m_response, pHeaders, &responseHeaders);
  }

  m_responseHeaders.reset();
  for (unsigned int i = 0; i < responseHeaders.size(); i++) {
    m_responseHeaders.append(responseHeaders[i]);
  }
  VMRegAnchor vra;
  ActRec* fp = vmfp();
  while (fp->skipFrame()) {
    fp = g_context->getPrevVMState(fp);
  }
  auto id = fp->func()->lookupVarId(s_http_response_header.get());
  if (id != kInvalidId) {
    auto tvTo = frame_local(fp, id);
    Variant varFrom(m_responseHeaders);
    const auto tvFrom(varFrom.asTypedValue());
    if (tvTo->m_type == KindOfRef) {
      tvTo = tvTo->m_data.pref->tv();
    }
    tvDup(*tvFrom, *tvTo);
  } else if ((fp->func()->attrs() & AttrMayUseVV) && fp->hasVarEnv()) {
    fp->getVarEnv()->set(s_http_response_header.get(),
                         Variant(m_responseHeaders).asTypedValue());
  }

  /*
   * If code == 0, Curl failed to connect; per PHP5, ignore_errors just means
   * to not worry if we get an http resonse code that isn't between 200 and 400,
   * but we shouldn't ignore other errors.
   * all status codes in the 2xx range are defined by the specification as
   * successful;
   * all status codes in the 3xx range are for redirection, and so also should
   * never fail.
   */
  if ((code >= 200 && code < 400) || (m_ignoreErrors && code != 0)) {
    setName(url.toCppString());
    m_data = const_cast<char*>(m_response.data());
    m_len = m_response.size();
    return true;
  } else {
    m_error = http.getLastError().c_str();
    return false;
  }
}

int64_t UrlFile::writeImpl(const char *buffer, int64_t length) {
  assert(m_len != -1);
  throw FatalErrorException((std::string("cannot write a url stream: ") +
                             getName()).c_str());
}

bool UrlFile::flush() {
  assert(m_len != -1);
  throw FatalErrorException((std::string("cannot flush a url stream: ") +
                             getName()).c_str());
}

String UrlFile::getLastError() {
  return m_error;
}

///////////////////////////////////////////////////////////////////////////////
}
