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

#include "hphp/runtime/vm/debugger-hook.h"

#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/debugger/break_point.h"
#include "hphp/runtime/debugger/debugger.h"
#include "hphp/runtime/debugger/debugger_hook_handler.h"
#include "hphp/runtime/ext/generator/ext_generator.h"
#include "hphp/runtime/vm/async-flow-stepper.h"
#include "hphp/runtime/vm/hhbc-codec.h"
#include "hphp/runtime/vm/jit/debugger.h"
#include "hphp/runtime/vm/jit/mcgen.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/pc-filter.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/util/logger.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debuggerflow);

using StepOutState = RequestInjectionData::StepOutState;

//////////////////////////////////////////////////////////////////////////
// DebuggerHook implementation

void DebuggerHook::detach(RequestInfo* ti /* = nullptr */) {
  // Legacy hphpd code expects no failure if no hook is attached.
  ti = (ti != nullptr) ? ti : &RI();
  if (!isDebuggerAttached(ti)) {
    return;
  }

  ti->m_reqInjectionData.setDebuggerAttached(false);

  // Do not remove/delete m_debuggerHook, it's a singleton, and code in another
  // thread could be using it.

  if (ti == &RI()) {
    // Clear the pc filters.  We can only do this for the current thread.
    ti->m_reqInjectionData.m_breakPointFilter.clear();
    ti->m_reqInjectionData.m_flowFilter.clear();
    ti->m_reqInjectionData.m_lineBreakPointFilter.clear();
    ti->m_reqInjectionData.m_callBreakPointFilter.clear();
    ti->m_reqInjectionData.m_retBreakPointFilter.clear();
  }

  // Disble function entry/exit events
  ti->m_reqInjectionData.clearFlag(DebuggerHookFlag);

  // If there are no more hooks attached, clear the blacklist.
  Lock lock(s_lock);
  if (--s_numAttached == 0) {
    s_activeHook = nullptr;
    jit::clearDbgBL();
  }
}

Mutex DebuggerHook::s_lock;
int DebuggerHook::s_numAttached {0};
DebuggerHook* DebuggerHook::s_activeHook {nullptr};

//////////////////////////////////////////////////////////////////////////
// Helpers

namespace {

// Ensure we interpret an entire function when the debugger is attached.
void blacklistFuncInJit(const Func* f) {
  if (jit::addDbgBLFunc(f)) {
    if (!jit::tc::addDbgGuards(f)) {
      Logger::Warning("Failed to set breakpoints in Jitted code");
    }
    // In this case, we may be setting a breakpoint in a tracelet which could
    // already be jitted, and present on the stack. Make sure we don't return
    // to it so we have a chance to honor breakpoints.
    debuggerPreventReturnsToTC();
  }
}

}

//////////////////////////////////////////////////////////////////////////
// Hooks

