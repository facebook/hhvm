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

#include "hphp/util/trace.h"
#include "hphp/runtime/vm/core_types.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/debugger_hook.h"

namespace HPHP {

TRACE_SET_MOD(unwind);

namespace {

//////////////////////////////////////////////////////////////////////

UnwindAction unwindFrag(Stack& stack, ActRec* fp, int offset,
                        PC& pc, Fault& fault) {
  const Func* func = fp->m_func;
  FTRACE(1, "unwindFrag: func {} ({})\n",
         func->fullName()->data(), func->unit()->filepath()->data());

  const bool unwindingGeneratorFrame = func->isGenerator();
  auto const curOp = *reinterpret_cast<const Opcode*>(pc);
  using namespace HPHP;
  const bool unwindingReturningFrame = curOp == OpRetC || curOp == OpRetV;
  TypedValue* evalTop;
  if (UNLIKELY(unwindingGeneratorFrame)) {
    assert(!stack.isValidAddress((uintptr_t)fp));
    evalTop = Stack::generatorStackBase(fp);
  } else {
    assert(stack.isValidAddress((uintptr_t)fp));
    evalTop = Stack::frameStackBase(fp);
  }
  assert(stack.isValidAddress((uintptr_t)evalTop));
  assert(evalTop >= stack.top());

  while (stack.top() < evalTop) {
    stack.popTV();
  }

  /*
   * This code is repeatedly called with the same offset when an
   * exception is raised and rethrown by fault handlers.  This
   * `faultNest' iterator is here to skip the EHEnt handlers that have
   * already been run for this in-flight exception.
   */
  if (const EHEnt* eh = func->findEH(offset)) {
    int faultNest = 0;
    for (;;) {
      assert(faultNest <= fault.m_handledCount);
      if (faultNest == fault.m_handledCount) {
        ++fault.m_handledCount;

        switch (eh->m_ehtype) {
        case EHEnt::EHType_Fault:
          FTRACE(1, "unwindFrag: entering fault at {}: save {}\n",
                 eh->m_fault,
                 func->unit()->offsetOf(pc));
          fault.m_savedRaiseOffset = func->unit()->offsetOf(pc);
          pc = (uchar*)(func->unit()->entry() + eh->m_fault);
          DEBUGGER_ATTACHED_ONLY(phpDebuggerExceptionHandlerHook());
          return UnwindAction::ResumeVM;
        case EHEnt::EHType_Catch:
          // Note: we skip catch clauses if we have a pending C++ exception
          // as part of our efforts to avoid running more PHP code in the
          // face of such exceptions.
          if ((fault.m_faultType == Fault::UserException) &&
              (ThreadInfo::s_threadInfo->m_pendingException == nullptr)) {
            ObjectData* obj = fault.m_userException;
            for (auto& idOff : eh->m_catches) {
              auto handler = func->unit()->at(idOff.second);
              FTRACE(1, "unwindFrag: catch candidate {}\n", handler);
              Class* cls = Unit::lookupClass(
                func->unit()->lookupNamedEntityId(idOff.first)
              );
              if (cls && obj->instanceof(cls)) {
                pc = handler;
                FTRACE(1, "unwindFrag: entering catch at {}\n", pc);
                DEBUGGER_ATTACHED_ONLY(phpDebuggerExceptionHandlerHook());
                return UnwindAction::ResumeVM;
              }
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
      ++faultNest;
    }
  }

  // We found no more handlers in this frame, so the nested fault
  // count starts over for the caller frame.
  fault.m_handledCount = 0;

  if (fp->isFromFPushCtor() && fp->hasThis()) {
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
  }
  FTRACE(1, "unwindFrag: propagate\n");
  return UnwindAction::Propagate;
}

// Pops everything between the current stack pointer and the passed ActRec*.
// It assumes everything there is values, not ActRecs.
void unwindARFrag(Stack& stack, ActRec* ar) {
  while (stack.top() < (TypedValue*)ar) {
    stack.popTV();
  }
}

// Pops everything up to and including the outermost unactivated ActRec. Since
// it's impossible to have more than one chain of nested unactivated ActRecs
// on the stack, this means that after this function returns, everything
// between the stack pointer and frame pointer is a value, Iter or local.
void unwindAR(Stack& stack, ActRec* fp, const FPIEnt* fe) {
  while (true) {
    TRACE(1, "unwindAR: function %s, pIdx %d\n",
          fp->m_func->name()->data(), fe->m_parentIndex);
    ActRec* ar;
    if (LIKELY(!fp->m_func->isGenerator())) {
      ar = arAtOffset(fp, -fe->m_fpOff);
    } else {
      // FIXME: duplicated logic from visitStackElems
      TypedValue* genStackBase = Stack::generatorStackBase(fp);
      ActRec* fakePrevFP =
        (ActRec*)(genStackBase + fp->m_func->numSlotsInFrame());
      ar = arAtOffset(fakePrevFP, -fe->m_fpOff);
    }
    assert((TypedValue*)ar >= stack.top());
    unwindARFrag(stack, ar);

    if (ar->isFromFPushCtor()) {
      assert(ar->hasThis());
      ar->getThis()->setNoDestruct();
    }

    stack.popAR();
    if (fe->m_parentIndex != -1) {
      fe = &fp->m_func->fpitab()[fe->m_parentIndex];
    } else {
      return;
    }
  }
}

UnwindAction unwindFrame(Stack& stack,
                         ActRec*& fp,
                         Offset offset,
                         PC& pc,
                         Fault fault) {
  VMExecutionContext* context = g_vmContext;

  while (true) {
    FTRACE(1, "unwindFrame: func {}, offset {} fp {}\n",
           fp->m_func->name()->data(),
           offset,
           static_cast<void*>(fp));

    // If the exception is already propagating, if it was in any FPI
    // region we already handled unwinding it the first time around.
    if (fault.m_handledCount == 0) {
      if (const FPIEnt *fe = fp->m_func->findFPI(offset)) {
        unwindAR(stack, fp, fe);
      }
    }

    if (unwindFrag(stack, fp, offset, pc, fault) == UnwindAction::ResumeVM) {
      // We've kept our own copy of the Fault, because m_faults may
      // change if we have a reentry during unwinding.  When we're
      // ready to resume, we need to replace the current fault to
      // reflect any state changes we've made (handledCount, etc).
      assert(!context->m_faults.empty());
      context->m_faults.back() = fault;
      return UnwindAction::ResumeVM;
    }

    ActRec *prevFp = fp->arGetSfp();
    FTRACE(1, "unwindFrame: fp {} prevFp {}\n",
           static_cast<void*>(fp),
           static_cast<void*>(prevFp));
    if (LIKELY(!fp->m_func->isGenerator())) {
      // We don't need to refcount the AR's refcounted members; that was
      // taken care of in frame_free_locals, called from unwindFrag().
      // If it's a generator, the AR doesn't live on this stack.
      stack.discardAR();
    }

    if (prevFp == fp) {
      TRACE(1, "unwindFrame: reached the end of this nesting's ActRec "
               "chain\n");
      break;
    }
    // Keep the pc up to date while unwinding.
    Offset prevOff = fp->m_soff + prevFp->m_func->base();
    const Func *prevF = prevFp->m_func;
    assert(stack.isValidAddress((uintptr_t)prevFp) || prevF->isGenerator());
    pc = prevF->unit()->at(prevOff);
    fp = prevFp;
    offset = prevOff;
  }

  return UnwindAction::Propagate;
}

void pushFault(Fault::Type t, Exception* e, const Object* o = nullptr) {
  FTRACE(1, "pushing new fault: {} {} {}\n",
         t == Fault::UserException ? "[user exception]" : "[cpp exception]",
         e, o);

  VMExecutionContext* ec = g_vmContext;
  Fault fault;
  fault.m_faultType = t;
  if (t == Fault::UserException) {
    // User object.
    assert(o);
    fault.m_userException = o->get();
    fault.m_userException->incRefCount();
  } else {
    fault.m_cppException = e;
  }
  ec->m_faults.push_back(fault);
}

UnwindAction handleUnwind(UnwindAction unwindType) {
  UnwindAction longJumpType;
  if (unwindType == UnwindAction::Propagate) {
    longJumpType = UnwindAction::Propagate;
    if (g_vmContext->m_nestedVMs.empty()) {
      g_vmContext->m_fp = nullptr;
      g_vmContext->m_pc = nullptr;
    }
  } else {
    assert(unwindType == UnwindAction::ResumeVM);
    longJumpType = UnwindAction::ResumeVM;
  }
  return longJumpType;
}

UnwindAction hhvmPrepareThrow() {
  Fault& fault = g_vmContext->m_faults.back();
  TRACE(2, "hhvmPrepareThrow: %p(\"%s\") {\n",
           g_vmContext->getFP(),
           g_vmContext->getFP()->m_func->name()->data());
  UnwindAction unwindType;
  unwindType = unwindFrame(g_vmContext->getStack(),
                           g_vmContext->m_fp, // by ref
                           g_vmContext->pcOff(),
                           g_vmContext->m_pc, // by ref
                           fault);
  return handleUnwind(unwindType);
}

const StaticString s_hphpd_break("hphpd_break");
const StaticString s_fb_enable_code_coverage("fb_enable_code_coverage");

// Unwind the frame for a builtin.  Currently only used when switching
// modes for hphpd_break and fb_enable_code_coverage.
void unwindBuiltinFrame() {
  auto& fp = g_vmContext->m_fp;
  assert(fp->m_func->info());
  assert(fp->m_func->name()->isame(s_hphpd_break.get()) ||
         fp->m_func->name()->isame(s_fb_enable_code_coverage.get()));

  auto& stack = g_vmContext->getStack();

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

//////////////////////////////////////////////////////////////////////

}

UnwindAction exception_handler() noexcept {
  try { throw; }

  /*
   * Unwind (repropagating from a fault funclet) is slightly different
   * from the throw case, because we need to re-raise the exception as
   * if it came from the same offset to handle nested fault handlers
   * correctly.
   */
  catch (const VMPrepareUnwind&) {
    Fault fault = g_vmContext->m_faults.back();
    Offset faultPC = fault.m_savedRaiseOffset;
    FTRACE(1, "unwind: restoring offset {}\n", faultPC);
    assert(faultPC != kInvalidOffset);
    fault.m_savedRaiseOffset = kInvalidOffset;
    UnwindAction unwindType = unwindFrame(
      g_vmContext->getStack(),
      g_vmContext->m_fp,
      faultPC,
      g_vmContext->m_pc,
      fault
    );
    return handleUnwind(unwindType);
  }

  catch (const Object& e) {
    pushFault(Fault::UserException, nullptr, &e);
    return hhvmPrepareThrow();
  }

  catch (VMSwitchMode&) {
    return UnwindAction::ResumeVM;
  }

  catch (VMSwitchModeBuiltin&) {
    unwindBuiltinFrame();
    g_vmContext->getStack().pushNull(); // return value
    return UnwindAction::ResumeVM;
  }

  catch (Exception& e) {
    pushFault(Fault::CppException, e.clone());
    return hhvmPrepareThrow();
  }

  catch (std::exception& e) {
    pushFault(Fault::CppException,
              new Exception("unexpected %s: %s", typeid(e).name(), e.what()));
    return hhvmPrepareThrow();
  }

  catch (...) {
    pushFault(Fault::CppException,
              new Exception("unknown exception"));
    return hhvmPrepareThrow();
  }

  not_reached();
}

//////////////////////////////////////////////////////////////////////

}
