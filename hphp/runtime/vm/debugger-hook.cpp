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

#include "hphp/runtime/vm/debugger-hook.h"
#include "hphp/runtime/vm/jit/translator-x64.h"
#include "hphp/runtime/debugger/break_point.h"
#include "hphp/runtime/debugger/debugger.h"
#include "hphp/runtime/debugger/debugger_proxy.h"
#include "hphp/runtime/base/file-repository.h"
#include "hphp/util/logger.h"
#include "hphp/util/util.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(debuggerflow);
using JIT::tx64;

// Hook called from the bytecode interpreter before every opcode executed while
// a debugger is attached. The debugger may choose to hold the thread below
// here and execute any number of commands from the client. Return from here
// lets the opcode execute.
void phpDebuggerOpcodeHook(const uchar* pc) {
  TRACE(5, "in phpDebuggerOpcodeHook() with pc %p\n", pc);
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
    TRACE_RB(5, "Location filter hit at pc %p\n", pc);
    return;
  }
  // Are we hitting a breakpoint?
  if (LIKELY(g_vmContext->m_breakPointFilter == nullptr ||
      !g_vmContext->m_breakPointFilter->checkPC(pc))) {
    TRACE(5, "not in the PC range for any breakpoints\n");
    if (LIKELY(!DEBUGGER_FORCE_INTR)) {
      return;
    }
    TRACE_RB(5, "DEBUGGER_FORCE_INTR\n");
  }
  Eval::Debugger::InterruptVMHook();
  TRACE(5, "out phpDebuggerOpcodeHook()\n");
}

// Hook called from iopThrow to signal that we are about to throw an exception.
void phpDebuggerExceptionThrownHook(ObjectData* exception) {
  TRACE(5, "in phpDebuggerExceptionThrownHook()\n");
  if (UNLIKELY(g_vmContext->m_dbgNoBreak)) {
    TRACE(5, "NoBreak flag is on\n");
    return;
  }
  Eval::Debugger::InterruptVMHook(Eval::ExceptionThrown, exception);
  TRACE(5, "out phpDebuggerExceptionThrownHook()\n");
}

// Hook called from exception unwind to signal that we are about to handle an
// exception.
void phpDebuggerExceptionHandlerHook() {
  TRACE(5, "in phpDebuggerExceptionHandlerHook()\n");
  if (UNLIKELY(g_vmContext->m_dbgNoBreak)) {
    TRACE(5, "NoBreak flag is on\n");
    return;
  }
  Eval::Debugger::InterruptVMHook(Eval::ExceptionHandler);
  TRACE(5, "out phpDebuggerExceptionHandlerHook()\n");
}

// Hook called when the VM raises an error.
void phpDebuggerErrorHook(const std::string& message) {
  TRACE(5, "in phpDebuggerErrorHook()\n");
  if (UNLIKELY(g_vmContext->m_dbgNoBreak)) {
    TRACE(5, "NoBreak flag is on\n");
    return;
  }
  Eval::Debugger::InterruptVMHook(Eval::ExceptionThrown, String(message));
  TRACE(5, "out phpDebuggerErrorHook()\n");
}

bool isDebuggerAttachedProcess() {
  return Eval::Debugger::CountConnectedProxy() > 0;
}

// Ensure we interpret all code at the given offsets. This sets up a guard for
// each piece of translated code to ensure we punt to the interpreter when the
// debugger is attached.
static void blacklistRangesInJit(const Unit* unit,
                                 const OffsetRangeVec& offsets) {
  for (OffsetRangeVec::const_iterator it = offsets.begin();
       it != offsets.end(); ++it) {
    for (PC pc = unit->at(it->m_base); pc < unit->at(it->m_past);
         pc += instrLen((Op*)pc)) {
      tx64->addDbgBLPC(pc);
    }
  }
  if (!tx64->addDbgGuards(unit)) {
    Logger::Warning("Failed to set breakpoints in Jitted code");
  }
  // In this case, we may be setting a breakpoint in a tracelet which could
  // already be jitted, and present on the stack. Make sure we don't return
  // to it so we have a chance to honor breakpoints.
  g_vmContext->preventReturnsToTC();
}