// Hook called from the bytecode interpreter before every opcode executed while
// a debugger is attached. The debugger may choose to hold the thread below
// here and execute any number of commands from the client. Return from here
// lets the opcode execute.
void phpDebuggerOpcodeHook(const unsigned char* pc) {
  VMRegAnchor anchor;
  TRACE(5, "in phpDebuggerOpcodeHook() with pc %p\n", pc);
  // Short-circuit when we're doing things like evaling PHP for print command,
  // or conditional breakpoints.
  if (UNLIKELY(g_context->m_dbgNoBreak)) {
    TRACE(5, "NoBreak flag is on\n");
    return;
  }

  auto& req_data = RID();
  // Are we hitting a breakpoint?
  if (LIKELY(!req_data.m_breakPointFilter.checkPC(pc))) {
    TRACE(5, "not in the PC range for any breakpoints\n");
    // Short-circuit for cases where we're executing a line of code that we know
    // we don't need an interrupt for, e.g., stepping over a line of code.
    if (UNLIKELY(req_data.m_flowFilter.checkPC(pc))) {
      TRACE_RB(5, "Location filter hit at pc %p\n", pc);
      return;
    }
    if (LIKELY(!DEBUGGER_FORCE_INTR)) {
      return;
    }
    TRACE_RB(5, "DEBUGGER_FORCE_INTR or DEBUGGER_ACTIVE_LINE_BREAKS\n");
  }

  // Notify the hook.
  auto hook = getDebuggerHook();
  hook->onOpcode(pc);

  // Try to grab needed context information.
  const ActRec* fp = g_context->getStackFrame();
  const Func* func = fp != nullptr ? fp->func() : nullptr;
  const Unit* unit = func != nullptr ? func->unit() : nullptr;
  if (UNLIKELY(unit == nullptr)) {
    TRACE(5, "Could not grab stack information\n");
    return;
  }

  // We can't set breakpoints in generated functions
  if (UNLIKELY(func->line1() == 0)) {
    TRACE(5, "In a generated function\n");
    return;
  }

  // If we are no longer on the active line breakpoint, clear it
  int active_line = req_data.getActiveLineBreak();
  int line = unit->getLineNumber(unit->offsetOf(pc));
  if (UNLIKELY(active_line != -1 && active_line != line)) {
    req_data.clearActiveLineBreak();
  }

  // Checking breakpoint before stepping logic.
  // Check if we are hitting a call breakpoint
  if (UNLIKELY(req_data.m_callBreakPointFilter.checkPC(pc))) {
    hook->onFuncEntryBreak(func);
    return;
  }

  // Check if we are hitting a return breakpoint
  if (UNLIKELY(req_data.m_retBreakPointFilter.checkPC(pc))) {
    hook->onFuncExitBreak(func);
    return;
  }

  // Check if we are hitting a line breakpoint.
  if (UNLIKELY(active_line != line &&
               req_data.m_lineBreakPointFilter.checkPC(pc))) {
    req_data.setActiveLineBreak(line);
    hook->onLineBreak(unit, line);
    return;
  }

  // Async stepper should handle before normal stepping logic.
  auto const handleResult = req_data.m_asyncStepper.handleOpcode(pc);
  if (handleResult != AsyncStepHandleOpcodeResult::Unhandled) {
    if (handleResult == AsyncStepHandleOpcodeResult::Completed) {
      hook->onNextBreak(unit, line);
    }
    // Handled by async stepper.
    return;
  }

  auto curStackDisp = getStackDisposition(req_data.getDebuggerFlowDepth());
  // Check if the step in command is active. Special case builtins because they
  // are meaningless to the user
  if (UNLIKELY(req_data.getDebuggerStepIn() && !func->isBuiltin())) {
    req_data.setDebuggerStepIn(false);
    if (!req_data.getDebuggerNext()) {
      // Next command is not active, just break.
      hook->onStepInBreak(unit, line);
      return;
    } else if (curStackDisp != StackDepthDisposition::Deeper) {
      // Next command is active but we didn't step in. We are done.
      req_data.setDebuggerNext(false);
      hook->onNextBreak(unit, line);
      return;
    } else {
      // Next command is active and we stepped in. Step out, but save the filter
      // first, as it is cleared when we step out.
      PCFilter filter;
      req_data.m_flowFilter.swap(filter);
      phpDebuggerStepOut();

      // Restore the saved filter and the next flag
      req_data.m_flowFilter.swap(filter);
      req_data.setDebuggerNext(true);
    }
  }

  // If the current state is OUT and we are still at a stack level less than the
  // original, then we skip over the PopC opcode if it exists and then break
  // (matching hphpd).
  if (UNLIKELY(req_data.getDebuggerStepOut() == StepOutState::Out &&
      curStackDisp == StackDepthDisposition::Shallower &&
      peek_op(pc) != OpPopC)) {
    req_data.setDebuggerStepOut(StepOutState::None);
    if (!req_data.getDebuggerNext()) {
      // Next command not active, break
      hook->onStepOutBreak(unit, line);
      return;
    } else {
      // Next command is active, but it is done. Break.
      req_data.setDebuggerNext(false);
      hook->onNextBreak(unit, line);
      return;
    }
  }
  TRACE(5, "out phpDebuggerOpcodeHook()\n");
}

StackDepthDisposition getStackDisposition(int baseline) {
  auto& req_data = RID();
  auto curStackDepth = req_data.getDebuggerStackDepth();
  if (curStackDepth > baseline) {
    return StackDepthDisposition::Deeper;
  } else if (curStackDepth < baseline) {
    return StackDepthDisposition::Shallower;
  }
  return StackDepthDisposition::Equal;
}

// Hook called on request start before main() is invoked
void phpDebuggerRequestInitHook() {
  VMRegAnchor anchor;
  getDebuggerHook()->onRequestInit();
}

// Hook called on function entry. Since function entry breakpoints are handled
// by onOpcode, this just handles pushing the active line breakpoint
void phpDebuggerFuncEntryHook(const ActRec* /*ar*/) {
  VMRegAnchor anchor;
  RID().pushActiveLineBreak();
}

