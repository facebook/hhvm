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

#include <memory>

#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/server/server.h"
#include "hphp/runtime/server/http-request-handler.h"
#include "hphp/util/job-queue.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct WarmupRequestHandlerFactory;

/**
 * WarmupRequestHandler is a small shim on top of HttpRequestHandler.
 * It counts the number of requests, and adds additional worker threads to the
 * server after a specified threshold.
 */
struct WarmupRequestHandler : RequestHandler {
  explicit WarmupRequestHandler(
      int timeout,
      const std::shared_ptr<WarmupRequestHandlerFactory>& factory)
    : RequestHandler(timeout), m_factory(factory), m_reqHandler(timeout) {}

  void setupRequest(Transport* transport) override;
  void teardownRequest(Transport* transport) noexcept override;
  void handleRequest(Transport* transport) override;
  void abortRequest(Transport* transport) override;
  void logToAccessLog(Transport* transport) override;

private:
  std::shared_ptr<WarmupRequestHandlerFactory> m_factory;
  HttpRequestHandler m_reqHandler;
};

struct WarmupRequestHandlerFactory
  : std::enable_shared_from_this<WarmupRequestHandlerFactory>
{
  WarmupRequestHandlerFactory(Server *server,
                              uint32_t reqCount,
                              int timeout)
    : m_reqNumber(0),
      m_warmupReqThreshold(reqCount),
      m_timeout(timeout),
      m_server(server) {}

  std::unique_ptr<RequestHandler> createHandler();

  void bumpReqCount();

private:
  std::atomic<uint32_t> m_reqNumber;
  uint32_t const m_warmupReqThreshold;
  int m_timeout;
  // The server owns this object so will by definition outlive us
  Server *m_server;
};

struct WarmupJob {
  const std::string hdfFile;
  unsigned index;
};

struct InternalWarmupWorker : JobQueueWorker<WarmupJob> {
  void doJob(WarmupJob job) override;
};

struct InternalWarmupRequestPlayer : JobQueueDispatcher<InternalWarmupWorker> {
  explicit InternalWarmupRequestPlayer(int threadCount, bool dedup = false);
  ~InternalWarmupRequestPlayer();

  // Start running after an optional delay.
  void runAfterDelay(const std::vector<std::string>& files,
                     unsigned nTimes = 1,
                     unsigned delaySeconds = 0);
private:
  // If set, duplicated files in the list will be ignored (but it is still
  // possible to play each file multiple times by setting nTimes in
  // runAfterDelay).
  bool m_noDuplicate;
};

// Can run multiple times, e.g., upon code changes.
void replayExtendedWarmupRequests();

///////////////////////////////////////////////////////////////////////////////
}
