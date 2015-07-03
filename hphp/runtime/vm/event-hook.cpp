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

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/intercept.h"
#include "hphp/runtime/base/surprise-flags.h"
#include "hphp/runtime/base/types.h"

#include "hphp/runtime/ext/asio/asio-session.h"
#include "hphp/runtime/ext/hotprofiler/ext_hotprofiler.h"
#include "hphp/runtime/ext/intervaltimer/ext_intervaltimer.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/ext/xenon/ext_xenon.h"

#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/func.h"

#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/vm-regs.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const StaticString
  s_args("args"),
  s_enter("enter"),
  s_exit("exit"),
  s_exception("exception"),
  s_name("name"),
  s_return("return");

// implemented in runtime/ext/ext_hotprofiler.cpp
extern void begin_profiler_frame(Profiler *p,
                                 const char *symbol);
extern void end_profiler_frame(Profiler *p,
                               const TypedValue *retval,
                               const char *symbol);

void EventHook::Enable() {
  setSurpriseFlag(EventHookFlag);
}

void EventHook::Disable() {
  clearSurpriseFlag(EventHookFlag);
}

void EventHook::EnableAsync() {
  setSurpriseFlag(AsyncEventHookFlag);
}

void EventHook::DisableAsync() {
  clearSurpriseFlag(AsyncEventHookFlag);
}

void EventHook::EnableDebug() {
  setSurpriseFlag(DebuggerHookFlag);
}

void EventHook::DisableDebug() {
  clearSurpriseFlag(DebuggerHookFlag);
}

void EventHook::EnableIntercept() {
  setSurpriseFlag(InterceptFlag);
}

void EventHook::DisableIntercept() {
  clearSurpriseFlag(InterceptFlag);
}

struct ExecutingSetprofileCallbackGuard {
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
  int numNonVariadic = ar->func()->numNonVariadicParams();
  int numArgs = ar->numArgs();

  PackedArrayInit retArray(numArgs);

  auto local = reinterpret_cast<TypedValue*>(
    uintptr_t(ar) - sizeof(TypedValue)
  );
  int i = 0;
  // The function's formal parameters are on the stack
  for (; i < numArgs && i < numNonVariadic; ++i) {
    retArray.appendWithRef(tvAsCVarRef(local));
    --local;
  }

