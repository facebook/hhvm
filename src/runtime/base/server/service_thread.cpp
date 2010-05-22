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

#include <runtime/base/server/service_thread.h>
#include <runtime/base/runtime_option.h>
#include <util/thread_local.h>
#include <util/hdf.h>
#include <runtime/base/server/replay_transport.h>
#include <runtime/base/server/http_request_handler.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// statics

static IMPLEMENT_THREAD_LOCAL_PROXY(ServiceThread, true, s_service_threads);

ServiceThread *ServiceThread::GetThisThread() {
  return s_service_threads.get();
}

///////////////////////////////////////////////////////////////////////////////

ServiceThread::ServiceThread(const std::string &url) :
  AsyncFunc<ServiceThread>(this, &ServiceThread::threadRun),
  m_started(false), m_url(url) {
}

void ServiceThread::threadRun() {
  Logger::Info("Service thread %s started", m_url.c_str());
  s_service_threads.set(this);

  Hdf hdf;
  hdf["get"] = 1;
  hdf["url"] = m_url;
  hdf["remote_host"] = RuntimeOption::ServerIP;

  ReplayTransport rt;
  rt.replayInput(hdf);
  HttpRequestHandler handler;
  handler.disablePathTranslation();
  handler.handleRequest(&rt);

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

///////////////////////////////////////////////////////////////////////////////
}
