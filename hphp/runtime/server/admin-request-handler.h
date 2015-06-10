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

#ifndef incl_HPHP_ADMIN_REQUEST_HANDLER_H_
#define incl_HPHP_ADMIN_REQUEST_HANDLER_H_

#include "hphp/runtime/server/access-log.h"
#include "hphp/runtime/server/server.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class AdminRequestHandler : public RequestHandler {
public:
  static AccessLog &GetAccessLog() { return s_accessLog; }

public:
  explicit AdminRequestHandler(int timeout);

  // implementing RequestHandler
  void setupRequest(Transport* transport) override;
  void teardownRequest(Transport* transport) noexcept override;
  void handleRequest(Transport *transport) override;
  void abortRequest(Transport *transport) override;
  void logToAccessLog(Transport* transport) override;

private:
  bool handleCheckRequest  (const std::string &cmd, Transport *transport);
  bool handleStatusRequest (const std::string &cmd, Transport *transport);
  bool handleMemoryRequest (const std::string &cmd, Transport *transport);
  bool handleStatsRequest  (const std::string &cmd, Transport *transport);
  bool handleProfileRequest(const std::string &cmd, Transport *transport);
  bool handleDumpCacheRequest (const std::string &cmd, Transport *transport);
  bool handleConstSizeRequest (const std::string &cmd, Transport *transport);
  bool handleStaticStringsRequest(const std::string &cmd,
                                  Transport *transport);
  bool handleDumpStaticStrings(const std::string &cmd, Transport *transport,
                               const std::string &filename);
  bool handleVMRequest      (const std::string &cmd, Transport *transport);
  void handleProxyRequest(const std::string& cmd, Transport *transport);
  bool handleRandomApcRequest (const std::string &cmd, Transport *transport);

#ifdef GOOGLE_CPU_PROFILER
  bool handleCPUProfilerRequest (const std::string &cmd, Transport *transport);
#endif

  static DECLARE_THREAD_LOCAL(AccessLog::ThreadData, s_accessLogThreadData);
  static AccessLog s_accessLog;

  static AccessLog::ThreadData* getAccessLogThreadData() {
    return s_accessLogThreadData.get();
  }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_ADMIN_REQUEST_HANDLER_H_
