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

#include "hphp/runtime/server/warmup-request-handler.h"

#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/program-functions.h"

#include "hphp/runtime/server/replay-transport.h"

#include "hphp/util/boot-stats.h"
#include "hphp/util/timer.h"

#include <folly/Range.h>
#include <folly/Format.h>
#include <folly/Memory.h>

using std::make_unique;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void WarmupRequestHandler::setupRequest(Transport* transport) {
  m_reqHandler.setupRequest(transport);
}

void WarmupRequestHandler::teardownRequest(Transport* transport) noexcept {
  m_reqHandler.teardownRequest(transport);
}

void WarmupRequestHandler::handleRequest(Transport *transport) {
  // There is one WarmupRequestHandler per-thread, but we want to track request
  // count across all threads.  Therefore we let WarmupRequestHandlerFactory
  // track the global request count.
  m_factory->bumpReqCount();
  m_reqHandler.handleRequest(transport);
}

void WarmupRequestHandler::abortRequest(Transport *transport) {
  m_reqHandler.abortRequest(transport);
}

void WarmupRequestHandler::logToAccessLog(Transport *transport) {
  m_reqHandler.logToAccessLog(transport);
}

std::unique_ptr<RequestHandler> WarmupRequestHandlerFactory::createHandler() {
  return
    std::make_unique<WarmupRequestHandler>(m_timeout, shared_from_this());
}

void WarmupRequestHandlerFactory::bumpReqCount() {
  // Bump the request count.  When we hit m_warmupReqThreshold,
  // add additional threads to the server.
  auto const oldReqNum = m_reqNumber.fetch_add(1, std::memory_order_relaxed);
  if (oldReqNum != m_warmupReqThreshold) {
    return;
  }

  Logger::Info("Finished warmup; saturating worker threads");
  m_server->saturateWorkers();
}

void InternalWarmupWorker::run() {
  folly::StringPiece f(m_hdfFile);
  auto pos = f.rfind('/');
  auto const str = (pos == f.npos) ? f : f.subpiece(pos + 1);
  BootStats::Block timer(folly::sformat("warmup:{}:{}", str, m_index),
                         RuntimeOption::ServerExecutionMode());

  // hphp_thread_init() and hphp_thread_exit() are called when we create the
  // thread, through AsyncFuncImpl::SetThreadInitFunc() and
  // AsyncFuncImpl::SetThreadFiniFunc().
  //
  // HttpRequestHandler takes care of doing hphp_session_init(),
  // hphp_session_exit(), and hphp_context_exit().

  try {
    HttpRequestHandler handler(0);
    ReplayTransport rt;
    Logger::FInfo("Replaying warmup request {}:{}", m_hdfFile, m_index);

    timespec start;
    Timer::GetMonotonicTime(start);
    rt.onRequestStart(start);
    rt.replayInput(Hdf(m_hdfFile));
    handler.run(&rt);
  } catch (std::exception& e) {
    Logger::FWarning("Got exception during warmup request {}:{}, {}",
                     m_hdfFile, m_index, e.what());
  }
}

///////////////////////////////////////////////////////////////////////////////
}
