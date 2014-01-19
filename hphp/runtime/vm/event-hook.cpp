/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/event-hook.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/jit/translator-x64.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/ext/ext_function.h"
#include "hphp/runtime/vm/runtime.h"

namespace HPHP {

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

void EventHook::EnableIntercept() {
  ThreadInfo::s_threadInfo->m_reqInjectionData.setInterceptFlag();
}

void EventHook::DisableIntercept() {
  ThreadInfo::s_threadInfo->m_reqInjectionData.clearInterceptFlag();
}

ssize_t EventHook::CheckSurprise() {
  ThreadInfo* info = ThreadInfo::s_threadInfo.getNoCheck();
  return check_request_surprise(info);
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
  JIT::VMRegAnchor _;
  ExecutingSetprofileCallbackGuard guard;

  Array params;
  Array frameinfo;

  if (mode == ProfileEnter) {
    params.append(s_enter);
    frameinfo.set(s_args, hhvm_get_frame_args(ar, 0));
  } else {
    params.append(s_exit);
    if (!g_vmContext->m_faults.empty()) {
      Fault fault = g_vmContext->m_faults.back();
      if (fault.m_faultType == Fault::Type::UserException) {
        frameinfo.set(s_exception, fault.m_userException);
      }
    } else if (!ar->m_func->isCPPBuiltin() &&
               !ar->m_func->isGenerator()) {
      // TODO (#1131400) This is wrong for builtins
      frameinfo.set(s_return, tvAsCVarRef(g_vmContext->m_stack.topTV()));
    }
  }

  params.append(VarNR(ar->m_func->fullName()));
  params.append(frameinfo);

  vm_call_user_func(g_vmContext->m_setprofileCallback, params);
}

static Array get_frame_args_with_ref(const ActRec* ar) {
  int numParams = ar->m_func->numParams();
  int numArgs = ar->numArgs();

  PackedArrayInit retArray(numArgs);

  auto local = reinterpret_cast<TypedValue*>(
    uintptr_t(ar) - sizeof(TypedValue)
  );
  for (int i = 0; i < numArgs; ++i) {
    if (i < numParams) {
      // This corresponds to one of the function's formal parameters, so it's
      // on the stack.
      retArray.appendWithRef(tvAsCVarRef(local));
      --local;
    } else {
      // This is not a formal parameter, so it's in the ExtraArgs.
      retArray.appendWithRef(tvAsCVarRef(ar->getExtraArg(i - numParams)));
    }
  }

  return retArray.toArray();
}

bool EventHook::RunInterceptHandler(ActRec* ar) {
  const Func* func = ar->m_func;
  if (LIKELY(func->maybeIntercepted() == 0)) return true;

  Variant *h = get_intercept_handler(func->fullNameRef(),
                                     &func->maybeIntercepted());
  if (!h) return true;

  JIT::VMRegAnchor _;

  PC savePc = g_vmContext->m_pc;

  Variant doneFlag = true;
  Variant called_on;

  if (ar->hasThis()) {
    called_on = Variant(ar->getThis());
  } else if (ar->hasClass()) {
    // For static methods, give handler the name of called class
    called_on = Variant(const_cast<StringData*>(ar->getClass()->name()));
  }
  Variant intArgs =
    PackedArrayInit(5)
      .append(ar->m_func->fullNameRef())
      .append(called_on)
      .append(get_frame_args_with_ref(ar))
      .append(h->asCArrRef()[1])
      .appendRef(doneFlag)
      .toArray();

  Variant ret = vm_call_user_func(h->asCArrRef()[0], intArgs);
  if (doneFlag.toBoolean()) {
    Offset pcOff;
    ActRec* outer = g_vmContext->getPrevVMState(ar, &pcOff);

    frame_free_locals_inl_no_hook<true>(ar, ar->m_func->numLocals());
    Stack& stack = g_vmContext->getStack();
    stack.top() = (Cell*)(ar + 1);
    cellDup(*ret.asCell(), *stack.allocTV());

    g_vmContext->m_fp = outer;
    g_vmContext->m_pc = outer ? outer->m_func->unit()->at(pcOff) : nullptr;

    return false;
  }
  g_vmContext->m_fp = ar;
  g_vmContext->m_pc = savePc;

  return true;
}

const char* EventHook::GetFunctionNameForProfiler(const ActRec* ar,
                                                  int funcType) {
  const char* name;
  switch (funcType) {
    case EventHook::NormalFunc:
      name = ar->m_func->fullName()->data();
      if (name[0] == '\0') {
        // We're evaling some code for internal purposes, most
        // likely getting the default value for a function parameter
        name = "{internal}";
      }
      break;
    case EventHook::PseudoMain:
      name = makeStaticString(
        std::string("run_init::") + ar->m_func->unit()->filepath()->data())
        ->data();
      break;
    case EventHook::Eval:
      name = "_";
      break;
    default:
      not_reached();
  }
  return name;
}

bool EventHook::onFunctionEnter(const ActRec* ar, int funcType) {
  ssize_t flags = CheckSurprise();
  if (flags & RequestInjectionData::InterceptFlag &&
      !RunInterceptHandler(const_cast<ActRec*>(ar))) {
    return false;
  }
  if (flags & RequestInjectionData::EventHookFlag) {
    RunUserProfiler(ar, ProfileEnter);
#ifdef HOTPROFILER
    Profiler* profiler = ThreadInfo::s_threadInfo->m_profiler;
    if (profiler != nullptr) {
      begin_profiler_frame(profiler, GetFunctionNameForProfiler(ar, funcType));
    }
#endif
  }
  return true;
}

void EventHook::onFunctionExit(const ActRec* ar) {
  auto const inlinedRip = JIT::tx64->uniqueStubs.retInlHelper;
  if ((JIT::TCA)ar->m_savedRip == inlinedRip) {
    // Inlined calls normally skip the function enter and exit events. If we
    // side exit in an inlined callee, we want to make sure to skip the exit
    // event to avoid unbalancing the call stack.
    return;
  }

#ifdef HOTPROFILER
  Profiler* profiler = ThreadInfo::s_threadInfo->m_profiler;
  if (profiler != nullptr) {
    // NB: we don't have a function type flag to match what we got in
    // onFunctionEnter. That's okay, though... we tolerate this in
    // TraceProfiler.
    end_profiler_frame(profiler, GetFunctionNameForProfiler(ar, NormalFunc));
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

} // namespace HPHP
