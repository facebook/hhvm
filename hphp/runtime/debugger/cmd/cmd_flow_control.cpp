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

#include "hphp/runtime/debugger/cmd/cmd_flow_control.h"
#include "hphp/runtime/vm/debugger_hook.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debuggerflow);

CmdFlowControl::~CmdFlowControl() {
  // Remove any location filter or step outs that may have been setup.
  removeLocationFilter();
  cleanupStepOuts();
}

void CmdFlowControl::sendImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::sendImpl(thrift);
  thrift.write(m_count);
  thrift.write(m_smallStep);
}

void CmdFlowControl::recvImpl(DebuggerThriftBuffer &thrift) {
  DebuggerCommand::recvImpl(thrift);
  thrift.read(m_count);
  thrift.read(m_smallStep);
}

void CmdFlowControl::onClientImpl(DebuggerClient &client) {
  if (DebuggerCommand::displayedHelp(client)) return;

  client.setFrame(0);

  if (client.argCount() > 1) {
    help(client);
    return;
  }

  if (client.argCount() == 1) {
    string snum = client.argValue(1);
    if (!DebuggerClient::IsValidNumber(snum)) {
      client.error("Count needs to be a number.");
      return;
    }

    m_count = atoi(snum.c_str());
    if (m_count < 1) {
      client.error("Count needs to be a positive number.");
      return;
    }
  }
  m_smallStep = client.getDebuggerClientSmallStep();
  client.sendToServer(this);
  throw DebuggerConsoleExitException();
}

bool CmdFlowControl::onServer(DebuggerProxy &proxy) {
  // Flow control cmds do their work in onSetup() and onBeginInterrupt(), so
  // there is no real work to do in here.
  return true;
}

// Setup the last location filter on the VM context for all offsets covered by
// the current source line. This will short-circuit the work done in
// phpDebuggerOpcodeHook() and ensure we don't interrupt on this source line.
// We exclude continuation opcodes which transfer control out of the function,
// which allows cmds to get a chance to alter their behavior when those opcodes
// are encountered.
void CmdFlowControl::installLocationFilterForLine(InterruptSite *site) {
  // We may be stopped at a place with no source info.
  if (!site || !site->valid()) return;
  if (g_vmContext->m_lastLocFilter) {
    g_vmContext->m_lastLocFilter->clear();
  } else {
    g_vmContext->m_lastLocFilter = new PCFilter();
  }
  TRACE(3, "Prepare location filter for %s:%d, unit %p:\n",
        site->getFile(), site->getLine0(), site->getUnit());
  OffsetRangeVec ranges;
  const auto unit = site->getUnit();
  if (m_smallStep) {
    // Get offset range for the pc only.
    OffsetRange range;
    if (unit->getOffsetRange(site->getCurOffset(), range)) {
      ranges.push_back(range);
    }
  } else {
    // Get offset ranges for the whole line.
    // We use line1 here because it seems to be working better than line0
    // in a handful of cases for our bytecode-source mapping.
    if (!unit->getOffsetRanges(site->getLine1(), ranges)) {
      ranges.clear();
    }
  }
  auto excludeContinuationReturns = [] (Op op) {
    return (op != OpContSuspend) && (op != OpContRetC);
  };
  g_vmContext->m_lastLocFilter->addRanges(unit, ranges,
                                          excludeContinuationReturns);
}

void CmdFlowControl::removeLocationFilter() {
  if (g_vmContext->m_lastLocFilter) {
    delete g_vmContext->m_lastLocFilter;
    g_vmContext->m_lastLocFilter = nullptr;
  }
}

bool CmdFlowControl::hasStepOuts() {
  return m_stepOutUnit != nullptr;
}

bool CmdFlowControl::atStepOutOffset(Unit* unit, Offset o) {
  return (unit == m_stepOutUnit) &&
    ((o == m_stepOutOffset1) || (o == m_stepOutOffset2));
}

