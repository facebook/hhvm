/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/http-client.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/base/curl-tls-workarounds.h"
#include "hphp/util/timer.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include "hphp/util/logger.h"
#include "hphp/util/ssl-init.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

//so that curl_global_init() is called ahead of time, avoiding crash
class StaticInitializer {
public:
  StaticInitializer() {
    curl_global_init(CURL_GLOBAL_ALL);
    SSLInit::Init();
  }
};
static StaticInitializer s_initCurl;

HttpClient::HttpClient(int timeout /* = 5 */, int maxRedirect /* = 1 */,
                       bool use11 /* = true */, bool decompress /* = false */)
  : m_timeout(timeout), m_maxRedirect(maxRedirect), m_use11(use11),
    m_decompress(decompress), m_response(nullptr), m_responseHeaders(nullptr),
    m_proxyPort(0) {
  if (m_timeout <= 0) {
    m_timeout = RuntimeOption::SocketDefaultTimeout;
  }
}

size_t HttpClient::curl_write(char *data, size_t size, size_t nmemb,
                              void *ctx) {
  return ((HttpClient*)ctx)->write(data, size, nmemb);
}
size_t HttpClient::write(char *data, size_t size, size_t nmemb) {
  size_t length = size * nmemb;
  if (length > 0 && m_response) {
    m_response->append(data, (int)length);
  }
  return length;
}

size_t HttpClient::curl_header(char *data, size_t size, size_t nmemb,
                               void *ctx) {
  return ((HttpClient*)ctx)->header(data, size, nmemb);
}
size_t HttpClient::header(char *data, size_t size, size_t nmemb) {
  size_t length = size * nmemb;
  if (length > 2 && data[length - 2] == '\r' && data[length - 1] == '\n' &&
      m_responseHeaders) {
    m_responseHeaders->push_back(String(data, length - 2, CopyString));
  }
  return length;
}

void HttpClient::auth(const std::string &username,
                      const std::string &password, bool basic /* = true */) {
  m_basic = true;
  m_username = username;
  m_password = password;
}

void HttpClient::proxy(const std::string &host, int port,
                       const std::string &username /* = "" */,
                       const std::string &password /* = "" */) {
  m_proxyHost = host;
  m_proxyPort = port;
  m_proxyUsername = username;
  m_proxyPassword = password;
}

int HttpClient::get(const char *url, StringBuffer &response,
                    const HeaderMap *requestHeaders /* = NULL */,
                    std::vector<String> *responseHeaders /* = NULL */) {
  return impl(url, nullptr, 0, response, requestHeaders, responseHeaders);
}

int HttpClient::post(const char *url, const char *data, int size,
                     StringBuffer &response,
                     const HeaderMap *requestHeaders /* = NULL */,
                     std::vector<String> *responseHeaders /* = NULL */) {
  return impl(url, data, size, response, requestHeaders, responseHeaders);
}

const StaticString
  s_ssl("ssl"),
  s_verify_peer("verify_peer"),
  s_capath("capath"),
  s_cafile("cafile"),
  s_local_cert("local_cert"),
  s_passphrase("passphrase");

