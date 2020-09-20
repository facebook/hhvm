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

#include "hphp/runtime/debugger/cmd/cmd_next.h"

#include "hphp/runtime/debugger/debugger_client.h"
#include "hphp/runtime/ext/asio/ext_async-function-wait-handle.h"
#include "hphp/runtime/ext/generator/ext_generator.h"
#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/vm-regs.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debuggerflow);

void CmdNext::help(DebuggerClient& client) {
  client.helpTitle("Next Command");
  client.helpCmds(
    "[n]ext {count=1}", "steps over lines of code",
    nullptr
  );
  client.helpBody(
    "Use this command at break to step over lines of code. Specify a "
    "count to step over more than one line of code."
  );
}

void CmdNext::onSetup(DebuggerProxy& proxy, CmdInterrupt& interrupt) {
  TRACE(2, "CmdNext::onSetup\n");
  assertx(!m_complete); // Complete cmds should not be asked to do work.
  m_stackDepth = proxy.getStackDepth();
  m_vmDepth = g_context->m_nesting;
  m_loc = interrupt.getFileLine();
  ActRec *fp = vmfp();
  if (!fp) {
    // If we have no frame just wait for the next instruction to be interpreted.
    m_needsVMInterrupt = true;
    return;
  }
  PC pc = vmpc();
  stepCurrentLine(interrupt, fp, pc);
}

