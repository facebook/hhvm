/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/base/intercept.h"
#include "hphp/runtime/base/surprise-flags.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/variable-serializer.h"

#include "hphp/runtime/ext/asio/asio-session.h"
#include "hphp/runtime/ext/hotprofiler/ext_hotprofiler.h"
#include "hphp/runtime/ext/intervaltimer/ext_intervaltimer.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/ext/xenon/ext_xenon.h"

#include "hphp/runtime/server/server-stats.h"

#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/interp-helpers.h"
#include "hphp/runtime/vm/unit-util.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/util/struct-log.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const StaticString
  s_args("args"),
  s_frame_ptr("frame_ptr"),
  s_parent_frame_ptr("parent_frame_ptr"),
  s_this_ptr("this_ptr"),
  s_enter("enter"),
  s_exit("exit"),
  s_suspend("suspend"),
  s_resume("resume"),
  s_exception("exception"),
  s_name("name"),
  s_return("return");

__thread uint64_t tl_func_sequence_id;

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

void EventHook::DoMemoryThresholdCallback() {
  clearSurpriseFlag(MemThresholdFlag);
  if (!g_context->m_memThresholdCallback.isNull()) {
    VMRegAnchor _;
    try {
      vm_call_user_func(g_context->m_memThresholdCallback, empty_array());
    } catch (Object& ex) {
      raise_error("Uncaught exception escaping mem Threshold callback: %s",
                  ex.toString().data());
    }
  }
}

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
  if ((g_context->m_setprofileFlags & EventHook::ProfileConstructors) == 0 &&
      func->cls() && func == func->cls()->getCtor() &&
      Func::isSpecial(func->name())) {
    return false;
  }
  return true;
}

ALWAYS_INLINE
ActRec* getParentFrame(const ActRec* ar) {
  return g_context->getPrevVMStateSkipFrame(ar);
}

void addFramePointers(const ActRec* ar, Array& frameinfo, bool isEnter) {
  if ((g_context->m_setprofileFlags & EventHook::ProfileFramePointers) == 0) {
    return;
  }

  if (isEnter) {
    auto this_ptr = ar->func()->cls() && ar->hasThis() ?
      intptr_t(ar->getThis()) : 0;
    frameinfo.set(s_this_ptr, Variant(this_ptr));
  }

  frameinfo.set(s_frame_ptr, Variant(intptr_t(ar)));
  ActRec* parent_ar = getParentFrame(ar);
  if (parent_ar != nullptr) {
    frameinfo.set(s_parent_frame_ptr, Variant(intptr_t(parent_ar)));
  }
}

inline bool isResumeAware() {
  return (g_context->m_setprofileFlags & EventHook::ProfileResumeAware) != 0;
}

void runUserProfilerOnFunctionEnter(const ActRec* ar, bool isResume) {
  if ((g_context->m_setprofileFlags & EventHook::ProfileEnters) == 0) {
    return;
  }

  VMRegAnchor _;
  ExecutingSetprofileCallbackGuard guard;

  Array params;
  params.append((isResume && isResumeAware()) ? s_resume : s_enter);
  params.append(make_tv<KindOfPersistentString>(ar->func()->fullName()));

  Array frameinfo;
  frameinfo.set(s_args, hhvm_get_frame_args(ar, 0));
  addFramePointers(ar, frameinfo, true);
  params.append(frameinfo);

  vm_call_user_func(g_context->m_setprofileCallback, params);
}