// Hook called on function exit. onOpcode handles function exit breakpoints,
// this just handles stack-related manipulations. This handles returns,
// suspends, and exceptions.
void phpDebuggerFuncExitHook(const ActRec* /*ar*/) {
  VMRegAnchor anchor;
  auto& req_data = RID();
  req_data.popActiveLineBreak();

  // If the step out command is active and if our stack depth has decreased,
  // we are out of the function being stepped out of
  auto baseline = req_data.getDebuggerFlowDepth();
  if (UNLIKELY(req_data.getDebuggerStepOut() == StepOutState::Stepping &&
      getStackDisposition(baseline) == StackDepthDisposition::Shallower)) {
      req_data.setDebuggerStepOut(StepOutState::Out);
  }
}

// Hook called from iopThrow to signal that we are about to throw an exception.
void phpDebuggerExceptionThrownHook(ObjectData* exception) {
  VMRegAnchor anchor;
  TRACE(5, "in phpDebuggerExceptionThrownHook()\n");
  if (UNLIKELY(g_context->m_dbgNoBreak)) {
    TRACE(5, "NoBreak flag is on\n");
    return;
  }
  RID().m_asyncStepper.handleExceptionThrown();
  getDebuggerHook()->onExceptionThrown(exception);
  TRACE(5, "out phpDebuggerExceptionThrownHook()\n");
}

// Hook called from exception unwind to signal that we are about to handle an
// exception.
void phpDebuggerExceptionHandlerHook() noexcept {
  try {
    VMRegAnchor anchor;
    TRACE(5, "in phpDebuggerExceptionHandlerHook()\n");
    if (UNLIKELY(g_context->m_dbgNoBreak)) {
      TRACE(5, "NoBreak flag is on\n");
      return;
    }
    if (RID().m_asyncStepper.handleExceptionHandler()) {
      auto const fp = g_context->getStackFrame();
      auto const func = fp != nullptr ? fp->func() : nullptr;
      auto const unit = func != nullptr ? func->unit() : nullptr;
      if (UNLIKELY(unit != nullptr)) {
        TRACE(5, "Could not grab stack information\n");
        return;
      }
      const auto pc = vmpc();
      auto line = unit->getLineNumber(unit->offsetOf(pc));
      getDebuggerHook()->onNextBreak(unit, line);
      return;
    }
    getDebuggerHook()->onExceptionHandle();
    TRACE(5, "out phpDebuggerExceptionHandlerHook()\n");
  } catch (...) {
  }
}

// Hook called when the VM raises an error.
void phpDebuggerErrorHook(const ExtendedException& ee,
                          int errnum,
                          const std::string& message) {
  VMRegAnchor anchor;
  TRACE(5, "in phpDebuggerErrorHook()\n");
  if (UNLIKELY(g_context->m_dbgNoBreak)) {
    TRACE(5, "NoBreak flag is on\n");
    return;
  }
  getDebuggerHook()->onError(ee, errnum, message);
  TRACE(5, "out phpDebuggerErrorHook()\n");
}

void phpDebuggerEvalHook(const Func* f) {
  VMRegAnchor anchor;
  if (RuntimeOption::EvalJit) {
    blacklistFuncInJit(f);
  }
  getDebuggerHook()->onEval(f);
}

// Called by the VM when a file is loaded.
void phpDebuggerFileLoadHook(Unit* unit) {
  VMRegAnchor anchor;
  getDebuggerHook()->onFileLoad(unit);
}

// Called by the VM when a class definition is loaded.
void phpDebuggerDefClassHook(const Class* cls) {
  VMRegAnchor anchor;
  getDebuggerHook()->onDefClass(cls);
}

// Called by the VM when a function definition is loaded.
void phpDebuggerDefFuncHook(const Func* func) {
  VMRegAnchor anchor;
  getDebuggerHook()->onDefFunc(func);
}

// Called by the VM when a function intercept is registered.
void phpDebuggerInterceptRegisterHook(const String& name) {
  VMRegAnchor anchor;
  auto hook = getDebuggerHook();
  if (hook != nullptr) {
    hook->onRegisterFuncIntercept(name);
  }
}

//////////////////////////////////////////////////////////////////////////
// Flow Control

void phpDebuggerContinue() {
  VMRegAnchor anchor;
  // Short-circuit other commands
  auto& req_data = RID();
  req_data.setDebuggerStepIn(false);
  req_data.setDebuggerStepOut(StepOutState::None);
  req_data.setDebuggerNext(false);

  // Clear the flow filter
  auto flow_filter = getFlowFilter();
  flow_filter->clear();
}

