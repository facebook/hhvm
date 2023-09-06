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
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/container-functions.h"
#include "hphp/runtime/base/backtrace.h"
#include "hphp/runtime/base/implicit-context.h"
#include "hphp/runtime/base/intercept.h"
#include "hphp/runtime/base/surprise-flags.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/type-structure-helpers-defs.h"
#include "hphp/runtime/base/variable-serializer.h"

#include "hphp/runtime/ext/asio/asio-session.h"
#include "hphp/runtime/ext/intervaltimer/ext_intervaltimer.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/ext/std/ext_std_variable.h"
#include "hphp/runtime/ext/strobelight/ext_strobelight.h"
#include "hphp/runtime/ext/xenon/ext_xenon.h"

#include "hphp/runtime/server/server-stats.h"

#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/resumable.h"
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
  s_this_obj("this_obj"),
  s_file("file"),
  s_line("line"),
  s_enter("enter"),
  s_exit("exit"),
  s_suspend("suspend"),
  s_resume("resume"),
  s_exception("exception"),
  s_return("return"),
  s_reified_classes("reified_classes"),
  s_reified_generics_var("0ReifiedGenerics");

RDS_LOCAL_NO_CHECK(uint64_t, rl_func_sequence_id){0};

void EventHook::Enable() {
  setSurpriseFlag(EventHookFlag);
}

void EventHook::Disable() {
  if (g_context->m_internalEventHookCallback == nullptr) {
    clearSurpriseFlag(EventHookFlag);
  }
}

void EventHook::EnableInternal(ExecutionContext::InternalEventHookCallbackType
                              callback) {
  g_context->m_internalEventHookCallback = callback;
  setSurpriseFlag(EventHookFlag);
}

void EventHook::DisableInternal() {
  if (g_context->m_setprofileCallback.isNull()) {
    clearSurpriseFlag(EventHookFlag);
  }
  g_context->m_internalEventHookCallback = nullptr;
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
      vm_call_user_func(g_context->m_memThresholdCallback, empty_vec_array());
    } catch (Object& ex) {
      raise_error("Uncaught exception escaping mem Threshold callback: %s",
                  throwable_to_string(ex.get()).data());
    }
  }
}

