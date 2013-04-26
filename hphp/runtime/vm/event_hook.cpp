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
#include "runtime/vm/func.h"
#include "runtime/vm/translator/translator-inline.h"
#include "runtime/base/builtin_functions.h"
#include "runtime/base/complex_types.h"
#include "runtime/ext/ext_function.h"

namespace HPHP {
namespace VM {

static StaticString s_args("args");
static StaticString s_enter("enter");
static StaticString s_exit("exit");
static StaticString s_exception("exception");
static StaticString s_name("name");
static StaticString s_return("return");

void EventHook::Enable() {
  ThreadInfo::s_threadInfo->m_reqInjectionData.setEventHookFlag();
}

void EventHook::Disable() {
  ThreadInfo::s_threadInfo->m_reqInjectionData.clearEventHookFlag();
}

void EventHook::CheckSurprise() {
  ThreadInfo* info = ThreadInfo::s_threadInfo.getNoCheck();
  check_request_surprise(info);
}

class ExecutingSetprofileCallbackGuard {
public:
  ExecutingSetprofileCallbackGuard() {
    g_vmContext->m_executingSetprofileCallback = true;
  }

  ~ExecutingSetprofileCallbackGuard() {
    g_vmContext->m_executingSetprofileCallback = false;
  }
};

void EventHook::RunUserProfiler(const ActRec* ar, int mode) {
  // Don't do anything if we are running the profiling function itself
  // or if we haven't set up a profiler.
  if (g_vmContext->m_executingSetprofileCallback ||
      g_vmContext->m_setprofileCallback.isNull()) {
    return;
  }
  // Don't profile 86ctor, since its an implementation detail,
  // and we dont guarantee to call it
  if (ar->m_func->cls() && ar->m_func == ar->m_func->cls()->getCtor() &&
      Func::isSpecial(ar->m_func->name())) {
    return;
  }
  Transl::VMRegAnchor _;
  ExecutingSetprofileCallbackGuard guard;

  Array params;
  Array frameinfo;

  if (mode == ProfileEnter) {
    params.append(s_enter);
    frameinfo.set(s_args, hhvm_get_frame_args(ar));
  } else {
    params.append(s_exit);
    if (!g_vmContext->m_faults.empty()) {
      Fault fault = g_vmContext->m_faults.back();
      if (fault.m_faultType == Fault::UserException) {
        frameinfo.set(s_exception, fault.m_userException);
      }
    } else if (!ar->m_func->isBuiltin() &&
               !ar->m_func->isGenerator()) {
      // TODO (#1131400) This is wrong for builtins
      frameinfo.set(s_return, tvAsCVarRef(g_vmContext->m_stack.topTV()));
    }
  }

  params.append(VarNR(ar->m_func->fullName()));
  params.append(frameinfo);

  vm_call_user_func(g_vmContext->m_setprofileCallback, params);
}

void EventHook::onFunctionEnter(const ActRec* ar, int funcType) {
  CheckSurprise();
  RunUserProfiler(ar, ProfileEnter);
#ifdef HOTPROFILER
  Profiler* profiler = ThreadInfo::s_threadInfo->m_profiler;
  if (profiler != nullptr) {
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
  if (profiler != nullptr) {
    end_profiler_frame(profiler);
  }
#endif

  // If we have a pending exception, then we're in the process of unwinding
  // for that exception. We avoid running more PHP code (the user profiler) and
  // also avoid raising more exceptions for surprises (including the pending
  // exception).
  if (ThreadInfo::s_threadInfo->m_pendingException == nullptr) {
    RunUserProfiler(ar, ProfileExit);
    // XXX Disabled until t2329497 is fixed:
    // CheckSurprise();
  }
}

} // namespace VM
} // namespace HPHP
