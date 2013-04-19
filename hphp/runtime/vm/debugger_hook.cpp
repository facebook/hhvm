/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "util/util.h"

#include <runtime/eval/debugger/debugger.h>
#include <runtime/eval/debugger/debugger_proxy.h>
#include <runtime/eval/debugger/break_point.h>
#include <runtime/eval/runtime/file_repository.h>
#include <runtime/vm/unit.h>
#include <runtime/vm/translator/translator.h>
#include <runtime/vm/debugger_hook.h>
#include <util/logger.h>

namespace HPHP {
namespace VM {

//////////////////////////////////////////////////////////////////////////

static const Trace::Module TRACEMOD = Trace::bcinterp;

static inline Transl::Translator* transl() {
  return Transl::Translator::Get();
}

// Hook called from the bytecode interpreter before every opcode executed while
// a debugger is attached. The debugger may choose to hold the thread below
// here and execute any number of commands from the client. Return from here
// lets the opcode execute.
void phpDebuggerOpcodeHook(const uchar* pc) {
  TRACE(5, "in phpDebuggerHook()\n");
  // Short-circuit when we're doing things like evaling PHP for print command,
  // or conditional breakpoints.
  if (UNLIKELY(g_vmContext->m_dbgNoBreak)) {
    TRACE(5, "NoBreak flag is on\n");
    return;
  }
  // Short-circuit for cases where we're executing a line of code that we know
  // we don't need an interrupt for, e.g., stepping over a line of code.
  if (UNLIKELY(g_vmContext->m_lastLocFilter != nullptr) &&
      g_vmContext->m_lastLocFilter->checkPC(pc)) {
    TRACE(5, "same location as last interrupt\n");
    return;
  }
  // Are we hitting a breakpoint?
  if (LIKELY(g_vmContext->m_breakPointFilter == nullptr ||
      !g_vmContext->m_breakPointFilter->checkPC(pc))) {
    TRACE(5, "not in the PC range for any breakpoints\n");
    if (LIKELY(!DEBUGGER_FORCE_INTR)) {
      // Implies we left the location for last break.
      delete g_vmContext->m_lastLocFilter;
      g_vmContext->m_lastLocFilter = nullptr;
      return;
    }
    TRACE(5, "DEBUGGER_FORCE_INTR\n");
  }
  Eval::Debugger::InterruptVMHook();
  TRACE(5, "out phpDebuggerHook()\n");
}

// Hook called from iopThrow to signal that we are about to throw an exception.
// NB: this does not hook any portion of exception unwind.
void phpDebuggerExceptionHook(ObjectData* e) {
  TRACE(5, "in phpExceptionHook()\n");
  if (UNLIKELY(g_vmContext->m_dbgNoBreak)) {
    TRACE(5, "NoBreak flag is on\n");
    return;
  }
  Eval::Debugger::InterruptVMHook(Eval::ExceptionThrown, e);
  TRACE(5, "out phpExceptionHook()\n");
}

bool isDebuggerAttachedProcess() {
  return Eval::Debugger::CountConnectedProxy() > 0;
}

// Ensure we interpret all code at the given offsets. This sets up a guard for
// each piece of tranlated code to ensure we punt ot the interpreter when the
// debugger is attached.
static void blacklistRangesInJit(const Unit* unit,
                                 const OffsetRangeVec& offsets) {
  for (OffsetRangeVec::const_iterator it = offsets.begin();
       it != offsets.end(); ++it) {
    for (PC pc = unit->at(it->m_base); pc < unit->at(it->m_past);
         pc += instrLen((Opcode*)pc)) {
      transl()->addDbgBLPC(pc);
    }
  }
  if (!transl()->addDbgGuards(unit)) {
    Logger::Warning("Failed to set breakpoints in Jitted code");
  }
}

// Ensure we interpret an entire function when the debugger is attached.
static void blacklistFuncInJit(const Func* f) {
  Unit* unit = f->unit();
  OffsetRangeVec ranges;
  ranges.push_back(OffsetRange(f->base(), f->past()));
  blacklistRangesInJit(unit, ranges);
}

static void addBreakPointsInFile(Eval::DebuggerProxy* proxy,
                                 Eval::PhpFile* efile) {
  Eval::BreakPointInfoPtrVec bps;
  proxy->getBreakPoints(bps);
  for(unsigned int i = 0; i < bps.size(); i++) {
    Eval::BreakPointInfoPtr bp = bps[i];
    if (bp->m_line1 == 0 || bp->m_file.empty()) {
      // invalid breakpoint for file:line
      continue;
    }
    if (!Eval::BreakPointInfo::MatchFile(bp->m_file, efile->getFileName(),
                                         efile->getRelPath())) {
      continue;
    }
    Unit* unit = efile->unit();
    OffsetRangeVec offsets;
    if (!unit->getOffsetRanges(bp->m_line1, offsets)) {
      continue;
    }
    if (!g_vmContext->m_breakPointFilter) {
      g_vmContext->m_breakPointFilter = new PCFilter();
    }
    if (debug && Trace::moduleEnabled(Trace::bcinterp, 5)) {
      for (OffsetRangeVec::const_iterator it = offsets.begin();
           it != offsets.end(); ++it) {
        Trace::trace("file:line break %s:%d : unit %p offset [%d, %d)\n",
                     efile->getFileName().c_str(), bp->m_line1, unit,
                     it->m_base, it->m_past);
      }
    }
    g_vmContext->m_breakPointFilter->addRanges(unit, offsets);
    if (RuntimeOption::EvalJit) {
      blacklistRangesInJit(unit, offsets);
    }
  }
}

static void addBreakPointFuncEntry(const Func* f) {
  PC pc = f->unit()->at(f->base());
  if (!g_vmContext->m_breakPointFilter) {
    g_vmContext->m_breakPointFilter = new PCFilter();
  }
  TRACE(5, "func() break %s : unit %p offset %d)\n",
        f->fullName()->data(), f->unit(), f->base());
  g_vmContext->m_breakPointFilter->addPC(pc);
  if (RuntimeOption::EvalJit) {
    if (transl()->addDbgBLPC(pc)) {
      // if a new entry is added in blacklist
      if (!transl()->addDbgGuard(f, f->base())) {
        Logger::Warning("Failed to set breakpoints in Jitted code");
      }
    }
  }
}

static void addBreakPointsClass(Eval::DebuggerProxy* proxy,
                                const Class* cls) {
  size_t numFuncs = cls->numMethods();
  Func* const* funcs = cls->methods();
  for (size_t i = 0; i < numFuncs; ++i) {
    if (proxy->couldBreakEnterFunc(funcs[i]->fullName())) {
      addBreakPointFuncEntry(funcs[i]);
    }
  }
}

void phpDebuggerEvalHook(const Func* f) {
  if (RuntimeOption::EvalJit) {
    blacklistFuncInJit(f);
  }
}

// Hook called by the VM when a file is loaded. Gives the debugger a chance
// to apply any pending breakpoints that might be in the file.
void phpDebuggerFileLoadHook(Eval::PhpFile* efile) {
  Eval::DebuggerProxyPtr proxy = Eval::Debugger::GetProxy();
  if (!proxy) {
    return;
  }
  addBreakPointsInFile(proxy.get(), efile);
}

void phpDebuggerDefClassHook(const Class* cls) {
  Eval::DebuggerProxyPtr proxy = Eval::Debugger::GetProxy();
  if (!proxy) {
    return;
  }
  addBreakPointsClass(proxy.get(), cls);
}

void phpDebuggerDefFuncHook(const Func* func) {
  Eval::DebuggerProxyPtr proxy = Eval::Debugger::GetProxy();
  if (proxy && proxy->couldBreakEnterFunc(func->fullName())) {
    addBreakPointFuncEntry(func);
  }
}

// Helper which will look at every loaded file and attempt to see if any
// existing file:line breakpoints should be set.
void phpSetBreakPointsInAllFiles(Eval::DebuggerProxyVM* proxy) {
  for (EvaledFilesMap::const_iterator it =
       g_vmContext->m_evaledFiles.begin();
       it != g_vmContext->m_evaledFiles.end(); ++it) {
    addBreakPointsInFile(proxy, it->second);
  }

  std::vector<const StringData*> clsNames;
  proxy->getBreakClsMethods(clsNames);
  for (unsigned int i = 0; i < clsNames.size(); i++) {
    Class* cls = Unit::lookupClass(clsNames[i]);
    if (cls) {
      addBreakPointsClass(proxy, cls);
    }
  }

  std::vector<const StringData*> funcFullNames;
  proxy->getBreakFuncs(funcFullNames);
  for (unsigned int i = 0; i < funcFullNames.size(); i++) {
    // This list contains class method as well but they shouldn't hit anything
    Func* f = Unit::lookupFunc(funcFullNames[i]);
    if (f) {
      addBreakPointFuncEntry(f);
    }
  }
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

int PCFilter::addRanges(const Unit* unit, const OffsetRangeVec& offsets) {
  int counter = 0;
  for (auto range = offsets.cbegin(); range != offsets.cend(); ++range) {
    for (PC pc = unit->at(range->m_base); pc < unit->at(range->m_past);
         pc += instrLen((Opcode*)pc)) {
      addPC(pc);
      counter++;
    }
  }
  return counter;
}

//////////////////////////////////////////////////////////////////////////
}}
