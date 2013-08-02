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

#ifndef incl_HPHP_HTTP_CLIENT_H_
#define incl_HPHP_HTTP_CLIENT_H_

#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/server/transport.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class HttpClient {
public:
  explicit HttpClient(int timeout = 5 /* seconds */, int maxRedirect = 1,
                      bool use11 = true, bool decompress = false);

  /**
   * Authentication.
   */
  void auth(const std::string &username, const std::string &password,
            bool basic = true);
  /**
   * Set up proxy server.
   */
  void proxy(const std::string &host, int port,
             const std::string &username = "",
             const std::string &password = "");

  /**
   * StreamContext settings for this connection
   */
  void setStreamContextOptions(const Array &opts) {
    m_stream_context_options = opts;
  }

  /**
   * GET an URL and returns its response code.
   */
  int get(const char *url, StringBuffer &response,
          const HeaderMap *requestHeaders = nullptr,
          std::vector<String> *responseHeaders = nullptr);

  /**
   * POST data to an URL and returns its response code.
   */
  int post(const char *url, const char *data, int size, StringBuffer &response,
           const HeaderMap *requestHeaders = nullptr,
           std::vector<String> *responseHeaders = nullptr);

  std::string getLastError() const { return m_error;}

private:
  int m_timeout;
  int m_maxRedirect;
  bool m_use11;
  bool m_decompress;

  StringBuffer *m_response;
  std::vector<String> *m_responseHeaders;
  std::string m_error;

  bool m_basic;
  std::string m_username;
  std::string m_password;

  std::string m_proxyHost;
  int         m_proxyPort;
  std::string m_proxyUsername;
  std::string m_proxyPassword;

  Array       m_stream_context_options;

  int impl(const char *url, const char *data, int size, StringBuffer &response,
           const HeaderMap *requestHeaders,
           std::vector<String> *responseHeaders);

  static size_t curl_write(char *data, size_t size, size_t nmemb, void *ctx);
  static size_t curl_header(char *data, size_t size, size_t nmemb, void *ctx);

  size_t write(char *data, size_t size, size_t nmemb);
  size_t header(char *data, size_t size, size_t nmemb);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_HTTP_CLIENT_H_
