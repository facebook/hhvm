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

#include "runtime/base/types.h"
#include "runtime/vm/dyn_tracer.h"
#include "runtime/vm/func.h"

namespace HPHP {
namespace VM {

void DynTracer::Enable() {
  if (g_context->m_dynTracer != NULL) {
    return;
  }
  g_context->m_dynTracer = new DynTracer();
}

void DynTracer::Disable() {
  delete g_context->m_dynTracer;
  g_context->m_dynTracer = NULL;
}

void DynTracer::onFunctionEnter(const ActRec* ar) {
#ifdef HOTPROFILER
  Profiler* profiler = ThreadInfo::s_threadInfo->m_profiler;
  if (profiler != NULL) {
    begin_profiler_frame(profiler, ar->m_func->m_fullName->data());
  }
#endif
}

void DynTracer::onFunctionExit(const ActRec* ar) {
#ifdef HOTPROFILER
  Profiler* profiler = ThreadInfo::s_threadInfo->m_profiler;
  if (profiler != NULL) {
    end_profiler_frame(profiler);
  }
#endif
}

} // namespace VM
} // namespace HPHP
