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

#ifndef incl_HPHP_URL_FILE_H_
#define incl_HPHP_URL_FILE_H_

#include "hphp/runtime/base/http-client.h"
#include "hphp/runtime/base/mem-file.h"
#include "hphp/runtime/base/string-buffer.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * url based files.
 */
class UrlFile : public MemFile {
public:
  DECLARE_RESOURCE_ALLOCATION(UrlFile);

  explicit UrlFile(const char *method = "GET", const Array& headers = null_array,
                   const String& postData = null_string,
                   int maxRedirect = HttpClient::defaultMaxRedirect,
                   int timeout = -1, bool ignoreErrors = false);

  // overriding ResourceData
  const String& o_getClassNameHook() const override { return classnameof(); }

  void setProxy(const String& proxy_host, int proxy_port,
                const String& proxy_user, const String& proxy_pass);
  bool open(const String& filename, const String& mode) override;
  int64_t writeImpl(const char *buffer, int64_t length) override;
  bool seekable() override { return false; }
  bool flush() override;
  Variant getWrapperMetaData() override { return Variant(m_responseHeaders); }
  String getLastError();

private:
  bool m_get;
  bool m_ignoreErrors;
  const char* m_method;
  Array m_headers;
  String m_postData;
  int m_maxRedirect;
  int m_timeout;
  std::string m_error;
  StringBuffer m_response;
  Array m_responseHeaders;

  std::string m_proxyHost;
  int         m_proxyPort;
  std::string m_proxyUsername;
  std::string m_proxyPassword;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_URL_FILE_H_
