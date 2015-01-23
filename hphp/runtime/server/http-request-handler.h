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

#ifndef incl_HPHP_HTTP_REQUEST_HANDLER_H_
#define incl_HPHP_HTTP_REQUEST_HANDLER_H_

#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/server/virtual-host.h"
#include "hphp/runtime/server/access-log.h"
#include "hphp/runtime/server/server.h"
#include "hphp/runtime/server/source-root-info.h"

#include <folly/Optional.h>

namespace HPHP {

class SourceRootInfo;
class RequestURI;

namespace ServiceData {
class ExportedTimeSeries;
}

/*
 * Atomically (with respect to concurrent calls to shouldProxyPath()) set a new
 * origin and proxy percentage. All other options are unmodified.
 */
void setProxyOriginPercentage(const std::string& origin, int percentage);

///////////////////////////////////////////////////////////////////////////////

class HttpRequestHandler : public RequestHandler {
public:
  static AccessLog &GetAccessLog() { return s_accessLog; }

public:
  explicit HttpRequestHandler(int timeout);

  // implementing RequestHandler
  void setupRequest(Transport* transport) override;
  void teardownRequest(Transport* transport) noexcept override;
  void handleRequest(Transport* transport) override;
  void abortRequest(Transport* transport) override;
  void logToAccessLog(Transport* transport) override;

  // for internal invoke of a special URL
  void disablePathTranslation() { m_pathTranslation = false;}

private:
  bool m_pathTranslation;
  ServiceData::ExportedTimeSeries* m_requestTimedOutOnQueue;
  folly::Optional<SourceRootInfo> m_sourceRootInfo;

  bool handleProxyRequest(Transport *transport, bool force);
  void sendStaticContent(Transport *transport, const char *data, int len,
                         time_t mtime, bool compressed,
                         const std::string &cmd,
                         const char *ext);
  bool executePHPRequest(Transport *transport, RequestURI &reqURI,
                         SourceRootInfo &sourceRootInfo,
                         bool cacheableDynamicContent);

  static DECLARE_THREAD_LOCAL(AccessLog::ThreadData, s_accessLogThreadData);
  static AccessLog s_accessLog;

  static AccessLog::ThreadData* getAccessLogThreadData() {
    return s_accessLogThreadData.get();
  }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_HTTP_REQUEST_HANDLER_H_
