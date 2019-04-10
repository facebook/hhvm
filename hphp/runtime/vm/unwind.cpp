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

#include "hphp/runtime/vm/unwind.h"

#include <boost/implicit_cast.hpp>

#include <folly/ScopeGuard.h>

#include "hphp/util/trace.h"

#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/ext/asio/ext_async-function-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_async-generator-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_async-generator.h"
#include "hphp/runtime/ext/asio/ext_static-wait-handle.h"
#include "hphp/runtime/ext/generator/ext_generator.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/vm-regs.h"

namespace HPHP {

TRACE_SET_MOD(unwind);
using boost::implicit_cast;

namespace {

//////////////////////////////////////////////////////////////////////

#if (!defined(NDEBUG) || defined(USE_TRACE))
std::string describeEx(ObjectData* phpException) {
  return folly::format("[user exception] {}",
                       implicit_cast<void*>(phpException)).str();
}
#endif

void discardStackTemps(const ActRec* const fp,
                       Stack& stack,
                       Offset const bcOffset) {
  ITRACE(2, "discardStackTemps with fp {} sp {} pc {} op {}\n",
         implicit_cast<const void*>(fp),
         implicit_cast<void*>(stack.top()),
         bcOffset, opcodeToName(fp->func()->unit()->getOp(bcOffset)));

  visitStackElems(
    fp, stack.top(), bcOffset,
    [&] (ActRec* ar, Offset) {
      assertx(ar == reinterpret_cast<ActRec*>(stack.top()));
      ITRACE(2, "  unwind pop AR : {}\n",
             implicit_cast<void*>(stack.top()));
      stack.popAR();
    },
    [&] (TypedValue* tv) {
      assertx(tv == stack.top());
      ITRACE(2, "  unwind pop TV : {}\n",
             implicit_cast<void*>(stack.top()));
      stack.popTV();
    }
  );

  if (debug) {
    auto const numSlots = fp->m_func->numClsRefSlots();
    for (int i = 0; i < numSlots; ++i) {
      ITRACE(2, "  trash class-ref slot : {}\n", i);
      auto const slot = frame_clsref_slot(fp, i);
      memset(slot, kTrashClsRef, sizeof(*slot));
    }
  }

  ITRACE(2, "discardStackTemps ends with sp = {}\n",
         implicit_cast<void*>(stack.top()));
}

void discardMemberTVRefs(PC pc) {
  auto const throwOp = peek_op(pc);

  /*
   * If the opcode that threw was a member instruction, we have to decref tvRef
   * and tvRef2. AssertRAT* instructions can appear while these values are live
   * but they will never throw.
   */
  if (UNLIKELY(isMemberDimOp(throwOp) || isMemberFinalOp(throwOp))) {
    auto& mstate = vmMInstrState();
    tvDecRefGen(mstate.tvRef);
    tvWriteUninit(mstate.tvRef);
    tvDecRefGen(mstate.tvRef2);
    tvWriteUninit(mstate.tvRef2);
  }
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
  auto const prevFp = fp->sfp();
  auto const callOff = fp->m_callOff;

  ITRACE(1, "tearDownFrame: {} ({})\n",
         func->fullName()->data(),
         func->unit()->filepath()->data());
  ITRACE(1, "  fp {} prevFp {}\n",
         implicit_cast<void*>(fp),
         implicit_cast<void*>(prevFp));

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
      fp->setLocalsDecRefd();
      try {
        frame_free_locals_unwind(fp, func->numLocals(), phpException);
      } catch (...) {}
    }
  };

