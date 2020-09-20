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

#include "hphp/runtime/debugger/cmd/cmd_flow_control.h"

#include <algorithm>

#include "hphp/runtime/debugger/debugger_client.h"
#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/vm-regs.h"

namespace HPHP { namespace Eval {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debuggerflow);

CmdFlowControl::~CmdFlowControl() {
  // Remove any location filter that may have been setup.
  removeLocationFilter();
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

void CmdFlowControl::onClient(DebuggerClient &client) {
  if (DebuggerCommand::displayedHelp(client)) return;

  client.setFrame(0);

  if (client.argCount() > 1) {
    help(client);
    return;
  }

  if (client.argCount() == 1) {
    std::string snum = client.argValue(1);
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

bool CmdFlowControl::onServer(DebuggerProxy& /*proxy*/) {
  // Flow control cmds do their work in onSetup() and onBeginInterrupt(), so
  // there is no real work to do in here.
  return true;
}

// Setup the flow filter on the VM context for all offsets covered by
// the current source line. This will short-circuit the work done in
// phpDebuggerOpcodeHook() and ensure we don't interrupt on this source line.
// We exclude continuation opcodes which transfer control out of the function,
// which allows cmds to get a chance to alter their behavior when those opcodes
// are encountered.
void CmdFlowControl::installLocationFilterForLine(InterruptSite *site) {
  // We may be stopped at a place with no source info.
  if (!site || !site->valid()) return;
  RequestInjectionData &rid = RequestInfo::s_requestInfo->m_reqInjectionData;
  rid.m_flowFilter.clear();
  auto const unit = site->getFunc()->unit();
  TRACE(3, "Prepare location filter for %s:%d, unit %p:\n",
        site->getFile(), site->getLine0(), unit);
  OffsetRangeVec ranges;
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
  auto const func = site->getFunc();
  if (func->isResumable()) {
    auto excludeResumableReturns = [] (Op op) {
      return (op != OpYield) &&
             (op != OpYieldK) &&
             (op != OpAwait) &&
             (op != OpRetC);
    };
    rid.m_flowFilter.addRanges(unit, ranges,
                                     excludeResumableReturns);
  } else {
    rid.m_flowFilter.addRanges(unit, ranges);
  }
}

void CmdFlowControl::removeLocationFilter() {
  RequestInfo::s_requestInfo->m_reqInjectionData.m_flowFilter.clear();
}

bool CmdFlowControl::hasStepOuts() {
  return m_stepOut1.valid() || m_stepOut2.valid();
}

bool CmdFlowControl::atStepOutOffset(const Func* func, Offset o) {
  return m_stepOut1.at(func, o) || m_stepOut2.at(func, o);
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
  assertx(!hasStepOuts());
  auto fp = vmfp();
  if (!fp) return; // No place to step out to!
  Offset callOffset;
  bool fromVMEntry;
  while (!hasStepOuts()) {
    fp = g_context->getPrevVMState(fp, &callOffset, nullptr, &fromVMEntry);
    // If we've run off the top of the stack, just return having setup no
    // step outs. This will cause cmds like Next and Out to just let the program
    // run, which is appropriate.
    if (!fp) break;
    PC callPC = fp->func()->at(callOffset);
    TRACE(2, "CmdFlowControl::setupStepOuts: at '%s' offset %d opcode %s\n",
          fp->func()->fullName()->data(), callOffset,
          opcodeToName(peek_op(callPC)));
    // Don't step out to generated or builtin functions, keep looking.
    if (fp->func()->line1() == 0) continue;
    if (fp->func()->isBuiltin()) continue;
    if (fromVMEntry) {
      TRACE(2, "CmdFlowControl::setupStepOuts: VM entry\n");
      // We only execute this for opcodes which invoke more PHP, and that does
      // not include switches. Thus, we'll have at most two destinations.
      auto const reentrantOp = peek_op(callPC);
      assertx(!isSwitch(reentrantOp) && numSuccs(callPC) <= 2);
      // Set an internal breakpoint after the instruction if it can fall thru.
      if (instrAllowsFallThru(reentrantOp)) {
        Offset nextOffset = callOffset + instrLen(callPC);
        TRACE(2, "CmdFlowControl: step out to '%s' offset %d (fall-thru)\n",
              fp->func()->fullName()->data(), nextOffset);
        m_stepOut1 = StepDestination(fp->func(), nextOffset);
      }
      // Set an internal breakpoint at the target of a control flow instruction.
      // A good example of a control flow op that invokes PHP is IterNext.
      if (instrIsControlFlow(reentrantOp)) {
        auto const targets = instrJumpTargets(callPC, 0);
        if (!targets.empty()) {
          assertx(targets.size() == 1);
          Offset targetOffset = callOffset + targets[0];
          TRACE(2, "CmdFlowControl: step out to '%s' offset %d (jump target)\n",
                fp->func()->fullName()->data(), targetOffset);
          m_stepOut2 = StepDestination(fp->func(), targetOffset);
        }
      }
      // If we have no place to step out to, then unwind another frame and try
      // again. The most common case that leads here is Ret*, which does not
      // fall-thru and has no encoded target.
    } else {
      auto const returnOffset = fp->func()->offsetOf(skipCall(callPC));
      TRACE(2, "CmdFlowControl: step out to '%s' offset %d\n",
            fp->func()->fullName()->data(), returnOffset);
      m_stepOut1 = StepDestination(fp->func(), returnOffset);
    }
  }
}

void CmdFlowControl::cleanupStepOuts() {
  if (m_stepOut1.valid()) {
    m_stepOut1 = StepDestination();
  }
  if (m_stepOut2.valid()) {
    m_stepOut2 = StepDestination();
  }
}

///////////////////////////////////////////////////////////////////////////////
// StepDestination
//
// NB: a StepDestination also manages an internal breakpoint at the
// given location. If there is already an internal breakpoint set when
// a StepDestination is constructed then it will not remove the
// breakpoint when it is destructed. The move assignment operator
// handles the transfer of ownership, and we delete the copy
// constructor/assignment operators explicitly to ensure no two
// StepDestinations believe they can remove the same internal
// breakpoint.
//
// We can manage internal breakpoints without a true refcount like
// this because no other operation can manipulate the breakpoint
// filter while a single flow control command is active, and because
// the lifetime of a StepDestination is scoped by the lifetime of its
// flow control command.

CmdFlowControl::StepDestination::StepDestination() :
    m_func(nullptr), m_offset(kInvalidOffset),
    m_ownsInternalBreakpoint(false)
{
}

CmdFlowControl::StepDestination::StepDestination(const Func* func,
                                                 Offset offset) :
    m_func(func), m_offset(offset)
{
  m_ownsInternalBreakpoint = !phpHasBreakpoint(m_func, m_offset);
  if (m_ownsInternalBreakpoint) phpAddBreakPoint(m_func, m_offset);
}

CmdFlowControl::StepDestination::StepDestination(StepDestination&& other) {
  *this = std::move(other);
}

CmdFlowControl::StepDestination&
CmdFlowControl::StepDestination::operator=(StepDestination&& other) {
  if (this != &other) {
    if (m_ownsInternalBreakpoint) phpRemoveBreakPoint(m_func, m_offset);
    m_func = other.m_func;
    m_offset = other.m_offset;
    m_ownsInternalBreakpoint = other.m_ownsInternalBreakpoint;
    other.m_func = nullptr;
    other.m_offset = kInvalidOffset;
    other.m_ownsInternalBreakpoint = false;
  }
  return *this;
}

CmdFlowControl::StepDestination::~StepDestination() {
  if (m_ownsInternalBreakpoint) phpRemoveBreakPoint(m_func, m_offset);
}

///////////////////////////////////////////////////////////////////////////////
}}
