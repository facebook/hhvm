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
#include "hphp/runtime/vm/unwind.h"

#include <boost/implicit_cast.hpp>

#include "folly/ScopeGuard.h"

#include "hphp/util/trace.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/debugger-hook.h"

namespace HPHP {

TRACE_SET_MOD(unwind);
using boost::implicit_cast;

namespace {

//////////////////////////////////////////////////////////////////////
#if (defined(DEBUG) || defined(USE_TRACE))
std::string describeFault(const Fault& f) {
  switch (f.m_faultType) {
  case Fault::Type::UserException:
    return folly::format("[user exception] {}",
                         implicit_cast<void*>(f.m_userException)).str();
  case Fault::Type::CppException:
    return folly::format("[cpp exception] {}",
                         implicit_cast<void*>(f.m_cppException)).str();
  }
  not_reached();
}
#endif

void discardStackTemps(const ActRec* const fp,
                       Stack& stack,
                       Offset const bcOffset) {
  FTRACE(2, "discardStackTemps with fp {} sp {} pc {}\n",
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
      FTRACE(2, "  unwind pop AR : {}\n",
             implicit_cast<void*>(stack.top()));
      stack.popAR();
    },
    [&] (TypedValue* tv) {
      assert(tv == stack.top());
      FTRACE(2, "  unwind pop TV : {}\n",
             implicit_cast<void*>(stack.top()));
      stack.popTV();
    }
  );

