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

#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/debugger/break_point.h"
#include "hphp/runtime/debugger/debugger.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/ext/generator/ext_generator.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/pc-filter.h"
#include "hphp/util/logger.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debuggerflow);
using jit::mcg;

using StepOutState = RequestInjectionData::StepOutState;

//////////////////////////////////////////////////////////////////////////
// DebugHookHandler implementation

void DebugHookHandler::detach(ThreadInfo* ti /* = nullptr */) {
  // legacy hphpd code expects no failure if no hook handler is attached
  ti = (ti != nullptr) ? ti : &TI();
  if (!isDebuggerAttached(ti)) {
    return;
  }

  ti->m_reqInjectionData.setDebuggerAttached(false);

  // do not remove/delete m_hookHandler, its a singleton, and
  // code in another thread could be using it.

  if (ti == &TI()) {
    // Clear the pc filters
    // We can only do this for the current thread
    ti->m_reqInjectionData.m_breakPointFilter.clear();
    ti->m_reqInjectionData.m_flowFilter.clear();
    ti->m_reqInjectionData.m_lineBreakPointFilter.clear();
    ti->m_reqInjectionData.m_callBreakPointFilter.clear();
    ti->m_reqInjectionData.m_retBreakPointFilter.clear();
  }

  // Disble function entry/exit events
  ti->m_reqInjectionData.clearFlag(DebuggerHookFlag);

  // If there are no more handlers attached, clear the blacklist
  Lock lock(s_lock);
  if (--s_numAttached == 0) {
    mcg->tx().clearDbgBL();
  }
}

Mutex DebugHookHandler::s_lock;
int DebugHookHandler::s_numAttached = 0;

//////////////////////////////////////////////////////////////////////////
// Helpers

namespace {

// Ensure we interpret all code at the given offsets. This sets up a guard for
// each piece of translated code to ensure we punt to the interpreter when the
// debugger is attached.
void blacklistRangesInJit(const Unit* unit, const OffsetRangeVec& offsets) {
  for (auto const& range : offsets) {
    for (PC pc = unit->at(range.base); pc < unit->at(range.past);
         pc += instrLen((Op*)pc)) {
      mcg->tx().addDbgBLPC(pc);
    }
  }
  if (!mcg->addDbgGuards(unit)) {
    Logger::Warning("Failed to set breakpoints in Jitted code");
  }
  // In this case, we may be setting a breakpoint in a tracelet which could
  // already be jitted, and present on the stack. Make sure we don't return
  // to it so we have a chance to honor breakpoints.
  debuggerPreventReturnsToTC();
}

// Ensure we interpret an entire function when the debugger is attached.
void blacklistFuncInJit(const Func* f) {
  auto unit = f->unit();
  OffsetRangeVec ranges;
  ranges.push_back(OffsetRange(f->base(), f->past()));
  blacklistRangesInJit(unit, ranges);
}

PCFilter* getBreakPointFilter() {
  return &RID().m_breakPointFilter;
}

PCFilter* getFlowFilter() {
  return &RID().m_flowFilter;
}

}

//////////////////////////////////////////////////////////////////////////
// Hooks

