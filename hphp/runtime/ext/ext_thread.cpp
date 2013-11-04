/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/ext/ext_thread.h"
#include "hphp/runtime/server/service-thread.h"
#include "hphp/runtime/server/http-server.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/util/process.h"

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(thread);
///////////////////////////////////////////////////////////////////////////////

bool f_hphp_is_service_thread() {
  return ServiceThread::IsServiceThread();
}

void f_hphp_service_thread_started() {
  if (!ServiceThread::IsServiceThread()) {
    raise_error("hphp_service_thread_started called "
                "from outside a service thread");
  }
  ServiceThread::GetThisThread()->notifyStarted();
}

bool f_hphp_service_thread_stopped(int timeout) {
  if (!ServiceThread::IsServiceThread()) {
    raise_error("hphp_service_thread_stopped called "
                "from outside a service thread");
  }
  return ServiceThread::GetThisThread()->waitForStopped(timeout);
}

int64_t f_hphp_get_thread_id() {
  return  (unsigned long)Process::GetThreadId();
}

int64_t f_hphp_gettid() {
  return (unsigned int)Process::GetThreadPid();
}

///////////////////////////////////////////////////////////////////////////////
}
