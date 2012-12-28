/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#include <runtime/base/preg.h>
#include <runtime/base/server/server_stats.h>
#include <runtime/base/server/server_note.h>
#include <runtime/base/zend/zend_strtod.h>
#include <runtime/base/zend/zend_math.h>
#include <util/async_func.h>
#include <util/alloc.h>
#include <runtime/base/hardware_counter.h>
#include <runtime/ext/ext_icu.h>
#include <runtime/base/intercept.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

InitFiniNode *extra_init, *extra_fini;

InitFiniNode::InitFiniNode(void(*f)(), bool init) {
  InitFiniNode *&ifn = init ? extra_init : extra_fini;
  func = f;
  next = ifn;
  ifn = this;
}

void init_thread_locals(void *arg /* = NULL */) {
  Sweepable::InitSweepableList();
  ObjectData::GetMaxId();
  ResourceData::GetMaxResourceId();
  ServerStats::GetLogger();
  zend_get_bigint_data();
  zend_get_rand_data();
  get_server_note();
  g_persistentObjects.getCheck();
  MemoryManager::TlsWrapper::getCheck();
  InitAllocatorThreadLocal();
  RefData::AllocatorType::getCheck();
  get_global_variables_check();
  ThreadInfo::s_threadInfo.getCheck();
  g_context.getCheck();
  s_hasRenamedFunction.getCheck();
  HardwareCounter::s_counter.getCheck();
  for (InitFiniNode *in = extra_init; in; in = in->next) {
    in->func();
  }
}

void finish_thread_locals(void *arg /* = NULL */) {
  if (!g_context.isNull()) g_context.destroy();
  if (!g_persistentObjects.isNull()) g_persistentObjects.destroy();
}

static class SetThreadInitFini {
public:
  SetThreadInitFini() {
    AsyncFuncImpl::SetThreadInitFunc(init_thread_locals, NULL);
    AsyncFuncImpl::SetThreadFiniFunc(finish_thread_locals, NULL);
  }
} s_SetThreadInitFini;

///////////////////////////////////////////////////////////////////////////////
}