// Ensure we interpret an entire function when the debugger is attached.
static void blacklistFuncInJit(const Func* f) {
  Unit* unit = f->unit();
  OffsetRangeVec ranges;
  ranges.push_back(OffsetRange(f->base(), f->past()));
  blacklistRangesInJit(unit, ranges);
}

static PCFilter *getBreakPointFilter() {
  if (!g_vmContext->m_breakPointFilter) {
    g_vmContext->m_breakPointFilter = new PCFilter();
  }
  return g_vmContext->m_breakPointFilter;
}

// Looks up the offset range in the given unit, of the given breakpoint.
// If the offset cannot be found, the breakpoint is marked as invalid.
// Otherwise it is marked as valid and the offset is added to the
// breakpoint filter and the offset range is black listed for the JIT.
static void addBreakPointInUnit(Eval::BreakPointInfoPtr bp, Unit* unit) {
  OffsetRangeVec offsets;
  if (!unit->getOffsetRanges(bp->m_line1, offsets) || offsets.size() == 0) {
    bp->m_bindState = Eval::BreakPointInfo::KnownToBeInvalid;
    return;
  }
  bp->m_bindState = Eval::BreakPointInfo::KnownToBeValid;
  TRACE(3, "Add to breakpoint filter for %s:%d, unit %p:\n",
      unit->filepath()->data(), bp->m_line1, unit);
  getBreakPointFilter()->addRanges(unit, offsets);
  if (RuntimeOption::EvalJit) {
    blacklistRangesInJit(unit, offsets);
  }
}

static void addBreakPointsInFile(Eval::DebuggerProxy* proxy,
                                 Eval::PhpFile* efile) {
  Eval::BreakPointInfoPtrVec bps;
  proxy->getBreakPoints(bps);
  for (unsigned int i = 0; i < bps.size(); i++) {
    Eval::BreakPointInfoPtr bp = bps[i];
    if (Eval::BreakPointInfo::MatchFile(bp->m_file, efile->getFileName())) {
      addBreakPointInUnit(bp, efile->unit());
    }
  }
}

static void addBreakPointFuncEntry(const Func* f) {
  PC pc = f->unit()->at(f->base());
  TRACE(5, "func() break %s : unit %p offset %d ==> pc %p)\n",
        f->fullName()->data(), f->unit(), f->base(), pc);
  getBreakPointFilter()->addPC(pc);
  if (RuntimeOption::EvalJit) {
    if (tx64->addDbgBLPC(pc)) {
      // if a new entry is added in blacklist
      if (!tx64->addDbgGuard(f, f->base())) {
        Logger::Warning("Failed to set breakpoints in Jitted code");
      }
    }
  }
}

// See if the given name matches the function's name. For generators,
// it will only return true if the given function is the generator,
// not the stub which makes the generator. I.e., given name="genFoo"
// this will return true when f's name is "genFoo$continuation", and
// false for "genFoo". Note that while async functions follow the same
// general codegen strategy as generators, the original function still
// starts the work, so we treat generators from async functions
// normally.
static bool matchFunctionName(string name, const Func* f) {
  if (f->hasGeneratorAsBody() && !f->isAsync()) return false; // Original func
  auto funcName = f->name()->data();
  if (!f->isGenerator() || f->isAsync()) {
    return name == funcName;
  } else {
    DEBUG_ONLY string s(funcName);
    assert(s.compare(s.length() - 13, string::npos, "$continuation") == 0);
    return name.compare(0, string::npos, funcName, strlen(funcName) - 13) == 0;
  }
}

// If the proxy has an enabled breakpoint that matches entry into the given
// function, arrange for the VM to stop execution and notify the debugger
// whenever execution enters the given function.
static void addBreakPointFuncEntry(Eval::DebuggerProxy* proxy, const Func* f) {
  Eval::BreakPointInfoPtrVec bps;
  proxy->getBreakPoints(bps);
  for (unsigned int i = 0; i < bps.size(); i++) {
    Eval::BreakPointInfoPtr bp = bps[i];
    if (bp->m_state == Eval::BreakPointInfo::Disabled) continue;
    if (!matchFunctionName(bp->getFuncName(), f)) continue;
    bp->m_bindState = Eval::BreakPointInfo::KnownToBeValid;
    addBreakPointFuncEntry(f);
    return;
  }
}

