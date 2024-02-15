/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/server/http-server.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/util/process.h"

namespace HPHP {

int64_t HHVM_FUNCTION(hphp_get_thread_id) {
  return  (unsigned long)Process::GetThreadId();
}

int64_t HHVM_FUNCTION(hphp_gettid) {
  return (unsigned int)Process::GetThreadPid();
}

///////////////////////////////////////////////////////////////////////////////

static struct ThreadExtension final : Extension {
  ThreadExtension() : Extension("thread", NO_EXTENSION_VERSION_YET, NO_ONCALL_YET) {}
  void moduleRegisterNative() override {
    HHVM_FE(hphp_get_thread_id);
    HHVM_FE(hphp_gettid);
  }
} s_thread_extension;

///////////////////////////////////////////////////////////////////////////////
}