void CmdNext::onBeginInterrupt(DebuggerProxy& proxy, CmdInterrupt& interrupt) {
  TRACE(2, "CmdNext::onBeginInterrupt\n");
  assertx(!m_complete); // Complete cmds should not be asked to do work.

  ActRec *fp = vmfp();
  if (!fp) {
    // If we have no frame just wait for the next instruction to be interpreted.
    m_needsVMInterrupt = true;
    return;
  }
  PC pc = vmpc();
  Offset offset = fp->func()->offsetOf(pc);
  TRACE(2, "CmdNext: pc %p, opcode %s at '%s' offset %d\n",
        pc,
        opcodeToName(peek_op(pc)),
        fp->func()->fullName()->data(),
        offset);

  int currentVMDepth = g_context->m_nesting;
  int currentStackDepth = proxy.getStackDepth();

  TRACE(2, "CmdNext: original depth %d:%d, current depth %d:%d\n",
        m_vmDepth, m_stackDepth, currentVMDepth, currentStackDepth);

  // Where are we on the stack now vs. when we started? Breaking the answer down
  // into distinct variables helps the clarity of the algorithm below.
  bool deeper = false;
  bool originalDepth = false;
  if ((currentVMDepth == m_vmDepth) && (currentStackDepth == m_stackDepth)) {
    originalDepth = true;
  } else if ((currentVMDepth > m_vmDepth) ||
             ((currentVMDepth == m_vmDepth) &&
              (currentStackDepth > m_stackDepth))) {
    deeper = true;
  }

  m_needsVMInterrupt = false; // Will be set again below if still needed.

  // First consider if we've got internal breakpoints setup. These are used when
  // we can make an accurate prediction of where execution should flow,
  // eventually, and when we want to let the program run normally until we get
  // there.
  if (hasStepOuts() || hasStepResumable()) {
    TRACE(2, "CmdNext: checking internal breakpoint(s)\n");
    if (atStepOutOffset(fp->func(), offset)) {
      if (deeper) return; // Recursion
      TRACE(2, "CmdNext: hit step-out\n");
    } else if (atStepResumableOffset(fp->func(), offset)) {
      if (m_stepResumableId != getResumableId(fp)) return;
      TRACE(2, "CmdNext: hit step-cont\n");
      // We're in the resumable we expect. This may be at a
      // different stack depth, though, especially if we've moved from
      // the original function to the resumable. Update the depth
      // accordingly.
      if (!originalDepth) {
        m_vmDepth = currentVMDepth;
        m_stackDepth = currentStackDepth;
        deeper = false;
        originalDepth = true;
      }
    } else if (interrupt.getInterruptType() == ExceptionHandler) {
      // Entering an exception handler may take us someplace we weren't
      // expecting. Adjust internal breakpoints accordingly. First case is easy.
      if (deeper) {
        TRACE(2, "CmdNext: exception handler, deeper\n");
        return;
      }
      // For step-conts, we ignore handlers at the original level if we're not
      // in the original resumable. We don't care about exception handlers
      // in resumables being driven at the same level.
      if (hasStepResumable() && originalDepth &&
          (m_stepResumableId != getResumableId(fp))) {
        TRACE(2, "CmdNext: exception handler, original depth, wrong cont\n");
        return;
      }
      // Sometimes we have handlers in generated code, i.e., Continuation::next.
      // These just help propagate exceptions so ignore those.
      if (fp->func()->line1() == 0) {
        TRACE(2, "CmdNext: exception handler, ignoring func with no source\n");
        return;
      }
      if (fp->func()->isBuiltin()) {
        TRACE(2, "CmdNext: exception handler, ignoring builtin functions\n");
        return;
      }
      TRACE(2, "CmdNext: exception handler altering expected flow\n");
    } else {
      // We have internal breakpoints setup, but we haven't hit one yet. Keep
      // running until we reach one.
      TRACE(2, "CmdNext: waiting to hit internal breakpoint...\n");
      return;
    }
    // We've hit one internal breakpoint at a useful place, or decided we don't,
    // need them, so we can remove them all now.
    cleanupStepOuts();
    cleanupStepResumable();
  }

  if (interrupt.getInterruptType() == ExceptionHandler) {
    // If we're about to enter an exception handler we turn interrupts on to
    // ensure we stop when control reaches the handler. The normal logic below
    // will decide if we're done at that point or not.
    TRACE(2, "CmdNext: exception handler\n");
    removeLocationFilter();
    m_needsVMInterrupt = true;
    return;
  }

  if (m_steppingWhileSuspendingFrame) {
    m_steppingWhileSuspendingFrame = false;
    stepIntoSuspendedFrame();
    return;
  }

  if (deeper) {
    TRACE(2, "CmdNext: deeper, setup step out to get back to original line\n");
    setupStepOuts();
    // We can nuke the entire location filter here since we'll re-install it
    // when we get back to the old level. Keeping it installed may be more
    // efficient if we were on a large line, but there is a penalty for every
    // opcode executed while it's installed and that's bad if there's a lot of
    // code called from that line.
    removeLocationFilter();
    return;
  }

  if (originalDepth && (m_loc == interrupt.getFileLine())) {
    TRACE(2, "CmdNext: not complete, still on same line\n");
    stepCurrentLine(interrupt, fp, pc);
    return;
  }

  TRACE(2, "CmdNext: operation complete.\n");
  m_complete = (decCount() == 0);
  if (!m_complete) {
    TRACE(2, "CmdNext: repeat count > 0, start fresh.\n");
    onSetup(proxy, interrupt);
  }
}

