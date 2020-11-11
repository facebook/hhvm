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

#pragma once

#include "hphp/runtime/server/access-log.h"
#include "hphp/runtime/server/server.h"

#include <folly/File.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Admin Command Ext allow you to register more admin commands that are not
 * located in the admin command file.
 */
struct AdminCommandExt {
  AdminCommandExt() {
    next = s_head;
    s_head = this;
  }

  virtual std::string usage() = 0;
  virtual bool handleRequest(Transport* transport) = 0;

  template <typename L>
  static bool iterate(L lambda) {
    for (auto p = s_head; p; p = p->next) {
      if (lambda(p)) return true;
    }
    return false;
  }

  AdminCommandExt* next{nullptr};
  static AdminCommandExt* s_head;
};

struct AdminRequestHandler : RequestHandler {
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
  bool handleInvalidateUnitRequest(const std::string &cmd,
                                   Transport *transport);
  bool handleStaticStringsRequest(const std::string &cmd,
                                  Transport *transport);
  bool handleDumpStaticStringsRequest(folly::File& file);
  bool handleRandomStaticStringsRequest(const std::string &cmd,
                                        Transport *transport);
  bool handleVMRequest      (const std::string &cmd, Transport *transport);
  void handleProxyRequest(const std::string& cmd, Transport *transport);
  bool handleRandomApcRequest (const std::string &cmd, Transport *transport);

#ifdef GOOGLE_CPU_PROFILER
  bool handleCPUProfilerRequest (const std::string &cmd, Transport *transport);
#endif

  static THREAD_LOCAL(AccessLog::ThreadData, s_accessLogThreadData);
  static AccessLog s_accessLog;

  static AccessLog::ThreadData* getAccessLogThreadData() {
    return s_accessLogThreadData.get();
  }
};

///////////////////////////////////////////////////////////////////////////////
}

