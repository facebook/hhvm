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
    TRACE_RB(5, "DEBUGGER_FORCE_INTR\n");
  }
  getHookHandler()->onOpcode(pc);
  TRACE(5, "out phpDebuggerOpcodeHook()\n");
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
  getBreakPointFilter()->addPC(pc);
  if (RuntimeOption::EvalJit) {
    if (mcg->tx().addDbgBLPC(pc)) {
      // if a new entry is added in blacklist
      if (!mcg->addDbgGuard(f, base, false)) {
        Logger::Warning("Failed to set breakpoints in Jitted code");
      }
    }
  }
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

//////////////////////////////////////////////////////////////////////////

struct PCFilter::PtrMapNode {
  void **m_entries;
  void clearImpl(unsigned short bits);
};

void PCFilter::PtrMapNode::clearImpl(unsigned short bits) {
  // clear all the sub levels and mark all slots NULL
  if (bits <= PTRMAP_LEVEL_BITS) {
    assert(bits == PTRMAP_LEVEL_BITS);
    // On bottom level, pointers are not PtrMapNode*
    memset(m_entries, 0, sizeof(void*) * PTRMAP_LEVEL_ENTRIES);
    return;
  }
  for (int i = 0; i < PTRMAP_LEVEL_ENTRIES; i++) {
    if (m_entries[i]) {
      ((PCFilter::PtrMapNode*)m_entries[i])->clearImpl(bits -
                                                       PTRMAP_LEVEL_BITS);
      free(((PCFilter::PtrMapNode*)m_entries[i])->m_entries);
      free(m_entries[i]);
      m_entries[i] = nullptr;
    }
  }
}

PCFilter::PtrMapNode* PCFilter::PtrMap::MakeNode() {
  PtrMapNode* node = (PtrMapNode*)malloc(sizeof(PtrMapNode));
  node->m_entries =
    (void**)calloc(1, PTRMAP_LEVEL_ENTRIES * sizeof(void*));
  return node;
}

PCFilter::PtrMap::~PtrMap() {
  clear();
  free(m_root->m_entries);
  free(m_root);
}

void* PCFilter::PtrMap::getPointer(void* ptr) {
  PtrMapNode* current = m_root;
  unsigned short cursor = PTRMAP_PTR_SIZE;
  while (current && cursor) {
    cursor -= PTRMAP_LEVEL_BITS;
    unsigned long index = ((PTRMAP_LEVEL_MASK << cursor) & (unsigned long)ptr)
                          >> cursor;
    assert(index < PTRMAP_LEVEL_ENTRIES);
    current = (PtrMapNode*)(current->m_entries[index]);
  }
  return (void*)current;
}

void PCFilter::PtrMap::setPointer(void* ptr, void* val) {
  PtrMapNode* current = m_root;
  unsigned short cursor = PTRMAP_PTR_SIZE;
  while (true) {
    cursor -= PTRMAP_LEVEL_BITS;
    unsigned long index = ((PTRMAP_LEVEL_MASK << cursor) & (unsigned long)ptr)
                          >> cursor;
    assert(index < PTRMAP_LEVEL_ENTRIES);
    if (!cursor) {
      current->m_entries[index] = val;
      break;
    }
    if (!current->m_entries[index])  {
      current->m_entries[index] = (void*) MakeNode();
    }
    current = (PtrMapNode*)(current->m_entries[index]);
  }
}

void PCFilter::PtrMap::clear() {
  m_root->clearImpl(PTRMAP_PTR_SIZE);
}

// Adds a range of PCs to the filter given a collection of offset ranges.
// Omit PCs which have opcodes that don't pass the given opcode filter.
void PCFilter::addRanges(const Unit* unit, const OffsetRangeVec& offsets,
                         OpcodeFilter isOpcodeAllowed) {
  for (auto range = offsets.cbegin(); range != offsets.cend(); ++range) {
    TRACE(3, "\toffsets [%d, %d)\n", range->m_base, range->m_past);
    for (PC pc = unit->at(range->m_base); pc < unit->at(range->m_past);
         pc += instrLen((Op*)pc)) {
      if (isOpcodeAllowed(*reinterpret_cast<const Op*>(pc))) {
        TRACE(3, "\t\tpc %p\n", pc);
        addPC(pc);
      } else {
        TRACE(3, "\t\tpc %p -- skipping (offset %d)\n", pc,
          unit->offsetOf(pc));
      }
    }
  }
}

void PCFilter::removeOffset(const Unit* unit, Offset offset) {
  removePC(unit->at(offset));
}

//////////////////////////////////////////////////////////////////////////
}