namespace {

bool shouldRunUserProfiler(const Func* func) {
  // Don't do anything if we are running the profiling function itself
  // or if we haven't set up a profiler.
  if (g_context->m_executingSetprofileCallback ||
      g_context->m_setprofileCallback.isNull() ||
      (!g_context->m_setprofileFunctions.empty() &&
        g_context->m_setprofileFunctions.count(
          StrNR(func->fullName())) == 0)) {
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

Array getReifiedClasses(const ActRec* ar) {
  using K = TypeStructure::Kind;

  auto const func = ar->func();
  auto const& tparams = func->getReifiedGenericsInfo();
  VecInit clist{tparams.m_typeParamInfo.size()};

  auto const loc = frame_local(ar, ar->func()->numParams());
  assertx(
    func->localVarName(func->numParams()) == s_reified_generics_var.get()
  );
  assertx(isArrayLikeType(type(loc)));

  int idx = 0;
  auto const generics = val(loc).parr;

  for (auto const& pinfo : tparams.m_typeParamInfo) {
    if (!pinfo.m_isReified) {
      clist.append(init_null());
      idx++;
      continue;
    }
    auto const ts = generics->at(idx++);
    assertx(isArrayLikeType(type(ts)));

    auto const kind = get_ts_kind(val(ts).parr);

    switch (kind) {
    case K::T_class:
    case K::T_interface:
    case K::T_trait:
    case K::T_enum:
      clist.append(
        String{const_cast<StringData*>(get_ts_classname(val(ts).parr))}
      );
      break;

    case K::T_xhp:
      clist.append(String{xhpNameFromTS(ArrNR{val(ts).parr})});
      break;

    case K::T_union:
    case K::T_void:
    case K::T_int:
    case K::T_bool:
    case K::T_float:
    case K::T_string:
    case K::T_resource:
    case K::T_num:
    case K::T_arraykey:
    case K::T_noreturn:
    case K::T_mixed:
    case K::T_tuple:
    case K::T_fun:
    case K::T_typevar:
    case K::T_shape:
    case K::T_dict:
    case K::T_vec:
    case K::T_keyset:
    case K::T_vec_or_dict:
    case K::T_nonnull:
    case K::T_darray:
    case K::T_varray:
    case K::T_varray_or_darray:
    case K::T_any_array:
    case K::T_null:
    case K::T_nothing:
    case K::T_dynamic:
      clist.append(init_null());
      break;

    case K::T_unresolved:
    case K::T_typeaccess:
    case K::T_reifiedtype:
      // These type structures should always be resolved
      always_assert(false);
    }
  }

  return clist.toArray();
}

ALWAYS_INLINE
ActRec* getParentFrame(const ActRec* ar, Offset* prevPc = nullptr) {
  return g_context->getPrevVMStateSkipFrame(ar, prevPc);
}

void addFramePointers(const ActRec* ar, Array& frameinfo, bool isCall) {
  if ((g_context->m_setprofileFlags & EventHook::ProfileFramePointers) == 0) {
    return;
  }
  if (frameinfo.isNull()) {
    frameinfo = Array::CreateDict();
  }

  if (isCall) {
    auto this_ = ar->func()->cls() && ar->hasThis() ?
      ar->getThis() : nullptr;
    frameinfo.set(s_this_ptr, Variant(intptr_t(this_)));

    if ((g_context->m_setprofileFlags & EventHook::ProfileThisObject) != 0
        && !RuntimeOption::SetProfileNullThisObject && this_) {
      frameinfo.set(s_this_obj, Variant(this_));
    }
  }

  frameinfo.set(s_frame_ptr, Variant(intptr_t(ar)));
  ActRec* parent_ar = getParentFrame(ar);
  if (parent_ar != nullptr) {
    frameinfo.set(s_parent_frame_ptr, Variant(intptr_t(parent_ar)));
  }
}

void addFileLine(const ActRec* ar, Array& frameinfo) {
  if ((g_context->m_setprofileFlags & EventHook::ProfileFileLine) != 0) {
    Offset offset;
    ActRec* parent_ar = getParentFrame(ar, &offset);
    if (parent_ar != nullptr) {
      frameinfo.set(s_file, Variant(const_cast<StringData*>(parent_ar->func()->filename())));
      frameinfo.set(s_line, Variant(parent_ar->func()->getLineNumber(offset)));
    }
  }
}

inline bool isResumeAware() {
  return (g_context->m_setprofileFlags & EventHook::ProfileResumeAware) != 0;
}

void runInternalEventHook(const ActRec* ar,
                          ExecutionContext::InternalEventHook event_type) {
  if (g_context->m_internalEventHookCallback != nullptr) {
    g_context->m_internalEventHookCallback(ar, event_type);
  }
}

void runUserProfilerOnFunctionEnter(const ActRec* ar, bool isResume) {
  if ((g_context->m_setprofileFlags & EventHook::ProfileEnters) == 0) {
    return;
  }

  VMRegAnchor _;
  ExecutingSetprofileCallbackGuard guard;

  CallCtx ctx;
  vm_decode_function(g_context->m_setprofileCallback, ctx);
  auto const func = ctx.func;
  if (!func) return;

  Array frameinfo;
  {
    frameinfo = Array::attach(ArrayData::CreateDict());
    if (!isResume) {
      // Add arguments only if this is a function call.
      frameinfo.set(s_args, hhvm_get_frame_args(ar));
    }
  }

  addFramePointers(ar, frameinfo, !isResume);
  if (!isResume) {
    addFileLine(ar, frameinfo);
  }

  if (!isResume && ar->func()->hasReifiedGenerics()) {
    // Add reified generics only if this is a function call.
    frameinfo.set(s_reified_classes, getReifiedClasses(ar));
  }

  const auto params = make_vec_array(
    (isResume && isResumeAware()) ? s_resume : s_enter,
    StrNR{ar->func()->fullNameWithClosureName()},
    frameinfo
  );

  ImplicitContext::Saver s;
  g_context->invokeFunc(func, params, ctx.this_, ctx.cls,
                        RuntimeCoeffects::defaults(), ctx.dynamic);
}

void runUserProfilerOnFunctionExit(const ActRec* ar, const TypedValue* retval,
                                   ObjectData* exception, bool isSuspend) {
  if ((g_context->m_setprofileFlags & EventHook::ProfileExits) == 0) {
    return;
  }

  VMRegAnchor _;
  ExecutingSetprofileCallbackGuard guard;

  CallCtx ctx;
  vm_decode_function(g_context->m_setprofileCallback, ctx);
  auto const func = ctx.func;
  if (!func) return;

  Array frameinfo;
  {
    if (retval) {
      frameinfo = make_dict_array(s_return, tvAsCVarRef(retval));
    } else if (exception) {
      frameinfo = make_dict_array(s_exception, Variant{exception});
    }
  }
  addFramePointers(ar, frameinfo, false);

  const auto params = make_vec_array(
    (isSuspend && isResumeAware()) ? s_suspend : s_exit,
    StrNR{ar->func()->fullNameWithClosureName()},
    frameinfo
  );

  ImplicitContext::Saver s;
  g_context->invokeFunc(func, params, ctx.this_, ctx.cls,
                        RuntimeCoeffects::defaults(), ctx.dynamic);
}

static Variant call_intercept_handler(
  const Variant& function,
  const Variant& called,
  const Variant& called_on,
  Array& args,
  ActRec* ar
) {
  CallCtx callCtx;
  vm_decode_function(function, callCtx);
  auto f = callCtx.func;
  if (!f) return uninit_null();

  auto const okay = [&] {
    if (f->numInOutParams() != 1) return false;
    return f->params().size() >= 3 && f->isInOut(2);
  }();

  if (!okay) {
    raise_error(
      "fb_intercept2 used with an inout handler with a bad signature "
      "(expected parameter three to be inout)"
    );
  }

  args = hhvm_get_frame_args(ar);

  VecInit par{3u};
  par.append(called);
  par.append(called_on);
  par.append(args);

  ImplicitContext::Saver s;
  auto ret = Variant::attach(
    g_context->invokeFunc(f, par.toVariant(), callCtx.this_, callCtx.cls,
                          RuntimeCoeffects::defaults(), callCtx.dynamic)
  );

  auto& arr = ret.asCArrRef();
  if (arr[1].isArray()) args = arr[1].toArray();
  return arr[0];
}

const StaticString
  s_callback("callback"),
  s_prepend_this("prepend_this");

static Variant call_intercept_handler_callback(
  ActRec* ar,
  const Variant& function,
  bool prepend_this,
  bool readonly_return
) {
  CallCtx callCtx;
  auto const origCallee = ar->func();
  vm_decode_function(function, callCtx, DecodeFlags::Warn, true);
  auto f = callCtx.func;
  if (!f) return uninit_null();
  if (origCallee->hasReifiedGenerics() ^ f->hasReifiedGenerics()) {
    SystemLib::throwRuntimeExceptionObject(
      Variant("Mismatch between reifiedness of callee and callback"));
  }
  if (f->takesInOutParams()) {
    //TODO(T52751806): Support inout arguments on fb_intercept2 callback
    SystemLib::throwRuntimeExceptionObject(
      Variant("The callback for fb_intercept2 cannot have inout parameters"));
  }

  auto const args = [&]{
    auto const curArgs = hhvm_get_frame_args(ar);
    VecInit args(prepend_this + curArgs.size());
    if (prepend_this) {
      auto const thiz = [&] {
        if (!origCallee->cls()) return make_tv<KindOfNull>();
        if (ar->hasThis()) return make_tv<KindOfObject>(ar->getThis());
        return make_tv<KindOfClass>(ar->getClass());
      }();
      args.append(thiz);
    }
    IterateV(curArgs.get(), [&](TypedValue v) { args.append(v); });
    return args.toArray();
  }();
  auto reifiedGenerics = [&] {
    if (!origCallee->hasReifiedGenerics()) return Array();
    // Reified generics is the first non param local
    auto const generics = frame_local(ar, origCallee->numParams());
    assertx(tvIsVec(generics));
    return Array(val(generics).parr);
  }();
  ImplicitContext::Saver s;
  return Variant::attach(
    g_context->invokeFunc(f, args, callCtx.this_, callCtx.cls,
                          RuntimeCoeffects::defaults(),
                          callCtx.dynamic, false, false, readonly_return,
                          std::move(reifiedGenerics))
  );
}

} // namespace

bool EventHook::RunInterceptHandler(ActRec* ar) {
  assertx(!isResumed(ar));

  const Func* func = ar->func();
  if (LIKELY(!func->maybeIntercepted())) return true;
  assertx(func->isInterceptable());

  Variant* h = get_intercept_handler(func);
  if (!h) return true;

  VMRegAnchor _;

  SCOPE_FAIL {
    // Callee frame already decrefd.
    if (vmfp() != ar) return;

    // Decref the locals of the callee frame, signalling to the unwinder that
    // the frame is already being exited and exception handlers should not be
    // invoked.
    ar->setLocalsDecRefd();
    frame_free_locals_inl_no_hook(ar, ar->func()->numFuncEntryInputs());
  };

  PC savePc = vmpc();

  auto done = true;
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

  Array args;
  VarNR called(ar->func()->fullName());

  Variant ret = call_intercept_handler(*h, called, called_on, args, ar);

  if (!ret.isArray()) {
    SystemLib::throwRuntimeExceptionObject(
      Variant("fb_intercept2 requires a darray to be returned"));
  }
  assertx(ret.isArray());
  auto const retArr = ret.toDict();
  if (retArr.exists(s_value)) {
    ret = retArr[s_value];
  } else if (retArr.exists(s_callback)) {
    bool prepend_this = retArr.exists(s_prepend_this) ?
      retArr[s_prepend_this].toBoolean() : false;
    bool readonly_return = func->attrs() & AttrReadonlyReturn;
    ret = call_intercept_handler_callback(
      ar,
      retArr[s_callback],
      prepend_this,
      readonly_return
    );
  } else {
    // neither value or callback are present, call the original function
    done = false;
  }

  if (done) {
    auto const sfp = ar->sfp();
    auto const callOff = ar->callOffset();

    Stack& stack = vmStack();
    auto const trim = [&] {
      ar->setLocalsDecRefd();
      frame_free_locals_inl_no_hook(ar, ar->func()->numFuncEntryInputs());

      // Tear down the callee frame, then push the return value.
      stack.trim((TypedValue*)(ar + 1));

      // Until the caller frame is loaded, remove references to the dead ActRec
      vmfp() = nullptr;
      vmpc() = nullptr;
    };

    if (UNLIKELY(func->takesInOutParams())) {
      assertx(!args.isNull());
      uint32_t count = func->numInOutParams();

      trim(); // discard the callee frame before pushing inout values
      auto start = stack.topTV();
      auto const end = start + count;

      auto push = [&] (TypedValue v) {
        assertx(start < end);
        tvIncRefGen(v);
        *start++ = v;
      };

      uint32_t param = 0;
      IterateKV(args.get(), [&] (TypedValue, TypedValue v) {
        if (param >= func->numParams() || !func->isInOut(param++)) {
          return;
        }
        push(v);
      });

      while (start < end) push(make_tv<KindOfNull>());
    } else {
      trim(); // nothing to do throw away the frame
    }

    tvDup(*ret.asTypedValue(), *stack.allocTV());
    stack.topTV()->m_aux.u_asyncEagerReturnFlag = 0;

    // Return to the caller or exit VM.
    vmfp() = sfp;
    vmpc() = LIKELY(sfp != nullptr)
      ? skipCall(sfp->func()->entry() + callOff)
      : nullptr;
    return false;
  }
  vmfp() = ar;
  vmpc() = savePc;

  return true;
}

static bool shouldLog(const Func* /*func*/) {
  return RID().logFunctionCalls();
}

void logCommon(StructuredLogEntry sample, const ActRec* ar, ssize_t flags) {
  sample.setInt("flags", flags);
  sample.setInt("func_id", ar->func()->getFuncId().toInt());
  sample.setStr("thread_mode",
    ServerStats::ThreadModeString(ServerStats::GetThreadMode()));
  sample.setStr("func_name", ar->func()->name()->data());
  sample.setStr("func_full_name", ar->func()->fullName()->data());
  sample.setStr("func_filename", ar->func()->filename()->data());
  addBacktraceToStructLog(
    createBacktrace(BacktraceArgs()), sample
  );
  sample.setInt("sequence_id", (*rl_func_sequence_id)++);
  StructuredLog::log("hhvm_function_calls2", sample);
}

void EventHook::onFunctionEnter(const ActRec* ar, int funcType,
                                ssize_t flags, bool isResume) {
  if (flags & HeapSamplingFlag) {
    gather_alloc_stack(/* skipTop */ true);
  }

  // User profiler
  if (flags & EventHookFlag) {
    if (shouldRunUserProfiler(ar->func())) {
      runUserProfilerOnFunctionEnter(ar, isResume);
    }
    runInternalEventHook(ar, isResume
                                 ? ExecutionContext::InternalEventHook::Resume
                                 : ExecutionContext::InternalEventHook::Call);
    if (shouldLog(ar->func())) {
      StructuredLogEntry sample;
      sample.setStr("event_name", "function_enter");
      sample.setInt("func_type", funcType);
      sample.setInt("is_resume", isResume);
      logCommon(sample, ar, flags);
    }
  }

  // Debugger hook
  if (flags & DebuggerHookFlag) {
    DEBUGGER_ATTACHED_ONLY(phpDebuggerFuncEntryHook(ar));
  }
}

void EventHook::onFunctionExit(const ActRec* ar, const TypedValue* retval,
                               bool unwind, ObjectData* phpException,
                               size_t flags, bool isSuspend,
                               EventHook::Source sourceType) {
  // Inlined calls normally skip the function enter and exit events. If we
  // side exit in an inlined callee, we short-circuit here in order to skip
  // exit events that could unbalance the call stack.
  if (RuntimeOption::EvalJit && ar->isInlined()) {
    return;
  }

  // Xenon
  if (flags & XenonSignalFlag) {
    if (Strobelight::active()) {
      Strobelight::getInstance().log(Xenon::ExitSample);
    } else {
      Xenon::getInstance().log(Xenon::ExitSample, sourceType);
    }
  }

  if (flags & HeapSamplingFlag) {
    gather_alloc_stack();
  }

  // Run callbacks only if it's safe to do so, i.e., not when
  // there's a pending exception or we're unwinding from a C++ exception.
  // It's not safe to run callbacks when we are in the middle of resolving a
  // constant since we maintain a static array with sentinel bits to detect
  // recursively defined constants which could be modified by a callback.
  if (RequestInfo::s_requestInfo->m_pendingException == nullptr
      && (!unwind || phpException)
      && ar->func()->name() != s_86cinit.get()) {

    // Memory Threhsold
    if (flags & MemThresholdFlag) {
      DoMemoryThresholdCallback();
    }
    // Time Thresholds
    if (flags & TimedOutFlag) {
      RID().invokeUserTimeoutCallback();
    }

    // Interval timer
    if (flags & IntervalTimerFlag) {
      IntervalTimer::RunCallbacks(IntervalTimer::ExitSample);
    }
  }

  // User profiler
  if (flags & EventHookFlag) {
    if (shouldRunUserProfiler(ar->func())) {
      if (RequestInfo::s_requestInfo->m_pendingException != nullptr) {
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
    if (!unwind) {
      runInternalEventHook(ar, isSuspend
                               ? ExecutionContext::InternalEventHook::Suspend
                               : ExecutionContext::InternalEventHook::Return);
    } else {
      runInternalEventHook(ar, ExecutionContext::InternalEventHook::Unwind);
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

uint64_t EventHook::onFunctionCallJit(const ActRec* ar, int funcType) {
  auto const savedRip = ar->m_savedRip;
  if (EventHook::onFunctionCall(ar, funcType, EventHook::Source::Jit)) {
    // Not intercepted, no return address.
    return 0;
  }

  // We may have entered with the CLEAN_VERIFY state set by the JIT, but
  // the functionEnterHelper is going to return to the parent frame, bypassing
  // the JIT logic that resets this state. Set the state to DIRTY so it won't
  // leak to other C++ calls.
  regState() = VMRegState::DIRTY;

  // If we entered this frame from the interpreter, use the resumeHelper,
  // as the retHelper logic has been already performed and the frame has
  // been overwritten by the return value.
  if (isReturnHelper(savedRip)) {
    return
      reinterpret_cast<uintptr_t>(jit::tc::ustubs().resumeHelperFromInterp);
  }
  return savedRip;
}

bool EventHook::onFunctionCall(const ActRec* ar, int funcType,
                               EventHook::Source sourceType) {
  assertx(!isResumed(ar));
  auto const flags = handle_request_surprise();
  if (flags & InterceptFlag) {
    DEBUG_ONLY auto const func = ar->func();
    if (!RunInterceptHandler(const_cast<ActRec*>(ar))) {
      assertx(func->isInterceptable());
      return false;
    }
  }

  // Xenon
  if (flags & XenonSignalFlag) {
    if (Strobelight::active()) {
      Strobelight::getInstance().log(Xenon::EnterSample);
    } else {
      Xenon::getInstance().log(Xenon::EnterSample, sourceType);
    }
  }

  // It's not safe to run callbacks when we are in the middle of resolving a
  // constant since we maintain a static array with sentinel bits to detect
  // recursively defined constants which could be modified by a callback.
  if (ar->func()->name() != s_86cinit.get()) {
    // Memory Threhsold
    if (flags & MemThresholdFlag) {
      DoMemoryThresholdCallback();
    }
    // Time Thresholds
    if (flags & TimedOutFlag) {
      RID().invokeUserTimeoutCallback();
    }

    if (flags & IntervalTimerFlag) {
      IntervalTimer::RunCallbacks(IntervalTimer::EnterSample);
    }
  }

  onFunctionEnter(ar, funcType, flags, false);
  return true;
}

void EventHook::onFunctionResumeAwait(const ActRec* ar,
                                      EventHook::Source sourceType) {
  auto const flags = handle_request_surprise();

  // Xenon
  if (flags & XenonSignalFlag) {
    if (Strobelight::active()) {
      Strobelight::getInstance().log(Xenon::ResumeAwaitSample);
    } else {
      Xenon::getInstance().log(Xenon::ResumeAwaitSample, sourceType);
    }
  }

  // Memory Threhsold
  if (flags & MemThresholdFlag) {
    DoMemoryThresholdCallback();
  }
  // Time Thresholds
  if (flags & TimedOutFlag) {
    RID().invokeUserTimeoutCallback();
  }

  if (flags & IntervalTimerFlag) {
    IntervalTimer::RunCallbacks(IntervalTimer::ResumeAwaitSample);
  }

  onFunctionEnter(ar, EventHook::NormalFunc, flags, true);
}

void EventHook::onFunctionResumeYield(const ActRec* ar,
                                      EventHook::Source sourceType) {
  auto const flags = handle_request_surprise();

  // Xenon
  if (flags & XenonSignalFlag) {
    if (Strobelight::active()) {
      Strobelight::getInstance().log(Xenon::EnterSample);
    } else {
      Xenon::getInstance().log(Xenon::EnterSample, sourceType);
    }
  }

  // Memory Threhsold
  if (flags & MemThresholdFlag) {
    DoMemoryThresholdCallback();
  }
  // Time Thresholds
  if (flags & TimedOutFlag) {
    RID().invokeUserTimeoutCallback();
  }

  if (flags & IntervalTimerFlag) {
    IntervalTimer::RunCallbacks(IntervalTimer::EnterSample);
  }

  onFunctionEnter(ar, EventHook::NormalFunc, flags, true);
}

void EventHook::onFunctionSuspendAwaitEFJit(ActRec* suspending,
                                         const ActRec* resumableAR) {
  EventHook::onFunctionSuspendAwaitEF(
    suspending,
    resumableAR,
    EventHook::Source::Jit
  );
}

// Eagerly executed async function initially suspending at Await.
void EventHook::onFunctionSuspendAwaitEF(ActRec* suspending,
                                         const ActRec* resumableAR,
                                         EventHook::Source sourceType) {
  assertx(suspending->func()->isAsyncFunction());
  assertx(suspending->func() == resumableAR->func());
  assertx(isResumed(resumableAR));

  // The locals were already teleported from suspending to resumableAR.
  suspending->setLocalsDecRefd();
  suspending->trashThis();

  try {
    auto const flags = handle_request_surprise();
    onFunctionExit(resumableAR, nullptr, false, nullptr, flags, true, sourceType);

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

void EventHook::onFunctionSuspendAwaitEGJit(ActRec* suspending) {
  EventHook::onFunctionSuspendAwaitEG(suspending, EventHook::Source::Jit);
}

// Eagerly executed async generator that was resumed at Yield suspending
// at Await.
void EventHook::onFunctionSuspendAwaitEG(ActRec* suspending,
                                         EventHook::Source sourceType) {
  assertx(suspending->func()->isAsyncGenerator());
  assertx(isResumed(suspending));

  // The generator is still being executed eagerly, make it look like that.
  auto const gen = frame_async_generator(suspending);
  auto wh = gen->detachWaitHandle();
  SCOPE_EXIT { gen->attachWaitHandle(std::move(wh)); };

  try {
    auto const flags = handle_request_surprise();
    onFunctionExit(suspending, nullptr, false, nullptr, flags, true, sourceType);

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

void EventHook::onFunctionSuspendAwaitRJit(ActRec* suspending, ObjectData* child) {
  EventHook::onFunctionSuspendAwaitR(suspending, child, EventHook::Source::Jit);
}

// Async function or async generator that was resumed at Await suspending
// again at Await. The suspending frame has an associated AFWH/AGWH. Child
// is the WH we are going to block on.
void EventHook::onFunctionSuspendAwaitR(ActRec* suspending,
                                        ObjectData* child,
                                        EventHook::Source sourceType) {
  assertx(suspending->func()->isAsync());
  assertx(isResumed(suspending));
  assertx(child->isWaitHandle());
  assertx(!static_cast<c_Awaitable*>(child)->isFinished());

  auto const flags = handle_request_surprise();
  onFunctionExit(suspending, nullptr, false, nullptr, flags, true, sourceType);

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

  if (UNLIKELY(static_cast<c_Awaitable*>(child)->isFinished())) {
    SystemLib::throwInvalidOperationExceptionObject(
      "The suspend await event hook entered a new asio scheduler context and "
      "awaited the same child, revoking the reason for suspension.");
  }
}

void EventHook::onFunctionSuspendCreateContJit(ActRec* suspending,
                                            const ActRec* resumableAR) {
  EventHook::onFunctionSuspendCreateCont(
    suspending,
    resumableAR,
    EventHook::Source::Jit
  );
}

// Generator or async generator suspending initially at CreateCont.
void EventHook::onFunctionSuspendCreateCont(ActRec* suspending,
                                            const ActRec* resumableAR,
                                            EventHook::Source sourceType) {
  assertx(suspending->func()->isGenerator());
  assertx(suspending->func() == resumableAR->func());
  assertx(isResumed(resumableAR));

  // The locals were already teleported from suspending to resumableAR.
  suspending->setLocalsDecRefd();
  suspending->trashThis();

  try {
    auto const flags = handle_request_surprise();
    onFunctionExit(resumableAR, nullptr, false, nullptr, flags, true, sourceType);
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

void EventHook::onFunctionSuspendYieldJit(ActRec* suspending) {
  EventHook::onFunctionSuspendYield(suspending, EventHook::Source::Jit);
}

// Generator or async generator suspending at Yield.
void EventHook::onFunctionSuspendYield(ActRec* suspending, EventHook::Source sourceType) {
  assertx(suspending->func()->isGenerator());
  assertx(isResumed(suspending));

  auto const flags = handle_request_surprise();
  onFunctionExit(suspending, nullptr, false, nullptr, flags, true, sourceType);

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

void EventHook::onFunctionReturnJit(ActRec* ar, TypedValue retval) {
  EventHook::onFunctionReturn(ar, retval, EventHook::Source::Jit);
}

void EventHook::onFunctionReturn(ActRec* ar,
                                 TypedValue retval,
                                 EventHook::Source sourceType) {
  // The locals are already gone. Tell everyone.
  ar->setLocalsDecRefd();
  ar->trashThis();

  try {
    auto const flags = handle_request_surprise();
    onFunctionExit(ar, &retval, false, nullptr, flags, false, sourceType);

    // Async profiler
    if ((flags & AsyncEventHookFlag) && isResumed(ar) &&
        ar->func()->isAsync()) {
      auto session = AsioSession::Get();
      if (session->hasOnResumableSuccess()) {
        if (!ar->func()->isGenerator()) {
          session->onResumableSuccess(frame_afwh(ar), tvAsCVarRef(retval));
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

void EventHook::onFunctionUnwind(ActRec* ar,
                                 ObjectData* phpException) {
  // The locals are already gone. Tell everyone.
  ar->setLocalsDecRefd();
  ar->trashThis();

  // TODO(#2329497) can't handle_request_surprise() yet, unwinder unable to
  // replace fault
  auto const flags = stackLimitAndSurprise().load() & kSurpriseFlagMask;
  onFunctionExit(ar, nullptr, true, phpException, flags, false, EventHook::Source::Unwinder);
}

} // namespace HPHP