void phpDebuggerStepIn() {
  VMRegAnchor anchor;
  // If this is called in the middle of a flow command we short-circuit the
  // other commands
  auto& req_data = RID();
  req_data.setDebuggerStepIn(true);
  req_data.setDebuggerStepOut(StepOutState::None);
  req_data.setDebuggerNext(false);

  // Ensure the flow filter is fresh
  auto flow_filter = getFlowFilter();
  flow_filter->clear();

  // Check if the site is valid.
  ActRec* fp = vmfp();
  PC pc = vmpc();
  if (fp == nullptr || pc == nullptr) {
    TRACE(5, "Could not grab stack or program counter\n");
    return;
  }

  // Try to get needed context info. Bail if we can't
  const Func* func = fp->func();
  const Unit* unit = func != nullptr ? func->unit() : nullptr;
  if (unit == nullptr) {
    TRACE(5, "Could not grab the current unit or function\n");
    return;
  }

  // We use line1 here because it works better than line0 in our
  // bytecode-source mapping.
  int line;
  SourceLoc source_loc;
  if (unit->getSourceLoc(unit->offsetOf(pc), source_loc)) {
    line = source_loc.line1;
  } else {
    TRACE(5, "Could not grab the current line number\n");
    return;
  }

  TRACE(3, "Prepare location filter for %s:%d, unit %p:\n",
        unit->filepath()->data(), line, unit);

  // Get offset ranges for the whole line.
  OffsetRangeVec ranges;
  if (!unit->getOffsetRanges(line, ranges)) {
    ranges.clear();
  }

  flow_filter->addRanges(unit, ranges);
}

void phpDebuggerStepOut() {
  VMRegAnchor anchor;
  // If this is called in the middle of a flow command we short-circuit the
  // other commands
  auto& req_data = RID();
  req_data.setDebuggerStepIn(false);
  req_data.setDebuggerStepOut(StepOutState::Stepping);
  req_data.setDebuggerNext(false);

  // Clear the flow filter
  auto flow_filter = getFlowFilter();
  flow_filter->clear();

  // Store the current stack depth
  req_data.setDebuggerFlowDepth(req_data.getDebuggerStackDepth());
}

void phpDebuggerNext() {
  VMRegAnchor anchor;
  // Grab the request data and set up a step in
  auto& req_data = RID();
  phpDebuggerStepIn();

  // Special case the top-level pseudo-main. What the user expects is a
  // "step-in" into pseudo-main, but the implementation otherwise would just
  // step over it.
  int stack_depth = req_data.getDebuggerStackDepth();
  if (stack_depth == 0) {
    return;
  }

  // Turn the next flag on. This indicates that we should do a step out after
  // our step completes if the stack depth has increased.
  req_data.setDebuggerNext(true);
  req_data.setDebuggerFlowDepth(stack_depth);

  // Setup async stepping if needed.
  req_data.m_asyncStepper.setup();

  // handleOpcode() here in case the current pc points to "await" opcode.
  auto pc = vmpc();
  req_data.m_asyncStepper.handleOpcode(pc);
}

//////////////////////////////////////////////////////////////////////////
// Breakpoint manipulation

void phpAddBreakPoint(const Unit* unit, Offset offset) {
  PC pc = unit->at(offset);
  getBreakPointFilter()->addPC(pc);
  if (RuntimeOption::EvalJit && !RuntimeOption::ForceDebuggerBpToInterp) {
    auto const func = unit->getFunc(offset);
    always_assert(func);
    if (jit::addDbgBLFunc(func)) {
      // if a new entry is added in blacklist
      if (!jit::tc::addDbgGuards(func)) {
        Logger::Warning("Failed to set breakpoints in Jitted code");
      }
      // In this case, we may be setting a breakpoint in a tracelet which could
      // already be jitted, and present on the stack. Make sure we don't return
      // to it so we have a chance to honor breakpoints.
      debuggerPreventReturnsToTC();
    }
  }
}

void phpAddBreakPointFuncEntry(const Func* f) {
  // we are in a generator, skip CreateCont / RetC / PopC opcodes
  auto base = f->isGenerator()
    ? BaseGenerator::userBase(f)
    : f->base();
  auto pc = f->unit()->at(base);

  TRACE(5, "func() break %s : unit %p offset %d ==> pc %p)\n",
        f->fullName()->data(), f->unit(), base, pc);

  // Add to the breakpoint filter and the func entry filter
  getBreakPointFilter()->addPC(pc);
  RID().m_callBreakPointFilter.addPC(pc);

  // Blacklist the location
  if (RuntimeOption::EvalJit && !RuntimeOption::ForceDebuggerBpToInterp) {
    if (jit::addDbgBLFunc(f)) {
      // if a new entry is added in blacklist
      if (!jit::tc::addDbgGuards(f)) {
        Logger::Warning("Failed to set breakpoints in Jitted code");
      }
    }
  }
}