// If the proxy has enabled breakpoints that match entry into methods of
// the given class, arrange for the VM to stop execution and notify the debugger
// whenever execution enters one of these matched method.
// This function is called once, when a class is first loaded, so it is not
// performance critical.
static void addBreakPointsClass(Eval::DebuggerProxy* proxy, const Class* cls) {
  size_t numFuncs = cls->numMethods();
  if (numFuncs == 0) return;
  auto clsName = cls->name();
  auto funcs = cls->methods();
  Eval::BreakPointInfoPtrVec bps;
  proxy->getBreakPoints(bps);
  for (unsigned int i = 0; i < bps.size(); i++) {
    Eval::BreakPointInfoPtr bp = bps[i];
    if (bp->m_state == Eval::BreakPointInfo::Disabled) continue;
    // TODO: check name space separately
    if (bp->getClass() != clsName->data()) continue;
    bp->m_bindState = Eval::BreakPointInfo::KnownToBeInvalid;
    for (size_t i = 0; i < numFuncs; ++i) {
      auto f = funcs[i];
      if (!matchFunctionName(bp->getFunction(), f)) continue;
      bp->m_bindState = Eval::BreakPointInfo::KnownToBeValid;
      addBreakPointFuncEntry(f);
    }
  }
}

void phpAddBreakPoint(const Unit* unit, Offset offset) {
  PC pc = unit->at(offset);
  getBreakPointFilter()->addPC(pc);
  if (RuntimeOption::EvalJit) {
    if (tx64->addDbgBLPC(pc)) {
      // if a new entry is added in blacklist
      if (!tx64->addDbgGuards(unit)) {
        Logger::Warning("Failed to set breakpoints in Jitted code");
      }
      // In this case, we may be setting a breakpoint in a tracelet which could
      // already be jitted, and present on the stack. Make sure we don't return
      // to it so we have a chance to honor breakpoints.
      g_vmContext->preventReturnsToTC();
    }
  }
}

void phpRemoveBreakPoint(const Unit* unit, Offset offset) {
  if (g_vmContext->m_breakPointFilter) {
    PC pc = unit->at(offset);
    g_vmContext->m_breakPointFilter->removePC(pc);
  }
}

bool phpHasBreakpoint(const Unit* unit, Offset offset) {
  if (g_vmContext->m_breakPointFilter) {
    PC pc = unit->at(offset);
    return g_vmContext->m_breakPointFilter->checkPC(pc);
  }
  return false;
}

void phpDebuggerEvalHook(const Func* f) {
  if (RuntimeOption::EvalJit) {
    blacklistFuncInJit(f);
  }
}

// Called by the VM when a file is loaded.
void phpDebuggerFileLoadHook(Eval::PhpFile* efile) {
  Eval::DebuggerProxyPtr proxy = Eval::Debugger::GetProxy();
  if (proxy == nullptr) return;
  addBreakPointsInFile(proxy.get(), efile);
}

// Called by the VM when a class definition is loaded.
void phpDebuggerDefClassHook(const Class* cls) {
  Eval::DebuggerProxyPtr proxy = Eval::Debugger::GetProxy();
  if (proxy == nullptr) return;
  addBreakPointsClass(proxy.get(), cls);
}

// Called by the VM when a function definition is loaded.
void phpDebuggerDefFuncHook(const Func* func) {
  Eval::DebuggerProxyPtr proxy = Eval::Debugger::GetProxy();
  if (proxy == nullptr) return;
  addBreakPointFuncEntry(proxy.get(), func);
}

