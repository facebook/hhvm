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

#include <runtime/base/thread_init_fini.h>
#include <runtime/base/memory/smart_allocator.h>
#include <runtime/base/execution_context.h>
#include <util/async_func.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void init_thread_locals(void *arg /* = NULL */) {
  InitAllocatorThreadLocal();
  get_global_variables_check();
  ThreadInfo::s_threadInfo.get();
}

void fini_thread_locals(void *arg /* = NULL */) {
  g_context.reset();
}

static class SetThreadInitFini {
public:
  SetThreadInitFini() {
    AsyncFuncImpl::SetThreadInitFunc(init_thread_locals, NULL);
    AsyncFuncImpl::SetThreadFiniFunc(fini_thread_locals, NULL);
  }
} s_SetThreadInitFini;

///////////////////////////////////////////////////////////////////////////////
}
