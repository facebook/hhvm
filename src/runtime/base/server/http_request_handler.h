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

#ifndef __HTTP_REQUEST_HANDLER_H__
#define __HTTP_REQUEST_HANDLER_H__

#include <runtime/base/server/libevent_server.h>
#include <runtime/base/util/string_buffer.h>
#include <runtime/base/server/virtual_host.h>
#include <runtime/base/server/access_log.h>

namespace HPHP {

class SourceRootInfo;
class RequestURI;
///////////////////////////////////////////////////////////////////////////////

class HttpRequestHandler : public RequestHandler {
public:
  static AccessLog &GetAccessLog() { return s_accessLog; }

public:
  HttpRequestHandler();

  // implementing RequestHandler
  virtual void handleRequest(Transport *transport);

  // for internal invoke of a special URL
  void disablePathTranslation() { m_pathTranslation = false;}

private:
  bool m_pathTranslation;

  bool handleProxyRequest(Transport *transport, bool force);
  void sendStaticContent(Transport *transport, const char *data, int len,
                         time_t mtime, bool compressed,
                         const std::string &cmd);
  bool executePHPRequest(Transport *transport, RequestURI &reqURI,
                         SourceRootInfo &sourceRootInfo,
                         bool cachableDynamicContent);
  bool MatchAnyPattern(const std::string &path,
                       const std::vector<std::string> &patterns);

  static DECLARE_THREAD_LOCAL(AccessLog::ThreadData, s_accessLog_tl);
  static AccessLog s_accessLog;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HTTP_REQUEST_HANDLER_H__
