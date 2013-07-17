/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/base/thread_init_fini.h"
#include "hphp/runtime/base/smart_allocator.h"
#include "hphp/runtime/base/execution_context.h"
#include "hphp/runtime/base/preg.h"
#include "hphp/runtime/server/server_stats.h"
#include "hphp/runtime/server/server_note.h"
#include "hphp/runtime/base/zend_strtod.h"
#include "hphp/runtime/base/zend_math.h"
#include "hphp/util/async_func.h"
#include "hphp/util/alloc.h"
#include "hphp/runtime/base/hardware_counter.h"
#include "hphp/runtime/ext/asio/asio_session.h"
#include "hphp/runtime/ext/ext_icu.h"
#include "hphp/runtime/base/intercept.h"

#include "hphp/runtime/vm/repo.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

InitFiniNode *extra_init, *extra_fini, *extra_process_init, *extra_process_exit;
InitFiniNode *extra_server_init, *extra_server_exit;

InitFiniNode::InitFiniNode(void(*f)(), When init) {
  InitFiniNode *&ifn =
    init == When::ThreadInit ? extra_init :
    init == When::ThreadFini ? extra_fini :
    init == When::ProcessInit ? extra_process_init :
    init == When::ProcessExit ? extra_process_exit :
    init == When::ServerInit ? extra_server_init : extra_server_exit;
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
  ThreadInfo::s_threadInfo.getCheck();
  g_context.getCheck();
  AsioSession::Init();
  s_hasRenamedFunction.getCheck();
  HardwareCounter::s_counter.getCheck();
  for (InitFiniNode *in = extra_init; in; in = in->next) {
    in->func();
  }
}

void finish_thread_locals(void *arg /* = NULL */) {
  for (InitFiniNode *in = extra_fini; in; in = in->next) {
    in->func();
  }
  if (!g_context.isNull()) g_context.destroy();
  if (!g_persistentObjects.isNull()) g_persistentObjects.destroy();
}

static class SetThreadInitFini {
public:
  SetThreadInitFini() {
    AsyncFuncImpl::SetThreadInitFunc(init_thread_locals, nullptr);
    AsyncFuncImpl::SetThreadFiniFunc(finish_thread_locals, nullptr);
  }
} s_SetThreadInitFini;

///////////////////////////////////////////////////////////////////////////////
}