// Place internal breakpoints to get out of the current function. This may place
// multiple internal breakpoints, and it may place them more than one frame up.
// Some instructions can cause PHP to be invoked without an explicit call. A set
// which causes a destructor to run, a iteration init which causes an object's
// next() method to run, a RetC which causes destructors to run, etc. This
// recgonizes such cases and ensures we have internal breakpoints to cover the
// destination(s) of such instructions.
void CmdFlowControl::setupStepOuts() {
  // Existing step outs should be cleaned up before making new ones.
  assert(!hasStepOuts());
  ActRec* fp = g_vmContext->getFP();
  assert(fp);
  Offset returnOffset;
  bool fromVMEntry;
  while (!hasStepOuts()) {
    fp = g_vmContext->getPrevVMState(fp, &returnOffset, nullptr, &fromVMEntry);
    // If we've run off the top of the stack, just return having setup no
    // step outs. This will cause cmds like Next and Out to just let the program
    // run, which is appropriate.
    if (!fp) break;
    Unit* returnUnit = fp->m_func->unit();
    PC returnPC = returnUnit->at(returnOffset);
    TRACE(2, "CmdFlowControl::setupStepOuts: at '%s' offset %d opcode %s\n",
          fp->m_func->fullName()->data(), returnOffset,
          opcodeToName(toOp(*returnPC)));
    // Don't step out to generated functions, keep looking.
    if (fp->m_func->line1() == 0) continue;
    if (fromVMEntry) {
      TRACE(2, "CmdFlowControl::setupStepOuts: VM entry\n");
      // We only execute this for opcodes which invoke more PHP, and that does
      // not include switches. Thus, we'll have at most two destinations.
      assert(!isSwitch(*returnPC) && (numSuccs((Op*)returnPC) <= 2));
      // Set an internal breakpoint after the instruction if it can fall thru.
      if (instrAllowsFallThru(toOp(*returnPC))) {
        m_stepOutUnit = returnUnit;
        m_stepOutOffset1 = returnOffset + instrLen((Op*)returnPC);
        TRACE(2, "CmdFlowControl: step out to '%s' offset %d (fall-thru)\n",
              fp->m_func->fullName()->data(), m_stepOutOffset1);
        phpAddBreakPoint(m_stepOutUnit, m_stepOutOffset1);
      }
      // Set an internal breakpoint at the target of a control flow instruction.
      // A good example of a control flow op that invokes PHP is IterNext.
      if (instrIsControlFlow(toOp(*returnPC))) {
        Offset target = instrJumpTarget((Op*)returnPC, 0);
        if (target != InvalidAbsoluteOffset) {
          m_stepOutUnit = returnUnit;
          m_stepOutOffset2 = returnOffset + target;
          TRACE(2, "CmdFlowControl: step out to '%s' offset %d (jump target)\n",
                fp->m_func->fullName()->data(), m_stepOutOffset2);
          phpAddBreakPoint(m_stepOutUnit, m_stepOutOffset2);
        }
      }
      // If we have no place to step out to, then unwind another frame and try
      // again. The most common case that leads here is Ret*, which does not
      // fall-thru and has no encoded target.
    } else {
      m_stepOutUnit = returnUnit;
      m_stepOutOffset1 = returnOffset;
      TRACE(2, "CmdFlowControl: step out to '%s' offset %d\n",
            fp->m_func->fullName()->data(), m_stepOutOffset1);
      phpAddBreakPoint(m_stepOutUnit, m_stepOutOffset1);
    }
  }
}

void CmdFlowControl::cleanupStepOuts() {
  if (m_stepOutUnit) {
    if (m_stepOutOffset1 != InvalidAbsoluteOffset) {
      phpRemoveBreakPoint(m_stepOutUnit, m_stepOutOffset1);
      m_stepOutOffset1 = InvalidAbsoluteOffset;
    }
    if (m_stepOutOffset2 != InvalidAbsoluteOffset) {
      phpRemoveBreakPoint(m_stepOutUnit, m_stepOutOffset2);
      m_stepOutOffset2 = InvalidAbsoluteOffset;
    }
    m_stepOutUnit = nullptr;
  }
}

///////////////////////////////////////////////////////////////////////////////
}}