  FTRACE(2, "discardStackTemps ends with sp = {}\n",
         implicit_cast<void*>(stack.top()));
}

UnwindAction checkHandlers(const EHEnt* eh,
                           const ActRec* const fp,
                           PC& pc,
                           Fault& fault) {
  auto const func = fp->m_func;
  FTRACE(1, "checkHandlers: func {} ({})\n",
         func->fullName()->data(),
         func->unit()->filepath()->data());

  // Always blindly propagate on fatal exception since those are
  // unrecoverable anyway.
  if (fault.m_faultType == Fault::Type::CppException) {
    return UnwindAction::Propagate;
  }

  for (int i = 0;; ++i) {
    // Skip the initial m_handledCount - 1 handlers that were
    // considered before.
    if (fault.m_handledCount <= i) {
      fault.m_handledCount++;
      switch (eh->m_type) {
      case EHEnt::Type::Fault:
        FTRACE(1, "checkHandlers: entering fault at {}: save {}\n",
               eh->m_fault,
               func->unit()->offsetOf(pc));
        pc = func->unit()->entry() + eh->m_fault;
        DEBUGGER_ATTACHED_ONLY(phpDebuggerExceptionHandlerHook());
        return UnwindAction::ResumeVM;
      case EHEnt::Type::Catch:
        // Note: we skip catch clauses if we have a pending C++ exception
        // as part of our efforts to avoid running more PHP code in the
        // face of such exceptions.
        if (fault.m_faultType == Fault::Type::UserException &&
            ThreadInfo::s_threadInfo->m_pendingException == nullptr) {
          auto const obj = fault.m_userException;
          for (auto& idOff : eh->m_catches) {
            FTRACE(1, "checkHandlers: catch candidate {}\n", idOff.second);
            auto handler = func->unit()->at(idOff.second);
            auto const cls = Unit::lookupClass(
              func->unit()->lookupNamedEntityId(idOff.first)
            );
            if (!cls || !obj->instanceof(cls)) continue;

            FTRACE(1, "checkHandlers: entering catch at {}\n", idOff.second);
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

void tearDownFrame(ActRec*& fp, Stack& stack, PC& pc) {
  auto const func = fp->m_func;
  auto const curOp = *reinterpret_cast<const Op*>(pc);
  auto const unwindingGeneratorFrame = func->isGenerator();
  auto const unwindingReturningFrame = curOp == OpRetC || curOp == OpRetV;
  auto const prevFp = fp->arGetSfp();
  auto const soff = fp->m_soff;

  FTRACE(1, "tearDownFrame: {} ({})\n  fp {} prevFp {}\n",
         func->fullName()->data(),
         func->unit()->filepath()->data(),
         implicit_cast<void*>(fp),
         implicit_cast<void*>(prevFp));

  // When throwing from a constructor, we normally want to avoid running the
  // destructor on an object that hasn't been fully constructed yet. But if
  // we're unwinding through the constructor's RetC, the constructor has
  // logically finished and we're unwinding for some internal reason (timeout
  // or user profiler, most likely). More importantly, fp->m_this may have
  // already been destructed and/or overwritten due to sharing space with
  // fp->m_r.
  if (!unwindingReturningFrame && fp->isFromFPushCtor() && fp->hasThis()) {
    fp->getThis()->setNoDestruct();
  }

  // A generator's locals don't live on this stack.
  if (LIKELY(!unwindingGeneratorFrame)) {
    /*
     * If we're unwinding through a frame that's returning, it's only
     * possible that its locals have already been decref'd.
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
     */
    if (!unwindingReturningFrame) {
      try {
        // Note that we must convert locals and the $this to
        // uninit/zero during unwind.  This is because a backtrace
        // from another destructing object during this unwind may try
        // to read them.
        frame_free_locals_unwind(fp, func->numLocals());
      } catch (...) {}
    }
    stack.ndiscard(func->numSlotsInFrame());
    stack.discardAR();
  } else {
    // The generator's locals will be cleaned up when the Continuation
    // object is destroyed. But we are leaving the generator function
    // now, so signal that to anyone who cares.
    try {
      EventHook::FunctionExit(fp);
    } catch (...) {} // As above, don't let new exceptions out of unwind.
  }

  /*
   * At the final ActRec in this nesting level.  We don't need to set
   * pc and fp since we're about to re-throw the exception.  And we
   * don't want to dereference prefFp since we just popped it.
   */
  if (prevFp == fp) return;

  assert(stack.isValidAddress(reinterpret_cast<uintptr_t>(prevFp)) ||
         prevFp->m_func->isGenerator());
  auto const prevOff = soff + prevFp->m_func->base();
  pc = prevFp->m_func->unit()->at(prevOff);
  fp = prevFp;
}

void chainFaultObjects(ObjectData* top, ObjectData* prev) {
  static const StaticString nProp("previous");
  bool visible, accessible, unset;
  while (true) {
    TypedValue* top_tv = top->getProp(
      SystemLib::s_ExceptionClass,
      nProp.get(),
      visible, accessible, unset
    );
    assert(visible && accessible && !unset);
    if (top_tv->m_type != KindOfObject ||
        !top_tv->m_data.pobj->instanceof(
                                SystemLib::s_ExceptionClass)) {
      // Since we are overwriting, decref.
      tvRefcountedDecRef(top_tv);
      // Objects held in m_faults are not refcounted, therefore
      // we need to increase the ref count here.
      top_tv->m_type = KindOfObject;
      top_tv->m_data.pobj = prev;
      prev->incRefCount();
      break;
    }
    top = top_tv->m_data.pobj;
  }
}

bool chainFaults(Fault& fault) {
  always_assert(!g_vmContext->m_faults.empty());
  auto& faults = g_vmContext->m_faults;
  faults.pop_back();
  if (faults.empty()) {
    faults.push_back(fault);
    return false;
  }
  auto prev = faults.back();
  if (fault.m_faultType == Fault::Type::CppException &&
      fault.m_raiseNesting == prev.m_raiseNesting &&
      fault.m_raiseFrame == prev.m_raiseFrame) {
    fault.m_raiseOffset = prev.m_raiseOffset;
    fault.m_handledCount = prev.m_handledCount;
    faults.pop_back();
    faults.push_back(fault);
    return true;
  }
  if (fault.m_faultType == Fault::Type::UserException &&
             fault.m_raiseNesting == prev.m_raiseNesting &&
             fault.m_raiseFrame == prev.m_raiseFrame) {
    assert(prev.m_faultType == Fault::Type::UserException);
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
 *   - Failing any of the above, pop the frame for the current
 *     function.  If the current function was the last frame in the
 *     current VM nesting level, return UnwindAction::Propagate,
 *     otherwise go to the first step and repeat this process in the
 *     caller's frame.
 *
 * Note: it's important that the unwinder makes a copy of the Fault
 * it's currently operating on, as the underlying faults vector may
 * reallocate due to nested exception handling.
 */
UnwindAction unwind(ActRec*& fp,
                    Stack& stack,
                    PC& pc,
                    Fault fault) {
  FTRACE(1, "entering unwinder for fault: {}\n", describeFault(fault));
  SCOPE_EXIT {
    FTRACE(1, "leaving unwinder for fault: {}\n", describeFault(fault));
  };

  for (;;) {
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
      fault.m_raiseNesting = g_vmContext->m_nestedVMs.size();
      // Raise frame is set to the current frame
      fault.m_raiseFrame = fp;
      // Raise offset is set to the offset of the current PC.
      fault.m_raiseOffset = fp->m_func->unit()->offsetOf(pc);
      // No handlers were yet examined for this fault.
      fault.m_handledCount = 0;
      // We will be also discarding stack temps.
      discard = true;
    }

    FTRACE(1, "unwind: func {}, raiseOffset {} fp {}\n",
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
          g_vmContext->m_faults.back() = fault;
          return UnwindAction::ResumeVM;
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
    auto const lastFrameForNesting = fp == fp->arGetSfp();
    tearDownFrame(fp, stack, pc);

    // Once we are done with EHs for the current frame we restore
    // default values for the fields inside Fault. This makes sure
    // that on another loop pass we will treat the fault just
    // as if it was freshly thrown.
    fault.m_raiseNesting = kInvalidNesting;
    fault.m_raiseFrame = nullptr;
    fault.m_raiseOffset = kInvalidOffset;
    fault.m_handledCount = 0;
    g_vmContext->m_faults.back() = fault;

    if (lastFrameForNesting) {
      FTRACE(1, "unwind: reached the end of this nesting's ActRec chain\n");
      break;
    }
  }

  return UnwindAction::Propagate;
}

const StaticString s_hphpd_break("hphpd_break");
const StaticString s_fb_enable_code_coverage("fb_enable_code_coverage");

// Unwind the frame for a builtin.  Currently only used when switching
// modes for hphpd_break and fb_enable_code_coverage.
void unwindBuiltinFrame() {
  auto& stack = g_vmContext->getStack();
  auto& fp = g_vmContext->m_fp;

  assert(fp->m_func->methInfo());
  assert(fp->m_func->name()->isame(s_hphpd_break.get()) ||
         fp->m_func->name()->isame(s_fb_enable_code_coverage.get()));

  // Free any values that may be on the eval stack.  We know there
  // can't be FPI regions and it can't be a generator body because
  // it's a builtin frame.
  auto const evalTop = reinterpret_cast<TypedValue*>(g_vmContext->getFP());
  while (stack.topTV() < evalTop) {
    stack.popTV();
  }

  // Free the locals and VarEnv if there is one
  frame_free_locals_inl(fp, fp->m_func->numLocals());

  // Tear down the frame
  Offset pc = -1;
  ActRec* sfp = g_vmContext->getPrevVMState(fp, &pc);
  assert(pc != -1);
  fp = sfp;
  g_vmContext->m_pc = fp->m_func->unit()->at(pc);
  stack.discardAR();
}

void pushFault(Exception* e) {
  Fault f;
  f.m_faultType = Fault::Type::CppException;
  f.m_cppException = e;
  g_vmContext->m_faults.push_back(f);
  FTRACE(1, "pushing new fault: {}\n", describeFault(f));
}

void pushFault(const Object& o) {
  Fault f;
  f.m_faultType = Fault::Type::UserException;
  f.m_userException = o.get();
  f.m_userException->incRefCount();
  g_vmContext->m_faults.push_back(f);
  FTRACE(1, "pushing new fault: {}\n", describeFault(f));
}

UnwindAction enterUnwinder() {
  auto fault = g_vmContext->m_faults.back();
  return unwind(
    g_vmContext->m_fp,      // by ref
    g_vmContext->getStack(),// by ref
    g_vmContext->m_pc,      // by ref
    fault
  );
}

//////////////////////////////////////////////////////////////////////

}

UnwindAction exception_handler() noexcept {
  FTRACE(1, "unwind exception_handler\n");

  g_vmContext->checkRegState();

  try { throw; }

  /*
   * Unwind (repropagating from a fault funclet) is slightly different
   * from the throw cases, because we need to re-raise the exception
   * as if it came from the same offset to handle nested fault
   * handlers correctly, and we continue propagating the current Fault
   * instead of pushing a new one.
   */
  catch (const VMPrepareUnwind&) {
    Fault fault = g_vmContext->m_faults.back();
    FTRACE(1, "unwind: restoring offset {}\n", g_vmContext->m_pc);
    return unwind(
      g_vmContext->m_fp,
      g_vmContext->getStack(),
      g_vmContext->m_pc,
      fault
    );
  }

  catch (const Object& o) {
    pushFault(o);
    return enterUnwinder();
  }

  catch (VMSwitchMode&) {
    return UnwindAction::ResumeVM;
  }

  catch (VMSwitchModeBuiltin&) {
    unwindBuiltinFrame();
    g_vmContext->getStack().pushNull(); // return value
    return UnwindAction::ResumeVM;
  }

  catch (VMReenterStackOverflow&) {
    pushFault(new FatalErrorException("Stack overflow"));
    return UnwindAction::Propagate;
  }

  catch (Exception& e) {
    pushFault(e.clone());;
    return enterUnwinder();
  }

  catch (std::exception& e) {
    pushFault(new Exception("unexpected %s: %s", typeid(e).name(), e.what()));
    return enterUnwinder();
  }

  catch (...) {
    pushFault(new Exception("unknown exception"));
    return enterUnwinder();
  }

  not_reached();
}

//////////////////////////////////////////////////////////////////////

}