  if (LIKELY(!fp->resumed())) {
    decRefLocals();
    if (UNLIKELY(func->isAsyncFunction()) &&
        phpException &&
        (!fp->isAsyncEagerReturn() || func->isMemoizeImpl())) {
      // If in an eagerly executed async function without request for async
      // eager return, wrap the user exception into a failed StaticWaitHandle
      // and return it to the caller.
      auto const waitHandle = c_StaticWaitHandle::CreateFailed(phpException);
      phpException = nullptr;
      stack.ndiscard(func->numSlotsInFrame());
      stack.ret();
      assertx(stack.topTV() == fp->retSlot());
      cellCopy(make_tv<KindOfObject>(waitHandle), *fp->retSlot());
      fp->retSlot()->m_aux.u_asyncNonEagerReturnFlag = -1;
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
      decRefObj(waitHandle);
      phpException = nullptr;
    } else if (waitHandle->isRunning()) {
      // Let the C++ exception propagate. If the current frame represents async
      // function that is running, mark it as abruptly interrupted. Some opcodes
      // like Await may change state of the async function just before exit hook
      // decides to throw C++ exception.
      decRefLocals();
      waitHandle->failCpp();
      decRefObj(waitHandle);
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

  assertx(stack.isValidAddress(reinterpret_cast<uintptr_t>(prevFp)) ||
         prevFp->resumed());
  pc = prevFp->func()->unit()->at(callOff + prevFp->func()->base());
  fp = prevFp;
  return phpException;
}

const StaticString s_previous("previous");
const Slot s_previousIdx{6};

DEBUG_ONLY bool is_throwable(ObjectData* throwable) {
  auto const erCls = SystemLib::s_ErrorClass;
  auto const exCls = SystemLib::s_ExceptionClass;
  return throwable->instanceof(erCls) || throwable->instanceof(exCls);
}

DEBUG_ONLY bool throwable_has_expected_props() {
  auto const erCls = SystemLib::s_ErrorClass;
  auto const exCls = SystemLib::s_ExceptionClass;
  if (erCls->lookupDeclProp(s_previous.get()) != s_previousIdx ||
      exCls->lookupDeclProp(s_previous.get()) != s_previousIdx) {
    return false;
  }

  // Check that we have the expected type-hints on these props so we don't need
  // to verify anything when setting. If someone changes the type-hint we want
  // to know.
  auto const isException = [&](const TypeConstraint& tc) {
    if (!tc.isObject()) return false;
    auto const cls = Unit::lookupClass(tc.namedEntity());
    return cls && cls == SystemLib::s_ExceptionClass;
  };

  return
    isException(erCls->declPropTypeConstraint(s_previousIdx)) &&
    isException(exCls->declPropTypeConstraint(s_previousIdx));
}

const StaticString s_hphpd_break("hphpd_break");
const StaticString s_fb_enable_code_coverage("fb_enable_code_coverage");
const StaticString s_xdebug_start_code_coverage("xdebug_start_code_coverage");

//////////////////////////////////////////////////////////////////////

}

Offset findCatchHandler(const Func* func, Offset raiseOffset) {
  auto const eh = func->findEH(raiseOffset);
  if (eh == nullptr) return InvalidAbsoluteOffset;
  return eh->m_handler;
}

void chainFaultObjects(ObjectData* top, ObjectData* prev) {
  assertx(throwable_has_expected_props());

  // We don't chain the fault objects if there is a cycle in top, prev, or the
  // resulting chained fault object.
  std::unordered_set<uintptr_t> seen;

  // Walk head's previous pointers untill we find an unset one, or determine
  // they form a cycle.
  auto findAcyclicPrev = [&](ObjectData* head) {
    tv_lval foundLval;
    do {
      assertx(is_throwable(head));

      if (!seen.emplace((uintptr_t)head).second) return tv_lval();

      foundLval = head->propLvalAtOffset(s_previousIdx);
      assertx(foundLval.type() != KindOfUninit);
      head = foundLval.val().pobj;
    } while (foundLval.type() == KindOfObject &&
             foundLval.val().pobj->instanceof(SystemLib::s_ThrowableClass));
    return foundLval;
  };

  auto const prevLval = findAcyclicPrev(top);
  if (!prevLval || !findAcyclicPrev(prev)) {
    decRefObj(prev);
    return;
  }

  // Found an unset previous pointer, and result will not have a cycle so chain
  // the fault objects.
  tvMoveIgnoreRef(make_tv<KindOfObject>(prev), prevLval);
}

/*
 * Unwinding proceeds as follows:
 *
 *   - Discard all evaluation stack temporaries (including pre-live
 *     activation records).
 *
 *   - Check if the current PC is inside a protected region, if so,
 *     leave the exception on the stack and resume the VM at the handler.
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
 */
void unwindPhp(ObjectData* phpException) {
  phpException->incRefCount();

  auto& fp = vmfp();
  auto& stack = vmStack();
  auto& pc = vmpc();
  bool fromTearDownFrame = false;

  ITRACE(1, "entering unwinder for exception: {}\n", describeEx(phpException));
  SCOPE_EXIT {
    ITRACE(1, "leaving unwinder for exception: {}\n", describeEx(phpException));
  };

  discardMemberTVRefs(pc);

  do {
    auto const func = fp->func();
    auto const raiseOffset = func->unit()->offsetOf(pc);

    ITRACE(1, "unwindPhp: func {}, raiseOffset {} fp {}\n",
           func->name()->data(),
           raiseOffset,
           implicit_cast<void*>(fp));

    if (fromTearDownFrame) {
      fromTearDownFrame = false;
      discardStackTemps(fp, stack, func->unit()->offsetOf(skipCall(pc)));
    } else {
      discardStackTemps(fp, stack, raiseOffset);
    }

    // Note: we skip catch/finally clauses if we have a pending C++
    // exception as part of our efforts to avoid running more PHP
    // code in the face of such exceptions. Similarly, if the frame
    // has already been torn down (eg an exception thrown by a user
    // profiler on function exit), we can't execute any handlers in
    // *this* frame.
    if (RequestInfo::s_requestInfo->m_pendingException == nullptr &&
        !UNLIKELY(fp->localsDecRefd())) {

      const EHEnt* eh = func->findEH(raiseOffset);
      if (eh != nullptr) {
        // Found exception handler. Push the exception on top of the
        // stack and resume VM.
        ITRACE(1, "unwindPhp: entering catch at {} func {} ({})\n",
               eh->m_handler,
               func->fullName()->data(),
               func->unit()->filepath()->data());

        vmStack().pushObjectNoRc(phpException);
        pc = func->unit()->at(eh->m_handler);
        DEBUGGER_ATTACHED_ONLY(phpDebuggerExceptionHandlerHook());
        return;
      }
    }

    // We found no more handlers in this frame.
    phpException = tearDownFrame(fp, stack, pc, phpException);
    if (phpException == nullptr) {
      if (fp) pc = skipCall(pc);
      return;
    }

    fromTearDownFrame = true;
  } while (fp);

  ITRACE(1, "unwind: reached the end of this nesting's ActRec chain\n");
  throw_object(Object::attach(phpException));
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

  assertx(!g_context->m_unwindingCppException);
  g_context->m_unwindingCppException = true;
  ITRACE(1, "entering unwinder for C++ exception: {}\n",
         implicit_cast<void*>(exception));
  SCOPE_EXIT {
    assertx(g_context->m_unwindingCppException);
    g_context->m_unwindingCppException = false;
    ITRACE(1, "leaving unwinder for C++ exception: {}\n",
           implicit_cast<void*>(exception));
  };

  discardMemberTVRefs(pc);

  do {
    auto const offset = fp->func()->unit()->offsetOf(pc);

    ITRACE(1, "unwindCpp: func {}, raiseOffset {} fp {}\n",
           fp->func()->name()->data(),
           offset,
           implicit_cast<void*>(fp));

    // Discard stack temporaries
    discardStackTemps(fp, stack, offset);

    // Discard the frame
    DEBUG_ONLY auto const phpException = tearDownFrame(fp, stack, pc, nullptr);
    assertx(phpException == nullptr);
    if (fp) pc = skipCall(pc);
  } while (fp);

  // Propagate the C++ exception to the outer VM nesting
  exception->throwException();
}

void unwindBuiltinFrame() {
  auto& stack = vmStack();
  auto& fp = vmfp();

  assertx(fp->m_func->name()->isame(s_hphpd_break.get()) ||
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
  assertx(pc != -1);
  fp = sfp;
  vmpc() = skipCall(fp->m_func->unit()->at(pc));
  stack.ndiscard(numSlots);
  stack.discardAR();
  stack.pushNull(); // return value
}

//////////////////////////////////////////////////////////////////////

}