// Hook called from the bytecode interpreter before every opcode executed while
// a debugger is attached. The debugger may choose to hold the thread below
// here and execute any number of commands from the client. Return from here
// lets the opcode execute.
void phpDebuggerOpcodeHook(const unsigned char* pc) {
  TRACE(5, "in phpDebuggerOpcodeHook() with pc %p\n", pc);
  // Short-circuit when we're doing things like evaling PHP for print command,
  // or conditional breakpoints.
  if (UNLIKELY(g_context->m_dbgNoBreak)) {
    TRACE(5, "NoBreak flag is on\n");
    return;
  }
  auto& req_data = RID();
  // Short-circuit for cases where we're executing a line of code that we know
  // we don't need an interrupt for, e.g., stepping over a line of code.
  if (UNLIKELY(req_data.m_flowFilter.checkPC(pc))) {
    TRACE_RB(5, "Location filter hit at pc %p\n", pc);
    return;
  }
  // Are we hitting a breakpoint?
  if (LIKELY(!req_data.m_breakPointFilter.checkPC(pc))) {
    TRACE(5, "not in the PC range for any breakpoints\n");
    if (LIKELY(!DEBUGGER_FORCE_INTR)) {
      return;
    }
    TRACE_RB(5, "DEBUGGER_FORCE_INTR or DEBUGGER_ACTIVE_LINE_BREAKS\n");
  }

  // Notify the hook handler. This is necessary for compatibility with hphpd
  auto handler = getHookHandler();
  handler->onOpcode(pc);

  // Try to grab needed context information
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
    req_data.setActiveLineBreak(-1);
  }

  // Check if the step in command is active. Special case builtins because they
  // are meaningless to the user
  if (UNLIKELY(req_data.getDebuggerStepIn() && !func->isBuiltin())) {
    req_data.setDebuggerStepIn(false);
    if (!req_data.getDebuggerNext()) {
      // Next command is not active, just break.
      handler->onStepInBreak(unit, line);
    } else if (req_data.getDebuggerStackDepth() <=
                req_data.getDebuggerFlowDepth()) {
      // Next command is active but we didn't step in. We are done.
      req_data.setDebuggerNext(false);
      handler->onNextBreak(unit, line);
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
  // original, then we skip over the PopR opcode if it exists and then break
  // (matching hphpd).
  if (UNLIKELY(req_data.getDebuggerStepOut() == StepOutState::OUT &&
      req_data.getDebuggerStackDepth() < req_data.getDebuggerFlowDepth() &&
      *reinterpret_cast<const Op*>(pc) != OpPopR)) {
    req_data.setDebuggerStepOut(StepOutState::NONE);
    if (!req_data.getDebuggerNext()) {
      // Next command not active, break
      handler->onStepOutBreak(unit, line);
    } else {
      // Next command is active, but it is done. Break.
      req_data.setDebuggerNext(false);
      handler->onNextBreak(unit, line);
    }
  }

  // Check if we are hitting a call breakpoint
  if (UNLIKELY(req_data.m_callBreakPointFilter.checkPC(pc))) {
    handler->onFuncEntryBreak(func);
  }

  // Check if we are hitting a return breakpoint
  if (UNLIKELY(req_data.m_retBreakPointFilter.checkPC(pc))) {
    handler->onFuncExitBreak(func);
  }

  // Check if we are hitting a line breakpoint. Also ensure the current line
  // hasn't already been set as the active line breakpoint.
  if (UNLIKELY(req_data.m_lineBreakPointFilter.checkPC(pc) &&
               active_line != line)) {
    req_data.setActiveLineBreak(line);
    handler->onLineBreak(unit, line);
  }

  TRACE(5, "out phpDebuggerOpcodeHook()\n");
}

// Hook called on request start before main() is invoked
void phpDebuggerRequestInitHook() {
  getHookHandler()->onRequestInit();
}

// Hook called on request shutdown after main() exits
void phpDebuggerRequestShutdownHook() {
  getHookHandler()->onRequestShutdown();
}

// Hook called on function entry. Since function entry breakpoints are handled
// by onOpcode, this just handles pushing the active line breakpoint
void phpDebuggerFuncEntryHook(const ActRec* ar) {
  RID().pushActiveLineBreak(-1);
}

// Hook called on function exit. onOpcode handles function exit breakpoints,
// this just handles stack-related manipulations. This handles returns,
// suspends, and exceptions.
void phpDebuggerFuncExitHook(const ActRec* ar) {
  auto& req_data = RID();
  req_data.popActiveLineBreak();

  // If the step out command is active and if our stack depth has decreased,
  // we are out of the function being stepped out of
  if (UNLIKELY(req_data.getDebuggerStepOut() == StepOutState::STEPPING &&
      req_data.getDebuggerStackDepth() < req_data.getDebuggerFlowDepth())) {
      req_data.setDebuggerStepOut(StepOutState::OUT);
  }
}

// Hook called from iopThrow to signal that we are about to throw an exception.
void phpDebuggerExceptionThrownHook(ObjectData* exception) {
TRACE(5, "in phpDebuggerExceptionThrownHook()\n");
if (UNLIKELY(g_context->m_dbgNoBreak)) {
    TRACE(5, "NoBreak flag is on\n");
    return;
  }
  getHookHandler()->onExceptionThrown(exception);
  TRACE(5, "out phpDebuggerExceptionThrownHook()\n");
}

// Hook called from exception unwind to signal that we are about to handle an
// exception.
void phpDebuggerExceptionHandlerHook() noexcept {
  try {
    TRACE(5, "in phpDebuggerExceptionHandlerHook()\n");
    if (UNLIKELY(g_context->m_dbgNoBreak)) {
      TRACE(5, "NoBreak flag is on\n");
      return;
    }
    getHookHandler()->onExceptionHandle();
    TRACE(5, "out phpDebuggerExceptionHandlerHook()\n");
  } catch (...) {
  }
}

// Hook called when the VM raises an error.
void phpDebuggerErrorHook(const ExtendedException &ee,
                          int errnum,
                          const std::string& message) {
  TRACE(5, "in phpDebuggerErrorHook()\n");
  if (UNLIKELY(g_context->m_dbgNoBreak)) {
    TRACE(5, "NoBreak flag is on\n");
    return;
  }
  getHookHandler()->onError(ee, errnum, message);
  TRACE(5, "out phpDebuggerErrorHook()\n");
}

void phpDebuggerEvalHook(const Func* f) {
  if (RuntimeOption::EvalJit) {
    blacklistFuncInJit(f);
  }
  getHookHandler()->onEval(f);
}

// Called by the VM when a file is loaded.
void phpDebuggerFileLoadHook(Unit* unit) {
  getHookHandler()->onFileLoad(unit);
}

// Called by the VM when a class definition is loaded.
void phpDebuggerDefClassHook(const Class* cls) {
  getHookHandler()->onDefClass(cls);
}

// Called by the VM when a function definition is loaded.
void phpDebuggerDefFuncHook(const Func* func) {
  getHookHandler()->onDefFunc(func);
}


//////////////////////////////////////////////////////////////////////////
// Flow Control

void phpDebuggerContinue() {
  // Short-circuit other commands
  auto& req_data = RID();
  req_data.setDebuggerStepIn(false);
  req_data.setDebuggerStepOut(StepOutState::NONE);
  req_data.setDebuggerNext(false);

  // Clear the flow filter
  auto flow_filter = getFlowFilter();
  flow_filter->clear();
}

void phpDebuggerStepIn() {
  // If this is called in the middle of a flow command we short-circuit the
  // other commands
  auto& req_data = RID();
  req_data.setDebuggerStepIn(true);
  req_data.setDebuggerStepOut(StepOutState::NONE);
  req_data.setDebuggerNext(false);

  // Ensure the flow filter is fresh
  auto flow_filter = getFlowFilter();
  flow_filter->clear();

  // Check if the site is valid.
  VMRegAnchor _;
  ActRec* fp = vmfp();
  PC pc = vmpc();
  if (fp == nullptr || pc == nullptr) {
    TRACE(5, "Could not grab stack or program counter\n");
    return;
  }

  // Try to get needed context info. Bail if we can't
  const Func* func = fp->func();
  const Unit* unit = func != nullptr ? func->unit() : nullptr;
  if (func == nullptr || func == nullptr) {
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
  // If this is called in the middle of a flow command we short-circuit the
  // other commands
  auto& req_data = RID();
  req_data.setDebuggerStepIn(false);
  req_data.setDebuggerStepOut(StepOutState::STEPPING);
  req_data.setDebuggerNext(false);

  // Clear the flow filter
  auto flow_filter = getFlowFilter();
  flow_filter->clear();

  // Store the current stack depth
  req_data.setDebuggerFlowDepth(req_data.getDebuggerStackDepth());
}

void phpDebuggerNext() {
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
}

//////////////////////////////////////////////////////////////////////////
// Breakpoint manipulation

void phpAddBreakPoint(const Unit* unit, Offset offset) {
  PC pc = unit->at(offset);
  getBreakPointFilter()->addPC(pc);
  if (RuntimeOption::EvalJit) {
    if (mcg->tx().addDbgBLPC(pc)) {
      // if a new entry is added in blacklist
      if (!mcg->addDbgGuards(unit)) {
        Logger::Warning("Failed to set breakpoints in Jitted code");
      }
      // In this case, we may be setting a breakpoint in a tracelet which could
      // already be jitted, and present on the stack. Make sure we don't return
      // to it so we have a chance to honor breakpoints.
      debuggerPreventReturnsToTC();
    }
  }
}

void phpAddBreakPointRange(const Unit* unit, OffsetRangeVec& offsets) {
  getBreakPointFilter()->addRanges(unit, offsets);
  if (RuntimeOption::EvalJit) {
    blacklistRangesInJit(unit, offsets);
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
  if (RuntimeOption::EvalJit) {
    if (mcg->tx().addDbgBLPC(pc)) {
      // if a new entry is added in blacklist
      if (!mcg->addDbgGuard(f, base, false)) {
        Logger::Warning("Failed to set breakpoints in Jitted code");
      }
    }
  }
}

void phpAddBreakPointFuncExit(const Func* f) {
  // Iterate through the function's opcodes and place breakpoints on each RetC
  const Unit* unit = f->unit();
  for (PC pc = unit->at(f->base()); pc < unit->at(f->past());
       pc += instrLen((Op*) pc)) {
    if (*reinterpret_cast<const Op*>(pc) != OpRetC) {
      continue;
    }

    // Add pc to the breakpoint filter and the func exit filter
    getBreakPointFilter()->addPC(pc);
    RID().m_retBreakPointFilter.addPC(pc);

    // Blacklist the location
    if (RuntimeOption::EvalJit && mcg->tx().addDbgBLPC(pc)) {
      if (!mcg->addDbgGuard(f, unit->offsetOf(pc), false)) {
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

  // Add to the breakpoint filter and the line filter
  phpAddBreakPointRange(unit, offsets);
  RID().m_lineBreakPointFilter.addRanges(unit, offsets);
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
       pc += instrLen((Op*) pc)) {
    if (*reinterpret_cast<const Op*>(pc) == OpRetC) {
      req_data.m_retBreakPointFilter.removePC(pc);
    }
  }
}

void phpRemoveBreakPointLine(const Unit* unit, int line) {
  // See note in debugger-hook.h. This can only remove from the line filter
  OffsetRangeVec offsets;
  if (unit->getOffsetRanges(line, offsets)) {
    ThreadInfo::s_threadInfo->
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

/////////////////////////////////////////////////////////////////////////
}