  if (i < numArgs) {
    // If there are still args that haven't been accounted for, they have
    // either been ... :
    if (ar->func()->hasVariadicCaptureParam()) {
      // ... shuffled into a packed array stored in the variadic capture
      // param on the stack
      for (ArrayIter iter(tvAsCVarRef(local)); iter; ++iter) {
        retArray.appendWithRef(iter.secondRef());
      }
    } else {
      // ... or moved into the ExtraArgs datastructure.
      for (; i < numArgs; ++i) {
        retArray.appendWithRef(
          tvAsCVarRef(ar->getExtraArg(i - numNonVariadic)));
      }
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
  // User profiler
  if (flags & EventHookFlag) {
    if (shouldRunUserProfiler(ar->func())) {
      runUserProfilerOnFunctionEnter(ar);
    }
    auto profiler = ThreadInfo::s_threadInfo->m_profiler;
    if (profiler != nullptr &&
        !(profiler->shouldSkipBuiltins() && ar->func()->isBuiltin())) {
      begin_profiler_frame(profiler,
                           GetFunctionNameForProfiler(ar->func(), funcType));
    }
  }

  // Debugger hook
  if (flags & DebuggerHookFlag) {
    DEBUGGER_ATTACHED_ONLY(phpDebuggerFuncEntryHook(ar));
  }
}

void EventHook::onFunctionExit(const ActRec* ar, const TypedValue* retval,
                               bool unwind, ObjectData* phpException,
                               size_t flags) {
  // Xenon
  if (flags & XenonSignalFlag) {
    Xenon::getInstance().log(Xenon::ExitSample);
  }

  // Run IntervalTimer callbacks only if it's safe to do so, i.e., not when
  // there's a pending exception or we're unwinding from a C++ exception.
  if (flags & IntervalTimerFlag
      && ThreadInfo::s_threadInfo->m_pendingException == nullptr
      && (!unwind || phpException)) {
    IntervalTimer::RunCallbacks(IntervalTimer::ExitSample);
  }

  // Inlined calls normally skip the function enter and exit events. If we
  // side exit in an inlined callee, we short-circuit here in order to skip
  // exit events that could unbalance the call stack.
  if (RuntimeOption::EvalJit &&
      ((jit::TCA) ar->m_savedRip == jit::mcg->tx().uniqueStubs.retInlHelper)) {
    return;
  }

  // User profiler
  if (flags & EventHookFlag) {
    auto profiler = ThreadInfo::s_threadInfo->m_profiler;
    if (profiler != nullptr &&
        !(profiler->shouldSkipBuiltins() && ar->func()->isBuiltin())) {
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
      } else if (!unwind) {
        runUserProfilerOnFunctionExit(ar, retval, nullptr);
      } else if (phpException) {
        runUserProfilerOnFunctionExit(ar, retval, phpException);
      } else {
        // Avoid running PHP code when unwinding C++ exception.
      }
    }
  }

  // Debugger hook
  if (flags & DebuggerHookFlag) {
    DEBUGGER_ATTACHED_ONLY(phpDebuggerFuncExitHook(ar));
  }
}

bool EventHook::onFunctionCall(const ActRec* ar, int funcType) {
  auto const flags = check_request_surprise();
  if (flags & InterceptFlag &&
      !RunInterceptHandler(const_cast<ActRec*>(ar))) {
    return false;
  }

  // Xenon
  if (flags & XenonSignalFlag) {
    Xenon::getInstance().log(Xenon::EnterSample);
  }

  if (flags & IntervalTimerFlag) {
    IntervalTimer::RunCallbacks(IntervalTimer::EnterSample);
  }

  onFunctionEnter(ar, funcType, flags);
  return true;
}

void EventHook::onFunctionResumeAwait(const ActRec* ar) {
  auto const flags = check_request_surprise();

  // Xenon
  if (flags & XenonSignalFlag) {
    Xenon::getInstance().log(Xenon::ResumeAwaitSample);
  }

  if (flags & IntervalTimerFlag) {
    IntervalTimer::RunCallbacks(IntervalTimer::ResumeAwaitSample);
  }

  onFunctionEnter(ar, EventHook::NormalFunc, flags);
}

void EventHook::onFunctionResumeYield(const ActRec* ar) {
  auto const flags = check_request_surprise();

  // Xenon
  if (flags & XenonSignalFlag) {
    Xenon::getInstance().log(Xenon::EnterSample);
  }

  if (flags & IntervalTimerFlag) {
    IntervalTimer::RunCallbacks(IntervalTimer::EnterSample);
  }

  onFunctionEnter(ar, EventHook::NormalFunc, flags);
}

// Child is the AFWH we're going to block on, nullptr iff this is a suspending
// generator.
void EventHook::onFunctionSuspendR(ActRec* suspending, ObjectData* child) {
  auto const flags = check_request_surprise();
  onFunctionExit(suspending, nullptr, false, nullptr, flags);

  if ((flags & AsyncEventHookFlag) &&
      suspending->func()->isAsyncFunction()) {
    assert(child != nullptr);  // This isn't a generator
    assert(child->instanceof(c_WaitableWaitHandle::classof()));
    assert(suspending->resumed());
    auto const afwh = frame_afwh(suspending);
    auto const session = AsioSession::Get();
    if (session->hasOnResumableAwait()) {
      session->onResumableAwait(
        afwh,
        static_cast<c_WaitableWaitHandle*>(child)
      );
    }
  }
}

void EventHook::onFunctionSuspendE(ActRec* suspending,
                                   const ActRec* resumableAR) {
  // When we're suspending an eagerly executing resumable, we've already
  // teleported the ActRec from suspending over to resumableAR, so we need to
  // make sure the unwinder knows not to touch the locals, $this, or
  // VarEnv/ExtraArgs.
  suspending->setThisOrClassAllowNull(nullptr);
  suspending->setLocalsDecRefd();
  suspending->setVarEnv(nullptr);

  try {
    auto const flags = check_request_surprise();
    onFunctionExit(resumableAR, nullptr, false, nullptr, flags);

    if ((flags & AsyncEventHookFlag) &&
        resumableAR->func()->isAsyncFunction()) {
      assert(resumableAR->resumed());
      auto const afwh = frame_afwh(resumableAR);
      auto const session = AsioSession::Get();
      if (session->hasOnResumableCreate()) {
        session->onResumableCreate(afwh, afwh->getChild());
      }
    }
  } catch (...) {
    auto const resumableObj = [&]() -> ObjectData* {
      if (resumableAR->func()->isAsyncFunction()) {
        return frame_afwh(resumableAR);
      }
      assert(resumableAR->func()->isGenerator());
      return !resumableAR->func()->isAsync()
        ? frame_generator(resumableAR)->toObject()
        : frame_async_generator(resumableAR)->toObject();
    }();
    decRefObj(resumableObj);
    throw;
  }
}

void EventHook::onFunctionReturn(ActRec* ar, TypedValue retval) {
  // The locals are already gone. Null out everything.
  ar->setThisOrClassAllowNull(nullptr);
  ar->setLocalsDecRefd();
  ar->setVarEnv(nullptr);

  try {
    auto const flags = check_request_surprise();
    onFunctionExit(ar, &retval, false, nullptr, flags);

    // Async profiler
    if ((flags & AsyncEventHookFlag) &&
        ar->func()->isAsyncFunction() && ar->resumed()) {
      auto session = AsioSession::Get();
      // Return @ resumed execution => AsyncFunctionWaitHandle succeeded.
      if (session->hasOnResumableSuccess()) {
        auto afwh = frame_afwh(ar);
        session->onResumableSuccess(afwh, cellAsCVarRef(retval));
      }
    }
  } catch (...) {
    /*
     * We're responsible for freeing the return value if we exit with an
     * exception.  See irgen-ret.
     */
    tvRefcountedDecRef(retval);
    throw;
  }
}

void EventHook::onFunctionUnwind(ActRec* ar, ObjectData* phpException) {
  // The locals are already gone. Null out everything.
  ar->setThisOrClassAllowNull(nullptr);
  ar->setLocalsDecRefd();
  ar->setVarEnv(nullptr);

  // TODO(#2329497) can't check_request_surprise() yet, unwinder unable to
  // replace fault
  auto const flags = stackLimitAndSurprise().load() & kSurpriseFlagMask;
  onFunctionExit(ar, nullptr, true, phpException, flags);
}

} // namespace HPHP
