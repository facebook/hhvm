/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/ext/ext_thread.h>
#include <runtime/base/server/service_thread.h>
#include <runtime/base/program_functions.h>
#include <util/process.h>

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(thread);
///////////////////////////////////////////////////////////////////////////////

void f_hphp_service_thread_started() {
  ServiceThread::GetThisThread()->notifyStarted();
}

bool f_hphp_thread_is_warmup_enabled() {
  return hphp_is_warmup_enabled();
}

void f_hphp_thread_set_warmup_enabled() {
  hphp_set_warmup_enabled();
}

int64 f_hphp_get_thread_id() {
  return Process::GetThreadId();
}

///////////////////////////////////////////////////////////////////////////////
}
