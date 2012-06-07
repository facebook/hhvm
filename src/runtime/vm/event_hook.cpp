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
#include "runtime/vm/event_hook.h"
#include "runtime/vm/exception_gate.h"
#include "runtime/vm/func.h"
#include "runtime/vm/translator/translator-inline.h"

namespace HPHP {
namespace VM {

void EventHook::Enable() {
  ThreadInfo::s_threadInfo->m_reqInjectionData.setEventHookFlag();
}

void EventHook::Disable() {
  ThreadInfo::s_threadInfo->m_reqInjectionData.clearEventHookFlag();
}

void EventHook::CheckSurprise() {
  ThreadInfo* info = ThreadInfo::s_threadInfo.getNoCheck();
  check_request_surprise(info);
  if (info->m_pendingException) {
    Transl::VMRegAnchor _;
    EXCEPTION_GATE_ENTER();
    throw_pending_exception(info);
    EXCEPTION_GATE_RETURN();
  }
}

void EventHook::onFunctionEnter(const ActRec* ar, int funcType) {
  CheckSurprise();
#ifdef HOTPROFILER
  Profiler* profiler = ThreadInfo::s_threadInfo->m_profiler;
  if (profiler != NULL) {
    const char* name;
    switch (funcType) {
      case NormalFunc:
        name = ar->m_func->fullName()->data();
        if (name[0] == '\0') {
          // We're evaling some code for internal purposes, most
          // likely getting the default value for a function parameter
          name = "{internal}";
        }
        break;
      case PseudoMain:
        name = StringData::GetStaticString(
          std::string("run_init::") + ar->m_func->unit()->filepath()->data())
          ->data();
        break;
      case Eval:
        name = "_";
        break;
      default:
        not_reached();
    }
    begin_profiler_frame(profiler, name);
  }
#endif
}

void EventHook::onFunctionExit(const ActRec* ar) {
#ifdef HOTPROFILER
  Profiler* profiler = ThreadInfo::s_threadInfo->m_profiler;
  if (profiler != NULL) {
    end_profiler_frame(profiler);
  }
#endif
}

} // namespace VM
} // namespace HPHP