int HttpClient::impl(const char *url, const char *data, int size,
                     StringBuffer &response, const HeaderMap *requestHeaders,
                     std::vector<String> *responseHeaders) {
  SlowTimer timer(RuntimeOption::HttpSlowQueryThreshold, "curl", url);

  m_response = &response;

  char error_str[CURL_ERROR_SIZE + 1];
  memset(error_str, 0, sizeof(error_str));

  CURL *cp = curl_easy_init();
  curl_easy_setopt(cp, CURLOPT_URL,               url);
  curl_easy_setopt(cp, CURLOPT_WRITEFUNCTION,     curl_write);
  curl_easy_setopt(cp, CURLOPT_WRITEDATA,         (void*)this);
  curl_easy_setopt(cp, CURLOPT_ERRORBUFFER,       error_str);
  curl_easy_setopt(cp, CURLOPT_NOPROGRESS,        1);
  curl_easy_setopt(cp, CURLOPT_VERBOSE,           0);
  curl_easy_setopt(cp, CURLOPT_NOSIGNAL,          1);
  curl_easy_setopt(cp, CURLOPT_DNS_USE_GLOBAL_CACHE, 0); // for thread-safe
  curl_easy_setopt(cp, CURLOPT_DNS_CACHE_TIMEOUT, 120);
  curl_easy_setopt(cp, CURLOPT_NOSIGNAL, 1); // for multithreading mode
  curl_easy_setopt(cp, CURLOPT_SSL_VERIFYPEER,    0);
  curl_easy_setopt(cp, CURLOPT_SSL_CTX_FUNCTION, curl_tls_workarounds_cb);

  curl_easy_setopt(cp, CURLOPT_TIMEOUT,           m_timeout);
  if (m_maxRedirect > 1) {
    curl_easy_setopt(cp, CURLOPT_FOLLOWLOCATION,    1);
    curl_easy_setopt(cp, CURLOPT_MAXREDIRS,         m_maxRedirect);
  } else {
    curl_easy_setopt(cp, CURLOPT_FOLLOWLOCATION,    0);
  }
  if (!m_use11) {
    curl_easy_setopt(cp, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
  }
  if (m_decompress) {
    curl_easy_setopt(cp, CURLOPT_ENCODING, "");
  }

  if (!m_username.empty()) {
    curl_easy_setopt(cp, CURLOPT_HTTPAUTH,
                     m_basic ? CURLAUTH_BASIC : CURLAUTH_DIGEST);
    curl_easy_setopt(cp, CURLOPT_USERNAME, m_username.c_str());
    curl_easy_setopt(cp, CURLOPT_PASSWORD, m_password.c_str());
  }

  if (!m_proxyHost.empty() && m_proxyPort) {
    curl_easy_setopt(cp, CURLOPT_PROXY,     m_proxyHost.c_str());
    curl_easy_setopt(cp, CURLOPT_PROXYPORT, m_proxyPort);
    if (!m_proxyUsername.empty()) {
      curl_easy_setopt(cp, CURLOPT_PROXYAUTH, CURLAUTH_BASIC);
      curl_easy_setopt(cp, CURLOPT_PROXYUSERNAME, m_proxyUsername.c_str());
      curl_easy_setopt(cp, CURLOPT_PROXYPASSWORD, m_proxyPassword.c_str());
    }
  }

  std::vector<String> headers; // holding those temporary strings
  curl_slist *slist = nullptr;
  if (requestHeaders) {
    for (HeaderMap::const_iterator iter = requestHeaders->begin();
         iter != requestHeaders->end(); ++iter) {
      for (unsigned int i = 0; i < iter->second.size(); i++) {
        String header = iter->first + ": " + iter->second[i];
        headers.push_back(header);
        slist = curl_slist_append(slist, header.data());
      }
    }
    if (slist) {
      curl_easy_setopt(cp, CURLOPT_HTTPHEADER, slist);
    }
  }

  if (data && size) {
    curl_easy_setopt(cp, CURLOPT_POST,          1);
    curl_easy_setopt(cp, CURLOPT_POSTFIELDS,    data);
    curl_easy_setopt(cp, CURLOPT_POSTFIELDSIZE, size);
  }

  if (responseHeaders) {
    m_responseHeaders = responseHeaders;
    curl_easy_setopt(cp, CURLOPT_HEADERFUNCTION, curl_header);
    curl_easy_setopt(cp, CURLOPT_WRITEHEADER, (void*)this);
  }

  if (m_stream_context_options[s_ssl].isArray()) {
    const Array ssl = m_stream_context_options[s_ssl].toArray();
    if (ssl[s_verify_peer].toBoolean()) {
      curl_easy_setopt(cp, CURLOPT_SSL_VERIFYPEER, 1);
    }
    if (ssl.exists(s_capath)) {
      curl_easy_setopt(cp, CURLOPT_CAPATH,
                       ssl[s_capath].toString().data());
    }
    if (ssl.exists(s_cafile)) {
      curl_easy_setopt(cp, CURLOPT_CAINFO,
                       ssl[s_cafile].toString().data());
    }
    if (ssl.exists(s_local_cert)) {
      curl_easy_setopt(cp, CURLOPT_SSLKEY,
                       ssl[s_local_cert].toString().data());
      curl_easy_setopt(cp, CURLOPT_SSLKEYTYPE, "PEM");
    }
    if (ssl.exists(s_passphrase)) {
      curl_easy_setopt(cp, CURLOPT_KEYPASSWD,
                       ssl[s_passphrase].toString().data());
    }
  }

  long code = 0;
  {
    IOStatusHelper io("http", url);
    CURLcode error_no = curl_easy_perform(cp);
    if (error_no != CURLE_OK) {
      m_error = error_str;
    } else {
      curl_easy_getinfo(cp, CURLINFO_RESPONSE_CODE, &code);
    }
  }

  set_curl_statuses(cp, url);

  if (slist) {
    curl_slist_free_all(slist);
  }

  curl_easy_cleanup(cp);
  return code;
}

///////////////////////////////////////////////////////////////////////////////
}
