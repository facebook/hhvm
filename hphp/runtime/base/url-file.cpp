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
#include "hphp/runtime/base/hphp-system.h"
#include "hphp/runtime/base/http-client.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/ext/pcre/ext_pcre.h"
#include "hphp/runtime/ext/url/ext_url.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

const StaticString s_http_response_header("http_response_header");

///////////////////////////////////////////////////////////////////////////////

UrlFile::UrlFile(const char *method /* = "GET" */,
                 const Array& headers /* = null_array */,
                 const String& postData /* = null_string */,
                 int maxRedirect /* = 20 */,
                 int timeout /* = -1 */) {
  m_get = (method == nullptr || strcasecmp(method, "GET") == 0);
  m_headers = headers;
  m_postData = postData;
  m_maxRedirect = maxRedirect;
  m_timeout = timeout;
  m_isLocal = false;
}

void UrlFile::sweep() {
  using std::string;
  m_error.~string();
  MemFile::sweep();
}

const StaticString
  s_remove_user_pass_pattern("#://[^@]+@#"),
  s_remove_user_pass_replace("://");
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
  m_response.clear();

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
    code = http.post(url.c_str(), m_postData.data(), m_postData.size(),
                     m_response, pHeaders, &responseHeaders);
  }

  m_responseHeaders = Array();
  for (unsigned int i = 0; i < responseHeaders.size(); i++) {
    m_responseHeaders.append(responseHeaders[i]);
  }
  GlobalVariables *g = get_global_variables();
  g->set(s_http_response_header, Variant(m_responseHeaders), /*copy=*/ true);

  if (code == 200) {
    m_name = (std::string) url;
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
                             m_name).c_str());
}

bool UrlFile::flush() {
  assert(m_len != -1);
  throw FatalErrorException((std::string("cannot flush a url stream: ") +
                             m_name).c_str());
}

String UrlFile::getLastError() {
  return m_error;
}

///////////////////////////////////////////////////////////////////////////////
}
