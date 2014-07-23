/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/ext/ext_function.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/ext/ext_xenon.h"
#include "hphp/runtime/ext/asio/asio_session.h"

namespace HPHP {

static StaticString s_args("args");
static StaticString s_enter("enter");
static StaticString s_exit("exit");
static StaticString s_exception("exception");
static StaticString s_name("name");
static StaticString s_return("return");

// implemented in runtime/ext/ext_hotprofiler.cpp
extern void begin_profiler_frame(Profiler *p,
                                 const char *symbol);
extern void end_profiler_frame(Profiler *p,
                               const TypedValue *retval,
                               const char *symbol);

void EventHook::Enable() {
  ThreadInfo::s_threadInfo->m_reqInjectionData.setEventHookFlag();
}

void EventHook::Disable() {
  ThreadInfo::s_threadInfo->m_reqInjectionData.clearEventHookFlag();
}

void EventHook::EnableAsync() {
  ThreadInfo::s_threadInfo->m_reqInjectionData.setAsyncEventHookFlag();
}

void EventHook::DisableAsync() {
  ThreadInfo::s_threadInfo->m_reqInjectionData.clearAsyncEventHookFlag();
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

ssize_t EventHook::GetConditionFlags() {
  return RDS::header()->conditionFlags.load();
}

class ExecutingSetprofileCallbackGuard {
public:
  ExecutingSetprofileCallbackGuard() {
    g_context->m_executingSetprofileCallback = true;
  }

  ~ExecutingSetprofileCallbackGuard() {
    g_context->m_executingSetprofileCallback = false;
  }
};

namespace {

bool shouldRunUserProfiler(const Func* func) {
  // Don't do anything if we are running the profiling function itself
  // or if we haven't set up a profiler.
  if (g_context->m_executingSetprofileCallback ||
      g_context->m_setprofileCallback.isNull()) {
    return false;
  }
  // Don't profile 86ctor, since its an implementation detail,
  // and we dont guarantee to call it
  if (func->cls() && func == func->cls()->getCtor() &&
      Func::isSpecial(func->name())) {
    return false;
  }
  return true;
}

void runUserProfilerOnFunctionEnter(const ActRec* ar) {
  VMRegAnchor _;
  ExecutingSetprofileCallbackGuard guard;

  Array params;
  params.append(s_enter);
  params.append(VarNR(ar->func()->fullName()));

  Array frameinfo;
  frameinfo.set(s_args, hhvm_get_frame_args(ar, 0));
  params.append(frameinfo);

  vm_call_user_func(g_context->m_setprofileCallback, params);
}

void runUserProfilerOnFunctionExit(const ActRec* ar, const TypedValue* retval,
                                   ObjectData* exception) {
  VMRegAnchor _;
  ExecutingSetprofileCallbackGuard guard;

  Array params;
  params.append(s_exit);
  params.append(VarNR(ar->func()->fullName()));

  Array frameinfo;
  if (retval) {
    frameinfo.set(s_return, tvAsCVarRef(retval));
  } else if (exception) {
    frameinfo.set(s_exception, exception);
  }
  params.append(frameinfo);

  vm_call_user_func(g_context->m_setprofileCallback, params);
}

}

static Array get_frame_args_with_ref(const ActRec* ar) {
  int numParams = ar->func()->numParams();
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
  const Func* func = ar->func();
  if (LIKELY(func->maybeIntercepted() == 0)) return true;

  // Intercept only original generator / async function calls, not resumption.
  if (ar->resumed()) return true;

  Variant* h = get_intercept_handler(func->fullNameStr(),
                                     &func->maybeIntercepted());
  if (!h) return true;

  /*
   * In production mode, only functions that we have assumed can be
   * intercepted during static analysis should actually be
   * intercepted.
   */
  if (RuntimeOption::RepoAuthoritative &&
      !RuntimeOption::EvalJitEnableRenameFunction) {
    if (!(func->attrs() & AttrInterceptable)) {
      raise_error("fb_intercept was used on a non-interceptable function (%s) "
                  "in RepoAuthoritative mode", func->fullName()->data());
    }
  }

  VMRegAnchor _;

  PC savePc = vmpc();

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
      .append(VarNR(ar->func()->fullName()))
      .append(called_on)
      .append(get_frame_args_with_ref(ar))
      .append(h->asCArrRef()[1])
      .appendRef(doneFlag)
      .toArray();

  Variant ret = vm_call_user_func(h->asCArrRef()[0], intArgs);
  if (doneFlag.toBoolean()) {
    Offset pcOff;
    ActRec* outer = g_context->getPrevVMState(ar, &pcOff);

    frame_free_locals_inl_no_hook<true>(ar, ar->func()->numLocals());
    Stack& stack = vmStack();
    stack.top() = (Cell*)(ar + 1);
    cellDup(*ret.asCell(), *stack.allocTV());

    vmfp() = outer;
    vmpc() = outer ? outer->func()->unit()->at(pcOff) : nullptr;

    return false;
  }
  vmfp() = ar;
  vmpc() = savePc;

  return true;
}

const char* EventHook::GetFunctionNameForProfiler(const Func* func,
                                                  int funcType) {
  const char* name;
  switch (funcType) {
    case EventHook::NormalFunc:
      name = func->fullName()->data();
      if (name[0] == '\0') {
        // We're evaling some code for internal purposes, most
        // likely getting the default value for a function parameter
        name = "{internal}";
      }
      break;
    case EventHook::PseudoMain:
      name = makeStaticString(
        std::string("run_init::") + func->unit()->filepath()->data())
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

void EventHook::onFunctionEnter(const ActRec* ar, int funcType, ssize_t flags) {
  // Xenon
  if (flags & RequestInjectionData::XenonSignalFlag) {
    Xenon::getInstance().log(Xenon::EnterSample);
  }

  // User profiler
  if (flags & RequestInjectionData::EventHookFlag) {
    if (shouldRunUserProfiler(ar->func())) {
      runUserProfilerOnFunctionEnter(ar);
    }
    Profiler* profiler = ThreadInfo::s_threadInfo->m_profiler;
    if (profiler != nullptr) {
      begin_profiler_frame(profiler,
                           GetFunctionNameForProfiler(ar->func(), funcType));
    }
  }
}

void EventHook::onFunctionExit(const ActRec* ar, const TypedValue* retval,
                               const Fault* fault, ssize_t flags) {
  // Xenon
  if (flags & RequestInjectionData::XenonSignalFlag) {
    Xenon::getInstance().log(Xenon::ExitSample);
  }

  // User profiler
  //
  // Inlined calls normally skip the function enter and exit events. If we
  // side exit in an inlined callee, we want to make sure to skip the exit
  // event to avoid unbalancing the call stack.
  if ((flags & RequestInjectionData::EventHookFlag) &&
      (JIT::TCA)ar->m_savedRip != JIT::mcg->tx().uniqueStubs.retInlHelper) {
    Profiler* profiler = ThreadInfo::s_threadInfo->m_profiler;
    if (profiler != nullptr) {
      // NB: we don't have a function type flag to match what we got in
      // onFunctionEnter. That's okay, though... we tolerate this in
      // TraceProfiler.
      end_profiler_frame(profiler,
                         retval,
                         GetFunctionNameForProfiler(ar->func(), NormalFunc));
    }

    if (shouldRunUserProfiler(ar->func())) {
      if (ThreadInfo::s_threadInfo->m_pendingException != nullptr) {
        // Avoid running PHP code when exception from destructor is pending.
        // TODO(#2329497) will not happen once CheckSurprise is used
      } else if (!fault) {
        runUserProfilerOnFunctionExit(ar, retval, nullptr);
      } else if (fault->m_faultType == Fault::Type::UserException) {
        runUserProfilerOnFunctionExit(ar, retval, fault->m_userException);
      } else {
        // Avoid running PHP code when unwinding C++ exception.
      }
    }
  }
}

bool EventHook::onFunctionCall(const ActRec* ar, int funcType) {
  ssize_t flags = CheckSurprise();
  if (flags & RequestInjectionData::InterceptFlag &&
      !RunInterceptHandler(const_cast<ActRec*>(ar))) {
    return false;
  }
  onFunctionEnter(ar, funcType, flags);
  return true;
}

void EventHook::onFunctionResume(const ActRec* ar) {
  ssize_t flags = CheckSurprise();
  onFunctionEnter(ar, EventHook::NormalFunc, flags);
}

void EventHook::onFunctionSuspend(const ActRec* ar, bool suspendingResumed) {
  // TODO(#2329497) can't CheckSurprise() yet, unwinder frees suspended locals
  ssize_t flags = GetConditionFlags();
  onFunctionExit(ar, nullptr, nullptr, flags);

  // Async profiler
  if ((flags & RequestInjectionData::AsyncEventHookFlag) &&
      ar->func()->isAsyncFunction()) {
    assert(ar->resumed());
    auto afwh = frame_afwh(ar);
    auto session = AsioSession::Get();
    // Blocking await @ eager execution => AsyncFunctionWaitHandle created.
    if (!suspendingResumed && session->hasOnResumableCreateCallback()) {
      session->onResumableCreate(afwh, afwh->getChild());
    }
    // Blocking await @ resumed execution => AsyncFunctionWaitHandle awaiting.
    if (suspendingResumed && session->hasOnResumableAwaitCallback()) {
      session->onResumableAwait(afwh, afwh->getChild());
    }
  }
}

void EventHook::onFunctionReturn(ActRec* ar, const TypedValue& retval) {
  // Null out $this for the exiting function, it has been decref'd so it's
  // garbage.
  ar->setThisOrClassAllowNull(nullptr);

  // The locals are already gone. Mark them as decref'd so that if this hook
  // fails and unwinder kicks in, it won't try to decref them again.
  ar->setLocalsDecRefd();

  ssize_t flags = CheckSurprise();
  onFunctionExit(ar, &retval, nullptr, flags);

  // Async profiler
  if ((flags & RequestInjectionData::AsyncEventHookFlag) &&
      ar->func()->isAsyncFunction() && ar->resumed()) {
    auto session = AsioSession::Get();
    // Return @ resumed execution => AsyncFunctionWaitHandle succeeded.
    if (session->hasOnResumableSuccessCallback()) {
      auto afwh = frame_afwh(ar);
      session->onResumableSuccess(afwh, cellAsCVarRef(retval));
    }
  }
}

void EventHook::onFunctionUnwind(const ActRec* ar, const Fault& fault) {
  // TODO(#2329497) can't CheckSurprise() yet, unwinder unable to replace fault
  ssize_t flags = GetConditionFlags();
  onFunctionExit(ar, nullptr, &fault, flags);
}

} // namespace HPHP
