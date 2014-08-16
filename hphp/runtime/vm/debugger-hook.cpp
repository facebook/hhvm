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
#include "hphp/runtime/ext/ext_generator.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/pc-filter.h"
#include "hphp/util/logger.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debuggerflow);
using JIT::mcg;

//////////////////////////////////////////////////////////////////////////
// DebugHookHandler implementation

void DebugHookHandler::detach(ThreadInfo* ti /* = nullptr */) {
  // legacy hphpd code expects no failure if no hook handler is attached
  ti = (ti != nullptr) ? ti : ThreadInfo::s_threadInfo.getNoCheck();
  if (ti->m_debugHookHandler == nullptr) {
    return;
  }

  delete ti->m_debugHookHandler;
  ti->m_debugHookHandler = nullptr;
  ti->m_reqInjectionData.setDebuggerAttached(false);
  s_numAttached--;
  EventHook::DisableDebug();
}

std::atomic_int DebugHookHandler::s_numAttached(0);

//////////////////////////////////////////////////////////////////////////
// Helpers

// Ensure we interpret all code at the given offsets. This sets up a guard for
// each piece of translated code to ensure we punt to the interpreter when the
// debugger is attached.
static void blacklistRangesInJit(const Unit* unit,
                                 const OffsetRangeVec& offsets) {
  for (OffsetRangeVec::const_iterator it = offsets.begin();
       it != offsets.end(); ++it) {
    for (PC pc = unit->at(it->m_base); pc < unit->at(it->m_past);
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
  g_context->preventReturnsToTC();
}

// Ensure we interpret an entire function when the debugger is attached.
static void blacklistFuncInJit(const Func* f) {
  Unit* unit = f->unit();
  OffsetRangeVec ranges;
  ranges.push_back(OffsetRange(f->base(), f->past()));
  blacklistRangesInJit(unit, ranges);
}

static PCFilter* getBreakPointFilter() {
  if (!g_context->m_breakPointFilter) {
    g_context->m_breakPointFilter = new PCFilter();
  }
  return g_context->m_breakPointFilter;
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
  // Short-circuit for cases where we're executing a line of code that we know
  // we don't need an interrupt for, e.g., stepping over a line of code.
  if (UNLIKELY(g_context->m_lastLocFilter != nullptr) &&
      g_context->m_lastLocFilter->checkPC(pc)) {
    TRACE_RB(5, "Location filter hit at pc %p\n", pc);
    return;
  }
  // Are we hitting a breakpoint?
  if (LIKELY(g_context->m_breakPointFilter == nullptr ||
      !g_context->m_breakPointFilter->checkPC(pc))) {
    TRACE(5, "not in the PC range for any breakpoints\n");
    if (LIKELY(!DEBUGGER_FORCE_INTR)) {
      return;
    }
    TRACE_RB(5, "DEBUGGER_FORCE_INTR or DEBUGGER_ACTIVE_LINE_BREAKS\n");
  }

  // Notify the hook handler. This is necessary for compatibility with hphpd
  DebugHookHandler* handler = getHookHandler();
  handler->onOpcode(pc);

  // Try to grab needed context information
  const ActRec* fp = g_context->getStackFrame();
  const Func* func = fp != nullptr ? fp->func() : nullptr;
  const Unit* unit = func != nullptr ? func->unit() : nullptr;
  if (UNLIKELY(func == nullptr)) {
    TRACE(5, "Could not grab stack information\n");
    return;
  }

  // If we are no longer on the active line breakpoint, clear it
  RequestInjectionData& req_data = ThreadInfo::s_threadInfo->m_reqInjectionData;
  int active_line = req_data.getActiveLineBreak();
  int line = unit->getLineNumber(unit->offsetOf(pc));
  if (UNLIKELY(active_line != -1 && active_line != line)) {
    req_data.setActiveLineBreak(-1);
  }

  // Check if we are hitting a call breakpoint
  if (UNLIKELY(g_context->m_callBreakPointFilter.checkPC(pc))) {
    handler->onFuncEntryBreak(func);
  }

  // Check if we are hitting a return breakpoint
  if (UNLIKELY(g_context->m_retBreakPointFilter.checkPC(pc))) {
    handler->onFuncExitBreak(func);
  }

  // Check if we are hitting a line breakpoint. Also ensure the current line
  // hasn't already been set as the active line breakpoint.
  if (UNLIKELY(g_context->m_lineBreakPointFilter.checkPC(pc) &&
               active_line != line)) {
    req_data.setActiveLineBreak(line);
    handler->onLineBreak(unit, line);
  }

  TRACE(5, "out phpDebuggerOpcodeHook()\n");
}

// Hook called on function entry. Since function entry breakpoints are handled
// by onOpcode, this just handles pushing the active line breakpoint
void phpDebuggerFuncEntryHook(const ActRec* ar) {
  ThreadInfo::s_threadInfo->m_reqInjectionData.pushActiveLineBreak(-1);
}

// Hook called on function exit. onOpcode handles function exit breakpoints,
// this just handles popping the active line breakpoint. This handles returns,
// suspends, and exceptions.
void phpDebuggerFuncExitHook(const ActRec* ar) {
  ThreadInfo::s_threadInfo->m_reqInjectionData.popActiveLineBreak();
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
void phpDebuggerExceptionHandlerHook() {
  TRACE(5, "in phpDebuggerExceptionHandlerHook()\n");
  if (UNLIKELY(g_context->m_dbgNoBreak)) {
    TRACE(5, "NoBreak flag is on\n");
    return;
  }
  getHookHandler()->onExceptionHandle();
  TRACE(5, "out phpDebuggerExceptionHandlerHook()\n");
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
      g_context->preventReturnsToTC();
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
  g_context->m_callBreakPointFilter.addPC(pc);

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
    g_context->m_retBreakPointFilter.addPC(pc);

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
  g_context->m_lineBreakPointFilter.addRanges(unit, offsets);
  return true;
}

void phpRemoveBreakPoint(const Unit* unit, Offset offset) {
  if (g_context->m_breakPointFilter) {
    PC pc = unit->at(offset);
    g_context->m_breakPointFilter->removePC(pc);
  }
}

bool phpHasBreakpoint(const Unit* unit, Offset offset) {
  if (g_context->m_breakPointFilter) {
    PC pc = unit->at(offset);
    return g_context->m_breakPointFilter->checkPC(pc);
  }
  return false;
}

/////////////////////////////////////////////////////////////////////////
}