void phpAddBreakPointFuncExit(const Func* f) {
  // Iterate through the function's opcodes and place breakpoints on each RetC
  const Unit* unit = f->unit();
  for (PC pc = unit->at(f->base()); pc < unit->at(f->past());
       pc += instrLen(pc)) {
    if (peek_op(pc) != OpRetC && peek_op(pc) != OpRetCSuspended &&
        peek_op(pc) != OpRetM) {
      continue;
    }

    // Add pc to the breakpoint filter and the func exit filter
    getBreakPointFilter()->addPC(pc);
    RID().m_retBreakPointFilter.addPC(pc);

    // Blacklist the location
    if (RuntimeOption::EvalJit && !RuntimeOption::ForceDebuggerBpToInterp &&
        jit::addDbgBLFunc(f)) {
      if (!jit::tc::addDbgGuards(f)) {
        Logger::Warning("Failed to set breakpoints in Jitted code");
      }
    }
  }
}

bool phpAddBreakPointLine(const Unit* unit, int line) {
  // Grab the unit offsets
  OffsetRangeVec offsets;
  if (!unit->getOffsetRanges(line, offsets)) {
    return false;
  }

  // Add to the breakpoint filter and the line filter.
  assertx(offsets.size() > 0);
  bool containsEntryNop = false;
  for (auto const offset : offsets) {
    auto bpOffset = offset.base;
    auto op = unit->getOp(bpOffset);
    if (op == Op::EntryNop) {
      containsEntryNop = true;
    }

    if (containsEntryNop) {
      phpAddBreakPoint(unit, offset.base);
    }
  }

  if (containsEntryNop) {
    RID().m_lineBreakPointFilter.addRanges(unit, offsets);
  } else {
    auto bpOffset = offsets[0].base;
    phpAddBreakPoint(unit, bpOffset);

    auto pc = unit->at(bpOffset);
    RID().m_lineBreakPointFilter.addPC(pc);
  }

  return true;
}

void phpRemoveBreakPoint(const Unit* unit, Offset offset) {
  auto const pc = unit->at(offset);
  RID().m_breakPointFilter.removePC(pc);
}

void phpRemoveBreakPointFuncEntry(const Func* f) {
  // See note in debugger-hook.h. This can only remove from the function entry
  // filter
  auto base = f->isGenerator() ? BaseGenerator::userBase(f) : f->base();
  auto pc = f->unit()->at(base);
  RID().m_callBreakPointFilter.removePC(pc);
}

void phpRemoveBreakPointFuncExit(const Func* f) {
  // See note in debugger-hook.h. This can only remove from the function exit
  // filter
  const Unit* unit = f->unit();
  auto& req_data = RID();
  for (PC pc = unit->at(f->base()); pc < unit->at(f->past());
       pc += instrLen(pc)) {
    if (peek_op(pc) == OpRetC || peek_op(pc) == OpRetCSuspended ||
        peek_op(pc) == OpRetM) {
      req_data.m_retBreakPointFilter.removePC(pc);
    }
  }
}

void phpRemoveBreakPointLine(const Unit* unit, int line) {
  // See note in debugger-hook.h. This can only remove from the line filter
  OffsetRangeVec offsets;
  if (unit->getOffsetRanges(line, offsets)) {
    RequestInfo::s_requestInfo->
      m_reqInjectionData.m_lineBreakPointFilter.removeRanges(unit, offsets);
  }
}

bool phpHasBreakpoint(const Unit* unit, Offset offset) {
  auto& req_data = RID();
  if (!req_data.m_breakPointFilter.isNull()) {
    auto const pc = unit->at(offset);
    return req_data.m_breakPointFilter.checkPC(pc);
  }
  return false;
}

PCFilter* getBreakPointFilter() {
  return &RID().m_breakPointFilter;
}

PCFilter* getFlowFilter() {
  return &RID().m_flowFilter;
}

String getCurrentFilePath(int* pLine) {
  VMRegAnchor anchor;
  auto pc = vmpc();
  auto const unit = vmfp()->func()->unit();
  if (pLine != nullptr) {
    *pLine = unit->getLineNumber(unit->offsetOf(pc));
  }
  auto const filepath = const_cast<StringData*>(unit->filepath());
  return File::TranslatePath(String(filepath));
}

/////////////////////////////////////////////////////////////////////////
}