void CmdNext::stepCurrentLine(CmdInterrupt& interrupt, ActRec* fp, PC pc) {
  // Special handling for yields from generators and awaits from
  // async. The destination of these instructions is somewhat counter
  // intuitive so we take care to ensure that we step to the most
  // appropriate place. For yields, we want to land on the next
  // statement when driven from a C++ iterator like ASIO. If the
  // generator is driven directly from PHP (i.e., a loop calling
  // send($foo)) then we'll land back at the callsite of send(). For
  // returns from generators, we follow the execution stack for now,
  // and end up at the caller of ASIO or send(). For async functions
  // stepping over an await, we land on the next statement.
  auto const op = peek_op(pc);
  if (op == OpAwait) {
    assertx(fp->func()->isAsync());
    auto wh = c_Awaitable::fromTV(*vmsp());
    if (wh && !wh->isFinished()) {
      TRACE(2, "CmdNext: encountered blocking await\n");
      if (isResumed(fp)) {
        setupStepSuspend(fp, pc);
        removeLocationFilter();
      } else {
        // Eager execution in non-resumed mode is supported only by async
        // functions. We need to step over this opcode, but that will cause this
        // frame to be moved off of the stack onto the heap, and will put us
        // in the caller with a new AsyncFunctionWaitHandle on the stack. We
        // will inspect that AsyncFunctionWaitHandle and run until the moved
        // frame it refers to resumes.
        assertx(fp->func()->isAsyncFunction());
        m_steppingWhileSuspendingFrame = true;
        m_needsVMInterrupt = true;
        removeLocationFilter();
      }
      return;
    }
  } else if (op == OpYield || op == OpYieldK) {
    assertx(isResumed(fp));
    assertx(fp->func()->isGenerator());
    TRACE(2, "CmdNext: encountered yield from generator\n");
    setupStepOuts();
    setupStepSuspend(fp, pc);
    removeLocationFilter();
    return;
  } else if (op == OpRetC && isResumed(fp)) {
    assertx(fp->func()->isResumable());
    TRACE(2, "CmdNext: encountered return from resumed resumable\n");
    setupStepOuts();
    removeLocationFilter();
    return;
  }

  installLocationFilterForLine(interrupt.getSite());
  m_needsVMInterrupt = true;
}

bool CmdNext::hasStepResumable() {
  return m_stepResumable.valid();
}

bool CmdNext::atStepResumableOffset(const Func* func, Offset o) {
  return m_stepResumable.at(func, o);
}

// Await / Yield opcodes mark a suspend points of async functions and
// generators. Execution will resume at this function later after the
// opcode.
void CmdNext::setupStepSuspend(ActRec* fp, PC pc) {
  // Yield is followed by the label where execution will continue.
  auto const op = decode_op(pc);
  assertx(op == OpAwait || op == OpYield || op == OpYieldK);
  if (op == OpAwait) {
    decode_iva(pc);
  }
  Offset nextInst = fp->func()->offsetOf(pc);
  assertx(nextInst != kInvalidOffset);
  m_stepResumableId = fp;
  TRACE(2, "CmdNext: patch for resumable step at '%s' offset %d\n",
        fp->func()->fullName()->data(), nextInst);
  m_stepResumable = StepDestination(fp->func(), nextInst);
}

// We were trying to step over an Await in an eagerly executed frame, and the
// frame we were in was moved from the stack to the heap. We are now in the
// callee with the AsyncFunctionWaitHandle for the frame we were in on top of
// the stack. Run until the now suspended frame resumes after the Await we were
// stepping over.
void CmdNext::stepIntoSuspendedFrame() {
  auto topObj = vmsp()->m_data.pobj;
  assertx(topObj->instanceof(c_AsyncFunctionWaitHandle::classof()));
  auto wh = static_cast<c_AsyncFunctionWaitHandle*>(topObj);
  auto func = wh->actRec()->func();
  Offset nextInst = wh->getNextExecutionOffset();
  m_stepResumableId = wh->actRec();
  TRACE(2,
        "CmdNext: patch for cont step after Await at '%s' offset %d\n",
        func->fullName()->data(), nextInst);
  m_stepResumable = StepDestination(func, nextInst);
}

void CmdNext::cleanupStepResumable() {
  if (m_stepResumable.valid()) {
    m_stepResumable = StepDestination();
    m_stepResumableId = nullptr;
  }
}

// Use the address of the c_Generator object as a tag for this stepping
// operation, to ensure we only stop once we're back to the same resumable.
// Since we'll either stop when we get out of whatever is driving this
// resumable, or we'll stop when we get back into it, we know the object
// will remain alive.
void* CmdNext::getResumableId(ActRec* fp) {
  assertx(isResumed(fp));
  assertx(fp->func()->isResumable());
  TRACE(2, "CmdNext: resumable tag %p for %s\n", fp,
        fp->func()->name()->data());
  return fp;
}

///////////////////////////////////////////////////////////////////////////////
}}
