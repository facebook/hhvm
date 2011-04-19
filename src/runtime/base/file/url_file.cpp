/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/base/file/url_file.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/util/http_client.h>
#include <system/gen/sys/system_globals.h>
#include <runtime/base/runtime_error.h>

using namespace std;

namespace HPHP {

IMPLEMENT_OBJECT_ALLOCATION(UrlFile)
///////////////////////////////////////////////////////////////////////////////

StaticString UrlFile::s_class_name("UrlFile");

///////////////////////////////////////////////////////////////////////////////

UrlFile::UrlFile(const char *method /* = "GET" */,
                 CArrRef headers /* = null_array */,
                 CStrRef postData /* = null_string */,
                 int maxRedirect /* = 20 */,
                 int timeout /* = -1 */) {
  m_get = (method == NULL || strcasecmp(method, "GET") == 0);
  m_headers = headers;
  m_postData = postData;
  m_maxRedirect = maxRedirect;
  m_timeout = timeout;
}

bool UrlFile::open(CStrRef url, CStrRef mode) {
  if (strchr(mode, '+') || strchr(mode, 'a') || strchr(mode, 'w')) {
    std::string msg = "cannot open a url stream for write/append operation: ";
    msg += url.c_str();
    m_error = msg;
    return false;
  }
  HttpClient http(m_timeout, m_maxRedirect);
  StringBuffer response;

  HeaderMap *pHeaders = NULL;
  HeaderMap requestHeaders;
  if (!m_headers.empty()) {
    pHeaders = &requestHeaders;
    for (ArrayIter iter(m_headers); iter; ++iter) {
      requestHeaders[string(iter.first().toString().data())].
        push_back(iter.second().toString().data());
    }
  }

  int code;
  vector<String> responseHeaders;
  if (m_get) {
    code = http.get(url.c_str(), response, pHeaders, &responseHeaders);
  } else {
    code = http.post(url.c_str(), m_postData.data(), m_postData.size(),
                     response, pHeaders, &responseHeaders);
  }

  SystemGlobals *g = (SystemGlobals*)get_global_variables();
  Variant &r = g->GV(http_response_header);
  r = Array::Create();
  for (unsigned int i = 0; i < responseHeaders.size(); i++) {
    r.append(responseHeaders[i]);
  }

  if (code == 200) {
    int len = m_len;
    m_name = url;
    m_data = response.detach(len);
    m_len = len;
    m_malloced = true;
    return true;
  } else {
    m_error = http.getLastError().c_str();
    return false;
  }
}

int64 UrlFile::writeImpl(const char *buffer, int64 length) {
  ASSERT(m_len != -1);
  throw FatalErrorException((string("cannot write a url stream: ") +
                             m_name).c_str());
}

bool UrlFile::flush() {
  ASSERT(m_len != -1);
  throw FatalErrorException((string("cannot flush a url stream: ") +
                             m_name).c_str());
}

String UrlFile::getLastError() {
  return m_error;
}

///////////////////////////////////////////////////////////////////////////////
}