void runUserProfilerOnFunctionExit(const ActRec* ar, const TypedValue* retval,
                                   ObjectData* exception, bool isSuspend) {
  if ((g_context->m_setprofileFlags & EventHook::ProfileExits) == 0) {
    return;
  }

  VMRegAnchor _;
  ExecutingSetprofileCallbackGuard guard;

  Array params;
  params.append((isSuspend && isResumeAware()) ? s_suspend : s_exit);
  params.append(make_tv<KindOfPersistentString>(ar->func()->fullName()));

  Array frameinfo;
  if (retval) {
    frameinfo.set(s_return, tvAsCVarRef(retval));
  } else if (exception) {
    frameinfo.set(s_exception, Variant{exception});
  }
  addFramePointers(ar, frameinfo, false);
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
        retArray.appendWithRef(iter.secondVal());
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

static Variant call_intercept_handler(
  const Variant& function,
  const Variant& called,
  const Variant& called_on,
  Array& args,
  const Variant& ctx,
  Variant& done
) {
  ObjectData* obj = nullptr;
  Class* cls = nullptr;
  CallerFrame cf;
  StringData* invName = nullptr;
  bool dynamic = false;
  auto f = vm_decode_function(function, cf(), false,
                              obj, cls, invName, dynamic);
  if (!f) {
    return uninit_null();
  }

  PackedArrayInit par(5);
  par.append(called);
  par.append(called_on);
  par.append(args);
  par.append(ctx);

  Variant intArgs;

  auto const inout = f->isInOutWrapper();
  if (inout) {
    auto const name = mangleInOutFuncName(f->name(), {2,4});

    if (!f->isMethod()) {
      f = Unit::lookupFunc(name.get());
    } else {
      assertx(cls);
      f = cls->lookupMethod(name.get());
    }
    if (!f) {
      raise_error(
        "fb_intercept used with an inout handler with a bad signature "
        "(expected parameters three and five to be inout)"
      );
    }
    intArgs = par.append(done).toArray();
  } else {
    intArgs = par.appendRef(done).toArray();
  }

  auto ret = Variant::attach(
    g_context->invokeFunc(f, intArgs, obj, cls,
                          nullptr, invName, ExecutionContext::InvokeNormal,
                          dynamic, false)
  );
  if (UNLIKELY(isRefType(ret.getRawType()))) {
    tvUnbox(*ret.asTypedValue());
  }

  if (inout) {
    auto& arr = ret.asCArrRef();
    if (arr[1].isArray()) args = arr[1].toArray();
    if (arr[2].isBoolean()) done = arr[2].toBoolean();
    return arr[0];
  }
  return ret;
}

bool EventHook::RunInterceptHandler(ActRec* ar) {
  const Func* func = ar->func();
  if (LIKELY(func->maybeIntercepted() == 0)) return true;

  // Intercept only original generator / async function calls, not resumption.
  if (ar->resumed()) return true;

  // Don't intercept inout wrappers. We'll intercept the inner function.
  if (func->isInOutWrapper()) return true;

  auto const name = func->takesInOutParams()
    ? stripInOutSuffix(func->fullName())
    : func->fullName();

  Variant* h = get_intercept_handler(StrNR(name), &func->maybeIntercepted());
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

  checkForRequiredCallM(ar);

  VMRegAnchor _;

  PC savePc = vmpc();

  Variant doneFlag = true;
  Variant called_on = [&] {
    if (func->cls()) {
      if (ar->hasThis()) {
        return Variant(ar->getThis());
      }
      // For static methods, give handler the name of called class
      return Variant{ar->getClass()->name(), Variant::PersistentStrInit{}};
    }
    return init_null();
  }();

  auto args = get_frame_args_with_ref(ar);
  VarNR called(ar->func()->fullDisplayName());

  Variant ret = call_intercept_handler(
    h->asCArrRef()[0], called, called_on, args, h->asCArrRef()[1], doneFlag
  );

  if (doneFlag.toBoolean()) {
    Offset pcOff;
    ActRec* outer = g_context->getPrevVMState(ar, &pcOff);

    ar->setLocalsDecRefd();
    frame_free_locals_no_hook(ar);

    // Tear down the callee frame, then push the return value.
    Stack& stack = vmStack();
    stack.trim((Cell*)(ar + 1));
    if (UNLIKELY(func->takesInOutParams())) {
      uint32_t count = func->numInOutParams();

      auto start = stack.topTV();
      auto const end = start + count;

      auto push = [&] (TypedValue v) {
        assertx(start < end);
        tvIncRefGen(v);
        *start++ = v;
      };

      uint32_t param = 0;
      IterateKV(args.get(), [&] (Cell, TypedValue v) {
        if (param >= func->numParams() || !func->params()[param++].inout) {
          return;
        }
        push(v);
      });

      while (start < end) push(make_tv<KindOfNull>());
    }

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

static bool shouldLog(const Func* /*func*/) {
  return RID().logFunctionCalls();
}

void logCommon(StructuredLogEntry sample, const ActRec* ar, ssize_t flags) {
  sample.setInt("flags", flags);
  sample.setInt("func_id", ar->func()->getFuncId());
  sample.setStr("thread_mode",
    ServerStats::ThreadModeString(ServerStats::GetThreadMode()));
  sample.setStr("func_name", ar->func()->name()->data());
  sample.setStr("func_full_name", ar->func()->fullName()->data());
  sample.setStr("func_filename", ar->func()->filename()->data());
  addBacktraceToStructLog(
    createBacktrace(BacktraceArgs()), sample
  );
  sample.setInt("sequence_id", tl_func_sequence_id++);
  StructuredLog::log("hhvm_function_calls2", sample);
}

void EventHook::onFunctionEnter(const ActRec* ar, int funcType,
                                ssize_t flags, bool isResume) {
  // User profiler
  if (flags & EventHookFlag) {
    if (shouldRunUserProfiler(ar->func())) {
      runUserProfilerOnFunctionEnter(ar, isResume);
    }
    if (shouldLog(ar->func())) {
      StructuredLogEntry sample;
      sample.setStr("event_name", "function_enter");
      sample.setInt("func_type", funcType);
      sample.setInt("is_resume", isResume);
      logCommon(sample, ar, flags);
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
                               size_t flags, bool isSuspend) {
  // Inlined calls normally skip the function enter and exit events. If we
  // side exit in an inlined callee, we short-circuit here in order to skip
  // exit events that could unbalance the call stack.
  if (RuntimeOption::EvalJit &&
      ((jit::TCA) ar->m_savedRip == jit::tc::ustubs().retInlHelper)) {
    return;
  }

  // Xenon
  if (flags & XenonSignalFlag) {
    Xenon::getInstance().log(Xenon::ExitSample);
  }

  // Run callbacks only if it's safe to do so, i.e., not when
  // there's a pending exception or we're unwinding from a C++ exception.
  if (ThreadInfo::s_threadInfo->m_pendingException == nullptr
      && (!unwind || phpException)) {

    // Memory Threhsold
    if (flags & MemThresholdFlag) {
      DoMemoryThresholdCallback();
    }

    // Interval timer
    if (flags & IntervalTimerFlag) {
      IntervalTimer::RunCallbacks(IntervalTimer::ExitSample);
    }
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
        runUserProfilerOnFunctionExit(ar, retval, nullptr, isSuspend);
      } else if (phpException) {
        runUserProfilerOnFunctionExit(ar, retval, phpException, isSuspend);
      } else {
        // Avoid running PHP code when unwinding C++ exception.
      }
    }
    if (shouldLog(ar->func())) {
      StructuredLogEntry sample;
      sample.setStr("event_name", "function_exit");
      sample.setInt("is_suspend", isSuspend);
      logCommon(sample, ar, flags);
    }
  }

  // Debugger hook
  if (flags & DebuggerHookFlag) {
    DEBUGGER_ATTACHED_ONLY(phpDebuggerFuncExitHook(ar));
  }
}

bool EventHook::onFunctionCall(const ActRec* ar, int funcType) {
  auto const flags = handle_request_surprise();
  if (flags & InterceptFlag &&
      !RunInterceptHandler(const_cast<ActRec*>(ar))) {
    return false;
  }

  // Xenon
  if (flags & XenonSignalFlag) {
    Xenon::getInstance().log(Xenon::EnterSample);
  }

  // Memory Threhsold
  if (flags & MemThresholdFlag) {
    DoMemoryThresholdCallback();
  }

  if (flags & IntervalTimerFlag) {
    IntervalTimer::RunCallbacks(IntervalTimer::EnterSample);
  }

  onFunctionEnter(ar, funcType, flags, false);
  return true;
}

void EventHook::onFunctionResumeAwait(const ActRec* ar) {
  auto const flags = handle_request_surprise();

  // Xenon
  if (flags & XenonSignalFlag) {
    Xenon::getInstance().log(Xenon::ResumeAwaitSample);
  }

  // Memory Threhsold
  if (flags & MemThresholdFlag) {
    DoMemoryThresholdCallback();
  }

  if (flags & IntervalTimerFlag) {
    IntervalTimer::RunCallbacks(IntervalTimer::ResumeAwaitSample);
  }

  onFunctionEnter(ar, EventHook::NormalFunc, flags, true);
}

void EventHook::onFunctionResumeYield(const ActRec* ar) {
  auto const flags = handle_request_surprise();

  // Xenon
  if (flags & XenonSignalFlag) {
    Xenon::getInstance().log(Xenon::EnterSample);
  }

  // Memory Threhsold
  if (flags & MemThresholdFlag) {
    DoMemoryThresholdCallback();
  }

  if (flags & IntervalTimerFlag) {
    IntervalTimer::RunCallbacks(IntervalTimer::EnterSample);
  }

  onFunctionEnter(ar, EventHook::NormalFunc, flags, true);
}

// Eagerly executed async function initially suspending at Await.
void EventHook::onFunctionSuspendAwaitEF(ActRec* suspending,
                                         const ActRec* resumableAR) {
  assertx(suspending->func()->isAsyncFunction());
  assertx(suspending->func() == resumableAR->func());
  assertx(resumableAR->resumed());

  // The locals were already teleported from suspending to resumableAR.
  suspending->setLocalsDecRefd();
  suspending->trashThis();
  suspending->trashVarEnv();

  try {
    auto const flags = handle_request_surprise();
    onFunctionExit(resumableAR, nullptr, false, nullptr, flags, true);

    if (flags & AsyncEventHookFlag) {
      auto const afwh = frame_afwh(resumableAR);
      auto const session = AsioSession::Get();
      if (session->hasOnResumableCreate()) {
        session->onResumableCreate(afwh, afwh->getChild());
      }
    }
  } catch (...) {
    decRefObj(frame_afwh(resumableAR));
    throw;
  }
}

// Eagerly executed async generator that was resumed at Yield suspending
// at Await.
void EventHook::onFunctionSuspendAwaitEG(ActRec* suspending) {
  assertx(suspending->func()->isAsyncGenerator());
  assertx(suspending->resumed());

  // The generator is still being executed eagerly, make it look like that.
  auto const gen = frame_async_generator(suspending);
  auto wh = gen->detachWaitHandle();
  SCOPE_EXIT { gen->attachWaitHandle(std::move(wh)); };

  try {
    auto const flags = handle_request_surprise();
    onFunctionExit(suspending, nullptr, false, nullptr, flags, true);

    if (flags & AsyncEventHookFlag) {
      auto const session = AsioSession::Get();
      if (session->hasOnResumableCreate()) {
        session->onResumableCreate(wh.get(), wh->getChild());
      }
    }
  } catch (...) {
    decRefObj(wh.get());
    throw;
  }
}

// Async function or async generator that was resumed at Await suspending
// again at Await. The suspending frame has an associated AFWH/AGWH. Child
// is the WH we are going to block on.
void EventHook::onFunctionSuspendAwaitR(ActRec* suspending, ObjectData* child) {
  assertx(suspending->func()->isAsync());
  assertx(suspending->resumed());
  assertx(child->isWaitHandle());

  auto const flags = handle_request_surprise();
  onFunctionExit(suspending, nullptr, false, nullptr, flags, true);

  if (flags & AsyncEventHookFlag) {
    assertx(child->instanceof(c_WaitableWaitHandle::classof()));
    auto const session = AsioSession::Get();
    if (session->hasOnResumableAwait()) {
      if (!suspending->func()->isGenerator()) {
        session->onResumableAwait(
          frame_afwh(suspending),
          static_cast<c_WaitableWaitHandle*>(child)
        );
      } else {
        session->onResumableAwait(
          frame_async_generator(suspending)->getWaitHandle(),
          static_cast<c_WaitableWaitHandle*>(child)
        );
      }
    }
  }
}

// Generator or async generator suspending initially at CreateCont.
void EventHook::onFunctionSuspendCreateCont(ActRec* suspending,
                                            const ActRec* resumableAR) {
  assertx(suspending->func()->isGenerator());
  assertx(suspending->func() == resumableAR->func());
  assertx(resumableAR->resumed());

  // The locals were already teleported from suspending to resumableAR.
  suspending->setLocalsDecRefd();
  suspending->trashThis();
  suspending->trashVarEnv();

  try {
    auto const flags = handle_request_surprise();
    onFunctionExit(resumableAR, nullptr, false, nullptr, flags, true);
  } catch (...) {
    auto const resumableObj = [&]() -> ObjectData* {
      return !resumableAR->func()->isAsync()
        ? frame_generator(resumableAR)->toObject()
        : frame_async_generator(resumableAR)->toObject();
    }();
    decRefObj(resumableObj);
    throw;
  }
}

// Generator or async generator suspending at Yield.
void EventHook::onFunctionSuspendYield(ActRec* suspending) {
  assertx(suspending->func()->isGenerator());
  assertx(suspending->resumed());

  auto const flags = handle_request_surprise();
  onFunctionExit(suspending, nullptr, false, nullptr, flags, true);

  if ((flags & AsyncEventHookFlag) && suspending->func()->isAsync()) {
    auto const ag = frame_async_generator(suspending);
    if (!ag->isEagerlyExecuted()) {
      auto const wh = ag->getWaitHandle();
      auto const session = AsioSession::Get();
      if (session->hasOnResumableSuccess()) {
        session->onResumableSuccess(wh, init_null());
      }
    }
  }
}

void EventHook::onFunctionReturn(ActRec* ar, TypedValue retval) {
  // The locals are already gone. Tell everyone
  ar->setLocalsDecRefd();
  ar->trashThis();
  ar->trashVarEnv();

  try {
    auto const flags = handle_request_surprise();
    onFunctionExit(ar, &retval, false, nullptr, flags, false);

    // Async profiler
    if ((flags & AsyncEventHookFlag) && ar->resumed() &&
        ar->func()->isAsync()) {
      auto session = AsioSession::Get();
      if (session->hasOnResumableSuccess()) {
        if (!ar->func()->isGenerator()) {
          session->onResumableSuccess(frame_afwh(ar), cellAsCVarRef(retval));
        } else {
          auto ag = frame_async_generator(ar);
          if (!ag->isEagerlyExecuted()) {
            session->onResumableSuccess(ag->getWaitHandle(), init_null());
          }
        }
      }
    }
  } catch (...) {
    /*
     * We're responsible for freeing the return value if we exit with an
     * exception.  See irgen-ret.
     */
    tvDecRefGen(retval);
    throw;
  }
}

void EventHook::onFunctionUnwind(ActRec* ar, ObjectData* phpException) {
  // The locals are already gone. Tell everyone
  ar->setLocalsDecRefd();
  ar->trashThis();
  ar->trashVarEnv();

  // TODO(#2329497) can't handle_request_surprise() yet, unwinder unable to
  // replace fault
  auto const flags = stackLimitAndSurprise().load() & kSurpriseFlagMask;
  onFunctionExit(ar, nullptr, true, phpException, flags, false);
}

} // namespace HPHP