// Called by the proxy whenever its breakpoint list is updated.
// Since this intended to be called when user input is received, it is not
// performance critical. Also, in typical scenarios, the list is short.
void phpSetBreakPoints(Eval::DebuggerProxy* proxy) {
  Eval::BreakPointInfoPtrVec bps;
  proxy->getBreakPoints(bps);
  for (unsigned int i = 0; i < bps.size(); i++) {
    Eval::BreakPointInfoPtr bp = bps[i];
    bp->m_bindState = Eval::BreakPointInfo::Unknown;
    auto className = bp->getClass();
    if (!className.empty()) {
      auto clsName = makeStaticString(className);
      auto cls = Unit::lookupClass(clsName);
      if (cls == nullptr) continue;
      bp->m_bindState = Eval::BreakPointInfo::KnownToBeInvalid;
      size_t numFuncs = cls->numMethods();
      if (numFuncs == 0) continue;
      auto methodName = bp->getFunction();
      Func* const* funcs = cls->methods();
      for (size_t i = 0; i < numFuncs; ++i) {
        auto f = funcs[i];
        if (!matchFunctionName(methodName, f)) continue;
        bp->m_bindState = Eval::BreakPointInfo::KnownToBeValid;
        addBreakPointFuncEntry(f);
        break;
      }
      //TODO: what about superclass methods accessed via the derived class?
      //Task 2527229.
      continue;
    }
    auto funcName = bp->getFuncName();
    if (!funcName.empty()) {
      auto fName = makeStaticString(funcName);
      Func* f = Unit::lookupFunc(fName);
      if (f == nullptr) continue;
      if (f->hasGeneratorAsBody() && !f->isAsync()) {
        // This function is a generator, and it's the original
        // function which has been turned into a stub which creates a
        // continuation. We want to set the breakpoint on the
        // continuation function instead.
        fName = makeStaticString(funcName + "$continuation");
        f = Unit::lookupFunc(fName);
        if (f == nullptr) continue;
      }
      bp->m_bindState = Eval::BreakPointInfo::KnownToBeValid;
      addBreakPointFuncEntry(f);
      continue;
    }
    auto fileName = bp->m_file;
    if (!fileName.empty()) {
      for (EvaledFilesMap::const_iterator it =
           g_vmContext->m_evaledFiles.begin();
           it != g_vmContext->m_evaledFiles.end(); ++it) {
        auto efile = it->second;
        if (!Eval::BreakPointInfo::MatchFile(fileName, efile->getFileName())) {
          continue;
        }
        addBreakPointInUnit(bp, efile->unit());
        break;
      }
      continue;
    }
    auto exceptionClassName = bp->getExceptionClass();
    if (exceptionClassName == "@") {
      bp->m_bindState = Eval::BreakPointInfo::KnownToBeValid;
      continue;
    } else if (!exceptionClassName.empty()) {
      auto expClsName = makeStaticString(exceptionClassName);
      auto cls = Unit::lookupClass(expClsName);
      if (cls != nullptr) {
        auto baseClsName = makeStaticString("Exception");
        auto baseCls = Unit::lookupClass(baseClsName);
        if (baseCls != nullptr) {
          if (cls->classof(baseCls)) {
            bp->m_bindState = Eval::BreakPointInfo::KnownToBeValid;
          } else {
            bp->m_bindState = Eval::BreakPointInfo::KnownToBeInvalid;
          }
        }
      }
      continue;
    } else {
      continue;
    }
    // If we get here, the break point is of a type that does
    // not need to be explicitly enabled in the VM. For example
    // a break point that get's triggered when the server starts
    // to process a page request.
    bp->m_bindState = Eval::BreakPointInfo::KnownToBeValid;
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

// Adds a range of PCs to the filter given a collection of offset ranges.
// Omit PCs which have opcodes that don't pass the given opcode filter.
void PCFilter::addRanges(const Unit* unit, const OffsetRangeVec& offsets,
                         OpcodeFilter isOpcodeAllowed) {
  for (auto range = offsets.cbegin(); range != offsets.cend(); ++range) {
    TRACE(3, "\toffsets [%d, %d)\n", range->m_base, range->m_past);
    for (PC pc = unit->at(range->m_base); pc < unit->at(range->m_past);
         pc += instrLen((Op*)pc)) {
      if (isOpcodeAllowed(toOp(*pc))) {
        TRACE(3, "\t\tpc %p\n", pc);
        addPC(pc);
      } else {
        TRACE(3, "\t\tpc %p -- skipping (offset %d)\n", pc, unit->offsetOf(pc));
      }
    }
  }
}

void PCFilter::removeOffset(const Unit* unit, Offset offset) {
  removePC(unit->at(offset));
}

//////////////////////////////////////////////////////////////////////////
}
