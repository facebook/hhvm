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

#include "hphp/runtime/vm/unwind.h"

#include <boost/implicit_cast.hpp>

#include <folly/ScopeGuard.h>

#include "hphp/util/trace.h"
#include "hphp/runtime/ext/generator/ext_generator.h"
#include "hphp/runtime/ext/asio/ext_async-function-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_async-generator.h"
#include "hphp/runtime/ext/asio/ext_async-generator-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_static-wait-handle.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/vm-regs.h"

namespace HPHP {

TRACE_SET_MOD(unwind);
using boost::implicit_cast;

namespace {

//////////////////////////////////////////////////////////////////////

/*
 * Enumerates actions that should be taken by the enterVM loop after
 * unwinding an exception.
 */
enum class UnwindAction {
  /*
   * The exception was not handled in this nesting of the VM---it
   * needs to be rethrown.
   */
  Propagate,

  /*
   * The exception was either handled, or a catch or fault handler was
   * identified and the VM state has been prepared for entry to it.
   */
  ResumeVM,
};

#if (defined(DEBUG) || defined(USE_TRACE))
std::string describeFault(const Fault& f) {
  return folly::format("[user exception] {}",
                       implicit_cast<void*>(f.m_userException)).str();
}
#endif

void discardStackTemps(const ActRec* const fp,
                       Stack& stack,
                       Offset const bcOffset) {
  ITRACE(2, "discardStackTemps with fp {} sp {} pc {}\n",
         implicit_cast<const void*>(fp),
         implicit_cast<void*>(stack.top()),
         bcOffset);

  visitStackElems(
    fp, stack.top(), bcOffset,
    [&] (ActRec* ar) {
      assert(ar == reinterpret_cast<ActRec*>(stack.top()));
      if (ar->isFromFPushCtor()) {
        assert(ar->hasThis());
        ar->getThis()->setNoDestruct();
      }
      ITRACE(2, "  unwind pop AR : {}\n",
             implicit_cast<void*>(stack.top()));
      stack.popAR();
    },
    [&] (TypedValue* tv) {
      assert(tv == stack.top());
      ITRACE(2, "  unwind pop TV : {}\n",
             implicit_cast<void*>(stack.top()));
      stack.popTV();
    }
  );

  ITRACE(2, "discardStackTemps ends with sp = {}\n",
         implicit_cast<void*>(stack.top()));
}

UnwindAction checkHandlers(const EHEnt* eh,
                           const ActRec* const fp,
                           PC& pc,
                           Fault& fault) {
  auto const func = fp->m_func;
  ITRACE(1, "checkHandlers: func {} ({})\n",
         func->fullName()->data(),
         func->unit()->filepath()->data());

  for (int i = 0;; ++i) {
    // Skip the initial m_handledCount - 1 handlers that were
    // considered before.
    if (fault.m_handledCount <= i) {
      fault.m_handledCount++;
      switch (eh->m_type) {
      case EHEnt::Type::Fault:
        ITRACE(1, "checkHandlers: entering fault at {}: save {}\n",
               eh->m_fault,
               func->unit()->offsetOf(pc));
        pc = func->unit()->entry() + eh->m_fault;
        DEBUGGER_ATTACHED_ONLY(phpDebuggerExceptionHandlerHook());
        return UnwindAction::ResumeVM;
      case EHEnt::Type::Catch:
        // Note: we skip catch clauses if we have a pending C++ exception
        // as part of our efforts to avoid running more PHP code in the
        // face of such exceptions.
        if (ThreadInfo::s_threadInfo->m_pendingException == nullptr) {
          auto const obj = fault.m_userException;
          for (auto& idOff : eh->m_catches) {
            ITRACE(1, "checkHandlers: catch candidate {}\n", idOff.second);
            auto handler = func->unit()->at(idOff.second);
            auto const cls = Unit::lookupClass(
              func->unit()->lookupNamedEntityId(idOff.first)
            );
            if (!cls || !obj->instanceof(cls)) continue;

            ITRACE(1, "checkHandlers: entering catch at {}\n", idOff.second);
            pc = handler;
            DEBUGGER_ATTACHED_ONLY(phpDebuggerExceptionHandlerHook());
            return UnwindAction::ResumeVM;
          }
        }
        break;
      }
    }
    if (eh->m_parentIndex != -1) {
      eh = &func->ehtab()[eh->m_parentIndex];
    } else {
      break;
    }
  }
  return UnwindAction::Propagate;
}

/**
 * Discard the current frame, assuming that a PHP exception given in
 * phpException argument, or C++ exception (phpException == nullptr)
 * is being thrown. Returns an exception to propagate, or nulltpr
 * if the VM execution should be resumed.
 */
ObjectData* tearDownFrame(ActRec*& fp, Stack& stack, PC& pc,
                          ObjectData* phpException) {
  auto const func = fp->func();
  auto const curOp = *reinterpret_cast<const Op*>(pc);
  auto const prevFp = fp->sfp();
  auto const soff = fp->m_soff;

  ITRACE(1, "tearDownFrame: {} ({})\n",
         func->fullName()->data(),
         func->unit()->filepath()->data());
  ITRACE(1, "  fp {} prevFp {}\n",
         implicit_cast<void*>(fp),
         implicit_cast<void*>(prevFp));

  // When throwing from a constructor, we normally want to avoid running the
  // destructor on an object that hasn't been fully constructed yet. But if
  // we're unwinding through the constructor's RetC, the constructor has
  // logically finished and we're unwinding for some internal reason (timeout
  // or user profiler, most likely). More importantly, fp->m_this may have
  // already been destructed and/or overwritten due to sharing space with
  // fp->m_r.
  if (fp->isFromFPushCtor() && fp->hasThis() && curOp != OpRetC) {
    fp->getThis()->setNoDestruct();
  }

  auto const decRefLocals = [&] {
    /*
     * It is possible that locals have already been decref'd.
     *
     * Here's why:
     *
     *   - If a destructor for any of these things throws a php
     *     exception, it's swallowed at the dtor boundary and we keep
     *     running php.
     *
     *   - If the destructor for any of these things throws a fatal,
     *     it's swallowed, and we set surprise flags to throw a fatal
     *     from now on.
     *
     *   - If the second case happened and we have to run another
     *     destructor, its enter hook will throw, but it will be
     *     swallowed again.
     *
     *   - Finally, the exit hook for the returning function can
     *     throw, but this happens last so everything is destructed.
     *
     *   - When that happens, exit hook sets localsDecRefd flag.
     */
    if (!fp->localsDecRefd()) {
      try {
        // Note that we must convert locals and the $this to
        // uninit/zero during unwind.  This is because a backtrace
        // from another destructing object during this unwind may try
        // to read them.
        frame_free_locals_unwind(fp, func->numLocals(), phpException);
      } catch (...) {}
    }
  };

  if (LIKELY(!fp->resumed())) {
    decRefLocals();
    if (UNLIKELY(func->isAsyncFunction()) && phpException) {
      // If in an eagerly executed async function, wrap the user exception
      // into a failed StaticWaitHandle and return it to the caller.
      auto const waitHandle = c_StaticWaitHandle::CreateFailed(phpException);
      phpException = nullptr;
      stack.ndiscard(func->numSlotsInFrame());
      stack.ret();
      assert(stack.topTV() == &fp->m_r);
      cellCopy(make_tv<KindOfObject>(waitHandle), fp->m_r);
    } else {
      // Free ActRec.
      stack.ndiscard(func->numSlotsInFrame());
      stack.discardAR();
    }
  } else if (func->isAsyncFunction()) {
    auto const waitHandle = frame_afwh(fp);
    if (phpException) {
      // Handle exception thrown by async function.
      decRefLocals();
      waitHandle->fail(phpException);
      phpException = nullptr;
    } else if (waitHandle->isRunning()) {
      // Let the C++ exception propagate. If the current frame represents async
      // function that is running, mark it as abruptly interrupted. Some opcodes
      // like Await may change state of the async function just before exit hook
      // decides to throw C++ exception.
      decRefLocals();
      waitHandle->failCpp();
    }
  } else if (func->isAsyncGenerator()) {
    auto const gen = frame_async_generator(fp);
    if (phpException) {
      // Handle exception thrown by async generator.
      decRefLocals();
      auto eagerResult = gen->fail(phpException);
      phpException = nullptr;
      if (eagerResult) {
        stack.pushObjectNoRc(eagerResult);
      }
    } else if (gen->isEagerlyExecuted() || gen->getWaitHandle()->isRunning()) {
      // Fail the async generator and let the C++ exception propagate.
      decRefLocals();
      gen->failCpp();
    }
  } else if (func->isNonAsyncGenerator()) {
    // Mark the generator as finished.
    decRefLocals();
    frame_generator(fp)->fail();
  } else {
    not_reached();
  }

  /*
   * At the final ActRec in this nesting level.
   */
  if (UNLIKELY(!prevFp)) {
    pc = nullptr;
    fp = nullptr;
    return phpException;
  }

  assert(stack.isValidAddress(reinterpret_cast<uintptr_t>(prevFp)) ||
         prevFp->resumed());
  auto const prevOff = soff + prevFp->func()->base();
  pc = prevFp->func()->unit()->at(prevOff);
  fp = prevFp;
  return phpException;
}

const StaticString s_previous("previous");

void chainFaultObjects(ObjectData* top, ObjectData* prev) {
  while (true) {
    auto const lookup = top->getProp(
      SystemLib::s_ExceptionClass,
      s_previous.get()
    );
    auto const top_tv = lookup.prop;
    assert(top_tv != nullptr);

    assert(top_tv->m_type != KindOfUninit && lookup.accessible);
    if (top_tv->m_type != KindOfObject ||
        !top_tv->m_data.pobj->instanceof(SystemLib::s_ExceptionClass)) {
      // Since we are overwriting, decref.
      tvRefcountedDecRef(top_tv);
      // Objects held in m_faults are not refcounted, therefore we need to
      // increase the ref count here.
      top_tv->m_type = KindOfObject;
      top_tv->m_data.pobj = prev;
      prev->incRefCount();
      break;
    }
    top = top_tv->m_data.pobj;
  }
}

bool chainFaults(Fault& fault) {
  always_assert(!g_context->m_faults.empty());
  auto& faults = g_context->m_faults;
  faults.pop_back();
  if (faults.empty()) {
    faults.push_back(fault);
    return false;
  }
  auto prev = faults.back();
  if (fault.m_raiseNesting == prev.m_raiseNesting &&
      fault.m_raiseFrame == prev.m_raiseFrame) {
    fault.m_raiseOffset = prev.m_raiseOffset;
    fault.m_handledCount = prev.m_handledCount;
    chainFaultObjects(fault.m_userException, prev.m_userException);
    faults.pop_back();
    faults.push_back(fault);
    return true;
  }
  faults.push_back(fault);
  return false;
}

const StaticString s_hphpd_break("hphpd_break");
const StaticString s_fb_enable_code_coverage("fb_enable_code_coverage");
const StaticString s_xdebug_start_code_coverage("xdebug_start_code_coverage");

//////////////////////////////////////////////////////////////////////

}

/*
 * Unwinding proceeds as follows:
 *
 *   - Discard all evaluation stack temporaries (including pre-live
 *     activation records).
 *
 *   - Check if the faultOffset that raised the exception is inside a
 *     protected region, if so, if it can handle the Fault resume the
 *     VM at the handler.
 *
 *   - Check if we are handling user exception in an eagerly executed
 *     async function. If so, pop its frame, wrap the exception into
 *     failed StaticWaitHandle object, leave it on the stack as
 *     a return value from the async function and resume VM.
 *
 *   - Failing any of the above, pop the frame for the current
 *     function.  If the current function was the last frame in the
 *     current VM nesting level, rethrow the exception, otherwise go
 *     to the first step and repeat this process in the caller's
 *     frame.
 *
 * Note: it's important that the unwinder makes a copy of the Fault
 * it's currently operating on, as the underlying faults vector may
 * reallocate due to nested exception handling.
 */
void unwindPhp() {
  assert(!g_context->m_faults.empty());
  auto& fp = vmfp();
  auto& stack = vmStack();
  auto& pc = vmpc();
  auto fault = g_context->m_faults.back();

  ITRACE(1, "entering unwinder for fault: {}\n", describeFault(fault));
  SCOPE_EXIT {
    ITRACE(1, "leaving unwinder for fault: {}\n", describeFault(fault));
  };

  do {
    bool discard = false;
    if (fault.m_raiseOffset == kInvalidOffset) {
      /*
       * This block executes whenever we want to treat the fault as if
       * it was freshly thrown. Freshly thrown faults either were never
       * previosly seen by the unwinder OR were propagated from the
       * previous frame. In such a case, we fill in the fields with
       * the information from the current frame.
       */
      always_assert(fault.m_raiseNesting == kInvalidNesting);
      // Nesting is set to the current VM nesting.
      fault.m_raiseNesting = g_context->m_nestedVMs.size();
      // Raise frame is set to the current frame
      fault.m_raiseFrame = fp;
      // Raise offset is set to the offset of the current PC.
      fault.m_raiseOffset = fp->m_func->unit()->offsetOf(pc);
      // No handlers were yet examined for this fault.
      fault.m_handledCount = 0;
      // We will be also discarding stack temps.
      discard = true;
    }

    ITRACE(1, "unwind: func {}, raiseOffset {} fp {}\n",
           fp->m_func->name()->data(),
           fault.m_raiseOffset,
           implicit_cast<void*>(fp));

    assert(fault.m_raiseNesting != kInvalidNesting);
    assert(fault.m_raiseFrame != nullptr);
    assert(fault.m_raiseOffset != kInvalidOffset);

    /*
     * If the handledCount is non-zero, we've already seen this fault once
     * while unwinding this frema, and popped all eval stack
     * temporaries the first time it was thrown (before entering a
     * fault funclet).  When the Unwind instruction was executed in
     * the funclet, the eval stack must have been left empty again.
     *
     * (We have to skip discardStackTemps in this case because it will
     * look for FPI regions and assume the stack offsets correspond to
     * what the FPI table expects.)
     */
    if (discard) {
      discardStackTemps(fp, stack, fault.m_raiseOffset);
    }

    do {
      const EHEnt* eh = fp->m_func->findEH(fault.m_raiseOffset);
      if (eh != nullptr) {
        switch (checkHandlers(eh, fp, pc, fault)) {
        case UnwindAction::ResumeVM:
          // We've kept our own copy of the Fault, because m_faults may
          // change if we have a reentry during unwinding.  When we're
          // ready to resume, we need to replace the fault to reflect
          // any state changes we've made (handledCount, etc).
          g_context->m_faults.back() = fault;
          return;
        case UnwindAction::Propagate:
          break;
        }
      }
      // If we came here, it means that no further EHs were found for
      // the current fault offset and handledCount. This means we are
      // allowed to chain the current exception with the previous
      // one (if it exists). This is because the current exception
      // escapes the exception handler where it was thrown.
    } while (chainFaults(fault));

    // We found no more handlers in this frame, so the nested fault
    // count starts over for the caller frame.
    fault.m_userException = tearDownFrame(fp, stack, pc, fault.m_userException);
    if (fault.m_userException == nullptr) {
      g_context->m_faults.pop_back();
      return;
    }

    // Once we are done with EHs for the current frame we restore
    // default values for the fields inside Fault. This makes sure
    // that on another loop pass we will treat the fault just
    // as if it was freshly thrown.
    fault.m_raiseNesting = kInvalidNesting;
    fault.m_raiseFrame = nullptr;
    fault.m_raiseOffset = kInvalidOffset;
    fault.m_handledCount = 0;
    g_context->m_faults.back() = fault;
  } while (fp);

  ITRACE(1, "unwind: reached the end of this nesting's ActRec chain\n");
  g_context->m_faults.pop_back();

  Object obj = Object::attach(fault.m_userException);
  throw obj;
}

void unwindPhp(ObjectData* phpException) {
  Fault fault;
  fault.m_userException = phpException;
  fault.m_userException->incRefCount();
  g_context->m_faults.push_back(fault);

  unwindPhp();
}

/*
 * Unwinding of C++ exceptions proceeds as follows:
 *
 *   - Discard all PHP exceptions pending for this frame.
 *
 *   - Discard all evaluation stack temporaries (including pre-live
 *     activation records).
 *
 *   - Pop the frame for the current function.  If the current function
 *     was the last frame in the current VM nesting level, re-throw
 *     the C++ exception, otherwise go to the first step and repeat
 *     this process in the caller's frame.
 */
void unwindCpp(Exception* exception) {
  auto& fp = vmfp();
  auto& stack = vmStack();
  auto& pc = vmpc();

  assert(!g_context->m_unwindingCppException);
  g_context->m_unwindingCppException = true;
  ITRACE(1, "entering unwinder for C++ exception: {}\n",
         implicit_cast<void*>(exception));
  SCOPE_EXIT {
    assert(g_context->m_unwindingCppException);
    g_context->m_unwindingCppException = false;
    ITRACE(1, "leaving unwinder for C++ exception: {}\n",
           implicit_cast<void*>(exception));
  };

  do {
    auto const offset = fp->func()->unit()->offsetOf(pc);

    ITRACE(1, "unwindCpp: func {}, raiseOffset {} fp {}\n",
           fp->func()->name()->data(),
           offset,
           implicit_cast<void*>(fp));

    // Discard all PHP exceptions pending for this frame
    auto& faults = g_context->m_faults;
    while (UNLIKELY(!faults.empty()) &&
           faults.back().m_raiseFrame == fp &&
           faults.back().m_raiseNesting == g_context->m_nestedVMs.size()) {
      decRefObj(faults.back().m_userException);
      faults.pop_back();
    }

    // Discard stack temporaries
    discardStackTemps(fp, stack, offset);

    // Discard the frame
    DEBUG_ONLY auto const phpException = tearDownFrame(fp, stack, pc, nullptr);
    assert(phpException == nullptr);
  } while (fp);

  // Propagate the C++ exception to the outer VM nesting
  exception->throwException();
}

void unwindBuiltinFrame() {
  auto& stack = vmStack();
  auto& fp = vmfp();

  assert(fp->m_func->name()->isame(s_hphpd_break.get()) ||
         fp->m_func->name()->isame(s_fb_enable_code_coverage.get()) ||
         fp->m_func->name()->isame(s_xdebug_start_code_coverage.get()));

  // Free any values that may be on the eval stack.  We know there
  // can't be FPI regions and it can't be a generator body because
  // it's a builtin frame.
  const int numSlots = fp->m_func->numSlotsInFrame();
  auto const evalTop = reinterpret_cast<TypedValue*>(vmfp()) - numSlots;
  while (stack.topTV() < evalTop) {
    stack.popTV();
  }

  // Free the locals and VarEnv if there is one
  auto rv = make_tv<KindOfNull>();
  frame_free_locals_inl(fp, fp->m_func->numLocals(), &rv);

  // Tear down the frame
  Offset pc = -1;
  ActRec* sfp = g_context->getPrevVMState(fp, &pc);
  assert(pc != -1);
  fp = sfp;
  vmpc() = fp->m_func->unit()->at(pc);
  stack.ndiscard(numSlots);
  stack.discardAR();
  stack.pushNull(); // return value
}

//////////////////////////////////////////////////////////////////////

}
