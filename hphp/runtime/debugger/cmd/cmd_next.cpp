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

#include "hphp/runtime/debugger/cmd/cmd_next.h"
#include "hphp/runtime/vm/debugger_hook.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/ext/ext_continuation.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debuggerflow);

CmdNext::~CmdNext() {
  cleanupStepCont();
}

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
  assert(!m_complete); // Complete cmds should not be asked to do work.
  m_stackDepth = proxy.getStackDepth();
  m_vmDepth = g_vmContext->m_nesting;
  m_loc = interrupt.getFileLine();
  ActRec *fp = g_vmContext->getFP();
  assert(fp); // All interrupts which reach a flow cmd have an AR.
  PC pc = g_vmContext->getPC();
  stepCurrentLine(interrupt, fp, pc);
}

void CmdNext::onBeginInterrupt(DebuggerProxy& proxy, CmdInterrupt& interrupt) {
  TRACE(2, "CmdNext::onBeginInterrupt\n");
  assert(!m_complete); // Complete cmds should not be asked to do work.

  ActRec *fp = g_vmContext->getFP();
  assert(fp); // All interrupts which reach a flow cmd have an AR.
  PC pc = g_vmContext->getPC();
  Unit* unit = fp->m_func->unit();
  Offset offset = unit->offsetOf(pc);
  TRACE(2, "CmdNext: pc %p, opcode %s at '%s' offset %d\n",
        pc, opcodeToName(toOp(*pc)), fp->m_func->fullName()->data(), offset);

  int currentVMDepth = g_vmContext->m_nesting;
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
  if (hasStepOuts() || hasStepCont()) {
    TRACE(2, "CmdNext: checking internal breakpoint(s)\n");
    if (atStepOutOffset(unit, offset)) {
      if (deeper) return; // Recursion
      TRACE(2, "CmdNext: hit step-out\n");
    } else if (atStepContOffset(unit, offset)) {
      // For step-conts we want to hit the exact same frame, for the same
      // continuation, not a call to the same function higher or lower on the
      // stack.
      if (!originalDepth || (m_stepContTag != getContinuationTag(fp))) return;
      TRACE(2, "CmdNext: hit step-cont\n");
    } else if (interrupt.getInterruptType() == ExceptionHandler) {
      // Entering an exception handler may take us someplace we weren't
      // expecting. Adjust internal breakpoints accordingly. First case is easy.
      if (deeper) {
        TRACE(2, "CmdNext: exception handler, deeper\n");
        return;
      }
      // For step-conts, we ignore handlers at the original level if we're not
      // in the original continuation. We don't care about exception handlers
      // in continuations being driven at the same level.
      if (hasStepCont() && originalDepth &&
          (m_stepContTag != getContinuationTag(fp))) {
        TRACE(2, "CmdNext: exception handler, original depth, wrong cont\n");
        return;
      }
      // Sometimes we have handlers in generated code, i.e., Continuation::next.
      // These just help propagate exceptions so ignore those.
      if (fp->m_func->line1() == 0) {
        TRACE(2, "CmdNext: exception handler, ignoring func with no source\n");
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
    cleanupStepCont();
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
  // Special handling for yields from generators. The destination of these
  // instructions is somewhat counter intuitive so we take care to ensure that
  // we step to the most appropriate place. For yeilds, we want to land on the
  // next statement when driven from a C++ iterator like ASIO. If the generator
  // is driven directly from PHP (i.e., a loop calling send($foo)) then we'll
  // land back at the callsite of send(). For returns from generators, we follow
  // the execution stack for now, and end up at the caller of ASIO or send().
  auto op = toOp(*pc);
  if (fp->m_func->isGenerator() &&
      (op == OpContSuspend || op == OpContSuspendK || op == OpContRetC)) {
    TRACE(2, "CmdNext: encountered yield or return from generator\n");
    // Patch the projected return point(s) in both cases, to catch if we exit
    // the the asio iterator or if we are being iterated directly by PHP.
    setupStepOuts();
    op = toOp(*pc);
    if (op == OpContSuspend || op == OpContSuspendK) {
      // Patch the next normal execution point so we can pickup the stepping
      // from there if the caller is C++.
      setupStepCont(fp, pc);
    }
    removeLocationFilter();
    return;
  }

  installLocationFilterForLine(interrupt.getSite());
  m_needsVMInterrupt = true;
}

bool CmdNext::hasStepCont() {
  return m_stepContUnit != nullptr;
}

bool CmdNext::atStepContOffset(Unit* unit, Offset o) {
  return (unit == m_stepContUnit) && (o == m_stepContOffset);
}

// A ContSuspend is followed by code to support ContRaise, then code for
// ContSend/ContNext. We want to continue stepping on the latter. The normal
// exception handling logic will take care of the former.
// This logic is sensitive to the code gen here... we don't have access to the
// offsets for the labels used to generate this code, so we rely on the
// simplicity of the exceptional path.
void CmdNext::setupStepCont(ActRec* fp, PC pc) {
  // One byte + one byte argument
  auto ops = reinterpret_cast<const Op*>(pc);
  assert(ops[0] == OpContSuspend || ops[0] == OpContSuspendK);
  assert(ops[2] == OpNull); // One byte
  assert(ops[3] == OpThrow); // One byte
  assert(ops[4] == OpNull); // One byte
  Offset nextInst = fp->m_func->unit()->offsetOf(pc + 5);
  m_stepContUnit = fp->m_func->unit();
  m_stepContOffset = nextInst;
  m_stepContTag = getContinuationTag(fp);
  TRACE(2, "CmdNext: patch for cont step at '%s' offset %d\n",
        fp->m_func->fullName()->data(), nextInst);
  phpAddBreakPoint(m_stepContUnit, m_stepContOffset);
}

void CmdNext::cleanupStepCont() {
  if (m_stepContUnit) {
    if (m_stepContOffset != InvalidAbsoluteOffset) {
      phpRemoveBreakPoint(m_stepContUnit, m_stepContOffset);
      m_stepContOffset = InvalidAbsoluteOffset;
    }
    m_stepContTag = nullptr;
    m_stepContUnit = nullptr;
  }
}

// Use the address of the c_Continuation object as a tag for this stepping
// operation, to ensure we only stop once we're back to the same continuation.
// Since we'll either stop when we get out of whatever is driving this
// continuation, or we'll stop when we get back into it, we know the object
// will remain alive.
void* CmdNext::getContinuationTag(ActRec* fp) {
  TypedValue* tv = frame_local(fp, 0);
  assert(tv->m_type == HPHP::KindOfObject);
  assert(dynamic_cast<c_Continuation*>(tv->m_data.pobj));
  c_Continuation* cont = static_cast<c_Continuation*>(tv->m_data.pobj);
  TRACE(2, "CmdNext: continuation tag %p for %s\n", cont,
        cont->t_getorigfuncname()->data());
  return cont;
}

///////////////////////////////////////////////////////////////////////////////
}}
