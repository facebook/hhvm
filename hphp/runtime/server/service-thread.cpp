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

#include "hphp/runtime/server/service-thread.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/util/thread-local.h"
#include "hphp/util/hdf.h"
#include "hphp/runtime/server/replay-transport.h"
#include "hphp/runtime/server/http-request-handler.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// statics

static IMPLEMENT_THREAD_LOCAL_PROXY(ServiceThread, true, s_service_threads);

bool ServiceThread::IsServiceThread() {
  return !s_service_threads.isNull();
}

ServiceThread *ServiceThread::GetThisThread() {
  return s_service_threads.get();
}

///////////////////////////////////////////////////////////////////////////////

ServiceThread::ServiceThread(const std::string &url, bool loop /*= false*/) :
  AsyncFunc<ServiceThread>(this, &ServiceThread::threadRun),
  m_loop(loop), m_started(loop), m_stopped(false), m_url(url) {
}

void ServiceThread::threadRun() {
  Logger::Info("Service thread %s started", m_url.c_str());
  s_service_threads.set(this);

  Hdf hdf;
  hdf["get"] = 1;
  hdf["url"] = m_url;
  hdf["remote_host"] = RuntimeOption::ServerIP;

  HttpRequestHandler handler(0);
  handler.disablePathTranslation();

  do {
    ReplayTransport rt;
    rt.replayInput(hdf);
    handler.handleRequest(&rt);
  } while (m_loop && !m_stopped);

  Logger::Info("Service thread %s stopped", m_url.c_str());
}

void ServiceThread::waitForStarted() {
  Lock lock(this);
  while (!m_started) {
    wait();
  }
}

void ServiceThread::notifyStarted() {
  Lock lock(this);
  m_started = true;
  notify();
}

bool ServiceThread::waitForStopped(int seconds) {
  Lock lock(this);
  if (!m_stopped) {
    wait(seconds);
  }
  return m_stopped;
}

void ServiceThread::notifyStopped() {
  Lock lock(this);
  m_stopped = true;
  notify();
}

///////////////////////////////////////////////////////////////////////////////
}
